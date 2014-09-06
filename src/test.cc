#include <iostream>
#include <stdlib.h>
#include <CL/cl.h>
using namespace std ;

inline  void  checkErr( cl_int err , const char * name ) 
{
	if( err != CL_SUCCESS )
	{
		cout << "Error : " << name << " (" << err << ")" << endl ;
		exit( EXIT_FAILURE ) ;
	}
}

void			 displayInfo() ;
cl_context       CreateContext() ;
cl_command_queue CreateQueue( cl_context , cl_device_id * ) ;

int main( int argc , char ** argv )
{
	cl_context       context = 0 ;
	cl_command_queue commandQueue = 0 ;
	cl_program		 program = 0 ;
	cl_device_id	 device = 0 ;
	cl_kernel		 kernel = 0 ;
	cl_int			 errNum  ;

	context = CreateContext() ;
	if( context == NULL )
	{
		cerr << "Failed to create OpenCL context." << endl ;
		return 1 ;
	}

	commandQueue = CreateQueue( context , &device ) ;
	if( commandQueue == NULL )
	{
		cerr << "Failed to create OpenCL commandQueue." << endl ;
		return 1 ;
	}
	system( "pause" ) ;
	return 0 ;
}

void  displayInfo()
{
}

cl_context CreateContext() 
{
	cl_int			errNum ;
	cl_uint         numPlatforms ;
	cl_platform_id  firstPlatformId ; 
	cl_context		context ;
	errNum = clGetPlatformIDs( 1 , &firstPlatformId , &numPlatforms ) ;
	checkErr( errNum , "clGetPlatformIDs" ) ;
	if( numPlatforms <= 0 ) 
	{
		cerr << "Failed to find any platform." << endl ;
		return NULL ;
	}
	
	cl_context_properties contextProperties[] = { CL_CONTEXT_PLATFORM , ( cl_context_properties )firstPlatformId , 0 } ;
	context = clCreateContextFromType( contextProperties , CL_DEVICE_TYPE_GPU , NULL , NULL , &errNum ) ;
	checkErr( errNum , "clCreateContext" ) ;

	return context ;
}

cl_command_queue CreateQueue( cl_context context , cl_device_id *device )
{
	cl_int errNum ;
	cl_device_id *devices ;
	cl_command_queue commandQueue = NULL ;
	size_t deviceBufferSize = -1 ;

	errNum = clGetContextInfo( context , CL_CONTEXT_DEVICES , 0 , NULL , &deviceBufferSize ) ;
	checkErr( errNum , "clGetContextInfo" ) ;

	if( deviceBufferSize <= 0 )
	{
		cout << "No device available." << endl ;
		return NULL ;
	}
	
	devices = new cl_device_id[deviceBufferSize/sizeof(cl_device_id)] ;
	
	errNum = clGetContextInfo( context , CL_CONTEXT_DEVICES , deviceBufferSize , devices , NULL ) ;
	checkErr( errNum , "clGetContextInfo" ) ;

	commandQueue = clCreateCommandQueue( context , devices[0] , 0 , NULL ) ;
	checkErr( errNum , "clCreateCommandQueue" ) ;

	*device = devices[0] ;

	delete [] devices ;

	return commandQueue ;
}