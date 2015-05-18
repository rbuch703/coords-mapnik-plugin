#ifndef COORDS_FEATURESET_H
#define COORDS_FEATURESET_H

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

#include "genericGeometry.h"
//#include "osmMappedTypes.h"

// comparator functor
struct StringLess {
    bool operator()( const char* p1, const char* p2) const { return strcmp(p1, p2) < 0;};
};

class coords_featureset : public mapnik::Featureset
{
public:
    enum GEOMETRY_TYPE {POINT, LINE, POLYGON};

    // this constructor can have any arguments you need
    coords_featureset(mapnik::box2d<double> const& box, std::string const& encoding, std::string path, std::set<std::string> propertyNames);

    // desctructor
    virtual ~coords_featureset();

    // mandatory: you must expose a next() method, called when rendering
    mapnik::feature_ptr next();

    bool getNextGeometry( GenericGeometry &geoOut);

private:

    bool wasReturnedBefore(OSM_ENTITY_TYPE entityType, uint64_t entityId) const;
    void markAsReturnedBefore(OSM_ENTITY_TYPE entityType, uint64_t entityId);
    void buildFileHierarchy(std::string path, std::vector<std::string> &files,
                            const mapnik::box2d<double> &queryBounds, 
                            mapnik::box2d<double> tileBounds);

private:
//    static bool stringLess( const char* p1, const char* p2) { return strcmp(p1, p2) < 0;};
    mapnik::box2d<double> box_;
    mapnik::value_integer feature_id_;
    boost::scoped_ptr<mapnik::transcoder> tr_;
    mapnik::context_ptr ctx_;
    FILE* fData;

    std::set<uint64_t> nodesReturned;
    std::vector<bool> waysReturned;
    std::vector<bool> relationsReturned;
    
    GenericGeometry lastReturnedGeometry;
    
    std::vector<std::string> files;
    GEOMETRY_TYPE geometryType;
    std::set<const char*, StringLess> propertyNames;
    std::vector<char*> propertyNamesRaw;
};

#endif
