#include <stdio.h>
#include <stdlib.h>

#define R_INT         (128)
#define F_INT         (100000000)

// getr - 得到 rand() 返回值, 并写入到文件中
static int getr(long long *pnt) {
    static int cnt;

    int r = rand();
    long long t = *pnt + 1;
    
    // 每次到万再提醒一下
    if(t % F_INT == 0)
        fprintf(stdout, "%d 个数据跑完了[%d, %lld]\n", F_INT, cnt, t);

    if(t < 0) { // 数据超标了
        ++cnt;

        fprintf(stderr, "Now %d T > %lld\n", cnt, t - 1);
        *pnt = 0; // 重新开始一轮
    }
    *pnt = t;
    return r;
}

// main - 验证 rand 函数的周期
int main(int argc, char* argv[]) {
    int base[R_INT];
    int r, i = -1;
    long long cnt = 0;


    // 先产生随机函数
    while(++i < R_INT)
        base[i] = getr(&cnt);

    // 这里开始随机了
    for(;;) {
        r = getr(&cnt);
        if (r != base[0])
            continue;

        // 继续匹配查找
        for(i = 1; i < R_INT; ++i) {
            r = getr(&cnt);
            if(r != base[i]) 
                break;
        }

        // 找见了数据
        if(i == R_INT) {
            printf("Now T = %lld\n", cnt);
            break;
        }
    }

    return EXIT_SUCCESS;
}
