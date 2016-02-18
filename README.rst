=========
Cache2Sim
=========
:Info: 2 Level cache simulator with VictimCache™
:Authors: Yoel Ivan (yivan3@gatech.edu)

Compiling
=========

+ To compile with g++ do:
    ``$ make``

+ To compile with cc do:
    ``$ make C=1``

+ In any case, to clean do:
    ``$ make clean``
    
File Information
================

+ **cache.h** Header file for cache object

+ **cache.cpp** Implementation of cache object

+ **\*.log** Output of *Cache2Sim* 

+ **test.sh** Script for automated compile, run, and verify

+ **verify.py** basically a diff

Best Configurations
===================

+ **A\***::

Best AAT: 2.804164
Best Configuration (c, b, s, v, C, B, S): (12, 12, 0, 4, 17, 12, 4)
Simulated Configuration: 428627
Invalid Configuration: 417523
Failed Configuration: 0

+ **bzip2**::

Best AAT: 2.000206
Best Configuration (c, b, s, v, C, B, S): (15, 12, 0, 3, 17, 14, 0)
Simulated Configuration: 428627
Invalid Configuration: 417523
Failed Configuration: 0

+ **mcf**::

Best AAT: 2.121731
Best Configuration (c, b, s, v, C, B, S): (15, 10, 0, 4, 17, 13, 4)
Simulated Configuration: 428627
Invalid Configuration: 417523
Failed Configuration: 0

+ **perlbench**::

Best AAT: 2.909599
Best Configuration (c, b, s, v, C, B, S): (15, 7, 0, 4, 17, 7, 7)
Simulated Configuration: 428627
Invalid Configuration: 417523
Failed Configuration: 0

