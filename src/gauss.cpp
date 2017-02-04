#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include "gauss.hpp"
#include "util.hpp"
#include "err_code.h"
#include "device_picker.hpp"
#include "matrix_lib.hpp"

float * createBlurMask(float sigma, int * maskSizePointer) {
    int maskSize = (int)ceil(3.0f*sigma);
    float * mask = new float[(maskSize*2+1)*(maskSize*2+1)];
    float sum = 0.0f;
    for(int a = -maskSize; a < maskSize+1; a++) {
        for(int b = -maskSize; b < maskSize+1; b++) {
            float temp = exp(-((float)(a*a+b*b) / (2*sigma*sigma)));
            sum += temp;
            mask[a+maskSize+(b+maskSize)*(maskSize*2+1)] = temp;
        }
    }
    // Normalize the mask
    for(int i = 0; i < (maskSize*2+1)*(maskSize*2+1); i++)
        mask[i] = mask[i] / sum;

    *maskSizePointer = maskSize;

    return mask;
}

//  Load an image using the OpenCV library and create an OpenCL
//  image out of it
cl::Image2D LoadImage(cl::Context context, char *fileName, int &width, int &height)
{
    cv::Mat image = cv::imread(fileName, CV_LOAD_IMAGE_COLOR);
    cv::Mat imageRGBA;
    
    width = image.rows;
    height = image.cols;
    
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

    int N;                  // A[N][N], B[N][N], C[N][N]
    int size;               // Number of elements in each matrix

    double start_time;      // Starting time
    double run_time;        // Timing
    util::Timer timer;      // Timing

    N    = ORDER;
    size = N * N;

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

        cl::size_t<3> origin;
        origin[0] = 0; origin[1] = 0, origin[2] = 0;
        cl::size_t<3> region;
        region[0] = width; region[1] = height; region[2] = 1;

        // Create a buffer for the result
        //cl::Buffer clResult(context, CL_MEM_WRITE_ONLY, sizeof(float)*width*height);

        // Create Gaussian mask
        int maskSize;
        float * mask = createBlurMask(2.0f, &maskSize);

        // Create buffer for mask and transfer it to the device
        cl::Buffer clMask(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float)*(maskSize*2+1)*(maskSize*2+1), mask);

//--------------------------------------------------------------------------------
// OpenCL gaussian blur
//--------------------------------------------------------------------------------

        // cl::Program program(context, util::loadProgram("gauss.cl"), true);
        cl::Program program(context, util::loadProgram("gauss.cl"), true);

        // Create the compute kernel from the program
        cl::make_kernel<cl::Image2D, cl::Buffer, cl::Image2D, int> gaussianBlur(program, "process");
        //cl::make_kernel<cl::Image2D, cl::Image2D> filter(program, "process");

        cl::NDRange global(width, height);
        gaussianBlur(cl::EnqueueArgs(queue, global),
                clImageInput, clMask, imageOutput, maskSize);

        // cl::NDRange global(width, height);
        // filter(cl::EnqueueArgs(queue, global),
        //         clImageInput, imageOutput);

        queue.finish();

        float* oup = new float[width * height];

        queue.enqueueReadImage(imageOutput, CL_TRUE, origin, region, 0, 0, oup);

        // if(clEnqueueReadImage(cl.getCommandQueue(), imageOutput, CL_TRUE, new long[] {0,0,0}, 
        //         new long[] {width, height, 1}, width, 0, Pointer.to(grayimg_byte), 0, null, null) != CL_SUCCESS) {
        //     System.err.println("Cant read values from Blackwhite-kernel");
        //     System.exit(1);
        // }

        // // Transfer image back to host
        //float* data = new float[width*height];
        //queue.enqueueReadBuffer(clResult, CL_TRUE, 0, sizeof(float)*width*height, data)


        cv::imwrite(filename_out,  cv::Mat(width, height, CV_8UC4, oup));

    } catch (cl::Error err)
    {
        std::cout << "Exception\n";
        std::cerr << "ERROR: "
                  << err.what()
                  << "("
                  << err_code(err.err())
                  << ")"
                  << std::endl;
    }

    return EXIT_SUCCESS;
}