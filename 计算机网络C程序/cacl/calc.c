#include <stdio.h>
#include <string.h>
#include <getopt.h>

const struct option long_opts[]=
{   { "add",       1, NULL,      'a' },
    { "sub",       1, NULL,      's' },
    { "mul",       1, NULL,      'm' },
    { "div",       1, NULL,      'd' },
    { "o",         1, NULL,      'o' },
    { "p",         1, NULL,      'p' },
    { "help",      1, NULL,      'h' },
};
const const char short_opts[] = 
    "a::s::m::d::o:p:h";

int main(int argc,char *argv[])
{  
    int ret,optint,num1,num2;
    float optfloat;
    int chidx = -1;
    char c,c2,c3;
    c = getopt_long (argc, argv,short_opts, long_opts, &chidx);
    switch (c){
    case 'a':
        c2 = getopt_long (argc, argv,short_opts, long_opts, &chidx);
        if (c2 = 'o')
            sscanf (optarg, "%d", &num1);
            printf("[*]num1===%d \n",num1);

        c3 = getopt_long (argc, argv,short_opts, long_opts, &chidx);
        if (c3 = 'p'){
            sscanf (optarg, "%d", &num2);
            printf("[*]num2===%d \n",num2);
            printf("%d+%d=%d",num1,num2,num1+num2);
            break;
        }
    case 's':
        c2 = getopt_long (argc, argv,short_opts, long_opts, &chidx);
        if (c2 = 'o')
            sscanf (optarg, "%d", &num1);
            printf("[*]num1===%d \n",num1);

        c3 = getopt_long (argc, argv,short_opts, long_opts, &chidx);
        if (c3 = 'p'){
            sscanf (optarg, "%d", &num2);
            printf("[*]num2===%d \n",num2);
            printf("%d-%d=%d",num1,num2,num1-num2);
            break;
        }    
    case 'm':
        c2 = getopt_long (argc, argv,short_opts, long_opts, &chidx);
        if (c2 = 'o')
            sscanf (optarg, "%d", &num1);
            printf("[*]num1===%d \n",num1);

        c3 = getopt_long (argc, argv,short_opts, long_opts, &chidx);
        if (c3 = 'p'){
            sscanf (optarg, "%d", &num2);
            if (num2 == 0){
                printf("[X] diviso can not is 0");
                break;}
            printf("[*]num2===%d \n",num2);
            printf("%d*%d=%d",num1,num2,num1*num2);
            break;
        }  
    case 'd':
        c2 = getopt_long (argc, argv,short_opts, long_opts, &chidx);
        if (c2 = 'o')
            sscanf (optarg, "%d", &num1);
            printf("[*]num1===%d \n",num1);

        c3 = getopt_long (argc, argv,short_opts, long_opts, &chidx);
        if (c3 = 'p'){
            sscanf (optarg, "%d", &num2);
            printf("[*]num2===%d \n",num2);
            printf("%d / %d=%f",num1,num2,num1/num2);
            break;
        }  
    case 'h':
        printf("    -a    add example: calc.exe -a -o 1 -p 2   \n");
        printf("    -m    mul example: calc.exe -m -o 2 -p 1   \n");
        printf("    -d    div example: calc.exe -d -o 1 -p 2   \n");
        printf("    -s    sub example: calc.exe -s -o 1 -p 2   \n");
        printf("    -h    printf descripts        \n");
        
        break;

    default:
    printf(" Unknown option!  pleace input -h to get help \n");
    break;
    }
}