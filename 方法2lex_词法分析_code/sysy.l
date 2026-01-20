%{
#include <stdio.h>
#include <string.h>

int lines = 1;
int chars = 0;
int keywords = 0;
int ids = 0;
int ints = 0;
int ops = 0;
int errs = 0;

void count() {
    chars += yyleng;
}
%}

DIGIT       [0-9]
LETTER      [a-zA-Z_]
ID          {LETTER}({LETTER}|{DIGIT})*
NUMBER      [1-9][0-9]*|0|0[0-7]+|0[xX][0-9a-fA-F]+

%x COMMENT

%%

"//".* { count(); }

"/*"            { count(); BEGIN(COMMENT); }
<COMMENT>"*/"   { count(); BEGIN(INITIAL); }
<COMMENT>\n     { count(); lines++; }
<COMMENT>.      { count(); }

"int"|"void"|"return"|"const"|"if"|"else"|"while"|"break"|"continue" {
    count(); 
    keywords++;
    printf("[KEYWORD] %s (Line: %d)\n", yytext, lines); 
}

{ID} {
    count();
    ids++;
    printf("[ID]      %s (Line: %d)\n", yytext, lines);
}

{NUMBER} {
    count();
    ints++;
    printf("[INT]     %s (Line: %d)\n", yytext, lines);
}

"+"|"-"|"*"|"/"|"%"|"="|"<"|">"|"!"|";"|","|"("|")"|"["|"]"|"{"|"}" {
    count();
    ops++;
    printf("[OP]      %s (Line: %d)\n", yytext, lines);
}

\n      { count(); lines++; }
[ \t\r] { count(); }

. {
    count();
    errs++;
    printf("[ERROR]   Unknown character '%s' at Line %d\n", yytext, lines);
}

%%

int yywrap() { return 1; }

int main(int argc, char** argv) {
    if (argc > 1) {
        if (!(yyin = fopen(argv[1], "r"))) {
            perror(argv[1]);
            return 1;
        }
    }

    printf("=== Start Analysis ===\n");
    yylex();
    printf("=== End Analysis ===\n\n");

    printf("--- Statistics ---\n");
    printf("Lines: %d\n", lines);
    printf("Chars: %d\n", chars);
    printf("Keywords: %d\n", keywords);
    printf("Identifiers: %d\n", ids);
    printf("Errors: %d\n", errs);

    return 0;
}