#define F_CPU 1000000UL  // 1 MHz

#include <util/delay.h>

void main(void)
{
    int i = 0;
    for(i = 0; i < 10; i++)
    {
        _delay_ms(100);
    }
}
