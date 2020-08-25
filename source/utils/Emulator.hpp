#include <iostream>
#include <stdio.h>
#include <string.h>
#include <switch.h>

using namespace std;

static void printToEmu(std::string str) {
  svcOutputDebugString(str.c_str(), strlen(str.c_str()));
}