# Procedural Terrain Generator

## Overview

This program allows the user to generate procedurally terrain and run an algorithm to simulate effects such as erosion and deposition. The initial terrain is generated using Perlin noise. The erosion algorithm used is a modified version of the algorithm described here: https://inria.hal.science/inria-00402079/document. The user can export the results as BMP images and MDL model files.

<p align="center">
  <img src="images/program.png" alt="program" width="400">
</p>

## Examples

The following table shows how the terrain can differ based on the settings. The first row shows the initial terrain before running the erosion algorithm. The second and third rows show the results of running the erosion algorithm with different settings.

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

### Terrain

### Simulation

### View

## Controls

| Keys | Action |
| :---: | :---: |
| S | Zoom out |
| W | Zoom in |
| A | Move left |
| D | Move right |
| Left Shift | Move down |
| Space | Move up |
| Escape | Hide GUI |
| Enter | Reset position |
