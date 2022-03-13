a.out: *.cpp *.hpp Makefile
	g++ *.cpp -std=c++2a -lpng -ltbb -O3