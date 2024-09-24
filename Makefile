main:
	rm -rf clusters.txt
	rm -rf incclusters*.txt
	rm -rf combinedclusters.txt
	g++-11 main.cpp -I include/  -I ../vendor/ -std=c++20 -o main
	./main
	rm -rf main
	python3.9 test/tester.py clusters.txt
	python3.9 test/tester.py incclusters.txt
	python3.9 test/tester.py incclusters2.txt
	python3.9 test/tester.py combinedclusters.txt
	
	

.PHONY:
	main