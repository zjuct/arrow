#include <prt.h>

#include <iostream>

int main(int argc, char** argv) {
//    if(argc != 3) {
//        std::cout << "Usage: prtbackend <scene-file> <skybox-file>" << std::endl;
//        exit(1);
//    }
//    PRTPrecompute(argv[1], argv[2]);
    PRTPrecompute("resource/assets/scene/scene.obj", "resource/assets/CornellBox");
    return 0;
}