# cpmtoolbox
A tool box of cp/m code that is known to run on cp/m 
This code repo contains cp/m programs that are known to build and run on cp/m systems.

These tools comprise code from Leor Zolman and other to be named.  Of course I will also
add some tools.

All the code here has been built by me on a cp/m system and tested, to at least work at the
surface level.  There are no garentees that its bugfree.  At least you will be able to build
the code and have it work on your machine.

I have found over the years that source code from places like walnut creek cd and so forth
may not build or run on my cp/m systems.  (I have a bunch of them).  This can be issues with
code feature creep or build tool feature creep.  Anyway these are never frozen in time.  I
will try to document the build tools used for each program here.

```
Souce of tools.

cat.c:      CAT will copy a file to another file (cat filename >newfilename)
sieve.c     SIEVE will calculate about 1900 primes.  This can be used to compute the system speed
            of your machine.
tail.c      TAIL will print the last lines of a source file.
wc.c        WC will compute the number of words/lines, and characters in a file.
xc.c        XC will create a cross referance listing of a C program.
sort.c      SORT is not ready yet.
uniq.c      UNIQ is not ready yet.

Libraries and headers used by the builds.

wildexp.c
dio.c
dio.h
trace.h
```
