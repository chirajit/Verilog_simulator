%{
#include<iostream>
#include "asmyacc.tab.h"
using namespace std;
int intval;
string label_name;

void yyerror(char* );
%}

%%
\n										{yylineno++ ;}
" "                                     {}
\t                                      {}
\-\>                                    {return ARROW_TOK ;}
\:										{return SLICE_TOK ;}
\,                                      {return COMMA_TOK;}
\(				                        {return LPAREN_TOK;}
\)				                        {return RPAREN_TOK ;}
INIT_ARCH								{return INIT_ARCH_TOK ;}
CREATE_REG								{return CREATE_REG_TOK ;}
CREATE_WIRE								{return CREATE_WIRE_TOK ;}
CREATE_INITIAL_PROC						{return CREATE_INITIAL_PROC_TOK ;}
CREATE_ALWAYS_PROC						{return CREATE_ALWAYS_PROC_TOK ;}
CREATE_ASSIGN_PROC						{return CREATE_ASSIGN_PROC_TOK ;}
ADD_TRIGGERS							{return ADD_TRIGGERS_TOK; }
PORT_CONNECTION							{return PORT_CONNECTION_TOK ;}
ARCH\+  								{return ARCH_TOK ;}
ADD_DRIVER                              {return ADD_DRIVER_TOK; }
MOVE									{return MOVE_TOK ;}
STORE									{return STORE_TOK ;}
LOAD									{return LOAD_TOK ;}
AND										{return AND_TOK ;}
OR										{return OR_TOK ;}
NOT										{return NOT_TOK ;}
NBA                                     {return NBA_TOK; }
UPDATE_DRIVER                           {return UPDATE_DRIVER_TOK; }
CREATE_MONITOR                          {return CREATE_MONITOR_TOK; }
DISPLAY                                 {return DISPLAY_TOK; }
END_DISPLAY                             {return END_DISPLAY_TOK; }
([a-z]|[A-Z]|\_)([a-z]|[A-Z]|[0-9]|\_)* {label_name = yytext; return ID_TOK;}
1'b[0-1]								{return BINARY_CONST; }
[0-9]*				       				{intval = stoi (yytext) ; return INTEGER_CONST; }
%%

int yywrap()
{
    return(1);
}
/*int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        printf("Error: add only a filename\n");
        exit(-1);
    }

    yyin = fopen(argv[1], "r");
    if (yyin == NULL)
        perror("fopen() error");

    int val;
    while((val = yylex()) > 0)
        printf("\rToken === %s\tToken ID === %d\n", yytext, val); 
           
    
    return 0;
}*/