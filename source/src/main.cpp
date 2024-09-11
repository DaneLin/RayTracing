#include "camera/film.hpp"
#include "camera/camera.hpp"

#include "renderer/normal_renderer.hpp"
#include "renderer/simple_rt_renderer.hpp"
#include "renderer/debug_renderer.hpp"

#include "shape/sphere.hpp"
#include "shape/model.hpp"
#include "shape/plane.hpp"
#include "shape/scene.hpp"

#include "utils/frame.hpp"
#include "utils/rgb.hpp"

int main(int, char**){


    Film film { 192 * 5, 108 *5 };
    Camera camera { film, { -7, 5, -7 }, { 0, 0, 0 }, 45 };
   

    Model model("../../models/dragon_87k.obj");
    Sphere sphere {
        { 1.5f, 0, 0 },
        1.f
    };

    Plane plane {
        { 0, 0, 0 },
        { 0, 1, 0 }
    };

    Scene scene;

    RNG rng{ 1234 };

    for (size_t i =0 ; i < 100; i++)
    {
	    glm::vec3 random_pos {
			rng.uniform() * 10 - 5,
			rng.uniform() * 2,
			rng.uniform() * 10 - 5
		};
		float u = rng.uniform();
        if (u<0.9)
        {
            scene.addShape(
                model,
                { RGB(202, 159, 117), rng.uniform() > 0.5 },
                random_pos,
                { rng.uniform() * 360,rng.uniform() * 360,rng.uniform() * 360 },
                { 1,1,1 }
            );
        }
        else if (u < 0.95)
        {
            scene.addShape(
                sphere,
                { { 1, 1, 1 }, true, RGB(255, 128, 128) },
                random_pos,
				{.4, .4, .4}
            );
        }
        else
        {
            random_pos.y += 4;
            scene.addShape(
                sphere,
                { { 1, 1, 1 }, false, RGB(128, 128, 255) },
                random_pos
            );
        }
    }


	scene.addShape(plane, { RGB(120, 204, 157) }, { 0, -0.5, 0 });

    scene.build();

    NormalRenderer normal_renderer(camera, scene);
    normal_renderer.render(32, "normal.ppm");

    BoundsTestCountRenderer btc_renderer(camera,scene);
    btc_renderer.render(1, "BTC.ppm");

    TriangleTestCountRenderer ttc_renderer(camera,scene);
    ttc_renderer.render(1, "TTC.ppm");


    SimpleRTRenderer simple_rt_renderer(camera, scene);
    simple_rt_renderer.render(256, "simple_rt.ppm");
//     int spp = 16;
//     Progress progress(film.getWidth() * film.getHeight() * spp);
//     RNG rng(12123);
//     thread_pool.parallelFor(film.getWidth(), film.getHeight(), [&](size_t x, size_t y) {
        
//        for (int i = 0; i < spp; i ++) 
//        {
//             auto ray = camera.generateRay({ x, y }, { rng.uniform(), rng.uniform() });
//             glm::vec3 beta = { 1, 1, 1 };
//             glm::vec3 color = { 0, 0, 0 };

//             while (true) {
//                 auto hit_info = scene.intersect(ray);
//                 if (hit_info.has_value()) {
//                     color += beta * hit_info->material->emissive;
//                     beta *= hit_info->material->albedo;

//                     ray.origin = hit_info->hit_point;

//                     Frame frame(hit_info->normal);
//                     glm::vec3 light_direction;
//                     if (hit_info->material->bIsSpecular) {
//                         glm::vec3 view_direction = frame.localFromWorld(-ray.direction);
//                         light_direction = { -view_direction.x, view_direction.y, -view_direction.z };
//                     } else {
//                         do {
//                             light_direction = {rng.uniform(), rng.uniform(), rng.uniform() };
//                             light_direction = light_direction * 2.f - 1.f;
//                         } while(glm::length(light_direction) > 1);
//                         if (light_direction.y < 0) {
//                             light_direction.y = -light_direction.y;
//                         }
//                     }
//                     ray.direction = frame.worldFromLocal(light_direction);
//                 } else {
//                     break;
//                 }
//             }

//             film.addSample(x, y, color);
//         }
//         progress.update(spp);
//     });


//    thread_pool.wait();
//    film.save("test.ppm");

    return 0;
}
