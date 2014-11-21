all: demo

INCLUDES = -I ../include/

demo: demo.cu
	nvcc -std=c++11 -O3 $(INCLUDES) demo.cu -o demo

clean:
	rm -f *.o demo
