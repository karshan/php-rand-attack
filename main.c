#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>

#define MAX_SOURCE_SIZE (0x100000)

typedef struct {
    ulong s;
    ulong us;
} timeval_t;

typedef struct {
    ulong timestamp;
    uint pid;
    timeval_t t1, t2;
} input_t;

/*
pid: 24011
time: 1417215250

Breakpoint 1, __srandom (x=4024746494) at random.c:210
210	random.c: No such file or directory.
(gdb) c
Continuing.
867059331
1136342954
1584836135
988535237
1562953284
51761418
1916084370
1298684710
1100087157
496444727
*/

void set_input(input_t *input, uint pid, ulong timestamp, ulong s1, ulong us1, ulong s2, ulong us2) {
    input->pid = pid;
    input->timestamp = timestamp;
    input->t1.s = s1;
    input->t1.us = us1;
    input->t2.s = s2;
    input->t2.us = us2;
}

int main(void) {
    // Create the two input vectors
    int i, j;
    const int NUM_TIMESTAMPS = 1000000;
    const int NUM_PER = 10;
    const int NUM_SAMPLES = 1;

    input_t *inputs = (input_t*)malloc(sizeof(input_t)*NUM_TIMESTAMPS);
    uint *samples = (uint*)malloc(sizeof(uint)*NUM_SAMPLES);
    for(i = 0; i < NUM_TIMESTAMPS/NUM_PER; i++) {
        for(j = 0; j < NUM_PER; j++) {
            // TODO timestamp generation
            ulong ts = 1417215250 + (i/1000000);
            set_input(&inputs[i * NUM_PER + j], 24011, ts, ts, i % 1000000, ts, ((i + j) % 1000000));
        }
    }

    for(i = 0; i < NUM_SAMPLES; i++) {
        // TODO sample input
        samples[i] = 4024746494;
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
            NUM_TIMESTAMPS * sizeof(input_t), NULL, &ret);
    cl_mem samples_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY,
            NUM_SAMPLES * sizeof(uint), NULL, &ret);
    cl_mem output_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, 
            sizeof(uint), NULL, &ret);

    // Copy the lists A and B to their respective memory buffers
    ret = clEnqueueWriteBuffer(command_queue, timestamps_mem_obj, CL_TRUE, 0,
            NUM_TIMESTAMPS * sizeof(input_t), inputs, 0, NULL, NULL);
    ret = clEnqueueWriteBuffer(command_queue, samples_mem_obj, CL_TRUE, 0, 
            NUM_SAMPLES * sizeof(uint), samples, 0, NULL, NULL);

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
    printf("clEnqueueNDRangeKernel: %d\n", ret);

    // Read the memory buffer C on the device to the local variable C
    uint *output = (uint*)malloc(sizeof(uint));
    *output = 0;
    ret = clEnqueueReadBuffer(command_queue, output_mem_obj, CL_TRUE, 0, 
            sizeof(uint), output, 0, NULL, NULL);

    printf("printin: %u\n", *output);

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
    free(inputs);
    free(samples);
    free(output);
    return 0;
}

