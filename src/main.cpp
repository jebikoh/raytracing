#include "camera.hpp"
#include "hittable.hpp"
#include "hittablelist.hpp"
#include "rtx.hpp"
#include "sphere.hpp"

using namespace jtx;

int main() {
    HittableList world;
    world.add(make_shared<Sphere>(Point3d(0, 0, -1), 0.5));
    world.add(make_shared<Sphere>(Point3d(0, -100.5, -1), 100));

    Camera cam;
    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;

    cam.render(world);
    cam.save("image.png");
}
