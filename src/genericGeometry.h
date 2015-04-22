
#ifndef GENERIC_GEOMETRY_H
#define GENERIC_GEOMETRY_H

#include "envelope.h"
#include <iostream>
#include <string>
#include <vector>
//#include "osm/osmBaseTypes.h"

enum struct FEATURE_TYPE: uint8_t {POINT, LINE, POLYGON};
enum struct OSM_ENTITY_TYPE : uint8_t { NODE, WAY, RELATION, CHANGESET, OTHER };

std::ostream& operator<<(std::ostream& os, FEATURE_TYPE ft);
std::ostream& operator<<(std::ostream& os, OSM_ENTITY_TYPE et);

typedef std::pair<std::string, std::string> Tag;

class GenericGeometry {
public:
    GenericGeometry(FILE* f);
    ~GenericGeometry();
    
    FEATURE_TYPE getFeatureType() const;    //POINT/LINE/POLYGON
    OSM_ENTITY_TYPE getEntityType() const;  //NODE/WAY/RELATION
    uint64_t getEntityId() const;
    Envelope getBounds() const;    
    std::vector<Tag> getTags() const;
private:
    Envelope getLineBounds() const;
    Envelope getPolygonBounds() const;
public:
    uint32_t numBytes;
    uint8_t *bytes;
    
        
};

#endif
