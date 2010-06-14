#include "graphstub.h"

using namespace xa;


static void drawBegin(struct XaGraphicsCallbacks *canvas)
{
}


/// flush drawed graphics to screen
static void drawEnd(struct XaGraphicsCallbacks *canvas)
{
}


/// load texture to memory.
/// Returns texture ID or -1 on failure.  On success returns texture width
//  and height in pixels
static int loadTexture(struct XaGraphicsCallbacks *canvas,
        const char *name, int *width, int *height)
{
    return -1;
}


// Unload texture from video memory.
static void freeTexture(struct XaGraphicsCallbacks *canvas, int textureId)
{
}


// draw line of specified color.
static void drawLine(struct XaGraphicsCallbacks *canvas, double x1,
        double y1, double x2, double y2, double r, double g, double b, double a)
{
}


// draw untextured triangle.
static void drawTriangle(struct XaGraphicsCallbacks *canvas, 
        double x1, double y1, double r1, double g1, double b1, double a1,
        double x2, double y2, double r2, double g2, double b2, double a2,
        double x3, double y3, double r3, double g3, double b3, double a3)
{
}


// draw textured triangle.
static void drawTexturedTriangle(struct XaGraphicsCallbacks *canvas, 
        int textureId,
        double x1, double y1, double u1, double v1, double r1, double g1, double b1, double a1,
        double x2, double y2, double u2, double v2, double r2, double g2, double b2, double a2,
        double x3, double y3, double u3, double v3, double r3, double g3, double b3, double a3)
{
}


// enable clipping to rectangle
static void setClipArea(struct XaGraphicsCallbacks *canvas, 
        double x1, double y1, double x2, double y2)
{
}


// disable clipping.
static void resetClipArea(struct XaGraphicsCallbacks *canvas)
{
}


// push affine translation state
static void pushTransform(struct XaGraphicsCallbacks *canvas)
{
}

// pop affine transform state
static void popTransform(struct XaGraphicsCallbacks *canvas)
{
}


// apply move transform to current state
static void translateTransform(struct XaGraphicsCallbacks *canvas, 
        double x, double y)
{
}


// apply scale transform to current state
static void scaleTransform(struct XaGraphicsCallbacks *canvas, 
        double x, double y)
{
}

// apply rotate transform to current state
static void rotateTransform(struct XaGraphicsCallbacks *canvas, 
        double angle)
{
}

static struct XaGraphicsCallbacks callbacks = { drawBegin, drawEnd,
    loadTexture, freeTexture, drawLine, drawTriangle, drawTexturedTriangle,
    setClipArea, resetClipArea, pushTransform, popTransform, 
    translateTransform, scaleTransform, rotateTransform };


XaGraphicsCallbacks* xa::getGraphicsStub()
{
    return &callbacks;
}


