#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdlib.h>
#include <CL/cl.h>
using namespace std ;

void			 checkErr( cl_int , const char *) ;
void			 displayInfo() ;
cl_context       CreateContext() ;
cl_command_queue CreateQueue( cl_context , cl_device_id * ) ;
cl_program		 CreateProgram( cl_context , cl_device_id , const char * ) ;

int main( int argc , char ** argv )
{
	cl_context       context = 0 ;
	cl_command_queue commandQueue = 0 ;
	cl_program		 program = 0 ;
	cl_device_id	 device = 0 ;
	cl_kernel		 kernel = 0 ;
	cl_mem			 memObject = 0 ;
	cl_int			 errNum  ;

	//displayInfo() ;

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

	program = CreateProgram( context , device , "../src/Kernel.cl" ) ;
	if( program == NULL )
	{
		cerr << "Failed to create OpenCL program." << endl ;
		return 1 ;
	}

	system( "pause" ) ;
	if( context != NULL )		clReleaseContext( context ) ;
	if( commandQueue != NULL )  clReleaseCommandQueue( commandQueue ) ;
	if( program != NULL )		clReleaseProgram( program ) ;
	if( kernel != NULL )		clReleaseKernel( kernel ) ;
	if( memObject != NULL )		clReleaseMemObject( memObject ) ;
	return 0 ;
}


void  checkErr( cl_int err , const char * name ) 
{
	if( err != CL_SUCCESS )
	{
		cout << "Error : " << name << " (" << err << ")" << endl ;
		system( "pause" ) ;
		exit( EXIT_FAILURE ) ;
	}
}

void  displayInfo()
{
	cl_int errNum ; 
	cl_platform_id *platformIDs ;
	cl_device_id   *deviceIDs ;
	cl_uint platformNum , deviceNum ;
	cl_char info[500] ;

	errNum = clGetPlatformIDs( 0 , NULL , &platformNum ) ;
	checkErr( errNum , "clGetPlatformIDs" ) ;

	cout << "There are " << platformNum << " platform(s)." << endl << endl ;

	platformIDs = new cl_platform_id[platformNum] ;

	for( int i = 0 ; i < platformNum ; i ++ )
	{
		errNum = clGetPlatformIDs( 1 , &platformIDs[i] , NULL ) ;
		checkErr( errNum , "clGetPlatformIDs" ) ;

		errNum = clGetPlatformInfo( platformIDs[i] , CL_PLATFORM_NAME , sizeof( cl_char ) * 500 , info , NULL ) ;
		checkErr( errNum , "clGetPlatformInfo  CL_PLATFORM_NAME" ) ;
		cout << "Platform name:  " << info << "." << endl ;

		errNum = clGetDeviceIDs( platformIDs[i] , CL_DEVICE_TYPE_ALL , 0 , NULL , &deviceNum ) ;
		checkErr( errNum , "clGetDeviceIDs  device number") ;
		cout << "There are " << deviceNum << " device(s) in this platform:  " << endl << endl ; 

		deviceIDs = new cl_device_id[deviceNum] ;

		for( int j = 0 ; j < deviceNum ; j ++ )
		{
			errNum = clGetDeviceIDs( platformIDs[i] , CL_DEVICE_TYPE_ALL , 1 , &deviceIDs[j] , NULL ) ;
			checkErr( errNum , "clGetDeviceIDs" ) ; 

			cl_device_type type ; 
			errNum = clGetDeviceInfo( deviceIDs[j] , CL_DEVICE_TYPE , sizeof( cl_device_type ) , &type , NULL ) ;
			checkErr( errNum , "clGetDeviceInfo  CL_DEVICE_TYPE" ) ; 
			if( type == CL_DEVICE_TYPE_GPU ) cout << "Device type is GPU." << endl ;
			else cout << "Device type is CPU." << endl ;

			char name[500] ;
			errNum = clGetDeviceInfo( deviceIDs[j] , CL_DEVICE_NAME , sizeof( char ) * 500 , name , NULL ) ;
			checkErr( errNum , "clGetDeviceInfo  CL_DEVICE_NAME" ) ; 
			cout << "Device name is :" << name << endl ;
			cout << endl ;
		}

		delete [] deviceIDs ;

		cout << endl <<  endl ;
	}
	delete [] platformIDs ;
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

cl_program CreateProgram( cl_context context , cl_device_id device , const char *fileName ) 
{
	cl_int     errNum ; 
	cl_program program ;
	ifstream   kernelFile( fileName ) ;
	if( !kernelFile.is_open() )
	{
		cerr << "Failed to open file for reading: " << fileName << endl ;
		return NULL ;
	}

	ostringstream oss ;
	oss << kernelFile.rdbuf() ;

	string src = oss.str() ;
	const char *srcStr = src.c_str() ;
	program = clCreateProgramWithSource( context , 1 , (const char **)&srcStr , NULL , NULL ) ;

	if( program == NULL )
	{
		cerr << "Failed to create CL program from source." << endl ;
		return NULL ;
	}

	errNum = clBuildProgram( program , 0 , NULL , NULL , NULL , NULL ) ;
	if( errNum != CL_SUCCESS ) 
	{
		char buildLog[20000];
		clGetProgramBuildInfo( program , device , CL_PROGRAM_BUILD_LOG , sizeof(buildLog) , buildLog , NULL ) ;
		cerr << "Error in kernel: " << endl ; 
		cerr << buildLog ;
		clReleaseProgram( program ) ;
		return NULL ;
	}
	return program ;
}