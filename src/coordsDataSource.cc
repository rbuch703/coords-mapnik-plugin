// file plugin
#include "coordsDataSource.h"
#include "coordsFeatureSet.h"

// boost
#include <boost/make_shared.hpp>

#include <iostream>

#include <unistd.h> //for stat()
#include <sys/stat.h> //for struct stat

using std::cout;
using std::endl;
using std::string;

using mapnik::datasource;
using mapnik::parameters;

static const int MAX_ZOOM_LEVEL = 24;

/* number are taken from http://wiki.openstreetmap.org/wiki/MinScaleDenominator,
 * but were given rounded to integers at that page, and thus are sometimes above and
 * sometimes below the actual scaleDenominator for the given zoom level. Adding +2
 * to each entry ensures that the value in this array is always consistently bigger
 * than the scaleDenominator used by Mapnik for the given zoom level
  */
static const uint64_t zoomLevelScaleDenominators[] = { 
    559082264+2,
    279541132+2,
    139770566+2,
    69885283+2,
    34942642+2,
    17471321+2,
    8735660+2,
    4367830+2,
    2183915+2,
    1091958+2,
    545979+2,
    272989+2,
    136495+2,
    68247+2,
    34124+2,
    17062+2,
    8531+2,
    4265+2,
    2133+2,
    1066+2,
    533+2
};

static const uint64_t numZoomLevelScaleDenominators =
    sizeof(zoomLevelScaleDenominators) / sizeof(uint64_t);

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

static int getZoomLevel (double scaleDenominator)
{
    uint64_t zoom = 0;
    while (zoomLevelScaleDenominators[zoom+1] > scaleDenominator && 
           zoom+1 < numZoomLevelScaleDenominators)
        zoom++;
        
    return zoom;
}

static std::string getClosestTileSet(std::string path, double scaleDenominator)
{
    int zoomLevel = getZoomLevel(scaleDenominator);
    cout << "\tzoom level is: " << zoomLevel << endl;
    int dataZoomLevel = MAX_ZOOM_LEVEL;
    std::string zoomLevelFileName = path;
    for (int i = dataZoomLevel; i >= zoomLevel; i--)
    {
        char num[4];
        #ifndef NDEBUG
        int res = 
        #endif
            snprintf(num, 4, "%d", i);
        assert(res < 4 && "overflow");
        std::string fileName = path + num +"_";
        //cout << "testing " << fileName;
        struct stat st;
        ;
        if ( (stat(fileName.c_str(), &st) == 0) &&
             S_ISREG(st.st_mode))  //exists and is a regular file
        {
            //cout << "*";
            dataZoomLevel = i;
            zoomLevelFileName = fileName;
        }
        //cout << endl;
    }
    
    cout << "optimal data zoom level is " << dataZoomLevel << endl;
    return zoomLevelFileName;
}

mapnik::featureset_ptr coords_datasource::features(mapnik::query const& q) const
{
    const mapnik::box2d<double> &bbox = q.get_bbox();
    cout << "processing query in bounds "
         << "x:" << bbox.minx() << " -> " << bbox.maxx() << ", "
         << "y:" << bbox.miny() << " -> " << bbox.maxy() << endl;
         
    cout << "\tfilter factor: " << q.get_filter_factor() << endl;
    
    
    cout.precision(17);
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
        return boost::make_shared<coords_featureset>(q.get_bbox(),desc_.get_encoding(),
            getClosestTileSet(path_, q.scale_denominator()), propertyNames);
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
