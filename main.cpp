#include "Core/Application.h"
#include "Core/Logger.h"

int main() {
    S67::Logger::Init();
    S67_CORE_INFO("Source67 Engine Initialized");

    auto app = new S67::Application();
    app->Run();
    delete app;

    return 0;
}