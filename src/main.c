#include "../include/cub3d.h"

int	main(void)
{
	unsigned int	color;
	char			*rgb_color;
	char			*original;
	char			del;

	// Time to test process_rgb
	// rgb_color = "F 10,20,30";
	// original = "F 255,255,255";
	rgb_color = NULL; // This will trigger a segfault
	original = NULL;
	del = 'F';
	process_rgb(&color, rgb_color, original, del);
	printf("Color: %u\n", color);
	return (0);
}
