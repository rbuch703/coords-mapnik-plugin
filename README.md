This software is licensed under the GNU Lesser General Public License version 2.1.

This plugin allows Mapnik to use COORDS (Chunk-Oriented OSM Render-Data Storage) as a data source for rendering. It does not require any additional services or daemons. Instead, it directly accesses the data files created by COORDS (cf. [http://github.com/rbuch703/coords](http://github.com/rbuch703/coords)).

This plugin is compatible with the Mapnik version `2.3.0-dev` shipped by MapBox from their [TileMill repository]([https://launchpad.net/~developmentseed/+archive/ubuntu/mapbox) for Ubuntu "Trusty" (14.04). Compatibility with other Mapnik `2.x` versions has not been tested. This plugin is **not** currently compatible with Mapnik `3.x`.
 
Prerequisites
=============
Compilation and usage of this plugin requires the following packages:

* libicu-dev 
* libmapnik-dev 
* build-essential
* cmake

On Ubuntu/Debian, these can be installed using `sudo apt-get install cmake libicu-dev libmapnik-dev build-essential`.

Compilation and Installation
============================
* `cmake .`
* `make`
* `sudo make install`

A guide of how to use this Mapnik plugin to render your own maps can be found on the [COORDS project page](http://rbuch703.github.io/coords).

