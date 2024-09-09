#include "renderer/base_renderer.hpp"
#include "thread/thread_pool.hpp"
#include "utils/progress.hpp"
#include <utils/profile.hpp>
void BaseRenderer::render(size_t spp, const std::filesystem::path& filename)
{
    PROFILE("Render " +std::to_string(spp) +"spp " + filename.string() )
    size_t current_spp = 0, increment =1;
    auto &film = camera.getFilm();
    film.clear();
    Progress progress(film.getWidth() * film.getHeight() * spp);
    while (current_spp < spp)
    {
        // Render increase

        thread_pool.parallelFor(film.getWidth(), film.getHeight(), [&](size_t x, size_t y){
            for (int i =0 ; i < increment; i++)
            {
                film.addSample(x, y, renderPixel({x,y}));
            }
            progress.update(increment);
        });
        thread_pool.wait();
        current_spp += increment;
        increment = std::min<size_t>(current_spp, 32);

        film.save(filename);
    }
}