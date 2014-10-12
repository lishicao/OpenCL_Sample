#include <cl/cl.h>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std ;

typedef struct 
{
	int *vertexArray ;
	int *edgeArray;
	int vertexCount , edgeCount ;
	int *weightArray; 
} GraphData ;

void runDijkstra( GraphData* , int , int * ) ;

void			 checkErr( cl_int , const char *) ;
void			 displayInfo() ;
cl_context       CreateContext() ;
cl_command_queue CreateCommandQueue( cl_context , cl_device_id * ) ;
cl_program		 CreateProgram( cl_context , cl_device_id , const char * ) ;
bool			 CreateMemObject( cl_context , cl_mem* , cl_int& ) ;
bool			 maskArrayEmpty( int * , int ) ;

int  map[1005][1005] ;
int  t , n ;
int  INF = 9999999 ;

int main()
{
	GraphData graph ;
	int  start , end , weight , Count = 0 ; 
	int  *costArray ;

	cin >> graph.edgeCount >> graph.vertexCount ;

	graph.edgeArray = new int[graph.edgeCount*2] ;
	graph.vertexArray = new int[graph.vertexCount] ;
	graph.weightArray = new int[graph.edgeCount*2] ;
	costArray = new int[graph.vertexCount] ;

	memset( map , 0 , sizeof( map ) ) ;
	memset( graph.vertexArray , -1 , sizeof( graph.vertexArray ) ) ;
	for( int i = 0 ; i < graph.vertexCount ; i ++ ) graph.vertexArray[i] = -1 ;
	for( int i = 0 ; i < graph.edgeCount ; ++ i )
	{
		cin >> start >> end >> weight ;
		if ( map[start-1][end-1] == 0) 
		{
			map[start-1][end-1] = weight ; 
			map[end-1][start-1] = weight ;
		}
		else if( weight < map[start-1][end-1] )
		{
			map[start-1][end-1] = weight ;
			map[end-1][start-1] = weight ;
		}
	}

	for( int i = 0 ; i < graph.vertexCount ; ++ i )
	{
		for( int j = 0 ; j < graph.vertexCount ; ++ j )
		{
			if( map[i][j] != 0 )
			{
				graph.edgeArray[Count] = j ; 
				graph.weightArray[Count] = map[i][j] ;
				if( graph.vertexArray[i] == -1 ) graph.vertexArray[i] = Count ;
				Count ++ ;
			}
		}
	}

	graph.edgeCount *= 2 ;
	runDijkstra( &graph , 0 , costArray ) ;
	cout << costArray[graph.vertexCount-1] << endl ;
	system( "pause" ) ;
	return 0 ;
}

void runDijkstra( GraphData* graph , int source , int *costArray )
{
	int *updateCostArray = new int[graph->vertexCount] ;
	int *maskArray = new int[graph->vertexCount];
	for( int i = 0 ; i < graph->vertexCount ; i ++ ) maskArray[i] = 0 ;
	for( int i = 0 ; i < graph->vertexCount ; i ++ ) costArray[i] = INF ;
	for( int i = 0 ; i < graph->vertexCount ; i ++ ) updateCostArray[i] = INF ;
	maskArray[source] = 1 ; 
	updateCostArray[source] = 0 ;
	costArray[source] = 0 ;

	cl_context       context = 0 ;
	cl_device_id     device = 0 ;
	cl_program       program = 0 ;
	cl_kernel        dijkstra_first = 0 , dijkstra_second = 0 ;
	cl_command_queue commandQueue = 0 ;
	cl_mem			 vertex = 0 , edge = 0 , weight = 0 , mask = 0 , cost = 0 , updateCost = 0  ;
	cl_int			 errNum ;

	context = CreateContext() ;
	if( context == NULL ) 
	{
		cerr << "Failed to create OpenCL context." << endl ;
		system( "pause" ) ;
		exit( 0 ) ;
	}

	commandQueue = CreateCommandQueue( context , &device) ;
	if( commandQueue == NULL )
	{
		cerr << "Failed to create OpenCL commad queue." << endl ;
		system( "pause" ) ;
		exit( 0 ) ;
	}

	program = CreateProgram( context , device , "../src/Kernel.cl" ) ;
	if( program == NULL )
	{
		cerr << "Failed to create OpenCL program." << endl ;
		system( "pause" ) ;
		exit( 0 ) ;
	}

	dijkstra_first  = clCreateKernel( program , "Dijkstra_first"  , NULL ) ;
	dijkstra_second = clCreateKernel( program , "Dijkstra_second" , NULL ) ;
	if( dijkstra_first == NULL || dijkstra_second == NULL )
	{
		cerr << "Failed to Create OpenCL kernel." << endl ;
		system( "pause" ) ;
		exit( 0 ) ;
	}

	vertex     = clCreateBuffer( context , CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR , sizeof(int) * graph->vertexCount , graph->vertexArray , NULL ) ;
	edge       = clCreateBuffer( context , CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR , sizeof(int) * graph->edgeCount   , graph->edgeArray   , NULL ) ;
	weight     = clCreateBuffer( context , CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR , sizeof(int) * graph->edgeCount   , graph->weightArray , NULL ) ;
	mask	   = clCreateBuffer( context , CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR , sizeof(int) * graph->vertexCount , maskArray          , NULL ) ;
	cost       = clCreateBuffer( context , CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR , sizeof(int) * graph->vertexCount , costArray			 , NULL ) ;
	updateCost = clCreateBuffer( context , CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR , sizeof(int) * graph->vertexCount , updateCostArray    , NULL ) ;

	if( vertex == NULL || edge == NULL || weight == NULL || mask == NULL || cost == NULL || updateCost == NULL )
	{
		cerr << "Error Creating memory objects." << endl ;
		system( "pause" ) ;
		exit( 0 ) ;
	}

	errNum  = clSetKernelArg( dijkstra_first , 0 , sizeof(cl_mem) , &vertex ) ; 
	errNum |= clSetKernelArg( dijkstra_first , 1 , sizeof(cl_mem) , &edge   ) ; 
	errNum |= clSetKernelArg( dijkstra_first , 2 , sizeof(cl_mem) , &weight ) ; 
	errNum |= clSetKernelArg( dijkstra_first , 3 , sizeof(cl_mem) , &mask   ) ; 
	errNum |= clSetKernelArg( dijkstra_first , 4 , sizeof(cl_mem) , &cost   ) ; 
	errNum |= clSetKernelArg( dijkstra_first , 5 , sizeof(cl_mem) , &updateCost ) ; 
	errNum |= clSetKernelArg( dijkstra_first , 6 , sizeof(cl_int) , &graph->vertexCount ) ; 
	errNum |= clSetKernelArg( dijkstra_first , 7 , sizeof(cl_int) , &graph->edgeCount ) ; 

	errNum  = clSetKernelArg( dijkstra_second , 0 , sizeof(cl_mem) , &vertex ) ; 
	errNum |= clSetKernelArg( dijkstra_second , 1 , sizeof(cl_mem) , &edge   ) ; 
	errNum |= clSetKernelArg( dijkstra_second , 2 , sizeof(cl_mem) , &weight ) ; 
	errNum |= clSetKernelArg( dijkstra_second , 3 , sizeof(cl_mem) , &mask   ) ; 
	errNum |= clSetKernelArg( dijkstra_second , 4 , sizeof(cl_mem) , &cost   ) ; 
	errNum |= clSetKernelArg( dijkstra_second , 5 , sizeof(cl_mem) , &updateCost ) ; 
	errNum |= clSetKernelArg( dijkstra_second , 6 , sizeof(cl_int) , &graph->vertexCount ) ; 

	if( errNum !=CL_SUCCESS )
	{
		cerr << "Error setting kernel argument." << endl ;
		system( "pause" ) ;
		exit( 0 ) ;
	}

	size_t globalWrokSize[1] = { graph->vertexCount } ;
	size_t localWrokeSize[1] = { 1 } ;

	cl_event readDone , first , second ;

	while( !maskArrayEmpty( maskArray  , graph->vertexCount ) )
	{
		for( int i = 0 ; i < 5 ; i ++ )
		{
			errNum = clEnqueueNDRangeKernel( commandQueue , dijkstra_first  , 1 , NULL , globalWrokSize , localWrokeSize , 0 , NULL , &first ) ;
			checkErr( errNum , "clEnqueueNDRangeKernel" ) ;
			clWaitForEvents(1, &first);

			errNum = clEnqueueNDRangeKernel( commandQueue , dijkstra_second , 1 , NULL , globalWrokSize , localWrokeSize , 0 , NULL , &second ) ;
			checkErr( errNum , "clEnqueueNDRangeKernel" ) ;
			clWaitForEvents(1,&second) ;
		}
		clFinish( commandQueue );
		errNum = clEnqueueReadBuffer( commandQueue , mask , CL_TRUE , 0 , sizeof( int ) * graph->vertexCount , maskArray , 0 , NULL , &readDone ) ;
		checkErr( errNum , "clEnqueueReadBuffer" ) ;
		clWaitForEvents(1, &readDone);
	}

	clFinish( commandQueue ) ;
	errNum = clEnqueueReadBuffer( commandQueue , cost , CL_TRUE , 0 , sizeof(int) * graph->vertexCount , costArray , 0 , NULL , NULL ) ; 
	if( errNum !=CL_SUCCESS )
	{
		cerr << "Error reading cost buffer." << endl ;
		system( "pause" ) ;
		exit( 0 ) ;
	}
}

void  checkErr( cl_int err , const char * name ) 
{
	if( err != CL_SUCCESS )
	{
		cout << "Error : " << name << " (" << err << ")" << endl ;
		cout << err << endl ;
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

			int units ;
			errNum = clGetDeviceInfo( deviceIDs[j] , CL_DEVICE_MAX_COMPUTE_UNITS , sizeof( units ) , &units , NULL ) ;
			checkErr( errNum , "clGetDeviceInfo CL_DEVICE_MAX_COMPUTE_UNITS" ) ;
			cout << "Device max compute units " << units << endl ;

			int size[3] ;
			errNum = clGetDeviceInfo( deviceIDs[j] , CL_DEVICE_MAX_WORK_ITEM_SIZES , sizeof( int ) * 3 , size , NULL ) ;
			checkErr( errNum , "clGetDeviceInfo CL_DEVICE_MAX_WORK_ITEM_SIZES" ) ;
			cout << "Device max work item size  " << size[0] << " " << size[1] << " " << size[2] << endl ;
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

cl_command_queue CreateCommandQueue( cl_context context , cl_device_id *device )
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

bool maskArrayEmpty(int *maskArray, int count)
{
    for(int i = 0; i < count; i++ )
    {
        if (maskArray[i] == 1)
        {
            return false;
        }
    }

    return true;
}