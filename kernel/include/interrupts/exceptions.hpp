#pragma once

extern void (*exceptionHandlers[32])();


void exceptionDoubleFault();
void exceptionGPFault();
void exceptionPageFault();


extern "C" void  _iExc0();
extern "C" void  _iExc1();
extern "C" void  _iExc2();
extern "C" void  _iExc3();
extern "C" void  _iExc4();
extern "C" void  _iExc5();
extern "C" void  _iExc6();
extern "C" void  _iExc7();
extern "C" void  _iExc8();
extern "C" void  _iExc9();
extern "C" void  _iExc10();
extern "C" void  _iExc11();
extern "C" void  _iExc12();
extern "C" void  _iExc13();
extern "C" void  _iExc14();
extern "C" void  _iExc15();
extern "C" void  _iExc16();
extern "C" void  _iExc17();
extern "C" void  _iExc18();
extern "C" void  _iExc19();
extern "C" void  _iExc20();
extern "C" void  _iExc21();
extern "C" void  _iExc22();
extern "C" void  _iExc23();
extern "C" void  _iExc24();
extern "C" void  _iExc25();
extern "C" void  _iExc26();
extern "C" void  _iExc27();
extern "C" void  _iExc28();
extern "C" void  _iExc29();
extern "C" void  _iExc30();
extern "C" void  _iExc31();