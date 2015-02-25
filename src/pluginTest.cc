
#include "coordsFeatureSet.h"
#include "coordsDataSource.h"

int main()
{
    std::set<std::string> properties{"admin_level", "building"};
    coords_featureset featureSet( coords_featureset::LINE,
        mapnik::box2d<double>(-20037508.34,-20037508.34,20037508.34,20037508.34), 
        "utf-8", 
        "/home/rbuchhol/Desktop/MapnikPlugin/data/admin.bin", properties);
    
    mapnik::feature_ptr feature;
    while ( (feature = featureSet.next()) != NULL)
    {
        //feature.reset();
    }

    
}
