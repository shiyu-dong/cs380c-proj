./clean.sh
for PROGRAM in base_test split_reuse_inner_name_bug parallel \
  vector_extern jit_stress parallel_nested \
  fibonacci \
  chunk_sharing mod logical c_function parallel_alloc reduction_schedule \
  halide_blur lighten wavelet

#use Func as Expr rather than image processing: #vector_bounds_inference bounds_inference bounds_inference_chunk chunk split_store_compute side_effects
#big memory: gpu_large_alloc
#strange type casting rules: vector_cast

#TODO:
#loops can't merge: two_vector_args

do
  echo ${PROGRAM}
  python ../halide.py < ${PROGRAM}.halide > ${PROGRAM}.cpp
  g++ -fopenmp -mavx -mno-align-double -I../ ${PROGRAM}.cpp
  ./a.out
  echo ""
done

