# Direct3D 12 in C-Style C++



A minimal Direct3D 12 example that draws an animated triangle, written
entirely in C-style C++, and all taking place inside a single
function.



## Fruity Frustration

Once upon a time (~10 days ago), I wanted to draw a lonely triangle
using the mysterious Direct3D 12.  I looked in the wild for a clear
example to follow but couldn't find any. I yearned for one that would
provide a head start and get out of the way immediately, but instead I
was met with all manner of verbose and cryptic messes. Each one
demanded a pursuit of its own right, which is distined to be littered
with dread and meaningless struggle, just to understand what it does;
and, should I refuse, it threatened to take away all my precious
enthusiasm and hope to one day witness my very own three-sided
creation upon the screen...  Long story short, this program is the
fruit of that very frustration!



## What Is Different?

The program draws an animated triangle on the screen as expected, with
the bare minimum amout of work it could get away with.  All this work
is expressed in clear, C-style C++; no OOP ornamentation or modern C++
spaghetti.  In addition to that, the entire program is simply a set of
steps laid out in a natural linear fashion, which all take place
inside the WinMain() function.

![screenshot](https://github.com/tellela/d3d12/blob/master/hello.png)

[You can also see it in motion.](https://youtu.be/nCEFEBWzfzo)



## The Code Layout

The program is made up of two files.

* hello.cxx -- sets up Direct3D 12 and uses it to feed the GPU with
three vertices and a contrived texture.

* shaders.hlsl -- holds the shaders which receive that data and
manipulate it to produce the final result.

As long as both files are located in the same directory, all you have
to do to build it is feed the C++ file to the compiler.  Alternatively
you can run the accompanied build script to avoid extraneous typing.



## Examples by Other People

This program is primarily inspired by the work of @mmozeiko.  His
[Direct3D 11 in C](https://gist.github.com/mmozeiko/5e727f845db182d468a34d524508ad5f)
example is the first program I come across that is written in this
linear fashion, all inside a single function.

Other examples:

* [Direct3D 11 in C++](https://gist.github.com/d7samurai/aee35fd5d132c51e8b0a78699cbaa1e4) by @d7samurai.

* [Direct3D 12 in C](https://github.com/rdunnington/d3d12-hello-triangle) by @rdunnington.



## Official Resources

[Direct3D 12 Programming Guide](https://docs.microsoft.com/en-us/windows/win32/direct3d12/directx-12-programming-guide)

[DirectX Graphics Samples](https://github.com/microsoft/DirectX-Graphics-Samples)

[Microsoft DirectX 12 and Graphics Education](https://www.youtube.com/c/MicrosoftDirectX12andGraphicsEducation)