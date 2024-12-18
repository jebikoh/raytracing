#pragma once
#include "camera.hpp"
#include "hittable.hpp"
#include "material.hpp"

// Very basic scene struct
// Will change if this starts running into performance issues
struct Scene {
    std::string name;

    std::vector<Lambertian> lambertians;
    std::vector<Dielectric> dielectrics;
    std::vector<Metal> metals;

    std::vector<Sphere> spheres;

    HittableList objects;
    Camera::Properties cameraProperties;
};

Scene createDefaultScene();