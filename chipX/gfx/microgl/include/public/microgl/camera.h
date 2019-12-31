#pragma once

#include <microgl/matrix_4x4.h>
#include <microgl/math.h>
#include <microgl/vec3.h>


namespace microgl {
    template <typename number>
    class camera {
    private:
        using const_ref = const number &;
        using vertex = vec3<number>;
        using mat4 = matrix_4x4<number> ;
    public:

        /**
         * compute the world to camera matrix by computing
         * compute the inverse of 4x4 Euclidean transformation matrix
         *
         * Euclidean transformation is translation, rotation, and reflection.
         * With Euclidean transform, only the position and orientation of the object
         * will be changed. Euclidean transform does not change the shape of an object
         * (no scaling). Length and angle are reserved.
         *
         * Use inverseAffine() if the matrix has scale and shear transformation.
         *
         * M = [ R | T ]
         *     [ --+-- ]    (R denotes 3x3 rotation/reflection matrix)
         *     [ 0 | 1 ]    (T denotes 1x3 translation matrix)
         *
         * y = M*x  ->  y = R*x + T  ->  x = R^-1*(y - T)  ->  x = R^T*y - R^T*T
         * (R is orthogonal,  R^-1 = R^T)
         *
         *  [ R | T ]-1    [ R^T | -R^T * T ]    (R denotes 3x3 rotation matrix)
         *  [ --+-- ]   =  [ ----+--------- ]    (T denotes 1x3 translation)
         *  [ 0 | 1 ]      [  0  |     1    ]    (R^T denotes R-transpose)
        */
        static
        mat4 angleAt(const vertex &position,
                     const_ref pitch, const_ref yaw, const_ref roll) {
            mat4 mat;
            vertex vec;
            // rotation angle about X-axis (pitch)
            number sx = microgl::math::sin(pitch);
            number cx = microgl::math::cos(pitch);
            // rotation angle about Y-axis (yaw)
            number sy = microgl::math::sin(yaw);
            number cy = microgl::math::cos(yaw);
            // rotation angle about Z-axis (roll)
            number sz = microgl::math::sin(roll);
            number cz = microgl::math::cos(roll);

            vertex x_axis {cy*cz, sx*sy*cz + cx*sz, -cx*sy*cz + sx*sz};
            vertex y_axis {-cy*sz, -sx*sy*sz + cx*cz, cx*sy*sz + sx*cz};
            vertex z_axis {sy, -sx*cy, cx*cy};

            // copy it to matrix transposed because it is inverted,
            // our matrix is column major, therefore inserting at rows
            // has the effect of inverting for euclidean transforms
            mat.setRow(0, x_axis);
            mat.setRow(1, y_axis);
            mat.setRow(2, z_axis);

            // set inverted translation
            vertex trans {-(x_axis*position), -(y_axis*position), -(z_axis*position)};
            mat.setColumn(3, trans);

            return mat;
        }


        ///////////////////////////////////////////////////////////////////////////////
        // set transform matrix equivalent to gluLookAt()
        // 1. Mt: Translate scene to camera position inversely, (-x, -y, -z)
        // 2. Mr: Rotate scene inversly so camera looks at the scene
        // 3. Find matrix = Mr * Mt
        //       Mr               Mt
        // |r0  r4  r8  0|   |1  0  0 -x|   |r0  r4  r8  r0*-x + r4*-y + r8 *-z|
        // |r1  r5  r9  0| * |0  1  0 -y| = |r1  r5  r9  r1*-x + r5*-y + r9 *-z|
        // |r2  r6  r10 0|   |0  0  1 -z|   |r2  r6  r10 r2*-x + r6*-y + r10*-z|
        // |0   0   0   1|   |0  0  0  1|   |0   0   0   1                     |
        ///////////////////////////////////////////////////////////////////////////////
        static
        mat4 lookAt(const vertex & position, const vertex& target, const vertex& up)
        {
            mat4 result{};

            // 3 axis of rotation matrix for scene
            vertex z_axis = (position-target).normalize(); // forward
            vertex x_axis = up.cross(z_axis).normalize(); // left
            vertex y_axis = z_axis.cross(x_axis); // up

            // copy it to matrix transposed because it is inverted
            result.identity();
            result.setRow(0, x_axis);
            result.setRow(1, y_axis);
            result.setRow(2, z_axis);

            // set translation part
            vertex trans {-(x_axis*position), -(y_axis*position), -(z_axis*position)};

            result.setColumn(3, trans);

            return result;
        }

        static
        matrix_4x4<number> perspective(const number &horizontal_fov_radians,
                                       const number & screen_width, const number & screen_height,
                                       const number & near, const number & far) {
            return perspective(horizontal_fov_radians, screen_width/screen_height, near, far);
        }

        static
        matrix_4x4<number> perspective(const number &horizontal_fov_radians,
                                       const number & aspect_ratio,
                                       const number & near, const number & far) {
            matrix_4x4<number> mat{};

//            auto tan = number(1) / (microgl::math::tan(horizontal_fov_radians/number(2)));
//            auto tan2 = tan/aspect_ratio;
//
//            // an optimized version
//            mat(0, 0) = tan2;
//            mat(1, 1) = tan;
//            mat(2, 2) = -(far + near) / (far - near);
//            mat(2, 3) = -(number(2) * far * near) / (far - near);
//            mat(3, 2) = -number(1);
//            mat(3, 3) = 0;
//
//            return mat;

            auto scale = microgl::math::tan(horizontal_fov_radians/number(2)) * near;
            auto r = aspect_ratio * scale, l = -r;
            auto t = scale, b = -t;
            return perspective(l,r,b,t,near,far);
        }

        static
        matrix_4x4<number> perspective(const number &l, const number & r,
                                       const number & b, const number & t,
                                       const number & near, const number & far) {
            matrix_4x4<number> mat{};

            // set OpenGL perspective projection matrix
            mat(0, 0) = number(2) * near / (r - l);
            mat(1, 0) = 0;
            mat(2, 0) = 0;
            mat(3, 0) = 0;

            mat(0, 1) = 0;
            mat(1, 1) = number(2) * near / (t - b);
            mat(2, 1) = 0;
            mat(3, 1) = 0;

            mat(0, 2) = (r + l) / (r - l);
            mat(1, 2) = (t + b) / (t - b);
            mat(2, 2) = -(far + near) / (far - near);
            mat(3, 2) = -number(1);

            mat(0, 3) = 0;
            mat(1, 3) = 0;
            mat(2, 3) = -(number(2) * far * near) / (far - near);
            mat(3, 3) = 0;

//            return mat;
            return mat;
        }

//        template <typename number>
        static
        matrix_4x4<number> orthographic(const number &l, const number & r,
                                       const number & b, const number & t,
                                       const number & near, const number & far) {
            // map [l,r]->[-1,1], [b,t]->[-1,1], [-near,-far]->[-1,1]
            // we assume the projection is to be used on right-handed system
            // observing the negative z axis
            matrix_4x4<number> mat{};

            mat(0, 0) = number(2) / (r - l);
            mat(0, 1) = 0;
            mat(0, 2) = 0;
            mat(0, 3) = 0;

            mat(1, 0) = 0;
            mat(1, 1) = number(2) / (t - b);
            mat(1, 2) = 0;
            mat(1, 3) = 0;

            mat(2, 0) = 0;
            mat(2, 1) = 0;
            mat(2, 2) = -number(2) / (far - near);
            mat(2, 3) = 0;

            mat(3, 0) = -(r + l) / (r - l);
            mat(3, 1) = -(t + b) / (t - b);
            mat(3, 2) = -(far + near) / (far - near); // runs from negative one
            mat(3, 3) = 1;

//            return mat;
            return mat.transpose();
        }

    };

}