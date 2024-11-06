#include <SDL2/SDL.h>

int main(const int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Hello World", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GL_CreateContext(window);
    SDL_GL_SwapWindow(window);
    SDL_Delay(3000);
    SDL_Quit();
    return 0;
}