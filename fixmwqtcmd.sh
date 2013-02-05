sed  's/qjson\\src\\/qjson\/src\//g' Makefile.Release > mymakefile
sed  's/libng\\/libng\//g' mymakefile > mymakefile2
sed  's/libmaia\\/libmaia\//g' mymakefile2 > mymakefile3
sed  's/tmp\\/tmp\//g' mymakefile3 > mymakefile4

