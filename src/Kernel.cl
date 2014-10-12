__kernel  void Dijkstra_first(__global int *vertexArray, __global int *edgeArray, __global float *weightArray,
                               __global int *maskArray, __global float *costArray, __global float *updatingCostArray,
                               int vertexCount, int edgeCount )
{
    // access thread id
    int tid = get_global_id(0);

    if ( maskArray[tid] != 0 )
    {
        maskArray[tid] = 0;

        int edgeStart = vertexArray[tid];
        int edgeEnd;
        if (tid + 1 < (vertexCount))
        {
            edgeEnd = vertexArray[tid + 1];
        }
        else
        {
            edgeEnd = edgeCount;
        }

        for(int edge = edgeStart; edge < edgeEnd; edge++)
        {
            int nid = edgeArray[edge];

            if (updatingCostArray[nid] > (costArray[tid] + weightArray[edge]))
            {
                updatingCostArray[nid] = (costArray[tid] + weightArray[edge]);
            }
        }
    }
}

__kernel  void Dijkstra_second(__global int *vertexArray, __global int *edgeArray, __global float *weightArray,
                                __global int *maskArray, __global float *costArray, __global float *updatingCostArray,
                                int vertexCount)
{
    int tid = get_global_id(0);


    if (costArray[tid] > updatingCostArray[tid])
    {
        costArray[tid] = updatingCostArray[tid];
        maskArray[tid] = 1;
    }

    updatingCostArray[tid] = costArray[tid];
}