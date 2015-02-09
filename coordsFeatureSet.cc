// mapnik
#include <mapnik/feature_factory.hpp>
#include <mapnik/geometry.hpp>

// boost
#include <boost/make_shared.hpp>

#include "coordsFeatureSet.h"

#include <iostream>
#include <unistd.h> //for stat()
#include <sys/stat.h>

using std::cout;
using std::endl;
using std::pair;
using std::string;

coords_featureset::coords_featureset(GEOMETRY_TYPE geoType, mapnik::box2d<double> const& box, std::string const& encoding, std::string path, std::set<std::string> propertyNames)
    : box_(box),
      feature_id_(1),
      tr_(new mapnik::transcoder(encoding)),
      ctx_(boost::make_shared<mapnik::context_type>()),
      fData(NULL), geometryType(geoType), propertyNames(propertyNames)
{

    buildFileHierarchy(path, files, box, mapnik::box2d<double>(-180, -90, 180, 90));
    
    for (std::string &s: files)
        cout << "\t registered file '" << s << "' for parsing" << endl;

//    fData = fopen(path.c_str(), "rb");

    // the featureset context needs to know the field schema
    for (const std::string &s: propertyNames)
        ctx_->push(s); // register attributes
}

coords_featureset::~coords_featureset() { }

void coords_featureset::buildFileHierarchy(std::string path, std::vector<std::string> &files, 
    const mapnik::box2d<double> &queryBounds, mapnik::box2d<double> tileBounds)
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

    double latMin = tileBounds.miny();
    double latMax = tileBounds.maxy();
    double latMid = (latMin + latMax) / 2.0;
    
    double lngMin = tileBounds.minx();
    double lngMax = tileBounds.maxx();
    double lngMid = (lngMin + lngMax) / 2.0;        

    buildFileHierarchy( path + "0", files, queryBounds, mapnik::box2d<double>(lngMin, latMid, lngMid, latMax));
    buildFileHierarchy( path + "1", files, queryBounds, mapnik::box2d<double>(lngMid, latMid, lngMax, latMax));
    buildFileHierarchy( path + "2", files, queryBounds, mapnik::box2d<double>(lngMin, latMin, lngMid, latMid));
    buildFileHierarchy( path + "3", files, queryBounds, mapnik::box2d<double>(lngMid, latMin, lngMax, latMid));
}

bool coords_featureset::wasReturnedBefore(uint64_t wayId)
{
    if (wayId >= waysReturned.size())
    {
        uint64_t oldSize = waysReturned.size();
        uint64_t newSize = (wayId +1) * 11 / 10;
        waysReturned.resize( newSize);
        for (uint64_t i = oldSize; i < newSize; i++)
            waysReturned[i] = false;
    }
    return waysReturned[wayId];
    //return false;
}

void coords_featureset::markAsReturnedBefore(uint64_t wayId)
{
    if (wayId >= waysReturned.size())
    {
        uint64_t oldSize = waysReturned.size();
        uint64_t newSize = (wayId +1) * 11 / 10;
        waysReturned.resize( newSize);
        for (uint64_t i = oldSize; i < newSize; i++)
            waysReturned[i] = false;        
    }
    
    waysReturned[wayId] = true;
}

boost::optional<OsmLightweightWay> coords_featureset::getNextWay()
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
            OsmLightweightWay way(fData);
            
            //cout << way.id << endl;
            if ( !wasReturnedBefore( way.id) )
            {
                markAsReturnedBefore( way.id);
                return way;
            }
        }

        //already read the whole file -> clean up
        fclose(fData);
        fData = NULL;
    }
    
    return boost::none;
}

mapnik::feature_ptr coords_featureset::next()
{    
    boost::optional<OsmLightweightWay> hasWay = getNextWay();
    if (!hasWay)
        return mapnik::feature_ptr();
        
    OsmLightweightWay &way = *hasWay;

    assert(way.numVertices > 0);
    mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx_,feature_id_++));
//    cout << "way has " << way.numVertices << " vertices" << endl;
    for (std::pair<std::string, std::string> kv : way.getTags())
    {
//        cout << "processing " << kv.first << " = " << kv.second << endl;
        if (propertyNames.count(kv.first))
        {
//            cout << "adding " << kv.first << " = " << kv.second << endl; 
            feature->put( kv.first ,tr_->transcode(kv.second.c_str()) );
        }
//              feature->put( kv.first, kv.second.c_str() );
             
        //cout << kv.first << " -> " << kv.second << endl;
    }
    
    // create a new feature

    
    mapnik::geometry_type * line;
    if (this->geometryType == LINE)
        line = new mapnik::geometry_type(mapnik::LineString);
    else if (this->geometryType == POLYGON)
        line = new mapnik::geometry_type(mapnik::Polygon);
    else 
        assert(false && "Invalid geometry type");
        
    for ( pair<string, string> kv : way.getTags())
    {
        if (propertyNames.count(kv.first))
            feature->put( kv.first, tr_->transcode(kv.second.c_str()));
    }

    double lat = way.vertices[0].lat / 10000000.0;
    double lng = way.vertices[0].lng / 10000000.0;
    line->move_to( lng, lat);
    for (int i = 1; i < way.numVertices; i++)
    {
        double lat = way.vertices[i].lat / 10000000.0;
        double lng = way.vertices[i].lng / 10000000.0;
        line->line_to( lng, lat);
    }
        
    feature->add_geometry(line);
    return feature;

    // otherwise return an empty feature
}

