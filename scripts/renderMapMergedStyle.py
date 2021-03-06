#! /usr/bin/python2
import mapnik

coordsTilePath = '/home/rbuchhol/Desktop/coords/nodes'
shapefilePath  = '/home/rbuchhol/Desktop/render/ne_10m_admin_0_countries_lakes.shp'


if not(coordsTilePath[-1] in ['/', '\\']):
    coordsTilePath += "/"

m = mapnik.Map(2048*2, 2048*2)
m.srs = "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +wktext  +no_defs"
#m.srs = "+proj=eqc"
m.background = mapnik.Color('steelblue')

# continents/countries
s = mapnik.Style()
r = mapnik.Rule()
r.symbols.append(mapnik.PolygonSymbolizer(mapnik.Color('#f2eff9')))
#r.symbols.append(mapnik.LineSymbolizer(mapnik.Color('rgb(50%,50%,50%)'),0.1))
s.rules.append(r)
m.append_style('Landcover Style',s)

ds = mapnik.Shapefile(file=shapefilePath)
layer = mapnik.Layer('world')
layer.datasource = ds
layer.styles.append('Landcover Style')
m.layers.append(layer)




dsCoords = mapnik.Datasource(type='coords', path= coordsTilePath + "node")

s = mapnik.Style()

r = mapnik.Rule()
r.symbols.append(mapnik.LineSymbolizer(mapnik.Color('rgb(50%,75%,50%)'), 1))
r.max_scale = 136495; #=zoom level 12
r.filter = mapnik.Filter("[highway]");
s.rules.append(r)

r = mapnik.Rule()
r.filter = mapnik.Filter("[railway]");
r.symbols.append(mapnik.LineSymbolizer(mapnik.Color('rgb(50%,50%,80%)'),0.5))
r.max_scale = 136495; # zoom level 12
s.rules.append(r)

r = mapnik.Rule()
r.filter = mapnik.Filter("[building]");
r.symbols.append(mapnik.PolygonSymbolizer(mapnik.Color('rgb(75%,50%,50%)')))
r.max_scale = 68247; #=zoom level 13
s.rules.append(r)

r = mapnik.Rule()
r.max_scale = 136495; #=zoom level 12
r.filter = mapnik.Filter("[admin_level] = '6'");
ls = mapnik.LineSymbolizer(mapnik.Color('rgb(20%,20%,20%)'), 2);
ls.stroke.opacity = 0.1;
r.symbols.append(ls)
s.rules.append(r)


r = mapnik.Rule()
r.max_scale = 136495; #=zoom level 12
r.filter = mapnik.Filter("[admin_level] = '4'");
ls = mapnik.LineSymbolizer(mapnik.Color('rgb(10%,10%,10%)'), 1);
ls.stroke.opacity = 1;
r.symbols.append(ls)
s.rules.append(r)

r = mapnik.Rule()
r.max_scale = 136495; #=zoom level 12
r.filter = mapnik.Filter("[admin_level] = '2'");
ls = mapnik.LineSymbolizer(mapnik.Color('rgb(20%,20%,20%)'), 1);
ls.stroke.opacity = 0.3;
r.symbols.append(ls)
#r.max_scale = 34942642; #=zoom level 4
s.rules.append(r)

m.append_style('CoordsStyle',s)
layer = mapnik.Layer('CoordsLayer')
layer.datasource = dsCoords
layer.styles.append('CoordsStyle')
m.layers.append(layer)

# ================ high zoom layers

dsCoords = mapnik.Datasource(type='coords', path=coordsTilePath +"lod12")

# big roads
s = mapnik.Style()
r = mapnik.Rule()
r.filter = mapnik.Filter("[highway] = 'primary' or [highway] = 'trunk' or [highway] = 'motorway'");
r.symbols.append(mapnik.LineSymbolizer(mapnik.Color('rgb(50%,80%,50%)'),0.5))
r.min_scale = 136495; # =< zoom level 12
r.max_scale = 34942642; #=zoom level 4
s.rules.append(r)

r = mapnik.Rule()
r.filter = mapnik.Filter("[railway] = 'rail'");
r.symbols.append(mapnik.LineSymbolizer(mapnik.Color('rgb(50%,50%,80%)'),0.5))
r.min_scale = 136495; # =< zoom level 12
r.max_scale = 34942642; #=zoom level 4
s.rules.append(r)


r = mapnik.Rule()
r.max_scale = 1091958 ; #=zoom level 9
r.min_scale = 136495; # < zoom level 12
r.filter = mapnik.Filter("[admin_level] = '6'");
ls = mapnik.LineSymbolizer(mapnik.Color('rgb(20%,20%,20%)'), 2);
ls.stroke.opacity = 0.1;
r.symbols.append(ls)
s.rules.append(r)


r = mapnik.Rule()
r.max_scale = 8735660; #=zoom level 6
r.min_scale = 136495; # <= zoom level 12
r.filter = mapnik.Filter("[admin_level] = '4'");
ls = mapnik.LineSymbolizer(mapnik.Color('rgb(10%,10%,10%)'), 1);
ls.stroke.opacity = 1;
r.symbols.append(ls)
s.rules.append(r)

r = mapnik.Rule()
r.max_scale = 34942642; #=zoom level 4
r.min_scale = 136495; # < zoom level 12
r.filter = mapnik.Filter("[admin_level] = '2'");
ls = mapnik.LineSymbolizer(mapnik.Color('rgb(20%,20%,20%)'), 1);
ls.stroke.opacity = 0.3;
r.symbols.append(ls)
s.rules.append(r)

m.append_style('CoordsLod12Style',s)
layer = mapnik.Layer('CoordsLod12Layer')
layer.datasource = dsCoords
layer.styles.append('CoordsLod12Style')
m.layers.append(layer)
# ====================================================


mapnik.save_map(m, "coordsTestStyle.xml");

m.zoom_to_box(mapnik.Box2d(-20037508.34,-20037508.34,20037508.34,20037508.34))
#m.zoom_to_box( mapnik.Box2d(10, 48,13, 53) )
#m.zoom_to_box( mapnik.Box2d(12, 52,13, 53) )
#m.zoom_all()
mapnik.render_to_file(m, 'world_roads.png', 'png')
print "rendered image to 'world_roads.png'"
