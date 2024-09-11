#pragma once

#include "bounds.hpp"
#include "../shape/shape.hpp"

#include <vector>

struct ShapeInstance
{
    const Shape& shape;
    const Material& material;
    glm::mat4 world_from_object;
    glm::mat4 object_from_world;

    Bounds bounds{}; // 获取世界空间的包围盒
    glm::vec3 center{};

    void updateBounds()
    {
        bounds = {};
        auto bounds_object = shape.getBounds();
        for (size_t idx = 0; idx < 8; idx++)
        {
            auto corner_object = bounds_object.getCorner(idx);
            glm::vec3 corner_world = world_from_object * glm::vec4(corner_object, 1.f);
			bounds.expand(corner_world);
        }
        center = (bounds.b_max + bounds.b_min) * 0.5f;
    }
};

// 树状节点
struct SceneBVHTreeNode
{
    Bounds bounds{};
    std::vector<ShapeInstance> instances;
    SceneBVHTreeNode* children[2];
    size_t depth;
    size_t split_axis;

    void updateBounds()
    {
        bounds = {};
        for (const auto& instance : instances)
        {
            bounds.expand(instance.bounds);
        }
    }
};

// 线性节点
struct alignas(32) SceneBVHNode
{
    Bounds bounds{};
    // 对于叶子节点，只会用到instance_index, 非叶子只会用到child1_index
    // 因此可以合并，从而节省空间
    union{
        int child1_index; // 指向第二个子结点的索引
        int instance_index;
    };
   
    uint16_t instance_count;
    uint8_t split_axis;
};

struct SceneBVHState
{
    size_t total_node_count{};
	size_t leaf_node_count{};
    size_t max_leaf_node_instance_count{};
    size_t max_leaf_node_depth{};

    void addLeafNode(const SceneBVHTreeNode* node)
    {
		leaf_node_count++;
		max_leaf_node_instance_count = glm::max(max_leaf_node_instance_count, node->instances.size());
		max_leaf_node_depth = glm::max(max_leaf_node_depth, node->depth);

    }
};

class SceneBVHTreeNodeAllocator
{
public:
    SceneBVHTreeNodeAllocator() : ptr(4096) { };

    ~SceneBVHTreeNodeAllocator()
    {
		for (auto* node : nodes_list)
		{
			delete[] node;
		}
    }

    SceneBVHTreeNode* allocate()
    {
	    if (ptr == 4096)
	    {
			nodes_list.push_back(new SceneBVHTreeNode[4096]);
			ptr = 0;
	    }
		return &nodes_list.back()[ptr++];
    }
private:
    size_t ptr;
    std::vector<SceneBVHTreeNode*> nodes_list;

};

class SceneBVH : public Shape
{
public:
    void build(std::vector<ShapeInstance> && instances);

    std::optional<HitInfo> intersect(const Ray& ray, float t_min, float t_max) const override;
	Bounds getBounds() const override { return nodes[0].bounds; }
private:
    void recursiveSplit(SceneBVHTreeNode* node,SceneBVHState& state);
    size_t recursiveFlatten(SceneBVHTreeNode* node);

private:
	SceneBVHTreeNodeAllocator allocator{};
    // 存储展开后的线性结构
    std::vector<SceneBVHNode> nodes;
    std::vector<ShapeInstance> ordered_instances;
    std::vector<ShapeInstance> infinity_instances;
};