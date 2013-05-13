INCLUDES=-I.

#set up compiler and options
CXX = nvcc
CXXFLAGS = $(INCLUDES)

SRC=backmain.cu setup.cu

EXEC=backtest

$(EXEC):
	$(CXX) -o $@ $(SRC) $(CXXFLAGS) -arch=sm_20
        
clean:
	rm -f $(EXEC)
