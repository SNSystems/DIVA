// Test compound types involving function pointers.

typedef int INT;
typedef INT (**ptr_to_fptr)(INT, int);

void foo() {
    ptr_to_fptr p;
}
