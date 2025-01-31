// Modified by:
// Date:
//
// Original author: Gordon Griesel
// Date:            2025
// Purpose:         OpenGL sample program
//
// To do list:
// 1/31/2025 add some text
//
#include <iostream>
using namespace std;
#include <stdio.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include <GL/glut.h> // Include GLUT for the font rendering
#include "fonts.h"

// Some structures
class Global {
public:
    int xres, yres;
    float w;
    float dir_x, dir_y;
    float pos[2];
    int bounce_count;
    Global();
} g;

class X11_wrapper {
private:
    Display *dpy;
    Window win;
    GLXContext glc;
public:
    ~X11_wrapper();
    X11_wrapper();
    void set_title();
    bool getXPending();
    XEvent getXNextEvent();
    void swapBuffers();
    void reshape_window(int width, int height);
    void check_resize(XEvent *e);
    void check_mouse(XEvent *e);
    int check_keys(XEvent *e);
} x11;

// Function prototypes
void init_opengl(void);
void physics(void);
void render(void);

int main(int argc, char **argv) {
    glutInit(&argc, argv); // Initialize GLUT
    init_opengl();
    int done = 0;
    // Main game loop
    while (!done) {
        // Look for external events such as keyboard, mouse.
        while (x11.getXPending()) {
            XEvent e = x11.getXNextEvent();
            x11.check_resize(&e);
            x11.check_mouse(&e);
            done = x11.check_keys(&e);
        }
        physics();
        render();
        x11.swapBuffers();
        usleep(200);
    }
    return 0;
}

Global::Global() {
    xres = 400;
    yres = 200;
    w = 20.0f;
    dir_x = 2.0f;
    dir_y = 1.5f;
    pos[0] = w;
    pos[1] = yres / 2.0f;
    bounce_count = 0;
}

X11_wrapper::~X11_wrapper() {
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
}

X11_wrapper::X11_wrapper() {
    GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    int w = g.xres, h = g.yres;
    dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
        cout << "\n\tcannot connect to X server\n" << endl;
        exit(EXIT_FAILURE);
    }
    Window root = DefaultRootWindow(dpy);
    XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
    if (vi == NULL) {
        cout << "\n\tno appropriate visual found\n" << endl;
        exit(EXIT_FAILURE);
    }
    Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
    XSetWindowAttributes swa;
    swa.colormap = cmap;
    swa.event_mask =
        ExposureMask | KeyPressMask | KeyReleaseMask |
        ButtonPress | ButtonReleaseMask |
        PointerMotionMask |
        StructureNotifyMask | SubstructureNotifyMask;
    win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
                        InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
    set_title();
    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, glc);
}

void X11_wrapper::set_title() {
    // Set the window title bar.
    XMapWindow(dpy, win);
    XStoreName(dpy, win, "3350 Lab2 - Esc to exit");
}

bool X11_wrapper::getXPending() {
    // See if there are pending events.
    return XPending(dpy);
}

XEvent X11_wrapper::getXNextEvent() {
    // Get a pending event.
    XEvent e;
    XNextEvent(dpy, &e);
    return e;
}

void X11_wrapper::swapBuffers() {
    glXSwapBuffers(dpy, win);
}

void X11_wrapper::reshape_window(int width, int height) {
    // Window has been resized.
    g.xres = width;
    g.yres = height;
    //
    glViewport(0, 0, (GLint)width, (GLint)height);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    glOrtho(0, g.xres, 0, g.yres, -1, 1);
}

void X11_wrapper::check_resize(XEvent *e) {
    // The ConfigureNotify is sent by the server if the window is resized.
    if (e->type != ConfigureNotify)
        return;
    XConfigureEvent xce = e->xconfigure;
    if (xce.width != g.xres || xce.height != g.yres) {
        // Window size did change.
        reshape_window(xce.width, xce.height);
    }
}

void X11_wrapper::check_mouse(XEvent *e) {
    static int savex = 0;
    static int savey = 0;
    // Weed out non-mouse events
    if (e->type != ButtonRelease &&
        e->type != ButtonPress &&
        e->type != MotionNotify) {
        // This is not a mouse event that we care about.
        return;
    }
    //
    if (e->type == ButtonRelease) {
        return;
    }
    if (e->type == ButtonPress) {
        if (e->xbutton.button==1) {
            // Left button was pressed.
            return;
        }
        if (e->xbutton.button==3) {
            // Right button was pressed.
            return;
        }
    }
    if (e->type == MotionNotify) {
        // The mouse moved!
        if (savex != e->xbutton.x || savey != e->xbutton.y) {
            savex = e->xbutton.x;
            savey = e->xbutton.y;
        }
    }
}

int X11_wrapper::check_keys(XEvent *e) {
    if (e->type != KeyPress && e->type != KeyRelease)
        return 0;
    int key = XLookupKeysym(&e->xkey, 0);
    if (e->type == KeyPress) {
        switch (key) {
            case XK_s:
                g.dir_x *= 0.8f;
                g.dir_y *= 0.8f;
                break;
            case XK_f:
                g.dir_x *= 1.2f;
                g.dir_y *= 1.2f;
                break;
            case XK_Escape:
                return 1;
        }
    }
    return 0;
}

void init_opengl(void) {
    // OpenGL initialization
    glViewport(0, 0, g.xres, g.yres);
    // Initialize matrices
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    // Set 2D mode (no perspective)
    glOrtho(0, g.xres, 0, g.yres, -1, 1);
    // Set the screen background color
    glClearColor(0.1, 0.1, 0.1, 1.0);
}

void physics() {
    // Update position
    g.pos[0] += g.dir_x;
    g.pos[1] += g.dir_y;
    // Collision detection
    if (g.pos[0] >= (g.xres - g.w)) {
        g.pos[0] = (g.xres - g.w);
        g.dir_x = -g.dir_x;
        g.bounce_count++;
    }
    if (g.pos[0] <= g.w) {
        g.pos[0] = g.w;
        g.dir_x = -g.dir_x;
        g.bounce_count++;
    }
    if (g.pos[1] >= (g.yres - g.w)) {
        g.pos[1] = (g.yres - g.w);
        g.dir_y = -g.dir_y;
    }
    if (g.pos[1] <= g.w) {
        g.pos[1] = g.w;
        g.dir_y = -g.dir_y;
    }
}

void render() {
    // Clear the window
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw the box
    glPushMatrix();
    glColor3ub(255, 0, 0);  // Red color for the box
    glTranslatef(g.pos[0], g.pos[1], 0.0);
    glBegin(GL_QUADS);
        glVertex2f(-g.w, -g.w);
        glVertex2f(-g.w,  g.w);
        glVertex2f( g.w,  g.w);
        glVertex2f( g.w, -g.w);
    glEnd();
    glPopMatrix();

    // Set up for text rendering
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, g.xres, 0, g.yres);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Render the text
    glColor3f(1.0, 1.0, 1.0); // White color for text
    glRasterPos2i(10, g.yres - 20); // Position at top left
    const char* text = "3350 Lab2 - A: Speed up, B: Slow down";
    while (*text) {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *text++);
    }

    // Restore the projection matrix
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

