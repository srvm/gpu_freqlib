all: demo

INCLUDES = -I ../include/
INCLUDES += -I $(GDK_PATH)
LIBS = -lnvidia-ml
CXXARGS = -std=c++11 -O3

demo: demo.cu
	nvcc $(CXXARGS) $(INCLUDES) $(LIBS) demo.cu -o demo

clean:
	rm -f *.o demo
