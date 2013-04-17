./clean.sh
for PROGRAM in base_test rdom_test split_reuse_inner_name_bug two_vector_args parallel \
  vector_extern jit_stress parallel_alloc vector_bounds_inference parallel_nested \
  bound_inference fibonacci
do
  echo ${PROGRAM}
  python ../halide.py < ${PROGRAM}.halide > ${PROGRAM}.cpp
  g++ -I../ ${PROGRAM}.cpp
  ./a.out
  echo ""
done

