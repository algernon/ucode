DESTDIR = /usr/bin

ucode: ucode.c
	gcc -std=gnu11 -pedantic ucode.c -o ucode -lX11 -lxdo
install: ucode
	install -D ucode ${DESTDIR}/ucode
clean:
	rm -f ucode
