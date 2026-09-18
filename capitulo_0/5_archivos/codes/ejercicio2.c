/*
 * Exercise 2: Save username in config.log
 */
#include <stdio.h>
#include <stdlib.h>

int main(){
    char name[50];
    printf("Enter your name: ");
    scanf("%s", name);

    FILE *fl = fopen("config.log", "a");
    if(!fl){
        printf("Log opening error\n");
        return 1;
    }

    fprintf(fl, "Registered user: %s\n", name);
    printf("Name saved in config.log\n");

    fclose(fl);
    return EXIT_SUCCESS;
}
