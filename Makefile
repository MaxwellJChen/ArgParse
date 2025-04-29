all:
	@clang++ -std=c++17 main.cpp foo.cpp -o main
	@./main