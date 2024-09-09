#include "accelerate/bvh.hpp"
#include "utils/debug_macro.hpp"
#include <array>

void BVH::build(std::vector<Triangle>&& triangles)
{
    auto *root = new BVHTreeNode{};
    root->triangles = std::move(triangles);
    root->updateBounds();
    root->depth = 1;
    recursiveSplit(root);
    recursiveFlatten(root);
}

std::optional<HitInfo> BVH::intersect(const Ray &ray, float t_min, float t_max) const
{
    std::optional<HitInfo> closest_hit_info{};

    // 提前计算出光线的方向
    glm::bvec3 dir_is_neg = {
        ray.direction.x < 0,
        ray.direction.y < 0,
        ray.direction.z < 0
    };

    DEBUG_LINE(size_t bounds_test_count =0, triangle_test_count = 0;)

    std::array<int, 32> stack;
    auto ptr = stack.begin();
    size_t current_node_index = 0;

    while (true)
    {
        auto &node = nodes[current_node_index];

        DEBUG_LINE(bounds_test_count++;)

        if (!node.bounds.hasIntersection(ray, t_min, t_max))
        {
            if (ptr == stack.begin()) break;
            current_node_index = *(--ptr);
            continue;
        }

        if (node.triangle_count == 0)
        {
            if (dir_is_neg[node.split_axis])
            {
                *(ptr++) = current_node_index + 1;
                current_node_index = node.child1_index;
            }
            else 
            {
                current_node_index++;
                *(ptr++) = node.child1_index;
            }   
        }
        else 
        {
            auto triangle_iter = ordered_triangles.begin() + node.triangle_index;
            DEBUG_LINE(triangle_test_count += node.triangle_count;)
            for (size_t i = 0; i < node.triangle_count; i++)
            {
                auto hit_info = triangle_iter->intersect(ray, t_min, t_max);
                ++triangle_iter;
                if (hit_info)
                {
                    t_max = hit_info->t;
                    closest_hit_info = hit_info;
                    DEBUG_LINE(closest_hit_info->bounds_depth = node.depth;)
                }
            }
             if (ptr == stack.begin()) break;
            current_node_index = *(--ptr);
        }
    }

    if (closest_hit_info.has_value())
    {
        DEBUG_LINE(closest_hit_info->bounds_test_count = bounds_test_count;)
        DEBUG_LINE(closest_hit_info->triangle_test_count = triangle_test_count;)
    }

    
    return closest_hit_info;
}

void BVH::recursiveSplit(BVHTreeNode *node)
{
    if (node->triangles.size() == 1 || node->depth > 32)
    {
        return;
    }

    glm::vec3 diag = node->bounds.diagonal();
    size_t max_axis = diag.x > diag.y ? (diag.x > diag.z ? 0 : 2) : (diag.y > diag.z ? 1 : 2);
    node->split_axis = max_axis;
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
    auto *child0 = new BVHTreeNode{};
    auto *child1 = new BVHTreeNode{};
    node->children[0] = child0;
    node->children[1] = child1;
    node->triangles.clear();
    node->triangles.shrink_to_fit();
    child0->depth = node->depth + 1;
    child1->depth = node->depth + 1;
    child0->triangles = std::move(child0_triangles);
    child1->triangles = std::move(child1_triangles);
    child0->updateBounds();
    child1->updateBounds();
    recursiveSplit(child0);
    recursiveSplit(child1);
}

// 现在从树形结构改变成线性结构，判断相交测试使用循环而不是递归
// void BVH::recursiveIntersect(BVHTreeNode *node, const Ray &ray, float t_min, float t_max, std::optional<HitInfo> &closest_hit_info) const
// {
//     if (!node->bounds.hasIntersection(ray, t_min, t_max))
//     {
//         return;
//     }
//     if (node->triangles.empty())
//     {
//         recursiveIntersect(node->children[0], ray, t_min, t_max,closest_hit_info);
//         recursiveIntersect(node->children[1], ray, t_min, t_max,closest_hit_info);
//     }
//     else
//     {
//         for (const auto& triangle : node->triangles)
//         {
//             auto hit_info = triangle.intersect(ray, t_min, t_max);
//             if (hit_info)
//             {
//                 t_max = hit_info->t;
//                 closest_hit_info = hit_info;
//             }
//         }
//     }
// }

size_t BVH::recursiveFlatten(BVHTreeNode* node)
{
    BVHNode bvh_node{
        node->bounds,
        0,
        static_cast<uint16_t>(node->triangles.size()),
        static_cast<uint8_t>(node->depth),
        static_cast<uint8_t>(node->split_axis)

    };
    auto idx = nodes.size();
    nodes.push_back(bvh_node);
    if (bvh_node.triangle_count == 0)
    {
        recursiveFlatten(node->children[0]);
        nodes[idx].child1_index = recursiveFlatten(node->children[1]);
    }
    else 
    {
        nodes[idx].triangle_index = ordered_triangles.size();
        for (const auto& triangle : node->triangles)
        {
            ordered_triangles.push_back(triangle);
        }
    }
    return idx;
}