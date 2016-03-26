1. Compiling OPENCL Kernel  
	a. 	change the directory to device folder using "cd pyramidal_down/device/"
	b.	Compile it using "aoc -v --board de1soc_sharedonly pyramid.cl -o pyramidal.aocx"

2. Compiling Host application
	Pre Request: make sure you have the "arm-linux-gnueabihf" cross-compiler setup on your system
	a.  Run the make file <in_source_directory>
	b. It generates the executable file in the directory "bin/<executable_name>"
	
3. Copy the necessary files into a memory stick
	a.File list
		i) 	pyramidal.aocx  - board binary file
		ii)	pyramidal_down	- executable file
		iii)balloons.ppm	- Image file
4. Mount the memory device
	a.create a mount directory at media/usb 
	b."mount /device/sda1 /media/usb"
	c.change directory to /media/usb

5. Running the OpenCL Code on DE1-SoC
	a.Load the FPGA hardware configuration stored in pyramidal.aocx using "aocl program /dev/acl0 pyramidal.aocx"
	b.Make sure the host program is executable by running "chmod +x ./pyramidal_down"
	c.Run the host program  "./pyramidscl <image.ppm> <conv> <runtime> <frames/sec>"

6. Change #defines in display.cpp if you want to change the input and output image file names.