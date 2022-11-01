#ifndef SURFACE_SPLATTING_UTILS_HPP
#define SURFACE_SPLATTING_UTILS_HPP

#include <string>

#include <GL/glew.h>

void save_png(GLuint texture_id, const std::string &filename);
void save_depth(GLuint texture_id, const std::string &filename, float prj_mat22, float prj_mat23);

namespace glm {
    std::ostream &operator<<(std::ostream &out, const glm::mat4 &m);
    void from_json(const nlohmann::json &j, glm::mat4 &P);
}

#endif //SURFACE_SPLATTING_UTILS_HPP
