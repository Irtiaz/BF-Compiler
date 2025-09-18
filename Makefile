all: bfc

bfc: main.c
	gcc -Wall -Wextra -Werror -pedantic -fsanitize=address,null,leak,undefined $^ -o $@
