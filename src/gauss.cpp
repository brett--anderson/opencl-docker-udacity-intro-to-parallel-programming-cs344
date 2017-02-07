#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include "gauss.hpp"
#include "util.hpp"
#include "err_code.h"
#include "device_picker.hpp"
#include "matrix_lib.hpp"

std::vector<double> create_convolution_matrix(double sigma, int maskSize){
    /*
    CREATES CONVOLUTION MATRIX FOR GAUSSIAN BLUR,
    GIVEN SIGMA AND DIMENSION OF THE DESIRED FILTER.
    */

    int W = maskSize;
    double kernel[maskSize][maskSize];
    std::vector<double> result;
    double mean = W / 2;
    double sum = 0.0;
    for (int x = 0; x < W; ++x)
        for (int y = 0; y < W; ++y) {
        kernel[x][y] =
            exp(-0.5 * (pow((x - mean) / sigma, 2.0) +
            pow((y - mean) / sigma, 2.0))) /
            (2 * 3.14159 * sigma * sigma);

        // ACCUMULATE VALUES
        sum += kernel[x][y];
        }

    // NORMALIZE
    for (int x = 0; x < W; ++x)
        for (int y = 0; y < W; ++y)
            kernel[x][y] /= sum;

    for (int x = 0; x < W; ++x) {
        for (int y = 0; y < W; ++y) {
            result.push_back(kernel[x][y]);
        }
    }
    return result;
}

//  Load an image using the OpenCV library and create an OpenCL
//  image out of it
cl::Image2D LoadImage(cl::Context context, char *fileName, int &width, int &height)
{
    cv::Mat image = cv::imread(fileName, CV_LOAD_IMAGE_COLOR);
    cv::Mat imageRGBA;
    
    width = image.cols;
    height = image.rows;
    
    cv::cvtColor(image, imageRGBA, CV_RGB2RGBA);

    char *buffer = reinterpret_cast<char *>(imageRGBA.data);
    
    cl::Image2D clImage(context,
                            CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                            cl::ImageFormat(CL_RGBA, CL_UNORM_INT8),
                            width,
                            height,
                            0,
                            buffer);
    return clImage;
}

int main(int argc, char *argv[])
{

    char * filename = "images/sunset.jpg";
    char * filename_out = "images/sunset_blur.jpg";

//--------------------------------------------------------------------------------
// Create a context and queue
//--------------------------------------------------------------------------------

    try
    {

        cl_uint deviceIndex = 0;
        parseArguments(argc, argv, &deviceIndex);

        // Get list of devices
        std::vector<cl::Device> devices;
        unsigned numDevices = getDeviceList(devices);

        // Check device index in range
        if (deviceIndex >= numDevices)
        {
          std::cout << "Invalid device index (try '--list')\n";
          return EXIT_FAILURE;
        }

        cl::Device device = devices[deviceIndex];

        std::string name;
        getDeviceName(device, name);
        std::cout << "\nUsing OpenCL device: " << name << "\n";

        std::vector<cl::Device> chosen_device;
        chosen_device.push_back(device);
        cl::Context context(chosen_device);
        cl::CommandQueue queue(context, device);

//--------------------------------------------------------------------------------
// Setup the buffers, initialize matrices, and write them into global memory
//--------------------------------------------------------------------------------

        //Load input image to the host and to the GPU
        int width, height;
        cl::Image2D clImageInput;
        clImageInput = LoadImage(context, filename, width, height);
        printf("\nwidth%d height: %d",width,height);
        
        //Create output image object
        cl::Image2D imageOutput(context,
                    CL_MEM_WRITE_ONLY,
                    cl::ImageFormat(CL_RGBA, CL_UNORM_INT8),
                    width,
                    height,
                    0,
                    NULL);

        // Create Gaussian mask
        float gauss_sigma = 2.0;
        int maskSize = 5;
        // CREATE CONVOLUTION MATRIX
        std::vector<double> matrix = create_convolution_matrix(gauss_sigma, maskSize);
        // CONVOLUTION MATRIX TO NORMAL ARRAY
        float flat_matrix[maskSize*maskSize];
        for (int i = 0; i < matrix.size(); ++i){
            flat_matrix[i] = matrix.at(i);
        }

        // Create buffer for mask and transfer it to the device
        cl::Buffer clMask(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float)*(maskSize*maskSize), flat_matrix);

//--------------------------------------------------------------------------------
// OpenCL gaussian blur
//--------------------------------------------------------------------------------

        cl::Program program(context, util::loadProgram("gauss.cl"), true);
        //cl::Program program(context, util::loadProgram("border.cl"), true);

        // Create the compute kernel from the program
        cl::make_kernel<cl::Image2D, cl::Buffer, cl::Image2D, int, int, int> gaussianBlur(program, "process");
        //cl::make_kernel<cl::Image2D, cl::Image2D> filter(program, "process");

        cl::NDRange global(width, height);
        gaussianBlur(cl::EnqueueArgs(queue, global),
                clImageInput, clMask, imageOutput, maskSize, width, height);

        // cl::NDRange global(width, height);
        // filter(cl::EnqueueArgs(queue, global),
        //         clImageInput, imageOutput);

        queue.finish();

        cl_uint8* oup = new cl_uint8[width * height];

        cl::size_t<3> origin;
        origin[0] = 0; origin[1] = 0, origin[2] = 0;
        cl::size_t<3> region;
        region[0] = width; region[1] = height; region[2] = 1;

        queue.enqueueReadImage(imageOutput, CL_TRUE, origin, region, 0, 0, oup);

        cv::imwrite(filename_out,  cv::Mat(height, width, CV_8UC4, oup));

    } catch (cl::Error err)
    {
        std::cout << "Exception\n";
        std::cout << "ERROR: "
                  << err.what()
                  << "("
                  << err_code(err.err())
                  << ")"
                  << std::endl;
    }

    return EXIT_SUCCESS;
}