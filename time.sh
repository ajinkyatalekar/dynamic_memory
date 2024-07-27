start=`date +%s%n`
LD_PRELOAD=./libcsemalloc.so ls /bin
end=`date +%s%n`
echo Execution time was `expr $end - $start` nanoseconds.
