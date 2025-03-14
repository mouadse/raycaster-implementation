CC = cc
CFLAGS = -Wall -Wextra -Iinclude -I./libft -fsanitize=address -g3 -O0
LDFLAGS = -L./libft -lft

NAME = cub3D

SRC = $(shell find src -name '*.c')

OBJ = $(SRC:.c=.o)

all: $(NAME)

$(NAME): $(OBJ)
	@make -C ./libft
	$(CC) $(CFLAGS) -o $(NAME) $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@make clean -C ./libft
	rm -f $(OBJ)

fclean: clean
	@make fclean -C ./libft
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re

