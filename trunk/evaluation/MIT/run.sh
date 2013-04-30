for PROGRAM in blur test1 test2 reduction_schedule bilateral_grid camera_pipe local_laplacian
do
  echo ${PROGRAM}
  g++ -std=c++0x -w ${PROGRAM}.cpp -L ../halide/ -lHalide -ldl -lpthread
  ./a.out
  echo ""
done
rm a.out
