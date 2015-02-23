This software is licensed under the GNU Lesser General Public License version 2.1.

This plugin allows Mapnik to use COORDS (Chunk-Oriented OSM Render-Data Storage) as a data source for rendering. It does not require any additional services or daemons. Instead, it directly accesses the data files created by COORDS (cf. [http://github.com/rbuch703/coords](http://github.com/rbuch703/coords)).

This plugin is compatible with the Mapnik version `2.3.0-dev` shipped by MapBox from their [TileMill repository]([https://launchpad.net/~developmentseed/+archive/ubuntu/mapbox) for Ubuntu "Trusty" (14.04). Compatibility with other Mapnik `2.x` versions has not been tested. This plugin is **not** currently compatible with Mapnik `3.x`.
 
Prerequisites
=============
Compilation and usage of this plugin requires the following packages:

* libicu-dev 
* libmapnik
* libmapnik-dev 
* build-essential
* python2 (should be pre-installed on all major distributions)

On Ubuntu/Debian, these can be installed using `sudo apt-get install libicu-dev libmapnik-dev libmapnik build-essential`.

Additionally, this plugin requires an initialized COORDS data storage as created by the [COORDS tool](http://github.com/rbuch703/coords).

Compilation and Installation
============================

* `make` (there is no `./configure` step)
* `sudo make deploy`


Testing and Configuration 
=========================

* Download the [Natural Earth 10M Countries Shapefile](http://www.naturalearthdata.com/http//www.naturalearthdata.com/download/10m/cultural/ne_10m_admin_0_countries.zip), and decompress the contents into a directory of your choosing. One of the zip file's contents should be a file called `ne_10m_admin_0_countries.shp`. Make note of the full path of where you extracted that file to. 
* open the `testMapnik.py` file in a text editor, locate the line containing the string `ds = mapnik.Shapefile(`, and adjust the path follwing that string to match the location where you extracted the shapefile to.
* execute `./testMapnik.py`. This should create an image file `world.png` containing only the world's continents. If this file exists, your Mapnik installation and your configuration of the the shapefile location is correct.
* open the file `renderMap.py` in a text editor and make the same modifications to the Shapefile path that you did for  `testMapnik.py`.
* in `renderMap.py`, locate all lines containing the string `ds = mapnik.Datasource(type='coords'`, and in each line, adjust the path to match the location and base name where the COORDS tools created the initialized data storage.
* execute `./renderMap.py`. This should output an image file `world.png` containing the world's continents (from the Shapefile data source) along with all worldwide major roads and admin boundaries (from the COORDS data source). If this file with this content exists, your Mapnik, your COORDS storage and your map style configuration are correct.

If all those steps succeeded, you succesfully set up Mapnik to use the COORDS data source. In this case, you can go on and update your tile server configuration to use the COORDS data source. The successful execution of `./renderMap.py` should also have created a file called `coordsTestStyle.xml`. This is a Mapnik style file that can be used in any OSM render server toolchain.

Render Server Setup (UNTESTED!)
===============================
Once you successfully set up Mapnik to use your COORDS data source, you can use the generated `coordsTestStyle.xml` file to set up an OSM render server that does not require a Postgres database backend.

**Warning**: This guide has not been tested.

**Warning 2**: This guide assumes that your operating system is Ubuntu 14.04 (Trusty)

**Warning 3**: These instructions will disable any site configuration you have made to your local Apache web server. 

Outline
-------

A standard OSM tile render server consists of the following components:

* an Apache 2.x web server to receive and relay the user requests for tiles
* the Apache module `mod_tile` to handle these requests by either serving them from its tile cache, or relaying the request to a render daemon.
* the render daemon `renderd` that performs the actual tile rendering using Mapnik as its backend.
* Some Mapnik data sources containing the raw data the renderd/Mapnik creates tiles from. The usual types of data sources are ESRI Shapefiles, and a Postgres/PostGIS database using the `osm2pgsql` schema. In this guide, we will use Shapefiles as well (those were already setup through the `Testing and Configuration` step above), but will substitute the Postgres data source by a COORDS data source that does not require a database server (this has also already been been configured in the `Testing and Configuration` step above.

Steps
-----

1. install the repository configuration tool: `sudo apt-get install software-properties-common`
2. configure the necessary repository: `sudo add-apt-repository ppa:rbuch703//openstreetmap`
3. update your local package list: `sudo apt-get update`
4. install the necessary packages: `sudo apt-get install libapache2-mod-tile`
5. open the `/etc/renderd.conf` file in a text editor. Find the line `XML=/etc/mapnik-osm-carto-data/osm.xml` and replace the path following `XML=` by the path to your coordsTestStyle.xml
6. restart `renderd`: `sudo service renderd restart`
7. Open the following URL in a web browser `http://localhost/osm_tiles/0/0/0.png`. This should be an image containing all continents along with all major roads and administrative boundaries. (All other tile names following the [slippy map tile name convention](http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames) should also show corresponding map sections.

If this image exists and shows the correct content, you successfully set up an OSM render server using a COORDS data source. You can now use popular web toolkits such as Leaflet or OpenLayers to create a web map view that allows you to arbitrarily zoom and pan the map view rendered on your tile server.


