#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
 * The reason for making the grid one-dimensional is because 1D heap arrays are
 * contiguous where as 2D+ heap arrays are not guaranteed to be contiguous. This
 * is mostly to speed up array access.
 *
 * The bottom left of the grid is (0, 0) and the top right is (width, height).
 * The reason for doing this is because grid updates are done from the bottom
 * up, so it made sense for the bottom left to be (0, 0)
 *
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

/**
 * Materials contain the actual particle type (eg, sand, water, etc.)
 * New particles can be added anywhere between EMPTY and COUNT. COUNT isn't an
 * actual particle type, but rather a support enum to tell us how many particle
 * types are already defined. Because the enum starts at 0, COUNT will always
 * contain the number of non-empty particles currently defined
 *
 * Whenever you add a new particle, do the following:
 * 1. Create a new material type
 * 2. Create a new update function
 * 3. Create a new entry within add_particle
 * 4. Update particle interactions
 * 5. Add particle color to get_color_from_mat
 */
typedef enum material_type
{
    MAT_EMPTY = 0,
    MAT_SAND,
    MAT_WATER,
    MAT_SMOKE,
    MAT_OIL,
    MAT_WALL,
    MAT_WOOD,
    MAT_FIRE,
    MAT_FLAME,
    MAT_COUNT
} material_type;

/**
 * Elements contain the particle behavior class
 * Empty is obvious, static is a particle that doesn't fall, solid falls and
 * is generally denser than other elements, liquid falls and is denser than
 * solid but less dense than gas, gas rises and is the least dense.
 *
 * These are used for checking how two particles should interact in general.
 * Specific interactions will have to be handled on a specific basis.
 */
typedef enum element_type
{
    ELEM_EMPTY = 0,
    ELEM_STATIC,
    ELEM_SOLID,
    ELEM_LIQUID,
    ELEM_GAS,
    ELEM_COUNT
} element_type;

/**
 * Most of these properties are pretty self explanatory. has_been_updated is for
 * checking if the current particle has been swapped with another particle in
 * the current frame (eg, if a water particle moves right, it'll swap with the
 * empty space. Since it's now next to be checked (again), we avoid re-updating
 * it by checking if it's been updated)
 *
 * The update function pointer allows us to have any number of particle types
 * without having to create new cases in a switch statement whenever we want to
 * add something. We simply have to call the current particle's function 
 * whenever we update it.
 */
struct particle_t
{
    material_type mat_type;
    element_type elem_type;
    float life_time;
    Vector2 velocity;
    Color color;
    bool has_been_updated;
    update_funcptr update_func;
};

/**
 * Creates a new grid with no particle array (init_grid must be used to create
 * the array)
 * @note I don't use this anywhere in the code just yet, but the idea behind it
 * is creating a new, uninitialized grid for later use within code
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
 * Initializes a grid that has no particle array (used after new_empty_grid)
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
particle_t *get_particle(const grid_t *grid, int x, int y);

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
 * Gets the material type of a particle
 *
 * @param p The particle
 * @return The material type of the particle
 */
material_type get_particle_type(const particle_t *p);

/**
 * Gets the material type of a particle at the input coordinates
 *
 * @param grid The grid of particles
 * @param x The x-coordinate in the particle array
 * @param y The y-coordinate in the particle array
 * @return The material type of the particle
 */
material_type get_particle_type_pos(const grid_t *grid, int x, int y);

/**
 * Adds a new particle of type m into the array at the input coordinates
 *
 * @param grid The grid of particles
 * @param x The x-coordinate in the particle array
 * @param y The y-coordinate in the particle array
 * @param m The particle type
 */
void add_particle(grid_t *grid, int x, int y, material_type m);

/**
 * Removes a particle in the array at the input coordinates. Used when erasing
 * particles with right-click
 * @note This really just sets the particle type to "Empty"
 *
 * @param grid The grid of particles
 * @param x The x-coordinate in the particle array
 * @param y The y-coordinate in the particle array
 */
void remove_particle(grid_t *grid, int x, int y);

/**
 * Swaps two particles in the particle array
 * @note Use this whenever you want a particle to move. Non-empty particles
 * "swap" places with empty ones to give them the appearance of moving
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
 * @note A line of particles is drawn to enable continuous particle drawing.
 * Otherwise, only one particle is drawn per frame and they are widely spread
 * out if the mouse is moved around rapidly.
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
 * Checks if the particle type is "EMPTY"
 *
 * @param particle The particle to check
 * @return A boolean indicating if the particle is empty
 */
bool is_particle_empty(const particle_t *particle);

/**
 * Checks if the particle's element type is "STATIC"
 *
 * @param particle The particle to check
 * @return A boolean indicating if the particle is static
 */
bool is_particle_static(const particle_t *particle);

/**
 * Checks if the particle's element type is "SOLID"
 *
 * @param particle The particle to check
 * @return A boolean indicating if the particle is solid
 */
bool is_particle_solid(const particle_t *particle);

/**
 * Checks if the particle's element type is "LIQUID"
 *
 * @param particle The particle to check
 * @return A boolean indicating if the particle is liquid
 */
bool is_particle_liquid(const particle_t *particle);

/**
 * Checks if the particle's element type is "GAS"
 *
 * @param particle The particle to check
 * @return A boolean indicating if the particle is gas
 */
bool is_particle_gas(const particle_t *particle);

/**
 * Checks if the particle type in the array at the input coordinates is "EMPTY"
 * @note This function does bounds checking. Coordinates outside of the grid
 * (ie, less than 0 or greater than width/height) are considered "occupied".
 * This is so the particles don't flow outside the bounds of the game window
 *
 * @param grid The grid of particles
 * @param x The x-coordinate in the particle array
 * @param y The y-coordinate in the particle array
 * @return A boolean indicating if the particle is empty
 */
bool is_pos_empty(const grid_t *grid, int x, int y);

/**
 * Checks if the particle type in the array at the input coordinates is "STATIC"
 *
 * @param grid The grid of particles
 * @param x The x-coordinate in the particle array
 * @param y The y-coordinate in the particle array
 * @return A boolean indicating if the particle is static
 */
bool is_pos_static(const grid_t *grid, int x, int y);

/**
 * Checks if the particle type in the array at the input coordinates is "SOLID"
 *
 * @param grid The grid of particles
 * @param x The x-coordinate in the particle array
 * @param y The y-coordinate in the particle array
 * @return A boolean indicating if the particle is solid
 */
bool is_pos_solid(const grid_t *grid, int x, int y);

/**
 * Checks if the particle type in the array at the input coordinates is "LIQUID"
 *
 * @param grid The grid of particles
 * @param x The x-coordinate in the particle array
 * @param y The y-coordinate in the particle array
 * @return A boolean indicating if the particle is liquid
 */
bool is_pos_liquid(const grid_t *grid, int x, int y);

/**
 * Checks if the particle type in the array at the input coordinates is "GAS"
 *
 * @param grid The grid of particles
 * @param x The x-coordinate in the particle array
 * @param y The y-coordinate in the particle array
 * @return A boolean indicating if the particle is gas
 */
bool is_pos_gas(const grid_t *grid, int x, int y);

/**
 * The update function for empty particles
 *
 * @param grid The grid of particles
 * @param x The x-coordinate in the particle array
 * @param y The y-coordinate in the particle array
 */
void update_empty(grid_t *grid, int x, int y);

/**
 * The update function for sand particles
 *
 * @param grid The grid of particles
 * @param x The x-coordinate in the particle array of the sand
 * @param y The y-coordinate in the particle array of the sand
 */
void update_sand(grid_t *grid, int x, int y);

/**
 * The update function for water particles
 *
 * @param grid The grid of particles
 * @param x The x-coordinate in the particle array of the water
 * @param y The y-coordinate in the particle array of the water
 */
void update_water(grid_t *grid, int x, int y);

/**
 * The update function for smoke particles
 *
 * @param grid The grid of particles
 * @param x The x-coordinate in the particle array of the smoke
 * @param y The y-coordinate in the particle array of the smoke
 */
void update_smoke(grid_t *grid, int x, int y);

/**
 * The update function for oil particles
 *
 * @param grid The grid of particles
 * @param x The x-coordinate in the particle array of the oil
 * @param y The y-coordinate in the particle array of the oil
 */
void update_oil(grid_t *grid, int x, int y);

/**
 * The update function for wall particles
 *
 * @param grid The grid of particles
 * @param x The x-coordinate in the particle array of the wall
 * @param y The y-coordinate in the particle array of the wall
 */
void update_wall(grid_t *grid, int x, int y);

/**
 * The update function for wood particles
 *
 * @param grid The grid of particles
 * @param x The x-coordinate in the particle array of the wood
 * @param y The y-coordinate in the particle array of the wood
 */
void update_wood(grid_t *grid, int x, int y);

/**
 * The update function for burning particles
 *
 * @param grid The grid of particles
 * @param x The x-coordinate in the particle array of the fire
 * @param y The y-coordinate in the particle array of the fire
 */
void update_fire(grid_t *grid, int x, int y);

/**
 * The update function for flame particles
 *
 * @param grid The grid of particles
 * @param x The x-coordinate in the particle array of the flame
 * @param y The y-coordinate in the particle array of the flame
 */
void update_flame(grid_t *grid, int x, int y);

/**
 * Sets the current drawing material to the next type
 *
 * @param m The current material
 * @return The new material type
 */
material_type next_material(material_type m);

/**
 * Sets the current drawing material to the previous type
 *
 * @param m The current material
 * @return The new material type
 */
material_type prev_material(material_type m);

/**
 * Gets the color of a material
 *
 * @param m The material
 * @return The color of the material
 */
Color get_color_from_mat(material_type m);

int 
main(void)
{
    const int grid_w = 256, grid_h = 256;
    const int scr_w = 256, scr_h = 320;
    int x = 0, y = 0, i = 0;
    int prev_pos[2] = {0, 0};
    int curr_pos[2] = {0, 0};
    material_type curr_mat = MAT_SAND;
    grid_t *grid = new_grid(grid_w, grid_h);
    particle_t *curr_particle = NULL;

    srand(time(NULL));

    InitWindow(scr_w, scr_h, "Falling Sand");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        curr_pos[0] = GetMouseX();
        curr_pos[1] = grid_h - 1 - GetMouseY();

        /*if (GetMouseWheelMoveV().y > 0) {*/
            /* Increase the drawing size */
        /*}*/
        /*else if (GetMouseWheelMoveV().y < 0) {*/
            /* Decrease the drawing size */
        /*}*/

        if (IsKeyPressed(KEY_RIGHT)) {
            curr_mat = next_material(curr_mat);
        }
        else if (IsKeyPressed(KEY_LEFT)) {
            curr_mat = prev_material(curr_mat);
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            particle_line(grid, prev_pos[0], prev_pos[1],
                                curr_pos[0], curr_pos[1], curr_mat);
        }
        else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            particle_line(grid, prev_pos[0], prev_pos[1],
                                curr_pos[0], curr_pos[1], MAT_EMPTY);
        }

        if (IsKeyPressed(KEY_C)) { clear_grid(grid); }

        BeginDrawing();
        {
            ClearBackground((Color){64, 64, 64, 255});

            /**
             * @note Two separate loops are used for grid updates. One for the
             * actual particle interactions and a second for drawing particles.
             * This is for simplicity because moving particles then drawing them
             * was hard logic that I'm too dumb to figure out.
             * Until I figure out how to update AND draw within the same loop
             * again, this will have to be two loops
             */
            for (y = 0; y < grid_h; y++) {
                for (x = 0; x < grid_w; x++) {
                    curr_particle = get_particle(grid, x, y);
                    if (curr_particle->has_been_updated) { continue; }
                    curr_particle->update_func(grid, x, y);
                }
            }

            for (y = 0; y < grid_h; y++) {
                for (x = 0; x < grid_w; x++) {
                    curr_particle = get_particle(grid, x, y);
                    curr_particle->has_been_updated = false;
                    DrawPixel(x, grid_w - 1 - y, curr_particle->color);
                }
            }

            /* UI drawing code */
            DrawRectangle(0, grid_h, scr_w, scr_h - grid_h, DARKBLUE);
            DrawFPS(4, grid_h);
            DrawRectangle(4, grid_h + 20, 40, 40, get_color_from_mat(curr_mat));

            for (i = 1; i < MAT_COUNT; i++) {
                DrawRectangle(30 + 20 * i, grid_h + 20, 15, 15,
                              get_color_from_mat(i));
            }
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
    grid_t *grid = malloc(sizeof(*grid));

    grid->width = width;
    grid->height = height;
    grid->arr = calloc(width * height, sizeof(*grid->arr));

    if (grid->arr == NULL) {
        fprintf(stderr, "Error: Could not allocate enough memory at %d in %s\n",
                __LINE__, __FILE__);
        exit(EXIT_FAILURE);
    }

    clear_grid(grid);

    return grid;
}

void
init_grid(grid_t *grid, int width, int height)
{
    grid->width = width;
    grid->height = height;
    grid->arr = calloc(width * height, sizeof(*grid->arr));

    if (grid->arr == NULL) {
        fprintf(stderr, "Error: Could not allocate enough memory at %d in %s\n",
                __LINE__, __FILE__);
        exit(EXIT_FAILURE);
    }

    clear_grid(grid);
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
    particle_t empty_particle = {
        MAT_EMPTY,
        ELEM_EMPTY,
        0.0f,
        (Vector2){0.0f, 0.0f},
        BLANK,
        false,
        update_empty
    };

    for (y = 0; y < grid->height; y++) {
        for (x = 0; x < grid->width; x++) {
            set_particle(grid, x, y, &empty_particle);
        }
    }
}

particle_t *
get_particle(const grid_t *grid, int x, int y)
{
    return &grid->arr[y * grid->width + x];
}

void
set_particle(grid_t *grid, int x, int y, particle_t *p)
{
    int index = y * grid->width + x;

    if (index < 0 || index > grid->width * grid->height) { return; }

    grid->arr[index].mat_type = p->mat_type;
    grid->arr[index].elem_type = p->elem_type;
    grid->arr[index].velocity = p->velocity;
    grid->arr[index].has_been_updated = p->has_been_updated;
    grid->arr[index].life_time = p->life_time;
    grid->arr[index].color = p->color;
    grid->arr[index].update_func = p->update_func;
}

material_type
get_particle_type(const particle_t *p)
{
    return p->mat_type;
}

material_type
get_particle_type_pos(const grid_t *grid, int x, int y)
{
    /* TODO: Fix this */
    if (x < 0 || x >= grid->width || y < 0 || y >= grid->height) {
        return MAT_EMPTY;
    }

    return get_particle(grid, x, y)->mat_type;
}

void
add_particle(grid_t *grid, int x, int y, material_type m)
{
    particle_t part;

    if (!is_pos_empty(grid, x, y)) { return; }

    part.mat_type = m;
    part.velocity = (Vector2){0.0f, 0.0f};
    part.has_been_updated = false;

    switch (m) {
        case MAT_SAND:
            part.elem_type = ELEM_SOLID;
            part.life_time = 0.0f;
            part.color = YELLOW;
            part.update_func = update_sand;
            break;
        case MAT_WATER:
            part.elem_type = ELEM_LIQUID;
            part.life_time = 0.0f;
            part.color = SKYBLUE;
            part.color.a = 128;
            part.update_func = update_water;
            break;
        case MAT_SMOKE:
            part.elem_type = ELEM_GAS;
            part.life_time = 3.0f;
            part.color = GRAY;
            part.update_func = update_smoke;
            break;
        case MAT_OIL:
            part.elem_type = ELEM_LIQUID;
            part.life_time = 3.0f;
            part.color = BLACK;
            part.update_func = update_oil;
            break;
        case MAT_WALL:
            part.elem_type = ELEM_STATIC;
            part.life_time = 0.0f;
            part.color = LIGHTGRAY;
            part.update_func = update_wall;
            break;
        case MAT_WOOD:
            part.elem_type = ELEM_STATIC;
            part.life_time = 7.5f;
            part.color = (Color){66, 27, 4, 255};
            part.update_func = update_wood;
            break;
        case MAT_FIRE:
            part.elem_type = ELEM_SOLID;
            part.life_time = 8.0f;
            part.color = RED;
            part.update_func = update_fire;
            break;
        case MAT_FLAME:
            part.elem_type = ELEM_GAS;
            part.life_time = 1.5f;
            part.color = ORANGE;
            part.update_func = update_flame;
            break;
        default:
            break;
    }

    set_particle(grid, x, y, &part);
}

void
remove_particle(grid_t *grid, int x, int y)
{
    particle_t empty_particle;

    if (is_pos_empty(grid, x,  y)) { return; }

    empty_particle.mat_type = MAT_EMPTY;
    empty_particle.elem_type = ELEM_EMPTY;
    empty_particle.life_time = 0.0f;
    empty_particle.velocity = (Vector2){0.0f, 0.0f};
    empty_particle.color = BLANK;
    empty_particle.has_been_updated = false;
    empty_particle.update_func = update_empty;

    set_particle(grid, x, y, &empty_particle);
}

void
swap_particles(grid_t *grid, int x1, int y1, int x2, int y2)
{
    int index1 = y1 * grid->width + x1;
    int index2 = y2 * grid->width + x2;

    particle_t temp = grid->arr[index1];
    grid->arr[index1] = grid->arr[index2];
    grid->arr[index2] = temp;

    grid->arr[index1].has_been_updated = true;
    grid->arr[index2].has_been_updated = true;
}

void
particle_line(grid_t *grid, int x1, int y1, int x2, int y2, material_type m)
{
    if (x1 > grid->width) { x1 = grid->width; }
    if (x2 > grid->width) { x2 = grid->width; }
    if (y1 > grid->height) { y1 = grid->height; }
    if (y2 > grid->height) { y2 = grid->height; }

    int dx = abs(x2 - x1);
    int sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1);
    int sy = y1 < y2 ? 1 : -1;
    int error = dx + dy;
    int e2;

    /** 
     * The if-else checks containing the while loops used to be within the
     * loops. They were moved out here as a little bit of optimization because
     * the check only needs to be done once to decide if a particle is getting
     * added or removed. Having it within the loop made it check ever iteration,
     * which was largely unnecessary. This does make the code uglier, but who
     * cares
     */
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
is_particle_empty(const particle_t *particle)
{
    return particle->mat_type == MAT_EMPTY;
}

bool 
is_particle_static(const particle_t *particle)
{
    return particle->elem_type == ELEM_STATIC;
}

bool 
is_particle_solid(const particle_t *particle)
{
    return particle->elem_type == ELEM_SOLID;
}

bool 
is_particle_liquid(const particle_t *particle)
{
    return particle->elem_type == ELEM_LIQUID;
}

bool 
is_particle_gas(const particle_t *particle)
{
    return particle->elem_type == ELEM_GAS;
}

bool
is_pos_empty(const grid_t *grid, int x, int y)
{
    if (x < 0 || x >= grid->width || y < 0 || y >= grid->height) {
        return false;
    }

    return is_particle_empty(get_particle(grid, x, y));
}

bool 
is_pos_static(const grid_t *grid, int x, int y)
{
    if (x < 0 || x >= grid->width || y < 0 || y >= grid->height) {
        return false;
    }

    return is_particle_static(get_particle(grid, x, y));
}

bool 
is_pos_solid(const grid_t *grid, int x, int y)
{
    if (x < 0 || x >= grid->width || y < 0 || y >= grid->height) {
        return false;
    }

    return is_particle_solid(get_particle(grid, x, y));
}

bool 
is_pos_liquid(const grid_t *grid, int x, int y)
{
    if (x < 0 || x >= grid->width || y < 0 || y >= grid->height) {
        return false;
    }

    return is_particle_liquid(get_particle(grid, x, y));
}

bool 
is_pos_gas(const grid_t *grid, int x, int y)
{
    if (x < 0 || x >= grid->width || y < 0 || y >= grid->height) {
        return false;
    }

    return is_particle_gas(get_particle(grid, x, y));
}

void update_empty(grid_t *grid, int x, int y)
{
    get_particle(grid, x, y)->has_been_updated = true;
}

void
update_sand(grid_t *grid, int x, int y)
{
    int below = y - 1;
    int left  = x - 1, right = x + 1;
    particle_t *curr_particle = get_particle(grid, x, y);

    if (y == 0) {
        curr_particle->has_been_updated = true;
        return;
    }
    
    if (is_pos_empty(grid, x, below)
        || is_pos_liquid(grid, x, below)
        || is_pos_gas(grid, x, below)) {
        swap_particles(grid, x, y, x, below);
    }
    else if ((is_pos_empty(grid, left, below)
             || is_pos_liquid(grid, left, below)
             || is_pos_gas(grid, left, below))
             && !is_pos_static(grid, x, below)) {
        swap_particles(grid, x, y, left, below);
    }
    else if ((is_pos_empty(grid, right, below)
             || is_pos_liquid(grid, right, below)
             || is_pos_gas(grid, right, below))
             && !is_pos_static(grid, x, below)) {
        swap_particles(grid, x, y, right, below);
    }

    curr_particle->has_been_updated = true;
}

void 
update_water(grid_t *grid, int x, int y)
{
    int below = y - 1;
    int left  = x - 1, right = x + 1;
    particle_t *curr_particle = get_particle(grid, x, y);

    if (is_pos_empty(grid, x, below)
        || is_pos_gas(grid, x, below)
        || get_particle_type_pos(grid, x, below) == MAT_OIL) {
        swap_particles(grid, x, y, x, below);
    }
    else if ((is_pos_empty(grid, left, below)
             || is_pos_gas(grid, left, below)
             || get_particle_type_pos(grid, left, below) == MAT_OIL)
             && !is_pos_static(grid, x, below)) {
        swap_particles(grid, x, y, left, below);
    }
    else if ((is_pos_empty(grid, right, below)
             || is_pos_gas(grid, right, below)
             || get_particle_type_pos(grid, right, below) == MAT_OIL)
             && !is_pos_static(grid, x, below)) {
        swap_particles(grid, x, y, right, below);
    }
    else if (is_pos_empty(grid, left, y)
             || is_pos_gas(grid, left, y)
             || get_particle_type_pos(grid, left, y) == MAT_OIL) {
        swap_particles(grid, x, y, left, y);
    }
    else if (is_pos_empty(grid, right, y)
             || is_pos_gas(grid, right, y)
             || get_particle_type_pos(grid, right, y) == MAT_OIL) {
        swap_particles(grid, x, y, right, y);
    }

    curr_particle->has_been_updated = true;
}

void
update_smoke(grid_t *grid, int x, int y)
{
    int above = y + 1;
    int left = x - 1, right = x + 1;
    particle_t *curr_particle = get_particle(grid, x, y);

    if (y == grid->height) {
        curr_particle->has_been_updated = true;
        return;
    }

    curr_particle->life_time -= (float)rand() / (float)(RAND_MAX / 0.1f);

    if (curr_particle->life_time <= 0.0f) {
        remove_particle(grid, x, y);
        curr_particle->has_been_updated = true;
        return;
    }

    if (is_pos_empty(grid, x, above)) {
        swap_particles(grid, x, y, x, above);
    }
    else if (is_pos_empty(grid, left, above)
             && !is_pos_static(grid, x, above)) {
        swap_particles(grid, x, y, left, above);
    }
    else if (is_pos_empty(grid, right, above)
             && !is_pos_static(grid, x, above)) {
        swap_particles(grid, x, y, right, above);
    }
    else if (is_pos_empty(grid, left, y)) {
        swap_particles(grid, x, y, left, y);
    }
    else if (is_pos_empty(grid, right, y)) {
        swap_particles(grid, x, y, right, y);
    }

    curr_particle->has_been_updated = true;
}

void
update_oil(grid_t *grid, int x, int y)
{
    int r = 0;
    int above = y + 1, below = y - 1;
    int left = x - 1, right = x + 1;
    particle_t *curr_particle = get_particle(grid, x, y);
    particle_t *temp_particle = NULL;
    Vector2 temp_vel = curr_particle->velocity;

    /**
     * There are 8 checks of the particles around the oil to see if they are
     * flames/fire. There might be some clean way to do this, but I don't know
     * it, so for now, ugliness.
     *
     * Anyway, this code basically just gives the oil a chance to ignite if it's
     * near a flame or something burning. If the oil ignites, it adds a "fire"
     * particle and gives it the oil's propereties (liquid element and velocity)
     */
    temp_particle = get_particle(grid, left, below);
    if (get_particle_type(temp_particle) == MAT_FLAME
        || get_particle_type(temp_particle) == MAT_FIRE) {
        /* 75% chance to ignite */
        r = rand() % 4;
        if (r != 0) {
            remove_particle(grid, x, y);
            add_particle(grid, x,  y, MAT_FIRE);
            curr_particle->elem_type = ELEM_LIQUID;
            curr_particle->velocity = temp_vel;
        }
    }

    temp_particle = get_particle(grid, x, below);
    if (get_particle_type(temp_particle) == MAT_FLAME
        || get_particle_type(temp_particle) == MAT_FIRE) {
        /* 75% chance to ignite */
        r = rand() % 4;
        if (r != 0) {
            remove_particle(grid, x, y);
            add_particle(grid, x,  y, MAT_FIRE);
            curr_particle->elem_type = ELEM_LIQUID;
            curr_particle->velocity = temp_vel;
        }
    }

    temp_particle = get_particle(grid, right, below);
    if (get_particle_type(temp_particle) == MAT_FLAME
        || get_particle_type(temp_particle) == MAT_FIRE) {
        /* 75% chance to ignite */
        r = rand() % 4;
        if (r != 0) {
            remove_particle(grid, x, y);
            add_particle(grid, x,  y, MAT_FIRE);
            curr_particle->elem_type = ELEM_LIQUID;
            curr_particle->velocity = temp_vel;
        }
    }

    temp_particle = get_particle(grid, left, y);
    if (get_particle_type(temp_particle) == MAT_FLAME
        || get_particle_type(temp_particle) == MAT_FIRE) {
        /* 75% chance to ignite */
        r = rand() % 4;
        if (r != 0) {
            remove_particle(grid, x, y);
            add_particle(grid, x,  y, MAT_FIRE);
            curr_particle->elem_type = ELEM_LIQUID;
            curr_particle->velocity = temp_vel;
        }
    }

    temp_particle = get_particle(grid, right, y);
    if (get_particle_type(temp_particle) == MAT_FLAME
        || get_particle_type(temp_particle) == MAT_FIRE) {
        /* 75% chance to ignite */
        r = rand() % 4;
        if (r != 0) {
            remove_particle(grid, x, y);
            add_particle(grid, x,  y, MAT_FIRE);
            curr_particle->elem_type = ELEM_LIQUID;
            curr_particle->velocity = temp_vel;
        }
    }

    temp_particle = get_particle(grid, left, above);
    if (get_particle_type(temp_particle) == MAT_FLAME
        || get_particle_type(temp_particle) == MAT_FIRE) {
        /* 75% chance to ignite */
        r = rand() % 4;
        if (r != 0) {
            remove_particle(grid, x, y);
            add_particle(grid, x,  y, MAT_FIRE);
            curr_particle->elem_type = ELEM_LIQUID;
            curr_particle->velocity = temp_vel;
        }
    }

    temp_particle = get_particle(grid, x, above);
    if (get_particle_type(temp_particle) == MAT_FLAME
        || get_particle_type(temp_particle) == MAT_FIRE) {
        /* 75% chance to ignite */
        r = rand() % 4;
        if (r != 0) {
            remove_particle(grid, x, y);
            add_particle(grid, x,  y, MAT_FIRE);
            curr_particle->elem_type = ELEM_LIQUID;
            curr_particle->velocity = temp_vel;
        }
    }

    temp_particle = get_particle(grid, right, above);
    if (get_particle_type(temp_particle) == MAT_FLAME
        || get_particle_type(temp_particle) == MAT_FIRE) {
        /* 75% chance to ignite */
        r = rand() % 4;
        if (r != 0) {
            remove_particle(grid, x, y);
            add_particle(grid, x,  y, MAT_FIRE);
            curr_particle->elem_type = ELEM_LIQUID;
            curr_particle->velocity = temp_vel;
        }
    }

    /* Moves the oil like a regular liquid */
    if (is_pos_empty(grid, x, below)) {
        swap_particles(grid, x, y, x, below);
    }
    else if ((is_pos_empty(grid, left, below)
             || is_pos_gas(grid, left, below))
             && !is_pos_static(grid, x, below)) {
        swap_particles(grid, x, y, left, below);
    }
    else if ((is_pos_empty(grid, right, below)
             || is_pos_gas(grid, right, below))
             && !is_pos_static(grid, x, below)) {
        swap_particles(grid, x, y, right, below);
    }
    else if (is_pos_empty(grid, left, y)) {
        swap_particles(grid, x, y, left, y);
    }
    else if (is_pos_empty(grid, right, y)) {
        swap_particles(grid, x, y, right, y);
    }

    curr_particle->has_been_updated = true;
}

void
update_wall(grid_t *grid, int x, int y)
{
    get_particle(grid, x, y)->has_been_updated = true;
}

void
update_wood(grid_t *grid, int x, int y)
{
    int r;
    int above = y + 1, below = y - 1;
    int  left = x - 1, right = x + 1;
    particle_t *curr_particle = get_particle(grid, x, y);
    particle_t *temp_particle = NULL;
    Vector2 temp_vel = curr_particle->velocity;

    temp_particle = get_particle(grid, left, below);
    if (get_particle_type(temp_particle) == MAT_FLAME
        || get_particle_type(temp_particle) == MAT_FIRE) {
        /* 50% chance to ignite */
        r = rand() % 2;
        if (r == 0) {
            remove_particle(grid, x, y);
            add_particle(grid, x,  y, MAT_FIRE);
            curr_particle->elem_type = ELEM_STATIC;
            curr_particle->velocity = temp_vel;
        }
    }

    temp_particle = get_particle(grid, x, below);
    if (get_particle_type(temp_particle) == MAT_FLAME
        || get_particle_type(temp_particle) == MAT_FIRE) {
        /* 50% chance to ignite */
        r = rand() % 2;
        if (r == 0) {
            remove_particle(grid, x, y);
            add_particle(grid, x,  y, MAT_FIRE);
            curr_particle->elem_type = ELEM_STATIC;
            curr_particle->velocity = temp_vel;
        }
    }

    temp_particle = get_particle(grid, right, below);
    if (get_particle_type(temp_particle) == MAT_FLAME
        || get_particle_type(temp_particle) == MAT_FIRE) {
        /* 50% chance to ignite */
        r = rand() % 2;
        if (r == 0) {
            remove_particle(grid, x, y);
            add_particle(grid, x,  y, MAT_FIRE);
            curr_particle->elem_type = ELEM_STATIC;
            curr_particle->velocity = temp_vel;
        }
    }

    temp_particle = get_particle(grid, left, y);
    if (get_particle_type(temp_particle) == MAT_FLAME
        || get_particle_type(temp_particle) == MAT_FIRE) {
        /* 50% chance to ignite */
        r = rand() % 2;
        if (r == 0) {
            remove_particle(grid, x, y);
            add_particle(grid, x,  y, MAT_FIRE);
            curr_particle->elem_type = ELEM_STATIC;
            curr_particle->velocity = temp_vel;
        }
    }

    temp_particle = get_particle(grid, right, y);
    if (get_particle_type(temp_particle) == MAT_FLAME
        || get_particle_type(temp_particle) == MAT_FIRE) {
        /* 50% chance to ignite */
        r = rand() % 2;
        if (r == 0) {
            remove_particle(grid, x, y);
            add_particle(grid, x,  y, MAT_FIRE);
            curr_particle->elem_type = ELEM_STATIC;
            curr_particle->velocity = temp_vel;
        }
    }

    temp_particle = get_particle(grid, left, above);
    if (get_particle_type(temp_particle) == MAT_FLAME
        || get_particle_type(temp_particle) == MAT_FIRE) {
        /* 50% chance to ignite */
        r = rand() % 2;
        if (r == 0) {
            remove_particle(grid, x, y);
            add_particle(grid, x,  y, MAT_FIRE);
            curr_particle->elem_type = ELEM_STATIC;
            curr_particle->velocity = temp_vel;
        }
    }

    temp_particle = get_particle(grid, x, above);
    if (get_particle_type(temp_particle) == MAT_FLAME
        || get_particle_type(temp_particle) == MAT_FIRE) {
        /* 50% chance to ignite */
        r = rand() % 2;
        if (r == 0) {
            remove_particle(grid, x, y);
            add_particle(grid, x,  y, MAT_FIRE);
            curr_particle->elem_type = ELEM_STATIC;
            curr_particle->velocity = temp_vel;
        }
    }

    temp_particle = get_particle(grid, right, above);
    if (get_particle_type(temp_particle) == MAT_FLAME
        || get_particle_type(temp_particle) == MAT_FIRE) {
        /* 50% chance to ignite */
        r = rand() % 2;
        if (r == 0) {
            remove_particle(grid, x, y);
            add_particle(grid, x,  y, MAT_FIRE);
            curr_particle->elem_type = ELEM_STATIC;
            curr_particle->velocity = temp_vel;
        }
    }

    curr_particle->has_been_updated = true;
}

void
update_fire(grid_t *grid, int x, int y)
{
    int r = rand() % 4;
    particle_t *curr_particle = get_particle(grid, x, y);

    switch (r) {
        case 0:
            curr_particle->color = (Color){255, 0, 0, 255};
            break;
        case 1:
            curr_particle->color = (Color){192, 0, 0, 255};
            break;
        case 2:
            curr_particle->color = (Color){160, 0, 0, 255};
            break;
        case 3:
            curr_particle->color = (Color){64, 0, 0, 255};
            break;
        default:
            break;
    }

    curr_particle->life_time -= (float)rand() / (float)(RAND_MAX / 0.15f);

    if (curr_particle->life_time <= 0.0f) {
        remove_particle(grid, x, y);
        if (rand() % 5 == 0) { add_particle(grid, x, y, MAT_SMOKE); }
        curr_particle->has_been_updated = true;
        return;
    }
}

void
update_flame(grid_t *grid, int x, int y)
{
    int above = y + 1;
    int left = x - 1, right = x + 1;
    particle_t *curr_particle = get_particle(grid, x, y);

    curr_particle->life_time -= (float)rand() / (float)(RAND_MAX / 0.25f);

    if (curr_particle->life_time <= 0.0f) {
        remove_particle(grid, x, y);
        curr_particle->has_been_updated = true;
        return;
    }

    if (is_pos_empty(grid, x, above)) {
        swap_particles(grid, x, y, x, above);
    }
    else if (is_pos_empty(grid, left, above)
             && !is_pos_static(grid, x, above)) {
        swap_particles(grid, x, y, left, above);
    }
    else if (is_pos_empty(grid, right, above)
             && !is_pos_static(grid, x, above)) {
        swap_particles(grid, x, y, right, above);
    }
    else if (is_pos_empty(grid, left, y)) {
        swap_particles(grid, x, y, left, y);
    }
    else if (is_pos_empty(grid, right, y)) {
        swap_particles(grid, x, y, right, y);
    }

    curr_particle->has_been_updated = true;
}

material_type 
next_material(material_type m)
{
    if (m == MAT_COUNT - 1) { m = 0; }

    return m + 1;
}

material_type 
prev_material(material_type m)
{
    if (m == 1) { m = MAT_COUNT; }

    return m - 1;
}

Color
get_color_from_mat(material_type m)
{
    switch (m) {
        case MAT_SAND:  return YELLOW;
        case MAT_WATER: return SKYBLUE;
        case MAT_SMOKE: return GRAY;
        case MAT_OIL:   return BLACK;
        case MAT_WALL:  return LIGHTGRAY;
        case MAT_WOOD:  return (Color){66, 27, 4, 255};
        case MAT_FIRE:  return RED;
        case MAT_FLAME: return ORANGE;
        default:        return BLANK;
    }
}
/* EOF */
