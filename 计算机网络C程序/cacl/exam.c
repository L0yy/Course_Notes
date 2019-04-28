#include <stdio.h>
#include <stdlib.h>


int main(int argc, char const *argv[])
{   
    printf("   /**    \n");
    printf("*           x     x    \n");
    printf("*          0 0   0 0    \n");
    printf("*          0 0   0 0    \n");
    printf("*    00000000000000000    \n");
    printf("*   0.0                0    \n");
    printf("*   0.0    x    0 0     0    \n");
    printf("*    0000000       [x]  0    \n");
    printf("*          0    U       0    \n");
    printf("*           0          0    \n");
    printf("*            000000000    \n");
    printf("*          0          0__    \n");
    printf("*         /0          0  |    \n");
    printf("*      __/ 0   0   0  0 _|    \n");
    printf("*       /\  00000000000 /\    \n");
    printf("*              0   0    \n");
    printf("*              0   0    \n");
    printf("*            000 000    \n");
    printf("*    \n");
    printf("*/    \n");
    while (1)
    {

    int num1 = rand()%10;
    int num2 = rand()%10;
    int res,cres;

    num1 = rand()%10;
    num2 = rand()%10;
    if (num1%2)
    {
        res = num1 + num2;
        printf("输入你计算结果: %d + %d =____\n",num1+num2);
    }
    else
    {
        res = num1 - num2;
        printf("输入你计算结果: %d - %d =____\n",num1-num2);
    }
    
    printf("请输入你的结果：");
    scanf("%d",&cres);
    
    }
    return 0;
}
