#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct grid_t grid_t;
typedef struct particle_t particle_t;
typedef enum material_type material_type;
typedef void (*update_funcptr)(grid_t *, unsigned int, unsigned int);

struct grid_t
{
    unsigned int width;
    unsigned int height;
    particle_t *arr;
};

struct particle_t
{
    unsigned int id;
    float life_time;
    Vector2 velocity;
    Color color;
    bool has_been_updated;
    update_funcptr update_func;
};

enum material_type
{
    SPECIES_EMPTY = 0,
    SPECIES_SAND,
    SPECIES_COUNT
};

grid_t * new_empty_grid(void);

grid_t * new_grid(unsigned int width, unsigned int height);

void init_grid(grid_t *grid, unsigned int width, unsigned int height);

void destroy_grid(grid_t *grid);

void clear_grid(grid_t *grid);

particle_t * get_particle(grid_t *grid, unsigned int x, unsigned int y);

void set_particle(grid_t *grid, unsigned int x, unsigned int y, particle_t *p);

void add_particle(grid_t *grid, unsigned int x, unsigned int y,
                  unsigned int id);

void remove_particle(grid_t *grid, unsigned int x, unsigned int y);

void swap_particles(grid_t *grid, unsigned int x1, unsigned int y1,
                    unsigned int x2, unsigned int y2);

bool is_empty(particle_t *particle);

bool is_empty_pos(grid_t *grid, unsigned int x, unsigned int y);

void update_sand(grid_t *grid, unsigned int x, unsigned int y);

int 
main(void)
{
    unsigned int x = 0, y = 0;
    grid_t *grid = new_grid(512, 512);
    particle_t *cur_particle = NULL;

    InitWindow(512, 512, "Falling Sand");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            add_particle(grid, GetMouseX(), GetMouseY(), SPECIES_SAND);
        }

        BeginDrawing();
        {
            ClearBackground(RAYWHITE);
            for (y = 0; y < 512; y++) {
                for (x = 0; x < 512; x++) {
                    cur_particle = get_particle(grid, x, y);
                    if (!is_empty(cur_particle)) {
                        cur_particle->update_func(grid, x, y);
                    }
                    DrawPixel(x + 1, 512 - y, cur_particle->color);
                }
            }
            DrawFPS(0, 0);
        }
        EndDrawing();
    }

    destroy_grid(grid);
    CloseWindow();

    return 0;
}

grid_t *
new_empty_grid(void)
{
    grid_t *grid = malloc(sizeof(*grid));

    grid->width = 0;
    grid->height = 0;
    grid->arr = NULL;

    return grid;
}

grid_t *
new_grid(unsigned int width, unsigned int height)
{
    unsigned int i;
    grid_t *grid = malloc(sizeof(*grid));

    grid->width = width;
    grid->height = height;
    grid->arr = calloc(width * height, sizeof(*grid->arr));

    if (grid->arr == NULL) {
        fprintf(stderr, "Error: Could not allocate enough memory at %d in %s\n",
                __LINE__, __FILE__);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < width * height; i++) {
        grid->arr[i].id = SPECIES_EMPTY;
        grid->arr[i].life_time = 0.0f;
        grid->arr[i].velocity = (Vector2){0.0f, 0.0f};
        grid->arr[i].color = BLANK;
        grid->arr[i].has_been_updated = false;
        grid->arr[i].update_func = NULL;
    }

    return grid;
}

void
init_grid(grid_t *grid, unsigned int width, unsigned int height)
{
    unsigned int i;

    grid->width = width;
    grid->height = height;
    grid->arr = calloc(width * height, sizeof(*grid->arr));

    if (grid->arr == NULL) {
        fprintf(stderr, "Error: Could not allocate enough memory at %d in %s\n",
                __LINE__, __FILE__);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < width * height; i++) {
        grid->arr[i].id = SPECIES_EMPTY;
        grid->arr[i].life_time = 0.0f;
        grid->arr[i].velocity = (Vector2){0.0f, 0.0f};
        grid->arr[i].color = BLANK;
        grid->arr[i].has_been_updated = false;
        grid->arr[i].update_func = NULL;
    }
}

void
destroy_grid(grid_t *grid)
{
    free(grid->arr);
    grid->arr = NULL;

    free(grid);
    grid = NULL;
}

void
clear_grid(grid_t *grid)
{
    memset(grid->arr, 0, sizeof(*grid->arr));
}

particle_t *
get_particle(grid_t *grid, unsigned int x, unsigned int y)
{
    return &grid->arr[y * grid->width + x];
}

void
set_particle(grid_t *grid, unsigned int x, unsigned int y, particle_t *p)
{
    grid->arr[y * grid->width + x] = *p;
}

void
add_particle(grid_t *grid, unsigned int x, unsigned int y, unsigned int id)
{
    particle_t part;
    
    if (!is_empty_pos(grid, x, y)) { return; }

    part.id = id;
    part.velocity = (Vector2){0, 0};
    part.has_been_updated = false;

    /* TODO: Update this with correct lifetimes */
    switch (id) {
        case SPECIES_SAND:
            part.life_time = 0.0f;
            part.color = YELLOW;
            part.update_func = update_sand;
            break;
        default:
            break;
    }
    set_particle(grid, x, y, &part);
}

void
remove_particle(grid_t *grid, unsigned int x, unsigned int y) {
    particle_t part = {
        SPECIES_EMPTY,
        0.0f,
        (Vector2){0.0f, 0.0f},
        BLANK,
        false,
        NULL
    };
    set_particle(grid, x, y, &part);
}

void
swap_particles(grid_t *grid, unsigned int x1, unsigned int y1, unsigned int x2,
               unsigned int y2)
{
    particle_t temp = grid->arr[y1 * grid->width + x1];
    grid->arr[y1 * grid->width + x1] = grid->arr[y2 * grid->width + x2];
    grid->arr[y2 * grid->width + x2] = temp;
}

bool
is_empty(particle_t *particle)
{
    return particle->id == SPECIES_EMPTY;
}

bool
is_empty_pos(grid_t *grid, unsigned int x, unsigned int y)
{
    if (x > grid->width || y > grid->height) { return false; }
    return grid->arr[y * grid->width + x].id == SPECIES_EMPTY;
}

void
update_sand(grid_t *grid, unsigned int x, unsigned int y)
{
    unsigned int below = y - 1;
    unsigned int left  = x - 1;
    unsigned int right = x + 1;
    particle_t *cur_particle = get_particle(grid, x, y);

    if (y == 0) {
        cur_particle->has_been_updated = true;
        return;
    }
    
    if (is_empty_pos(grid, x, below)) {
        swap_particles(grid, x, y, x, below);
    }
    else if (is_empty_pos(grid, left, below)) {
        swap_particles(grid, x, y, left, below);
    }
    else if (is_empty_pos(grid, right, below)) {
        swap_particles(grid, x, y, right, below);
    }

    cur_particle->has_been_updated = true;
}
/* EOF */
