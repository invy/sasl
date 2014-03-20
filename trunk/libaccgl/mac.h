#include "ogl.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <SOIL.h>
#include "glheaders.h"


// graphics context
struct OglCanvas
{
    struct SaslGraphicsCallbacks callbacks;
    saslgl_bind_texture_2d_callback binderCallback;
    saslgl_gen_tex_name_callback genTexNameCallback;

    int triangles;
    int lines;
    int textures;
    int texturesSize;
};



/// initialize graphics before frame start
static void drawBegin(struct SaslGraphicsCallbacks *canvas)
{
    OglCanvas *c = (OglCanvas*)canvas;
    assert(canvas);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    c->triangles = 0;
    c->lines = 0;
}


/// flush drawed graphics to screen
static void drawEnd(struct SaslGraphicsCallbacks *canvas)
{
/*    OglCanvas *c = (OglCanvas*)canvas;
    assert(canvas);

    printf("textures: %i %i triangles: %i lines: %i\n", 
            c->textures, c->texturesSize / 1024, c->triangles, c->lines);*/
}


/// load texture to memory.
/// Returns texture ID or -1 on failure.  On success returns texture width
//  and height in pixels
static int loadTexture(struct SaslGraphicsCallbacks *canvas,
        const char *buffer, int length, int *width, int *height)
{
    OglCanvas *c = (OglCanvas*)canvas;
    if (! c)
        return -1;

    GLuint texId = 0;
    if (c->genTexNameCallback)
        texId = c->genTexNameCallback();

    unsigned id = SOIL_load_OGL_texture_from_memory(
            (const unsigned char*)buffer, length, 
            0, texId, SOIL_FLAG_POWER_OF_TWO);
    if (! id) 
        return -1;
 
    texId = id;

    // because of SOIL issue
    if (c->binderCallback)
        c->binderCallback(id);
    else
        glBindTexture(GL_TEXTURE_2D, id);

    if (width) {
        GLint w;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
        *width = w;
    }
    if (height) {
        GLint h;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
        *height = h;
    }

    c->textures++;

    if (width && height)
        c->texturesSize += (*width) * (*height);

    return texId;
}


// Unload texture from video memory.
static void freeTexture(struct SaslGraphicsCallbacks *canvas, int textureId)
{
    GLuint id = (GLuint)textureId;
    glDeleteTextures(1, &id);
}


// draw line of specified color.
static void drawLine(struct SaslGraphicsCallbacks *canvas, double x1,
        double y1, double x2, double y2, double r, double g, double b, double a)
{
    OglCanvas *c = (OglCanvas*)canvas;
    assert(canvas);

    c->lines++;

    glDisable(GL_TEXTURE_2D);

    glColor4f(r, g, b, a);
    glBegin(GL_LINES);
      glVertex2f(x1, y1);
      glVertex2f(x2, y2);
    glEnd();
    
    glEnable(GL_TEXTURE_2D);
}


// draw untextured triangle.
static void drawTriangle(struct SaslGraphicsCallbacks *canvas, 
        double x1, double y1, double r1, double g1, double b1, double a1,
        double x2, double y2, double r2, double g2, double b2, double a2,
        double x3, double y3, double r3, double g3, double b3, double a3)
{
    OglCanvas *c = (OglCanvas*)canvas;
    assert(canvas);

    c->triangles++;

    glDisable(GL_TEXTURE_2D);

    glBegin(GL_TRIANGLES);
        glColor4d(r1, g1, b1, a1); glVertex2d(x1, y1);
        glColor4d(r2, g2, b2, a2); glVertex2d(x2, y2);
        glColor4d(r3, g3, b3, a3); glVertex2d(x3, y3);
    glEnd();
    
    glEnable(GL_TEXTURE_2D);
}


// draw textured triangle.
static void drawTexturedTriangle(struct SaslGraphicsCallbacks *canvas, 
        int textureId,
        double x1, double y1, double u1, double v1, double r1, double g1, double b1, double a1,
        double x2, double y2, double u2, double v2, double r2, double g2, double b2, double a2,
        double x3, double y3, double u3, double v3, double r3, double g3, double b3, double a3)
{
    OglCanvas *c = (OglCanvas*)canvas;
    if (! c)
        return;
    
    c->triangles++;

    if (c->binderCallback)
        c->binderCallback(textureId);
    else
        glBindTexture(GL_TEXTURE_2D, textureId);

    glBegin(GL_TRIANGLES);
        glTexCoord2d(u1, v1); glColor4d(r1, g1, b1, a1); glVertex2d(x1, y1);
        glTexCoord2d(u2, v2); glColor4d(r2, g2, b2, a2); glVertex2d(x2, y2);
        glTexCoord2d(u3, v3); glColor4d(r3, g3, b3, a3); glVertex2d(x3, y3);
    glEnd();
}


// enable clipping to rectangle
static void setClipArea(struct SaslGraphicsCallbacks *canvas, 
        double x1, double y1, double x2, double y2)
{
}


// disable clipping.
static void resetClipArea(struct SaslGraphicsCallbacks *canvas)
{
}


// push affine translation state
static void pushTransform(struct SaslGraphicsCallbacks *canvas)
{
    glPushMatrix();
}

// pop affine transform state
static void popTransform(struct SaslGraphicsCallbacks *canvas)
{
    glPopMatrix();
}


// apply move transform to current state
static void translateTransform(struct SaslGraphicsCallbacks *canvas, 
        double x, double y)
{
    glTranslated(x, y, 0);
}


// apply scale transform to current state
static void scaleTransform(struct SaslGraphicsCallbacks *canvas, 
        double x, double y)
{
    glScaled(x, y, 1.0f);
}

// apply rotate transform to current state
static void rotateTransform(struct SaslGraphicsCallbacks *canvas, 
        double angle)
{
    glRotated(angle, 0, 0, -1.0);
}


// initializa canvas structure
struct SaslGraphicsCallbacks* saslgl_init_graphics()
{
    OglCanvas *canvas = new OglCanvas;

    memset(canvas, 0, sizeof(OglCanvas));

    canvas->callbacks.draw_begin = drawBegin;
    canvas->callbacks.draw_end = drawEnd;
    canvas->callbacks.load_texture = loadTexture;
    canvas->callbacks.free_texture = freeTexture;
    canvas->callbacks.draw_line = drawLine;
    canvas->callbacks.draw_triangle = drawTriangle;
    canvas->callbacks.draw_textured_triangle = drawTexturedTriangle;
    canvas->callbacks.set_clip_area = setClipArea;
    canvas->callbacks.reset_clip_area = resetClipArea;
    canvas->callbacks.push_transform = pushTransform;
    canvas->callbacks.pop_transform = popTransform;
    canvas->callbacks.translate_transform = translateTransform;
    canvas->callbacks.scale_transform = scaleTransform;
    canvas->callbacks.rotate_transform = rotateTransform;

    return (struct SaslGraphicsCallbacks*)canvas;
}


// free canvas structure
void saslgl_done_graphics(struct SaslGraphicsCallbacks *canvas)
{
    OglCanvas *c = (OglCanvas*)canvas;
    if (c)
        delete c;
}

void saslgl_set_texture2d_binder_callback(struct SaslGraphicsCallbacks *canvas, 
        saslgl_bind_texture_2d_callback binder)
{
    OglCanvas *c = (OglCanvas*)canvas;
    if (! c)
        return;
    c->binderCallback = binder;
}

/// Setup texture name generator function.
/// \param canvas graphics canvas.
/// \param generator ID generator. if NULL default OpenGL function will be used
void saslgl_set_gen_tex_name_callback(struct SaslGraphicsCallbacks *canvas, 
        saslgl_gen_tex_name_callback generator)
{
    OglCanvas *c = (OglCanvas*)canvas;
    if (! c)
        return;
    c->genTexNameCallback = generator;
}


