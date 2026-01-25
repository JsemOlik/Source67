#include "Core/Application.h"
#include "Core/Logger.h"

int main(int argc, char** argv) {
    S67::Logger::Init();
    S67_CORE_INFO("Source67 Engine Initialized");

    auto app = new S67::Application(argv[0]);
    app->Run();
    delete app;

    return 0;
}