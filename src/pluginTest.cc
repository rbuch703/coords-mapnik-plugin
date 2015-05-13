
#include "coordsFeatureSet.h"
#include "coordsDataSource.h"

int main()
{
    std::set<std::string> properties{"admin_level", "building", "highway"};
    coords_featureset featureSet( 
        mapnik::box2d<double>(-20037508.34,-20037508.34,20037508.34,20037508.34), 
        "utf-8", 
        "/home/rbuchhol/Desktop/coords-mapnik-plugin/data/line", properties);
    
    mapnik::feature_ptr feature;
    while ( (feature = featureSet.next()) != NULL)
    {
        //feature.reset();
    }

    
}
