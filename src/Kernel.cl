kernel void convolution( global int *input , global int *mask , global uint *result , int InputWidth , int maskWidth )
{
	int y = get_global_id( 0 ) ;
	int x = get_global_id( 1 ) ; 

	int sum = 0 ;
	for( int r = 0 ; r < maskWidth ; r ++ )
	{
		for( int c = 0 ; c < maskWidth ; c ++ )
		{
			sum += mask[r*maskWidth+c] * input[(y+r)*InputWidth+(x+c)] ;
		}
	}
	result[y*get_global_size(0)+x] = sum ;
}