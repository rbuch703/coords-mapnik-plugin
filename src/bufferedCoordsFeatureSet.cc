// mapnik
#include <mapnik/feature_factory.hpp>
#include <mapnik/geometry.hpp>

// boost
#include <boost/make_shared.hpp>

#include "bufferedCoordsFeatureSet.h"
#include "varInt.h"

#include <iostream>
#include <algorithm>
#include <unistd.h> //for stat()
#include <sys/stat.h>

using std::cout;
using std::endl;
using std::pair;
using std::string;

typedef mapnik::box2d<double> Box2D;

static double INT_TO_MERCATOR_METERS = 1/100.0;
static double HALF_EARTH_CIRCUMFERENCE = 20037508.34;

template <typename T>
bool biggerFirst( const T& a, const T& b) { return a.first > b.first;}

BufferedCoordsFeatureSet::BufferedCoordsFeatureSet(/*GEOMETRY_TYPE geoType,*/ Box2D const& box, std::string const& encoding, std::string path, std::set<std::string> pPropertyNames)
    : feature_id_(1),
      tr_(new mapnik::transcoder(encoding)),
      ctx_(boost::make_shared<mapnik::context_type>())
{

    box_ = Envelope( box.minx() / INT_TO_MERCATOR_METERS, 
                     box.maxx() / INT_TO_MERCATOR_METERS, 
                     box.miny() / INT_TO_MERCATOR_METERS, 
                     box.maxy() / INT_TO_MERCATOR_METERS);
    readFiles(path, box, mapnik::box2d<double>(
        -HALF_EARTH_CIRCUMFERENCE, -HALF_EARTH_CIRCUMFERENCE, 
         HALF_EARTH_CIRCUMFERENCE,  HALF_EARTH_CIRCUMFERENCE));
    
    cout << "registered " << (loadedGeometries.size()/1000) << "k geometries for " 
         << path << endl;
         
    isRoad = path.find("road") != string::npos;
    if (isRoad)
    {
        std::sort( loadedGeometries.begin(), loadedGeometries.end(), 
               biggerFirst<std::pair<int8_t, GenericGeometry*> >);
        //cout << "SORTING!!!" << endl;
        
        /*for ( std::pair<int8_t, GenericGeometry*> p : loadedGeometries)
        {
            cout << (int)p.first << ", " << (int)p.second->getZIndex() << endl;
        }*/
        
    }            

//    fData = fopen(path.c_str(), "rb");
    
    for (const std::string & propName : pPropertyNames)
    {
        propertyNamesRaw.push_back( strdup(propName.c_str()) );
        propertyNames.insert(propertyNamesRaw.back());
    }

    // the featureset context needs to know the field schema
    for (const std::string &s: propertyNames)
        ctx_->push(s); // register attributes
}

BufferedCoordsFeatureSet::~BufferedCoordsFeatureSet() 
{ 
    for (char* ch : propertyNamesRaw)
        free(ch);
        
    for ( std::pair<int8_t, GenericGeometry*> geom : loadedGeometries)
        delete geom.second;
}

void BufferedCoordsFeatureSet::loadFileContents( const std::string &fileName)
{
        FILE* f = fopen(fileName.c_str(), "rb");
        fseek(f, 0, SEEK_END);
        uint64_t fileSize = ftell(f);
        rewind(f);
        uint8_t* data = new uint8_t[fileSize];
#ifndef NDEBUG
        int nRead =
#endif
        fread(data, fileSize, 1, f);
        assert(nRead == 1 && "read error");
        fclose(f);
        
        uint8_t *pos = data;
        uint8_t *beyond = data + fileSize;
        
        while (pos < beyond)
        {
            uint32_t numBytes = *(uint32_t*)pos;
            pos += sizeof(uint32_t);
            GenericGeometry *geom = new GenericGeometry(pos, numBytes, false);
            loadedGeometries.push_back( std::make_pair( geom->getZIndex(), geom));
            pos += numBytes;
        }
        
        assert( pos == beyond && "overflow");
        
        delete [] data;

}

void BufferedCoordsFeatureSet::readFiles(std::string path, 
                               const mapnik::box2d<double> &queryBounds, Box2D tileBounds)
{
    //cout << "tile bounds for " << path << " are " << tileBounds << queryBounds.intersect(tileBounds).valid() << endl;
    //if current tile is outside the query bounds
    if (! queryBounds.intersect(tileBounds).valid() )
        return;
        
    struct stat buf;
    int res = stat(path.c_str(), &buf);
    if (res != 0) return;
    
    if (! S_ISREG(buf.st_mode)) return;
    
    /* If the file is empty, it does not contain relevant data, but it may have child
     * nodes that do. So do not parse its contents, but still recurse
     * to its child nodes. */
    if ( buf.st_size > 0)
        loadFileContents(path);

    double yMin = tileBounds.miny();
    double yMax = tileBounds.maxy();
    double yMid = (yMin + yMax) / 2.0;
    
    double xMin = tileBounds.minx();
    double xMax = tileBounds.maxx();
    double xMid = (xMin + xMax) / 2.0;        

    readFiles( path + "0", queryBounds, Box2D(xMin, yMid, xMid, yMax));
    readFiles( path + "1", queryBounds, Box2D(xMid, yMid, xMax, yMax));
    readFiles( path + "2", queryBounds, Box2D(xMin, yMin, xMid, yMid));
    readFiles( path + "3", queryBounds, Box2D(xMid, yMin, xMax, yMid));
}


static bool parsePointGeometry(const GenericGeometry &geom, mapnik::feature_ptr &feature)
{
    const int32_t* pos = (const int32_t*) (geom.getGeometryPtr());

    mapnik::geometry_type * point = new mapnik::geometry_type(mapnik::Point);
    point->move_to( pos[0] * INT_TO_MERCATOR_METERS, pos[1] * INT_TO_MERCATOR_METERS);
    feature->add_geometry(point);
    return true;
}

static bool parseLineGeometry(const uint8_t* geoPtr, mapnik::feature_ptr &feature, const Envelope &bounds)
{
    Envelope polygonBounds;
    int nBytes = 0;

    uint64_t numPoints = varUintFromBytes(geoPtr, &nBytes);
    geoPtr += nBytes;
    
    if (numPoints == 0)
        return false;
        
    int64_t x = varIntFromBytes(geoPtr, &nBytes);
    geoPtr += nBytes;
    
    int64_t y = varIntFromBytes(geoPtr, &nBytes);
    geoPtr += nBytes;
    
    mapnik::geometry_type * line = new mapnik::geometry_type(mapnik::LineString);
    line->move_to( x * INT_TO_MERCATOR_METERS, 
                   y * INT_TO_MERCATOR_METERS);
    polygonBounds.add(x, y);
    
    // skip over 0th point, it was processed by move_to()
    for (uint64_t i = 1; i < numPoints; i++)
    {
        x += varIntFromBytes(geoPtr, &nBytes);
        geoPtr += nBytes;
        y += varIntFromBytes(geoPtr, &nBytes);
        geoPtr += nBytes;
        
        line->line_to( x * INT_TO_MERCATOR_METERS, 
                       y * INT_TO_MERCATOR_METERS);
        polygonBounds.add(x, y);
    }
    
    feature->add_geometry(line);
    return polygonBounds.overlapsWith(bounds);
}

static bool parsePolygonGeometry(const uint8_t* geoPtr, mapnik::feature_ptr &feature, const Envelope &bounds)
{
    Envelope polygonBounds;
    int nBytes = 0;
    uint64_t numRings = varUintFromBytes(geoPtr, &nBytes);
    geoPtr += nBytes;
            
    while (numRings--)
    {
        uint64_t numPoints = varUintFromBytes(geoPtr, &nBytes);
        geoPtr += nBytes;

        assert(numPoints < 10000000 && "overflow");
        if (numPoints == 0)
            continue;
            
        int64_t x = varIntFromBytes(geoPtr, &nBytes);
        geoPtr += nBytes;
        int64_t y = varIntFromBytes(geoPtr, &nBytes);
        geoPtr += nBytes;
        polygonBounds.add(x, y);
        
        mapnik::geometry_type * line = new mapnik::geometry_type(mapnik::Polygon);
        line->move_to( x * INT_TO_MERCATOR_METERS, 
                       y * INT_TO_MERCATOR_METERS);
        
        // skip over 0th point, it was processed by move_to()
        for (uint64_t i = 1; i < numPoints; i++)
        {
            x += varIntFromBytes(geoPtr, &nBytes);
            geoPtr += nBytes;
            y += varIntFromBytes(geoPtr, &nBytes);
            geoPtr += nBytes;
        
            line->line_to( x * INT_TO_MERCATOR_METERS, 
                           y * INT_TO_MERCATOR_METERS);
            polygonBounds.add(x, y);
        }
        
        feature->add_geometry(line);
    }
    //cout << bounds << " vs. " << polygonBounds << endl;
    return bounds.overlapsWith(polygonBounds);
}


mapnik::feature_ptr BufferedCoordsFeatureSet::next()
{

    while (loadedGeometries.size())
    {        
        GenericGeometry *geom = loadedGeometries.back().second;
        loadedGeometries.pop_back();
        /*
        if (isRoad)
            cout << (int)geom->getZIndex() << endl;*/
            
        
        mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx_,feature_id_++));

        for (const pair<const char*, const char*> &kv : geom->getTags())
        {
            if (propertyNames.count(kv.first))
            {
                //cout << "adding " << kv.first << " = " << kv.second << endl; 
                feature->put( kv.first ,tr_->transcode(kv.second) );
            }
        }
        
        bool accepted = false;
        switch (geom->getFeatureType())
        {
            case FEATURE_TYPE::POINT:   
                accepted = parsePointGeometry( *geom, feature);
                break;
            case FEATURE_TYPE::LINE:    
                accepted = parseLineGeometry(  geom->getGeometryPtr(), feature, box_);
                break;
            case FEATURE_TYPE::POLYGON: 
                accepted = parsePolygonGeometry(geom->getGeometryPtr(), feature, box_);
                break;
            default :
                assert(false && "invalid feature type");
                break;
        }
        
        delete geom;
        if (accepted)
            return feature;
    }
    
    return mapnik::feature_ptr();
    
}

