#include <stdio.h>
#include <windows.h>

static void WINAPI fiber_run(LPVOID fiber) {
    puts("fiber_run begin");
    // 切换到主纤程中
    SwitchToFiber(fiber);
    puts("fiber_run e n d");

    // 主动切换到主纤程中, 子纤程不会主动切换到主纤程
    SwitchToFiber(fiber);
}

//
// winds fiber hello world
//
int main(int argc, char * argv[]) {
    PVOID fiber, fibec;
    // A pointer to a variable that is passed to the fiber. 
    // The fiber can retrieve this data by using the GetFiberData macro.
    fiber = ConvertThreadToFiberEx(NULL, FIBER_FLAG_FLOAT_SWITCH);
    // 创建普通纤程, 当前还是在主纤程中
    fibec = CreateFiberEx(0, 0, FIBER_FLAG_FLOAT_SWITCH, fiber_run, fiber);
    puts("main ConvertThreadToFiberEx begin");

    SwitchToFiber(fibec);
    puts("main ConvertThreadToFiberEx SwitchToFiber begin");

    SwitchToFiber(fibec);
    puts("main ConvertThreadToFiberEx SwitchToFiber again begin");

    DeleteFiber(fibec);
    ConvertFiberToThread();
    puts("main ConvertThreadToFiberEx e n d");
    return EXIT_SUCCESS;
}
