#include "mmu.hpp"
#include <SDL/SDL.h>

void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
    *p = pixel;
}

enum class GPUMode {
    OAM_READ,
    VRAM_READ,
    HBLANK,
    VBLANK
};

class GPU {
  private:
    MMU &mmu;
    SDL_Surface *screen;
    GPUMode mode;
    uint8_t line;
    int mode_clock;

    uint8_t scroll_x;
    uint8_t scroll_y;

  public:
    static constexpr uint8_t WIDTH = 160;
    static constexpr uint8_t HEIGHT = 144;

    GPU(MMU &_mmu) : mmu(_mmu) {
        reset();
    }

    void reset() {
        scroll_x = 0;
        scroll_y = 0;
        mode = GPUMode::OAM_READ;
        line = 0;
        mode_clock = 0;
    }

    void render_scan() {
        // bgmap ? 0x1c00 : 0x1800
        uint16_t mapoffs = 0x1800;
        mapoffs += ((line + scroll_y) & 0xff) >> 3;

        uint8_t lineoffs = scroll_x >> 3;

        // where in the tile to read
        auto y = static_cast<uint16_t>((line + scroll_y) & 7);
        auto x = static_cast<uint16_t>(scroll_x & 7);

        uint8_t lower = mmu.rb(static_cast<uint16_t>(0x8000) + mapoffs + lineoffs);
        uint8_t upper = mmu.rb(static_cast<uint16_t>(0x8001) + mapoffs + lineoffs);
        //	if (_bgtile == 1 && tile < 128) tile += 256;
        for (int i = 0; i < WIDTH; i++) {
            uint32_t color = 0;
            color |= (upper >> (x-1)) & 2;
            color |= (lower >> x) & 1;
            switch (color) {
            case 0:
                color = 0;
                break;
            case 1:
                color = 0xff606060;
                break;
            case 2:
                color = 0xffc0c0c0;
                break;
            case 3:
                color = 0xffffffff;
                break;
            default:
                throw std::out_of_range(std::string("Unexpected color bits: ") + std::to_string(color));
            }

            putpixel(screen, i, line, color);

            x++;
            if (x == 8) {
                x = 0;
                // read next tile
            }
        }
    }

    void render_image() {
        // TODO
    }

    void step(uint16_t reg_t) {
        mode_clock += reg_t;

        switch (mode) {
        case GPUMode::OAM_READ:
            if (mode_clock >= 80) {
                mode_clock = 0;
                mode = GPUMode::VRAM_READ;
            }
            break;
        case GPUMode::VRAM_READ:
            if (mode_clock >= 172) {
                mode_clock = 0;
                mode = GPUMode::HBLANK;

                render_scan();
            }
            break;
        case GPUMode::HBLANK:
            if (mode_clock >= 204) {
                mode_clock = 0;
                line++;

                if (line == 143) {
                    mode = GPUMode::VBLANK;
                    render_image();
                } else {
                    mode = GPUMode::OAM_READ;
                }
            }
            break;
        case GPUMode::VBLANK:
            if (mode_clock >= 456) {
                mode_clock = 0;
                line++;
            }

            if (line > 153) {
                mode = GPUMode::OAM_READ;
                line = 0;
            }
        }
    }

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
