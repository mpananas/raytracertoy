clear
flags="-fcompare-debug-second -Wall -static-libgcc -g"
gcc $flags *.c -c
gcc *.o -o p -lglfw3 -lm -lGLEW -lGLU -lGL
rm -f *.o
if [ "$1" = "x" ]; then
  valgrind --leak-check=yes --log-file=valgrind.rpt ./p
fi
