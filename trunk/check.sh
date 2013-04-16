./clean.sh
for PROGRAM in base_test rdom_test split_reuse_inner_name_bug two_vector_args
do
  echo ${PROGRAM}
  python halide.py < test/${PROGRAM}.halide > ${PROGRAM}.cpp
  g++ ${PROGRAM}.cpp 
  ./a.out
  echo ""
done

