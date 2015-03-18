# TODO #
## Research ##
#### Rasterization ####
  * Research cutting edge methods of rasterization.
    * Tile-based Deferred Rendering, [AndrewLauritzen2010](http://visual-computing.intel-research.net/art/publications/deferred_rendering/)
    * Decoupled Deferred Rendering, [GaborLiktor2012](http://hgpu.org/?p=6820)
#### Raytracing ####
  * Research cutting edge methods of raytracing.
  * Evaluate cutting edge raytracing tools/engines.
    * [OptiX](http://www.nvidia.com/object/optix.html)
#### GPU Architecture ####
  * Research GPU programming. What programs map well to GPU's? Explore the costs of memory-access (Arrays of Structs vs Structs of Arrays layout, linked lists vs arrays, program flow ( branching ) and loops.

## Paper ##
## Programming ##
#### Framework ####
  * Set up basic deferred rasterization render pipeline. Done.
  * Use file watcher to reload edited data files at runtime.
  * Set up scene and engine using data files for easy parameter tweaking.
  * Rewrite render pipeline into something flexible enough to give mix-and-match support to rasterization and raytracing techniques.
  * Integrate a good performance/profiling tool/logger.
#### Rasterization ####
  * Vanilla deferred shading integration based on [Coding Labs](http://www.codinglabs.net/tutorial_simple_def_rendering.aspx)'s article.
#### Raytracing ####

Evaluate "forward vs deferred collab with raytracer". Try variations on render order. When does it make sense to run raster first? Ray first?

For example:
1. render forward to texture.
2. raytrace, compose ray image with forward image.