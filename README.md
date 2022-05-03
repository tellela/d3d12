# Direct3D 12 in C-Style C++



A minimal Direct3D 12 example that draws a triangle on the screen, written
entirely in C-style C++, and all taking place inside a single function.



## A Frustrated Whim

On a whim one day, I decided to draw a triangle on the screen using Direct3D
12.  So, as is customary, the first thing I did was to look in the wild for
some simple and clear example to follow, in the hope to minimize my exposure
to the dull language of manuals.  But to my surprise I couldn't find any. All
I wanted was one that would serve as a quick leg up and get out of the way
immediately, but instead I was met with a variety of gluttonously verbose and
cryptic messes.  Each one of them demanded significant effort just to make
sense of its demoralizing code, much less discern which parts of it were
responsible for drawing the triangle and distinguish from them those that did
something else.  I was frustrated as a result, but despite that I somehow
remained determined to bring the three-sided figure into existence.  Thus I
was left with no choice but to endure the manuals and write the example
myself.



## What Is Different?

This program draws a triangle on the screen as expected.  It also textures
this triangle and animates it, to make the drawing a little more pleasant to
look at.  It achieves this with the bare minimum work it could get away with,
which is expressed in clear C-style C++.  No Object-Oriented Programming
ornamentation nor modern C++ spaghetti is involved.  In addition to that, the
entire program is simply a set of steps laid out in their natural linear
fashion, and they all take place inside a single function, namely `WinMain()`.

[See it in video.](https://youtu.be/nCEFEBWzfzo)

![screenshot](https://github.com/tellela/d3d12/blob/master/hello.png)



## The Code Layout

The program is made up of two files.

* `hello.cpp` sets up Direct3D 12 and uses it to feed the GPU with the
  necessary (albeit contrived) data.

* `shaders.hlsl` holds the shaders which receive that data and manipulate it
  to produce the final result.

As long as both files are located in the same directory, all you have to do to
build it is either to run the accompanied build script, or to feed the C++
file directly to the compiler yourself.



## Similar Examples

This example is primarily inspired by the work of @mmozeiko.  His
[Direct3D 11 in C](https://gist.github.com/mmozeiko/5e727f845db182d468a34d524508ad5f)
example is the first program I came across that is written in this format.

Additional examples:

* [Direct3D 11 in C++](https://gist.github.com/d7samurai/aee35fd5d132c51e8b0a78699cbaa1e4) by @d7samurai.

* [Direct3D 12 in C](https://github.com/rdunnington/d3d12-hello-triangle) by @rdunnington.



## Official Resources

[Direct3D 12 Programming Guide](https://docs.microsoft.com/en-us/windows/win32/direct3d12/directx-12-programming-guide)

[DirectX Graphics Samples](https://github.com/microsoft/DirectX-Graphics-Samples)

[Microsoft DirectX 12 and Graphics Education](https://www.youtube.com/c/MicrosoftDirectX12andGraphicsEducation)