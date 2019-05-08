/**
* @file Main.cpp
*/
#include <iostream>

class Stack {
public:

	void push(int num) {
		sArray[count] = num;
		count++;
	}

	int pop() {	
		count > 0 ? count-- : 0;		
		return sArray[count];
	}	

private:
	unsigned int sArray[10];
	unsigned count = 0;
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