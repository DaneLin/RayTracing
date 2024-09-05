#include "shape/model.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

#include "utils/profile.hpp"
#include "rapidobj/rapidobj.hpp"


Model::Model(const std::filesystem::path & filename)
{
    PROFILE("Load model " + filename.string());

    auto result = rapidobj::ParseFile(filename, rapidobj::MaterialLibrary::Ignore());

    for (const auto& shape : result.shapes)
    {
        size_t index_offset = 0;
        for (size_t num_face_vertex : shape.mesh.num_face_vertices)
        {
            if (num_face_vertex == 3)
            {
                auto index = shape.mesh.indices[index_offset];
                glm::vec3 pos0 {
                    result.attributes.positions[index.position_index* 3 + 0],
                    result.attributes.positions[index.position_index *3 + 1],
                    result.attributes.positions[index.position_index *3 + 2]
                };
                index = shape.mesh.indices[index_offset + 1];
                glm::vec3 pos1 {
                    result.attributes.positions[index.position_index* 3 + 0],
                    result.attributes.positions[index.position_index *3 + 1],
                    result.attributes.positions[index.position_index *3 + 2]
                };
                index = shape.mesh.indices[index_offset +2];
                glm::vec3 pos2 {
                    result.attributes.positions[index.position_index* 3 + 0],
                    result.attributes.positions[index.position_index *3 + 1],
                    result.attributes.positions[index.position_index *3 + 2]
                };

                if (index.normal_index >= 0)
                {
                    index = shape.mesh.indices[index_offset];
                    glm::vec3 normal0 {
                        result.attributes.normals[index.normal_index* 3 + 0],
                        result.attributes.normals[index.normal_index *3 + 1],
                        result.attributes.normals[index.normal_index *3 + 2]
                    };
                    index = shape.mesh.indices[index_offset + 1];
                    glm::vec3 normal1 {
                        result.attributes.normals[index.normal_index* 3 + 0],
                        result.attributes.normals[index.normal_index *3 + 1],
                        result.attributes.normals[index.normal_index *3 + 2]
                    };
                    index = shape.mesh.indices[index_offset +2];
                    glm::vec3 normal2 {
                        result.attributes.normals[index.normal_index* 3 + 0],
                        result.attributes.normals[index.normal_index *3 + 1],
                        result.attributes.normals[index.normal_index *3 + 2]
                    };
                    triangles.push_back(Triangle(pos0, pos1, pos2, normal0, normal1, normal2));
                }
                else
                {
                    triangles.push_back(Triangle(pos0, pos1, pos2));
                }
            }
            index_offset += num_face_vertex;
        }
    }

    build();
}

std::optional<HitInfo> Model::intersect(const Ray &ray, float t_min, float t_max) const
{
    if (!bounds.hasIntersection(ray, t_min, t_max))
    {
        return {};
    }
    std::optional<HitInfo> closest_hit_info {};

    for (const auto &triangle : triangles) {
        auto hit_info = triangle.intersect(ray, t_min, t_max);
        if (hit_info.has_value()) {
            t_max = hit_info->t;
            closest_hit_info = hit_info;
        }
    }

    return closest_hit_info;
}

void Model::build()
{
    for (const auto& triangle : triangles)
    {
        bounds.expand(triangle.p0);
        bounds.expand(triangle.p1);
        bounds.expand(triangle.p2);
    }
}
