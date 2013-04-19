./clean.sh
for PROGRAM in base_test split_reuse_inner_name_bug two_vector_args parallel \
  vector_extern jit_stress parallel_nested \
  fibonacci \
  chunk_sharing mod logical c_function vector_cast vector_math parallel_alloc
#seg fault: #vector_bounds_inference bounds_inference bounds_inference_chunk chunk split_store_compute
#big memory: gpu_large_alloc
#need new reduction implementation: reduction_schedule side_effects
do
  echo ${PROGRAM}
  python ../halide.py < ${PROGRAM}.halide > ${PROGRAM}.cpp
  g++ -I../ ${PROGRAM}.cpp
  ./a.out
  echo ""
done

