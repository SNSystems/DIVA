int main() {
    int x;
    int y;
    int z;

    x = 0;
    {
        y = 1;
    }
    z = 2;

    x = y; y = z; z = x;
}
