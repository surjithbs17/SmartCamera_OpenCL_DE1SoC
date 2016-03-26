/*
Author Name	: 	Surjith Bhagavath Singh
Email 		:	surjithbs17@gmail.com / surjith.bhagavathsingh@colorado.edu
Description	:	Hough OpenCL kernel file
				

Credits: http://www.keymolen.com/2013/05/hough-transformation-c-implementation.html
				
GNU GENERAL PUBLIC LICENSE
Version 3, 29 June 2007
Copyright (C) 2007 Free Software Foundation, Inc. <http://fsf.org/>

*/


__kernel
void sobel(global unsigned int * restrict frame_in, global unsigned int * restrict frame_out)
{
    
	int x = get_global_id(0);					//getting the work item parameters and size
	int y = get_global_id(1);
	int width = get_global_size(0);
	int height = get_global_size(1);
	int index = x + y*width;					//indexing for array elements
	long int size = width*height;
	
	
	//Sobel Implementation
	
	int Gx[3][3] = {{-1,-2,-1},{0,0,0},{1,2,1}};		//sobel kernel
    int Gy[3][3] = {{-1,0,1},{-2,0,2},{-1,0,1}};
	
	int G_x=0,G_y=0,G = 0; 								//Initialization
	int i=0,j=0;
    
	if (index < size && (x>1 && y>1) && (x < (width-1) && y < (height-1 ))   )	//Checking boundry conditions  
	{
	
		#pragma unroll 
		for(i=0;i<3;i++)								//looping through 3*3 sobel kernel
		{
			#pragma unroll
			for(j=0;j<3;j++)
			{
				G_y += (Gy[i][j])*frame_in[(x+j-1)+(width*(y+i-1))];	
				G_x += (Gx[i][j])*frame_in[(x+j-1)+(width*(y+i-1))];
			}
		}
		
				
		G = abs(G_x) + abs(G_y);						//Calculating absolute values of gradients in x and y direction
			
		if(G>255)										//Writing back to the output
			frame_out[index] = 255;
		else
			frame_out[index] = G;
		
    }
}

//Hough Implementation

__kernel
void hough(global unsigned int * restrict frame_in, global unsigned int * restrict frame_out,
           const unsigned int hough_h)
{
    
	int x = get_global_id(0);						//getting the work item parameters and size
	int y = get_global_id(1);
	int width = get_global_size(0);
	int height = get_global_size(1);
	int index = x + y*width;
	long int size = width*height;
	double DEG2RAD = 0.0174533;
	double center_x = width/2;
	double center_y = height/2;   

	if( frame_in[index] > 250 )						//checking for the values greater than 250, has to be modified if you have different threshold 
	{
		for(int t=0;t<180;t++)  
		{
		double r = ( ((double)x - center_x) * cos((double)t * DEG2RAD)) + (((double)y - center_y) * sin((double)t * DEG2RAD));		//plotting x and y in ro and theta
		frame_out[ (int)((round(r + hough_h) * 180.0)) + t]++;
		 
		}
	
	}

	 

}