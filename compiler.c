#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CODE_BUFFER_LENGTH 10000

void print_usage(const char *program_name) {
  fprintf(stderr, "Wrong usage\n");
  fprintf(stderr, "Sample usage: %s <bf code path> <output path>\n",
          program_name);
}
bool is_operator(const char c) {
  const char *operators = "><+-.,[]";
  for (size_t i = 0; operators[i] != '\0'; ++i) {
    if (operators[i] == c)
      return true;
  }
  return false;
}

void print_prologue(FILE *output) {
  fprintf(output, "global _start\n");
  fprintf(output, "section .text\n\n");
  fprintf(output, "_start:\n");
  fprintf(output, "	mov rbp, rsp\n");
  fprintf(output, "	mov byte [rbp], 0\n");
  fprintf(output, "\n\n");
}

void print_epilogue(FILE *output) {
  fprintf(output, "	mov rsp, rbp\n");
  fprintf(output, "	mov rax, 60\n");
  fprintf(output, "	xor rdi, rdi\n");
  fprintf(output, "	syscall\n");

  fprintf(output, "\n\n");
  fprintf(output, "section .data\n");
  fprintf(output, "	pointer dq 0\n");
  fprintf(output, "	allocated dq 0\n");
}

int main(int argc, char **argv) {
  if (argc != 3) {
    print_usage(argv[0]);
    exit(1);
  }

  FILE *code_file = fopen(argv[1], "r");
  const char *program_name = argv[2];

  char asm_out_path[100];
  strcpy(asm_out_path, program_name);
  strcat(asm_out_path, ".asm");
  FILE *asm_out = fopen(asm_out_path, "w");

  char code[CODE_BUFFER_LENGTH];
  size_t code_length = 0;

  size_t pair_indices[CODE_BUFFER_LENGTH];
  size_t open_stack[CODE_BUFFER_LENGTH];
  size_t open_stack_length = 0;
  size_t next_pair_index = 0;

  char c;

  while ((c = fgetc(code_file)) != EOF) {
    bool valid_op = is_operator(c);
    if (valid_op) {
      code[code_length] = c;
    }

    if (c == '[') {
      pair_indices[code_length] = next_pair_index;
      open_stack[open_stack_length++] = next_pair_index++;
    }

    else if (c == ']') {
      if (open_stack_length == 0) {
	fprintf(stderr, "No matching [ found\n");
	exit(1);
      }

      pair_indices[code_length] = open_stack[--open_stack_length];
    }

    if (valid_op)
      ++code_length;
  }
  code[code_length] = '\0';

  if (open_stack_length != 0) {
    fprintf(stderr, "No matching ] found\n");
    exit(1);
  }

  print_prologue(asm_out);

  int label_counter = 0;
  for (size_t i = 0; i < code_length; ++i) {
    char op = code[i];

    switch (op) {

    case '>':
      fprintf(asm_out, "	mov rax, [allocated]\n");
      fprintf(asm_out, "	cmp [pointer], rax\n");
      fprintf(asm_out, "	jl .L%d\n", label_counter);
      fprintf(asm_out, "	sub rsp, 1\n");
      fprintf(asm_out, "	mov byte [rsp], 0\n");
      fprintf(asm_out, "	inc qword [allocated]\n");
      fprintf(asm_out, ".L%d:\n", label_counter++);
      fprintf(asm_out, "	inc qword [pointer]\n");
      break;

    case '<':
      fprintf(asm_out, "	dec qword [pointer]\n");
      break;

    case '+':
      fprintf(asm_out, "	mov rbx, rbp\n");
      fprintf(asm_out, "	sub rbx, [pointer]\n");
      fprintf(asm_out, "	inc byte [rbx]\n");
      break;

    case '-':
      fprintf(asm_out, "	mov rbx, rbp\n");
      fprintf(asm_out, "	sub rbx, [pointer]\n");
      fprintf(asm_out, "	dec byte [rbx]\n");
      break;

    case '.':
      fprintf(asm_out, "	mov rax, 1\n");
      fprintf(asm_out, "	mov rdi, 1\n");
      fprintf(asm_out, "	mov rsi, rbp\n");
      fprintf(asm_out, "	sub rsi, [pointer]\n");
      fprintf(asm_out, "	mov rdx, 1\n");
      fprintf(asm_out, "	syscall");
      break;

    case ',':
      fprintf(asm_out, "	xor rax, rax\n");
      fprintf(asm_out, "	xor rdi, rdi\n");
      fprintf(asm_out, "	mov rsi, rbp\n");
      fprintf(asm_out, "	sub rsi, [pointer]\n");
      fprintf(asm_out, "	mov rdx, 1\n");
      fprintf(asm_out, "	syscall");
      break;

    case '[':
      fprintf(asm_out, "	mov rbx, rbp\n");
      fprintf(asm_out, "	sub rbx, [pointer]\n");
      fprintf(asm_out, "	cmp byte [rbx], 0\n");
      fprintf(asm_out, "	je .cnd_end%lu\n", pair_indices[i]);
      fprintf(asm_out, ".cnd_start%lu:\n", pair_indices[i]);
      break;

    case ']':
      fprintf(asm_out, "	mov rbx, rbp\n");
      fprintf(asm_out, "	sub rbx, [pointer]\n");
      fprintf(asm_out, "	cmp byte [rbx], 0\n");
      fprintf(asm_out, "	jne .cnd_start%lu\n", pair_indices[i]);
      fprintf(asm_out, ".cnd_end%lu:\n", pair_indices[i]);
      break;

    default: {
      fprintf(stderr, "Undefined operator %c\n", op);
      exit(1);
    }
    }

    fprintf(asm_out, "\n");
  }

  print_epilogue(asm_out);

  fclose(code_file);
  fclose(asm_out);

  char nasm_command[500];
  sprintf(nasm_command, "nasm -felf64 -gdwarf %s -o %s.o", asm_out_path,
          program_name);
  system(nasm_command);

  char ld_command[500];
  sprintf(ld_command, "ld %s.o -o %s", program_name, program_name);
  system(ld_command);

  return 0;
}
