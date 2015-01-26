#include "CConsolePrinter.h"

CConsolePrinter* CConsolePrinter::_instance = 0;  
  
CConsolePrinter::CConsolePrinter()  
{  
    // create a new console to the process  
    AllocConsole();  
      
    int hCrun;      
    hCrun = _open_osfhandle((long)GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT);  
    file  = _fdopen(hCrun, "w");  
      
    // use default stream buffer  
    setvbuf(file, NULL, _IONBF, 0);  
    *stdout = *file;  
	std::cout << "ready!" << std::endl; 
}  
  
CConsolePrinter::~CConsolePrinter()  
{  
    FreeConsole();  
    fclose(file);  
  
}  
  
CConsolePrinter* CConsolePrinter::Instance()  
{  
    if (_instance == 0)  
    {  
        _instance = new CConsolePrinter;  
    }  
    return _instance;  
}  
  
void CConsolePrinter::Destroy()  
{  
    if (_instance)  
    {  
        delete _instance;  
    }  
    _instance = 0;  
}  
  
void CConsolePrinter::print(const char *str)  
{  
    std::cout << str << std::endl;  
} 