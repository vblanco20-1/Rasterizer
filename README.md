# Rasterizer
Fully multithreaded C++ CPU rasterizer

Using libraries: 
* [Concurrent Queue](https://github.com/cameron314/concurrentqueue). Used for triangle binning.
* [Fiber Tasking Lib](https://github.com/RichieSams/FiberTaskingLib). Used as the multithreading engine.
* [Remotery](https://github.com/Celtoys/Remotery). Used for profiling.
* [GLM](https://glm.g-truc.net/0.9.9/index.html). Vector/matrix math
* [SDL](https://www.libsdl.org/). Window display.

Very basic WIP rasterizer. The goal of this project is to learn high performance multithreading and SIMD practises.
The current version can rasterize the 870.000 triangles stanford dragon in a 1024x1024 window at 20 +-1 miliseconds per frame.

It works by launching a parallel for on the triangle array, wich runs a "vertex shader" to update the matrices. This also bins the triangles into screenspace tiles.
After that, another parallel for is launched, on the tiles, to perform thread safe tile based rasterization without data races.
