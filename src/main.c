#include "../include/cub3d.h" // Include the main header for the project
#include <X11/keysym.h>  // For XK_ Key Symbols (like XK_Escape, XK_w)
#include <X11/X.h>       // For Event Masks (KeyPressMask, StructureNotifyMask)
#include <string.h>      // For memset, strdup (if used in init) // Use ft_memset, ft_strdup if from libft
#include <stdio.h>       // For printf, perror
#include <stdlib.h>      // For exit, malloc, free
#include <math.h>        // For M_PI, cos, sin, fmod, sqrt, pow, tan (already in cub3d.h)
#include <limits.h>      // For INT_MAX (already in cub3d.h)
#include <stdbool.h>     // For bool, true, false
#include <sys/time.h>    // For gettimeofday
#include <unistd.h>      // For usleep

// -------- Demo-specific Constants --------
#define MAP_SCALE 10       // How many pixels per TILE_SIZE on the minimap
#define NUM_RAYS WINDOW_WIDTH // Number of rays to cast for the demo
#define PLAYER_FOV (M_PI / 3.0) // Player Field of View (60 degrees)

// -------- Optimization & Rendering Constants --------
#define FRAME_RATE_CAP 60       // Cap FPS to avoid excessive CPU usage
#define MAX_VISIBLE_DISTANCE (15.0 * TILE_SIZE) // Max distance for rendering walls clearly
#define MINIMAP_RAY_STEP 8     // Draw every Nth ray on the minimap for performance

// -------- Colors (Demo specific) --------
#define C_BLACK 0x000000
#define C_WHITE 0xFFFFFF
#define C_RED 0xFF0000
#define C_GREEN 0x00FF00      // Example: North/South walls
#define C_BLUE 0x0000FF       // Example: East/West walls
#define C_YELLOW 0xFFFF00
#define C_GRAY 0x808080
#define C_DARK_GRAY 0x404040
#define C_CEILING 0x303060    // Example ceiling color
#define C_FLOOR 0x604040      // Example floor color

// --- Additional data structures for optimization ---
typedef struct s_ray_hit {
    double distance;         // Perpendicular distance to wall
    t_fpoint hit_point;      // Where the ray hit
    bool is_vertical;        // Was hit on vertical wall?
    int map_x, map_y;        // Map coordinates of hit (redundant if hit_point is accurate)
    double ray_angle;        // Store the angle for potential texturing later
} t_ray_hit;

// --- Forward declarations for functions defined in this file ---
void    init_params(t_params *params);
int     game_loop(t_params *params);
int     key_press_hook(int keycode, t_params *params);
int     close_window_hook(t_params *params);
void    draw_map(t_params *params);
void    draw_player(t_params *params);
void    cast_rays(t_params *params, t_ray_hit *ray_hits);
void    draw_rays_minimap(t_params *params, t_ray_hit *ray_hits);
double  normalize_angle(double angle);
void    clear_image_direct(t_params *params, int color); // Renamed for clarity
int     is_wall_at(t_params *params, double x, double y);
void    cleanup(t_params *params);
void    render_3d_view(t_params *params, t_ray_hit *ray_hits);
void    draw_vertical_slice_direct(t_params *params, int x, int y_start, int y_end, int color, double distance); // Added distance for shading
long    get_time_ms(void);
void    frame_rate_control(long *last_time, int target_fps);
int     apply_shading(int color, double distance);

// --- Forward declarations for functions assumed defined elsewhere ---
// Replace with your actual libft/helper function names if different
void        *ft_memset(void *b, int c, size_t len);
char        *ft_strdup(const char *s1);
size_t      ft_strlen(const char *s);
int         put_pixel(t_params *params, int x, int y, int color); // Keep for minimap? Or replace too?
void        draw_line_img(t_params *params, t_point p1, t_point p2, int color); // Used for minimap
double      calculate_euclidean_distance(double x1, double y1, double x2, double y2);
t_fpoint    find_horizontal_wall_intersection(t_params *params, double ray_angle);
t_fpoint    find_vertical_wall_intersection(t_params *params, double ray_angle);

// --- Optimized Drawing & Helper Functions ---

// Writes a pixel directly to the image buffer
// Assumes 32 bits per pixel (4 bytes), adjust if needed
static inline void put_pixel_direct(t_img *img, int x, int y, int color)
{
    char    *dst;

    if (x >= 0 && x < img->width && y >= 0 && y < img->height)
    {
        dst = img->addr + (y * img->line_length + x * (img->bits_per_pixel / 8));
        *(unsigned int*)dst = color;
    }
}

// Faster image clearing using direct buffer access and memset
void clear_image_direct(t_params *params, int color)
{
    int     y;
    char    *line_start;
    int     pixel_bytes = params->window_img.bits_per_pixel / 8;

    // Optimization: If color components are identical (e.g., black, white, gray)
    // and architecture allows, we might use memset more broadly.
    // But a line-by-line fill is generally safe and fast.
    if (pixel_bytes == 4) // Most common case (32bpp)
    {
        line_start = params->window_img.addr;
        for (y = 0; y < params->window_img.height; y++)
        {
            // Fill one line with the color
            // This is faster than calling put_pixel repeatedly
            for (int x = 0; x < params->window_img.width; ++x) {
                 *(unsigned int*)(line_start + x * pixel_bytes) = color;
            }
            line_start += params->window_img.line_length;
        }
    }
    else // Fallback for other BPP (less common for mlx)
    {
         for (y = 0; y < params->window_img.height; y++) {
            for (int x = 0; x < params->window_img.width; x++) {
                put_pixel_direct(&params->window_img, x, y, color);
            }
        }
    }
}


// Angle normalization function (unchanged)
double normalize_angle(double angle)
{
    angle = fmod(angle, 2.0 * M_PI);
    if (angle < 0)
        angle += (2.0 * M_PI);
    return angle;
}

// Wall check function (unchanged, but ensure TILE_SIZE is correct)
int is_wall_at(t_params *params, double x, double y)
{
    int map_x, map_y;

    // Check bounds first
    if (x < 0 || y < 0 || x >= params->map.cols * TILE_SIZE || y >= params->map.rows * TILE_SIZE)
        return 1; // Out of bounds is considered a wall

    map_x = (int)(x / TILE_SIZE);
    map_y = (int)(y / TILE_SIZE);

    // Check array bounds again after division (important!)
    if (map_y < 0 || map_y >= params->map.rows || map_x < 0 || map_x >= params->map.cols)
        return 1; // Should not happen if first check passed, but safety first

    return (params->map.map_data[map_y][map_x] == WALL);
}

// Get current time in milliseconds (unchanged)
long get_time_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

// Control frame rate (unchanged)
void frame_rate_control(long *last_time, int target_fps)
{
    long current_time, elapsed, delay;
    if (target_fps <= 0) return;
    current_time = get_time_ms();
    elapsed = current_time - *last_time;
    delay = (1000 / target_fps) - elapsed;
    if (delay > 0)
        usleep(delay * 1000); // usleep takes microseconds
    *last_time = get_time_ms();
}

// Apply distance shading (fog effect)
int apply_shading(int color, double distance)
{
    double brightness;
    int r, g, b;

    if (distance <= 0) return color; // Avoid division by zero or invalid distances

    // Calculate brightness factor (linear falloff)
    // Clamped between 0.0 (fully fogged) and 1.0 (full brightness)
    brightness = 1.0 - (distance / MAX_VISIBLE_DISTANCE);
    if (brightness < 0.0) brightness = 0.0;
    if (brightness > 1.0) brightness = 1.0; // Should not happen if distance > 0

    // Extract RGB components
    r = (color >> 16) & 0xFF;
    g = (color >> 8) & 0xFF;
    b = color & 0xFF;

    // Apply brightness
    r = (int)(r * brightness);
    g = (int)(g * brightness);
    b = (int)(b * brightness);

    // Clamp components (just in case, although brightness <= 1.0)
    r = (r > 255) ? 255 : (r < 0 ? 0 : r);
    g = (g > 255) ? 255 : (g < 0 ? 0 : g);
    b = (b > 255) ? 255 : (b < 0 ? 0 : b);

    // Reassemble color
    return (r << 16) | (g << 8) | b;
}


// --- Drawing Functions ---

// Draw minimap (optimized bounds check)
void draw_map(t_params *params)
{
    int x, y, tile_x, tile_y, color;
    int draw_x_base, draw_y_base;
    int map_pixel_width = params->map.cols * MAP_SCALE;
    int map_pixel_height = params->map.rows * MAP_SCALE;

    // Determine max draw coords based on window size or map size
    int max_draw_x = (map_pixel_width < params->window_img.width) ? map_pixel_width : params->window_img.width;
    int max_draw_y = (map_pixel_height < params->window_img.height) ? map_pixel_height : params->window_img.height;


    for (y = 0; y < params->map.rows; y++) {
        draw_y_base = y * MAP_SCALE;
        // Optimization: Skip row if entirely off-screen
        if (draw_y_base >= max_draw_y) break;

        for (x = 0; x < params->map.cols; x++) {
            draw_x_base = x * MAP_SCALE;
            // Optimization: Skip column if entirely off-screen
             if (draw_x_base >= max_draw_x) break; // Assumes drawing left-to-right

            color = (params->map.map_data[y][x] == WALL) ? C_GRAY : C_DARK_GRAY;

            // Draw the tile using direct pixel access for potential speedup
            for (tile_y = 0; tile_y < MAP_SCALE -1 ; tile_y++) { // -1 for grid lines
                int py = draw_y_base + tile_y;
                 if (py >= max_draw_y) break; // Stop drawing if row goes off bottom

                for (tile_x = 0; tile_x < MAP_SCALE -1 ; tile_x++) { // -1 for grid lines
                    int px = draw_x_base + tile_x;
                    if (px >= max_draw_x) break; // Stop drawing if column goes off right

                    // Use direct put pixel here if you replace put_pixel everywhere
                    // put_pixel(params, px, py, color);
                    put_pixel_direct(&params->window_img, px, py, color);
                }
            }
        }
    }
}

// Draw player on minimap (using direct pixel access)
void draw_player(t_params *params)
{
    int player_marker_size = 4;
    int player_screen_x = (int)(params->player.x / TILE_SIZE * MAP_SCALE);
    int player_screen_y = (int)(params->player.y / TILE_SIZE * MAP_SCALE);
    int i, j;
    int px, py;

    // Draw player marker
    for (i = -player_marker_size / 2; i <= player_marker_size / 2; i++) {
        for (j = -player_marker_size / 2; j <= player_marker_size / 2; j++) {
            px = player_screen_x + i;
            py = player_screen_y + j;
            // Use direct put pixel
            put_pixel_direct(&params->window_img, px, py, C_RED);
        }
    }

    // Draw direction line using your existing line function
    // If draw_line_img also uses put_pixel, consider optimizing it too
    t_point p1 = {player_screen_x, player_screen_y};
    t_point p2 = {
        player_screen_x + (int)(cos(params->player.direction) * MAP_SCALE * 1.5), // Make line longer
        player_screen_y + (int)(sin(params->player.direction) * MAP_SCALE * 1.5)
    };
    draw_line_img(params, p1, p2, C_RED); // Assumes draw_line_img handles clipping
}


// Consolidated ray casting function (minor optimization on distance calc)
void cast_rays(t_params *params, t_ray_hit *ray_hits)
{
    double ray_angle;
    double angle_step;
    int i;
    t_fpoint h_intersect, v_intersect;
    double h_dist_sq, v_dist_sq; // Use squared distances for comparison

    angle_step = PLAYER_FOV / (double)NUM_RAYS;
    ray_angle = params->player.direction - (PLAYER_FOV / 2.0);

    for (i = 0; i < NUM_RAYS; i++) {
        ray_angle = normalize_angle(ray_angle);

        h_intersect = find_horizontal_wall_intersection(params, ray_angle);
        v_intersect = find_vertical_wall_intersection(params, ray_angle);

        // Calculate squared distances (avoid sqrt for comparison)
        h_dist_sq = (h_intersect.x != INT_MAX) ?
            pow(h_intersect.x - params->player.x, 2) + pow(h_intersect.y - params->player.y, 2) :
            __DBL_MAX__; // Use max double instead of INT_MAX for squared comparison

        v_dist_sq = (v_intersect.x != INT_MAX) ?
            pow(v_intersect.x - params->player.x, 2) + pow(v_intersect.y - params->player.y, 2) :
            __DBL_MAX__;

        if (h_dist_sq < v_dist_sq) {
            ray_hits[i].distance = sqrt(h_dist_sq); // Calculate actual distance now
            ray_hits[i].hit_point = h_intersect;
            ray_hits[i].is_vertical = false;
        } else {
            ray_hits[i].distance = sqrt(v_dist_sq); // Calculate actual distance now
            ray_hits[i].hit_point = v_intersect;
            ray_hits[i].is_vertical = true;
        }

         // Calculate perpendicular distance (corrects fisheye)
        ray_hits[i].distance *= cos(ray_angle - params->player.direction);
        ray_hits[i].ray_angle = ray_angle; // Store angle

        // Assign map coords (can be useful for texturing or wall types)
        // Ensure division occurs *after* potential offset for vertical hits
        if (ray_hits[i].distance < __DBL_MAX__) {
             // Small offset inwards to prevent hitting adjacent tile due to precision
            double check_x = ray_hits[i].hit_point.x;
            double check_y = ray_hits[i].hit_point.y;
             if(ray_hits[i].is_vertical) { // Vertical wall hit
                // Check slightly inside the wall based on ray direction
                 check_x += (cos(ray_angle) > 0 ? -0.01 : 0.01);
            } else { // Horizontal wall hit
                 // Check slightly inside the wall based on ray direction
                 check_y += (sin(ray_angle) > 0 ? -0.01 : 0.01);
            }
            ray_hits[i].map_x = (int)(check_x / TILE_SIZE);
            ray_hits[i].map_y = (int)(check_y / TILE_SIZE);
        } else {
             ray_hits[i].map_x = -1; // Indicate no hit
             ray_hits[i].map_y = -1;
        }


        ray_angle += angle_step;
    }
}

// Draw rays on minimap (unchanged logic, uses draw_line_img)
void draw_rays_minimap(t_params *params, t_ray_hit *ray_hits)
{
    int i;
    t_point p1, p2;

    p1.x = (int)(params->player.x / TILE_SIZE * MAP_SCALE);
    p1.y = (int)(params->player.y / TILE_SIZE * MAP_SCALE);

    for (i = 0; i < NUM_RAYS; i += MINIMAP_RAY_STEP) { // Use constant
        if (ray_hits[i].distance < MAX_VISIBLE_DISTANCE && ray_hits[i].distance > 0.01) {
            p2.x = (int)(ray_hits[i].hit_point.x / TILE_SIZE * MAP_SCALE);
            p2.y = (int)(ray_hits[i].hit_point.y / TILE_SIZE * MAP_SCALE);
            draw_line_img(params, p1, p2, C_YELLOW);
        }
    }
}

// Draw a vertical slice using direct buffer access and shading
void draw_vertical_slice_direct(t_params *params, int x, int y_start, int y_end, int base_color, double distance)
{
    int y;
    int shaded_color;
    char *pixel_addr;
    int pixel_bytes = params->window_img.bits_per_pixel / 8;

    // Clamp coordinates to screen bounds
    if (x < 0 || x >= params->window_img.width) return;
    int clamped_y_start = (y_start < 0) ? 0 : y_start;
    int clamped_y_end = (y_end >= params->window_img.height) ? params->window_img.height - 1 : y_end;

    // Apply distance shading
    shaded_color = apply_shading(base_color, distance);

    // Get starting pixel address for the column
    pixel_addr = params->window_img.addr + (clamped_y_start * params->window_img.line_length) + (x * pixel_bytes);

    // Draw the vertical line directly into the buffer
    for (y = clamped_y_start; y <= clamped_y_end; y++) {
        // Handle different BPP if necessary, assuming 32bpp (4 bytes)
        *(unsigned int*)pixel_addr = shaded_color;
        pixel_addr += params->window_img.line_length; // Move down one row
    }
}


// Render 3D view using direct buffer access and improved shading
void render_3d_view(t_params *params, t_ray_hit *ray_hits)
{
    int i;
    double slice_height; // Use double for precision before casting
    int draw_start, draw_end;
    int wall_color;

    for (i = 0; i < NUM_RAYS; i++) {
        // Use perpendicular distance for height calculation
        double perp_distance = ray_hits[i].distance;

        if (perp_distance < MAX_VISIBLE_DISTANCE && perp_distance > 0.01) // Check against max visibility
        {
            // Calculate projected wall height (perspective projection)
            // TILE_SIZE acts as the reference height in world units
            slice_height = (TILE_SIZE / perp_distance) * params->dist_proj_plane; // Use projection plane distance

            // Calculate drawing bounds on screen
            // Center the wall slice vertically
            draw_start = (params->window_img.height / 2) - ((int)slice_height / 2);
            draw_end = draw_start + (int)slice_height;

            // Choose base wall color (example: based on orientation)
            wall_color = ray_hits[i].is_vertical ? C_GREEN : C_BLUE;
            // Could also use ray_hits[i].map_x/map_y to get wall type from map

            // Draw ceiling (from top to wall start)
            draw_vertical_slice_direct(params, i, 0, draw_start - 1, C_CEILING, MAX_VISIBLE_DISTANCE); // Ceiling is always "far"
            // Draw wall slice (with distance shading)
            draw_vertical_slice_direct(params, i, draw_start, draw_end, wall_color, perp_distance);
            // Draw floor (from wall end to bottom)
             draw_vertical_slice_direct(params, i, draw_end + 1, params->window_img.height - 1, C_FLOOR, MAX_VISIBLE_DISTANCE); // Floor is always "far"

        }
        else // Ray hit nothing within visible distance or hit nothing at all
        {
            // Draw ceiling and floor covering the whole column
             draw_vertical_slice_direct(params, i, 0, params->window_img.height / 2 - 1, C_CEILING, MAX_VISIBLE_DISTANCE);
             draw_vertical_slice_direct(params, i, params->window_img.height / 2, params->window_img.height - 1, C_FLOOR, MAX_VISIBLE_DISTANCE);
        }
    }
}

// --- Game Logic and Hooks ---

int game_loop(t_params *params)
{
    // Static to avoid stack allocation each frame (minor optimization)
    static t_ray_hit ray_hits[NUM_RAYS];
    static long last_frame_time = 0;

    // --- Update Game State (currently only done in key_press_hook) ---
    // E.g., animations, enemy movement would go here

    // --- Rendering ---
    // 1. Clear the buffer (optimized)
    clear_image_direct(params, C_BLACK); // Or C_CEILING for top half, C_FLOOR for bottom?

    // 2. Cast rays to get wall intersection data
    cast_rays(params, ray_hits);

    // 3. Render the 3D perspective view (optimized)
    render_3d_view(params, ray_hits);

    // 4. Draw the 2D Minimap overlay (optional, draws on top)
    #ifdef DRAW_MINIMAP // Compile-time toggle for minimap to do that compile with -D DRAW_MINIMAP
    draw_map(params);
    draw_rays_minimap(params, ray_hits);
    draw_player(params);
    #endif // DRAW_MINIMAP

    // 5. Put the final image to the window
    mlx_put_image_to_window(params->mlx, params->win, params->window_img.img, 0, 0);

    // --- Frame Rate Control ---
    frame_rate_control(&last_frame_time, FRAME_RATE_CAP);

    return 0;
}


// Optimized key press hook with refined collision
int key_press_hook(int keycode, t_params *params)
{
    double move_speed = MOVE_SPEED; // Adjust as needed (world units per second? per frame?)
    double rot_speed = ROTATE_SPEED; // Adjust as needed (radians per second? per frame?)
    // Consider delta time if implementing frame-rate independent movement

    double move_step = move_speed; // Simplified: units per key press
    double rot_step = rot_speed;   // Simplified: radians per key press

    double new_x = params->player.x;
    double new_y = params->player.y;
    double dir_x = cos(params->player.direction);
    double dir_y = sin(params->player.direction);
    double side_x = -dir_y; // Vector perpendicular to direction
    double side_y = dir_x;

    // Collision buffer distance (fraction of TILE_SIZE)
    double collision_dist = TILE_SIZE * 0.25;


    if (keycode == XK_Escape) {
        close_window_hook(params);
        return 0; // Indicate handled
    }

    // --- Rotation ---
    if (keycode == XK_Left) {
        params->player.direction -= rot_step;
    } else if (keycode == XK_Right) {
        params->player.direction += rot_step;
    }

    // Normalize angle after rotation
    params->player.direction = normalize_angle(params->player.direction);
    // Update direction vectors after potential rotation
    dir_x = cos(params->player.direction);
    dir_y = sin(params->player.direction);
    side_x = -dir_y;
    side_y = dir_x;


    // --- Movement ---
    double move_x_comp = 0.0;
    double move_y_comp = 0.0;

    if (keycode == XK_w || keycode == XK_Up) { // Forward
        move_x_comp += dir_x * move_step;
        move_y_comp += dir_y * move_step;
    } else if (keycode == XK_s || keycode == XK_Down) { // Backward
        move_x_comp -= dir_x * move_step;
        move_y_comp -= dir_y * move_step;
    } else if (keycode == XK_a) { // Strafe Left
        move_x_comp += side_x * move_step;
        move_y_comp += side_y * move_step;
    } else if (keycode == XK_d) { // Strafe Right
        move_x_comp -= side_x * move_step;
        move_y_comp -= side_y * move_step;
    }

    // --- Collision Detection & Final Movement ---
    if (move_x_comp != 0.0 || move_y_comp != 0.0) {
        // Check collision for X movement component
        double check_x = params->player.x + move_x_comp;
        double check_y_x_move = params->player.y; // Check at current y
        // Check slightly ahead based on movement direction for X
        double collision_check_x = check_x + (move_x_comp > 0 ? collision_dist : -collision_dist);

        if (!is_wall_at(params, collision_check_x, check_y_x_move)) {
            new_x = check_x;
        }

        // Check collision for Y movement component (using potentially updated new_x)
        double check_y = params->player.y + move_y_comp;
        double check_x_y_move = new_x; // Check at potentially updated x
         // Check slightly ahead based on movement direction for Y
        double collision_check_y = check_y + (move_y_comp > 0 ? collision_dist : -collision_dist);

        if (!is_wall_at(params, check_x_y_move, collision_check_y)) {
            new_y = check_y;
        }

        // Update player position
        params->player.x = new_x;
        params->player.y = new_y;
    }


    return 0; // Indicate key press was handled
}


// Handle window close button (unchanged)
int close_window_hook(t_params *params)
{
    printf("Closing window and cleaning up...\n");
    cleanup(params);
    exit(EXIT_SUCCESS);
    //return (0); // Should not be reached after exit
}

// Free allocated resources (ensure ft_ functions are used if necessary)
void cleanup(t_params *params)
{
    int i = 0;

    // Free map data
    if (params->map.map_data) {
        // Use params->map.rows which should be correct
        for (i = 0; i < params->map.rows; i++) {
            if (params->map.map_data[i]) {
                free(params->map.map_data[i]); // Use standard free
                params->map.map_data[i] = NULL;
            }
        }
        free(params->map.map_data);
        params->map.map_data = NULL;
    }

    // Destroy MiniLibX resources
    if (params->mlx) { // Check mlx first
        if (params->window_img.img)
            mlx_destroy_image(params->mlx, params->window_img.img);
        if (params->win)
            mlx_destroy_window(params->mlx, params->win);
        // Only needed for Linux MLX versions usually
        #ifndef __APPLE__
        mlx_destroy_display(params->mlx);
        #endif
        free(params->mlx); // Standard free for the mlx pointer itself
    }


    printf("Cleanup complete.\n");
}


// Initialize game parameters and MiniLibX
void init_params(t_params *params)
{
    // --- Hardcoded Map (Keep for demo, replace with file loading) ---
    const char *map_layout[] = {
        "1111111111111111111111111",
        "1000000001000000000000101",
        "1011010111011001011101101",
        "1001000000010001000100001",
        "10110111110110W0011101001", // Player starting West
        "1000000010000000000000001",
        "1001000010000111111000101",
        "1010001010000000001000101",
        "1111111111111111111111111"
    };
    int rows = sizeof(map_layout) / sizeof(map_layout[0]);
    int cols = 0;
    int x, y;
    bool player_found = false;

    // Zero out the struct first
    // Use ft_memset if it's your libft version
    memset(params, 0, sizeof(t_params)); // Or ft_memset

    // --- Map Initialization ---
    if (rows > 0)
        cols = ft_strlen(map_layout[0]); // Use ft_strlen

    params->map.rows = rows;
    params->map.cols = cols;
    // Basic validation
    if (rows < 3 || cols < 3) {
         fprintf(stderr, "Error: Map is too small.\n");
         exit(EXIT_FAILURE); // No cleanup needed yet
    }
    params->map.map_data = malloc(rows * sizeof(char *));
    if (!params->map.map_data) {
        perror("Failed to allocate map rows");
        exit(EXIT_FAILURE);
    }
    // Null-terminate the array for safety in cleanup loop
    // for (int i = 0; i < rows; ++i) params->map.map_data[i] = NULL;

    for (y = 0; y < rows; y++) {
        // Validate row length
        if (ft_strlen(map_layout[y]) != cols) {
             fprintf(stderr, "Error: Map row %d has inconsistent length.\n", y);
             // Basic cleanup before exit
             while (--y >= 0) free(params->map.map_data[y]);
             free(params->map.map_data);
             exit(EXIT_FAILURE);
        }
        params->map.map_data[y] = ft_strdup(map_layout[y]); // Use ft_strdup
        if (!params->map.map_data[y]) {
            perror("Failed to allocate map row");
             // Proper cleanup
             while (--y >= 0) free(params->map.map_data[y]);
             free(params->map.map_data);
             exit(EXIT_FAILURE);
        }

        // Find player start position & validate map characters
        for (x = 0; x < cols; x++) {
             char cell = params->map.map_data[y][x];
             if (strchr("01NSEW ", cell) == NULL) { // Allow space too? Depends on parser
                 fprintf(stderr, "Error: Invalid character '%c' in map at (%d, %d).\n", cell, x, y);
                 cleanup(params); // Use full cleanup
                 exit(EXIT_FAILURE);
             }

            if (!player_found && strchr("NSEW", cell)) {
                params->player.x = (x + 0.5) * TILE_SIZE;
                params->player.y = (y + 0.5) * TILE_SIZE;

                if (cell == PLAYER_NORTH) params->player.direction = 3.0 * M_PI / 2.0; // -90 deg
                else if (cell == PLAYER_SOUTH) params->player.direction = M_PI / 2.0;   // 90 deg
                else if (cell == PLAYER_EAST) params->player.direction = 0.0;          // 0 deg
                else if (cell == PLAYER_WEST) params->player.direction = M_PI;         // 180 deg

                params->map.map_data[y][x] = EMPTY; // Clear player marker
                player_found = true;
                // Don't break here, continue validating the rest of the row
            } else if (player_found && strchr("NSEW", cell)) {
                 fprintf(stderr, "Error: Multiple player start positions found.\n");
                 cleanup(params);
                 exit(EXIT_FAILURE);
            }
        }
    }

    if (!player_found) {
        fprintf(stderr, "Error: No player start position (N, S, E, W) found in map.\n");
        cleanup(params);
        exit(EXIT_FAILURE);
    }

     // Add map boundary check (ensure surrounded by walls '1') - Basic example
    for(x = 0; x < cols; x++) if(params->map.map_data[0][x] != WALL || params->map.map_data[rows-1][x] != WALL) goto map_error;
    for(y = 0; y < rows; y++) if(params->map.map_data[y][0] != WALL || params->map.map_data[y][cols-1] != WALL) goto map_error;
    goto map_ok;
map_error:
    fprintf(stderr, "Error: Map must be surrounded by walls ('1').\n");
    cleanup(params);
    exit(EXIT_FAILURE);
map_ok:; // Label requires a statement


    // --- Player Initialization ---
    params->player.fov = PLAYER_FOV; // Field of View

    // Calculate distance to projection plane based on FOV
    // dist = (screenWidth / 2) / tan(FOV / 2)
    params->dist_proj_plane = (WINDOW_WIDTH / 2.0) / tan(PLAYER_FOV / 2.0);


    // --- MiniLibX Initialization ---
    params->mlx = mlx_init();
    if (!params->mlx) {
        perror("mlx_init failed");
        cleanup(params);
        exit(EXIT_FAILURE);
    }

    params->win = mlx_new_window(params->mlx, WINDOW_WIDTH, WINDOW_HEIGHT, "Cub3D - Optimized");
    if (!params->win) {
        perror("mlx_new_window failed");
        cleanup(params);
        exit(EXIT_FAILURE);
    }

    // --- Image Buffer Initialization ---
    params->window_img.width = WINDOW_WIDTH;
    params->window_img.height = WINDOW_HEIGHT;
    params->window_img.img = mlx_new_image(params->mlx, params->window_img.width, params->window_img.height);
    if (!params->window_img.img) {
        perror("mlx_new_image failed");
        cleanup(params);
        exit(EXIT_FAILURE);
    }

    params->window_img.addr = mlx_get_data_addr(params->window_img.img,
                                               &params->window_img.bits_per_pixel,
                                               &params->window_img.line_length,
                                               &params->window_img.endian);
    if (!params->window_img.addr) {
        perror("mlx_get_data_addr failed");
        cleanup(params);
        exit(EXIT_FAILURE);
    }
    // Store bytes per pixel for convenience
    params->window_img.bpp = params->window_img.bits_per_pixel / 8;
    if (params->window_img.bpp != 4) {
         fprintf(stderr, "Warning: Code optimized for 32bpp. Current bpp: %d\n", params->window_img.bpp * 8);
         // Might need adjustments in direct pixel writing if not 4 bytes/pixel
    }


    printf("Initialization complete. Player at (%.2f, %.2f), Angle: %.2f rad\n",
           params->player.x, params->player.y, params->player.direction);
    printf("Map: %d rows x %d cols. TILE_SIZE: %d\n", params->map.rows, params->map.cols, TILE_SIZE);
    printf("Window: %d x %d. Rays: %d. FOV: %.1f deg.\n", WINDOW_WIDTH, WINDOW_HEIGHT, NUM_RAYS, PLAYER_FOV * 180.0 / M_PI);
    printf("Projection plane distance: %.2f\n", params->dist_proj_plane);

}

// Main function (unchanged structure)
int main(void)
{
    t_params params;

    // Initialize game (includes MLX setup)
    init_params(&params); // Handles map loading, player setup, MLX init

    // Setup MiniLibX hooks
    mlx_loop_hook(params.mlx, game_loop, &params); // Game loop
    mlx_hook(params.win, KeyPress, KeyPressMask, key_press_hook, &params);
    // Consider KeyRelease hook if you want smoother movement stop
    mlx_hook(params.win, DestroyNotify, StructureNotifyMask, close_window_hook,&params); // Window close button

    // Start the game loop
    printf("\nStarting game...\n");
    printf("Controls: W/A/S/D or Arrows = Move/Strafe | Left/Right Arrows = Turn | ESC = Exit\n");
    mlx_loop(params.mlx);

    // Cleanup is handled by the close_window_hook or upon error in init
    return (0); // Should not be reached if mlx_loop runs correctly
}
