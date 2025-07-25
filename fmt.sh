# Format source files
uncrustify -c fmt.cfg --replace src/main.c
uncrustify -c fmt.cfg --replace src/util.c
uncrustify -c fmt.cfg --replace src/viewport.c

# Format header files
uncrustify -c fmt.cfg --replace include/viewport.h
uncrustify -c fmt.cfg --replace include/util.h