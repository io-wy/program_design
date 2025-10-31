#include <iostream>
#include "pharmacy.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    system("chcp 65001>nul"); 
    std::string dataPath = "data/pharmacy.db";
    Pharmacy app(dataPath);
    app.run();
    return 0;
}