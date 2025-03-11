#include "../../include/cub3d.h"

int is_cub_file(char *filename) {
  char *extension;

  if (!filename)
	return (0);
  extension = ft_strrchr(filename, '.');
  if (!extension)
    return (0);

  return (ft_strcmp(extension, ".cub") == 0);
}
