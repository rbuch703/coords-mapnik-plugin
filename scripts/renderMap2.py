#! /usr/bin/python2

import mapnik

m = mapnik.Map(2048, 2048)
m.srs = "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +no_defs"
m.maximum_extent = mapnik.Box2d(-20037508.342800,-20037508.342800,
                                 20037508.342800, 20037508.342800);
#m.srs = "+proj=eqc"
m.background = mapnik.Color('steelblue')

# continents/countries
s = mapnik.Style()
r = mapnik.Rule()
r.symbols.append(mapnik.PolygonSymbolizer(mapnik.Color('#f2eff9')))
r.symbols.append(mapnik.LineSymbolizer(mapnik.Color('rgb(50%,50%,50%)'),0.1))
s.rules.append(r)
m.append_style('Landcover Style',s)

ds = mapnik.Shapefile(file='/home/rbuchhol/Desktop/coords-mapnik-plugin/shapefiles/ne_10m_admin_0_countries.shp')
layer = mapnik.Layer('world')
layer.datasource = ds
layer.styles.append('Landcover Style')
m.layers.append(layer)

dsCoordsAreas = mapnik.Datasource(type='coords', path="/home/rbuchhol/Desktop/coords-mapnik-plugin/data/area_")
dsCoordsLines = mapnik.Datasource(type='coords', path="/home/rbuchhol/Desktop/coords-mapnik-plugin/data/line_")

dsCoordsLanduse = mapnik.Datasource(type='coords', path="/home/rbuchhol/Desktop/coords-mapnik-plugin/data/landuse_")

dsCoordsBuilding = mapnik.Datasource(type='coords', path="/home/rbuchhol/Desktop/coords-mapnik-plugin/data/building_")

dsCoordsWater = mapnik.Datasource(type='coords', path="/home/rbuchhol/Desktop/coords-mapnik-plugin/data/water_")


def water():
    global m;
    s = mapnik.Style()
    r = mapnik.Rule()
    r.symbols.append(mapnik.PolygonSymbolizer(mapnik.Color('steelblue')))
    s.rules.append(r)

    m.append_style('WaterStyle',s)

    layer = mapnik.Layer('l132')
    layer.datasource = dsCoordsWater;
    #layer.datasource = dsCoordsLanduse;
    layer.srs = "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +no_defs"
    layer.styles.append('WaterStyle')
    m.layers.append(layer)


def bigRoads():
    global m;
# big roads

    s = mapnik.Style()
    r = mapnik.Rule()
    r.filter = mapnik.Filter("[highway] = 'primary' or [highway] = 'trunk' or [highway] = 'motorway'");
    r.symbols.append(mapnik.LineSymbolizer(mapnik.Color('rgb(0,0,0)'), 0.2))
#    r.symbols.append(mapnik.LineSymbolizer(mapnik.Color('rgb(80%,20%,20%)'), 0.5))
    #r.symbols.append(mapnik.LineSymbolizer(mapnik.Color('rgb(50%,100%,50%)'),0.5))
    r.min_scale = 136495; # < zoom level 12
    #r.max_scale = 34942642; #=zoom level 4
    s.rules.append(r)
    m.append_style('BigRoadStyle',s)

    layer = mapnik.Layer('l2')
    layer.datasource = dsCoordsLines;
    layer.srs = "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +no_defs"
    layer.styles.append('BigRoadStyle')
    m.layers.append(layer)


# all roads
def roads():
    global m;
    s = mapnik.Style()
    r = mapnik.Rule()
    r.symbols.append(mapnik.LineSymbolizer(mapnik.Color('rgb(0,0,0)'), 0.3))
    r.max_scale = 136495; #=zoom level 12
    r.filter = mapnik.Filter("[highway]");
    s.rules.append(r)

    m.append_style('RoadStyle',s)
    #ds = mapnik.Datasource(type='coords', path="/home/rbuchhol/Desktop/coords-mapnik-plugin/data/node")
    layer = mapnik.Layer('l4')
    layer.datasource = dsCoordsLines
    layer.srs = "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +no_defs"
    layer.styles.append('RoadStyle')
    m.layers.append(layer)

# all buildings
def buildings():
    global m;
    s = mapnik.Style()
    r = mapnik.Rule()
    r.symbols.append(mapnik.PolygonSymbolizer(mapnik.Color('rgb(65%,60%,60%)')))
    #r.filter = mapnik.Filter("[building]");
    #r.max_scale = 68247; #=zoom level 12
    s.rules.append(r)

    m.append_style('BuildingStyle',s)
    #ds = mapnik.Datasource(type='coords', path="/home/rbuchhol/Desktop/coords-mapnik-plugin/data/node", geometryType="polygon")
    layer = mapnik.Layer('l5')
    layer.datasource = dsCoordsBuilding
    layer.srs = "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +no_defs"
    layer.styles.append('BuildingStyle')
    m.layers.append(layer)

def landuse():
    global m;
    
    s = mapnik.Style()

    #r = mapnik.Rule()
    #r.symbols.append(mapnik.PolygonSymbolizer(mapnik.Color('rgb(100%,0%,0%)')))
    #r.symbols.append(mapnik.LineSymbolizer(mapnik.Color('rgb(0%,0%,0%)'), 0.1))
    #r.filter = mapnik.Filter("[landuse]");
    #s.rules.append(r)

    r = mapnik.Rule()
    r.symbols.append(mapnik.PolygonSymbolizer(mapnik.Color('rgb(100%,90%,90%)')))
    r.filter = mapnik.Filter("[landuse]='farmland' or [landuse]='farm' or [landuse]='farmyard' or [landuse]='orchard' or [landuse]='vineyard' or [landuse]='field'");
    s.rules.append(r)

    r = mapnik.Rule()
    r.symbols.append(mapnik.PolygonSymbolizer(mapnik.Color('rgb(144,207,144)')))
    r.filter = mapnik.Filter("[landuse]='forest' or [landuse]='conservation' or [natural]='wood'");
    s.rules.append(r)

    r = mapnik.Rule()
    r.symbols.append(mapnik.PolygonSymbolizer(mapnik.Color('rgb(195,249,195)')))
    r.filter = mapnik.Filter("[landuse]='grass' or [landuse]='village_green' or [landuse]='meadow' or [landuse]='common' or [landuse]='recreation_ground' or [natural]='scrub' or [natural]='grassland'");
    s.rules.append(r)

    #cities
    r = mapnik.Rule()
    r.symbols.append(mapnik.PolygonSymbolizer(mapnik.Color('rgb(80%,80%,80%)')))
    r.filter = mapnik.Filter("[landuse]='residential' or [landuse]='industrial' or [landuse]='commercial' or [landuse]='retail' or [landuse]='cemetery' or [landuse]='garages' or[landuse]='construction' or [landuse]='brownfield'");
    s.rules.append(r)

    #r = mapnik.Rule()
    #r.symbols.append(mapnik.PolygonSymbolizer(mapnik.Color('rgb(100%,70%,70%)')))
    #r.filter = mapnik.Filter("[landuse]='military'");
    #s.rules.append(r)

    #misc
    r = mapnik.Rule()
    r.symbols.append(mapnik.PolygonSymbolizer(mapnik.Color('yellow')))
    r.filter = mapnik.Filter("[landuse]='quarry' or [landuse]='greenfield' or [landuse]='railway' or [landuse]='landfill' or [landuse]='greenhouse_horticulture' or [landuse]='allotments'");
    s.rules.append(r)


    #r.max_scale = 68247; #=zoom level 12

    m.append_style('LanduseStyle',s)
    #ds = mapnik.Datasource(type='coords', path="/home/rbuchhol/Desktop/coords-mapnik-plugin/data/node", geometryType="polygon")
    layer = mapnik.Layer('l6')
    layer.datasource = dsCoordsLanduse
    layer.srs = "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +no_defs"
    layer.styles.append('LanduseStyle')
    m.layers.append(layer)



def admin():
    s = mapnik.Style()

    r = mapnik.Rule()
    #r.max_scale = 1091958 ; #=zoom level 9
    r.filter = mapnik.Filter("[boundary]='administrative' and [admin_level] = '6'");
    ls = mapnik.LineSymbolizer(mapnik.Color('rgba(60%,30%,30%, 0.3)'), 2);
    r.symbols.append(ls)
    s.rules.append(r)

    r = mapnik.Rule()
    #r.max_scale = 1091958 ; #=zoom level 9
    r.filter = mapnik.Filter("[boundary]='administrative' and [admin_level] = '5'");
    ls = mapnik.LineSymbolizer(mapnik.Color('rgba(60%,30%,30%, 0.4)'), 2);
    r.symbols.append(ls)
    s.rules.append(r)


    r = mapnik.Rule()
    #r.max_scale = 8735660; #=zoom level 6
    r.filter = mapnik.Filter("[boundary]='administrative' and [admin_level] = '4'");
    ls = mapnik.LineSymbolizer(mapnik.Color('rgba(60%,10%,10%, 0.5)'), 3);
    ls.stroke.opacity = 1;
    r.symbols.append(ls)
    s.rules.append(r)

    r = mapnik.Rule()
    #r.max_scale = 68247; #=zoom level 12
    r.filter = mapnik.Filter("[boundary]='administrative' and [admin_level] = '3'");
    ls = mapnik.LineSymbolizer(mapnik.Color('rgba(100%,10%,10%, 0.7)'), 4);
    #ls.stroke.opacity = 0.3;
    r.symbols.append(ls)
    #r.max_scale = 34942642; #=zoom level 4
    s.rules.append(r)


    r = mapnik.Rule()
    #r.max_scale = 68247; #=zoom level 12
    r.filter = mapnik.Filter("[boundary]='administrative' and [admin_level] = '2'");
    ls = mapnik.LineSymbolizer(mapnik.Color('rgba(100%,10%,10%, 0.8)'), 5);
    #ls.stroke.opacity = 0.3;
    r.symbols.append(ls)
    #r.max_scale = 34942642; #=zoom level 4
    s.rules.append(r)

    m.append_style('BoundaryStyle',s)
    #ds = mapnik.Datasource(type='coords', path="/home/rbuchhol/Desktop/coords-mapnik-plugin/data/node")
    layer = mapnik.Layer('l3')
    layer.datasource = dsCoordsLines
    layer.srs = "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +no_defs"
    layer.styles.append('BoundaryStyle')
    m.layers.append(layer)

   
landuse()
buildings()
#roads()
#bigRoads()
water()
#admin()

#def dummy():
# boundaries




mapnik.save_map(m, "coordsTestStyle.xml");

#m.zoom_to_box(mapnik.Box2d(-20037508.34,-20037508.34,20037508.34,20037508.34))

m.zoom_to_box(mapnik.Box2d(  -700000, 6400000,
                              200000, 7550000))

#m.zoom_all()
mapnik.render_to_file(m, 'world.png', 'png')
print "rendered image to 'world.png'"
