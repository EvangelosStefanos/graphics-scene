# graphics-scene
  A computer graphics scene made with openGL and GLUT.

## Details
  A house, a spotlight, a sun and a day/night cycle all together. What more could a man possibly want?

![Project Preview Gif](preview.gif)

## Compiling
  - Platform: Windows
  - Compiler: [MinGW v8.1.0](https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win64/Personal%20Builds/mingw-builds/8.1.0/threads-win32/seh/)
  - GLUT: [freeglut](http://freeglut.sourceforge.net/) 3.0.0-1.mp [for MinGW](https://www.transmissionzero.co.uk/software/freeglut-devel/)

  ```
  gcc -c -o obj/main.o src/main.c -D FREEGLUT_STATIC -I "freeglut/include"
  gcc -o bin/main.exe obj/main.o -L "freeglut/lib/x64" -lfreeglut_static -lopengl32 -lglu32 -lwinmm -lgdi32 -static -static-libgcc
  ```
  Optionally add `-mwindows` as a linker option to build it as a GUI application instead of the default console application.
  ```
  gcc -o bin/main.exe obj/main.o -L "freeglut/lib/x64" -lfreeglut_static -lopengl32 -lglu32 -lwinmm -lgdi32 -static -static-libgcc -mwindows
  ```
