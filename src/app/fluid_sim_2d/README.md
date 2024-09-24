# Fluid Simulation

Fluid simulation app WIP, calculation all on CPU for now, will use compute shader soon

Reference: [Sebastian Lague Fluid Sim in Unity](https://www.youtube.com/watch?v=rSKMYc1CQHE)

Source: [Github Repo](https://github.com/Kyriota/VulkanCppEngine)

## Key mappings

Controls:

-   `Space`: Pause/Resume simulation
-   `F`: Render next frame (only works when paused)
-   `Mouse Left Click`: Add repulsive external force
-   `Mouse Right Click`: Add attractive external force
-   `R`: Reload configuration (excluding particle count setting, only restarting the app will apply new particle count)

Visualizations:

-   `D`: Toggle density visualization
-   `N`: Toggle neighbor visualization
-   `V`: Toggle vector debug visualization
    -   `1`: Switch to velocity
    -   `2`: Switch to pressure force
    -   `3`: Switch to external force

## Change Config

You can change configuration of this app by editing `config/fluidSim2D.yaml`
