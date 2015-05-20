// file plugin
#include "coordsDataSource.h"
#include "coordsFeatureSet.h"

// boost
#include <boost/make_shared.hpp>

#include <iostream>

using std::cout;
using std::endl;
using std::string;

using mapnik::datasource;
using mapnik::parameters;

DATASOURCE_PLUGIN(coords_datasource)

coords_datasource::coords_datasource(parameters const& params): 
    datasource(params),
    desc_( *params.get<std::string>("type"), *params.get<std::string>("encoding","utf-8")),
    extent_(), path_( *params.get<std::string>("path", "") )
{
    string geoTypeStr = *params.get<string>("geometryType", "line");
    if (geoTypeStr == "polygon") geometryType = POLYGON;
    else if (geoTypeStr == "point") assert(false && "support for point geometries not implemented");
    else geometryType = LINE;

    this->init(params);
}

void coords_datasource::init(mapnik::parameters const& params)
{
    cout << "initializing plugin '" << name() << "' with parameters: " << endl;
    for (auto x : params)
    {
        cout << "\t" << x.first << " : " <<*params.get<std::string>(x.first) << endl;
        //int val = x.second;
        
    }
    
    static double HALF_EARTH_CIRCUMFERENCE = 20037508.34;
    
    // don't know the actual datasource geographic extent, so set it to cover the whole world
    extent_ = mapnik::box2d<double>(-HALF_EARTH_CIRCUMFERENCE, -HALF_EARTH_CIRCUMFERENCE,
                                     HALF_EARTH_CIRCUMFERENCE,  HALF_EARTH_CIRCUMFERENCE);
}

coords_datasource::~coords_datasource() { }

// This name must match the plugin filename
const char * coords_datasource::name()
{
    return "coords";
}

mapnik::datasource::datasource_t coords_datasource::type() const
{
    return datasource::Vector;
}

mapnik::box2d<double> coords_datasource::envelope() const
{
    return extent_;
}

boost::optional<mapnik::datasource::geometry_t> coords_datasource::get_geometry_type() const
{
    return mapnik::datasource::Collection;
}

mapnik::layer_descriptor coords_datasource::get_descriptor() const
{
    return desc_;
}

mapnik::featureset_ptr coords_datasource::features(mapnik::query const& q) const
{
    const mapnik::box2d<double> &bbox = q.get_bbox();
    cout << "processing query in bounds " 
         << "lat:" << bbox.miny() << " -> " << bbox.maxy() << ", "
         << "lng:" << bbox.minx() << " -> " << bbox.maxx() << endl;
         
    cout << "\tfilter factor: " << q.get_filter_factor() << endl;
    cout << "\tscale denominator: " << q.scale_denominator() << endl;
    
    //double d1 = 
    cout << "\tresolution type: " << q.resolution().get<0>()
         << " / " << q.resolution().get<1>() << endl;
     
         
    std::set<string> propertyNames;
    for (const std::string s: q.property_names())
    {
        cout << "\texpecting property '" << s << "'" << endl;
        propertyNames.insert(s);
    }
    // if the query box intersects our world extent then query for features
    if (extent_.intersects(q.get_bbox()))
    {
        return boost::make_shared<coords_featureset>(q.get_bbox(),desc_.get_encoding(), path_, propertyNames);
    }

    // otherwise return an empty featureset pointer
    return mapnik::featureset_ptr();
}

mapnik::featureset_ptr coords_datasource::features_at_point(mapnik::coord2d const&, double) const
{
    // features_at_point is rarely used - only by custom applications,
    // so for this sample plugin let's do nothing...
    return mapnik::featureset_ptr();
}
