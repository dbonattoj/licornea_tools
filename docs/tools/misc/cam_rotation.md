# misc/cam\_rotation

Convert camera rotation from Euler angles to rotation matrix, and reverse.

## Usage

    misc/cam_rotation to out_rotation_matrix.json x y z
    
In the first form, writes a 3x3 rotation matrix to `out_rotation_matrix.json`, containing the rotation given by the angles in `x`, `y`, `z`, given in degrees.

The rotation matrix is `R = transpose(R_z * R_y * R_x)`: The camera first does a _pitch_ rotation around its X-axis by `x` degrees. Then is does a _yaw_ rotation around its new Y-axis, by `y` degrees. Finally it does a _roll_ rotation around its viewing axis, by `z` degrees.

The transpose is taken, because the rotation matrix `R` is the rotation from world space to camera space.

---

    misc/cam_rotation from rotation_matrix.json

In the second form, reads the rotation matrix in `rotation_matrix.json`, and prints the corresponding `x`, `y`, `z` angles.