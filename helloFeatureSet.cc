// mapnik
#include <mapnik/feature_factory.hpp>
#include <mapnik/geometry.hpp>

// boost
#include <boost/make_shared.hpp>

#include "helloFeatureSet.h"
#include "osmMappedTypes.h"

#include <iostream>

using std::cout;
using std::endl;

hello_featureset::hello_featureset(mapnik::box2d<double> const& box, std::string const& encoding)
    : box_(box),
      feature_id_(1),
      tr_(new mapnik::transcoder(encoding)),
      ctx_(boost::make_shared<mapnik::context_type>()) 
{ 
    fData = fopen("/home/rbuchhol/Desktop/MapnikPlugin/node", "rb");

    // the featureset context needs to know the field schema
    ctx_->push("key" ); // let us pretend it just has one column/attribute name "key"

}

hello_featureset::~hello_featureset() { }

mapnik::feature_ptr hello_featureset::next()
{

    if (!fData)
    //if (feature_id_ > 1)
        return mapnik::feature_ptr();
        
    int ch = fgetc(fData);
    if (ch == EOF) //read the whole file -> clean up
    {
        fclose(fData);
        return mapnik::feature_ptr();
    }
    ungetc(ch, fData);
    
    //if (feature_id_ > 1)
    //    return mapnik::feature_ptr();
    
    OsmLightweightWay way(fData);
    assert(way.numVertices > 0);
    //cout << way << endl;
    // create a new feature
    mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx_,feature_id_));

    feature->put( "key" ,tr_->transcode("hello world5!") );

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

