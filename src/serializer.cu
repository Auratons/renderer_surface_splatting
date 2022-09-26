#include <string>
#include <thread>

#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <CLI/App.hpp>
#include <CLI/Formatter.hpp>  // Even thought seems unused it's needed
#include <CLI/Config.hpp>  // Even thought seems unused it's needed
#include <glm/glm.hpp>
#include <thrust/device_vector.h>

#include "ply_loader.hpp"

using namespace std;

int main(int argc, char** argv) {
  string pcd_path;
  CLI::App args{"Serializer for radii"};
  auto file = args.add_option("-f,--file", pcd_path, "Path to pointcloud to process");
  file->required();
  CLI11_PARSE(args, argc, argv);

  std::vector<glm::vec3> vertices_host, normals_host;
  std::vector<std::array<unsigned int, 3>> faces_host, colors_host;

  load_ply<glm::vec3>(pcd_path, vertices_host, normals_host, faces_host, colors_host);

//  std::vector<float3> vertices_host_f3(vertices_host.size());
//  std::transform(vertices_host.begin(), vertices_host.end(), vertices_host_f3.begin(), [](auto pt){ return make_float3(pt[0], pt[1], pt[2]); });

//  auto vertices = thrust::device_vector<float3>(vertices_host_f3.begin(), vertices_host_f3.end());
  auto vertices = thrust::device_vector<glm::vec3>(vertices_host.begin(), vertices_host.end());


//  auto radii = thrust::device_vector<float>(vertices_host.size());
//  std::vector<std::thread> threads(std::thread::hardware_concurrency());
//
//  for (std::size_t i(0); i < threads.size(); ++i)
//  {
//    std::size_t b = i * vertices.size() / threads.size();
//    std::size_t e = (i + 1) * vertices.size() / threads.size();
//
//    threads[i] = std::thread([b, e, &vertices, &radii]() {
//      //brute force
//      for (std::size_t i = b; i < e; ++i) {
//        auto tmp = thrust::device_vector<float>(vertices.size());
//        auto op = [vertices_begin = vertices.data().get(), current_target = i] __device__ (auto neighbor){
//          auto target_center = *(vertices_begin + current_target);
//          auto d = glm::length(target_center - neighbor);
//          return (d < 0.0005) ? 10.0f : d;  // There is always zero for the same point, this is workaround to get nn.
//        };
//        thrust::transform(vertices.begin(), vertices.end(), tmp.begin(), op);
//        radii[i] = *thrust::min_element(tmp.begin(), tmp.end());
//      }
//    });
//  }
//
//  for (auto& t : threads) { t.join(); }

  //brute force                                                                                                                                                                                                                               
  auto radii = thrust::device_vector<float>(vertices_host.size());
  auto tmp = thrust::device_vector<float>(vertices_host.size());
  for (size_t i = 0; i < vertices.size(); ++i) {
    auto op = [vertices_begin = vertices.data().get(), current_target = i] __device__ (auto neighbor){
        auto target_center = *(vertices_begin + current_target);
        auto d = glm::length(target_center - neighbor);
        return (d < 0.0005) ? 10.0f : d;  // There is always zero for the same point, this is workaround to get nn.
    };
    thrust::transform(vertices.begin(), vertices.end(), tmp.begin(), op);
    radii[i] = *thrust::min_element(tmp.begin(), tmp.end());
  }
  std::vector<float> radii_host(radii.begin(), radii.end());

  std::ofstream ofs(pcd_path + ".radii");
  boost::archive::text_oarchive oa(ofs);
  // write class instance to archive
  oa & radii_host;
}