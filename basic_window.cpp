#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <thread>

using namespace std::chrono_literals;

template <typename T, typename Deleter> class managed_resource {
public:
  template <typename R, typename Del>
  managed_resource(R &&res, Del &&del)
      : resource{std::forward<R>(res)}, del{std::forward<Del>(del)} {}

  managed_resource(managed_resource &&) = default;
  managed_resource(const managed_resource &) = delete;

  T &get() { return resource; }
  const T &get() const { return resource; }

  ~managed_resource() { del(resource); }

private:
  T resource;
  Deleter del;
};

float remap(float val, float in1, float in2, float out1, float out2) {
  return out1 + (val - in1) * (out2 - out1) / (in2 - in1);
}

void draw_triangle() {
  const GLfloat z = 0;
  glBegin(GL_TRIANGLES);
  glVertex3f(0.0f, 0.0f, z);
  glVertex3f(-0.25f, -0.5f, z);
  glVertex3f(0.25f, -0.5f, z);
  glEnd();
}

int main() {
  // Open the display
  const auto display = std::unique_ptr<Display, int (*)(Display *)>(
      XOpenDisplay(std::getenv("DISPLAY")), XCloseDisplay);

  if (!display) {
    std::cout << "Could not open display\n";
    return 1;
  }
  const auto screenId = DefaultScreen(display.get());

  // GLX, create XVisualInfo, this is the minimum visuals we want
  const auto visual = [&] {
    GLint glxAttribs[] = {GLX_RGBA, GLX_DOUBLEBUFFER, None};

    return std::unique_ptr<XVisualInfo, int (*)(void *)>(
        glXChooseVisual(display.get(), screenId, glxAttribs), XFree);
  }();

  if (!visual) {
    std::cout << "Could not create correct visual window.\n";
    return 1;
  }

  // Open the window
  const auto window = [&] {
    XSetWindowAttributes windowAttribs{};
    windowAttribs.colormap =
        XCreateColormap(display.get(), RootWindow(display.get(), screenId),
                        visual->visual, AllocNone);

    windowAttribs.event_mask =
        ExposureMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask;

    const auto w =
        XCreateWindow(display.get(), RootWindow(display.get(), screenId), 0, 0,
                      320, 200, 0, visual->depth, InputOutput, visual->visual,
                      CWColormap | CWEventMask, &windowAttribs);

    auto deleter = [&, colormap = windowAttribs.colormap](Window win) {
      XFreeColormap(display.get(), colormap);
      XDestroyWindow(display.get(), win);
    };
    return managed_resource<Window, decltype(deleter)>(w, deleter);
  }();

  auto wmDeleteMessage = XInternAtom(display.get(), "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display.get(), window.get(), &wmDeleteMessage, 1);

  const auto context = [&] {
    auto deleter = [&](GLXContext ctx) {
      glXDestroyContext(display.get(), ctx);
    };
    return managed_resource<GLXContext, decltype(deleter)>(
        glXCreateContext(display.get(), visual.get(), NULL, GL_TRUE), deleter);
  }();

  glXMakeCurrent(display.get(), window.get(), context.get());

  // Show the window
  XClearWindow(display.get(), window.get());
  XMapRaised(display.get(), window.get());

  glClearColor(37.f / 255.f, 133.f / 255.f, 75.f / 255.f, 1.0f); // #25854b

  auto previous = std::chrono::system_clock::now();

  bool buttonPressed = false;
  while (true) {
    const auto start = std::chrono::system_clock::now();

    XEvent ev;
    XNextEvent(display.get(), &ev);

    XWindowAttributes attribs;
    XGetWindowAttributes(display.get(), window.get(), &attribs);

    // causes memory leak
    // https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=890846
    glClear(GL_COLOR_BUFFER_BIT);

    switch (ev.type) {
    case Expose: {
      glViewport(0, 0, attribs.width, attribs.height);
      break;
    };
    case ButtonPress: {
      buttonPressed = true;
      // [[fallthrough]];
    };
    case MotionNotify: {
      if (start - previous < (1000ms / 60)) {
        continue;
      }

      previous = start;

      if (buttonPressed) {
        const auto x = ev.xmotion.x;
        const auto y = ev.xmotion.y;
        glLoadIdentity();
        glTranslatef(remap(x, 0, attribs.width, -1, 1),
                     -remap(y, 0, attribs.height, -1, 1), -1);
        draw_triangle();
      }
      break;
    };
    case ButtonRelease: {
      buttonPressed = false;
      break;
    };
    case ClientMessage: {
      if (static_cast<Atom>(ev.xclient.data.l[0]) == wmDeleteMessage) {
        glXMakeCurrent(display.get(), None, NULL);
        return 0;
      }
      break;
    };
    default:
      break;
    }

    glXSwapBuffers(display.get(), window.get());
  }

  glXMakeCurrent(display.get(), None, NULL);
  return 1;
}
