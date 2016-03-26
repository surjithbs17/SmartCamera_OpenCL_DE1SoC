/*
Author Name	: 	Surjith Bhagavath Singh
Email 		:	surjithbs17@gmail.com / surjith.bhagavathsingh@colorado.edu
Description	:	Pyramidal transform OpenCL kernel file
				

Credits: Karthikeyan Mani, Chris Wagner
				
GNU GENERAL PUBLIC LICENSE
Version 3, 29 June 2007
Copyright (C) 2007 Free Software Foundation, Inc. <http://fsf.org/>

*/

__kernel void pyrDown(__global unsigned int * restrict g_DataIn, __global unsigned int * restrict g_DataOut)
{
    const float gaussianMatrix[25] = {1, 4, 6, 4, 1, 4, 16, 24, 16, 4, 6, 2, 36, 24, 6, 4, 16, 24, 16, 4, 1, 4, 6, 4, 1};

    
   int width = get_global_size(0);
   int height = get_global_size(1);
   int x = get_global_id(0);
   int y = get_global_id(1);
   int index = (y*(width))+x;
   int out_index = (y/2)*(width/2)+(x/2);

    const int filterRadius = 2;     //FILTER_RADIUS;
    const int filterDiameter = 5;   //FILTER_DIAMETER;

    // Threads outside the image
    if(x>=width||y>=height)
        return;

    // only compute even rows and cols
    if(x%2!=0||y%2!=0)
    {
        return;
    }

    // Border cases of the global image
    if(x<filterRadius||y<filterRadius)
    {
        g_DataOut[out_index] = g_DataIn[index];
        //sharedMem[sharedIndex] = g_DataIn[index];
        return;
    }

    if((x>width-filterRadius-1)&&(x<width))
    {
        g_DataOut[out_index] = g_DataIn[index];
        //sharedMem[sharedIndex] = g_DataIn[index];
        return;
    }

    if((y>height-filterRadius-1)&&(y<height))
    {
        g_DataOut[out_index] = g_DataIn[index];
        //sharedMem[sharedIndex] = g_DataIn[index];
        return;
    }

    // Computation for active threads inside the current block
    float sumX = 0;
    int matx = 0;
    int maty = 0;
    for(int dy = -filterRadius ; dy<=filterRadius ; dy++)
    {
        for(int dx = -filterRadius ; dx<=filterRadius ; dx++)
        {
            matx = x+dx;
            maty = y+dy;
            float Pixel = (float)(g_DataIn[maty*width+matx]);
            sumX += Pixel*gaussianMatrix[(dy+filterRadius)*filterDiameter+(dx+filterRadius)];
        }
    }
    g_DataOut[out_index] = (unsigned int)clamp((int)sumX/256, (int)0, (int)255);
}




__kernel void pyrUp(__global unsigned int * restrict g_DataIn, __global unsigned int * restrict g_DataOut)
{
    const float laplacianMatrix[25] = {1, 4, 6, 4, 1, 4, 16, 24, 16, 4, 6, 24, 36, 24, 6, 4, 16, 24, 16, 4, 1, 4, 6, 4,
            1};

   int x = get_global_id(0);
	int y = get_global_id(1);
	int width = get_global_size(0);
	int height = get_global_size(1);
	int index = y/2 * (width/2) + x/2;
	int out_index = y * (width) + x;

    const int filterRadius = 2;     //FILTER_RADIUS;
    const int filterDiameter = 5;   //FILTER_DIAMETER;

    // Threads outside the image
    if(x>=width||y>=height)
        return;

    // Border cases of the global image
    if(x<filterRadius||y<filterRadius)
    {
        g_DataOut[out_index] = g_DataIn[index];
        return;
    }

    if((x>width-filterRadius-1)&&(x<width))
    {
        g_DataOut[out_index] = g_DataIn[index];
        return;
    }

    if((y>height-filterRadius-1)&&(y<height))
    {
        g_DataOut[out_index] = g_DataIn[index];
        return;
    }

    float sumX = 0;
    int matx = 0;
    int maty = 0;
    for(int dy = -filterRadius ; dy<=filterRadius ; dy++)
    {
        for(int dx = -filterRadius ; dx<=filterRadius ; dx++)
        {
            matx = x+dx;
            maty = y+dy;
            if(matx%2!=0||maty%2!=0)
            {
                continue;
            }
            float Pixel = (float)(g_DataIn[maty/2*width/2+matx/2]);
            sumX += Pixel*laplacianMatrix[(dy+filterRadius)*filterDiameter+(dx+filterRadius)];
        }
    }
    g_DataOut[out_index] = (unsigned int)clamp((int)sumX/64, (int)0, (int)255);
}


