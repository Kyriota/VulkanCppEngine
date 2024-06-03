#include "app/fluid_sim/2d/app.hpp"
#include "app/renderer/app.hpp"

// std
#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main()
{
    // lve::FluidSim2DApp app{};
    lve::RendererApp app{};

    try
    {
        app.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        // pause to see error message
        std::cin.get();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}