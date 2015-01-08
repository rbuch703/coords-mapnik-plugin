// mapnik
#include <mapnik/feature_factory.hpp>
#include <mapnik/geometry.hpp>

// boost
#include <boost/make_shared.hpp>

#include "helloFeatureSet.h"

#include <iostream>
#include <unistd.h> //for stat()
#include <sys/stat.h>

using std::cout;
using std::endl;

hello_featureset::hello_featureset(mapnik::box2d<double> const& box, std::string const& encoding, std::string path)
    : box_(box),
      feature_id_(1),
      tr_(new mapnik::transcoder(encoding)),
      ctx_(boost::make_shared<mapnik::context_type>())
{ 

    buildFileHierarchy(path, files, box, mapnik::box2d<double>(-180, -90, 180, 90));
    
    for (std::string &s: files)
        cout << "\t registered file '" << s << "' for parsing" << endl;

    fData = fopen(path.c_str(), "rb");
//    fData = NULL;

    // the featureset context needs to know the field schema
    ctx_->push("key" ); // let us pretend it just has one column/attribute name "key"
}

hello_featureset::~hello_featureset() { }

void hello_featureset::buildFileHierarchy(std::string path, std::vector<std::string> &files, 
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

bool hello_featureset::wasReturnedBefore(uint64_t wayId)
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

void hello_featureset::markAsReturnedBefore(uint64_t wayId)
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

boost::optional<OsmLightweightWay> hello_featureset::getNextWay()
{
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
    return boost::none;
}

mapnik::feature_ptr hello_featureset::next()
{

    if (!fData)
    //if (feature_id_ > 1)
        return mapnik::feature_ptr();
    
    boost::optional<OsmLightweightWay> hasWay = getNextWay();
    if (!hasWay)
        return mapnik::feature_ptr();
        
    OsmLightweightWay &way = *hasWay;
    
    assert(way.numVertices > 0);
    // create a new feature
    mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx_,feature_id_));

    //feature->put( "key" ,tr_->transcode("hello world5!") );
    mapnik::geometry_type * line = new mapnik::geometry_type(mapnik::LineString);

    double lat = way.vertices[0].lat / 10000000.0;
    double lng = way.vertices[0].lng / 10000000.0;
    line->move_to( lng, lat);
    for (int i = 1; i < way.numVertices; i++)
    {
        double lat = way.vertices[i].lat / 10000000.0;
        double lng = way.vertices[i].lng / 10000000.0;
        line->line_to( lng, lat);
    }
        
    //line->line_to(0,0);
    feature->add_geometry(line);
    /*
    // we need a geometry to display so just for fun here
    // we take the center of the bbox that was used to query
    // since we don't actually have any data to pull from...
    mapnik::coord2d center = box_.center();

    // create a new point geometry
    mapnik::geometry_type * pt = new mapnik::geometry_type(mapnik::Point);

    // we use path type geometries in Mapnik to fit nicely with AGG and Cairo
    // here we stick an x,y pair into the geometry using move_to()
    pt->move_to(center.x,center.y);

    // add the geometry to the feature
    feature->add_geometry(pt);

    // A feature usually will have just one geometry of a given type
    // but mapnik does support many geometries per feature of any type
    // so here we draw a line around the point
    mapnik::geometry_type * line = new mapnik::geometry_type(mapnik::LineString);
    line->move_to(box_.minx(),box_.miny());
    line->line_to(box_.maxx(),box_.maxy());
    feature->add_geometry(line); 

    line = new mapnik::geometry_type(mapnik::LineString);
    line->move_to(box_.minx(),box_.maxy());
    line->line_to(box_.maxx(),box_.miny());
    feature->add_geometry(line); */

    //fclose(fData);
    //fData = NULL;
    // return the feature!
    return feature;

    // otherwise return an empty feature
}

