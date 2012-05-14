README file for the Museum scene, part of
BART: Benchmark for Animated Ray Tracing
by Jonas Lext, Ulf Assarsson, Tomas Moller
May 5, 2000
================================================

We have provided 6 different complexity levels 
(levels of detail) of the museum. There is
a separate file for each of these:
museum[3-8].aff
(the higher number, the more primitives)
These are the actual files that should be fed
to your renderer.
Each file includes the same basic environment
(i.e., they include the file room.aff),
the pillar (files: pillar.aff and block.aff,
the paintings (paintings.aff), and they also
have the same camera animation (camera.aff).
The main difference is that they include
different animart[3-8].aff files, which
include the piece of "abstract animated
art".

Five texture files are used:
balls4.ppm
checkmarb1.ppm
gears4.ppm
mount6.ppm
tetra6.ppm

Facts about the museum files:

museum3.aff
===========
Polygons: 10143
Cones:    8
Anim Tris:64

museum4.aff
===========
Polygons: 10143
Cones:    8
Anim Tris:256

museum5.aff
===========
Polygons: 10143
Cones:    8
Anim Tris:1024

museum6.aff
===========
Polygons: 10143
Cones:    8
Anim Tris:4096

museum7.aff
===========
Polygons: 10143
Cones:    8
Anim Tris:16384

museum8.aff
===========
Polygons: 10143
Cones:    8
Anim Tris:65536
