#! /usr/bin/python2

import mapnik

m = mapnik.Map(2048, 2048)
m.srs = "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +wktext  +no_defs"
#m.srs = "+proj=eqc"
m.background = mapnik.Color('steelblue')

# continents/countries
s = mapnik.Style()
r = mapnik.Rule()
r.symbols.append(mapnik.PolygonSymbolizer(mapnik.Color('#f2eff9')))
r.symbols.append(mapnik.LineSymbolizer(mapnik.Color('rgb(50%,50%,50%)'),0.1))
s.rules.append(r)
m.append_style('My Style',s)

ds = mapnik.Shapefile(file='/home/rbuchhol/Desktop/render/ne_10m_admin_0_countries_lakes.shp')
layer = mapnik.Layer('world')
layer.datasource = ds
layer.styles.append('My Style')
m.layers.append(layer)


m.zoom_to_box(mapnik.Box2d(-20037508.34,-20037508.34,20037508.34,20037508.34))
mapnik.render_to_file(m, 'world.png', 'png')
print "rendered image to 'world.png'"
