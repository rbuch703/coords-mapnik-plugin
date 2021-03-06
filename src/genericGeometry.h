
#ifndef GENERIC_GEOMETRY_H
#define GENERIC_GEOMETRY_H

#ifndef COORDS_MAPNIK_PLUGIN
    #include "geom/envelope.h"
    #include "osm/osmBaseTypes.h"
    #include "misc/rawTags.h"
#else
    #include "envelope.h"
    #include "rawTags.h"
    enum struct OSM_ENTITY_TYPE : uint8_t { NODE, WAY, RELATION, CHANGESET, OTHER };
#endif

#include <iostream>
#include <string>
#include <vector>

enum struct FEATURE_TYPE: uint8_t {POINT = 0, LINE = 1, POLYGON = 2};

/* new flag byte (not yet in use) to replace the FEATURE_TYPE byte*/
enum struct GEOMETRY_FLAGS: uint8_t { 
    POINT = 0, LINE = 1, WAY_POLYGON = 2, RELATION_POLYGON = 3,  // bits 0-1: type
    IS_DUPLICATE = 4  // bit 2: whether the same geometry has been stored in another tile as well
};

std::ostream& operator<<(std::ostream& os, FEATURE_TYPE ft);
std::ostream& operator<<(std::ostream& os, OSM_ENTITY_TYPE et);

typedef std::pair<std::string, std::string> Tag;

class GenericGeometry {
public:
    GenericGeometry(FILE* f);
    GenericGeometry(const GenericGeometry &other);
    GenericGeometry(uint8_t *bytes, uint32_t numBytes, bool takeOwnership);

#ifdef COORDS_MAPNIK_PLUGIN
    GenericGeometry();
#endif

    ~GenericGeometry();
    
    void init(FILE *f, bool avoidRealloc);
    void serialize(FILE* f) const;
    
    FEATURE_TYPE getFeatureType() const;    //POINT/LINE/POLYGON
    OSM_ENTITY_TYPE getEntityType() const;  //NODE/WAY/RELATION
    GEOMETRY_FLAGS getGeometryFlags() const;    
    uint64_t getEntityId() const;
    int8_t getZIndex() const;
    Envelope getBounds() const;    
    //std::vector<Tag> getTags() const;
    RawTags getTags() const;
    const uint8_t* getGeometryPtr() const;
    //bool hasMultipleRings() const;

private:
    Envelope getLineBounds() const;
    Envelope getPolygonBounds() const;
public:
    uint32_t numBytes;
    uint32_t numBytesAllocated;
    uint8_t *bytes;
    
        
};

#endif

