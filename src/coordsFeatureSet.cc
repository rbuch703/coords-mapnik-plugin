// mapnik
#include <mapnik/feature_factory.hpp>
#include <mapnik/geometry.hpp>

// boost
#include <boost/make_shared.hpp>

#include "coordsFeatureSet.h"
#include "varInt.h"

#include <iostream>
#include <unistd.h> //for stat()
#include <sys/stat.h>

using std::cout;
using std::endl;
using std::pair;
using std::string;

typedef mapnik::box2d<double> Box2D;

static double INT_TO_MERCATOR_METERS = 1/100.0;
static double HALF_EARTH_CIRCUMFERENCE = 20037508.34;

coords_featureset::coords_featureset(/*GEOMETRY_TYPE geoType,*/ Box2D const& box, std::string const& encoding, std::string path, std::set<std::string> pPropertyNames)
    : feature_id_(1),
      tr_(new mapnik::transcoder(encoding)),
      ctx_(boost::make_shared<mapnik::context_type>()),
      fData(NULL)/*, geometryType(geoType)*/
{

    box_ = Envelope( box.minx() / INT_TO_MERCATOR_METERS, 
                     box.maxx() / INT_TO_MERCATOR_METERS, 
                     box.miny() / INT_TO_MERCATOR_METERS, 
                     box.maxy() / INT_TO_MERCATOR_METERS);
    buildFileHierarchy(path, files, box, mapnik::box2d<double>(
        -HALF_EARTH_CIRCUMFERENCE, -HALF_EARTH_CIRCUMFERENCE, 
         HALF_EARTH_CIRCUMFERENCE,  HALF_EARTH_CIRCUMFERENCE));
    
    for (std::string &s: files)
        cout << "    register file '" << s << "'" << endl;

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

coords_featureset::~coords_featureset() 
{ 
    for (char* ch : propertyNamesRaw)
        free(ch);
}

void coords_featureset::buildFileHierarchy(std::string path, std::vector<std::string> &files, 
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
     * nodes that do. So do not add it to the list of readable files, but still recurse
     * to its child nodes. */
    if ( buf.st_size > 0)
        files.push_back(path);

    double yMin = tileBounds.miny();
    double yMax = tileBounds.maxy();
    double yMid = (yMin + yMax) / 2.0;
    
    double xMin = tileBounds.minx();
    double xMax = tileBounds.maxx();
    double xMid = (xMin + xMax) / 2.0;        

    buildFileHierarchy( path + "0", files, queryBounds, Box2D(xMin, yMid, xMid, yMax));
    buildFileHierarchy( path + "1", files, queryBounds, Box2D(xMid, yMid, xMax, yMax));
    buildFileHierarchy( path + "2", files, queryBounds, Box2D(xMin, yMin, xMid, yMid));
    buildFileHierarchy( path + "3", files, queryBounds, Box2D(xMid, yMin, xMax, yMid));
}

bool coords_featureset::wasReturnedBefore(OSM_ENTITY_TYPE entityType, uint64_t entityId) const
{

    switch (entityType)
    {
        case OSM_ENTITY_TYPE::NODE: 
            return nodesReturned.count(entityId);
        
        case OSM_ENTITY_TYPE::WAY:  
            return entityId >= waysReturned.size() ? false : waysReturned[entityId];
            
        case OSM_ENTITY_TYPE::RELATION: 
            return entityId >= relationsReturned.size() ? false : relationsReturned[entityId];

        default: 
            assert(false && "invalid entity type");
            return false;
    }
    

}

void coords_featureset::markAsReturnedBefore(OSM_ENTITY_TYPE entityType, uint64_t entityId)
{
    switch (entityType)
    {
        case OSM_ENTITY_TYPE::NODE: 
            nodesReturned.insert(entityId);
            return;
        
        case OSM_ENTITY_TYPE::WAY:  
            if (entityId >= waysReturned.size())
            {
                uint64_t oldSize = waysReturned.size();
                uint64_t newSize = (entityId + 1) * 11 / 10;
                waysReturned.resize( newSize);
                for (uint64_t i = oldSize; i < newSize; i++)
                    waysReturned[i] = false;        
            }
            
            waysReturned[entityId] = true;
            return;
            
        case OSM_ENTITY_TYPE::RELATION: 
            if (entityId >= relationsReturned.size())
            {
                uint64_t oldSize = relationsReturned.size();
                uint64_t newSize = (entityId + 1) * 11 / 10;
                relationsReturned.resize( newSize);
                for (uint64_t i = oldSize; i < newSize; i++)
                    relationsReturned[i] = false;        
            }
            
            relationsReturned[entityId] = true;
            return;

        default: 
            assert(false && "invalid entity type");
            return;
    }

}

bool coords_featureset::getNextGeometry( GenericGeometry &geoOut)
{
//    cout << "has " << files.size() << " files." << endl;    
    while ( files.size() > 0 || fData != NULL)
    {
//        cout << "way " << feature_id_ << endl;
        if (fData == NULL)
        {
            cout << "opening next file '" << files.back() << "#" << endl;
            fData = fopen( files.back().c_str(), "rb");
            files.pop_back();
        }

        int ch;

        while ( (ch = fgetc(fData)) != EOF)
        {
            ungetc(ch, fData);
            geoOut.init(fData, true);
            
            //cout << "read geometry " << geom.getEntityType() << " " << geom.getEntityId() << endl;
            //cout << way.id << endl;
            if ( !wasReturnedBefore( geoOut.getEntityType(), geoOut.getEntityId() ))
            {
                markAsReturnedBefore( geoOut.getEntityType(), geoOut.getEntityId());
                return true;
            }
        }

        //already read the whole file -> clean up
        fclose(fData);
        fData = NULL;
    }
    
    return false;
}

bool parsePointGeometry(const GenericGeometry &geom, mapnik::feature_ptr &feature)
{
    const int32_t* pos = (const int32_t*) (geom.getGeometryPtr());

    mapnik::geometry_type * point = new mapnik::geometry_type(mapnik::Point);
    point->move_to( pos[0] * INT_TO_MERCATOR_METERS, pos[1] * INT_TO_MERCATOR_METERS);
    feature->add_geometry(point);
    return true;
}

bool parseLineGeometry(const uint8_t* geoPtr, mapnik::feature_ptr &feature, const Envelope &bounds)
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

bool parsePolygonGeometry(const uint8_t* geoPtr, mapnik::feature_ptr &feature, const Envelope &bounds)
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


mapnik::feature_ptr coords_featureset::next()
{
    while (getNextGeometry(lastReturnedGeometry))
    {
        
        GenericGeometry &geom = lastReturnedGeometry;

        mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx_,feature_id_++));

        for (const pair<const char*, const char*> &kv : geom.getTags())
        {
            if (propertyNames.count(kv.first))
            {
                //cout << "adding " << kv.first << " = " << kv.second << endl; 
                feature->put( kv.first ,tr_->transcode(kv.second) );
            }
        }
        
        switch (geom.getFeatureType())
        {
            case FEATURE_TYPE::POINT:   
                if (parsePointGeometry( geom, feature)) 
                    return feature; 
                break;
            case FEATURE_TYPE::LINE:    
                if (parseLineGeometry(  geom.getGeometryPtr(), feature, box_)) 
                    return feature;
                break;
            case FEATURE_TYPE::POLYGON: 
                if (parsePolygonGeometry(geom.getGeometryPtr(), feature, box_)) 
                    return feature;
                break;
            default :
                assert(false && "invalid feature type");
                return mapnik::feature_ptr(); 
        }
    }
    
    return mapnik::feature_ptr();
    
}

