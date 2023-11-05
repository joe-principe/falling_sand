#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct grid_t grid_t;
typedef struct particle_t particle_t;
typedef void (*update_funcptr)(grid_t *, int, int);

struct grid_t
{
    int width;
    int height;
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

typedef enum material_type
{
    SPECIES_EMPTY = 0,
    SPECIES_SAND,
    SPECIES_COUNT
} material_type;

grid_t * new_empty_grid(void);

grid_t * new_grid(int width, int height);

void init_grid(grid_t *grid, int width, int height);

void destroy_grid(grid_t *grid);

void clear_grid(grid_t *grid);

particle_t * get_particle(grid_t *grid, int x, int y);

void set_particle(grid_t *grid, int x, int y, particle_t *p);

void add_particle(grid_t *grid, int x, int y, unsigned int id);

void remove_particle(grid_t *grid, int x, int y);

void swap_particles(grid_t *grid, int x1, int y1, int x2, int y2);

void particle_line(grid_t *grid, int x1, int y1, int x2, int y2,
                   material_type m);

bool is_empty(particle_t *particle);

bool is_empty_pos(grid_t *grid, int x, int y);

void update_sand(grid_t *grid, int x, int y);

void update_empty(grid_t *grid, int x, int y);

int 
main(void)
{
    int x = 0, y = 0;
    int prev_pos[2] = {0, 0};
    int curr_pos[2] = {0, 0};
    grid_t *grid = new_grid(512, 512);
    particle_t *cur_particle = NULL;

    InitWindow(512, 512, "Falling Sand");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        curr_pos[0] = GetMouseX();
        curr_pos[1] = 511 - GetMouseY();

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            particle_line(grid, prev_pos[0], prev_pos[1], curr_pos[0],
                          curr_pos[1], SPECIES_SAND);
        }
        /*else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {*/
            /*remove_particle(grid, curr_pos[0], curr_pos[1]);*/
        /*}*/

        BeginDrawing();
        {
            ClearBackground(BLACK);
            for (y = 0; y < 512; y++) {
                for (x = 0; x < 512; x++) {
                    cur_particle = get_particle(grid, x, y);
                    DrawPixel(x, 511 - y, cur_particle->color);
                    if (!is_empty(cur_particle)) {
                        cur_particle->update_func(grid, x, y);
                    }
                }
            }
            DrawFPS(0, 0);
        }
        EndDrawing();

        prev_pos[0] = curr_pos[0];
        prev_pos[1] = curr_pos[1];
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
new_grid(int width, int height)
{
    int i;
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
init_grid(grid_t *grid, int width, int height)
{
    int i;

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
get_particle(grid_t *grid, int x, int y)
{
    return &grid->arr[y * grid->width + x];
}

void
set_particle(grid_t *grid, int x, int y, particle_t *p)
{
    int index = y * grid->width + x;

    grid->arr[index].id = p->id;
    grid->arr[index].velocity = p->velocity;
    grid->arr[index].has_been_updated = p->has_been_updated;
    grid->arr[index].life_time = p->life_time;
    grid->arr[index].color = p->color;
    grid->arr[index].update_func = p->update_func;
}

void
add_particle(grid_t *grid, int x, int y, unsigned int id)
{
    particle_t part;

    if (!is_empty_pos(grid, x, y)) { return; }

    part.id = id;
    part.velocity = (Vector2){0.0f, 0.0f};
    part.has_been_updated = false;

    /* TODO: Update this with correct lifetimes */
    switch (id) {
        case SPECIES_SAND:
            part.life_time = 0.0f;
            part.color = YELLOW;
            part.update_func = update_sand;
            break;
        default:
            part.life_time = 0.0f;
            part.color = BLANK;
            part.update_func = update_empty;
            break;
    }
    set_particle(grid, x, y, &part);
}

void
remove_particle(grid_t *grid, int x, int y)
{
    int index = y * grid->width + x;

    if (is_empty_pos(grid, x,  y)) { return; }

    grid->arr[index].id = SPECIES_EMPTY;
    grid->arr[index].velocity = (Vector2){0.0f, 0.0f};
    grid->arr[index].has_been_updated = false;
    grid->arr[index].life_time = 0.0f;
    grid->arr[index].color = BLANK;
    grid->arr[index].update_func = update_empty;
}

void
swap_particles(grid_t *grid, int x1, int y1, int x2, int y2)
{
    particle_t temp = grid->arr[y1 * grid->width + x1];
    grid->arr[y1 * grid->width + x1] = grid->arr[y2 * grid->width + x2];
    grid->arr[y2 * grid->width + x2] = temp;
}

void
particle_line(grid_t *grid, int x1, int y1, int x2, int y2, material_type m)
{
    int dx = abs(x2 - x1);
    int sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1);
    int sy = y1 < y2 ? 1 : -1;
    int error = dx + dy;
    int e2;

    while (1) {
        add_particle(grid, x1, y1, m);

        if (x1 == x2 && y1 == y2) { break; }

        e2 = 2 * error;

        if (e2 >= dy) {
            if (x1 == x2) { break; }
            error += dy;
            x1 += sx;
        }

        if (e2 <= dx) {
            if (y1 == y2) { break; }
            error += dx;
            y1 += sy;
        }
    }
}

bool
is_empty(particle_t *particle)
{
    return particle->id == SPECIES_EMPTY;
}

bool
is_empty_pos(grid_t *grid, int x, int y)
{
    if (x < 0 || x >= grid->width || y < 0 || y >= grid->height) {
        return false;
    }
    return grid->arr[y * grid->width + x].id == SPECIES_EMPTY;
}

void
update_sand(grid_t *grid, int x, int y)
{
    int below = y - 1;
    int left  = x - 1;
    int right = x + 1;
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

void update_empty(grid_t *grid, int x, int y)
{
    return;
}
/* EOF */
