INCLUDES=-I.


#set up compiler and options
CXX = nvcc
CXXFLAGS = $(INCLUDES) -Xcompiler -fopenmp -DTHRUST_DEVICE_BACKEND=THRUST_DEVICE_BACKEND_OMP -lgomp

SRC=backmain.cu setup.cu

EXEC=backtest

$(EXEC):
	$(CXX) -o $@ $(SRC) $(CXXFLAGS)
        
clean:
	rm -f $(EXEC)
