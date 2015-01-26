#include <io.h>
#include <Fcntl.h>
#include <stdarg.h>
#include <iostream>
#include <windows.h>
#include "stdio.h"
class CConsolePrinter    
{  
public:  
    void print(const char* str);  
    static void Destroy();  
    static CConsolePrinter* Instance();  
      
      
    virtual ~CConsolePrinter();  
  
protected:  
    CConsolePrinter();  
  
private:  
    static CConsolePrinter* _instance;  
    FILE *file;  
  
};