#include "../../include/cub3d.h"

int validate_textures(t_game *game) {
  if (!game->textures.north_path || !game->textures.south_path ||
      !game->textures.west_path || !game->textures.east_path)
    return (0);

  if (!file_exists(game->textures.north_path) ||
      !file_exists(game->textures.south_path) ||
      !file_exists(game->textures.west_path) ||
      !file_exists(game->textures.east_path))
    return (0);

  return (1);
}
