# Cloth-Simulation

Cloth-simulation in C++ using Verlet Integration and visualized using SFML library. The cloth starts at the top of the screen and cascades down with the force of gravity (with the top row staying pinned to the top of the screen). Every little while a random point on the cloth will be moved to demonstrate the movement physics of the cloth.

## Video
Link to video: https://www.loom.com/share/7f241f054fd94396a116429529139ad0?sid=bc3677f6-a7df-4c74-9cc3-cb32e7142330
(sorry for the lag my CPU was going through it)
The video shows me running the ./app-parallelized, it's the same code but I (attempted to) use openMP to parallelize the point position calculation.

## Use
1. Ensure SFML is properly installed to view the simulation (the pop up window).
2. run ```make``` within the same directory that main.cpp is located.
3. run using ```./app```.

## Reference Link
Used this link for reference on using Verlet Integration for the Cloth Simulation: https://pikuma.com/blog/verlet-integration-2d-cloth-physics-simulation
