//
// Created by allan on 08/05/2025.
//

#include "../include/mantis/app/app.h"

int main(const int argc, char* argv[])
{
    mantis::MantisApp app(argc, argv);
    app.init();
    return app.run();
}
