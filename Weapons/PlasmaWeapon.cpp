#include <algorithm>
#include "Math\PhysicsOps.h"
#include "Generators\PhysicsGenerator.h"
#include "Generators\RockGenerator.h"
#include "WeaponBase.h"
#include "PlasmaWeapon.h"

PlasmaProgram PlasmaWeapon::program;

glm::vec3 PlasmaWeapon::ballVertices[BallVerticesCount] =
{
    glm::vec3(9.7545161e-2, 3.0027889e-17, 0.49039264),
    glm::vec3(9.0119978e-2, -3.7328917e-2, 0.49039264),
    glm::vec3(6.8974845e-2, -6.8974845e-2, 0.49039264),
    glm::vec3(3.7328917e-2, -9.0119978e-2, 0.49039264),
    glm::vec3(5.9729185e-18, -9.7545161e-2, 0.49039264),
    glm::vec3(-3.7328917e-2, -9.0119978e-2, 0.49039264),
    glm::vec3(-6.8974845e-2, -6.8974845e-2, 0.49039264),
    glm::vec3(-9.0119978e-2, -3.7328917e-2, 0.49039264),
    glm::vec3(-9.7545161e-2, 1.8082052e-17, 0.49039264),
    glm::vec3(-9.0119978e-2, 3.7328917e-2, 0.49039264),
    glm::vec3(-6.8974845e-2, 6.8974845e-2, 0.49039264),
    glm::vec3(-3.7328917e-2, 9.0119978e-2, 0.49039264),
    glm::vec3(-1.7918755e-17, 9.7545161e-2, 0.49039264),
    glm::vec3(3.7328917e-2, 9.0119978e-2, 0.49039264),
    glm::vec3(6.8974845e-2, 6.8974845e-2, 0.49039264),
    glm::vec3(9.0119978e-2, 3.7328917e-2, 0.49039264),
    glm::vec3(0.19134172, 2.8285653e-17, 0.46193977),
    glm::vec3(0.17677670, -7.3223305e-2, 0.46193977),
    glm::vec3(0.13529903, -0.13529903, 0.46193977),
    glm::vec3(7.3223305e-2, -0.17677670, 0.46193977),
    glm::vec3(1.1716301e-17, -0.19134172, 0.46193977),
    glm::vec3(-7.3223305e-2, -0.17677670, 0.46193977),
    glm::vec3(-0.13529903, -0.13529903, 0.46193977),
    glm::vec3(-0.17677670, -7.3223305e-2, 0.46193977),
    glm::vec3(-0.19134172, 4.8530508e-18, 0.46193977),
    glm::vec3(-0.17677670, 7.3223305e-2, 0.46193977),
    glm::vec3(-0.13529903, 0.13529903, 0.46193977),
    glm::vec3(-7.3223305e-2, 0.17677670, 0.46193977),
    glm::vec3(-3.5148903e-17, 0.19134172, 0.46193977),
    glm::vec3(7.3223305e-2, 0.17677670, 0.46193977),
    glm::vec3(0.13529903, 0.13529903, 0.46193977),
    glm::vec3(0.17677670, 7.3223305e-2, 0.46193977),
    glm::vec3(0.27778512, 2.5456415e-17, 0.41573481),
    glm::vec3(0.25663998, -0.10630376, 0.41573481),
    glm::vec3(0.19642374, -0.19642374, 0.41573481),
    glm::vec3(0.10630376, -0.25663998, 0.41573481),
    glm::vec3(1.7009433e-17, -0.27778512, 0.41573481),
    glm::vec3(-0.10630376, -0.25663998, 0.41573481),
    glm::vec3(-0.19642374, -0.19642374, 0.41573481),
    glm::vec3(-0.25663998, -0.10630376, 0.41573481),
    glm::vec3(-0.27778512, -8.5624504e-18, 0.41573481),
    glm::vec3(-0.25663998, 0.10630376, 0.41573481),
    glm::vec3(-0.19642374, 0.19642374, 0.41573481),
    glm::vec3(-0.10630376, 0.25663998, 0.41573481),
    glm::vec3(-5.1028298e-17, 0.27778512, 0.41573481),
    glm::vec3(0.10630376, 0.25663998, 0.41573481),
    glm::vec3(0.19642374, 0.19642374, 0.41573481),
    glm::vec3(0.25663998, 0.10630376, 0.41573481),
    glm::vec3(0.35355339, 2.1648901e-17, 0.35355339),
    glm::vec3(0.32664074, -0.13529903, 0.35355339),
    glm::vec3(0.25000000, -0.25000000, 0.35355339),
    glm::vec3(0.13529903, -0.32664074, 0.35355339),
    glm::vec3(2.1648901e-17, -0.35355339, 0.35355339),
    glm::vec3(-0.13529903, -0.32664074, 0.35355339),
    glm::vec3(-0.25000000, -0.25000000, 0.35355339),
    glm::vec3(-0.32664074, -0.13529903, 0.35355339),
    glm::vec3(-0.35355339, -2.1648901e-17, 0.35355339),
    glm::vec3(-0.32664074, 0.13529903, 0.35355339),
    glm::vec3(-0.25000000, 0.25000000, 0.35355339),
    glm::vec3(-0.13529903, 0.32664074, 0.35355339),
    glm::vec3(-6.4946704e-17, 0.35355339, 0.35355339),
    glm::vec3(0.13529903, 0.32664074, 0.35355339),
    glm::vec3(0.25000000, 0.25000000, 0.35355339),
    glm::vec3(0.32664074, 0.13529903, 0.35355339),
    glm::vec3(0.41573481, 1.7009433e-17, 0.27778512),
    glm::vec3(0.38408888, -0.15909482, 0.27778512),
    glm::vec3(0.29396890, -0.29396890, 0.27778512),
    glm::vec3(0.15909482, -0.38408888, 0.27778512),
    glm::vec3(2.5456415e-17, -0.41573481, 0.27778512),
    glm::vec3(-0.15909482, -0.38408888, 0.27778512),
    glm::vec3(-0.29396890, -0.29396890, 0.27778512),
    glm::vec3(-0.38408888, -0.15909482, 0.27778512),
    glm::vec3(-0.41573481, -3.3903397e-17, 0.27778512),
    glm::vec3(-0.38408888, 0.15909482, 0.27778512),
    glm::vec3(-0.29396890, 0.29396890, 0.27778512),
    glm::vec3(-0.15909482, 0.38408888, 0.27778512),
    glm::vec3(-7.6369245e-17, 0.41573481, 0.27778512),
    glm::vec3(0.15909482, 0.38408888, 0.27778512),
    glm::vec3(0.29396890, 0.29396890, 0.27778512),
    glm::vec3(0.38408888, 0.15909482, 0.27778512),
    glm::vec3(0.46193977, 1.1716301e-17, 0.19134172),
    glm::vec3(0.42677670, -0.17677670, 0.19134172),
    glm::vec3(0.32664074, -0.32664074, 0.19134172),
    glm::vec3(0.17677670, -0.42677670, 0.19134172),
    glm::vec3(2.8285653e-17, -0.46193977, 0.19134172),
    glm::vec3(-0.17677670, -0.42677670, 0.19134172),
    glm::vec3(-0.32664074, -0.32664074, 0.19134172),
    glm::vec3(-0.42677670, -0.17677670, 0.19134172),
    glm::vec3(-0.46193977, -4.4855005e-17, 0.19134172),
    glm::vec3(-0.42677670, 0.17677670, 0.19134172),
    glm::vec3(-0.32664074, 0.32664074, 0.19134172),
    glm::vec3(-0.17677670, 0.42677670, 0.19134172),
    glm::vec3(-8.4856958e-17, 0.46193977, 0.19134172),
    glm::vec3(0.17677670, 0.42677670, 0.19134172),
    glm::vec3(0.32664074, 0.32664074, 0.19134172),
    glm::vec3(0.42677670, 0.17677670, 0.19134172),
    glm::vec3(0.49039264, 5.9729185e-18, 9.7545161e-2),
    glm::vec3(0.45306372, -0.18766514, 9.7545161e-2),
    glm::vec3(0.34675996, -0.34675996, 9.7545161e-2),
    glm::vec3(0.18766514, -0.45306372, 9.7545161e-2),
    glm::vec3(3.0027889e-17, -0.49039264, 9.7545161e-2),
    glm::vec3(-0.18766514, -0.45306372, 9.7545161e-2),
    glm::vec3(-0.34675996, -0.34675996, 9.7545161e-2),
    glm::vec3(-0.45306372, -0.18766514, 9.7545161e-2),
    glm::vec3(-0.49039264, -5.4082859e-17, 9.7545161e-2),
    glm::vec3(-0.45306372, 0.18766514, 9.7545161e-2),
    glm::vec3(-0.34675996, 0.34675996, 9.7545161e-2),
    glm::vec3(-0.18766514, 0.45306372, 9.7545161e-2),
    glm::vec3(-9.0083667e-17, 0.49039264, 9.7545161e-2),
    glm::vec3(0.18766514, 0.45306372, 9.7545161e-2),
    glm::vec3(0.34675996, 0.34675996, 9.7545161e-2),
    glm::vec3(0.45306372, 0.18766514, 9.7545161e-2),
    glm::vec3(0.50000000, 1.8746997e-33, 3.0616170e-17),
    glm::vec3(0.46193977, -0.19134172, 4.2332471e-17),
    glm::vec3(0.35355339, -0.35355339, 5.2265071e-17),
    glm::vec3(0.19134172, -0.46193977, 5.8901823e-17),
    glm::vec3(3.0616170e-17, -0.50000000, 6.1232340e-17),
    glm::vec3(-0.19134172, -0.46193977, 5.8901823e-17),
    glm::vec3(-0.35355339, -0.35355339, 5.2265071e-17),
    glm::vec3(-0.46193977, -0.19134172, 4.2332471e-17),
    glm::vec3(-0.50000000, -6.1232340e-17, 3.0616170e-17),
    glm::vec3(-0.46193977, 0.19134172, 1.8899869e-17),
    glm::vec3(-0.35355339, 0.35355339, 8.9672686e-18),
    glm::vec3(-0.19134172, 0.46193977, 2.3305172e-18),
    glm::vec3(-9.1848510e-17, 0.50000000, 0.0000000e+0),
    glm::vec3(0.19134172, 0.46193977, 2.3305172e-18),
    glm::vec3(0.35355339, 0.35355339, 8.9672686e-18),
    glm::vec3(0.46193977, 0.19134172, 1.8899869e-17),
    glm::vec3(0.49039264, -5.9729185e-18, -9.7545161e-2),
    glm::vec3(0.45306372, -0.18766514, -9.7545161e-2),
    glm::vec3(0.34675996, -0.34675996, -9.7545161e-2),
    glm::vec3(0.18766514, -0.45306372, -9.7545161e-2),
    glm::vec3(3.0027889e-17, -0.49039264, -9.7545161e-2),
    glm::vec3(-0.18766514, -0.45306372, -9.7545161e-2),
    glm::vec3(-0.34675996, -0.34675996, -9.7545161e-2),
    glm::vec3(-0.45306372, -0.18766514, -9.7545161e-2),
    glm::vec3(-0.49039264, -6.6028696e-17, -9.7545161e-2),
    glm::vec3(-0.45306372, 0.18766514, -9.7545161e-2),
    glm::vec3(-0.34675996, 0.34675996, -9.7545161e-2),
    glm::vec3(-0.18766514, 0.45306372, -9.7545161e-2),
    glm::vec3(-9.0083667e-17, 0.49039264, -9.7545161e-2),
    glm::vec3(0.18766514, 0.45306372, -9.7545161e-2),
    glm::vec3(0.34675996, 0.34675996, -9.7545161e-2),
    glm::vec3(0.45306372, 0.18766514, -9.7545161e-2),
    glm::vec3(0.46193977, -1.1716301e-17, -0.19134172),
    glm::vec3(0.42677670, -0.17677670, -0.19134172),
    glm::vec3(0.32664074, -0.32664074, -0.19134172),
    glm::vec3(0.17677670, -0.42677670, -0.19134172),
    glm::vec3(2.8285653e-17, -0.46193977, -0.19134172),
    glm::vec3(-0.17677670, -0.42677670, -0.19134172),
    glm::vec3(-0.32664074, -0.32664074, -0.19134172),
    glm::vec3(-0.42677670, -0.17677670, -0.19134172),
    glm::vec3(-0.46193977, -6.8287607e-17, -0.19134172),
    glm::vec3(-0.42677670, 0.17677670, -0.19134172),
    glm::vec3(-0.32664074, 0.32664074, -0.19134172),
    glm::vec3(-0.17677670, 0.42677670, -0.19134172),
    glm::vec3(-8.4856958e-17, 0.46193977, -0.19134172),
    glm::vec3(0.17677670, 0.42677670, -0.19134172),
    glm::vec3(0.32664074, 0.32664074, -0.19134172),
    glm::vec3(0.42677670, 0.17677670, -0.19134172),
    glm::vec3(0.41573481, -1.7009433e-17, -0.27778512),
    glm::vec3(0.38408888, -0.15909482, -0.27778512),
    glm::vec3(0.29396890, -0.29396890, -0.27778512),
    glm::vec3(0.15909482, -0.38408888, -0.27778512),
    glm::vec3(2.5456415e-17, -0.41573481, -0.27778512),
    glm::vec3(-0.15909482, -0.38408888, -0.27778512),
    glm::vec3(-0.29396890, -0.29396890, -0.27778512),
    glm::vec3(-0.38408888, -0.15909482, -0.27778512),
    glm::vec3(-0.41573481, -6.7922263e-17, -0.27778512),
    glm::vec3(-0.38408888, 0.15909482, -0.27778512),
    glm::vec3(-0.29396890, 0.29396890, -0.27778512),
    glm::vec3(-0.15909482, 0.38408888, -0.27778512),
    glm::vec3(-7.6369245e-17, 0.41573481, -0.27778512),
    glm::vec3(0.15909482, 0.38408888, -0.27778512),
    glm::vec3(0.29396890, 0.29396890, -0.27778512),
    glm::vec3(0.38408888, 0.15909482, -0.27778512),
    glm::vec3(0.35355339, -2.1648901e-17, -0.35355339),
    glm::vec3(0.32664074, -0.13529903, -0.35355339),
    glm::vec3(0.25000000, -0.25000000, -0.35355339),
    glm::vec3(0.13529903, -0.32664074, -0.35355339),
    glm::vec3(2.1648901e-17, -0.35355339, -0.35355339),
    glm::vec3(-0.13529903, -0.32664074, -0.35355339),
    glm::vec3(-0.25000000, -0.25000000, -0.35355339),
    glm::vec3(-0.32664074, -0.13529903, -0.35355339),
    glm::vec3(-0.35355339, -6.4946704e-17, -0.35355339),
    glm::vec3(-0.32664074, 0.13529903, -0.35355339),
    glm::vec3(-0.25000000, 0.25000000, -0.35355339),
    glm::vec3(-0.13529903, 0.32664074, -0.35355339),
    glm::vec3(-6.4946704e-17, 0.35355339, -0.35355339),
    glm::vec3(0.13529903, 0.32664074, -0.35355339),
    glm::vec3(0.25000000, 0.25000000, -0.35355339),
    glm::vec3(0.32664074, 0.13529903, -0.35355339),
    glm::vec3(0.27778512, -2.5456415e-17, -0.41573481),
    glm::vec3(0.25663998, -0.10630376, -0.41573481),
    glm::vec3(0.19642374, -0.19642374, -0.41573481),
    glm::vec3(0.10630376, -0.25663998, -0.41573481),
    glm::vec3(1.7009433e-17, -0.27778512, -0.41573481),
    glm::vec3(-0.10630376, -0.25663998, -0.41573481),
    glm::vec3(-0.19642374, -0.19642374, -0.41573481),
    glm::vec3(-0.25663998, -0.10630376, -0.41573481),
    glm::vec3(-0.27778512, -5.9475280e-17, -0.41573481),
    glm::vec3(-0.25663998, 0.10630376, -0.41573481),
    glm::vec3(-0.19642374, 0.19642374, -0.41573481),
    glm::vec3(-0.10630376, 0.25663998, -0.41573481),
    glm::vec3(-5.1028298e-17, 0.27778512, -0.41573481),
    glm::vec3(0.10630376, 0.25663998, -0.41573481),
    glm::vec3(0.19642374, 0.19642374, -0.41573481),
    glm::vec3(0.25663998, 0.10630376, -0.41573481),
    glm::vec3(0.19134172, -2.8285653e-17, -0.46193977),
    glm::vec3(0.17677670, -7.3223305e-2, -0.46193977),
    glm::vec3(0.13529903, -0.13529903, -0.46193977),
    glm::vec3(7.3223305e-2, -0.17677670, -0.46193977),
    glm::vec3(1.1716301e-17, -0.19134172, -0.46193977),
    glm::vec3(-7.3223305e-2, -0.17677670, -0.46193977),
    glm::vec3(-0.13529903, -0.13529903, -0.46193977),
    glm::vec3(-0.17677670, -7.3223305e-2, -0.46193977),
    glm::vec3(-0.19134172, -5.1718255e-17, -0.46193977),
    glm::vec3(-0.17677670, 7.3223305e-2, -0.46193977),
    glm::vec3(-0.13529903, 0.13529903, -0.46193977),
    glm::vec3(-7.3223305e-2, 0.17677670, -0.46193977),
    glm::vec3(-3.5148903e-17, 0.19134172, -0.46193977),
    glm::vec3(7.3223305e-2, 0.17677670, -0.46193977),
    glm::vec3(0.13529903, 0.13529903, -0.46193977),
    glm::vec3(0.17677670, 7.3223305e-2, -0.46193977),
    glm::vec3(9.7545161e-2, -3.0027889e-17, -0.49039264),
    glm::vec3(9.0119978e-2, -3.7328917e-2, -0.49039264),
    glm::vec3(6.8974845e-2, -6.8974845e-2, -0.49039264),
    glm::vec3(3.7328917e-2, -9.0119978e-2, -0.49039264),
    glm::vec3(5.9729185e-18, -9.7545161e-2, -0.49039264),
    glm::vec3(-3.7328917e-2, -9.0119978e-2, -0.49039264),
    glm::vec3(-6.8974845e-2, -6.8974845e-2, -0.49039264),
    glm::vec3(-9.0119978e-2, -3.7328917e-2, -0.49039264),
    glm::vec3(-9.7545161e-2, -4.1973726e-17, -0.49039264),
    glm::vec3(-9.0119978e-2, 3.7328917e-2, -0.49039264),
    glm::vec3(-6.8974845e-2, 6.8974845e-2, -0.49039264),
    glm::vec3(-3.7328917e-2, 9.0119978e-2, -0.49039264),
    glm::vec3(-1.7918755e-17, 9.7545161e-2, -0.49039264),
    glm::vec3(3.7328917e-2, 9.0119978e-2, -0.49039264),
    glm::vec3(6.8974845e-2, 6.8974845e-2, -0.49039264),
    glm::vec3(9.0119978e-2, 3.7328917e-2, -0.49039264),
    glm::vec3(0.0000000e+0, 3.0616170e-17, 0.50000000),
    glm::vec3(0.0000000e+0, -3.0616170e-17, -0.50000000)
};

unsigned int PlasmaWeapon::ballIndices[BallIndicesCount] =
{
    1, 18, 17,
    1, 32, 16,
    1, 241, 2,
    2, 18, 1,
    2, 241, 3,
    3, 18, 2,
    3, 20, 19,
    3, 241, 4,
    4, 20, 3,
    4, 241, 5,
    5, 20, 4,
    5, 22, 21,
    5, 241, 6,
    6, 22, 5,
    6, 241, 7,
    7, 22, 6,
    7, 24, 23,
    7, 241, 8,
    8, 24, 7,
    8, 241, 9,
    9, 24, 8,
    9, 26, 25,
    9, 241, 10,
    10, 26, 9,
    10, 241, 11,
    11, 26, 10,
    11, 28, 27,
    11, 241, 12,
    12, 28, 11,
    12, 241, 13,
    13, 28, 12,
    13, 30, 29,
    13, 241, 14,
    14, 30, 13,
    14, 241, 15,
    15, 30, 14,
    15, 32, 31,
    15, 241, 16,
    16, 32, 15,
    16, 241, 1,
    17, 32, 1,
    17, 33, 32,
    18, 33, 17,
    18, 35, 34,
    19, 18, 3,
    19, 35, 18,
    20, 35, 19,
    20, 37, 36,
    21, 20, 5,
    21, 37, 20,
    22, 37, 21,
    22, 39, 38,
    23, 22, 7,
    23, 39, 22,
    24, 39, 23,
    24, 41, 40,
    25, 24, 9,
    25, 41, 24,
    26, 41, 25,
    26, 43, 42,
    27, 26, 11,
    27, 43, 26,
    28, 43, 27,
    28, 45, 44,
    29, 28, 13,
    29, 45, 28,
    30, 45, 29,
    30, 47, 46,
    31, 30, 15,
    31, 47, 30,
    32, 33, 48,
    32, 47, 31,
    33, 50, 49,
    33, 64, 48,
    34, 33, 18,
    34, 50, 33,
    35, 50, 34,
    35, 52, 51,
    36, 35, 20,
    36, 52, 35,
    37, 52, 36,
    37, 54, 53,
    38, 37, 22,
    38, 54, 37,
    39, 54, 38,
    39, 56, 55,
    40, 39, 24,
    40, 56, 39,
    41, 56, 40,
    41, 58, 57,
    42, 41, 26,
    42, 58, 41,
    43, 58, 42,
    43, 60, 59,
    44, 43, 28,
    44, 60, 43,
    45, 60, 44,
    45, 62, 61,
    46, 45, 30,
    46, 62, 45,
    47, 62, 46,
    47, 64, 63,
    48, 47, 32,
    48, 64, 47,
    49, 64, 33,
    49, 65, 64,
    50, 65, 49,
    50, 67, 66,
    51, 50, 35,
    51, 67, 50,
    52, 67, 51,
    52, 69, 68,
    53, 52, 37,
    53, 69, 52,
    54, 69, 53,
    54, 71, 70,
    55, 54, 39,
    55, 71, 54,
    56, 71, 55,
    56, 73, 72,
    57, 56, 41,
    57, 73, 56,
    58, 73, 57,
    58, 75, 74,
    59, 58, 43,
    59, 75, 58,
    60, 75, 59,
    60, 77, 76,
    61, 60, 45,
    61, 77, 60,
    62, 77, 61,
    62, 79, 78,
    63, 62, 47,
    63, 79, 62,
    64, 65, 80,
    64, 79, 63,
    65, 82, 81,
    65, 96, 80,
    66, 65, 50,
    66, 82, 65,
    67, 82, 66,
    67, 84, 83,
    68, 67, 52,
    68, 84, 67,
    69, 84, 68,
    69, 86, 85,
    70, 69, 54,
    70, 86, 69,
    71, 86, 70,
    71, 88, 87,
    72, 71, 56,
    72, 88, 71,
    73, 88, 72,
    73, 90, 89,
    74, 73, 58,
    74, 90, 73,
    75, 90, 74,
    75, 92, 91,
    76, 75, 60,
    76, 92, 75,
    77, 92, 76,
    77, 94, 93,
    78, 77, 62,
    78, 94, 77,
    79, 94, 78,
    79, 96, 95,
    80, 79, 64,
    80, 96, 79,
    81, 96, 65,
    81, 97, 96,
    82, 97, 81,
    82, 99, 98,
    83, 82, 67,
    83, 99, 82,
    84, 99, 83,
    84, 101, 100,
    85, 84, 69,
    85, 101, 84,
    86, 101, 85,
    86, 103, 102,
    87, 86, 71,
    87, 103, 86,
    88, 103, 87,
    88, 105, 104,
    89, 88, 73,
    89, 105, 88,
    90, 105, 89,
    90, 107, 106,
    91, 90, 75,
    91, 107, 90,
    92, 107, 91,
    92, 109, 108,
    93, 92, 77,
    93, 109, 92,
    94, 109, 93,
    94, 111, 110,
    95, 94, 79,
    95, 111, 94,
    96, 97, 112,
    96, 111, 95,
    97, 114, 113,
    97, 128, 112,
    98, 97, 82,
    98, 114, 97,
    99, 114, 98,
    99, 116, 115,
    100, 99, 84,
    100, 116, 99,
    101, 116, 100,
    101, 118, 117,
    102, 101, 86,
    102, 118, 101,
    103, 118, 102,
    103, 120, 119,
    104, 103, 88,
    104, 120, 103,
    105, 120, 104,
    105, 122, 121,
    106, 105, 90,
    106, 122, 105,
    107, 122, 106,
    107, 124, 123,
    108, 107, 92,
    108, 124, 107,
    109, 124, 108,
    109, 126, 125,
    110, 109, 94,
    110, 126, 109,
    111, 126, 110,
    111, 128, 127,
    112, 111, 96,
    112, 128, 111,
    113, 128, 97,
    113, 129, 128,
    114, 129, 113,
    114, 131, 130,
    115, 114, 99,
    115, 131, 114,
    116, 131, 115,
    116, 133, 132,
    117, 116, 101,
    117, 133, 116,
    118, 133, 117,
    118, 135, 134,
    119, 118, 103,
    119, 135, 118,
    120, 135, 119,
    120, 137, 136,
    121, 120, 105,
    121, 137, 120,
    122, 137, 121,
    122, 139, 138,
    123, 122, 107,
    123, 139, 122,
    124, 139, 123,
    124, 141, 140,
    125, 124, 109,
    125, 141, 124,
    126, 141, 125,
    126, 143, 142,
    127, 126, 111,
    127, 143, 126,
    128, 129, 144,
    128, 143, 127,
    129, 146, 145,
    129, 160, 144,
    130, 129, 114,
    130, 146, 129,
    131, 146, 130,
    131, 148, 147,
    132, 131, 116,
    132, 148, 131,
    133, 148, 132,
    133, 150, 149,
    134, 133, 118,
    134, 150, 133,
    135, 150, 134,
    135, 152, 151,
    136, 135, 120,
    136, 152, 135,
    137, 152, 136,
    137, 154, 153,
    138, 137, 122,
    138, 154, 137,
    139, 154, 138,
    139, 156, 155,
    140, 139, 124,
    140, 156, 139,
    141, 156, 140,
    141, 158, 157,
    142, 141, 126,
    142, 158, 141,
    143, 158, 142,
    143, 160, 159,
    144, 143, 128,
    144, 160, 143,
    145, 160, 129,
    145, 161, 160,
    146, 161, 145,
    146, 163, 162,
    147, 146, 131,
    147, 163, 146,
    148, 163, 147,
    148, 165, 164,
    149, 148, 133,
    149, 165, 148,
    150, 165, 149,
    150, 167, 166,
    151, 150, 135,
    151, 167, 150,
    152, 167, 151,
    152, 169, 168,
    153, 152, 137,
    153, 169, 152,
    154, 169, 153,
    154, 171, 170,
    155, 154, 139,
    155, 171, 154,
    156, 171, 155,
    156, 173, 172,
    157, 156, 141,
    157, 173, 156,
    158, 173, 157,
    158, 175, 174,
    159, 158, 143,
    159, 175, 158,
    160, 161, 176,
    160, 175, 159,
    161, 178, 177,
    161, 192, 176,
    162, 161, 146,
    162, 178, 161,
    163, 178, 162,
    163, 180, 179,
    164, 163, 148,
    164, 180, 163,
    165, 180, 164,
    165, 182, 181,
    166, 165, 150,
    166, 182, 165,
    167, 182, 166,
    167, 184, 183,
    168, 167, 152,
    168, 184, 167,
    169, 184, 168,
    169, 186, 185,
    170, 169, 154,
    170, 186, 169,
    171, 186, 170,
    171, 188, 187,
    172, 171, 156,
    172, 188, 171,
    173, 188, 172,
    173, 190, 189,
    174, 173, 158,
    174, 190, 173,
    175, 190, 174,
    175, 192, 191,
    176, 175, 160,
    176, 192, 175,
    177, 192, 161,
    177, 193, 192,
    178, 193, 177,
    178, 195, 194,
    179, 178, 163,
    179, 195, 178,
    180, 195, 179,
    180, 197, 196,
    181, 180, 165,
    181, 197, 180,
    182, 197, 181,
    182, 199, 198,
    183, 182, 167,
    183, 199, 182,
    184, 199, 183,
    184, 201, 200,
    185, 184, 169,
    185, 201, 184,
    186, 201, 185,
    186, 203, 202,
    187, 186, 171,
    187, 203, 186,
    188, 203, 187,
    188, 205, 204,
    189, 188, 173,
    189, 205, 188,
    190, 205, 189,
    190, 207, 206,
    191, 190, 175,
    191, 207, 190,
    192, 193, 208,
    192, 207, 191,
    193, 210, 209,
    193, 224, 208,
    194, 193, 178,
    194, 210, 193,
    195, 210, 194,
    195, 212, 211,
    196, 195, 180,
    196, 212, 195,
    197, 212, 196,
    197, 214, 213,
    198, 197, 182,
    198, 214, 197,
    199, 214, 198,
    199, 216, 215,
    200, 199, 184,
    200, 216, 199,
    201, 216, 200,
    201, 218, 217,
    202, 201, 186,
    202, 218, 201,
    203, 218, 202,
    203, 220, 219,
    204, 203, 188,
    204, 220, 203,
    205, 220, 204,
    205, 222, 221,
    206, 205, 190,
    206, 222, 205,
    207, 222, 206,
    207, 224, 223,
    208, 207, 192,
    208, 224, 207,
    209, 224, 193,
    209, 225, 224,
    210, 225, 209,
    210, 227, 226,
    211, 210, 195,
    211, 227, 210,
    212, 227, 211,
    212, 229, 228,
    213, 212, 197,
    213, 229, 212,
    214, 229, 213,
    214, 231, 230,
    215, 214, 199,
    215, 231, 214,
    216, 231, 215,
    216, 233, 232,
    217, 216, 201,
    217, 233, 216,
    218, 233, 217,
    218, 235, 234,
    219, 218, 203,
    219, 235, 218,
    220, 235, 219,
    220, 237, 236,
    221, 220, 205,
    221, 237, 220,
    222, 237, 221,
    222, 239, 238,
    223, 222, 207,
    223, 239, 222,
    224, 225, 240,
    224, 239, 223,
    225, 242, 240,
    226, 225, 210,
    226, 242, 225,
    227, 242, 226,
    228, 227, 212,
    228, 242, 227,
    229, 242, 228,
    230, 229, 214,
    230, 242, 229,
    231, 242, 230,
    232, 231, 216,
    232, 242, 231,
    233, 242, 232,
    234, 233, 218,
    234, 242, 233,
    235, 242, 234,
    236, 235, 220,
    236, 242, 235,
    237, 242, 236,
    238, 237, 222,
    238, 242, 237,
    239, 242, 238,
    240, 239, 224,
    240, 242, 239
};

PlasmaWeapon::PlasmaWeapon(Physics* physics)
    : WeaponBase(physics, "Plasma Fyre", 2000000.0f, false, 0.3f, 0.0f),
      maxDistance(50.0f), maxProjectiles(100) // TODO configurable
{
}

float PlasmaWeapon::GetRequiredAmmoToFire()
{
    // TODO configurable.
    return 10.0f;
}

bool PlasmaWeapon::LoadGraphics(ShaderFactory* shaderManager)
{
    // Sky program.
    if (!shaderManager->CreateShaderProgram("plasmaRender", &program.programId))
    {
        Logger::Log("Failure creating the plasma shader program!");
        return false;
    }

    program.projMatrixLocation = glGetUniformLocation(program.programId, "projMatrix");
    program.positionLocation = glGetUniformLocation(program.programId, "position");
    program.frameTimeLocation = glGetUniformLocation(program.programId, "frameTime");

    for (unsigned int i = 0; i < BallIndicesCount; i++)
    {
        ballIndices[i]--;
    }
    
    // We need the VAO to actually render without sending any vertex data to the shader, which handles the stars.
    glGenVertexArrays(1, &program.vao);
    glBindVertexArray(program.vao);
    glGenBuffers(1, &program.positionBuffer);
    glGenBuffers(1, &program.indexBuffer);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, program.positionBuffer);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBufferData(GL_ARRAY_BUFFER, BallVerticesCount * sizeof(glm::vec3), &ballVertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, program.indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, BallIndicesCount * sizeof(unsigned int), &ballIndices[0], GL_STATIC_DRAW);
    
    return true;
}

void PlasmaWeapon::FireInternal(glm::vec3 fireOrigin, glm::vec3 fireDirection)
{
    PlasmaProjectileData* projectile = new PlasmaProjectileData();
    projectile->body = PhysicsGenerator::GetDynamicBody(PhysicsGenerator::CShape::WEAPON_PLASMA, PhysicsOps::Convert(fireOrigin), 1.0f);
    projectile->flightTime = 0.0f;

    glm::vec3 vel = 10.0f * fireDirection;
    projectile->body->setLinearVelocity(btVector3(vel.x, vel.y, vel.z));
    projectile->body->setUserPointer(new TypedCallback<UserPhysics::ObjectType>(UserPhysics::ObjectType::PLASMA_BALL, this));
    // projectile->body->setCcdMotionThreshold(0.3f);
    // projectile->body->setCcdSweptSphereRadius(0.2f);

    // Save the rock to the known projectiles.
    while (projectiles.size() >= (unsigned int)maxProjectiles)
    {
        PlasmaProjectileData* projectileToRemove = (PlasmaProjectileData*)projectiles.front();

        physics->RemoveBody(projectileToRemove->body);
        physics->DeleteBody(projectileToRemove->body, false);
        projectiles.pop_front();
    }

    physics->AddBody(projectile->body);
    projectiles.push_back(projectile);
}

void PlasmaWeapon::Update(float elapsedTime)
{
    WeaponBase::Update(elapsedTime);
    for (auto iter = projectiles.cbegin(); iter != projectiles.cend(); iter++)
    {
        PlasmaProjectileData* projectile = (PlasmaProjectileData*)(*iter);
        projectile->flightTime += elapsedTime;
    }
}

void PlasmaWeapon::Render(const glm::mat4& projectionMatrix)
{
    // Render all the moving projectiles.
    for (auto iter = projectiles.cbegin(); iter != projectiles.cend(); iter++)
    {
        PlasmaProjectileData* projectile = (PlasmaProjectileData*)(*iter);
        glUseProgram(program.programId);
        glBindVertexArray(program.vao);

        glm::vec3 pos = PhysicsGenerator::GetBodyPosition(projectile->body);
        glUniform3f(program.positionLocation, pos.x, pos.y, pos.z);
        glUniform1f(program.frameTimeLocation, projectile->flightTime);
        glUniformMatrix4fv(program.projMatrixLocation, 1, GL_FALSE, &projectionMatrix[0][0]);

        // 36 == cube.
        glDrawElements(GL_TRIANGLES, BallIndicesCount, GL_UNSIGNED_INT, 0);
    }
}

void PlasmaWeapon::Callback(UserPhysics::ObjectType callingObject, void* callbackSpecificData)
{
    // TODO have the plasma ball explode when it hits stuff.
}

void PlasmaWeapon::UnloadGraphics()
{
    glDeleteBuffers(1, &program.indexBuffer);
    glDeleteBuffers(1, &program.positionBuffer);
    glDeleteVertexArrays(1, &program.vao);
    glDeleteProgram(program.programId);
}
