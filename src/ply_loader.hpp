#ifndef SURFACE_SPLATTING_PLY_LOADER_HPP
#define SURFACE_SPLATTING_PLY_LOADER_HPP

#include <array>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

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
  } catch (const std::runtime_error &) {
    try {
      std::vector<double> input_normals_x_, input_normals_y_, input_normals_z_;
      input_normals_x_ = ply.getElement("vertex").getProperty<double>("nx");
      input_normals_x.resize(input_normals_x_.size());
      std::transform(input_normals_x_.begin(), input_normals_x_.end(), input_normals_x.begin(), [](auto&& i){ return (float)i; });
      input_normals_y_ = ply.getElement("vertex").getProperty<double>("ny");
      input_normals_y.resize(input_normals_y_.size());
      std::transform(input_normals_y_.begin(), input_normals_y_.end(), input_normals_y.begin(), [](auto&& i){ return (float)i; });
      input_normals_z_ = ply.getElement("vertex").getProperty<double>("nz");
      input_normals_z.resize(input_normals_z_.size());
      std::transform(input_normals_z_.begin(), input_normals_z_.end(), input_normals_z.begin(), [](auto&& i){ return (float)i; });
    } catch (const std::runtime_error &) {
      throw std::runtime_error("For splatting, normals are necessary!");
    }
  }
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

  for (auto i = 0; i < input_normals_x.size(); ++i) {
    normals.emplace_back(input_normals_x[i], input_normals_y[i], input_normals_z[i]);
  }

  std::cout << "  #vertices " << vertices.size() << std::endl;
  std::cout << "  #normals  " << normals.size() << std::endl;
  std::cout << "  #colors   " << colors.size() << std::endl;
  std::cout << "  #faces    " << faces.size() << std::endl;
}

#endif //SURFACE_SPLATTING_PLY_LOADER_HPP
