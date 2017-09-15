#CC = arm-none-eabi-gcc

main.exe: main.c source.c header.h
	$(CC) -nostdlib -gdwarf-4 -g3 -o $@ $^

clean:
	-rm main.exe
