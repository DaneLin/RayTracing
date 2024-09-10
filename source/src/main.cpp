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
    Camera camera { film, { -3.6, 0, 0 }, { 0, 0, 0 }, 45 };
   

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
    scene.addShape(
        model,
        { RGB(202, 159, 117) },
        { 0, 0, 0 },
        { 1, 3, 2 },
        {2,2,2}
    );
     scene.addShape(
         sphere,
         { { 1, 1, 1 }, false, RGB(255, 128, 128) },
         { 0, 0, 2.5 }
     );
     scene.addShape(
         sphere,
         { { 1, 1, 1 }, false, RGB(128, 128, 255) },
         { 0, 0, -2.5 }
     );
     scene.addShape(
         sphere,
         { { 1, 1, 1 }, true },
         { 3, 0.5, -2 }
     );
     scene.addShape(plane, { RGB(120, 204, 157) }, { 0, -0.5, 0 });

    NormalRenderer normal_renderer(camera, scene);
    normal_renderer.render(32, "normal.ppm");

    BoundsTestCountRenderer btc_renderer(camera,scene);
    btc_renderer.render(1, "BTC.ppm");

    TriangleTestCountRenderer ttc_renderer(camera,scene);
    ttc_renderer.render(1, "TTC.ppm");

    BoundsDepthRenderer bd_renderer(camera,scene);
    bd_renderer.render(1, "BD.ppm");


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
