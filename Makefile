main:
	rm -rf clusters.txt
	rm -rf incclusters*.txt
	rm -rf combinedclusters.txt
	g++-11 main.cpp -I include/  -I ../vendor/ -std=c++20 -o main
	./main
	rm -rf main
	

.PHONY:
	main