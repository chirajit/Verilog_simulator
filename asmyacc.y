%{
#include "lex.yy.c"
#include "asmcpp.cpp"
%}

%token SLICE_TOK 
%token COMMA_TOK
%token LPAREN_TOK
%token RPAREN_TOK 
%token INIT_ARCH_TOK 
%token CREATE_REG_TOK 
%token CREATE_WIRE_TOK
%token CREATE_INITIAL_PROC_TOK 
%token CREATE_ALWAYS_PROC_TOK 
%token CREATE_ASSIGN_PROC_TOK 
%token ADD_TRIGGERS_TOK
%token PORT_CONNECTION_TOK
%token ARCH_TOK 
%token MOVE_TOK 
%token STORE_TOK 
%token LOAD_TOK 
%token AND_TOK 
%token OR_TOK 
%token NOT_TOK 
%token INTEGER_CONST
%token ID_TOK
%token BINARY_CONST
%token ADD_DRIVER_TOK
%token UPDATE_DRIVER_TOK
%token ARROW_TOK
%token CREATE_MONITOR_TOK
%token NBA_TOK
%token DISPLAY_TOK
%token END_DISPLAY_TOK

%start S

%%
S  			: func S
				| oper S
				| label S
				|
				;

oper 			: operators args 
				;

func 			: func_name LPAREN_TOK parameters RPAREN_TOK {execute_function();}
				;

parameters 		: parameters INTEGER_CONST {parameters.push_back(yytext);}
				| parameters COMMA_TOK
				| parameters ID_TOK {parameters.push_back(yytext);}
				| parameters ARCH_TOK INTEGER_CONST {parameters.push_back(yytext);}
				|
				;

func_name 	: INIT_ARCH_TOK { func_name = INIT_ARCH ;}
				| CREATE_REG_TOK { func_name = CREATE_REG ;}
				| CREATE_WIRE_TOK { func_name = CREATE_WIRE ;}
				| CREATE_INITIAL_PROC_TOK { func_name = CREATE_INITIAL_PROC ;}
				| CREATE_ALWAYS_PROC_TOK { func_name = CREATE_ALWAYS_PROC ;}
				| CREATE_ASSIGN_PROC_TOK { func_name = CREATE_ASSIGN_PROC ;}
				| ADD_TRIGGERS_TOK { func_name = ADD_TRIGGERS ;}
				| PORT_CONNECTION_TOK { func_name = PORT_CONNECTION ;}
				| ADD_DRIVER_TOK {func_name = ADD_DRIVER ;}
				| CREATE_MONITOR_TOK {func_name = CREATE_MONITOR; }
				;

operators 	: STORE_TOK
				| MOVE_TOK
				| LOAD_TOK
				| OR_TOK
				| AND_TOK
				| NOT_TOK
				| UPDATE_DRIVER_TOK
				| NBA_TOK STORE_TOK
				| NBA_TOK MOVE_TOK
				| NBA_TOK UPDATE_DRIVER_TOK
				| DISPLAY_TOK
				| END_DISPLAY_TOK
				;

args 			: args ID_TOK  {parameters.push_back(yytext); }
				| args ARCH_TOK INTEGER_CONST {parameters.push_back(yytext); }
				| args BINARY_CONST {parameters.push_back(yytext); }
				| 
				;

label 			: ARROW_TOK ID_TOK SLICE_TOK 
				;
%%

int main(int argc, char* argv[])
{
   if(argc != 2){
        fprintf(stderr, "Error: required arguments : <filename> ");
        exit(1);
   } 
   yyin = fopen(argv[1], "r");
   if (yyin == NULL)
   {
      perror("fopen() error");
      exit(1);
   }

	if (yyparse()==0)
    {
      printf("Parsed Successfully\n");
    }
	else printf("\nParsing Error at Line No %d\n", yylineno);
	second_read(argv[1]);
	//print_arch();
	pque.schedule0();
	port_to_trig();
	pque.execute_all(argv[1]);
	return 0;
}

void yyerror(char* s)
{
	cout << "yyeror : "<< s << endl;
}