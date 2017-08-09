zlib example
------------

Back up [parent page](https://github.com/troydhanson/info-theory).

This code shows how to use the zlib C API. The documentation
for the zlib C API is in the zlib.h header file. On Ubuntu 
that is normally found in `/usr/include/zlib.h`.

 * gz: a simple implementation of "gzip" 
 * ungz: a simple "gunzip" implementation 

You need to have zlib's development headers and library
installed.  

    #ubuntu
    sudo apt-get install zlib1g-dev 

    #centos/rhel
    sudo yum install zlib-devel 

Usage 

Run 'make' to build gz and ungz. Then:

    ./gz file > file.gz
    ./ungz file.gz
