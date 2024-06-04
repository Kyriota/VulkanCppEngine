#include "app/fluid_sim/2d/app.hpp"
#include "app/renderer/app.hpp"
#include "lve/util/file_io.hpp"

// std
#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main()
{
    try
    {
        FluidSim2DApp app{};
        // RendererApp app{};

        app.run();
    }
    catch (const std::exception &e)
    {
        lve::io::writeFile("error.log", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}