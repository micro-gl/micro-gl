#pragma once

#include <microgl/matrix_4x4.h>
#include <microgl/math.h>
#include <microgl/vec3.h>


namespace microgl {
    template <typename number>
    class camera {
    private:
        using vertex = vec3<number>;
        using mat4 = matrix_4x4<number> ;
        vertex _coords;
        number _pitch, _yaw, _roll; // x,y,z
        mat4 _camera_matrix;
    public:

        camera() : _coords{}, _pitch{0}, _yaw{0}, _roll {0}, _camera_matrix{} {
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
        mat4& compute_world_to_camera_matrix() {
            // transpose 3x3 rotation matrix part
            // | R^T | 0 |
            // | ----+-- |
            // |  0  | 1 |
            // first compute the rotation matrix
            mat4 rotation = mat4::rotation(_pitch, _yaw, _roll);
            // transpose the 3x3 rotation block part fast
            mat4 rotation_transposed = rotation.fast_3x3_in_place_transpose(rotation);
            auto & mat = rotation_transposed;

            // compute translation part -R^T * T
            // | 0 | -R^T x |
            // | --+------- |
            // | 0 |   0    |
            number x = _coords.x;//mat[12];
            number y = _coords.y;//mat[13];
            number z = _coords.z;//mat[14];
            mat[12] = -(mat[0] * x + mat[4] * y + mat[8] * z);
            mat[13] = -(mat[1] * x + mat[5] * y + mat[9] * z);
            mat[14] = -(mat[2] * x + mat[6] * y + mat[10]* z);

            _camera_matrix=mat;
            return _camera_matrix;
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
        void lookAt(mat4 & result, const vertex & position, const vertex& target)
        {
            // remeber the camera posision & target position
//            this->position = position;
//            this->target = target;

            // if pos and target are same, only translate camera to position without rotation
            if(position==target) {
                result.identity();
                result.setColumn(3, -position);
                // rotation stuff
//                matrixRotation.identity();
//                angle.set(0,0,0);
//                quaternion.set(1,0,0,0);
                return;
            }

            // 3 axis of matrix for scene
            vertex left, up, forward;

            // first, compute the forward vector of rotation matrix
            // NOTE: the direction is reversed (target to camera pos) because of camera transform
            forward = position - target;
//            this->distance = forward.length();  // remember the distance
//             normalize
//            forward /= this->distance;

            forward.normalize();
            // compute temporal up vector based on the forward vector
            // watch out when look up/down at 90 degree
            // for example, forward vector is on the Y axis
            number epsilon = number(1)/number(10);
            if(microgl::math::abs_(forward.x) <= epsilon && fabs(forward.z) <= epsilon) {
                // forward vector is pointing +Y axis
                if (forward.y > 0)
                    up = vertex(0, 0, -1);
                else // forward vector is pointing -Y axis
                    up = vertex(0, 0, 1);
            } else // in general, up vector is straight up
                up = vertex(0, 1, 0);

            // compute the left vector of rotation matrix
            left = up.cross(forward).normalize();

            // re-calculate the orthonormal up vector
            up = forward.cross(left);

            // set inverse rotation matrix: M^-1 = M^T for Euclidean transform
//            matrixRotation.identity();
//            matrixRotation.setRow(0, left);
//            matrixRotation.setRow(1, up);
//            matrixRotation.setRow(2, forward);

            // copy it to matrix
            result.identity();
            result.setRow(0, left);
            result.setRow(1, up);
            result.setRow(2, forward);

            // set translation part
            vertex trans;
            trans.x = result[0]*-position.x + result[4]*-position.y + result[8]*-position.z;
            trans.y = result[1]*-position.x + result[5]*-position.y + result[9]*-position.z;
            trans.z = result[2]*-position.x + result[6]*-position.y + result[10]*-position.z;

            result.setColumn(3, trans);

//            // set Euler angles
//            angle = matrixToAngle(matrixRotation);
//
//            // set quaternion from angle
//            Vector3 reversedAngle(angle.x, -angle.y, angle.z);
//            quaternion = Quaternion::getQuaternion(reversedAngle * DEG2RAD * 0.5f); // half angle
//
//            //DEBUG
//            //std::cout << matrixRotation << std::endl;
        }

        //        template <typename number>
        static
        matrix_4x4<number> perspective(const number &fov_radians, const number & aspect_ratio,
                                       const number & near, const number & far) {
            matrix_4x4<number> mat{};

            auto scale = microgl::math::tan(fov_radians/number(2)) * near;
            auto r = aspect_ratio * scale, l = -r;
            auto t = scale, b = -t;

            return perspective(l, r, b, t, near, far);
        }

//        template <typename number>
        static
        matrix_4x4<number> perspective(const number &l, const number & r,
                                       const number & b, const number & t,
                                       const number & near, const number & far) {
            matrix_4x4<number> mat{};

            // set OpenGL perspective projection matrix
            mat(0, 0) = 2 * near / (r - l);
            mat(0, 1) = 0;
            mat(0, 2) = 0;
            mat(0, 3) = 0;

            mat(1, 0) = 0;
            mat(1, 1) = 2 * near / (t - b);
            mat(1, 2) = 0;
            mat(1, 3) = 0;

            mat(2, 0) = (r + l) / (r - l);
            mat(2, 1) = (t + b) / (t - b);
            mat(2, 2) = -(far + near) / (far - near);
            mat(2, 3) = -1;

            mat(3, 0) = 0;
            mat(3, 1) = 0;
            mat(3, 2) = -(2 * far * near) / (far - near);
            mat(3, 3) = 0;

            return mat;
        }

//        template <typename number>
        static
        matrix_4x4<number> orthographic(const number &l, const number & r,
                                       const number & b, const number & t,
                                       const number & near, const number & far) {
            matrix_4x4<number> mat{};

            mat(0, 0) = 2 / (r - l);
            mat(0, 1) = 0;
            mat(0, 2) = 0;
            mat(0, 3) = 0;

            mat(1, 0) = 0;
            mat(1, 1) = 2 / (t - b);
            mat(1, 2) = 0;
            mat(1, 3) = 0;

            mat(2, 0) = 0;
            mat(2, 1) = 0;
            mat(2, 2) = -2 / (far - near);
            mat(2, 3) = 0;

            mat(3, 0) = -(r + l) / (r - l);
            mat(3, 1) = -(t + b) / (t - b);
            mat(3, 2) = -(far + near) / (far - near);
            mat(3, 3) = 0;

            return mat;
        }

        /*
        ///////////////////////////////////////////////////////////////////////////////
// set a orthographic frustum with 6 params similar to glOrtho()
// (left, right, bottom, top, near, far)
///////////////////////////////////////////////////////////////////////////////
        Matrix4 setOrthoFrustum(float l, float r, float b, float t, float n, float f)
        {
            Matrix4 mat;
            mat[0]  =  2 / (r - l);
            mat[5]  =  2 / (t - b);
            mat[10] = -2 / (f - n);
            mat[12] = -(r + l) / (r - l);
            mat[13] = -(t + b) / (t - b);
            mat[14] = -(f + n) / (f - n);
            return mat;
        }


        ///////////////////////////////////////////////////////////////////////////////
// set a perspective frustum with 6 params similar to glFrustum()
// (left, right, bottom, top, near, far)
///////////////////////////////////////////////////////////////////////////////
        Matrix4 setFrustum(float l, float r, float b, float t, float n, float f)
        {
            Matrix4 mat;
            mat[0]  =  2 * n / (r - l);
            mat[5]  =  2 * n / (t - b);
            mat[8]  =  (r + l) / (r - l);
            mat[9]  =  (t + b) / (t - b);
            mat[10] = -(f + n) / (f - n);
            mat[11] = -1;
            mat[14] = -(2 * f * n) / (f - n);
            mat[15] =  0;
            return mat;
        }



///////////////////////////////////////////////////////////////////////////////
// set a symmetric perspective frustum with 4 params similar to gluPerspective
// (vertical field of view, aspect ratio, near, far)
///////////////////////////////////////////////////////////////////////////////
        Matrix4 setFrustum(float fovY, float aspectRatio, float front, float back)
        {
            float tangent = tanf(fovY/2 * DEG2RAD);   // tangent of half fovY
            float height = front * tangent;           // half height of near plane
            float width = height * aspectRatio;       // half width of near plane

            // params: left, right, bottom, top, near, far
            return setFrustum(-width, width, -height, height, front, back);
        }




*/
    };
}