/*
Author Name	: 	Surjith Bhagavath Singh, Claus Ryan
Email 		:	surjithbs17@gmail.com / surjith.bhagavathsingh@colorado.edu , clausr@my.erau.edu
Description	:	display.cpp / Host Application
Credits: https://www.altera.com/support/support-resources/design-examples/design-software/opencl/sobel-filter.html
				
GNU GENERAL PUBLIC LICENSE
Version 3, 29 June 2007
Copyright (C) 2007 Free Software Foundation, Inc. <http://fsf.org/>

*/
#define NOMINMAX
#define FILTER_FPS_ITERATIONS_TO_AVG 10
#define CL_FUNCTION "pyramid"
#define DEFAULT_IMAGE "balloons.ppm"
#define DUMP_IMAGE    "balloons_pyrUp.ppm"

// Standard includes
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>

#include <CL/opencl.h>
#include "ppm.h"
#include "AOCLUtils/aocl_utils.h"

using namespace aocl_utils;

std::string imageFilename = DEFAULT_IMAGE;

bool continuousMode = false;
bool FPSLimited = false;

unsigned int FPSLimit;
unsigned int iterationsToAvg = FILTER_FPS_ITERATIONS_TO_AVG;

cl_uint *input = NULL;
cl_uint *output = NULL;

unsigned int width, height, channels;
unsigned int out_width, out_height;

cl_platform_id platform;
cl_uint num_devices;
cl_device_id device;
cl_context context;
cl_command_queue queue;
cl_program program;
cl_kernel pyramidalKernel;
cl_mem in_buffer, out_buffer;

size_t workSize[2];

void initCL();
void cleanup();
void teardown(int exit_status = 1);

//***************************************************************//
// Take the difference of two timespec structures
//***************************************************************//
void timespec_diff(struct timespec *start, struct timespec *stop,
                   struct timespec *result, bool check_neg)
{        
	result->tv_sec = stop->tv_sec - start->tv_sec;
  result->tv_nsec = stop->tv_nsec - start->tv_nsec;

  if ( check_neg && result->tv_nsec < 0) {
      result->tv_sec = result->tv_sec - 1;
      result->tv_nsec = result->tv_nsec + 1000000000;

  }
}

void
filter() {
  cl_int status;
  
  status = clEnqueueWriteBuffer(queue, in_buffer, CL_FALSE, 0, sizeof(unsigned int) * height * width, input, 0, NULL, NULL);
  checkError(status, "Error: could not copy data into device");

  status = clFinish(queue);
  checkError(status, "Error: could not finish successfully");
    
  status = clEnqueueNDRangeKernel(queue, pyramidalKernel, 2, NULL, workSize, workSize, 0, NULL, NULL);
  checkError(status, "Error: could not enqueue pyramidal kernel");
  
  status  = clFinish(queue);
  checkError(status, "Error: could not finish successfully");

  status = clEnqueueReadBuffer(queue, out_buffer, CL_FALSE, 0, sizeof(unsigned int) * out_width * out_height, output, 0, NULL, NULL);
  checkError(status, "Error: could not copy data from device");

  status = clFinish(queue);
  checkError(status, "Error: could not successfully finish copy");
}

int
main(int argc, char **argv) {
  Options options(argc, argv);
  
  if(options.has("help") || options.has("h") || options.has("?")) {
    std::cout << "Usage: ./sobel_filter [-continuous [-fps=FPS] [-avg=Iterations]] [-img=imageFilename]  " << std::endl;
    exit(EXIT_SUCCESS);
  }
  
  if(options.has("img")) {
    imageFilename = options.get<std::string>("img");
  }
  std::cout << "Img set to " << imageFilename << std::endl;

  if(options.has("continuous")) {
    continuousMode = true;
    std::cout << "Continuous mode" << std::endl;
    if(options.has("fps")) {
      FPSLimited = true;
      FPSLimit = options.get<unsigned int>("fps");
      std::cout << "FPS Limiter set at " << FPSLimit << std::endl;
    } else {
      std::cout << "Unlimited FPS mode" << std::endl;
    }
    if(options.has("avg")) {
      iterationsToAvg = options.get<unsigned int>("avg");    
    }
    std::cout << "System will average FPS over " << iterationsToAvg << " iterations." << std::endl;
  } else {
    std::cout << "Single shot mode" << std::endl;
  }
  
  // Read the image dimensions
  if (!parse_ppm_header(imageFilename.c_str(), &width, &height, &channels)) {
    std::cerr << "Error: could not load " << imageFilename << std::endl;
    teardown(EXIT_FAILURE);
  }
  
  std::cout << "Dimensions are " << width << "x" << height << "x" << channels << std::endl;

  initCL();
   
  // Read the image
  if (!parse_ppm_data(imageFilename.c_str(), &width, &height, &channels, (unsigned char *)input)) {
    std::cerr << "Error: could not load " << imageFilename << std::endl;
    teardown(EXIT_FAILURE);
  }

  if (continuousMode) { // If we are in continous mode
    int i;
    unsigned long long total_time = 0;
    float avgFPS = 0;
    struct timespec start_time = {0,0}, end_time = {0,0}, elap_time = {0,0}, diff_time = {0,0};

    // Determine Average FPS
    for (i=0; i < iterationsToAvg; i++) {
      // Get end of transform time timing
      if(clock_gettime(CLOCK_REALTIME, &start_time)) {
        std::cout << "clock_gettime() - end - error.. exiting." << std::endl;
        teardown(EXIT_FAILURE);
      }
      filter();
      if(clock_gettime(CLOCK_REALTIME, &end_time)) {
        std::cout << "clock_gettime() - end - error.. exiting." << std::endl;
        teardown(EXIT_FAILURE);
      }
      timespec_diff(&start_time, &end_time, &elap_time, true);
      total_time += elap_time.tv_nsec;
    }
    avgFPS = 1000000000.0f / (total_time / iterationsToAvg);
    std::cout << "Avg FPS (" << iterationsToAvg << " iterations): " << avgFPS << std::endl;

    // If we are limiting our FPS
    if (FPSLimited) {
      struct timespec run_time = {0, 0};
      int errVal;

      if (FPSLimit > avgFPS) {
        std::cout << "FPS Limit (" << FPSLimit << ") is greater than the avg fps (" << avgFPS << ")!" << std::endl;
        if (avgFPS - 1 > 0) {
          FPSLimit = floor(avgFPS - 1);
          std::cout << "\tFPS limit set to " << FPSLimit << "." << std::endl;
        } else {
          std::cout << "\tTransform cannot operate in continuous mode due to system limitations. Exiting now!" << std::endl;
          teardown(EXIT_SUCCESS);
        }
      }

      // Inform user that infinite loop will be entered to allow for power measurement
      std::cout << "Program will enter an infinite loop. Use Ctrl+C to exit program when done!" << std::endl;

      // Infinite loop to allow for power measurement
      while (true) {
        if(clock_gettime(CLOCK_REALTIME, &start_time)) {
          std::cout << "clock_gettime() - start - error.. exiting." << std::endl;
          teardown(EXIT_FAILURE);
        }
        filter();
        if(clock_gettime(CLOCK_REALTIME, &end_time)) {
          std::cout << "clock_gettime() - end - error.. exiting." << std::endl;
          teardown(EXIT_FAILURE);
        }
        timespec_diff(&start_time, &end_time, &elap_time, true);
        if(run_time.tv_nsec != 0) {
          // Calculate the timing
          timespec_diff(&elap_time, &run_time, &diff_time, false);

          // Sleep for time needed to allow for running at known frequency
          errVal = nanosleep(&diff_time, NULL);			
          if(errVal == -1) {
            std::cout << std::endl << "Freq delay interrupted. Exiting.." << std::endl;
            std::cout << "**" << errno << " - " << strerror(errno) << "**" << std::endl;
            teardown(EXIT_FAILURE);
          }
        }
      }

    } else { // otherwise transform as fast as possible
      do {
        filter();
      } while (true);
    }
  } else { // otherwise filter once
    filter();
    dump_ppm_data(DUMP_IMAGE, out_width, out_height, channels, output);
  }
  teardown(EXIT_SUCCESS);
}

void 
initCL() {
  cl_int status;

  if (!setCwdToExeDir()) {
    teardown();
  }

  platform = findPlatform("Altera");
  if (platform == NULL) {
    teardown();
  }

  status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, NULL);
  checkError (status, "Error: could not query devices");
  num_devices = 1; // always only using one device

  context = clCreateContext(0, num_devices, &device, &oclContextCallback, NULL, &status);
  checkError(status, "Error: could not create OpenCL context");

  queue = clCreateCommandQueue(context, device, 0, &status);
  checkError(status, "Error: could not create command queue");

  std::string binary_file = getBoardBinaryFile(CL_FUNCTION, device);
  std::cout << "Using AOCX: " << binary_file << std::endl;
  
  program = createProgramFromBinary(context, binary_file.c_str(), &device, 1);
  status = clBuildProgram(program, num_devices, &device, "", NULL, NULL);
  checkError(status, "Error: could not build program");

  pyramidalKernel = clCreateKernel(program, "pyrUp", &status);
  checkError(status, "Error: could not create pyrUp kernel");
  
  out_width = width * 2;
  out_height = height * 2;

  in_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(unsigned int) * height * width, NULL, &status);
  checkError(status, "Error: could not create device buffer");

  out_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(unsigned int) * out_height * out_width, NULL, &status);
  checkError(status, "Error: could not create output buffer");

  status  = clSetKernelArg(pyramidalKernel, 0, sizeof(cl_mem), &in_buffer);
  status |= clSetKernelArg(pyramidalKernel, 1, sizeof(cl_mem), &out_buffer);
  checkError(status, "Error: could not set pyramidal args"); 
    
  workSize[0] = out_width;   // Tell kernel the size of the image
  workSize[1] = out_height;  // ^  
    
  // Create appropriate input/output buffers
  input = (cl_uint*)alignedMalloc(sizeof(unsigned int) * height * width);
  output = (cl_uint*)alignedMalloc(sizeof(unsigned int) * out_width * out_height);
}

void 
cleanup() {
  // Called from aocl_utils::check_error, so there's an error.
  teardown(-1);
}

void 
teardown(int exit_status) {
  if (input) alignedFree(input);
  if (output) alignedFree(output);
  if (in_buffer) clReleaseMemObject(in_buffer);
  if (out_buffer) clReleaseMemObject(out_buffer);
  if (pyramidalKernel) clReleaseKernel(pyramidalKernel);
  if (program) clReleaseProgram(program);
  if (queue) clReleaseCommandQueue(queue);
  if (context) clReleaseContext(context);

  exit(exit_status);
}

