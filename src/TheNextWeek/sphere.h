#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"
#include "vec3.h"

class sphere :public hittable {
public:
    sphere(point3 _center, double _radius, shared_ptr<material> _material) :center1(_center), radius(_radius), mat(_material), is_moving(false) {
        auto rvec = vec3(radius, radius, radius);
        bbox = aabb(center1 - rvec, center1 + rvec);
    }

    sphere(point3 _center1, point3 _center2, double _radius, shared_ptr<material> _material) :center1(_center1), radius(_radius), mat(_material), is_moving(true) {
        auto rvec = vec3(radius, radius, radius);
        aabb box1(_center1 - rvec, _center1 + rvec);
        aabb box2(_center2 - rvec, _center2 + rvec);
        bbox = aabb(box1, box2);

        center_vec = _center2 - _center1;
    }

    bool hit(const ray &r, interval ray_t, hit_record &rec) const override {
        point3 center = is_moving ? sphere_center(r.time()) : center1;
        vec3 oc = r.origin() - center;
        auto a = r.direction().length_squared();
        auto half_b = dot(oc, r.direction());
        auto c = oc.length_squared() - radius * radius;

        auto discriminant = half_b * half_b - a * c;
        if (discriminant < 0)
            return false;
        auto sqrtd = std::sqrt(discriminant);

        //Find the nearest root that lies in the acceptable range.
        auto root = (-half_b - sqrtd) / a;
        if (!ray_t.surrounds(root)) {
            root = (-half_b + sqrtd) / a;
            if (!ray_t.surrounds(root))
                return false;
        }

        rec.t = root;
        rec.p = r.at(root);
        //rec.normal = (rec.p - center) / radius;
        vec3 outward_normal = (rec.p - center) / radius;
        rec.set_face_normal(r, outward_normal);
        rec.mat = mat;

        /*
        double latitude = asin(outward_normal.y()) + 0.5 * pi;
        vec3 plane_vec = vec3(outward_normal.x(), 0, outward_normal.z());
        double longitude = (plane_vec.z() > 0) ? atan(plane_vec.x() / plane_vec.z()) + 1.5 * pi
            : atan(plane_vec.x() / plane_vec.z()) + 0.5 * pi;
        rec.u = latitude / pi;
        rec.v = longitude / (2 * pi);
        */
        get_sphere_uv(outward_normal, rec.u, rec.v);


        return true;
    }

    aabb bounding_box() const override { return bbox; }

private:
    point3 center1;
    double radius;
    shared_ptr<material> mat;
    bool is_moving;
    vec3 center_vec;
    aabb bbox;

    point3 sphere_center(double time) const {
        return center1 + center_vec * time;
    }

    static void get_sphere_uv(const vec3& p, double& u, double& v) {
        // p: a given point on the sphere of radius one, centered at the origin.
        // u: returned value [0,1] of angle around the Y axis from X=-1.
        // v: returned value [0,1] of angle from Y=-1 to Y=+1.
        //     <1 0 0> yields <0.50 0.50>       <-1  0  0> yields <0.00 0.50>
        //     <0 1 0> yields <0.50 1.00>       < 0 -1  0> yields <0.50 0.00>
        //     <0 0 1> yields <0.25 0.50>       < 0  0 -1> yields <0.75 0.50>

        auto theta = acos(-p.y());
        //auto phi = atan2(-p.z(), p.x()) + pi;
        auto phi = atan2(-p.z(), p.x()) + pi;

        u = phi / (2*pi);
        v = theta / pi;
    }
};

#endif