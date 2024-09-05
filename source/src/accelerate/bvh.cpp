#include "accelerate/bvh.hpp"

void BVH::build(std::vector<Triangle>&& triangles)
{
    root = new BVHNode{};
    root->triangles = std::move(triangles);
    root->updateBounds();
    recursiveSplit(root);
}

std::optional<HitInfo> BVH::intersect(const Ray &ray, float t_min, float t_max) const
{
    std::optional<HitInfo> closest_hit_info{};

    recursiveIntersect(root, ray, t_min, t_max,closest_hit_info);
    return closest_hit_info;
}

void BVH::recursiveSplit(BVHNode *node)
{
    if (node->triangles.size() == 1)
    {
        return;
    }

    glm::vec3 diag = node->bounds.diagonal();
    size_t max_axis = diag.x > diag.y ? (diag.x > diag.z ? 0 : 2) : (diag.y > diag.z ? 1 : 2);
    float mid = node->bounds.b_min[max_axis] + diag[max_axis] * 0.5f;
    std::vector<Triangle> child0_triangles, child1_triangles;
    for (const auto&triangle : node->triangles)
    {
        if ((triangle.p0[max_axis] + triangle.p1[max_axis] + triangle.p2[max_axis])/ 3.f < mid)
        {
            child0_triangles.push_back(triangle);
        }
        else
        {
            child1_triangles.push_back(triangle);
        }
    }
    if (child0_triangles.empty() || child1_triangles.empty())
    {
        return;
    }
    auto *child0 = new BVHNode{};
    auto *child1 = new BVHNode{};
    node->children[0] = child0;
    node->children[1] = child1;
    node->triangles.clear();
    node->triangles.shrink_to_fit();
    child0->triangles = std::move(child0_triangles);
    child1->triangles = std::move(child1_triangles);
    child0->updateBounds();
    child1->updateBounds();
    recursiveSplit(child0);
    recursiveSplit(child1);
}
void BVH::recursiveIntersect(BVHNode *node, const Ray &ray, float t_min, float t_max, std::optional<HitInfo> &closest_hit_info) const
{
    if (!node->bounds.hasIntersection(ray, t_min, t_max))
    {
        return;
    }
    if (node->triangles.empty())
    {
        recursiveIntersect(node->children[0], ray, t_min, t_max,closest_hit_info);
        recursiveIntersect(node->children[1], ray, t_min, t_max,closest_hit_info);
    }
    else
    {
        for (const auto& triangle : node->triangles)
        {
            auto hit_info = triangle.intersect(ray, t_min, t_max);
            if (hit_info)
            {
                t_max = hit_info->t;
                closest_hit_info = hit_info;
            }
        }
    }
}