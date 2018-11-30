#include "MyForm.h"

using namespace System;
using namespace System::Windows::Forms;


[STAThread]
void WinMain()
{
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);

	Prototype7_WindowForm1::MyForm form;
	
	Application::Run(%form);
}

void Main(array<String^>^ args)
{

}