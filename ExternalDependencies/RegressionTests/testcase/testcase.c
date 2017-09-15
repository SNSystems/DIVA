/** The below include statements should provoke gcc to insert a
 * DW_LNS_advance_line.
 */
#include <stdio.h>
#include <stdio.h>
#include <stdio.h>
#include <stdio.h>
#include <stdio.h>
#include <stdio.h>
#include <stdio.h>
#include <stdio.h>
#include <stdio.h>

extern int last_line;
int first_line = __LINE__;

int
main(void)
{
    printf("The first line is %d\n", first_line);
    printf("The last line is %d\n", last_line);
    return 0;
}

int last_line = __LINE__;
