all: exe output 
exe: output
	./output
output: main.o Performance.o MemoryManager.o TaskManager.o
	g++ main.o Performance.o MemoryManager.o TaskManager.o -o output

main.o:
	g++ -c main.cpp
Performance.o: Performance.cpp
	g++ -c Performance.cpp
MemoryManager.o: MemoryManager.cpp
	g++ -c MemoryManager.cpp
TaskManager.o: TaskManager.cpp
	g++ -c TaskManager.cpp


clean:
	rm -rf *o output