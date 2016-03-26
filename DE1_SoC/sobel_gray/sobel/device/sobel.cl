/*
Author Name	: 	Surjith Bhagavath Singh
Email 		:	surjithbs17@gmail.com / surjith.bhagavathsingh@colorado.edu
Description	:	Hough OpenCL kernel file
				

Credits: https://www.altera.com/support/support-resources/design-examples/design-software/opencl/sobel-filter.html
				
GNU GENERAL PUBLIC LICENSE
Version 3, 29 June 2007
Copyright (C) 2007 Free Software Foundation, Inc. <http://fsf.org/>

*/

__kernel
void sobel(global unsigned int * restrict frame_in, global unsigned int * restrict frame_out)
{
    
	int x = get_global_id(0);
	int y = get_global_id(1);
	int width = get_global_size(0);
	int height = get_global_size(1);
	int index = x + y*width;
	long int size = width*height;
	
	int Gx[3][3] = {{-1,-2,-1},{0,0,0},{1,2,1}};
    int Gy[3][3] = {{-1,0,1},{-2,0,2},{-1,0,1}};
	int G_x=0,G_y=0,G = 0; 
	int i=0,j=0;
    if (index < size && (x>1 && y>1) && (x < (width-1) && y < (height-1 ))   ) 
	{
	
		#pragma unroll 
		for(i=0;i<3;i++)
		{
			#pragma unroll
			for(j=0;j<3;j++)
			{
				G_y += (Gy[i][j])*frame_in[(x+j-1)+(width*(y+i-1))];
				G_x += (Gx[i][j])*frame_in[(x+j-1)+(width*(y+i-1))];
			}
		}
						
		G = abs(G_x) + abs(G_y);
			
		if(G>255)
			frame_out[index] = 255;
		else
			frame_out[index] = G;
		
    }
}