CUDA Path Tracer
================

**University of Pennsylvania, CIS 565: GPU Programming and Architecture, Project 3**

* Taylor Nelms
  * [LinkedIn](https://www.linkedin.com/in/taylor-k-7b2110191/), [twitter](https://twitter.com/nelms_taylor), etc.
* Tested on: Windows 10, Intel i3 Coffee Lake 4-core 3.6GHz processor, 16GB RAM, NVidia GeForce GTX1650 4GB

## Path Tracer

![Glass Zelda on a Textured Altar](progressImages/oidn_zelda_2000fil.png)

A Path Tracer is a method of rendering virtual geometry onto the screen. Notably, they do so by simulating how light moves around a scene. This is in contrast to traditional rendering methods, which transform the geometry more directly from world-space to screen-space. While path tracers are slower than traditional renderers, they are able to natively perform much more impressive feats overall.

![glass, steel, and mirror ball in a checkered hellscape](progressImages/checkers_unfiltered.png)

For example, features such as *caustics* (the more intense light on the floor at the bottom-left of the image), or complex reflections and re-reflections, are easier to get with path-tracing than other methods.

The overall operation is as such: for each pixel in an image, shoot a ray from our camera out, and see what it intersects. If it hit a light source, we're done; otherwise, use the material properties of what we hit to generate a "bounced" ray, and do the same with that one until we either didn't hit anything, or we hit a light source. Multiply the color effects of each of our materials together, and then we get the color that pixel should be. We do that thousands of times, relfecting in different ways each time, until our image becomes something sensible.

This particular implementation is running on the GPU (graphics processing unit) via the CUDA framework, which allows us to parallelize the rigorous task of doing all our calculations for each pixel one at a time. This allows for significant speed-ups over CPU path tracers.

## Features

### Object loading

For an Object file in the scene description, it may be given the type “mesh.” Their transformation parameters act the same way, but you may also specify a “FILE” string. This can be the path to an `.obj` file, or a `.gltf` file (the latter of which must be in the same place as its assets).

This loads all the file’s triangles into our data structures, and can then be rendered sensibly. As of now, no material characteristics are loaded in; however, textures are recognized for `.gltf` files.

#### Bounding Volume

Simple axis-aligned bounding box for each Shape/Primitive in the mesh file.

##### Performance

TODO: performance analysis

#### Textures

Using the CUDA texture memory, I was able to hook up `.gltf` files with their textures within the ray tracer. I worked almost entirely from files downloaded from [Sketchfab](https://sketchfab.com/), so their naming conventions may have ended up baked in to my implementation. (Specifically, the file naming for their texture image files is how I distinguish between different types of texture mappings.)

Notably, these assets provide a few different attributes. In addition to base color, some models have normal (bump) maps, textures for metallic roughness, or emissivity. An example of an asset displaying both base color texture and emissivity textures is here:

![Altar mesh with base color and emissivity implemented](progressImages/day7AltarTexture2.png)

Here is a series of images of the same scene, with differing level of textures applied to them.

<figure>
<img src="progressImages/altar0.png" alt="No Textures"
	title="No Textures" width="500" height="500" />
 <figcaption>No Textures</figcaption>
</figure>
 
<figure>
<img src="progressImages/altarC.png" alt="Color Texture"
	title="Color Texture" width="500" height="500" />
 <figcaption>Color Texture</figcaption>
</figure>

<figure>
<img src="progressImages/altarCE.png" alt="Color and Emissivity Textures"
	title="Color and Emissivity Textures" width="500" height="500" />
 <figcaption>Color and Emissivity Textures</figcaption>
</figure>

<figure>
<img src="progressImages/altarCEM.png" alt="Color, Emissivity, and Metallic Textures"
	title="Color, Emissivity, and Metallic Textures" width="500" height="500" />
 <figcaption>Color, Emissivity, and Metallic Textures</figcaption>
</figure>

<figure>
<img src="progressImages/altarCEMN.png" alt=Color, Emissivity, Metallic, and Normal Textures"
	title="Color, Emissivity, Metallic, and Normal Textures" width="500" height="500" />
 <figcaption>Color, Emissivity, Metallic, and Normal Textures</figcaption>
</figure>

##### Performance

TODO: Performance analysis

### Specular Sampling with Exponent

Implemented specular reflections with configurable exponent. Pictured below is a comparison of various exponential values for specularity. Notice that the very high value is effectively mirror-like; with such a highly specular object, the slight variations we get off the "mirror" direction are small enough to, effectively, not alter the ray at all. In this fashion, if we wished, we could eliminate the idea of "reflectivity" from our material description altogether.

![Shiny balls with their exponents noted](progressImages/day4ShinyBallAnnotated.png)

### Refraction

Refraction turned out to be trickier than I anticipated. Notably, it made triangle intersection tests more difficult, because I now had to check my meshes for backface triangles. (A smarter implementation than mine might only do so if the material for the mesh as a whole were refractive.) However, this led to the possibility for very interesting results.

TODO: put in a couple of glass images.

### Material Sorting

In order to attempt to reduce warp divergence, and better make use of the GPU resources, I implemented a pass to allow for material sorting between computing intersections and shading the materials.

For a simple scene, such as `scenes/checkersdemo.txt`, I was able to get 200 iterations deep in `20s` without material sorting. With sorting, it took `50s`.

Now, that was with only a dozen or so primitives; surely, when dealing with hundreds or thousands of triangles, the performance will be improved!

For a more complex scene, such as `scenes/teapotdemo.txt` (containing some 16,000 triangles), without sorting the materials, it took `75s` to get to 200 iterations; with sorting, it took `79s` to get to 200 iterations. Still not a performance boost, but better nonetheless.

When working with a significantly complex scene, such as `scenes/bunnydemo.txt` (containing 144,000 triangles), it took


## Configuration Notes

### CMakeLists changes

I put the `tinyobjloader` library contents into the `external` folder, so I had to include the relevant header and source file in the project, as well as mark their locations to be included and linked.

I added the [OpenImageDenoiser](https://github.com/OpenImageDenoise/oidn) library to the `external` folder, and so added the line `include_directories(external/oidn/include)` so that the headers could be read sensibly. Additionally, I added the subdirectory `external/oidn`, and linked the `OpenImageDenoise` library to the `target_link_libraries` function. This did not end up doing all the necessary linking (see below), but it helped.

Notably, this required having Intels `tbb` installed; I acheived this by signing up for, and subsequently installing, [Intel Parallel Studio](https://software.intel.com/en-us/parallel-studio-xe). Time will tell if I made the right decision.

Additionally, I decided to compile this all with `C++17`, in case I decided to make use of the `std::filesystem` library (a slight quality of life fix over just calling it via `std::experimental::filesystem`). I admittedly am not sure whether this change actually took.

#### Moving DLLs

Look, I don't like what I did either.

I manually copied the `OpenImageDenoise.dll` and `tbb.dll` from their rightful homes to the directory where my built `Release` executable was, so that it might run.

Certainly, CMake has a way to do this, but as somebody who is not a CMake wizard at this point, this will have to do.

## Sources

### 3D Models
* Models downloaded from Morgan McGuire's [Computer Graphics Archive](https://casual-effects.com/data)
    * Bunny, Dragon, Teapot, Tree, Fireplace Room
* Turbosquid
    * [Wine Glass](https://www.turbosquid.com/FullPreview/Index.cfm/ID/667624) by OmniStorm
    * [Secondary Wine Glass](https://www.turbosquid.com/FullPreview/Index.cfm/ID/932821) by Mig91
* Sketchfab
    * [Fountain](https://sketchfab.com/3d-models/fountain-07b16f0c118d4073a81522a526183c11) by Eugen Shuklin
    * [Altar](https://sketchfab.com/3d-models/altar-9b20f669e75441bcb34476255d248564) by William Chang
    * [Zelda](https://sketchfab.com/3d-models/ssbb-zelda-6612b024962b4141b1f867babe0f0e6c) by ThatOneGuyWhoDoesThings
    * [Sheik](https://sketchfab.com/3d-models/ssbb-sheik-4916d918d2c44f6bb984b59f082fc48c) by ThatOneGuyWhoDoesThings
    * [Hunter Rifle](https://sketchfab.com/3d-models/hunter-rifle-wip-ae83df4cc35c4eff89b34f266de9af3c) by cotman sam
    * [Textured Cube](https://sketchfab.com/3d-models/textured-cube-a883bf6dfd144419929067067c7f6dff) by Stakler

### Other Code
* Used [TinyObjLoader](https://github.com/syoyo/tinyobjloader) library for loading `*.obj` files
* Used [TinyGltf](https://github.com/syoyo/tinygltf) library for loading `*.gltf` files
    * I also lifted their `gltf_loader` files from their raytrace examples. I did not use any other code from the example folder.
* [OpenImageDenoiser](https://github.com/OpenImageDenoise/oidn) for post-processing
* Formerly: Ray-triangle intersection algorithm stolen from the Wikipedia article for the [Moller-Trumbore Intersection Algorithm](https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm). Now, using glm.
   
