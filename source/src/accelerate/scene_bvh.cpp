#include "accelerate/scene_bvh.hpp"
#include "utils/debug_macro.hpp"
#include <array>
#include <iostream>

void SceneBVH::build(std::vector<ShapeInstance>&& instances)
{
    auto *root = allocator.allocate();
    auto temp_instances = std::move(instances);

	for (auto& instance : temp_instances)
	{
		if (instance.shape.getBounds().isValid())
		{
            instance.updateBounds();
			root->instances.push_back(instance);
		}
		else
		{
			infinity_instances.push_back(instance);
		}
	}

    root->updateBounds();
    root->depth = 1;

    SceneBVHState state{};
	size_t instance_count = root->instances.size();
    recursiveSplit(root,state);

	std::cout << "Total Node Count: " << state.total_node_count << '\n';
	std::cout << "Leaf Node Count: " << state.leaf_node_count << std::endl;
    std::cout << "ShapeInstance Count: " << instance_count << std::endl;
	std::cout << "Max Leaf Node ShapeInstance Count: " << state.max_leaf_node_instance_count << std::endl;
	std::cout << "Max Left Node Depth: " << state.max_leaf_node_depth << std::endl;

	nodes.reserve(state.total_node_count);
    ordered_instances.reserve(instance_count);
    recursiveFlatten(root);
}

std::optional<HitInfo> SceneBVH::intersect(const Ray &ray, float t_min, float t_max) const
{
    std::optional<HitInfo> closest_hit_info{};
    const ShapeInstance* closest_instance = nullptr;
    // 提前计算出光线的方向
    glm::bvec3 dir_is_neg = {
        ray.direction.x < 0,
        ray.direction.y < 0,
        ray.direction.z < 0
    };

	glm::vec3 inv_direction = 1.f / ray.direction;

    DEBUG_LINE(size_t bounds_test_count =0);

    std::array<int, 32> stack;
    auto ptr = stack.begin();
    size_t current_node_index = 0;

    while (true)
    {
        auto &node = nodes[current_node_index];

        DEBUG_LINE(bounds_test_count++;)

        if (!node.bounds.hasIntersection(ray, inv_direction, t_min, t_max))
        {
            if (ptr == stack.begin()) break;
            current_node_index = *(--ptr);
            continue;
        }

        if (node.instance_count == 0)
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
            auto instance_iter = ordered_instances.begin() + node.instance_index;
            for (size_t i = 0; i < node.instance_count; i++)
            {
                auto ray_object = ray.ObjectFromWorld(instance_iter->object_from_world);
                auto hit_info = instance_iter->shape.intersect(ray_object, t_min, t_max);

                DEBUG_LINE(ray.bounds_test_count += ray_object.bounds_test_count;)
            	DEBUG_LINE(ray.triangle_test_count += ray_object.triangle_test_count;)
            
                if (hit_info)
                {
                    t_max = hit_info->t;
                    closest_hit_info = hit_info;
                    closest_instance = &(*instance_iter);
                }
                ++instance_iter;
            }
             if (ptr == stack.begin()) break;
            current_node_index = *(--ptr);
        }
    }

    for (const auto& infinity_instance : infinity_instances)
    {
        auto ray_object = ray.ObjectFromWorld(infinity_instance.object_from_world);
        auto hit_info = infinity_instance.shape.intersect(ray_object, t_min, t_max);

        DEBUG_LINE(ray.bounds_test_count += ray_object.bounds_test_count;)
            DEBUG_LINE(ray.triangle_test_count += ray_object.triangle_test_count;)

            if (hit_info)
            {
                t_max = hit_info->t;
                closest_hit_info = hit_info;
                closest_instance = &(infinity_instance);
            }
    }

    // 我们最终得到的结果是在物体空间中的，所以需要将其转换到世界空间中
    if (closest_instance)
    {
        closest_hit_info->hit_point = closest_instance->world_from_object * glm::vec4(closest_hit_info.value().hit_point, 1.0f);
        // 需要注意的是，法线的变换需要使用到法线矩阵，即世界空间到物体空间的转置逆矩阵，这里我们已经有了物体空间到世界空间的逆矩阵，所以我们可以直接使用它的转置矩阵
        closest_hit_info->normal = glm::normalize(glm::vec3(glm::transpose(closest_instance->object_from_world) * glm::vec4(closest_hit_info->normal, 0.f)));
        // 添加材质信息
        closest_hit_info->material = &closest_instance->material;
        // closest_hit_info->hit_point += EPS * closest_hit_info->normal;
    }

    if (closest_hit_info.has_value())
    {
        DEBUG_LINE(ray.bounds_test_count = bounds_test_count;)
    }

    
    return closest_hit_info;
}

void SceneBVH::recursiveSplit(SceneBVHTreeNode *node, SceneBVHState& state)
{
    state.total_node_count++;
    if (node->instances.size() == 1 || node->depth > 32)
    {
        state.addLeafNode(node);
        return;
    }

    glm::vec3 diag = node->bounds.diagonal();

    float min_cost = std::numeric_limits<float>::infinity();
    size_t min_split_index = 0;
    Bounds min_child0_bounds{}, min_child1_bounds{};
    size_t min_child0_instance_count = 0, min_child1_instance_count = 0;
    constexpr size_t bucket_count = 12;
    std::vector<size_t> instance_indice_buckets[3][bucket_count] = {};

    for (size_t axis = 0; axis < 3; axis++)
    {
        Bounds bounds_buckets[bucket_count] = {};
		size_t instance_count_buckets[bucket_count] = {};
        size_t instance_index = 0;
        for (const auto& instance : node->instances)
        {
			size_t bucket_index = glm::clamp<size_t>(
                glm::floor((instance.center[axis] - node->bounds.b_min[axis]) * bucket_count / diag[axis]),
                0u, bucket_count - 1);
			bounds_buckets[bucket_index].expand(instance.bounds);
			instance_count_buckets[bucket_index]++;
            instance_indice_buckets[axis][bucket_index].push_back(instance_index);
            instance_index++;
        }

        Bounds left_bounds = bounds_buckets[0];
        size_t left_instance_count = instance_count_buckets[0];
        for (size_t i =1 ;i <= bucket_count - 1; i++)
        {
			Bounds right_bounds{};
			size_t right_instance_count = 0;
			for (size_t j = bucket_count - 1; j >= i; j--)
			{
				right_bounds.expand(bounds_buckets[j]);
				right_instance_count += instance_count_buckets[j];
			}
            if (right_instance_count == 0)
            {
                break;
            }
            if (left_instance_count != 0)
            {
				float cost = left_bounds.area() * left_instance_count + right_bounds.area() * right_instance_count;
				if (cost < min_cost)
				{
					min_cost = cost;
					node->split_axis = axis;
                    min_split_index = i;
                    min_child0_bounds = left_bounds;
					min_child1_bounds = right_bounds;
					min_child0_instance_count = left_instance_count;
					min_child1_instance_count = right_instance_count;
				}
			}
			left_bounds.expand(bounds_buckets[i]);
			left_instance_count += instance_count_buckets[i];
        }

        if (min_split_index ==  0)
        {
            state.addLeafNode(node);
            return;
        }
    }

    auto *child0 = allocator.allocate();
    auto *child1 = allocator.allocate();
    node->children[0] = child0;
    node->children[1] = child1;

	child0->instances.reserve(min_child0_instance_count);
	child1->instances.reserve(min_child1_instance_count);

    for (size_t i = 0; i < min_split_index; i++)
    {
        for (size_t idx : instance_indice_buckets[node->split_axis][i])
        {
            child0->instances.push_back(node->instances[idx]);
        }
    }

    for (size_t i = min_split_index; i < bucket_count; i++)
    {
        for (size_t idx : instance_indice_buckets[node->split_axis][i])
        {
            child1->instances.push_back(node->instances[idx]);
        }
    }

    node->instances.clear();
    node->instances.shrink_to_fit();
    child0->depth = node->depth + 1;
    child1->depth = node->depth + 1;

    child0->bounds = min_child0_bounds;
    child1->bounds = min_child1_bounds;

    recursiveSplit(child0,state);
    recursiveSplit(child1,state);
}

size_t SceneBVH::recursiveFlatten(SceneBVHTreeNode* node)
{
    SceneBVHNode bvh_node{
        node->bounds,
        0,
        static_cast<uint16_t>(node->instances.size()),
        static_cast<uint8_t>(node->split_axis)
    };
    auto idx = nodes.size();
    nodes.push_back(bvh_node);
    if (bvh_node.instance_count == 0)
    {
        recursiveFlatten(node->children[0]);
        nodes[idx].child1_index = recursiveFlatten(node->children[1]);
    }
    else 
    {
        nodes[idx].instance_index = ordered_instances.size();
        for (const auto& instance : node->instances)
        {
            ordered_instances.push_back(instance);
        }
    }
    return idx;
}