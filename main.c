#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @note ALL x- and y-coordinates in function definitions refer to the particle
 * array coordinates, not screenspace coordinates. Look at the grid_t definition
 * for more explanation
 */

typedef struct grid_t grid_t;
typedef struct particle_t particle_t;
typedef void (*update_funcptr)(grid_t *, int, int);

/**
 * The particle grid. The array is a one-dimensional array of particles.
 * The bottom left of the grid is (0, 0) and the top right is (width, height).
 * Indexing into the array is done with the formula index = y * width + x
 *
 * @note Raylib does screen coordinates with (0, 0) as the top left and
 * (width, height) as the bottom right. The x-coordinates are the same, but the
 * y-coordinates from raylib to grid array are found with height - y
 */
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
    MAT_EMPTY = 0,
    MAT_SAND,
    MAT_WATER,
    MAT_COUNT
} material_type;

/**
 * Creates a new grid with no particle array (init_grid must be used to create
 * the array)
 *
 * @return The new grid
 */
grid_t *new_empty_grid(void);

/**
 * Creates a new grid with a particle array full of empty particles (init_grid
 * does not need to be used)
 *
 * @param width The width of the grid
 * @param height The height of the grid
 * @return The new grid
 */
grid_t *new_grid(int width, int height);

/**
 * Initializes a grid with no particle array (used after new_empty_grid)
 *
 * @param grid The grid to initialize
 * @param width The width of the grid
 * @param height The height of the grid
 */
void init_grid(grid_t *grid, int width, int height);

/**
 * Destroys a grid, freeing the allocated particle array
 *
 * @param grid The grid to destroy
 */
void destroy_grid(grid_t *grid);

/**
 * Clears a grid, setting all the particles to empty
 * @note I haven't used this anywhere yet
 *
 * @param grid The grid to clear
 */
void clear_grid(grid_t *grid);

/**
 * Gets the particle at the input coordinates
 *
 * @param grid The grid into which to index
 * @param x The x-coordinate in the particle array
 * @param y The y-coordinate in the particle array
 * @return A pointer to the particle at the input coordinates
 */
particle_t *get_particle(grid_t *grid, int x, int y);

/**
 * Copies the information about an input particle to the particle at the input
 * coordinates
 *
 * @param grid The grid of particles
 * @param x The x-coordinate in the particle array
 * @param y The y-coordinate in the particle array
 * @param p The particle whose data is set into the array
 */
void set_particle(grid_t *grid, int x, int y, particle_t *p);

/**
 * Adds a new particle of type "id" into the array at the input coordinates
 *
 * @param grid The grid of particles
 * @param x The x-coordinate in the particle array
 * @param y The y-coordinate in the particle array
 * @param id The particle type
 */
void add_particle(grid_t *grid, int x, int y, unsigned int id);

/**
 * Removes a particle in the array at the input coordinates
 * @note This really just sets the particle type to "Empty"
 *
 * @param grid The grid of particles
 * @param x The x-coordinate in the particle array
 * @param y The y-coordinate in the particle array
 */
void remove_particle(grid_t *grid, int x, int y);

/**
 * Swaps two particles in the particle array
 *
 * @param grid The grid of particles
 * @param x1 The x-coordinate of the first particle
 * @param y1 The y-coordinate of the first particle
 * @param x2 The x-coordinate of the second particle
 * @param y2 The y-coordinate of the second particle
 */
void swap_particles(grid_t *grid, int x1, int y1, int x2, int y2);

/**
 * Adds or removes a line of particles of type m from (x1, y1) to (x2, y2)
 * @note Removal is only done if the particle type is "Empty"
 *
 * @param grid The grid of particles
 * @param x1 The starting x-coordinate
 * @param y1 The starting y-coordinate
 * @param x2 The ending x-coordinate
 * @param y2 The ending y-coordinate
 * @param m The type of the particle to add/remove
 */
void particle_line(grid_t *grid, int x1, int y1, int x2, int y2,
                   material_type m);

/**
 * Checks if the particle type is "Empty"
 *
 * @param particle The particle to check
 * @return A boolean indicating if the particle is empty
 */
bool is_empty(particle_t *particle);

/**
 * Checks if the particle type in the array at the input coordinates is "Empty"
 * @note This function does bounds checking. Coordinates outside of the grid
 * (ie, less than 0 or greater than width/height) are considered "occupied"
 *
 * @param grid The grid of particles
 * @param x The x-coordinate in the particle array
 * @param y The y-coordinate in the particle array
 * @return A boolean indicating if the particle is empty
 */
bool is_empty_pos(grid_t *grid, int x, int y);

/**
 * The update function for sand particles. Moves the sand down if possible
 *
 * @param grid The grid of particles
 * @param x The x-coordinate in the particle array of the sand
 * @param y The y-coordinate in the particle array of the sand
 */
void update_sand(grid_t *grid, int x, int y);

/**
 * The update function for empty particles.
 * @note This simply returns
 *
 * @param grid The grid of particles
 * @param x The x-coordinate in the particle array
 * @param y The y-coordinate in the particle array
 */
void update_empty(grid_t *grid, int x, int y);

/**
 * The update function for water particles
 *
 * @param grid The grid of particles
 * @param x The x-coordinate in the particle array of the water
 * @param y The y-coordinate in the particle array of the water
 */
void update_water(grid_t *grid, int x, int y);

/**
 * Sets the current material to the next type
 *
 * @param m The current material
 * @return The new material type
 */
material_type next_material(material_type m);

/**
 * Sets the current material to the previous type
 *
 * @param m The current material
 * @return The new material type
 */
material_type prev_material(material_type m);

int 
main(void)
{
    int x = 0, y = 0, cursor_size = 1;
    int prev_pos[2] = {0, 0};
    int curr_pos[2] = {0, 0};
    material_type cur_mat = MAT_SAND;
    grid_t *grid = new_grid(512, 512);
    particle_t *cur_particle = NULL;

    InitWindow(512, 512, "Falling Sand");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        curr_pos[0] = GetMouseX();
        curr_pos[1] = 511 - GetMouseY();

        /*if (GetMouseWheelMoveV().y > 0) {*/
            /* Increase the drawing size */
        /*}*/
        /*else if (GetMouseWheelMoveV().y < 0) {*/
            /* Decrease the drawing size */
        /*}*/

        if (IsKeyPressed(KEY_RIGHT)) {
            cur_mat = next_material(cur_mat);
        }
        else if (IsKeyPressed(KEY_LEFT)) {
            cur_mat = prev_material(cur_mat);
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            particle_line(grid, prev_pos[0], prev_pos[1], curr_pos[0],
                          curr_pos[1], cur_mat);
        }
        else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            particle_line(grid, prev_pos[0], prev_pos[1], curr_pos[0],
                          curr_pos[1], MAT_EMPTY);
        }

        BeginDrawing();
        {
            ClearBackground(BLACK);
            for (y = 0; y < 512; y++) {
                for (x = 0; x < 512; x++) {
                    cur_particle = get_particle(grid, x, y);
                    DrawPixel(x, 511 - y, cur_particle->color);
                    cur_particle->update_func(grid, x, y);
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
        grid->arr[i].id = MAT_EMPTY;
        grid->arr[i].life_time = 0.0f;
        grid->arr[i].velocity = (Vector2){0.0f, 0.0f};
        grid->arr[i].color = BLANK;
        grid->arr[i].has_been_updated = false;
        grid->arr[i].update_func = update_empty;
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
        grid->arr[i].id = MAT_EMPTY;
        grid->arr[i].life_time = 0.0f;
        grid->arr[i].velocity = (Vector2){0.0f, 0.0f};
        grid->arr[i].color = BLANK;
        grid->arr[i].has_been_updated = false;
        grid->arr[i].update_func = update_empty;
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
    int x, y;

    for (y = 0; y < grid->height; y++) {
        for (x = 0; x < grid->width; x++) {
            remove_particle(grid, x, y);
        }
    }
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

    if (index < 0 || index > grid->width * grid->height) { return; }

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
        case MAT_SAND:
            part.life_time = 0.0f;
            part.color = YELLOW;
            part.update_func = update_sand;
            break;
        case MAT_WATER:
            part.life_time = 0.0f;
            part.color = SKYBLUE;
            part.update_func = update_water;
        default:
            break;
    }
    set_particle(grid, x, y, &part);
}

void
remove_particle(grid_t *grid, int x, int y)
{
    particle_t part;

    if (is_empty_pos(grid, x,  y)) { return; }

    part.id = MAT_EMPTY;
    part.velocity = (Vector2){0.0f, 0.0f};
    part.has_been_updated = false;
    part.life_time = 0.0f;
    part.color = BLANK;
    part.update_func = update_empty;

    set_particle(grid, x, y, &part);
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

    if (m == MAT_EMPTY) {
        while (1) {
            remove_particle(grid, x1, y1);
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
    else {
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

}

bool
is_empty(particle_t *particle)
{
    return particle->id == MAT_EMPTY;
}

bool
is_empty_pos(grid_t *grid, int x, int y)
{
    if (x < 0 || x >= grid->width || y < 0 || y >= grid->height) {
        return false;
    }
    return grid->arr[y * grid->width + x].id == MAT_EMPTY;
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

void 
update_water(grid_t *grid, int x, int y)
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
    else if (is_empty_pos(grid, left, y)) {
        swap_particles(grid, x, y, left, y);
    }
    else if (is_empty_pos(grid, right, y)) {
        swap_particles(grid, x, y, right, y);
    }

    cur_particle->has_been_updated = true;
}

material_type 
next_material(material_type m)
{
    if (m == MAT_COUNT - 1) { return m; }

    return m + 1;
}

material_type 
prev_material(material_type m)
{
    if (m == 1) { return m; }

    return m - 1;
}
/* EOF */
