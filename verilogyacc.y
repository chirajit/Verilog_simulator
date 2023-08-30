%{
#include "genasm.c"
#include "design.cpp"
#include "evaluate_expressions.cpp"
void yyerror(char *);
int scope=0, dffs = 1, display_no = 1;
char data_type[10];
string modulename, inst_type, inout_type, instance_name, var_to_update_val;
expressions myexp;
static initial init; 
unordered_map<string, always*> alw;
display mydisplay;

%}

%token SEMICOLON_TOK
%token COMMA_TOK
%token LCRLY_TOK
%token RCRLY_TOK
%token LPAREN_TOK
%token RPAREN_TOK
%token EQUAL_TOK
%token DIV_TOK
%token STAR_TOK
%token MOD_TOK
%token PLUS_TOK
%token MINUS_TOK
%token NOT_TOK
%token AND_TOK
%token OR_TOK
%token XOR_TOK
%token COMPLEMENT_TOK
%token SQUARE_BRACKET_START_TOK
%token SQUARE_BRACKET_END_TOK
%token LT_TOK
%token GT_TOK
%token AT_TOK
%token MODULE_TOK
%token ENDMODULE_TOK
%token ALWAYS_TOK
%token INITIAL_TOK
%token FOR_TOK
%token WHILE_TOK
%token INPUT_TOK 
%token OUTPUT_TOK
%token INOUT_TOK
%token REG_TOK 
%token WIRE_TOK
%token WOR_TOK
%token WAND_TOK
%token MONITOR_TOK 
%token ID_TOK
%token INTEGER_TOK
%token STRING_TOK
%token FLOAT_TOK
%token INTEGER_CONST
%token BINARY_CONST
%token OCTAL_CONST
%token DECIMAL_CONST
%token HEX_CONST
%token STRING_CONST
%token COND_OR_TOK
%token COND_AND_TOK
%token COND_EQUAL_TOK
%token DELAY_TOK
%token POWER_TOK
%token ASSIGN_TOK
%token BEGIN_TOK
%token END_TOK
%token IF_TOK
%token ELSE_TOK
%token FLOAT_CONST
%token BIT_OR_TOK
%token BIT_AND_TOK
%token BIT_XOR_TOK
%token LOGIC_TOK
%token PARAMETER_TOK
%token POSEDGE_TOK
%token NBA_TOK
%token LOCALPARAM_TOK
%token LSQUARE_TOK
%token RSQUARE_TOK
%token SLICE_TOK
%token CASE_TOK
%token ENDCASE_TOK
%token DISPLAY_TOK

%start S

%% 
S                 :  MODULE S
                  |  stmt S                        
                  |  MODULE
                  |  stmt
                  ;

MODULE 				:  MODULE_TOK ID_TOK {modulename = var_name;inout_type = "none";module_port = 1;insert_into_arr3addr(LABEL,"","","",modulename); } LPAREN_TOK  PORTS RPAREN_TOK {module_port = 0; } SEMICOLON_TOK STATEMENTS ENDMODULE_TOK
						|  MODULE_TOK ID_TOK {modulename = var_name;inout_type = "none";insert_into_arr3addr(LABEL,"","","",modulename); } SEMICOLON_TOK  STATEMENTS ENDMODULE_TOK
                  |  MODULE_TOK ID_TOK {modulename = var_name;inout_type = "none";module_port = 1;insert_into_arr3addr(LABEL,"","","",modulename); } DELAY_TOK LPAREN_TOK PARAMETER_TOK DECLARATION RPAREN_TOK LPAREN_TOK  PORTS RPAREN_TOK {module_port = 0; } SEMICOLON_TOK STATEMENTS ENDMODULE_TOK
                  ;

BLOCK             :  BEGIN_TOK STATEMENTS BLOCKS END_TOK
                  |  SEMICOLON_TOK
                  ;       

BLOCKS   			: BLOCK STATEMENTS BLOCKS
						|
						;

STATEMENTS        :  STATEMENTS stmt 
                  |  STATEMENTS conditional
                  |  STATEMENTS iterative
                  |  STATEMENTS module_inst
                  |  STATEMENTS inior
                  |  STATEMENTS cases
                  |       
                  ; 

cases             :  CASE_TOK LPAREN_TOK id_token RPAREN_TOK case_elems ENDCASE_TOK
                  ;

case_elems        : case_elems ID_TOK SLICE_TOK BLOCK
                  | ID_TOK SLICE_TOK BLOCK
                  ;

inior  				: ALWAYS_TOK BLOCK {initial_state = 0;}
                  | ALWAYS_TOK stmt {initial_state = 0;}
                  | ALWAYS_TOK conditional
                  | ALWAYS_TOK AT_TOK LPAREN_TOK alw_triggers RPAREN_TOK {insert_into_arr3addr(LABEL, "", "", "", "always"+to_string(always_no)); alw["always"+to_string(always_no)] = new always;} BLOCK 
                  {initial_state = 0; 
                  myexp.schedule_always(alw["always"+to_string(always_no)]);
                  modules[modulename].processes.push_back(make_pair("ALWAYS", "always" + to_string(always_no)));
                  always_no++; 
                  }
                  | ALWAYS_TOK AT_TOK LPAREN_TOK alw_triggers RPAREN_TOK stmt {initial_state = 0;}
                  | ALWAYS_TOK AT_TOK LPAREN_TOK alw_triggers RPAREN_TOK conditional {initial_state = 0;}
						| INITIAL_TOK {initial_state = 1; init.update_time(0);insert_into_arr3addr(LABEL, "", "", "", "initial"+to_string(initial_no));modules[modulename].processes.push_back(make_pair("INITIAL","initial"+to_string(initial_no)));} BLOCK {initial_state = 0; myexp.schedule_initial(init);}
						;

alw_triggers      : ID_TOK  {((valueholder*)(modules[modulename].netlist[var_name].second))->triggers.push_back(make_pair("always"+to_string(always_no), (void *)(alw["always"+to_string(always_no)]) )); } alw_triggers  
                  | COMMA_TOK alw_triggers
                  | ID_TOK {((valueholder*)(modules[modulename].netlist[var_name].second))->triggers.push_back(make_pair("always"+to_string(always_no), (void *)(alw["always"+to_string(always_no)]) )); }
                  ;

module_inst       :  ID_TOK ID_TOK {insert_to_instancelist (modulename, prev_name, var_name); instance_name = var_name;} LPAREN_TOK module_inst_args RPAREN_TOK SEMICOLON_TOK
                  |  ID_TOK {inst_type = var_name;} DELAY_TOK LPAREN_TOK CONSTANTS RPAREN_TOK ID_TOK {insert_to_instancelist (modulename, inst_type, var_name);instance_name = var_name;} LPAREN_TOK module_inst_args RPAREN_TOK SEMICOLON_TOK
                  |  AND_TOK LPAREN_TOK module_inst_args RPAREN_TOK SEMICOLON_TOK
                  |  OR_TOK LPAREN_TOK module_inst_args RPAREN_TOK SEMICOLON_TOK
                  |  XOR_TOK LPAREN_TOK module_inst_args RPAREN_TOK SEMICOLON_TOK
                  ;

module_inst_args  :  module_inst_args ID_TOK { add_instance_connections(modulename,instance_name,var_name);}
                  |  module_inst_args COMMA_TOK
                  |
                  ;

CONSTANTS         : CONSTANTS COMMA_TOK
                  | CONSTANTS constant
                  | CONSTANTS ID_TOK
                  | 
                  ;

PORTS        		:  PORT PORTS
                  |  COMMA_TOK PORTS
                  |  
                  ;

PORT              :  ID_TOK                        {insert_to_netlist(modulename, var_name, inout_type, "wire");}
                  |  inout_type ID_TOK             {insert_to_netlist(modulename, var_name, inout_type, "wire");}
                  |  inout_type datatype ID_TOK    {insert_to_netlist(modulename, var_name, inout_type, data_type);} 	
                  |  datatype ID_TOK               {insert_to_netlist(modulename, var_name, inout_type, data_type);}
                  |  inout_type datatype LSQUARE_TOK expression {myexp.clear();} RSQUARE_TOK ID_TOK    {insert_to_netlist(modulename, var_name, inout_type, data_type);} 
                  ;

inout_type  		:  INPUT_TOK       {inout_type = "input";}
						|  OUTPUT_TOK     {inout_type = "output";}
						|  INOUT_TOK      {inout_type = "inout";}
						;


iterative         :  FOR_TOK LPAREN_TOK DECLARATION SEMICOLON_TOK comparision SEMICOLON_TOK DECLARATION RPAREN_TOK BLOCK
                  |  WHILE_TOK LPAREN_TOK comparision RPAREN_TOK BLOCK
                  ;

conditional       :  IF_TOK LPAREN_TOK cond_expression RPAREN_TOK BLOCK 
                  |  IF_TOK LPAREN_TOK cond_expression RPAREN_TOK stmt
                  |  IF_TOK LPAREN_TOK cond_expression RPAREN_TOK conditional
                  |  IF_TOK LPAREN_TOK cond_expression RPAREN_TOK iterative
                  |  ELSE_TOK stmt
                  |  ELSE_TOK conditional
                  |  ELSE_TOK BLOCK
                  ;

cond_expression   :  cond_expression cond_op cond_expression
                  |  LPAREN_TOK cond_expression RPAREN_TOK 
                  |  NOT_TOK LPAREN_TOK cond_expression RPAREN_TOK 
                  |  comparision
                  ;

cond_op           :  COND_AND_TOK
                  |  COND_OR_TOK
                  ;

comparision       :  comparision data
                  |  data  
                  ;

data              :  constant
                  |  ID_TOK  
                  |  expression_op
                  |  LT_TOK
                  |  GT_TOK
                  |  COND_EQUAL_TOK
                  |  NOT_TOK EQUAL_TOK
                  ;

stmt              :  DECLARATION SEMICOLON_TOK 
						|	DELAY_TOK INTEGER_CONST SEMICOLON_TOK
                  { 
                     if (initial_state == 2)
                     {
                        myexp.schedule_always(alw["always"+to_string(always_no)]); alw["always"+to_string(always_no)]->analyse_delay_tok(intvalue);
                     }
                     else
                     {myexp.schedule_initial(init); init.analyse_delay_tok(intvalue); }
                     insert_into_arr3addr(LABEL, "", "", "", "delay"+(string)intvalue); 
                  }
						|	DELAY_TOK INTEGER_CONST 
                  { 
                     if (initial_state == 2)
                     {
                        myexp.schedule_always(alw["always"+to_string(always_no)]); alw["always"+to_string(always_no)]->analyse_delay_tok(intvalue);
                     }
                     else
                     {myexp.schedule_initial(init); init.analyse_delay_tok(intvalue); }       
                     insert_into_arr3addr(LABEL, "", "", "", "delay"+(string)intvalue); 
                  }
                  |  MONITOR_TOK LPAREN_TOK monitor_vars RPAREN_TOK SEMICOLON_TOK 
                  |  DISPLAY_TOK LPAREN_TOK display_vars RPAREN_TOK SEMICOLON_TOK 
                  {  myexp.schedule_initial(init);
                     reg * disreg = new reg ("disreg");
                     disreg->update_val('d');
                     disreg->display_no = display_no;
                     disreg->global_display_var = (void*) (&mydisplay);
                     string ch = modulename;
                     init.add_event(disreg,ch);
                     for (auto a = (mydisplay.display_vars[display_no]).begin(); a!= (mydisplay.display_vars[display_no]).end(); a++)
                        insert_into_arr3addr(DISPLAY, "ARCH+"+to_string(modules[a->first].offset[a->second]));
                     insert_into_arr3addr(END_DISPLAY);
                     display_no++;
                  }
                  ;

display_vars      :  ID_TOK {mydisplay.add_display_var(modulename, var_name, display_no); }  display_vars
                  |  COMMA_TOK display_vars
                  |  ID_TOK {mydisplay.add_display_var(modulename, var_name, display_no);}
                  ;

monitor_vars      :  ID_TOK {mymonitor.add_monitor_var(modulename, var_name);}  monitor_vars
                  |  COMMA_TOK monitor_vars
                  |  ID_TOK {mymonitor.add_monitor_var(modulename, var_name);}
                  ;


DECLARATION       :  datatype new_vars
                  |  datatype LSQUARE_TOK expression {myexp.clear();} RSQUARE_TOK new_vars;
                  |  inout_type datatype new_vars
                  |  inout_type new_vars 
                  |  LOCALPARAM_TOK id_token 
                  |  id_token     
                  ;

new_vars          :  ID_TOK {insert_to_netlist(modulename, var_name, "none", data_type);}
                  |  new_vars COMMA_TOK new_vars     
                  |  ID_TOK {insert_to_netlist(modulename, var_name, "none", data_type);var_to_update_val = var_name;} EQUAL_TOK expression   {myexp.insert_into_expression_update_list(var_to_update_val, modulename, initial_state,nba_state);}
                  |  ID_TOK {insert_to_netlist(modulename, var_name, "none", data_type);var_to_update_val = var_name; nba_state = 1;} NBA_TOK expression   {myexp.insert_into_expression_update_list(var_to_update_val, modulename, initial_state,nba_state);nba_state = 0;}
                  ;

datatype          :  INTEGER_TOK        	{strcpy(data_type, yytext);}
                  |  FLOAT_TOK          	{strcpy(data_type, yytext);} 
                  |  STRING_TOK				{strcpy(data_type, yytext);}
                  |  WIRE_TOK					{strcpy(data_type, yytext);}
                  |  REG_TOK					{strcpy(data_type, yytext);}
                  |  WOR_TOK					{strcpy(data_type, yytext);}
                  |  WAND_TOK					{strcpy(data_type, yytext);}
                  |  LOGIC_TOK            {strcpy(data_type, yytext);}
                  ;

id_token          :  id_token COMMA_TOK id_token
                  |  ID_TOK                       
                  |  ID_TOK {var_to_update_val = var_name;} EQUAL_TOK expression {myexp.insert_into_expression_update_list(var_to_update_val, modulename, initial_state,nba_state);}
                  |  ID_TOK {var_to_update_val = var_name;nba_state = 1;} NBA_TOK expression {myexp.insert_into_expression_update_list(var_to_update_val, modulename, initial_state,nba_state);nba_state = 0;}
                  |  ASSIGN_TOK ID_TOK {var_to_update_val = var_name;} EQUAL_TOK expression {assign_var= 1; myexp.insert_into_expression_update_list(var_to_update_val, modulename, initial_state,nba_state); assign_var = 0;}
                  |  ASSIGN_TOK ID_TOK {var_to_update_val = var_name;nba_state = 1;} NBA_TOK expression {assign_var = 1; myexp.insert_into_expression_update_list(var_to_update_val, modulename, initial_state,nba_state);nba_state = 1; assign_var = 0;}
                  |  POSEDGE_TOK ID_TOK
                  ;

expression        :  expression expression_op {myexp.add_ops(yytext, "operator"); } expression
                  |  LPAREN_TOK {myexp.add_ops(yytext, "lparen"); } expression RPAREN_TOK {myexp.add_ops(yytext, "rparen");}
                  |  NOT_TOK {myexp.add_ops(yytext, "operator"); } expression
                  |  constant {myexp.add_ops(yytext, constant_type); }
                  |  ID_TOK  {myexp.add_ops(yytext, "id"); }
                  ;

expression_op     :  PLUS_TOK
                  |  MINUS_TOK
                  |  STAR_TOK
                  |  DIV_TOK
                  |  BIT_OR_TOK
                  |	BIT_AND_TOK
                  |	BIT_XOR_TOK
                  |  cond_op
                  |  SLICE_TOK
                  ;

constant				:  FLOAT_CONST
                  |  STRING_CONST
                  |  INTEGER_CONST
                  |  BINARY_CONST
                  |  OCTAL_CONST
                  |  DECIMAL_CONST
                  |  HEX_CONST
                  ;



%%

int main(int argc, char* argv[])
{
   if(argc != 3){
        fprintf(stderr, "Error: required arguments : <filename> <main_module>");
        exit(1);
   } 
   yyin = fopen(argv[1], "r");
   if (yyin == NULL)
      perror("fopen() error");
   global_void_ptr_for_init = (int*)(&init);
   //alw["always1"] = new always;

   if (yyparse()==0)
   {
      printf("Parsed Successfully\n");
   }
   else printf("\nParsing Error at Line No %d\n", yylineno);

   myexp.trim_list();
   //myexp.print();
   myexp.update_variables();
   //init.print();
   recur_elab(argv[2], argv[2]);
   print_port_conn();
   print_arr_3addr();
   connect_modules();
   //init.execute_all(mymonitor);
   //print_all_modules ();
   fptr.close();


   return(0);	
}

void yyerror(char* s)
{
	printf("yyerror:%s\n",s);
}
