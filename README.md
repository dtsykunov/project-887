# project-887

Test task for JetBrains internship

## Dependencies

* Xorg 1.20.10
* GLX 1.4
* cmake 3.13.4+
* C++-17 compliant compiler

## Building locally

``` sh
$ git clone https://github.com/dtsykunov/project-887.git 

$ cd project-887

$ mkdir build

$ cd build

$ cmake -DCMAKE_BUILD_TYPE=Release .. && make

$ ./basic_window
```


## Running with docker

Build the docker image

``` sh

$ docker build -t project-887 .

```


(linux) With Xserver running:

``` sh

$ docker run --rm -e DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix:ro project-887

```

(macos) 

``` sh

$ brew install xquartz

$ xquartz & # Preferences -> Security -> Allow connections from network clients

$ xhost +

# Skip to here if you already know what you're doing

$ docker run -e DISPLAY=docker.for.mac.host.internal:0 project-887

```


## Sources

* https://github.com/gamedevtech/X11OpenGLWindow
* http://blog.mecheye.net/2012/06/the-linux-graphics-stack/
* https://tronche.com/gui/x/xlib-tutorial/
* https://www.khronos.org/opengl/wiki/Programming_OpenGL_in_Linux:_GLX_and_Xlib
* https://gameprogrammingpatterns.com/game-loop.html
