# Clipfs - Clipboard File Interface

A fuse program expose your clipboard as a pseudo filesystem (just like the /dev/null).

# Dependency

- fuse3
- libfuse-dev
- [libclipboard](https://github.com/jtanx/libclipboard)

# Build

```bash
$ make
```

# Start

```bash
# Mount the clipfs
$ CLIPFS=/run/user/$(id -u)/clipfs
$ mkdir $CLIPFS
$ ./clipfs $CLIPFS

# Make a symbolic link from <your home>/clip, for convenience
$ ln -s $CLIPFS/clip $HOME/clip
```

# Usage

1. Ctrl-C to copy any text.
2. `cat ~/clip` and you can get it.
3. `cp some_file ~/clip`
4. Ctrl-V to paste the content.
