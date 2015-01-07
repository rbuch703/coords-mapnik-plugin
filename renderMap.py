#! /usr/bin/python2

import mapnik

m = mapnik.Map(2048, 2048)
m.srs = "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +wktext  +no_defs"
m.background = mapnik.Color('steelblue')

# continents
s = mapnik.Style()
r = mapnik.Rule()
r.symbols.append(mapnik.PolygonSymbolizer(mapnik.Color('#f2eff9')))
r.symbols.append(mapnik.LineSymbolizer(mapnik.Color('rgb(50%,50%,50%)'),0.1))
s.rules.append(r)
m.append_style('My Style',s)

ds = mapnik.Shapefile(file='/home/rbuchhol/Desktop/MapnikPlugin/ne_10m_admin_0_countries.shp')
layer = mapnik.Layer('world')
layer.datasource = ds
layer.styles.append('My Style')
m.layers.append(layer)



# big roads
s = mapnik.Style()
r = mapnik.Rule()
r.symbols.append(mapnik.LineSymbolizer(mapnik.Color('rgb(100%,50%,50%)'),0.5))
r.min_scale = 136495; # < zoom level 12

#r.max_scale = 34942642; #=zoom level 4
s.rules.append(r)

m.append_style('BigRoadStyle',s)
ds = mapnik.Datasource(type='hello', path="/home/rbuchhol/Desktop/MapnikPlugin/data/primary_roads.bin")
layer = mapnik.Layer('l2')
layer.datasource = ds
layer.styles.append('BigRoadStyle')
m.layers.append(layer)


# all roads
s = mapnik.Style()
r = mapnik.Rule()
r.symbols.append(mapnik.LineSymbolizer(mapnik.Color('rgb(75%,50%,50%)'), 1))
r.max_scale = 136495; #=zoom level 12
s.rules.append(r)

m.append_style('RoadStyle',s)
ds = mapnik.Datasource(type='hello', path="/home/rbuchhol/Desktop/MapnikPlugin/data/road")
layer = mapnik.Layer('l4')
layer.datasource = ds
layer.styles.append('RoadStyle')
m.layers.append(layer)



# boundaries
s = mapnik.Style()
r = mapnik.Rule()
r.symbols.append(mapnik.LineSymbolizer(mapnik.Color('rgb(10%,10%,10%)'),0.5))
#r.max_scale = 34942642; #=zoom level 4
s.rules.append(r)

m.append_style('BoundaryStyle',s)
ds = mapnik.Datasource(type='hello', path="/home/rbuchhol/Desktop/MapnikPlugin/data/admin.bin")
layer = mapnik.Layer('l3')
layer.datasource = ds
layer.styles.append('BoundaryStyle')
m.layers.append(layer)




mapnik.save_map(m, "map.xml");

m.zoom_to_box(mapnik.Box2d(-20037508.34,-20037508.34,20037508.34,20037508.34))

#m.zoom_all()
mapnik.render_to_file(m,'world.png', 'png')
print "rendered image to 'world.png'"
