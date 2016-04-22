#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

/* Function pointers to hw3 functions */
void* (*mm_malloc)(size_t);
void* (*mm_realloc)(void*, size_t);
void (*mm_free)(void*);

void load_alloc_functions() {
    void *handle = dlopen("hw3lib.so", RTLD_NOW);
    if (!handle) {
        fprintf(stderr, "%s\n", dlerror());
        exit(1);
    }

    char* error;
    mm_malloc = dlsym(handle, "mm_malloc");
    if ((error = dlerror()) != NULL)  {
        fprintf(stderr, "%s\n", dlerror());
        exit(1);
    }

    mm_realloc = dlsym(handle, "mm_realloc");
    if ((error = dlerror()) != NULL)  {
        fprintf(stderr, "%s\n", dlerror());
        exit(1);
    }

    mm_free = dlsym(handle, "mm_free");
    if ((error = dlerror()) != NULL)  {
        fprintf(stderr, "%s\n", dlerror());
        exit(1);
    }
}

int main() {
    /*
    load_alloc_functions();

    int *data = (int*) mm_malloc(sizeof(int));
    assert(data != NULL);
    data[0] = 0x162;
    mm_free(data);
    printf("malloc test successful!\n");
    */

    /* Marcus: Added Tests */

    /*
    load_alloc_functions();

    int *data = (int*) mm_malloc(sizeof(int));
    assert(data != NULL);
    data[0] = 0x162;
    printf("data[0]: %x\n", data[0]);
    printf("pointer: %d\n", data);
    mm_free(data);
    printf("data[0]: %x\n", data[0]);
    printf("pointer: %d\n", data);
 
    printf("-----------\n");

    data = (int*) mm_malloc(sizeof(int));
    printf("data[0]: %x\n", data[0]);
    printf("pointer: %d\n", data);
    mm_free(data);

    printf("-----------\n");

    printf("malloc test successful!\n");
    */

    // test 2

    /*
    load_alloc_functions();

    int *a = (int*) mm_malloc(sizeof(int)*10);
    int *b = (int*) mm_malloc(sizeof(int)*10);
    b[1] = 1;
    b[2] = 2;
    printf("should be 2: %d\n", b[2]);
    mm_free(a);
    int *c = (int*) mm_malloc(sizeof(int)*1000);
    mm_realloc(c, sizeof(int)*10);
    printf("should be 2: %d\n", b[2]);
    */

    // test 3

    load_alloc_functions();

    printf("making a\n");
    char *a = (char*) mm_malloc(sizeof(char)*100);
    printf("making b\n");
    char *b = (char*) mm_malloc(sizeof(char)*100);

    printf("freeing a\n");
    mm_free(a);

    printf("making c\n");
    char *c = (char*) mm_malloc(sizeof(char)*10);
    printf("making d\n");
    char *d = (char*) mm_malloc(sizeof(char)*10);

    printf("a: %d; b: %d; c: %d; d: %d\n", a, b, c, d);
    printf("b - a: %d; c - a %d; d - a: %d\n", b - a, c - a, d - a);


    /* Marcus: End Added Tests */
    return 0;
}
