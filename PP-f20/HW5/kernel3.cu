#include <cuda.h>
#include <stdio.h>
#include <stdlib.h>

__device__ int mandel(float c_re, float c_im, int maxIteration)
{
  float z_re = c_re, z_im = c_im;
  int i;
  for (i = 0; i < maxIteration; ++i)
  {

    if (z_re * z_re + z_im * z_im > 4.f)
      break;

    float new_re = z_re * z_re - z_im * z_im;
    float new_im = 2.f * z_re * z_im;
    z_re = c_re + new_re;
    z_im = c_im + new_im;
  }

  return i;
}

__global__ void mandelKernel(float x1, float y1, float x0, float y0, int* output, int width, int height, int maxIterations, int pitch, int pixels) {
    // To avoid error caused by the floating number, use the following pseudo code
    // float x = lowerX + thisX * stepX;
    // float y = lowerY + thisY * stepY;
    int i = (blockIdx.x * blockDim.x + threadIdx.x) * pixels;
    int j = blockIdx.y * blockDim.y + threadIdx.y;
    // if (i >= width || j >= height) return;
    if (i >= width || j >= height) return;
    // i -> 1600, j -> 1200
    float dx = (x1 - x0) / width;
    float dy = (y1 - y0) / height;

    float y = y0 + j * dy;
    for(int pixel = 0; pixel < pixels; pixel++) {
        float x = x0 + (i + pixel) * dx;
        int index = (j * pitch + i) + pixel;
        output[index] = mandel(x, y, maxIterations);
    }
    
}

// Host front-end function that allocates the memory and launches the GPU kernel
#define N 1600
#define BLOCK_SIZE 64
void hostFE (float x1, float y1, float x0, float y0, int* output, int width, int height, int maxIterations)
{   
    int *h_img = NULL, *d_img = NULL;
    size_t pitch;
    // Locate CPU memory and GPU memory
    cudaHostAlloc((void**)&h_img, width * height * sizeof(int), cudaHostAllocDefault);
    cudaMallocPitch((void**)&d_img, &pitch, (size_t)width * sizeof(int), (size_t)height);
    // Copy memory from CPU to GPU
    cudaMemcpy2D(d_img, pitch, h_img, width * sizeof(int), width * sizeof(int), height, cudaMemcpyHostToDevice);
    // 
    dim3 blockSize(BLOCK_SIZE, BLOCK_SIZE);
    dim3 numBlock(N / BLOCK_SIZE, N / BLOCK_SIZE);
    mandelKernel<<<blockSize, numBlock>>>(x1, y1, x0, y0, d_img, width, height, maxIterations, pitch / sizeof(int), 2);
    // Sync
    cudaDeviceSynchronize();
    // // Copy memory from GPU to CPU
    cudaMemcpy2D(h_img, width * sizeof(int), d_img, pitch, width * sizeof(int), height, cudaMemcpyDeviceToHost);
    // // Copy memory from CPU to CPU(answers)
    memcpy(output, h_img, width * height * sizeof(int));
    cudaFree(h_img);
    cudaFree(d_img);
}
