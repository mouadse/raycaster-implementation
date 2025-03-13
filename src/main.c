#include "../include/cub3d.h"

int	main(void)
{
	unsigned int	color;

	// Time to test process_rgb
	char *rgb_color = "F 10,20,30"; // Added the "F " prefix with a space
	char *original = "F 255,255,255";  // Keep original consistent
	char del = 'F';                    // Set delimiter to 'F'
	process_rgb(&color, rgb_color, original, del);
	printf("Color: %u\n", color);
	return (0);
}
