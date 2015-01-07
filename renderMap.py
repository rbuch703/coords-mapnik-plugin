#! /usr/bin/python2

import mapnik

m = mapnik.Map(1500, 2000)
m.srs = "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +wktext  +no_defs"
m.background = mapnik.Color('steelblue')

s = mapnik.Style()
r = mapnik.Rule()
polygon_symbolizer = mapnik.PolygonSymbolizer(mapnik.Color('#f2eff9'))
r.symbols.append(polygon_symbolizer)
line_symbolizer = mapnik.LineSymbolizer(mapnik.Color('rgb(50%,50%,50%)'),0.1)
r.symbols.append(line_symbolizer)

s.rules.append(r)
m.append_style('My Style',s)

ds = mapnik.Shapefile(file='/home/rbuchhol/Desktop/MapnikPlugin/ne_10m_admin_0_countries.shp')
layer = mapnik.Layer('world')
layer.datasource = ds
layer.styles.append('My Style')
m.layers.append(layer)




s = mapnik.Style()
r = mapnik.Rule()
line_symbolizer = mapnik.LineSymbolizer(mapnik.Color('rgb(100%,50%,50%)'),0.5)
r.symbols.append(line_symbolizer)
#r.symbols.append( mapnik.TextSymbolizer(mapnik.Expression('[key]'), 'DejaVu Sans Book', 10, mapnik.Color('black')) )
s.rules.append(r)
m.append_style('HelloStyle',s)

ds = mapnik.Datasource(type='hello')
layer = mapnik.Layer('l2')
layer.datasource = ds
layer.styles.append('HelloStyle')
m.layers.append(layer)

#mapnik.save_map(m, "map.xml");

m.zoom_to_box(mapnik.Box2d(0.0, 0, 10000000.0, 20000000.0))

#m.zoom_all()
mapnik.render_to_file(m,'world.png', 'png')
print "rendered image to 'world.png'"
