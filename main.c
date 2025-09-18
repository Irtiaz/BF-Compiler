#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

void print_usage(const char *program_name) {
  fprintf(stderr, "Wrong usage\n");
  fprintf(stderr, "Sample usage: %s <bf code>\n", program_name);
}

bool is_operator(const char c) {
  const char *operators = "><+-.,[]";
  for (size_t i = 0; operators[i] != '\0'; ++i) {
    if (operators[i] == c) return true;
  }
  return false;
}

int main(int argc, char **argv) {

  if (argc != 2) {
    print_usage(argv[0]);
    exit(1);
  }

  FILE *code_file = fopen(argv[1], "r");

  char code[100];
  size_t code_length = 0;
  char c;

  while ((c = fgetc(code_file)) != EOF) {
    if (is_operator(c)) code[code_length++] = c;
  }
  code[code_length] = '\0';

  printf("%s\n", code);
  fclose(code_file);
  return 0;
}
