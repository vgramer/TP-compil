int x = 5;
// [Valeur: 5]
int y = 10;
// [Valeur: 10]
#include <stdio.h>

int main()
{
printf("%s","TEST si 1");
 if( x < y )
{
    printf("%s","x<y");
} else {
    printf("%s","x>=y");
}

 printf("%s","TEST si 2");
 if( x < y )
{
    printf("%s","y>x");
}
}

"TEST si 1"
"x<y"
"TEST si 2"
"y>x"

result: 1
