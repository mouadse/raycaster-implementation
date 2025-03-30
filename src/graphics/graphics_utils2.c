
#include "../../include/cub3d.h"

/**
 * Calculates the Euclidean distance between two points (x1,y1) and (x2,y2).
 *
 * @param x1 X-coordinate of the first point (usually player position)
 * @param y1 Y-coordinate of the first point (usually player position)
 * @param x2 X-coordinate of the second point (usually wall intersection)
 * @param y2 Y-coordinate of the second point (usually wall intersection)
 * @return The distance between the points, or INT_MAX if no valid intersection
 */
 double calculate_euclidean_distance(double x1, double y1, double x2, double y2)
 {
	 // Check if the target point is marked as invalid
	 if ((int)x2 == INT_MAX && (int)y2 == INT_MAX)
		 return (INT_MAX);

	 // Calculate Euclidean distance using Pythagorean theorem
	 return (sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2)));
 }

 /**
  * Sets up the initial position and step values for a ray checking horizontal wall intersections.
  * Horizontal intersections occur when the ray crosses a horizontal grid line.
  *
  * @param params Game parameters containing player position and map data
  * @param ray_angle The angle at which the ray is cast (in radians)
  * @param ray_pos Pointer to store the initial ray position
  * @param ray_step Pointer to store the step values for ray advancement
  */
 static void initialize_horizontal_ray_intersection(t_params *params, double ray_angle,
		 t_fpoint *ray_pos, t_fpoint *ray_step)
 {
	 double inverse_tangent;

	 // Initialize ray position to invalid values
	 ray_pos->x = INT_MAX;
	 ray_pos->y = INT_MAX;

	 // If the ray is perfectly horizontal, there are no horizontal intersections
	 if (ray_angle == 0 || ray_angle == M_PI || ray_angle == 2 * M_PI)
		 return;

	 // For rays pointing downward (0 to PI)
	 else if (ray_angle > 0 && ray_angle < M_PI)
	 {
		 // Find the next horizontal grid line below the player
		 ray_pos->y = (((int)params->player.y / TILE_SIZE) * TILE_SIZE) + TILE_SIZE;
		 ray_step->y = TILE_SIZE;  // Move down one tile at a time
	 }
	 // For rays pointing upward (PI to 2*PI)
	 else
	 {
		 // Find the next horizontal grid line above the player (subtract small value to ensure we're above the line)
		 ray_pos->y = (((int)params->player.y / TILE_SIZE) * TILE_SIZE) - 0.0001;
		 ray_step->y = -TILE_SIZE;  // Move up one tile at a time
	 }

	 // Calculate the corresponding x-coordinate using the inverse tangent
	 inverse_tangent = 1.0 / tan(ray_angle);
	 ray_pos->x = params->player.x + (ray_pos->y - params->player.y) * inverse_tangent;

	 // Calculate x step based on y step and angle
	 ray_step->x = ray_step->y * inverse_tangent;
 }

 /**
  * Finds the intersection point between a ray and a horizontal wall.
  * The ray is cast from the player's position at the specified angle.
  *
  * @param params Game parameters containing player position and map data
  * @param ray_angle The angle at which the ray is cast (in radians)
  * @return The coordinates of the intersection point, or (INT_MAX, INT_MAX) if no intersection found
  */
 t_fpoint find_horizontal_wall_intersection(t_params *params, double ray_angle)
 {
	 t_fpoint ray_position;     // Current position of the ray
	 t_fpoint ray_step;         // Step values for advancing the ray
	 t_point map_cell;          // Map grid cell indices

	 // Set up initial ray position and step values
	 initialize_horizontal_ray_intersection(params, ray_angle, &ray_position, &ray_step);

	 // If ray is perfectly horizontal, no horizontal intersections possible
	 if (ray_position.x == INT_MAX && ray_position.y == INT_MAX)
		 return (ray_position);

	 // Convert ray position to map grid indices
	 map_cell.x = (int)ray_position.x / TILE_SIZE;
	 map_cell.y = (int)ray_position.y / TILE_SIZE;

	 // Continue until ray goes out of map bounds
	 while (map_cell.x >= 0 && map_cell.x < params->map.cols &&
			map_cell.y >= 0 && map_cell.y < params->map.rows)
	 {
		 // Check if ray has hit a wall (represented by '1' in map data)
		 if (params->map.map_data[map_cell.y][map_cell.x] == '1')
			 return (ray_position);

		 // Move ray to next potential intersection
		 ray_position.x += ray_step.x;
		 ray_position.y += ray_step.y;

		 // Update grid indices based on new ray position
		 map_cell.x = (int)ray_position.x / TILE_SIZE;
		 map_cell.y = (int)ray_position.y / TILE_SIZE;
	 }

	 // If no intersection found within map bounds, mark as invalid
	 ray_position.x = INT_MAX;
	 ray_position.y = INT_MAX;
	 return (ray_position);
 }

 /**
  * Sets up the initial position and step values for a ray checking vertical wall intersections.
  * Vertical intersections occur when the ray crosses a vertical grid line.
  *
  * @param params Game parameters containing player position and map data
  * @param ray_angle The angle at which the ray is cast (in radians)
  * @param ray_pos Pointer to store the initial ray position
  * @param ray_step Pointer to store the step values for ray advancement
  */
 static void initialize_vertical_ray_intersection(t_params *params, double ray_angle,
		 t_fpoint *ray_pos, t_fpoint *ray_step)
 {
	 double tangent;

	 // Initialize ray position to invalid values
	 ray_pos->x = INT_MAX;
	 ray_pos->y = INT_MAX;

	 // If the ray is perfectly vertical, there are no vertical intersections
	 if (ray_angle == M_PI / 2 || ray_angle == 3 * M_PI / 2)
		 return;

	 // For rays pointing left (PI/2 to 3*PI/2)
	 else if (ray_angle > M_PI / 2 && ray_angle < 3 * M_PI / 2)
	 {
		 // Find the next vertical grid line to the left of the player (subtract small value to ensure we're left of the line)
		 ray_pos->x = (((int)params->player.x / TILE_SIZE) * TILE_SIZE) - 0.0001;
		 ray_step->x = -TILE_SIZE;  // Move left one tile at a time
	 }
	 // For rays pointing right (0 to PI/2 or 3*PI/2 to 2*PI)
	 else
	 {
		 // Find the next vertical grid line to the right of the player
		 ray_pos->x = (((int)params->player.x / TILE_SIZE) * TILE_SIZE) + TILE_SIZE;
		 ray_step->x = TILE_SIZE;  // Move right one tile at a time
	 }

	 // Calculate the corresponding y-coordinate using the tangent
	 tangent = tan(ray_angle);
	 ray_pos->y = params->player.y + (ray_pos->x - params->player.x) * tangent;

	 // Calculate y step based on x step and angle
	 ray_step->y = ray_step->x * tangent;
 }

 /**
  * Finds the intersection point between a ray and a vertical wall.
  * The ray is cast from the player's position at the specified angle.
  *
  * @param params Game parameters containing player position and map data
  * @param ray_angle The angle at which the ray is cast (in radians)
  * @return The coordinates of the intersection point, or (INT_MAX, INT_MAX) if no intersection found
  */
 t_fpoint find_vertical_wall_intersection(t_params *params, double ray_angle)
 {
	 t_fpoint ray_position;     // Current position of the ray
	 t_fpoint ray_step;         // Step values for advancing the ray
	 t_point map_cell;          // Map grid cell indices

	 // Set up initial ray position and step values
	 initialize_vertical_ray_intersection(params, ray_angle, &ray_position, &ray_step);

	 // If ray is perfectly vertical, no vertical intersections possible
	 if (ray_position.x == INT_MAX && ray_position.y == INT_MAX)
		 return (ray_position);

	 // Convert ray position to map grid indices
	 map_cell.x = (int)ray_position.x / TILE_SIZE;
	 map_cell.y = (int)ray_position.y / TILE_SIZE;

	 // Continue until ray goes out of map bounds
	 while (map_cell.x >= 0 && map_cell.x < params->map.cols &&
			map_cell.y >= 0 && map_cell.y < params->map.rows)
	 {
		 // Check if ray has hit a wall (represented by '1' in map data)
		 if (params->map.map_data[map_cell.y][map_cell.x] == '1')
			 return (ray_position);

		 // Move ray to next potential intersection
		 ray_position.x += ray_step.x;
		 ray_position.y += ray_step.y;

		 // Update grid indices based on new ray position
		 map_cell.x = (int)ray_position.x / TILE_SIZE;
		 map_cell.y = (int)ray_position.y / TILE_SIZE;
	 }

	 // If no intersection found within map bounds, mark as invalid
	 ray_position.x = INT_MAX;
	 ray_position.y = INT_MAX;
	 return (ray_position);
 }
