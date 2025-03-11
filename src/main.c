#include "../include/cub3d.h"

int main(void) {
  t_game *game;

  game = malloc(sizeof(t_game));
  if (!game) {
    perror("Error\nMalloc failed\n");
    return (1);
  }
  char *floor = "F 220,100,0";
  //   char *floor = "";
  //   char *ceiling = "C 100,220,0";
  char *ceiling = "";

  if (!parse_color(floor, FLOOR, game)) {
    printf("Error: %s\n", floor);
    return (1);
  } else {
    printf("Floor: %d, %d, %d\n", game->colors.floor_r, game->colors.floor_g,
           game->colors.floor_b);
  }
  // This is a test
  if (!parse_color(ceiling, CEILING, game)) {
    printf("Error: %s\n", ceiling);
    return (1);
  } else {
    printf("Ceiling: %d, %d, %d\n", game->colors.ceiling_r,
           game->colors.ceiling_g, game->colors.ceiling_b);
  }
  return (0);
}
