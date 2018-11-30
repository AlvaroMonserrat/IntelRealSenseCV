// ClassLibrary1.h

#pragma once

using namespace System;

namespace ClassLibrary1 {

	ref class MyPrinter
	{
		public:
		void print2(int x)
		{
			Console::WriteLine("print2(): " + x);
		}
		static void print3(int x)
		{
			Console::WriteLine("print3(): " + x);
		}
	};

	public ref class MyClass
	{
	public:
		void test();
		void test(String ^s);
	};

	public ref class MyMessage
	{
	public:
		String ^message;
	
	};
}
