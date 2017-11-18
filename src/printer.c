#include <stdio.h>
#include <assert.h>
#include "printer.h"

void exp_printer(void* data);
void comma_printer(void* data);
void print_list(ast_list* l, void (*printer)(void*), void (*sep)(void*));
void foreach(ast_list* l, void (*f)(void*));

void print_timing_control_statement(ast_timing_control_statement* tstmt);
void print_statement_block(void*);
void print_conditional_statement(void*);
void print_sing_asgn(void*);
void print_lvalue(void*);

void print_statement(void* data) {
  ast_statement* stmt = (ast_statement*) data;
  assert(stmt != NULL);

  switch (stmt->type) {
  case STM_WAIT : { 
    ast_wait_statement* w = stmt->wait;
    assert(w != NULL);

    printf("%s", ast_expression_tostring(w->expression));           
    printf(" ==> ");
    print_statement(w->statement);
    printf("\n");
    break;
  }
  case STM_CONDITIONAL : { 
    ast_if_else* ife =  (ast_if_else*) stmt->conditional;

    printf("{{{\n");
    foreach(ife->conditional_statements, &print_conditional_statement);
    if(ife->else_condition) {
      printf("else {\n");
      print_statement(ife->else_condition);
      printf("}\n");
    }
    printf("}}}\n");
    break;
  }
  case STM_BLOCK          :
  case STM_BLOCK_ALWAYS   :
  case STM_BLOCK_INITIAL  : {
    print_statement_block((void*) stmt->block);
    printf("\n");
    break;
  }
  case STM_ASSIGNMENT     : { 
    ast_assignment* a = stmt->assignment;

    switch(a->type){
    case ASSIGNMENT_CONTINUOUS: {
      ast_continuous_assignment * cont = a->continuous;
      printf("continuous asgn {\n"); fflush(stdout);
      foreach(cont->assignments, &print_sing_asgn);
      printf("}\n"); fflush(stdout);
      break;
    }
    case ASSIGNMENT_BLOCKING: {
      ast_procedural_assignment* proc = a->procedural;
      print_lvalue((void*) proc->lval); fflush(stdout);
      printf(" = |||%s|||;\n", ast_expression_tostring(proc->expression)); fflush(stdout);
      break;
    }
    case ASSIGNMENT_NONBLOCKING: {
      ast_procedural_assignment* proc = a->procedural;
      print_lvalue((void*) proc->lval); fflush(stdout);
      printf(" <= |||%s|||;\n", ast_expression_tostring(proc->expression)); fflush(stdout);
      break;
    }
    case ASSIGNMENT_HYBRID: {
      printf("<assignment>\n");
      break;
    }
    }
  }
  case STM_GENERATE : {
    break;
  }
    // --------------------------------------------------------------
  case STM_CASE           : { printf("STM_CASE");           printf("\n"); break;}
  case STM_DISABLE        : { printf("STM_DISABLE");        printf("\n"); break;}
  case STM_EVENT_TRIGGER  : { printf("STM_EVENT_TRIGGER");  printf("\n"); break;}
  case STM_LOOP           : { printf("STM_LOOP");           printf("\n"); break;}
  case STM_TIMING_CONTROL : { printf("STM_TIMING_CONTROL"); printf("\n"); break;}
  case STM_FUNCTION_CALL  : { printf("STM_FUNCTION_CALL");  printf("\n"); break;}
  case STM_TASK_ENABLE    : { printf("STM_TASK_ENABLE");    printf("\n"); break;}
  case STM_MODULE_ITEM    : { printf("STM_MODULE_ITEM");    printf("\n"); break;}
  }

}

void print_statement_block(void* data) {
  ast_statement_block* block = (ast_statement_block*) data;
  assert(block != NULL);
  
  switch(block->type){
  case BLOCK_SEQUENTIAL: {printf("{\n"); break;}
  case BLOCK_SEQUENTIAL_ALWAYS: { 
    printf("always"); 
    print_timing_control_statement(block->trigger);
    printf(" {\n");
    fflush(stdout);
    break;
  } 
  case BLOCK_SEQUENTIAL_INITIAL: { 
    printf("initial {\n");
    fflush(stdout);
    break;
  } 
  default: {assert(0);}
  }

  /* if(block->declarations) { */
  /*   printf("decl size: %d\n", block->declarations->items); */
  /*   fflush(stdout); */
  /* } */

  if(block->statements) {
    foreach(block->statements, &print_statement);
  }

  printf("}\n");
}

void print_module(ast_module_declaration* module) {
  printf("# Initial blocks #####################\n");
  fflush(stdout);
  foreach(module->initial_blocks, &print_statement_block);

  printf("# Always blocks ######################\n");
  fflush(stdout);
  foreach(module->always_blocks, &print_statement_block);
}

// helper functions

void foreach(ast_list* l, void (*f)(void*)) {
  assert(l != NULL);
  ast_list_element* e = l->head;

  while(e != NULL) {
    (*f)(e->data);
    e = e->next;
  }
}

void print_event_control(ast_event_control* ec) {
  
  switch(ec->type) {
  case EVENT_CTRL_NONE:     { printf("(no triggers)"); break;}
  case EVENT_CTRL_ANY:      { printf("(any trigger)"); break;}
  case EVENT_CTRL_TRIGGERS: { break;}
  }

  ast_event_expression* ee = ec->expression;

  if(ee) {
    switch(ee->type) {
    case EVENT_EXPRESSION: { printf("(exp: %s)", ast_expression_tostring(ee->expression)); break;}
    case EVENT_POSEDGE:    { printf("(pos: %s)", ast_expression_tostring(ee->expression)); break;}
    case EVENT_NEGEDGE:    { printf("(neg: %s)", ast_expression_tostring(ee->expression)); break;}
    case EVENT_SEQUENCE:   { printf("(list-of-events)");                                   break;}
    }
  }
}

void print_timing_control_statement(ast_timing_control_statement* tstmt) {
  assert(tstmt);

  switch(tstmt->type) {
  case TIMING_CTRL_DELAY_CONTROL: break;
  case TIMING_CTRL_EVENT_CONTROL_REPEAT:
    printf("(repeat: %s)", ast_expression_tostring(tstmt->repeat));
  case TIMING_CTRL_EVENT_CONTROL:
    print_event_control(tstmt->event_ctrl);
  }
}

void print_conditional_statement(void* data) {
  ast_conditional_statement* c = (ast_conditional_statement*) data;
  
  printf("elif (%s) {\n", ast_expression_tostring(c->condition));
  print_statement(c->statement);
  printf("}\n");
}

void print_sing_asgn(void* data) {
  ast_single_assignment* asgn = (ast_single_assignment*) data;
  assert(asgn);
  
  print_lvalue(asgn->lval);
  printf(" = %s;\n", ast_expression_tostring(asgn->expression));
}

void print_lvalue(void* data) {
  ast_lvalue* lval = (ast_lvalue*) data;

  switch(lval->type) {
  case SPECPARAM_ID:
  case PARAM_ID:
  case NET_IDENTIFIER:
  case VAR_IDENTIFIER:
  case GENVAR_IDENTIFIER: {
    printf("%s", ast_identifier_tostring(lval->data.identifier));
    break;
  }

  case NET_CONCATENATION:
  case VAR_CONCATENATION: {
    ast_concatenation* conc = lval->data.concatenation;
    printf("(repeat: %s)", ast_expression_tostring(conc->repeat));
      
    switch(conc->type) {
    case CONCATENATION_EXPRESSION:
    case CONCATENATION_CONSTANT_EXPRESSION: {
      print_list(conc->items, &exp_printer, &comma_printer);
      break;
    }
    case CONCATENATION_NET:
    case CONCATENATION_VARIABLE: { 
      print_list(conc->items, &print_lvalue, &comma_printer);
      break;
    }
    case CONCATENATION_MODULE_PATH: { 
      break;
    }
    }

    break;
  }
  }
}

void print_list(ast_list* l, void (*printer)(void*), void (*sep)(void*)) {
  assert(l != NULL);
  ast_list_element* e = l->head;
  
  if (e == NULL) return;
  (*printer)(e->data);
  
  e = e->next;

  while(e != NULL) {
    (*sep)(e->data);
    (*printer)(e->data);
    e = e->next;
  }
}

void exp_printer(void* data) {
  ast_expression* exp = (ast_expression*) data;
  assert(exp != NULL);
  printf("%s", ast_expression_tostring(exp));
}

void comma_printer(__attribute__((unused)) void* data) {
  printf(", ");
}
