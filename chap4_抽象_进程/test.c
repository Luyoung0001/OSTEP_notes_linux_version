#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
int main(){
    close(STDOUT_FILENO);
    open("./test.out",O_CREAT|O_WRONLY|O_TRUNC,S_IRWXU);
    printf("this is a text\n");
    return 0;
}