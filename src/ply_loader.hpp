#ifndef SURFACE_SPLATTING_PLY_LOADER_HPP
#define SURFACE_SPLATTING_PLY_LOADER_HPP

#include <array>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include <boost/range/combine.hpp>
#include <happly.h>


template<typename VectorType>
void load_ply(
        const std::string &path,
        std::vector<VectorType>& vertices,
        std::vector<VectorType>& normals,
        std::vector<std::array<unsigned int, 3>>& faces,
        std::vector<std::array<unsigned int, 3>>& colors) {
  std::cout << "Opening PLY file: " << std::filesystem::absolute(std::filesystem::path(path)) << std::endl;
  happly::PLYData ply(path);
  auto input_vertices = ply.getVertexPositions();
  std::vector<float> input_normals_x, input_normals_y, input_normals_z;
  try {
    input_normals_x = ply.getElement("vertex").getProperty<float>("nx");
    input_normals_y = ply.getElement("vertex").getProperty<float>("ny");
    input_normals_z = ply.getElement("vertex").getProperty<float>("nz");
  } catch (const std::runtime_error &) {}
  std::vector<std::vector<unsigned long>> input_faces;
  try {
    input_faces = ply.getFaceIndices();
  } catch (const std::runtime_error &) {}
  auto input_colors = ply.getVertexColors();
  vertices.resize(input_vertices.size());
  normals.reserve(input_normals_x.size());
  faces.resize(input_faces.size());
  colors.resize(input_colors.size());
  transform(
          input_vertices.begin(), input_vertices.end(), vertices.begin(),
          [] (const std::array<double, 3> &pt){ return VectorType((float)pt[0], (float)pt[1], (float)pt[2]); }
  );
  transform(
          input_faces.begin(), input_faces.end(), faces.begin(),
          [] (const std::vector<long unsigned int> &pt){ return std::array<unsigned int, 3>{(unsigned int)pt[0], (unsigned int)pt[1], (unsigned int)pt[2]}; }
  );
  transform(
          input_colors.begin(), input_colors.end(), colors.begin(),
          [] (const std::array<unsigned char, 3> &pt){ return std::array<unsigned int, 3>{(unsigned int)pt[0], (unsigned int)pt[1], (unsigned int)pt[2]}; }
  );

  for (auto tup : boost::combine(input_normals_x, input_normals_y, input_normals_z)) {    // <---
    float x, y, z;
    boost::tie(x, y, z) = tup;
    normals.emplace_back(x, y, z);
  }

  std::cout << "  #vertices " << vertices.size() << std::endl;
  std::cout << "  #normals  " << normals.size() << std::endl;
  std::cout << "  #colors   " << colors.size() << std::endl;
  std::cout << "  #faces    " << faces.size() << std::endl;
}

#endif //SURFACE_SPLATTING_PLY_LOADER_HPP
