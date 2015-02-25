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
    cout << "initializing plugin '" << name() << "'" << endl;
    for (auto x : params)
    {
        cout << x.first << " : " <<*params.get<std::string>(x.first) << endl;
        //int val = x.second;
        
    }
    // every datasource must have some way of reporting its extent
    // in this case we are not actually reading from any data so for fun
    // let's just create a world extent in Mapnik's default srs:
    // '+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs' (equivalent to +init=epsg:4326)
    // see http://spatialreference.org/ref/epsg/4326/ for more details
    extent_ = mapnik::box2d<double>(-180,-90,180,90);
}

coords_datasource::~coords_datasource() { }

// This name must match the plugin filename, eg 'hello.input'
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
         
    std::set<string> propertyNames;
    for (const std::string s: q.property_names())
    {
        cout << "\texpecting property '" << s << "'" << endl;
        propertyNames.insert(s);
    }
    // if the query box intersects our world extent then query for features
    if (extent_.intersects(q.get_bbox()))
    {
        return boost::make_shared<coords_featureset>((coords_featureset::GEOMETRY_TYPE)geometryType, q.get_bbox(),desc_.get_encoding(), path_, propertyNames);
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
