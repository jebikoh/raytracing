#pragma once

#include "rt.hpp"
#include "interval.hpp"
#include <jtxlib/util/taggedptr.hpp>

class Material;

struct HitRecord {
    Vec3 point;
    Vec3 normal;
    std::shared_ptr<Material> material;
    Float t;
    bool frontFace;

    void setFaceNormal(const Ray &r, const Vec3 &n) {
        frontFace = jtx::dot(r.dir, n) < 0;
        normal = frontFace ? n : -n;
    }
};

class Sphere;
class HittableList;

class Hittable : jtx::TaggedPtr<Sphere, HittableList> {
public:
    using TaggedPtr::TaggedPtr;

    bool hit(const Ray &r, Interval t, HitRecord &record) const;
};

class Sphere {
public:
    Sphere(const Vec3 &center, Float radius, std::shared_ptr<Material> material) : _center(center), _radius(radius), _material(material) {}

    bool hit(const Ray &r, Interval t, HitRecord &record) const {
        const Vec3 oc = _center - r.origin;
        const Float a = r.dir.lenSqr();
        const Float h = jtx::dot(r.dir, oc);
        const Float c = oc.lenSqr() - _radius * _radius;

        const Float discriminant = h * h - a * c;
        if (discriminant < 0) {
            return false;
        }

        auto sqrtd = jtx::sqrt(discriminant);
        auto root = (h - sqrtd) / a;
        if (!t.surrounds(root)) {
            root = (h + sqrtd) / a;
            if (!t.surrounds(root)) {
                return false;
            }
        }

        record.t = root;
        record.point = r.at(root);
        auto n = (record.point - _center) / _radius;
        record.setFaceNormal(r, n);
        record.material = _material;

        return true;
    }

private:
    Vec3  _center;
    Float _radius;
    std::shared_ptr<Material> _material;
};

class HittableList {
public:
    HittableList() = default;
    explicit HittableList(const std::shared_ptr<Hittable> &object) { add(object); }

    void add(const std::shared_ptr<Hittable>& object) { _objects.push_back(object); }
    void clear() { _objects.clear(); }

    bool hit(const Ray &r, Interval t, HitRecord &record) const {
        HitRecord tmpRecord;
        bool hitAnything = false;
        auto closestSoFar = t.max;

        for (const auto &object : _objects) {
            if (object->hit(r, Interval(t.min, closestSoFar), tmpRecord)) {
                hitAnything = true;
                closestSoFar = tmpRecord.t;
                record = tmpRecord;
            }
        }

        return hitAnything;
    }

private:
    std::vector<std::shared_ptr<Hittable>> _objects;
};

inline bool Hittable::hit(const Ray &r, Interval  t, HitRecord &record) const {
    auto fn = [&](auto ptr) { return ptr->hit(r, t, record); };
    return dispatch(fn);
}

