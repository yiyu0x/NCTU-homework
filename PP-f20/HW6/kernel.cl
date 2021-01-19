__kernel void convolution(
    __global const float*  inputImage,
    __global float*  outputImage,
    int rows,
    int cols,
    __global float* filter,
    int filterWidth
) {
    int column = get_global_id(0);
    int row    = get_global_id(1);

    int halfWidth = (int)(filterWidth/2);
    float sum = 0.0f;
        
    for(int k = -halfWidth; k <= halfWidth; k++) {
        for(int l = -halfWidth; l <= halfWidth; l++) {
            sum += inputImage[(row + k) * cols + column + l] *
                        filter[(k + halfWidth) * filterWidth + l + halfWidth];
        }
    }

    if(row < rows && column < cols) {
        outputImage[row * cols + column] = sum;
    } 
}
