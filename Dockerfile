FROM ubuntu:16.10 
LABEL maintainer "NVIDIA CORPORATION <cudatools@nvidia.com>"

LABEL com.nvidia.volumes.needed="nvidia_driver"

RUN apt-get update && apt-get install -y --no-install-recommends \
        ocl-icd-libopencl1 \
        clinfo && \
    rm -rf /var/lib/apt/lists/*

RUN mkdir -p /etc/OpenCL/vendors && \
    echo "libnvidia-opencl.so.1" > /etc/OpenCL/vendors/nvidia.icd

RUN echo "/usr/local/nvidia/lib" >> /etc/ld.so.conf.d/nvidia.conf && \
    echo "/usr/local/nvidia/lib64" >> /etc/ld.so.conf.d/nvidia.conf

ENV PATH /usr/local/nvidia/bin:${PATH}
ENV LD_LIBRARY_PATH /usr/local/nvidia/lib:/usr/local/nvidia/lib64

# Devel specific commands follow, can be ommited for runtime only

RUN apt-get update && apt-get install -y --no-install-recommends \
        clinfo \
        ocl-icd-opencl-dev && \
rm -rf /var/lib/apt/lists/*


#setup make
RUN apt-get update
RUN apt-get install -y build-essential

#setup OpenGl headers 
RUN apt-get install -y libglu1-mesa-dev freeglut3-dev mesa-common-dev

# RUN apt-get install nvidia-modprobe

#setup cmake
RUN apt-get install -y cmake
RUN apt-get install -y pkg-config

RUN apt-get install -y opencl-headers

RUN apt-get install -y libopencv-dev

RUN mkdir /opt/opencl
RUN mkdir /opt/opencl/src
COPY src /opt/opencl/src

RUN cd /opt/opencl/src && make
CMD cd /opt/opencl/src && ./gauss
