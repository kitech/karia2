sed   's/libng\\\\/libng\//g' Makefile.Release| sed 's/stream\\\\/stream\//g'| sed 's/release\\\\/release\//g' | sed 's/plugins\\\\/plugins\//g'| sed 's/d:\\\\Qt\\\\MGW-4.2.2\\\\bin\\\\moc.exe/d:\/Qt\/MGW-4.2.2\/bin\/moc.exe/g' | sed 's/libhttpd\\\\/libhttpd\//g'  | sed 's/= del/= rm/g' > RMakefile


del Makefile.Release

ren RMakefile Makefile.Release

sed   's/libng\\\\/libng\//g' Makefile.Debug| sed 's/stream\\\\/stream\//g'| sed 's/release\\\\/release\//g'| sed 's/debug\\\\/debug\//g' |sed 's/plugins\\\\/plugins\//g'| sed 's/d:\\\\Qt\\\\MGW-4.2.2\\\\bin\\\\moc.exe/d:\/Qt\/MGW-4.2.2\/bin\/moc.exe/g'  | sed 's/libhttpd\\\\/libhttpd\//g' | sed 's/= del/= rm/g' > DMakefile


del Makefile.Debug

ren DMakefile Makefile.Debug

