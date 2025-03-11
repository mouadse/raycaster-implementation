#include "../../include/cub3d.h"

int is_valid_map_char(char c) {
  return (c == '0' || c == '1' || c == ' ' || c == 'N' || c == 'S' ||
          c == 'E' || c == 'W');
}
