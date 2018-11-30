#include <iostream>

int main() {

	System::Console::WriteLine(L"Managed Hello World");
	std::cout << "Native Hello World" << std::endl;
	
	system("pause");
	return 0;
}