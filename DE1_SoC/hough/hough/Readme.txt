1. Compiling OPENCL Kernel  
	a. 	change the directory to device folder using "cd device/"
	b.	Compile it using "aoc -v --board de1soc_sharedonly hough.cl -o hough.aocx"

2. Compiling Host application
	Pre Request: make sure you have the "arm-linux-gnueabihf" cross-compiler setup on your system
	a.  Run the make file <in_source_directory>
	
3. Copy the necessary files into a memory stick
	a.File list
		i) 	hough.aocx  - board binary file
		ii)	hough 		- executable file
		iii)	balloons.ppm- Image file
4. Mount the memory device
	a.create a mount directory at media/usb 
	b."mount /device/sda1 /media/usb"
	c.change directory to /media/usb

5. Running the OpenCL Code on DE1-SoC
	a.Load the FPGA hardware configuration stored in hough.aocx using "aocl program /dev/acl0 hough.aocx"
	b.Make sure the host program is executable by running "chmod +x ./hough"
	c.Run the host program  "./pyramidscl <image.ppm> <conv> <runtime> <frames/sec>"

6. Change #defines in display.cpp if you want to change the input and output image file names.