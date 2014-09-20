/*
 *auther: Shicao Li
 *date  : 20th September 2014
 *
 * OpenCL Sample Code
 */
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdlib.h>
#include <vector>
#include <CL/cl.h>
using namespace std ;

const int InputSize = 8 ;
const int OutputSize = 6 ;
const int maskSize = 3 ;

cl_uint inputSignal[InputSize][InputSize] = {
	{ 3 , 1 , 1 , 4 , 8 , 2 , 1 , 3 } ,
	{ 4 , 2 , 1 , 1 , 2 , 1 , 2 , 3 } ,
	{ 4 , 4 , 4 , 4 , 3 , 2 , 2 , 2 } ,
	{ 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 } ,
	{ 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 } ,
	{ 0 , 1 , 1 , 2 , 3 , 4 , 4 , 5 } ,
	{ 0 , 2 , 1 , 2 , 1 , 3 , 2 , 1 } ,
	{ 0 , 1 , 0 , 1 , 1 , 1 , 1 , 1 } 
} ;

cl_uint mask[maskSize][maskSize] = {
	{ 1 , 1 , 1 } ,
	{ 1 , 0 , 1 } ,
	{ 1 , 1 , 1 } 
};

cl_uint OutputSignal[OutputSize][OutputSize] ;

void			 checkErr( cl_int , const char *) ;
void			 displayInfo() ;
cl_context       CreateContext() ;
cl_command_queue CreateQueue( cl_context , cl_device_id * ) ;
cl_program		 CreateProgram( cl_context , cl_device_id , const char * ) ;
bool			 CreateMemObject( cl_context , cl_mem* , cl_int& ) ;
void			 print() ;

int main( int argc , char ** argv )
{
	cl_context       context = 0 ;
	cl_command_queue commandQueue = 0 ;
	cl_program		 program = 0 ;
	cl_device_id	 device = 0 ;
	cl_kernel		 kernel = 0 ;
	cl_mem			 memObject[3] = { 0 , 0 , 0 } ;
	cl_int			 errNum  ;

	//displayInfo() ;

	context = CreateContext() ;
	if( context == NULL )
	{
		cerr << "Failed to create OpenCL context." << endl ;
		system( "pause" ) ;
		return 1 ;
	}

	commandQueue = CreateQueue( context , &device ) ;
	if( commandQueue == NULL )
	{
		cerr << "Failed to create OpenCL commandQueue." << endl ;
		system( "pause" ) ;
		return 1 ;
	}

	program = CreateProgram( context , device , "../src/Kernel.cl" ) ;
	if( program == NULL )
	{
		cerr << "Failed to create OpenCL program." << endl ;
		system( "pause" ) ;
		return 1 ;
	}

	kernel = clCreateKernel( program , "convolution" , NULL ) ;
	if( kernel == NULL )
	{
		cerr << "Failed to create OpenCL kernel." << endl ;
		system( "pause" ) ;
		return 1 ;
	}

	CreateMemObject( context , memObject , errNum ) ;
	if( memObject[0] == NULL || memObject[1] == NULL || memObject[2] == NULL )
	{
		cerr << "Failed to create OpenCL memery objects." << endl ;
		system( "pause" ) ;
		return 1 ;
	}


	errNum |= clSetKernelArg( kernel , 0 , sizeof(cl_mem) , &memObject[0] ) ;
	errNum |= clSetKernelArg( kernel , 1 , sizeof(cl_mem) , &memObject[1] ) ;
	errNum |= clSetKernelArg( kernel , 2 , sizeof(cl_mem) , &memObject[2] ) ;
	errNum |= clSetKernelArg( kernel , 3 , sizeof(cl_int) , &InputSize ) ;
	errNum |= clSetKernelArg( kernel , 4 , sizeof(cl_int) , &maskSize ) ;

	size_t globalWorkSize[2] = { OutputSize * OutputSize } ;
	size_t localWorkSize[2] = { 1 , 1 } ;
	
	errNum = clEnqueueNDRangeKernel( commandQueue , kernel , 2 , 0 , globalWorkSize , localWorkSize , 0 , NULL , NULL ) ;
	checkErr( errNum , "clEnqueueNDRangeKernel" ) ;

	errNum = clEnqueueReadBuffer( commandQueue , memObject[2] , CL_TRUE , 0 , sizeof(cl_uint) * OutputSize * OutputSize , OutputSignal , 0 , NULL , NULL  ) ;
	checkErr( errNum , "clEnqueueReadBuffer" ) ;

	print() ;

	system( "pause" ) ;
	if( context != NULL )		clReleaseContext( context ) ;
	if( commandQueue != NULL )  clReleaseCommandQueue( commandQueue ) ;
	if( program != NULL )		clReleaseProgram( program ) ;
	if( kernel != NULL )		clReleaseKernel( kernel ) ;
	clReleaseMemObject( memObject[0] ) ; clReleaseMemObject( memObject[1] ) ; clReleaseMemObject( memObject[2] ) ;
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

bool CreateMemObject( cl_context context , cl_mem  memObject[3] , cl_int &errNum )
{
	memObject[0] = clCreateBuffer( context , CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR , sizeof(cl_uint) * InputSize * InputSize , static_cast<void*>(inputSignal) , &errNum ) ;
	checkErr( errNum , "clCreateBuffer(InputSignal)" ) ;
		
	memObject[1] = clCreateBuffer( context , CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR , sizeof(cl_uint) * maskSize * maskSize , static_cast<void*>(mask) , &errNum ) ;
	checkErr( errNum , "clCreateBuffer(mask)" ) ;

	memObject[2] = clCreateBuffer( context , CL_MEM_READ_WRITE , sizeof(cl_uint) * OutputSize * OutputSize , NULL , &errNum ) ;
	checkErr( errNum , "clCreateBuffer(Output)" ) ;

	if( memObject[0] == NULL || memObject[1] == NULL || memObject[2] == NULL )
		return false ;
	return true ;
}

void print()
{
	for( int i = 0 ; i < OutputSize ; i ++ )
	{
		for( int j = 0 ; j < OutputSize ; j ++ )
			cout << OutputSignal[i][j] << " " ;
		cout << endl ;
	}
}