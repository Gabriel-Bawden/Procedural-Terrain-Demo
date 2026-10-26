# Procedural Terrain Generator

## Overview

This program allows the user to generate procedurally terrain and run an algorithm to simulate effects such as erosion and deposition. The initial terrain is generated using Perlin noise. The erosion algorithm used is a modified version of the algorithm described here: https://inria.hal.science/inria-00402079/document. The user can export the results as BMP images and MDL model files.

<p align="center">
  <img src="https://github.com/user-attachments/assets/bba50022-17da-4b1b-a104-d8fcac833f51" alt="program" width="400">
</p>

## Examples

The following table shows how the terrain can differ based on the settings. The first row shows the initial terrain before running the erosion algorithm. The second and third rows show the results of running the erosion algorithm with different settings.

<table>
  <tr>
    <td align="center">
      <img src="https://github.com/user-attachments/assets/c4089437-54ca-43f5-a5c6-29a8e8440800" alt="settings_no_erosion" width="250"><br>
      Settings
    </td>
    <td align="center">
      <img src="https://github.com/user-attachments/assets/dda1bbed-cd3a-44d6-8abe-78eb302a9ae4" alt="heightmap_color_no_erosion" width="250"><br>
      Heightmap
    </td>
    <td align="center">
      <img src="https://github.com/user-attachments/assets/28a99dbb-bb5a-4b66-b55e-46c11ee6419f" alt="heightmap_net_change_no_erosion" width="250"><br>
      Net Change
    </td>
    <td align="center">
      <img src="https://github.com/user-attachments/assets/7f779e87-1b5c-40cd-8e44-b986f4cea0aa" alt="model_no_erosion" width="250"><br>
      3D Model
    </td>
  </tr>
  <tr>
    <td align="center">
      <img src="https://github.com/user-attachments/assets/bcfd1402-a72a-488b-ac1e-ce9653e4efd3" alt="settings_erosion_1" width="250"><br>
      Settings
    </td>
    <td align="center">
      <img src="https://github.com/user-attachments/assets/9456f204-583a-45be-8f52-c8b94e1c8b30" alt="heightmap_color_erosion_1" width="250"><br>
      Heightmap
    </td>
    <td align="center">
      <img src="https://github.com/user-attachments/assets/f89e9f7b-77a9-4e89-8aaf-716548f3befb" alt="heightmap_net_change_erosion_1" width="250"><br>
      Net Change
    </td>
    <td align="center">
      <img src="https://github.com/user-attachments/assets/e5709c08-bd14-4c6a-a24a-5931e4df4d1e" alt="model_erosion_1" width="250"><br>
      3D Model
    </td>
  </tr>
  <tr>
    <td align="center">
      <img src="https://github.com/user-attachments/assets/41103d93-2d23-4c35-a097-f3beb60fae2f" alt="settings_erosion_2" width="250"><br>
      Settings
    </td>
    <td align="center">
      <img src="https://github.com/user-attachments/assets/00afebf5-2c53-41c5-b13c-65eecf0ed9cf" alt="heightmap_color_erosion_2" width="250"><br>
      Heightmap
    </td>
    <td align="center">
      <img src="https://github.com/user-attachments/assets/ce77096e-dd4a-4b53-a8b0-b6704277c4a7" alt="heightmap_net_change_erosion_2" width="250"><br>
      Net Change
    </td>
    <td align="center">
      <img src="https://github.com/user-attachments/assets/de9d542e-a772-4997-a0ec-960d3071a82a" alt="model_erosion_2" width="250"><br>
      3D Model
    </td>
  </tr>
</table>

## Interface

### Terrain

### Simulation

### View

## Controls

| Keys | Action |
| - | - |
| S | Zoom out |
| W | Zoom in |
| A | Move left |
| D | Move right |
| Left Shift | Move down |
| Space | Move up |
| Escape | Hide GUI |
| Enter | Reset position |
