#include <Application.h>

int main(int argc, char* argv[]) {
    Application app;
    app.run(argc, argv);
    app.waitForExit();
    return 0;
}