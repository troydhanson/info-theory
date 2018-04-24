# lz4 compression

## install

    sudo apt install liblz4-dev

### headers

    sudo dpkg-query --listfiles liblz4-dev
		...
		/usr/include/lz4hc.h     # high compression
		/usr/include/lz4frame.h  # standalone framed buffers
		/usr/include/lz4.h       # standard api
		...

### configure macro

See `lz4.m4` for an example configure.ac snippet to check for it.

## framed buffers

The lz4frame.h header provides a standalone API for interoperable
framed buffers. It has a "simple API" and a full API. The simple
API uses only two functions for compression; one for bounds sizing
and one for compression.

### example

See `do-lz4.c` and `undo-lz4.c`.

    % make
    % ./do-lz4 /etc/passwd > p.lz4
    mapped /etc/passwd: 1525 bytes
    compressed to 942 bytes

    % ./undo-lz4 p.lz4 >/tmp/passwd
    mapped p.lz4: 942 bytes
    decompressed 512 bytes
    decompressed 512 bytes
    decompressed 501 bytes
    % diff /etc/passwd /tmp/passwd

