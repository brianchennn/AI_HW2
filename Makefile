all:
	g++ -std=c++11 main2.cpp -o main2 -lpthread
	g++ -std=c++11 main.cpp -o main -lpthread
	g++ -std=c++11 decen.cpp -o decen -lpthread
clean:
	rm ./main ./main2 ./decen
