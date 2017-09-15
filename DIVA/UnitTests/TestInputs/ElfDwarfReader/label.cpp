void test() {
    int x = 5;
loophead:
    --x;
    if (x != 0) {
        goto loophead;
    }
}
