#include "app/fluid_sim_2d/app.hpp"
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
        fluidsim2d::App app{};
        // RendererApp app{};

        app.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        lve::io::writeFile("error.log", e.what());
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "Unknown exception" << std::endl;
        lve::io::writeFile("error.log", "Unknown exception");
        return EXIT_FAILURE;
    }
}