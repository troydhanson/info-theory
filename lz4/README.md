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

## framed buffers

### compression

The lz4frame.h header provides a standalone API for interoperable
framed buffers. It has a "simple API" and a full API. The simple
API uses only two functions for compression; one for bounds sizing
and one for compression.

    make
    ./do-lz4 /etc/passwd
    mapped /etc/passwd: 1525 bytes
    compressed to 942 bytes


