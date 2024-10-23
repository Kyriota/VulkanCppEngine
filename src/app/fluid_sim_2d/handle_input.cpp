#include "app.hpp"

// std
#include <iostream>

namespace app::fluidsim
{
void App::handleInput()
{
    // if (lveWindow.input.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT) ||
    //     lveWindow.input.isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT))
    // {
    //     double mouseX, mouseY;
    //     lveWindow.input.getMousePosition(mouseX, mouseY);
    //     glm::vec2 mousePos = {static_cast<float>(mouseX), static_cast<float>(mouseY)};
    //     fluidParticleSys.setRangeForcePos(
    //         lveWindow.input.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT), mousePos);
    // }

    // lveWindow.input.oneTimeKeyUse(GLFW_KEY_R, [this] {
    //     fluidParticleSys.reloadConfigParam();
    //     std::cout << "Reloaded config parameters" << std::endl;
    // });
    // lveWindow.input.oneTimeKeyUse(GLFW_KEY_SPACE, [this] {
    //     fluidParticleSys.togglePause();
    // });
    // lveWindow.input.oneTimeKeyUse(GLFW_KEY_F, [this] {
    //     fluidParticleSys.renderPausedNextFrame();
    // });

    // lveWindow.input.oneTimeKeyUse(GLFW_KEY_V, [this] {
    //     fluidParticleSys.toggleDebugLine();
    // });
    // lveWindow.input.oneTimeKeyUse(GLFW_KEY_1, [this] {
    //     fluidParticleSys.setDebugLineType(SPH::VELOCITY);
    // });
    // lveWindow.input.oneTimeKeyUse(GLFW_KEY_2, [this] {
    //     fluidParticleSys.setDebugLineType(SPH::PRESSURE_FORCE);
    // });
    // lveWindow.input.oneTimeKeyUse(GLFW_KEY_3, [this] {
    //     fluidParticleSys.setDebugLineType(SPH::EXTERNAL_FORCE);
    // });

    // lveWindow.input.oneTimeKeyUse(GLFW_KEY_N, [this] {
    //     fluidParticleSys.toggleNeighborView();
    // });
}
} // namespace app::fluidsim