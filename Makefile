all: bfc

bfc: compiler.c
	gcc -Wall -Wextra -Werror -pedantic -fsanitize=address,null,leak,undefined $^ -o $@
