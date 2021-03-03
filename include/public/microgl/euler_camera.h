#pragma once

#include <microgl/camera.h>

namespace microgl {
    template <typename number>
    class euler_camera {
    private:
        using vertex = vec3<number>;
        using mat4 = matrix_4x4<number> ;
        vertex _coords;
        number _pitch, _yaw, _roll; // x,y,z
        mat4 _camera_matrix;
    public:

        euler_camera() : _coords{}, _pitch{0}, _yaw{0}, _roll {0}, _camera_matrix{} {
        }

        void rotateX(const number & radians) {
            _pitch+=radians;
        }
        void rotateY(const number & radians) {
            _yaw+=radians;
        }
        void rotateZ(const number & radians) {
            _roll+=radians;
        }
        void rotate(const number & x, const number & y, const number & z) {
            _pitch+=x;_yaw+=x;_roll+=z;
        }
        void translateX(const number & val) {
            _coords.x+=val;
        }
        void translateY(const number & val) {
            _coords.y+=val;
        }
        void translateZ(const number & val) {
            _coords.z+=val;
        }
        void translate(const number & x, const number & y, const number & z) {
            _coords.x+=x;_coords.y+=x;_coords.z+=z;
        }

        mat4& compute_world_to_camera_matrix() {

        }

    };

}