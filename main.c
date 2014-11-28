#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>

#define MAX_SOURCE_SIZE (0x100000)

int main(void) {
    // Create the two input vectors
    int i;
    const int NUM_TIMESTAMPS = 1024;
    const int NUM_SAMPLES = 10;

    int *timestamps = (int*)malloc(sizeof(int)*NUM_TIMESTAMPS);
    int *samples = (int*)malloc(sizeof(int)*NUM_SAMPLES);
    for(i = 0; i < NUM_TIMESTAMPS; i++) {
        // TODO timestamp generation
        timestamps[i] = i;
    }

    for(i = 0; i < NUM_SAMPLES; i++) {
        // TODO sample input
        samples[i] = i;
    }

    // Load the kernel source code into the array source_str
    FILE *fp;
    char *source_str;
    size_t source_size;

    fp = fopen("attack.cl", "r");
    if (!fp) {
        fprintf(stderr, "Failed to load kernel.\n");
        exit(1);
    }
    source_str = (char*)malloc(MAX_SOURCE_SIZE);
    source_size = fread( source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose( fp );

    // Get platform and device information
    cl_platform_id platform_id = NULL;
    cl_device_id device_id = NULL;   
    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;
    cl_int ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
    ret = clGetDeviceIDs( platform_id, CL_DEVICE_TYPE_ALL, 1, 
            &device_id, &ret_num_devices);

    // Create an OpenCL context
    cl_context context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret);

    // Create a command queue
    cl_command_queue command_queue = clCreateCommandQueue(context, device_id, 0, &ret);

    // Create memory buffers on the device for each vector 
    cl_mem timestamps_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, 
            NUM_TIMESTAMPS * sizeof(int), NULL, &ret);
    cl_mem samples_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY,
            NUM_SAMPLES * sizeof(int), NULL, &ret);
    cl_mem output_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, 
            sizeof(int), NULL, &ret);

    // Copy the lists A and B to their respective memory buffers
    ret = clEnqueueWriteBuffer(command_queue, timestamps_mem_obj, CL_TRUE, 0,
            NUM_TIMESTAMPS * sizeof(int), timestamps, 0, NULL, NULL);
    ret = clEnqueueWriteBuffer(command_queue, samples_mem_obj, CL_TRUE, 0, 
            NUM_SAMPLES * sizeof(int), samples, 0, NULL, NULL);

    // Create a program from the kernel source
    cl_program program = clCreateProgramWithSource(context, 1, 
            (const char **)&source_str, (const size_t *)&source_size, &ret);

    // Build the program
    ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
    printf("clBuildProgram: %d\n", ret);

    // Create the OpenCL kernel
    cl_kernel kernel = clCreateKernel(program, "attack", &ret);

    // Set the arguments of the kernel
    ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&timestamps_mem_obj);
    ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&samples_mem_obj);
    ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&output_mem_obj);
    
    // Execute the OpenCL kernel on the list
    size_t global_item_size = NUM_TIMESTAMPS; // Process the entire lists
    size_t local_item_size = 64; // Process in groups of 64
    ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, 
            &global_item_size, &local_item_size, 0, NULL, NULL);

    // Read the memory buffer C on the device to the local variable C
    int *output = (int*)malloc(sizeof(int));
    ret = clEnqueueReadBuffer(command_queue, output_mem_obj, CL_TRUE, 0, 
            NUM_TIMESTAMPS * sizeof(int), output, 0, NULL, NULL);

    printf("printin: %d\n", *output);

    // Clean up
    ret = clFlush(command_queue);
    ret = clFinish(command_queue);
    ret = clReleaseKernel(kernel);
    ret = clReleaseProgram(program);
    ret = clReleaseMemObject(timestamps_mem_obj);
    ret = clReleaseMemObject(samples_mem_obj);
    ret = clReleaseMemObject(output_mem_obj);
    ret = clReleaseCommandQueue(command_queue);
    ret = clReleaseContext(context);
    free(timestamps);
    free(samples);
    free(output);
    return 0;
}

