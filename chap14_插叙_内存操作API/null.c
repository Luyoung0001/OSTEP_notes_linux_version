#include <stdio.h>
#include <stdlib.h>
int main(){
    int a;
     scanf("%d",&a);
    int *num = malloc(10);
    printf("hello\n");
    num = NULL;
    free(num);
    return 0;
}
// man 3 free用法，是这样说的：

//"The free() function frees the memory space pointed to by ptr, which
// must have been returned by a previous call to malloc(), calloc() or realloc().
// Otherwise, or if free(ptr) has already been called before, undefined  behavior occurs.
// If ptr is NULL, no operation is performed."
