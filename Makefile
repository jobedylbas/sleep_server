export ROOT_DIR=${PWD}

CXX=g++
CFLAGS=-Wall -pthread

main: manager

manager: ${ROOT_DIR}/src/main.cpp
	$(CXX) $(CFLAGS) -o sleep_server ${ROOT_DIR}/src/main.cpp ${ROOT_DIR}/src/manager/manager.cpp ${ROOT_DIR}/src/manager/client.cpp

clean:
	rm $(EXEC) *.o