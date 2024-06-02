#include "app/fluid_sim/2d/app.hpp"
#include "app/renderer/renderer.hpp"

// std
#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main()
{
    lve::FluidSim2DApp app{};

    try
    {
        app.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}