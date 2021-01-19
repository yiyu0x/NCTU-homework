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

__global__ void mandelKernel(float x1, float y1, float x0, float y0, int* output, int width, int height, int maxIterations) {
    // To avoid error caused by the floating number, use the following pseudo code
    // float x = lowerX + thisX * stepX;
    // float y = lowerY + thisY * stepY;
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    int j = blockIdx.y * blockDim.y + threadIdx.y;
    if (i >= width || j >= height) return;
    // i -> 1600, j -> 1200
    float dx = (x1 - x0) / width;
    float dy = (y1 - y0) / height;

    float x = x0 + i * dx;
    float y = y0 + j * dy;
    int index = (j * width + i);
    output[index] = mandel(x, y, maxIterations);
}

// Host front-end function that allocates the memory and launches the GPU kernel
#define N 1600
#define BLOCK_SIZE 64
void hostFE (float x1, float y1, float x0, float y0, int* output, int width, int height, int maxIterations)
{
    int *h_img, *d_img;
    // Locate CPU memory and GPU memory
    h_img = (int*)malloc(width * height * sizeof(int)); 
    cudaMalloc((void **)&d_img, width * height * sizeof(int));
    // Copy memory from CPU to GPU
    // cudaMemcpy(d_img, h_img, width * height * sizeof(int), cudaMemcpyHostToDevice);
    // 
    dim3 blockSize(BLOCK_SIZE, BLOCK_SIZE);
    dim3 numBlock(N / BLOCK_SIZE, N / BLOCK_SIZE);
    mandelKernel<<<blockSize, numBlock>>>(x1, y1, x0, y0, d_img, width, height, maxIterations);
    // Sync
    cudaDeviceSynchronize();
    // Copy memory from GPU to CPU
    cudaMemcpy(h_img, d_img, width * height * sizeof(int), cudaMemcpyDeviceToHost);
    // Copy memory from CPU to CPU(answers)
    memcpy(output, h_img, width * height * sizeof(int));
    free(h_img);
    cudaFree(d_img);
}
