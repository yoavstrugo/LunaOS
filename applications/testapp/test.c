#include <stdio.h>

int main(int argc, char** argv) {
    char c;
    while ((c = getchar()) == EOF);
    printf("The entered character is!!! %c", c);
}