./clean.sh
for PROGRAM in base_test rdom_test split_reuse_inner_name_bug two_vector_args parallel \
  vector_extern jit_stress parallel_nested \
  fibonacci \
  chunk_sharing mod chunk logical 
#seg fault: #parallel_alloc vector_bounds_inference bounds_inference bounds_inference_chunk
do
  echo ${PROGRAM}
  python ../halide.py < ${PROGRAM}.halide > ${PROGRAM}.cpp
  g++ -I../ ${PROGRAM}.cpp
  ./a.out
  echo ""
done

