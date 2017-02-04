ifndef CPPC
	CPPC=g++
endif

OPENCV_LIBPATH=/usr/local/opt/opencv3/lib/
OPENCV_INCLUDEPATH=/usr/local/opt/opencv3/include/

OPENCV_LIBS=-lopencv_core -lopencv_imgproc -lopencv_highgui

CCFLAGS=-O3 -ffast-math

LIBS = -lm -lOpenCL -fopenmp

COMMON_DIR = Cpp_common

INC = -I $(COMMON_DIR) -I $(OPENCV_LIBPATH) -I $(OPENCV_INCLUDEPATH)

GAUSS_OBJS = gauss.o matrix_lib.o
EXEC = gauss

# Check our platform and make sure we define the APPLE variable
# and set up the right compiler flags and libraries
PLATFORM = $(shell uname -s)
ifeq ($(PLATFORM), Darwin)
	CPPC = clang++
	CCFLAGS += -stdlib=libc++
	LIBS = -lm -framework OpenCL
endif

all: $(EXEC)

gauss: $(GAUSS_OBJS)
	$(CPPC) $(GAUSS_OBJS) $(CCFLAGS) -L $(OPENCV_LIBPATH) $(LIBS) $(OPENCV_LIBS) -o $(EXEC)

.c.o:
	$(CPPC) -c $< $(CCFLAGS) -o $@

.cpp.o:
	$(CPPC) -c $< $(CCFLAGS) $(INC) -o $@

gauss.o:	gauss.hpp matrix_lib.hpp

matrix_lib.o:	gauss.hpp

clean:
	rm -f $(GAUSS_OBJS) $(EXEC)
