#include <stdio.h>
#include <string>

#ifdef WINDOWS
#include <Winsock2.h>
#endif

#ifdef APL
#include <SDLMain.h>
#else
#include <SDL.h>
#include <SDL_video.h>
#ifndef USE_GLES1
#include <SDL_opengl.h>
#else
#include <SDL_opengles.h>
#endif
#include <SDL_events.h>
#endif

#include "libavionics.h"
#include "ogl.h"
#include "cmdline.h"
#include "fps.h"
#include "alsound.h"


using namespace slava;


// application window
static SDL_Window *window = NULL;

// OpenGL context
static SDL_GLContext glContext = NULL;


static void initSDL()
{
    if (-1 == SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_EVENTS)) {
        fprintf(stderr, "Can't initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }
    atexit(SDL_Quit);
}


static void updateScreenSettings(int width, int height)
{
    glViewport(0, 0, width, height);
    glClearColor(0, 0, 0, 0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
#ifdef USE_GLES1
    glOrthof(0, width, 0, height, -1, 1);
#else
    glOrtho(0, width, 0, height, -1, 1);
#endif
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


static void initScreen(int &width, int &height, bool fullscreen, 
        const std::string &caption)
{
    int flags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    if (fullscreen)
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

#ifdef USE_GLES1
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#endif
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    window = SDL_CreateWindow(caption.c_str(), SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, width, height, flags);
    if (! window) {
        fprintf(stderr, "Unable to set video: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_GetWindowSize(window, &width, &height);

    glContext = SDL_GL_CreateContext(window);
    if (! glContext) {
        fprintf(stderr, "Unable to create OpenGL context: %s\n", 
                SDL_GetError());
        exit(1);
    }

    updateScreenSettings(width, height);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_COLOR_MATERIAL);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
}


static SASL createPanel(SaslGraphicsCallbacks* graphics, int width, int height, 
        const std::string &data, const std::string &panel, 
        const std::string &host, int port, const std::string &secret,
        std::vector<std::string> &paths, SaslAlSound* &sound)
{
    SASL sasl = sasl_init(data.c_str(), NULL, NULL);
    if (! sasl) {
        fprintf(stderr, "Unable to initialize avionics library\n");
        exit(1);
    }
        
    sasl_set_graphics_callbacks(sasl, graphics);
    sound = sasl_init_al_sound(sasl);
    
    sasl_set_panel_size(sasl, width, height);
    sasl_set_popup_size(sasl, width, height);
    sasl_enable_click_emulator(sasl, true);
    sasl_set_background_color(sasl, 1, 1, 1, 1);

    for (std::vector<std::string>::iterator i = paths.begin(); 
            i != paths.end(); i++) 
        sasl_add_search_path(sasl, (*i).c_str());

    if (host.size())
        if (sasl_connect_to_server(sasl, host.c_str(), port, secret.c_str())) {
            fprintf(stderr, "Can't connect to server %s %i\n", host.c_str(), port);
            exit(1);
        }

    if (sasl_load_panel(sasl, panel.c_str())) {
        fprintf(stderr, "Can't load panel\n");
        exit(1);
    }

    return sasl;
}


int main(int argc, char *argv[])
{
#ifdef WINDOWS
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(2, 0);
    int err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        /* Tell the user that we could not find a usable */
        /* Winsock DLL.                                  */
        printf("WSAStartup failed with error: %d\n", err);
        return 1;
    }
#endif

    bool showClickable = false;

    CmdLine cmdLine(argc, argv);

    initSDL();

    int width = cmdLine.getScreenWidth();
    int height = cmdLine.getScreenHeight();

    std::string title = "SLAVA - " + cmdLine.getPanel();
    initScreen(width, height, cmdLine.isFullscreen(), title);

    if (cmdLine.isHideMouse())
        SDL_ShowCursor(0);

    SaslGraphicsCallbacks* graphics = saslgl_init_graphics();
    SaslAlSound *sound = NULL;

    SASL sasl = createPanel(graphics, width, height, 
            cmdLine.getDataDir(), cmdLine.getPanel(), 
            cmdLine.getNetHost(), cmdLine.getNetPort(),
            cmdLine.getNetSecret(), cmdLine.getPaths(), sound);

    Fps fps(cmdLine.isShowFps());
    fps.setTargetFps(cmdLine.getTargetFps());

    bool done = false;
    while (! done) {
        if (sasl_update(sasl))
            break;

        glClear(GL_COLOR_BUFFER_BIT);
        if (sasl_draw_panel(sasl, STAGE_ALL))
            break;
        SDL_GL_SwapWindow(window);

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT: 
                    done = true; 
                    break;

                case SDL_MOUSEBUTTONDOWN: 
                    sasl_mouse_button_down(sasl, event.button.x, 
                            height - event.button.y, event.button.button, 3);
                    break;

                case SDL_MOUSEBUTTONUP: 
                    sasl_mouse_button_up(sasl, event.button.x, 
                            height - event.button.y, event.button.button, 3);
                    break;

                case SDL_MOUSEMOTION: 
                    sasl_mouse_move(sasl, event.button.x, 
                            height - event.button.y, 3);
                    break;

                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_F7:
                            showClickable = ! showClickable;
                            sasl_set_show_clickable(sasl, showClickable);
                            break;

                        case SDLK_F8:
                            if (sound)
                                sasl_done_al_sound(sound);
                            sasl_done(sasl);
                            sasl = createPanel(graphics, width, height, 
                                    cmdLine.getDataDir(), cmdLine.getPanel(), 
                                    cmdLine.getNetHost(), cmdLine.getNetPort(),
                                    cmdLine.getNetSecret(), cmdLine.getPaths(),
                                    sound);
                            showClickable = false;
                            break;
                        
                        case SDLK_ESCAPE:
                            done = true;
                            break;

                        default: break;
                    };
                    break;

                case SDL_WINDOWEVENT: 
                    {
                        int newWidth, newHeight;
                        SDL_GetWindowSize(window, &newWidth, &newHeight);
                        if ((newWidth != width) || (newHeight != height)) {
                            width = newWidth;
                            height = newHeight;
                            updateScreenSettings(width, height);
                            sasl_set_panel_size(sasl, width, height);
                            sasl_set_popup_size(sasl, width, height);
                        }
                    }
                    break;
            }
        }

        fps.update();
    }

    if (sound)
        sasl_done_al_sound(sound);

    sasl_done(sasl);
    saslgl_done_graphics(graphics);

    if (glContext)
        SDL_GL_DeleteContext(glContext);
    if (window)
        SDL_DestroyWindow(window);

    return 0;
}

