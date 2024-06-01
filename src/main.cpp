#include "app/fluid_sim/2d/app.hpp"
#include "app/renderer/renderer.hpp"

// std
#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main()
{
    lve::RendererApp app{};

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

// test yaml cpp

// #include "include/yaml.hpp"

// #include <cstdlib>
// #include <iostream>
// #include <stdexcept>

// int main()
// {
//     // read config.yaml and print it
//     YAML::Node config = YAML::LoadFile("config.yaml");
//     /*
//     name: "Barack Obama"
//     children:
//     - Sasha
//     - Malia
//     */
//     std::cout << "name: " << config["name"].as<std::string>() << std::endl;
//     std::cout << "children: " << std::endl;
//     for (const auto &child : config["children"])
//     {
//         std::cout << "- " << child.as<std::string>() << std::endl;
//     }
// }