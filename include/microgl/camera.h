/*========================================================================================
 Copyright (2021), Tomer Shalev (tomer.shalev@gmail.com, https://github.com/HendrixString).
 All Rights Reserved.
 License is a custom open source semi-permissive license with the following guidelines:
 1. unless otherwise stated, derivative work and usage of this file is permitted and
    should be credited to the project and the author of this project.
 2. Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
========================================================================================*/
#pragma once

#include "math/vertex4.h"
#include "math/matrix_4x4.h"
#include "math.h"

namespace microgl {

    class camera {
    public:
        camera()=delete;
        camera(const camera &)=delete;
        camera & operator=(const camera &)=delete;
        ~camera()=delete;

        template <typename number> static
        vertex3<number> viewport(const vertex3<number> &ndc, unsigned width, unsigned height) {
            // given NDC= Normalized Device Coordinates, then transform them into
            // raster/canvas/viewport coords. We assume, that NDC coords are [-1,1] range.
            // todo:; currently I assume no z clipping has occured
            // z value is mapped to [0,1] range
            // convert to raster space
            const number zero=number(0), one = number(1), two=number(2);
            vertex3<number> result{};
            result.x = ((ndc.x + one)*width)/two;
            result.y = number(height) - (((ndc.y + one)*number(height))/two);
            result.z = (ndc.z + one)/two;
            // z clamping
//            if(result.z<zero)
//                result.z = zero;
//            if(result.z>one)
//                result.z = one;

            return result;
        }

        /**
         * compute the world to camera VIEW matrix by computing
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
         *
         * @tparam number the number type
         *
         * @tparam position 3d position of camera
         * @tparam pitch    x-axis rotation
         * @tparam yaw      y-axis rotation
         * @tparam roll     z-axis rotation
        **/
        template <typename number>
        static matrix_4x4<number> angleAt(const vertex3<number> & position,
                                          const number & pitch,
                                          const number & yaw,
                                          const number & roll) {
            using vertex3 = vertex3<number>;
            matrix_4x4<number> mat;
            vertex3 vec;
            // rotation angle about X-axis (pitch)
            number sx = microgl::math::sin(pitch);
            number cx = microgl::math::cos(pitch);
            // rotation angle about Y-axis (yaw)
            number sy = microgl::math::sin(yaw);
            number cy = microgl::math::cos(yaw);
            // rotation angle about Z-axis (roll)
            number sz = microgl::math::sin(roll);
            number cz = microgl::math::cos(roll);

            vertex3 x_axis {cy*cz, sx*sy*cz + cx*sz, -cx*sy*cz + sx*sz};
            vertex3 y_axis {-cy*sz, -sx*sy*sz + cx*cz, cx*sy*sz + sx*cz};
            vertex3 z_axis {sy, -sx*cy, cx*cy};

            // copy it to matrix transposed because it is inverted,
            // our matrix is column major, therefore inserting at rows
            // has the effect of inverting for euclidean transforms
            mat.setRow(0, x_axis);
            mat.setRow(1, y_axis);
            mat.setRow(2, z_axis);

            // set inverted translation
            vertex3 trans {-x_axis.dot(position), -y_axis.dot(position), -z_axis.dot(position)};
            mat.setColumn(3, trans);

            return mat;
        }

        template <typename number>
        vertex3<number> cross(const vertex3<number>& a, const vertex3<number>& b) const {
            return {a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x};
        }

        template <typename number>
        vertex3<number> & normalize(const vertex3<number> & v) {
            auto d = (v.x*v.x) + (v.y*v.y) + (v.z*v.z);
            if(d==number(0))
                return *this;
            auto inv_len = number(1) / microgl::math::sqrt(d);
            v.x *= inv_len;
            v.y *= inv_len;
            v.z *= inv_len;
            return *this;
        }

        /**
         * set view matrix equivalent to gluLookAt() VIEW MATRIX
         *
         * 1. Mt: Translate scene to camera position inversely, (-x, -y, -z)
         * 2. Mr: Rotate scene inversly so camera looks at the scene
         * 3. Find matrix = Mr * Mt
         *
         *       Mr               Mt
         * |r0  r4  r8  0|   |1  0  0 -x|   |r0  r4  r8  r0*-x + r4*-y + r8 *-z|
         * |r1  r5  r9  0| * |0  1  0 -y| = |r1  r5  r9  r1*-x + r5*-y + r9 *-z|
         * |r2  r6  r10 0|   |0  0  1 -z|   |r2  r6  r10 r2*-x + r6*-y + r10*-z|
         * |0   0   0   1|   |0  0  0  1|   |0   0   0   1                     |
         *
         * @tparam number   the number type
         *
         * @param position  3d position of camera
         * @param target    look at 3d position
         * @param up        the vector pointing up
         * @return
         */
        template <typename number>
        static matrix_4x4<number> lookAt(const vertex3<number> & position,
                                         const vertex3<number>& target,
                                         const vertex3<number>& up)
        {
            using vertex3 = vertex3<number>;
            matrix_4x4<number> result{};

            // 3 axis of rotation matrix for scene
            vertex3 z_axis = (position-target).normalize(); // forward
            vertex3 x_axis = up.cross(z_axis).normalize(); // left
            vertex3 y_axis = z_axis.cross(x_axis).normalize(); // up

            // copy it to matrix transposed because it is inverted
            result.identity();
            result.setRow(0, x_axis);
            result.setRow(1, y_axis);
            result.setRow(2, z_axis);

            // set translation part
            vertex3 trans {-x_axis.dot(position), -y_axis.dot(position), -z_axis.dot(position)};

            result.setColumn(3, trans);

            return result;
        }

        /**
         * compute perspective projection matrix
         *
         * @tparam number                   number type
         *
         * @param horizontal_fov_radians    horizontal angle of the camera frustum
         * @param screen_width              surface width
         * @param screen_height             surface height
         * @param near                      near plane
         * @param far                       far plane
         *
         * @return matrix_4x4<number> result
         */
        template <typename number> static
        matrix_4x4<number> perspective(const number &horizontal_fov_radians,
                                       const number & screen_width, const number & screen_height,
                                       const number & near, const number & far) {
            return perspective(horizontal_fov_radians, screen_width/screen_height, near, far);
        }

        /**
         * compute perspective projection matrix
         *
         * @tparam number                   number type
         *
         * @param horizontal_fov_radians    horizontal angle of the camera frustum
         * @param aspect_ratio              aspect ratio of the surface
         * @param near                      near plane
         * @param far                       far plane
         *
         * @return matrix_4x4<number> result
         */
        template <typename number> static
        matrix_4x4<number> perspective(const number & horizontal_fov_radians,
                                       const number & aspect_ratio,
                                       const number & near, const number & far) {
            matrix_4x4<number> mat{};

            auto tan = number(1) / (microgl::math::tan(horizontal_fov_radians/number(2)));
            auto tan2 = tan/aspect_ratio;

            // an optimized version
            mat(0, 0) = tan2;
            mat(1, 1) = tan;
            mat(2, 2) = -(far + near) / (far - near);
            mat(2, 3) = -(number(2) * far * near) / (far - near);
            mat(3, 2) = -number(1);
            mat(3, 3) = 0;

            return mat;

            auto scale = microgl::math::tan(horizontal_fov_radians/number(2)) * near;
            auto r = aspect_ratio * scale, l = -r;
            auto t = scale, b = -t;
            return perspective(l,r,b,t,near,far);
        }

        /**
         * compute perspective projection matrix
         *
         * @tparam number   number type
         *
         * @param l         left plane
         * @param r         right plane
         * @param b         bottom plane
         * @param t         top plane
         * @param n         near plane
         * @param f         far plane
         *
         * @return matrix_4x4<number> result
         */
        template <typename number> static
        matrix_4x4<number> perspective(const number & l, const number & r,
                                       const number & b, const number & t,
                                       const number & n, const number & f) {
            matrix_4x4<number> m{};
            number two = number(2);
            number one = number(1);
            // set OpenGL like perspective projection matrix columns, we assume, that
            // points that are projected with this matrix are right handed system.
            // they are on the negative z-axis, mapping is done to the cube [-1,1]x[-1,1]x[-1,1]
            m(0,0) = two*n/(r-l); m(0,1) = 0;             m(0,2) = (r+l)/(r-l);  m(0,3) = 0;
            m(1,0) = 0;           m(1,1) = two*n/(t-b);   m(1,2) = (t+b)/(t-b);  m(1,3) = 0;
            m(2,0) = 0;           m(2,1) = 0;             m(2,2) = -(f+n)/(f-n); m(2,3) = -(two*f*n)/(f-n);
            m(3,0) = 0;           m(3,1) = 0;             m(3,2) = -one;         m(3,3) = 0;

            return m;
        }

        /**
         * compute orthographic projection
         *
         * @tparam number number type
         *
         * @param l         left plane
         * @param r         right plane
         * @param b         bottom plane
         * @param t         top plane
         * @param n         near plane
         * @param f         far plane
         *
         * @return matrix_4x4<number> result
         */
        template <typename number> static
        matrix_4x4<number> orthographic(const number & l, const number & r,
                                        const number & b, const number & t,
                                        const number & n, const number & f) {
            // map [l,r]->[-1,1], [b,t]->[-1,1], [-n,-far]->[-1,1]
            // we assume the projection is to be used on right-handed system
            // observing the negative z axis
            matrix_4x4<number> m{};
            number two = number(2);
            number one = number(1);
            number O = number(0);
            // columns
            m(0, 0) = two/(r-l);    m(0, 1) = O;            m(0, 2) = O;            m(0, 3) = -(r+l)/(r-l);
            m(1, 0) = O;            m(1, 1) = two/(t-b);    m(1, 2) = O;            m(1, 3) = -(t+b)/(t-b);
            m(2, 0) = O;            m(2, 1) = O;            m(2, 2) = -two/(f-n);   m(2, 3) = -(f+n)/(f-n);
            m(3, 0) = O;            m(3, 1) = O;            m(3, 2) = O;            m(3, 3) = one;

            return m;
        }
    };
}