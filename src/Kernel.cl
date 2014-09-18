__kernel void hello_kernel( global const  float *a , global const float *b , global float *result )
{
	int gid = get_global_id( 0 ) ;
	result[gid] = a[gid] + b[gid] ;
}