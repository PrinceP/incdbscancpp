main:
	rm -rf clusters.txt
	rm -rf incclusters.txt
	g++-11 main.cpp -I include/  -I ../vendor/ -std=c++20 -o main
	./main
	rm -rf main
	

.PHONY:
	main