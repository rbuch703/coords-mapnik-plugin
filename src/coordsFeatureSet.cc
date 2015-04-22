// mapnik
#include <mapnik/feature_factory.hpp>
#include <mapnik/geometry.hpp>

// boost
#include <boost/make_shared.hpp>

#include "coordsFeatureSet.h"

#include <iostream>
#include <unistd.h> //for stat()
#include <sys/stat.h>

using std::cout;
using std::endl;
using std::pair;
using std::string;

typedef mapnik::box2d<double> Box2D;

coords_featureset::coords_featureset(/*GEOMETRY_TYPE geoType,*/ Box2D const& box, std::string const& encoding, std::string path, std::set<std::string> propertyNames)
    : box_(box),
      feature_id_(1),
      tr_(new mapnik::transcoder(encoding)),
      ctx_(boost::make_shared<mapnik::context_type>()),
      fData(NULL)/*, geometryType(geoType)*/, propertyNames(propertyNames)
{

    buildFileHierarchy(path, files, box, mapnik::box2d<double>(-180, -90, 180, 90));
    
    for (std::string &s: files)
        cout << "\t registered file '" << s << "' for parsing" << endl;

//    fData = fopen(path.c_str(), "rb");

    // the featureset context needs to know the field schema
    for (const std::string &s: propertyNames)
        ctx_->push(s); // register attributes
}

coords_featureset::~coords_featureset() { }

void coords_featureset::buildFileHierarchy(std::string path, std::vector<std::string> &files, 
    const mapnik::box2d<double> &queryBounds, Box2D tileBounds)
{
    //cout << "tile bounds for " << path << " are " << tileBounds << queryBounds.intersect(tileBounds).valid() << endl;
    //if current tile is outside the query bounds
    if (! queryBounds.intersect(tileBounds).valid() )
        return;
        
    struct stat buf;
    int res = stat(path.c_str(), &buf);
    if (res != 0) return;
    
    if (! S_ISREG(buf.st_mode)) return;
    
    /* If the file is empty, it does not contain relevant data, but it may have child
     * nodes that do. So do not add it to the list of readable files, but still recurse
     * to its child nodes. */
    if ( buf.st_size > 0)
        files.push_back(path);

    double latMin = tileBounds.miny();
    double latMax = tileBounds.maxy();
    double latMid = (latMin + latMax) / 2.0;
    
    double lngMin = tileBounds.minx();
    double lngMax = tileBounds.maxx();
    double lngMid = (lngMin + lngMax) / 2.0;        

    buildFileHierarchy( path + "0", files, queryBounds, Box2D(lngMin, latMid, lngMid, latMax));
    buildFileHierarchy( path + "1", files, queryBounds, Box2D(lngMid, latMid, lngMax, latMax));
    buildFileHierarchy( path + "2", files, queryBounds, Box2D(lngMin, latMin, lngMid, latMid));
    buildFileHierarchy( path + "3", files, queryBounds, Box2D(lngMid, latMin, lngMax, latMid));
}

bool coords_featureset::wasReturnedBefore(OSM_ENTITY_TYPE entityType, uint64_t entityId) const
{

    switch (entityType)
    {
        case OSM_ENTITY_TYPE::NODE: 
            return nodesReturned.count(entityId);
        
        case OSM_ENTITY_TYPE::WAY:  
            return entityId >= waysReturned.size() ? false : waysReturned[entityId];
            
        case OSM_ENTITY_TYPE::RELATION: 
            return entityId >= relationsReturned.size() ? false : relationsReturned[entityId];

        default: 
            assert(false && "invalid entity type");
            return false;
    }
    

}

void coords_featureset::markAsReturnedBefore(OSM_ENTITY_TYPE entityType, uint64_t entityId)
{
    switch (entityType)
    {
        case OSM_ENTITY_TYPE::NODE: 
            nodesReturned.insert(entityId);
            return;
        
        case OSM_ENTITY_TYPE::WAY:  
            if (entityId >= waysReturned.size())
            {
                uint64_t oldSize = waysReturned.size();
                uint64_t newSize = (entityId + 1) * 11 / 10;
                waysReturned.resize( newSize);
                for (uint64_t i = oldSize; i < newSize; i++)
                    waysReturned[i] = false;        
            }
            
            waysReturned[entityId] = true;
            return;
            
        case OSM_ENTITY_TYPE::RELATION: 
            if (entityId >= relationsReturned.size())
            {
                uint64_t oldSize = relationsReturned.size();
                uint64_t newSize = (entityId + 1) * 11 / 10;
                relationsReturned.resize( newSize);
                for (uint64_t i = oldSize; i < newSize; i++)
                    relationsReturned[i] = false;        
            }
            
            relationsReturned[entityId] = true;
            return;

        default: 
            assert(false && "invalid entity type");
            return;
    }

}

boost::optional<GenericGeometry> coords_featureset::getNextGeometry()
{
//    cout << "has " << files.size() << " files." << endl;    
    while ( files.size() > 0 || fData != NULL)
    {
//        cout << "way " << feature_id_ << endl;
        if (fData == NULL)
        {
            cout << "opening next file '" << files.back() << "#" << endl;
            fData = fopen( files.back().c_str(), "rb");
            files.pop_back();
        }

        int ch;

        while ( (ch = fgetc(fData)) != EOF)
        {
            ungetc(ch, fData);
            GenericGeometry geom(fData);
            
            //cout << way.id << endl;
            if ( !wasReturnedBefore( geom.getEntityType(), geom.getEntityId() ))
            {
                markAsReturnedBefore( geom.getEntityType(), geom.getEntityId());
                return geom;
            }
        }

        //already read the whole file -> clean up
        fclose(fData);
        fData = NULL;
    }
    
    return boost::none;
}

mapnik::feature_ptr coords_featureset::next()
{    
    boost::optional<GenericGeometry> hasGeom = getNextGeometry();
    if (!hasGeom)
        return mapnik::feature_ptr();
        
    GenericGeometry &geom = *hasGeom;

    //assert(way.numVertices > 0);
    mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx_,feature_id_++));

    for (Tag &kv : geom.getTags())
    {
//        cout << "processing " << kv.first << " = " << kv.second << endl;
        if (propertyNames.count(kv.first))
        {
//            cout << "adding " << kv.first << " = " << kv.second << endl; 
            feature->put( kv.first ,tr_->transcode(kv.second.c_str()) );
        }
//              feature->put( kv.first, kv.second.c_str() );
             
        //cout << kv.first << " -> " << kv.second << endl;
    }
    
    // create a new feature

    /*
    mapnik::geometry_type * line;
    if (this->geometryType == LINE)
        line = new mapnik::geometry_type(mapnik::LineString);
    else if (this->geometryType == POLYGON)
        line = new mapnik::geometry_type(mapnik::Polygon);
    else 
        assert(false && "Invalid geometry type");
        
    for ( pair<string, string> kv : way.getTags())
    {
        if (propertyNames.count(kv.first))
            feature->put( kv.first, tr_->transcode(kv.second.c_str()));
    }*/
    
    #warning code to parse and pass geometry is missing
    /*
    double lat = way.vertices[0].lat / 10000000.0;
    double lng = way.vertices[0].lng / 10000000.0;
    line->move_to( lng, lat);
    for (int i = 1; i < way.numVertices; i++)
    {
        double lat = way.vertices[i].lat / 10000000.0;
        double lng = way.vertices[i].lng / 10000000.0;
        line->line_to( lng, lat);
    }
        
    feature->add_geometry(line);
    return feature;*/

    // otherwise return an empty feature
    return mapnik::feature_ptr();
}

