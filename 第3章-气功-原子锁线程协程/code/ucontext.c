#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <ucontext.h>

#define PERROR_EXIT(msg)    \
do { perror(msg); exit(EXIT_FAILURE); } while (0)

static void func1(uint32_t l32, uint32_t h32) {
    // 得到所传入的指针类型
    uintptr_t ptr = (uintptr_t)l32 | ((uintptr_t)h32 << 32);
    ucontext_t * uts = (ucontext_t *)ptr;

    puts("func1: started");
    puts("func1: swapcontext(uts + 1, uts + 2)");
    if (swapcontext(uts + 1, uts + 2) < 0)
        PERROR_EXIT("swapcontext");
    puts("func1: returning");
}

static void func2(uint32_t l32, uint32_t h32) {
    uintptr_t ptr = (uintptr_t)l32 | ((uintptr_t)h32 << 32);
    ucontext_t * uts = (ucontext_t *)ptr;

    puts("func2: started");
    puts("func2: swapcontext(uts + 2, uts + 1)");
    if (swapcontext(uts + 2, uts + 1) < 0)
        PERROR_EXIT("swapcontext");
    puts("func2: returning");
}

//
// use ucontext hello world
//
int main(int argc, char * argv[]) {
    ucontext_t uts[3];
    char stack1[16384];
    char stack2[16384];
    uintptr_t ptr = (uintptr_t)uts;
    uint32_t l32 = (uint32_t)ptr;
    uint32_t h32 = (uint32_t)(ptr >> 32);

    // uts[1] -> uts[0]
    if (getcontext(uts + 1) < 0)
        PERROR_EXIT("getcontext");
    uts[1].uc_stack.ss_sp = stack1;
    uts[1].uc_stack.ss_size = sizeof stack1;
    uts[1].uc_link = uts;
    makecontext(uts + 1, (void (*)())func1, 2, l32, h32);

    // uts[2] -> uts[1]
    if (getcontext(uts + 2) < 0)
        PERROR_EXIT("getcontext");
    uts[2].uc_stack.ss_sp = stack2;
    uts[2].uc_stack.ss_size = sizeof stack2;
    uts[2].uc_link = uts + 1;
    makecontext(uts + 2, (void (*)())func2, 2, l32, h32);

    puts("main: swapcontext(uts, uts + 2)");
    if (swapcontext(uts, uts + 2) < 0)
        PERROR_EXIT("swapcontext");

    puts("main: exiting");
    return EXIT_SUCCESS;
}
