#ifndef SHAPES_HPP
#define SHAPES_HPP

const std::vector<float> square_vertices = {
    // positions         // normals          // texture coords
     0.5f,  0.5f, 0.0f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,  // top right
     0.5f, -0.5f, 0.0f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f,  // bottom right
    -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,  // bottom left
    -0.5f,  0.5f, 0.0f,  0.0f, 0.0f, -1.0f,  0.0f, 1.0f,  // top left
};

const std::vector<unsigned int> square_indices = {
    0, 1, 3,  // right triangle
    1, 2, 3,  // left triangle
};

// std::vector<float> scale_square_texture_coords(float scale_factor)
// {
//     std::size_t texture_coords_offset = 6;
//     std::size_t bytes_per_vertex = 8;
//     std::size_t num_texture_coords = 2;
//
//     std::vector<float> scaled_vertices = square_vertices;
//
//     for (std::size_t i = texture_coords_offset; i < scaled_vertices.size(); i += bytes_per_vertex)
//     {
//         for (std::size_t j = i; j < i + num_texture_coords; j++)
//             scaled_vertices[j] *= scale_factor;
//     }
//
//     return scaled_vertices;
// }
//
// /*
//  * Creates a rectangle shorter along one axis, according to the x/y ratio. A
//  * value greater than 1.0 results in a rectangle shorter along the y-axis, while
//  * a value less than 1.0 results in a rectangle shorter along the x-axis.
//  */
// std::vector<float> generate_rectangle_vertices(float xy_ratio)
// {
//     std::size_t position_coords_offset = 0;
//     std::size_t bytes_per_vertex = 8;
//
//     // If greater than 1.0, shrink along the y-axis.
//     if (xy_ratio > 1.0f)
//         position_coords_offset = 1;
//
//     std::vector<float> rectangle_vertices = square_vertices;
//
//     for (std::size_t i = position_coords_offset; i < rectangle_vertices.size(); i += bytes_per_vertex)
//     {
//         rectangle_vertices[i] /= xy_ratio;
//     }
//
//     return rectangle_vertices;
// }

const std::vector<float> cube_vertices = {
    // positions          // normals           // texture coords
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
};

#endif /* SHAPES_HPP */
