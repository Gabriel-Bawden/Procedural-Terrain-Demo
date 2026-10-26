# Procedural Terrain Generator

## Overview

This program allows the user to generate procedurally terrain and run an algorithm to simulate effects such as erosion and deposition. The initial terrain is generated using several layers of Perlin noise. The erosion algorithm used is a modified version of the algorithm described here: https://inria.hal.science/inria-00402079/document. The user can export the results as BMP images and MDL model files.

<p align="center">
  <img src="images/program.png" alt="program" width="400">
</p>

## Examples

The following table shows how the terrain can differ based on the settings. The first row shows the initial terrain before running the erosion algorithm. The second and third rows show the results of running the erosion algorithm with different settings:

<table>
  <tr>
    <td align="center">Settings</td>
    <td align="center">Heightmap</td>
    <td align="center">Net Change</td>
    <td align="center">3D Model</td>
  </tr>
  <tr>
    <td align="center">
      <img src="images/settings_no_erosion.png" alt="settings_no_erosion" width="250"><br>
    </td>
    <td align="center">
      <img src="images/heightmap_color_no_erosion.png" alt="heightmap_color_no_erosion" width="250"><br>
    </td>
    <td align="center">
      <img src="images/heightmap_net_change_no_erosion.png" alt="heightmap_net_change_no_erosion" width="250"><br>
    </td>
    <td align="center">
      <img src="images/model_no_erosion.png" alt="model_no_erosion" width="250"><br>
    </td>
  </tr>
  <tr>
    <td align="center">
      <img src="images/settings_erosion_1.png" alt="settings_erosion_1" width="250"><br>
    </td>
    <td align="center">
      <img src="images/heightmap_color_erosion_1.png" alt="heightmap_color_erosion_1" width="250"><br>
    </td>
    <td align="center">
      <img src="images/heightmap_net_change_erosion_1.png" alt="heightmap_net_change_erosion_1" width="250"><br>
    </td>
    <td align="center">
      <img src="images/model_erosion_1.png" alt="model_erosion_1" width="250"><br>
    </td>
  </tr>
  <tr>
    <td align="center">
      <img src="images/settings_erosion_2.png" alt="settings_erosion_2" width="250"><br>
    </td>
    <td align="center">
      <img src="images/heightmap_color_erosion_2.png" alt="heightmap_color_erosion_2" width="250"><br>
    </td>
    <td align="center">
      <img src="images/heightmap_net_change_erosion_2.png" alt="heightmap_net_change_erosion_2" width="250"><br>
    </td>
    <td align="center">
      <img src="images/model_erosion_2.png" alt="model_erosion_2" width="250"><br>
    </td>
  </tr>
</table>

## Interface

The interface consists of three tabs, Terrain, Simulation, and View, which enable the user to alter settings about the terrain or choose what information is visible.  

### Terrain

<p align="center">
  <img src="images/settings_no_erosion.png" alt="settings_no_erosion" width="250">
</p>

The Terrain tab contains options for the terrain's initial generation as well as an option to save and export the results. The user can input a specific seed or request a random seed. The scale slider alters the terrain's zoom level. A higher scale results in terrain that appears more zoomed out and lower in resolution. Once satisfied with the options, the user can click generate to create the initial terrain. If the user clicks save, then the current state of the terrain is exported as a BMP image and and MDL file. The MDL file can be converted to an OBJ file using the provided `model_converter.py` script.

### Simulation

<p align="center">
  <img src="images/settings_erosion_1.png" alt="settings_erosion_1" width="250">
</p>

The Simulation tab contains options for the erosion simulation. The simulation checkbox at the top toggles the erosion algorithm. Once checked, the program will slow down significantly as it is single-threaded. The six sliders have the following functions:

- **Flow**: How fast the water flows
- **Capacity**: How much sediment the water can carry
- **Dissolution**: The rate at which the water dissolves sediment from the terrain
- **Deposition**: The rate at which the water deposits sediment to the terrain
- **Evaporation**: The rate at which the water evaporates
- **Precipitation**: The rate at which rain falls, increasing the amount of water

### View

<p align="center">
  <img src="images/menu_view.png" alt="menu_view" width="250">
</p>

The View tab enables the user to choose what information is displayed. The seven views convey the following information:

- **Color**: The terrain's heightmap in color
- **Grayscale**: The terrain's heightmap in grayscale
- **Water**: The amount of water present at each tile
- **Velocity**: The velocity of the water at each tile
- **Sediment**: The amount of sediment at each tile
- **Net Change**: The net change in the terrain's heightmap over the course of the simulation, with red for negative and green for positive
- **Watershed**: Calculates the watersheds for the terrain. Generated only on save

The following table shows examples of the different views:

<table>
  <tr>
    <td align="center">Color</td>
    <td align="center">Grayscale</td>
    <td align="center">Water</td>
  </tr>
  <tr>
    <td align="center">
      <img src="images/heightmap_color_erosion_2.png" alt="heightmap_color_erosion_2" width="250"><br>
    </td>
    <td align="center">
      <img src="images/heightmap_grayscale_erosion_2.png" alt="heightmap_grayscale_erosion_2" width="250"><br>
    </td>
    <td align="center">
      <img src="images/water_erosion_2.png" alt="water_erosion_2" width="250"><br>
    </td>
  </tr>
    <tr>
    <td align="center">Velocity</td>
    <td align="center">Sediment</td>
    <td align="center">Net Change</td>
  </tr>
  <tr>
    <td align="center">
      <img src="images/velocity_erosion_2.png" alt="velocity_erosion_2" width="250"><br>
    </td>
    <td align="center">
      <img src="images/sediment_erosion_2.png" alt="sediment_erosion_2" width="250"><br>
    </td>
    <td align="center">
      <img src="images/heightmap_net_change_erosion_2.png" alt="heightmap_net_change_erosion_2" width="250"><br>
    </td>
  </tr>
</table>

## Controls

| Key | Action |
| :---: | :---: |
| S | Zoom out |
| W | Zoom in |
| A | Move left |
| D | Move right |
| Left Shift | Move down |
| Space | Move up |
| Escape | Toggle GUI |
| Enter | Reset position |
