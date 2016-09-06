# Reference
## OpenGL Basics

*OpenGL uses column-major matrix format*<br/>
0 4 8  12<br/>
1 5 9  13<br/>
2 6 10 14<br/>
3 7 11 15



#### Normalized Device Coordinates (LH):
- X: -1 to 1 (Left to Right)
- Y: -1 to 1 (Bottom to Top)
- Z: -1 to 1 (Near to Far)
- W:  1 (normalized)

#### LH to RH (or vice versa) coordinate system swap:
- Swap Y and Z axis (or)
- Swap 2nd and 3rd rows in a 4x4 matrix, 4th column (0, 0, 0, 1)