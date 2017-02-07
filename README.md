## Getting Started

nvidia-docker build .

nvidia-docker run -v $PWD/src/images:/opt/opencl/src/images < id returned from build >

## Setting up Nvidia-Docker

Installed Nvidia drivers, after much messing around I figured out the best way was to go into Software & Updates => Additional drivers and there was the Nvidia card using the Nouveau open source drivers (which cause all sorts of issues with some later steps). There you can choose to use the proprietary Nvidia drivers. I spent hours trying to do this from the command line before figuring this out, ugghhh. The main problem is that you need to disable the Nouveau drivers before installing anything else, this is non-trivial from the command line.

Nvidia actually provide their own Docker engine for running containers that need access to the GPU. I haven’t looked into exactly how specialised this is but it does rely on docker-engine as a dependency, so I guess it just augments it. Speaking of, I needed to install docker-engine from the third party docker repo (Provided by Docker) rather than apt-get install docker-io which uses the docker engine that ships with Ubuntu. Nvidia-docker doesn’t recognize the ‘io’ engine, it’s also not the latest.
So, installed the correct docker-engine and Nvidia drivers, then the nvidia-docker deb file from the nvida-docker github install instructions failed because it doesn’t support 16.10 yet. I was able to grab the deb spec from github and modify the ‘From’ line at the top from 14.04 to 16.04. Then I installed make and was able to build the deb file with ‘make deb’. Then I could install it with: sudo dpkg -i < deb file name >
This successfully installed nvidia-docker from this modified deb file.

## Motivation and Repo's I used

The original motivation for this project was to convert the CUDA examples across to OpenCL from the excellent [Udacity course](www.udacity.com/course/intro-to-parallel-programming--cs344) on parallel programming. This is essentially the equivalant of the first weeks problem set (if you use the grayscale kernel). Udacity uses OpenCV for working with the images and I wanted to use it in the OpenCL version as well to familiarise myself with a library that I would like to use to learn more about machine vision.

The Gaussian mask and kernel function was copied [from this repo](https://github.com/mnmnc/gaussian_blur_opencl). I created my own verison because I wanted to use OpenCV to work with the images and I also wanted to use the cl::Image2D class to move the image to and from the device.

I also took insipration from [this version](https://github.com/smistad/OpenCL-Gaussian-Blur) too. I created my own version rather than use this one because I wanted to use the cl::Image2D class for both input and output from the device, rather than just input. I also wanted to use OpenCV and the version linked to uses it's own rather large image library. I also had trouble running it inside nvidia-docker since the image library tries to open a render window to display the result.

Much of the helper libraries came from [this excellent resource](https://github.com/HandsOnOpenCL/Exercises-Solutions). They also provide a great introductory set of lectures and problem sets for someone starting out with OpenCL

