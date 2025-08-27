#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL_blendmode.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL.h>

#define RGBA_A(rgba) rgba & 0x000ff
#define RGBA_B(rgba) (rgba >> 8) & 0x000ff
#define RGBA_G(rgba) (rgba >> 16) & 0x000ff
#define RGBA_R(rgba) (rgba >> 24) & 0x000ff
#define RGBA(rgba) RGBA_R(rgba), RGBA_G(rgba), RGBA_B(rgba), RGBA_A(rgba)

int running = 1;

typedef struct block_t {
    char shape[4][3];
    char w, h;
    unsigned rgba;
} block_t;

#define BLOCK_NUM 7
block_t blocks[BLOCK_NUM] = {
    {
        .shape =  {{1, 0, 0}, // ██
                   {1, 0, 0}, // ██
                   {1, 0, 0}, // ██
                   {1, 0, 0}  // ██
            }, .w = 1, .h = 4, .rgba = 0xffffffff
    },
    {
        .shape =  {{0, 1, 0}, //   ██
                   {1, 1, 1}, // ██████
                   {0, 0, 0},
                   {0, 0, 0}
            }, .w = 3, .h = 2, .rgba = 0xffffffff
    },
    {
        .shape =  {{0, 1, 1}, //   ████
                   {1, 1, 0}, // ████
                   {0, 0, 0},
                   {0, 0, 0}
            }, .w = 3, .h = 2, .rgba = 0xffffffff
    },
    {
        .shape =  {{1, 1, 0}, // ████
                   {0, 1, 1}, //   ████
                   {0, 0, 0},
                   {0, 0, 0}
            }, .w = 3, .h = 2, .rgba = 0xffffffff
    },
    {
        .shape =  {{1, 0, 0}, // ██
                   {1, 1, 1}, // ██████
                   {0, 0, 0},
                   {0, 0, 0}
            }, .w = 3, .h = 2, .rgba = 0xffffffff
    },
    {
        .shape =  {{0, 0, 1}, //     ██
                   {1, 1, 1}, // ██████
                   {0, 0, 0},
                   {0, 0, 0}
            }, .w = 3, .h = 2, .rgba = 0xffffffff
    },
    {
        .shape =  {{1, 1, 0}, // ████
                   {1, 1, 0}, // ████
                   {0, 0, 0},
                   {0, 0, 0}
            }, .w = 2, .h = 2, .rgba = 0xffffffff
    },
};

#define BLOCK_COL 15
#define BLOCK_ROW 25
#define BLOCK_SIZE 20

char tetris[BLOCK_ROW][BLOCK_COL] = {0};

block_t get_block() {
    int index = rand() % BLOCK_NUM;
    return blocks[index];
}

block_t block_rotate(block_t block) {
    block_t rotated = {
        .shape = {{0}}, .w = block.h, .h = block.w, .rgba = block.rgba
    };
    for (int y = 0; y < block.h; y++) {
        for (int x = 0; x < block.w; x++) {
            rotated.shape[x][block.h - y - 1] = block.shape[y][x];
        }
    }

    return rotated;
}

void block_fix(block_t block, int x, int y) {
    for (int i = x; i < x + block.w; i++) {
        for (int j = y; j < y + block.h; j++) {
            if (block.shape[j - y][i - x]) {
                tetris[j][i] = block.shape[j - y][i - x];
            }
        }
    }
}

int check_move(block_t block, int x, int y) {
    int check = 1;
    if (x + block.w > BLOCK_COL || x < 0 || y + block.h > BLOCK_ROW || y < 0) check = 0;
    for (int j = block.h - 1; j > -1; j--) {
        for (int i = block.w - 1; i > -1; i--) {
            if (block.shape[j][i] == 1 && tetris[y + j][x + i] == 1) {
                check = 0;
            }
        }
    }

    return check;
}

void rect_draw(SDL_Renderer *renderer, unsigned rgba, int x, int y) {
    SDL_SetRenderDrawColor(renderer, RGBA(rgba));
    SDL_Rect rect = {
        .x = x * BLOCK_SIZE,
        .y = y * BLOCK_SIZE,
        .w = BLOCK_SIZE,
        .h = BLOCK_SIZE
    };
    SDL_RenderFillRect(renderer, &rect);
}

void block_undraw(SDL_Renderer *renderer, block_t block, int x, int y) {
    for (int j = 0; j < block.h; j++) {
        for (int i = 0; i < block.w; i++) {
            if (block.shape[j][i]) rect_draw(renderer, 0x000000ff, x + i, y + j);
        }
    }
}

void block_draw(SDL_Renderer *renderer, block_t block, int x, int y) {
    for (int j = 0; j < block.h; j++) {
        for (int i = 0; i < block.w; i++) {
            if (block.shape[j][i]) rect_draw(renderer, 0xffffffff, x + i, y + j);
        }
    }
}

void tetris_draw(SDL_Renderer *renderer) {
    for (int y = 0; y < BLOCK_ROW; y++) {
        for (int x = 0; x < BLOCK_COL; x++) {
            if(tetris[y][x]) {
                rect_draw(renderer, 0xffffffff, x, y);
            }
            else {
                rect_draw(renderer, 0x000000ff, x, y);
            }
        }
    }
    SDL_RenderPresent(renderer);
}

void clear_line() {
    for (int y = BLOCK_ROW - 1; y > -1; y--) {
        int is_filled = 1;
        for (int x = BLOCK_COL - 1; x > -1; x--) {
            if (!tetris[y][x]) {
                is_filled = 0;
                break;
            }
        }
        if (is_filled) {
            for (int j = y; j > 0; j--) {
                for (int i = BLOCK_COL - 1; i > -1; i--) {
                    tetris[j][i] = tetris[j - 1][i];
                }
            }
        }
    }
}

int run(SDL_Window *win, SDL_Renderer *renderer) {
    srand((unsigned)time(NULL));
    SDL_Event event;
    int running = 1;

    while (running) {
        int x = 0, y = 0, old_x = 0, old_y = 0;
        block_t block = get_block(), old_block;
        x = (BLOCK_COL - block.w) / 2;
        running = check_move(block, x, y);
        int i = 0;
        while (check_move(block, x, y + 1) && running) {
            old_block = block;
            old_x = x;
            old_y = y;

            if (i >= 50) {
                i = 0;
                y++;
            }

            if (SDL_PollEvent(&event) != 0) {
                switch (event.type) {
                    case SDL_QUIT: running = 0;  break;
                    case SDL_KEYDOWN: 
                        if (event.key.keysym.sym == SDLK_RETURN) {
                            block_t rotated = block_rotate(block);
                            if (check_move(rotated, x, y)) block = rotated;
                        }
                        if (event.key.keysym.sym == SDLK_DOWN) {
                            for (; check_move(block, x, y + 1); y++) {}
                        }
                        if (event.key.keysym.sym == SDLK_LEFT) {
                            if (check_move(block, x - 1, y)) x--;
                        }
                        if (event.key.keysym.sym == SDLK_RIGHT) {
                            if (check_move(block, x + 1, y)) x++;
                        }
                        break;
                }
            }

            block_undraw(renderer, old_block, old_x, old_y);
            block_draw(renderer, block, x, y);
            SDL_RenderPresent(renderer);
            SDL_Delay(10); i++;
        }
        block_fix(block, x, y);
        clear_line();
        tetris_draw(renderer);
    }
}

int main() {
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window *win;
    SDL_Renderer *renderer;
    int result = SDL_CreateWindowAndRenderer(BLOCK_COL * BLOCK_SIZE, BLOCK_ROW * BLOCK_SIZE, SDL_WINDOW_SHOWN, &win, &renderer);

    if (result != 0) {
        fprintf(stderr, "Failed to create a window and renderer! Error: %s", SDL_GetError());
        return 1;
    }

    run(win, renderer);

    SDL_DestroyWindow(win);
    SDL_DestroyRenderer(renderer);

    win = NULL;
    renderer = NULL;

    SDL_Quit();

    return 0;
}
