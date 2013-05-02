./clean.sh
for PROGRAM in base1 base2 base_test c_function chunk_sharing fibonacci gpu_large_alloc jit_stress \
  logical mod parallel parallel_alloc parallel_nested reduction split_reuse_inner_name_bug vec_test \
  vector_extern

#use Func as Expr rather than image processing: bounds_inference bounds_inference_chunk chunk side_effects split_store_compute vector_bounds_inference
#TODO:
#big memory (Don't know what it means. Looks good for the test case): gpu_large_alloc
#loops can't merge: two_vector_args
#seg falut when using parallel: vector_cast

do
  echo ${PROGRAM}
  python ../halide.py < ${PROGRAM}.halide > ${PROGRAM}.cpp
  #g++ -fopenmp -mavx -mno-align-double -I../ ${PROGRAM}.cpp
  g++ -fopenmp -mno-align-double -I../ ${PROGRAM}.cpp
  ./a.out
  echo ""
done

