CC = cc
CFLAGS = -Wall -Wextra -Iinclude -I./libft
NAME = cub3D
SRC = $(shell find src -name '*.c')
OBJ = $(SRC:.c=.o)

# Colors for output
CLR_RMV := \033[0m
RED     := \033[1;31m
GREEN   := \033[1;32m
YELLOW  := \033[1;33m
BLUE    := \033[1;34m
CYAN    := \033[1;36m

# Platform detection
UNAME := $(shell uname)

# Platform-specific configurations
ifeq ($(UNAME), Darwin)
# macOS configuration
CFLAGS += -I./mlx
LDFLAGS = -L./libft -lft -Lmlx -lmlx -framework OpenGL -framework AppKit
else
# Linux configuration
CFLAGS += -I./mlx
LDFLAGS = -L./libft -lft -L./mlx -lmlx -lXext -lX11 -lm -lz
endif

all: $(NAME)

ifeq ($(UNAME), Darwin)
# macOS build
$(NAME): $(OBJ)
	@echo "$(GREEN)Compiling $(NAME) for macOS...$(CLR_RMV)"
	@make -C ./libft
	@$(MAKE) -C ./mlx
	@cp ./mlx/libmlx.a .
	$(CC) $(CFLAGS) -o $(NAME) $(OBJ) $(LDFLAGS)
	@echo "$(GREEN)$(NAME) created ✅$(CLR_RMV)"
else
# Linux build
$(NAME): $(OBJ)
	@echo "$(GREEN)Compiling $(NAME) for Linux...$(CLR_RMV)"
	@make -C ./libft
	@$(MAKE) -C ./mlx
	$(CC) $(CFLAGS) -o $(NAME) $(OBJ) $(LDFLAGS)
	@echo "$(GREEN)$(NAME) created ✅$(CLR_RMV)"
endif

# Compile source files to object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean object files
clean:
	@make clean -C ./libft
	@$(MAKE) -C ./mlx clean
	@rm -f $(OBJ)
	@echo "$(RED)Deleted object files ✅$(CLR_RMV)"

# Full clean (objects and binaries)
ifeq ($(UNAME), Darwin)
fclean: clean
	@make fclean -C ./libft
	@rm -f $(NAME)
	@rm -f libmlx.a
	@echo "$(RED)Deleted $(NAME) binary ✅$(CLR_RMV)"
else
fclean: clean
	@make fclean -C ./libft
	@rm -f $(NAME)
	@echo "$(RED)Deleted $(NAME) binary ✅$(CLR_RMV)"
endif

# Rebuild everything
re: fclean all

.PHONY: all clean fclean re
