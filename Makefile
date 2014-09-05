all:
	gcc -std=c99 -pedantic ucode.c -o ucode -lX11 -lxdo 
