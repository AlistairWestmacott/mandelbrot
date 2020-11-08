#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <complex.h>
#include <stdbool.h>

#define GREYSCALE 1

extern int opterr;

double resolution = 0.01;
int SCREEN_WIDTH = 1920;
int SCREEN_HEIGHT = 1080;
complex double center = 0;
int maxIterations = 127;
double cutoff = 2.0;

void sdlErrReport(char * msg) {
    fprintf(stderr, msg, SDL_GetError());
}

uint32_t iterationsToPixel(unsigned int iterations) {
#ifdef HSVCONV
    int H = (int)(0xff * iterations / maxIterations);
    int S = 0xff;
    int V = iterations < maxIterations? 255 : 0;

    int C = (1 - abs(2 * V - 1)) * S;
    int H1 = H * 6.0f / 255.0f;
    int X = C * (1 - abs((H1 % 2) - 1));

    int r1, g1, b1;

    switch (H1) {
        case 0:
            r1 = C; g1 = X; b1 = 0; break;
        case 1:
            r1 = X; g1 = C; b1 = 0; break;
        case 2:
            r1 = 0; g1 = C; b1 = X; break;
        case 3:
            r1 = 0; g1 = X; b1 = C; break;
        case 4:
            r1 = X; g1 = 0; b1 = C; break;
        case 5:
            r1 = C; g1 = 0; b1 = X; break;
        default:
            r1 = g1 = b1 = 0;
    }
    int m = V - C / 2;
    int r = r1 + m, g = g1 + m, b = b1 + m;
#endif //HSVCONV
#ifdef GREYSCALE
    //iterations = maxIterations - iterations;
    int r, g, b;

    int half = (maxIterations / 2) - iterations;
    half = abs(half);
    iterations = maxIterations - half * 2;

    if (iterations == 0 || iterations == maxIterations) {
        r = 0; g = 0; b = 0;
    } else {
        r = g = b = 0xff * iterations / maxIterations;
    }
    r = g = b = 0xff * iterations / maxIterations;
    /*if (iterations > maxIterations) {
        r = 0x8f * iterations / maxIterations;
        g = 0x8f * iterations / maxIterations;
        b = 0x8f * iterations / maxIterations;
    } else {
        r = 0xff;
        g = 0x8f * (maxIterations - iterations) / maxIterations;
        b = 0x8f * (maxIterations - iterations) / maxIterations;
    }*/
#endif // GREYSCALE
    uint32_t result = 0;
    result += r;
    result *= 0x100;
    result += g;
    result *= 0x100;
    result += b;

#ifndef NDEBUG
    printf("%d iterations (out of %d) resulted in colour %x\n",
            iterations,
            maxIterations,
            result
    );
#endif

    return result;
    
}

unsigned int iterate(const complex double zn, const complex double c, unsigned int iteration) {
    if (iteration > maxIterations)
        return maxIterations;
    complex double zn1 = zn * zn + c;
    return cabs(zn1) > cutoff? iteration : iterate(zn1, c, iteration + 1);
}

int main(int argc, char* args[]) {

    extern char *optarg;
    int opt;
    const char* validArgs = "r:w:h:x:y:i:c:";
    opterr = 0;

    // parse command line args
    while ((opt = getopt(argc, args, validArgs)) != -1) {
        switch (opt) {
        case 'r':
            resolution = strtod(optarg, NULL);
            break;
        case 'w':
            SCREEN_WIDTH = 1 * strtof(optarg, NULL);
            break;
        case 'h':
            SCREEN_HEIGHT = 1 * strtof(optarg, NULL);
            break;
        case 'x':
            center = strtod(optarg, NULL) + cimag(center) * I;
            break;
        case 'y':
            center = creal(center) + strtod(optarg, NULL) * I;
            break;
        case 'i':
            maxIterations = (int)strtof(optarg, NULL);
            break;
        case 'c':
            cutoff = strtod(optarg, NULL);
            break;
        default:
            fprintf(stderr, "Usage: %s [-%s]\n", args[0], validArgs);
            return -1;
        } 
    }

    SDL_Window* window = NULL;
    SDL_Surface* surface = NULL;

    if(SDL_Init( SDL_INIT_VIDEO ) < 0){
        sdlErrReport("SDL could not initialize! SDL_Error: %s\n");
        return -1;
    }

    window = SDL_CreateWindow(
            "SDL Tutorial",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            SCREEN_WIDTH,
            SCREEN_HEIGHT,
            SDL_WINDOW_SHOWN
    );
    if( window == NULL ) {
        sdlErrReport("Window could not be created! SDL_Error: %s\n");
        return -1;
    }

    surface = SDL_GetWindowSurface( window );

    // clear surface
    SDL_FillRect(
            surface,
            NULL,
            SDL_MapRGB(surface->format, 0xFF, 0xFF, 0xFF)
    );

    int boundx = SCREEN_HEIGHT/2;
    int boundy = SCREEN_WIDTH/2;
    for (int i = -boundx; i < boundx; i++) {
      for (int j = -boundy; j < boundy; j++) {
        SDL_LockSurface(surface);
          Uint32 *pixels = (Uint32 *)surface->pixels;
          pixels[(i + boundx) * surface->w + (j + boundy)] =
              iterationsToPixel(iterate(0, center + (j * resolution) + (i * resolution * I), 0));
        SDL_UnlockSurface(surface);
      }
    }
    
    printf("Mandelbrot written\n");

    SDL_UpdateWindowSurface(window);

    SDL_Event e;
    bool quit = false;
    while (!quit){
        while (SDL_PollEvent(&e)){
            if (e.type == SDL_QUIT){
                quit = true;
            }
            if (e.type == SDL_KEYDOWN){
                quit = true;
            }
        }
        SDL_Delay(100);
    }

    // cleanup
    SDL_FreeSurface(surface);
    SDL_DestroyWindow( window );
    SDL_Quit();

    return 0;
}
