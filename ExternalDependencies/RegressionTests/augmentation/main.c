#include <stdio.h>

void print_sample(char * str)
{
   printf("%s\n",str);
   print_sample("Done!");
}

int main()
{
    for (int i=0; i<10; i++)
        print_sample("printing...");

    return 0;
}
