zlib example

gz: a simple implementation of "gzip" using zlib's C API
ungz: a simple "gunzip" implementation based on same API

You need to have zlib's development headers and library
installed.  On Ubuntu you can get them like this:

    sudo apt-get install zlib1g-dev

Usage 

Run 'make' to build gz and ungz. Then:

    gz somefile > somefile.gz
    ungz somefile.gz
