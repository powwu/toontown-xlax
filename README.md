# toontown-xlax
A patch of xlax for use as a Toontown multicontroller on Linux.

![](https://powwu.s-ul.eu/hJRxrTjV)

## Building
In the toontown-xlax directory:
```
xmkmf
```
```
make
```
Then move the `xlax` output to a directory listed in your PATH. (or just run it from there, I'm not the boss of you)

##

Original README:
```
xlax is a program that allows output to
be sent to several windows (via SendEvent()).

It can be useful for some tasks that require almost
the same thing to be done on different machines.

Send questions, comments or whatever to 
fine@head.cfa.harvard.edu

Author: Frank Adelstein, with code for window selection
taken from "dsimple" in MIT clients, and "vroot.h" used
so it'll work under tvtwm.
Version2.0 Author: Thomas A. Fine, added code for finding
windows based on the name hint, also improved "send string"
setting, and automatic dropping of dead windows.

To compile:
  #go to the directory where you untarred the distiribution
  xmkmf
  make
  #then hand-install xlax, mkxlax, xlax.man, and mkxlax.man

For more information, see the accompanying man page, distributed
as "xlax.man", and also see:
  http://hea-www.harvard.edu/~fine/Tech/xlax.html

Also included is the perl script mkxlax, which is documented in
"mkxlax.man".  This comes with a sample (optional) configuration
file, "dot-mkxlax", which (if used) belongs in your home directory
as ".mkxlax".

The first line of the perl-script may need to be hand-edited to the
proper location for perl, if it is not installed as "/usr/bin/perl".

Standard X copying policies apply (see the copyright notice
at the beginning of code for full details).

--Frank Adelstein.
--Thomas A. Fine.
```
