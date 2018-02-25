#include <SDL/SDL.h>

void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
    *p = pixel;
}

class GPU {
  private:
    SDL_Surface *screen;

  public:
    static constexpr int WIDTH = 160;
    static constexpr int HEIGHT = 144;

    bool init() {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            return false;
        }

        if ((screen = SDL_SetVideoMode(WIDTH, HEIGHT, 0, 0)) == NULL) {
            return false;
        }

        SDL_Event event;
        bool running = true;
        while (running)
        {
            if (SDL_WaitEvent(&event)) {
                switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                default:
                    break;
                }
            }
            SDL_Surface *bitmap = SDL_CreateRGBSurface(0, WIDTH, HEIGHT, 32, 0, 0, 0, 0);
            draw(bitmap);
            SDL_BlitSurface(bitmap, NULL, screen, NULL);
            SDL_UpdateRect(screen, 0, 0, WIDTH, HEIGHT);
        }

        return true;
    }

    void draw(SDL_Surface *surface) {
        SDL_LockSurface(surface);
        for (int x = 0; x < WIDTH; x++) {
            for (int y = 0; y < HEIGHT; y++) {
                putpixel(surface, x, y, 0xff0000ff);
            }
        }
        SDL_UnlockSurface(surface);
    }
};
