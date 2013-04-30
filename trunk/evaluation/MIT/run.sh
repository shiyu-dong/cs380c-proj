#for PROGRAM in blur test1 test2 reduction_schedule
for PROGRAM in bilateral_grid
do
  echo ${PROGRAM}
  g++ -std=c++0x -w ${PROGRAM}.cpp -L ../halide/ -lHalide -ldl -lpthread
  ./a.out
  echo ""
done
rm a.out
