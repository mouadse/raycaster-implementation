#include "../../include/cub3d.h"

int	is_cub_file(char *filename)
{
	char	*extension;

	if (!filename)
		return (0);
	extension = ft_strrchr(filename, '.');
	if (!extension)
		return (0);
	return (ft_strcmp(extension, ".cub") == 0);
}

static void	print_spacing_error_and_exit(char *identifier, char *line)
{
	// Added this function for clean code and to avoid code duplication
	ft_putstr_fd("Error\n", 2);
	ft_putstr_fd("Invalid spacing for identifier: ", 2);
	ft_putstr_fd(identifier, 2);
	ft_putstr_fd(" in line: ", 2);
	ft_putstr_fd(line, 2);
	ft_putstr_fd("\n", 2);
	exit(ONE);
}

static void	validate_spacing(char *text, char *identifier, char *line)
{
	int	i;

	i = 0;
	if (ft_strncmp(text, identifier, ft_strlen(identifier)) == 0)
		i += ft_strlen(identifier);
	if (ft_isspace(text[i]))
		return ;
	print_spacing_error_and_exit(identifier, line);
}

void	trim_trailing_whitespace(char *str, char *line)
{
	int	i;
	int	whitespace_count;

	i = 0;
	whitespace_count = 0;
	if (!str || !line)
		return ;
	while (str[i] && !ft_isspace(str[i]))
		i++;
	while (str[i] && ft_isspace(str[i]))
	{
		whitespace_count++;
		i++;
	}
	if (str[i] == '\0' || str[i] == '\n')
	{
		ft_putstr_fd("Error\n", 2);
		ft_putstr_fd("Invalid spacing for identifier: ", 2);
		ft_putstr_fd(str, 2);
		ft_putstr_fd(" in line: ", 2);
		ft_putstr_fd(line, 2);
		ft_putstr_fd("\n", 2);
		exit(ONE);
	}
	str[i - whitespace_count] = '\0';
}

void	store_texture_path(char **texture_ptr, char *raw_text, char *identifier,
		char *line_buffer)
{
	int		count;
	char	*path_start;

	count = 0;
	if (*texture_ptr != NULL)
	{
		ft_putstr_fd("Error\n", 2);
		ft_putstr_fd("Duplicate identifier: ", 2);
		ft_putstr_fd(identifier, 2);
		ft_putstr_fd(" in line: ", 2);
		ft_putstr_fd(line_buffer, 2);
		ft_putstr_fd("\n", 2);
		exit(ONE);
	}
	validate_spacing(raw_text, identifier, line_buffer);
	path_start = raw_text;
	while (ft_isspace(*path_start) || ft_strncmp(identifier, path_start,
			2) == 0)
	{
		if (ft_strncmp(identifier, path_start, 2) == 0)
		{
			count++;
			path_start++;
		}
		path_start++;
	}
	trim_trailing_whitespace(path_start, line_buffer);
	if (!file_exists(path_start) || count != 1)
	{
		ft_putstr_fd("Error\n", 2);
		ft_putstr_fd("Invalid path for identifier: ", 2);
		ft_putstr_fd(identifier, 2);
		ft_putstr_fd(" in line: ", 2);
		ft_putstr_fd(line_buffer, 2);
		ft_putstr_fd("\n", 2);
		exit(ONE);
	}
	*texture_ptr = ft_strdup(path_start);
}

void	parse_scene_element(t_textures *textures, char *identifier,
		char *line_buffer)
{
	bool	is_floor;
	bool	is_ceiling;

	if (ft_strncmp("NO", identifier, 2) == 0)
		store_texture_path(&(textures->north_path), identifier, "NO",
			line_buffer);
	else if (ft_strncmp("SO", identifier, 2) == 0)
		store_texture_path(&(textures->south_path), identifier, "SO",
			line_buffer);
	else if (ft_strncmp("WE", identifier, 2) == 0)
		store_texture_path(&(textures->west_path), identifier, "WE",
			line_buffer);
	else if (ft_strncmp("EA", identifier, 2) == 0)
		store_texture_path(&(textures->east_path), identifier, "EA",
			line_buffer);
	else if (ft_strncmp("F", identifier, 1) == 0)
	{
		// process_rgb(&scene->floor, identifier, line_buffer, 'F');
		is_floor = true;
	}
	else if (ft_strncmp("C", identifier, 1) == 0)
	{
		// process_rgb(&scene->ceiling, identifier, line_buffer, 'C');
		is_ceiling = true;
	}
	else if (identifier[0] != '\n' && identifier[0] != '\0')
	{
		// free(line_buffer);
		ft_putstr_fd("Error\n", 2);
		ft_putstr_fd("Invalid identifier: ", 2);
		ft_putstr_fd(identifier, 2);
		ft_putstr_fd(" in line: ", 2);
		ft_putstr_fd(line_buffer, 2);
		ft_putstr_fd("\n", 2);
		exit(ONE);
	}
	if (is_floor)
	{
		textures->floor_color_count++;
		if (textures->floor_color_count > 1)
		{
			ft_putstr_fd("Error\n", 2);
			ft_putstr_fd("Duplicate floor color definition in line: ", 2);
			ft_putstr_fd(line_buffer, 2);
			ft_putstr_fd("\n", 2);
			exit(ONE);
		}
	}
	if (is_ceiling)
	{
		textures->ceiling_color_count++;
		if (textures->ceiling_color_count > 1)
		{
			ft_putstr_fd("Error\n", 2);
			ft_putstr_fd("Duplicate ceiling color definition in line: ", 2);
			ft_putstr_fd(line_buffer, 2);
			ft_putstr_fd("\n", 2);
			exit(ONE);
		}
	}
	// Set colors flag when both are defined exactly once
	if (textures->floor_color_count == 1 && textures->ceiling_color_count == 1)
		textures->colors_complete = true;
	textures->element_count++;
}
