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
/*
 * Fixed parsing code for Cub3D scene files
 */

// Centralized error reporting function to avoid duplication
static void	report_error(char *message, char *detail, char *line_context)
{
	ft_putstr_fd("Error\n", 2);
	ft_putstr_fd(message, 2);
	if (detail)
	{
		ft_putstr_fd(detail, 2);
	}
	if (line_context)
	{
		ft_putstr_fd(" in line: ", 2);
		ft_putstr_fd(line_context, 2);
	}
	ft_putstr_fd("\n", 2);
	exit(ONE); // Assuming ONE is defined as 1
}

static void	validate_spacing(char *text, char *identifier, char *line)
{
	int	i;

	i = 0;
	// Use consistent length comparison matching the identifier's actual length
	if (ft_strncmp(text, identifier, ft_strlen(identifier)) == 0)
		i += ft_strlen(identifier);
	else
		report_error("Unexpected identifier format: ", identifier, line);
	// Check if there's at least one whitespace after the identifier
	if (ft_isspace(text[i]))
		return ;
	report_error("Invalid spacing after identifier: ", identifier, line);
}

void	trim_trailing_whitespace(char *str, char *line)
{
	int	i;
	int	whitespace_count;
	int	start_of_whitespace;

	i = 0;
	whitespace_count = 0;
	// Guard against NULL pointers
	if (!str || !line)
		return ;
	// Find the end of the path (first whitespace)
	while (str[i] && !ft_isspace(str[i]))
		i++;
	// Count trailing whitespace
	start_of_whitespace = i;
	while (str[i] && ft_isspace(str[i]))
	{
		whitespace_count++;
		i++;
	}
	// Report error if there are unexpected characters after the path
	if (str[i] != '\0' && str[i] != '\n')
		report_error("Unexpected characters after texture path: ", str, line);
	// Terminate string at the end of path
	str[start_of_whitespace] = '\0';
}

void	store_texture_path(char **texture_ptr, char *raw_text, char *identifier,
		char *line_buffer)
{
	int		count;
	char	*path_start;

	count = 0;
	// Check for duplicate texture
	if (*texture_ptr != NULL)
		report_error("Duplicate identifier: ", identifier, line_buffer);
	// Validate spacing
	validate_spacing(raw_text, identifier, line_buffer);
	// Skip identifier and leading spaces
	path_start = raw_text;
	while (ft_isspace(*path_start) || ft_strncmp(identifier, path_start,
			ft_strlen(identifier)) == 0)
	{
		if (ft_strncmp(identifier, path_start, ft_strlen(identifier)) == 0)
		{
			count++;
			path_start += ft_strlen(identifier);
		}
		else
			path_start++;
	}
	// Trim trailing whitespace and validate
	trim_trailing_whitespace(path_start, line_buffer);
	// Verify path exists and identifier count
	if (!file_exists(path_start))
		report_error("File not found: ", path_start, line_buffer);
	if (count != 1)
		report_error("Identifier must appear exactly once: ", identifier,
			line_buffer);
	// Store the validated path
	*texture_ptr = ft_strdup(path_start);
	if (!*texture_ptr)
		report_error("Memory allocation failed", NULL, NULL);
}

void	parse_scene_element(t_textures *textures, char *identifier,
		char *line_buffer)
{
	bool	is_floor;
	bool	is_ceiling;

	// Initialize boolean flags properly
	is_floor = false;
	is_ceiling = false;
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
		// Uncomment and replace with actual implementation
		// process_rgb(&textures->floor, identifier, line_buffer, 'F');
		is_floor = true;
	}
	else if (ft_strncmp("C", identifier, 1) == 0)
	{
		// Uncomment and replace with actual implementation
		// process_rgb(&textures->ceiling, identifier, line_buffer, 'C');
		is_ceiling = true;
	}
	else if (identifier[0] != '\n' && identifier[0] != '\0')
		report_error("Invalid identifier: ", identifier, line_buffer);
	// Handle floor color
	if (is_floor)
	{
		textures->floor_color_count++;
		if (textures->floor_color_count > 1)
			report_error("Duplicate floor color definition", NULL, line_buffer);
	}
	// Handle ceiling color
	if (is_ceiling)
	{
		textures->ceiling_color_count++;
		if (textures->ceiling_color_count > 1)
			report_error("Duplicate ceiling color definition", NULL,
				line_buffer);
	}
	// Set colors flag when both are defined exactly once
	if (textures->floor_color_count == 1 && textures->ceiling_color_count == 1)
		textures->colors_complete = true;
	textures->element_count++;
}

void	initialize_textures_data(t_textures *textures)
{
	if (!textures)
		return ;
	// Initialize texture paths
	textures->north_path = NULL;
	textures->south_path = NULL;
	textures->west_path = NULL;
	textures->east_path = NULL;
	// Initialize color tracking
	textures->colors_complete = false;
	textures->floor_color_count = 0;
	textures->ceiling_color_count = 0;
	textures->element_count = 0;
}
