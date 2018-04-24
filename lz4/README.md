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

### documentation

The headers themselves document the API clearly and succinctly.

### configure macro

See `lz4.m4` for an example configure.ac snippet to check for it.

# Framed vs Regular API

Both API are simple to use.

 * The framed API is for "inter-operable" buffers. I think that
   means, because they encode their decompressed lengths, it is
   more convenient to send them to recipient that wouldn't know 
 * The framed API uses `size_t` for length arguments
 * The "regular" API requires you to know (or estimate above)
   the decompressed size prior to decompression.
 * The regular API uses `int` for length arguments

## framed buffers: lz4frame.h

The lz4frame.h header provides a standalone API for interoperable
framed buffers.

### example

See `frame-lz4.c` and `unframe-lz4.c`.

    % make
    % ./fram-lz4 /etc/passwd > p.lz4
    mapped /etc/passwd: 1525 bytes
    compressed to 942 bytes

    % ./unfram-lz4 p.lz4 >/tmp/passwd
    mapped p.lz4: 942 bytes
    decompressed 512 bytes
    decompressed 512 bytes
    decompressed 501 bytes
    % diff /etc/passwd /tmp/passwd

## regular buffers - lz4.h

An apparently lower-level interface giving the programmer more
control over the buffer management is found in lz4.h.

    % ./do-lz4 /tmp/passwd > p.lz4
    mapped /tmp/passwd: 1525 bytes
    compressed to 927 bytes
    % ./undo-lz4 p.lz4 1525 > p
    mapped p.lz4: 927 bytes
    decompressed: 1525 bytes
    % diff /etc/passwd p

