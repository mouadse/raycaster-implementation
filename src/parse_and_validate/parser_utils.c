#include "../../include/cub3d.h"

int ft_isspace(char c) {
  return (c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' ||
          c == '\r');
}

int starts_with(char *str, char *prefix) {

  if (!str || !prefix)
    return (0);
  while (*prefix) {
    if (*str != *prefix)
      return (0);
    str++;
    prefix++;
  }
  return (1);
}

int is_empty_line(char *line) {
  if (!line) // This is a guard clause but it can be removed since we check for
    return (1); // NULL in another function

  while (*line) {
    if (!ft_isspace(*line))
      return (0);
    line++;
  }
  return (1);
}

int file_exists(char *file_path) {
  int fd;

  if (!file_path)
    return (0);

  fd = open(file_path, O_RDONLY);
  if (fd < 0)
    return (0);

  close(fd);
  return (1);
}

int is_map_line(char *line) {
  if (!line || !*line)
    return (0);

  while (*line) {
    if (!is_valid_map_char(*line))
      return (0);
    line++;
  }
  return (1);
}
