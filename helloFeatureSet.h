#ifndef HELLO_FEATURESET_HPP
#define HELLO_FEATURESET_HPP

// mapnik
#include <mapnik/datasource.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/unicode.hpp>

// boost
#include <boost/scoped_ptr.hpp> // needed for wrapping the transcoder
#include <boost/optional.hpp>

#include <stdio.h>
#include <set>
#include <string>
#include "osmMappedTypes.h"

class hello_featureset : public mapnik::Featureset
{
public:
    enum GEOMETRY_TYPE {POINT, LINE, POLYGON};

    // this constructor can have any arguments you need
    hello_featureset(GEOMETRY_TYPE geoType, mapnik::box2d<double> const& box, std::string const& encoding, std::string path, std::set<std::string> propertyNames);

    // desctructor
    virtual ~hello_featureset();

    // mandatory: you must expose a next() method, called when rendering
    mapnik::feature_ptr next();

    boost::optional<OsmLightweightWay> getNextWay();

private:

    bool wasReturnedBefore(uint64_t wayId);
    void markAsReturnedBefore(uint64_t wayId);
    void buildFileHierarchy(std::string path, std::vector<std::string> &files,
                            const mapnik::box2d<double> &queryBounds, 
                            mapnik::box2d<double> tileBounds);

private:
    mapnik::box2d<double> box_;
    mapnik::value_integer feature_id_;
    boost::scoped_ptr<mapnik::transcoder> tr_;
    mapnik::context_ptr ctx_;
    FILE* fData;
    std::vector<bool> waysReturned;
    std::vector<std::string> files;
    GEOMETRY_TYPE geometryType;
    std::set<std::string> propertyNames;
};

#endif // HELLO_FEATURESET_HPP
