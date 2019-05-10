/**
* @file Main.cpp
*/
#include <iostream>

#define STACK_SIZE 10

/**
* スタッククラスの実装
*/
class Stack {
public:
	void push(int num) {
		stackArray[dataSize] = num;
		dataSize < STACK_SIZE ? dataSize++ : STACK_SIZE;
	}

	int pop() {	
		dataSize > 0 ? dataSize-- : 0;
		return stackArray[dataSize];
	}	

private:
	int stackArray[STACK_SIZE];
	unsigned dataSize = 0;
};



int main() {
	/*std::cout << "Hello World" << std::endl;*/

	Stack s;

	s.push(10);	
	s.push(30);
	std::cout << s.pop() << std::endl;
	s.push(20);
	s.push(100);
	s.pop();
	std::cout << s.pop() << std::endl;
	std::cout << s.pop() << std::endl;
	return 0;
}