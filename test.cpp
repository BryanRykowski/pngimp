#define PNGIMP_IMPL
#include "pngimp/pngimp.hpp"

int main(int argc, char** argv)
{
    pngimp::BufferStruct buff = pngimp::import("test0.png");
    
    return 0;
}