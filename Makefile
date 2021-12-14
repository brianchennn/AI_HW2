all:
	g++ -std=c++11 decen.cpp -o main -lpthread
clean:
	rm ./main
