main:
	rm -rf clusters.txt
	rm -rf incclusters*.txt
	rm -rf combinedclusters.txt
	g++-11 main.cpp -I include/  -I ../vendor/ -std=c++20 -o main
	./main
	rm -rf main
	python3.9 test/tester.py clusters.txt
	python3.9 test/tester.py incclusters1.txt
	python3.9 test/tester.py incclusters2.txt
	python3.9 test/tester.py incclusters3.txt
	python3.9 test/tester.py incclusters4.txt
	python3.9 test/tester.py incclusters5.txt
	python3.9 test/tester.py incclusters6.txt
	python3.9 test/tester.py incclusters7.txt
	python3.9 test/tester.py incclusters8.txt
	python3.9 test/tester.py combinedclusters.txt
	
	

.PHONY:
	main