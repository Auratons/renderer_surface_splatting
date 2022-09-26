#include <algorithm>
#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

#include "stb_image_write.h"
#include "utils.hpp"

void save_png(GLuint texture_id, const std::string &filename) {
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  auto raw_data = std::vector<float>(4 * viewport[2] * viewport[3]);
  glBindTexture(GL_TEXTURE_2D, texture_id);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, (void*)raw_data.data());
//  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
  auto png = std::vector<unsigned char>(4 * viewport[2] * viewport[3]);  // 4=RGBA
  auto begin = (const float*)raw_data.data();
  auto end = (const float*)(raw_data.data() + raw_data.size());
  std::transform(begin, end, png.begin(), [](const float &val){ return (unsigned char)(val * 255.0f); });
  // OpenGL expects the 0.0 coordinate on the y-axis to be on the bottom side of the image, but images usually
  // have 0.0 at the top of the y-axis. For now, this unifies output with the visualisation on the screen.
  stbi_flip_vertically_on_write(true);
  stbi_write_png(filename.c_str(), viewport[2], viewport[3], 4, png.data(), 4 * viewport[2]);  // 4=RGBA
}

std::ostream &glm::operator<<(std::ostream &out, const glm::mat4 &m) {
  std::ios out_state(nullptr);
  out_state.copyfmt(out);
  out << std::setprecision(std::numeric_limits<float>::digits10)
      << std::fixed
      <<  "[[ " << m[0][0] << ", " << m[1][0] << ", " << m[2][0] << ", " << m[3][0]
      << " ][ " << m[0][1] << ", " << m[1][1] << ", " << m[2][1] << ", " << m[3][1]
      << " ][ " << m[0][2] << ", " << m[1][2] << ", " << m[2][2] << ", " << m[3][2]
      << " ][ " << m[0][3] << ", " << m[1][3] << ", " << m[2][3] << ", " << m[3][3] << " ]]";
  out.copyfmt(out_state);
  return out;
}

void glm::from_json(const nlohmann::json &j, glm::mat4 &P) {
  // Create matrix from row-major to column-major
  auto row0 = j[0].get<std::vector<float>>();
  auto row1 = j[1].get<std::vector<float>>();
  auto row2 = j[2].get<std::vector<float>>();
  auto row3 = j[3].get<std::vector<float>>();
  P = glm::mat4(
          row0[0], row1[0], row2[0], row3[0],
          row0[1], row1[1], row2[1], row3[1],
          row0[2], row1[2], row2[2], row3[2],
          row0[3], row1[3], row2[3], row3[3]
  );
}
