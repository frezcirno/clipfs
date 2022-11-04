clipfs: clipfs.c
	gcc -Wall -o $@ $^ $$(pkg-config fuse3 --cflags --libs) $$(pkg-config libclipboard --cflags --libs) -lxcb

.PHONY: clean
clean:
	rm -f clipfs
