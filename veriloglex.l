%{
#include<stdio.h>
#include "verilogyacc.tab.h"
char var_name[100];
char prev_name[100];
char constant_type[100];
char intvalue[100];
int initial_state = 0, nba_state = 0, module_port = 0, always_no = 1, initial_no = 0, assign_no = 0;
void * global_void_ptr_for_init = NULL;
%}

%%
\n  									{yylineno++; }
" "                                     {}
\t                                      {}
\/\/.*                                  {}
\=\=                                    {return COND_EQUAL_TOK;}
\*\*                                    {return POWER_TOK;}
\|\|                                    {return COND_OR_TOK;}
\&\&                                    {return COND_AND_TOK;}
\<\=                                    {return NBA_TOK; }
\;                                      {return(SEMICOLON_TOK);}
\,                                      {return COMMA_TOK;}
\{                                      {return(LCRLY_TOK);}
\}                                      {return(RCRLY_TOK);}
\(				                        {return(LPAREN_TOK);}
\)				                        {return(RPAREN_TOK);}
\<				                        {return(LT_TOK);}
\>				                        {return(GT_TOK);}
\=                                      {return EQUAL_TOK;}
\!                                      {return NOT_TOK;}
\+                                      {return PLUS_TOK;}
\-                                      {return MINUS_TOK;}
\*                                      {return STAR_TOK;}
\/                                      {return DIV_TOK;}
\@										{return AT_TOK;}
\^                                      {return BIT_XOR_TOK; }
\|                                      {return BIT_OR_TOK; }
\&                                      {return BIT_AND_TOK;}
\[                                      {return LSQUARE_TOK; }
\]                                      {return RSQUARE_TOK; }
\:                                      {return SLICE_TOK; }
and 									{return AND_TOK;}
or 										{return OR_TOK; }
not 									{return NOT_TOK;}
xor                                     {return XOR_TOK;}
begin                                   {return BEGIN_TOK;}
end                                     {return END_TOK;}
assign									{return ASSIGN_TOK;}
module    								{return MODULE_TOK; }
endmodule 								{return ENDMODULE_TOK; }
always									{initial_state = 2; return ALWAYS_TOK; }
initial									{initial_no++ ; return INITIAL_TOK; }
for										{return FOR_TOK; }
while									{return WHILE_TOK; }
input     								{return INPUT_TOK; } 
output    								{return OUTPUT_TOK; }
inout     								{return INOUT_TOK; }
reg       								{return REG_TOK; }
wire      								{return WIRE_TOK; } 
wor       								{return WOR_TOK; }
wand      								{return WAND_TOK; }
logic                                   {return LOGIC_TOK;}
parameter                               {return PARAMETER_TOK;}
$monitor								{return MONITOR_TOK; }
$display                                {return DISPLAY_TOK; }
integer                                 {return INTEGER_TOK; }
float                                   {return FLOAT_TOK;}
string                                  {return STRING_TOK;}
if                                      {return IF_TOK; }
else                                    {return ELSE_TOK; }
posedge                                 {return POSEDGE_TOK; }
localparam                              {return LOCALPARAM_TOK; }
case                                    {return CASE_TOK; }
endcase                                 {return ENDCASE_TOK; }
\#                                      {return DELAY_TOK; }
([a-z]|[A-Z]|\_)([a-z]|[A-Z]|[0-9]|\_)* {strcpy(prev_name , var_name); strcpy(var_name ,yytext); return ID_TOK;}
[1-9][0-9]*|0  					        {strcpy(constant_type ,"int");strcpy(intvalue, yytext); return INTEGER_CONST; }
[0-9]+\.[0-9]+                          {strcpy(constant_type ,"float");return FLOAT_CONST;} 
[0-9]*'[Bb][01_xXzZ]+  			        {strcpy(constant_type ,"binary");return BINARY_CONST; }
[0-9]*'[Oo][0-7_xXzZ]+			        {strcpy(constant_type ,"octal");return OCTAL_CONST; }
[0-9]*'[Dd][0-9_xXzZ]+			        {strcpy(constant_type ,"decimal");return DECIMAL_CONST; }
[0-9]*'[Hh][0-9a-fA-F_xXzZ]+ 	        {strcpy(constant_type ,"hex");return HEX_CONST; }
\".*\"                                  {return STRING_CONST;}
\'.*\'                                  {return STRING_CONST;}
%%

int yywrap()
{
    return(1);
}
/*
int main(int argc, char *argv[])
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
}
*/
