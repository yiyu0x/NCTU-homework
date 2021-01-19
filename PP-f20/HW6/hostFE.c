#include <stdio.h>
#include <stdlib.h>
#include "hostFE.h"
#include "helper.h"

void hostFE(int filterWidth, float *filter, int imageHeight, int imageWidth,
            float *inputImage, float *outputImage, cl_device_id *device,
            cl_context *context, cl_program *program)
{
    cl_int status;
    int filterSize = filterWidth * filterWidth;
    int imageSize = imageWidth * imageHeight;
    
    cl_command_queue queue;
    queue = clCreateCommandQueue(*context, *device, 0, &status);

    cl_mem d_inputImage = clCreateBuffer(*context, 0, imageSize * sizeof(float),
                                                                NULL, &status);
    cl_mem d_outputImage = clCreateBuffer(*context, 0, imageSize * sizeof(float),
                                                                NULL, &status);
    cl_mem d_filter = clCreateBuffer(*context, 0, filterSize * sizeof(float), 
                                                                NULL, &status);

    status = clEnqueueWriteBuffer(queue, d_inputImage, CL_TRUE, 0, 
                            imageSize * sizeof(float), inputImage, 0, NULL, NULL);
    status = clEnqueueWriteBuffer(queue, d_filter, CL_TRUE, 0, 
                            filterSize * sizeof(float), filter, 0, NULL, NULL);

    cl_kernel kernel;
    kernel = clCreateKernel(*program, "convolution", &status);

    status  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_inputImage);
    status |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &d_outputImage);
    status |= clSetKernelArg(kernel, 2, sizeof(int), &imageHeight);
    status |= clSetKernelArg(kernel, 3, sizeof(int), &imageWidth);
    status |= clSetKernelArg(kernel, 4, sizeof(cl_mem), &d_filter);
    status |= clSetKernelArg(kernel, 5, sizeof(int), &filterWidth);

    size_t globalSize[2] = {imageWidth, imageHeight};
    status = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, globalSize, NULL, 0,
                                                                    NULL, NULL);
    status = clEnqueueReadBuffer(queue, d_outputImage, CL_TRUE, 0, 
                                    imageSize * sizeof(float), outputImage, 0,
                                                                    NULL, NULL);                                
}