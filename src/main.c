/*!
  @file main.c
  @brief A simple test program for the C library code.
*/

#include "stdio.h"

#include "verilog_parser.h"
#include "verilog_ast_common.h"
#include "verilog_preprocessor.h"
#include "verilog_ast_util.h"

#include "printer.h"

int main(int argc, char ** argv) {
  if(argc != 3) {
    printf("usage: parser <filename> <module name>\n");
    return 1;
  } else {
    int F = 1;
            
    // Initialise the parser.
    verilog_parser_init();

    // Setup the preprocessor to look in ./tests/ for include files.
    ast_list_append(yy_preproc -> search_dirs, "./tests/");
    ast_list_append(yy_preproc -> search_dirs, "./");

    // Load the file.
    FILE * fh = fopen(argv[F], "r");
    if(fh == NULL) {
      fprintf(stderr, "error while opening %s\n", argv[F]);
      return 1;
    }

    verilog_preprocessor_set_file(yy_preproc, argv[F]);
            
    // Parse the file and store the result.
    int result = verilog_parse_file(fh);

    // Close the file handle
    fclose(fh);
            
    if(result) {
      fprintf(stderr, "%s - Parse failed\n", argv[F]);
      return 1;
    }
  }
    
  ast_identifier id = ast_new_identifier(argv[2], 0);
  ast_module_declaration* m = verilog_find_module_declaration(yy_verilog_source_tree, id);
  if(m) print_module(m);

  verilog_resolve_modules(yy_verilog_source_tree);
  ast_free_all();
  return 0;
}
