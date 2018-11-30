// Archivo DLL principal.

#include "stdafx.h"

#include "ClassLibrary1.h"
#include <vector>
using namespace ClassLibrary1;

delegate void MyDelegate(int);

void print1(int x)
{
	Console::WriteLine("print1(): " + x);
}

void print(MyMessage ^msg) {
	
	Console::WriteLine(msg->message);
}

void MyClass::test() {

	MyDelegate ^my_delegate = gcnew MyDelegate(print1);

	MyPrinter ^my_printer = gcnew MyPrinter();
	my_delegate += gcnew MyDelegate(my_printer, &MyPrinter::print2);
	my_delegate += gcnew MyDelegate(&MyPrinter::print3);

	my_delegate(5);
}

void MyClass::test(String ^s) {

	
}