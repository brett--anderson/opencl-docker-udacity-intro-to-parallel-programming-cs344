## Getting Started

nvidia-docker build .

nvidia-docker run -v $PWD/src/images:/opt/opencl/src/images < id returned from build >

## Setting up Nvidia-Docker

I ran this code with an Nvidia GeForce 980M. The code is OpenCL so it should run on other processors without any modification. That said, the Docker file and docker engine are Nvidia specific so these will need to be modified to run the code on other hardware from inside Docker.

You don't have to use Docker to run this project and the Dockerfile can be examined to determine the libraries that need to be installed on Ubuntu 16.10 for the code to work with an Nvidia GPU. Some of the installation steps will be different for other GPU's. If you are using Docker and an Nvidia GPU, read on...

You first need to install the Nvidia drivers. After much messing around I figured out the best way was to go into Software & Updates => Additional drivers and there was the Nvidia card using the Nouveau open source drivers (My host OS is Ubuntu-Gnome 16.10). The Nouveau drivers did not play nice with my setup and I need to switch these to the proprietary Nvidia drivers (which can be done easily from this window). I spent hours trying to do this from the command line before figuring this out. The main problem is that you need to disable the Nouveau drivers before installing anything else, this is non-trivial from the command line.

I found I also needed to install ```sudo apt install nvidia-modprobe```

Nvidia actually provide their own Docker engine for running containers that need access to the GPU. I haven’t looked into exactly how specialised this is but it does rely on docker-engine as a dependency, so I guess it just augments it. Speaking of, I needed to install docker-engine from the third party docker repo ([Provided by Docker](https://docs.docker.com/engine/installation/linux/ubuntu/)) rather than apt-get install docker-io which uses the docker engine that ships with Ubuntu. Nvidia-docker doesn’t recognize the ‘io’ engine, it’s also not the latest.

After installing the correct docker-engine and Nvidia drivers I then tried to install the nvidia-docker deb file as per the [instructions](https://github.com/NVIDIA/nvidia-docker). The nvidia-docker deb file from the nvida-docker github install instructions failed because it doesn’t support 16.10 yet. I was able to grab the deb spec [from github](https://github.com/NVIDIA/nvidia-docker/blob/master/Dockerfile.deb) and modify the ‘From’ line at the top from 14.04 to 16.04. Then I installed make and was able to build the deb file with ‘make deb’. Then I could install it with: sudo dpkg -i < deb file name >
This successfully installed nvidia-docker from this modified deb file.

## Motivation and Repo's I used

The original motivation for this project was to convert the CUDA examples across to OpenCL from the excellent [Udacity course](www.udacity.com/course/intro-to-parallel-programming--cs344) on parallel programming. This is essentially the equivalent of the first weeks problem set (if you use the grayscale kernel). Udacity uses OpenCV for working with the images and I wanted to use it in the OpenCL version as well to familiarise myself with a machine vision library.

I also wanted to try running everything in docker and later I will see how seamlesly I can deploy the image to AWS HPC.

The Gaussian mask and kernel function was copied [from this repo](https://github.com/mnmnc/gaussian_blur_opencl). I created my own version because I wanted to use OpenCV to work with the images and I also wanted to use the cl::Image2D class to move the image to and from the device.

I also took inspiration from [this version](https://github.com/smistad/OpenCL-Gaussian-Blur). I created my own version rather than use this one because I wanted to use the cl::Image2D class for both input and output from the device, rather than just input. I also wanted to use OpenCV and the version linked to uses it's own rather large image library. I also had trouble running it inside nvidia-docker since the image library tries to open a render window to display the result.

Much of the helper libraries came from [this excellent resource](https://github.com/HandsOnOpenCL/Exercises-Solutions). They also provide a great introductory set of lectures and problem sets for someone starting out with OpenCL

