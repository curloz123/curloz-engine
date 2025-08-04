#pragma once
#include "precision.h"
#include "../glm/glm.hpp"
namespace physics
{
    class Vector3
    {
        public:
            real x;
            real y;
            real z;
        
        private:
            // real pad;

        public:
            //Defining Constructors
            Vector3(): x(0), y(0), z(0){}
            Vector3(real x, real y, real z):x(x), y(y), z(z){}

            //Defining all our arithmetic operations
            void operator*=(const real value)
            {
                x *= value;
                y *= value;
                z *= value;
            }
            Vector3 operator*(const real value) const
            {
                return Vector3(x*value, y*value, z*value);
            }

            void operator+=(const Vector3& v)
            {
                x += v.x;
                y += v.y;
                z += v.z;
            }
            Vector3 operator+(const Vector3& v) const
            {
                return Vector3(x+v.x, y+v.y, z+v.z);
            }

            void operator-=(const Vector3& v) 
            {
                x -= v.x;
                y -= v.y;
                z -= v.z;
            }
            Vector3 operator-(const Vector3& v) const
            {
                return Vector3(x-v.x, y-v.y, z-v.z);
            }



            void addScaledVector(const Vector3& vector, real scale)
            {
                x += vector.x * scale;
                y += vector.y * scale;
                z += vector.z * scale; 
            }
            
            //Invert our current vector
            void invert()
            {
                x = -x;
                y = -y;
                z = -z;
            }

            //Return the magnitude
            real magnitude() const
            {
                return real_sqrt(x*x+ y*y+ z*z);
            }
            real squareMagnitude() const
            {
                return x*x+ y*y+ z*z;
            }
            //Normalize our vector
            void normalize()
            {
                real length = magnitude();
                if(length > 0)
                {
                    (*this) *= ((real)1)/length;
                }
            }

            Vector3 componentProduct(const Vector3& vector) const
            {
                return Vector3(x*vector.x, y*vector.y, z*vector.z);
            }

            //Scalar product
            
            //This called like real x = a.scalarProduct(b);
            real scalarProduct(const Vector3 & vector) const
            {
                return x*vector.x + y*vector.y + z*vector.z;
            }
            //This is called like real x = a*b;
            real operator*(const Vector3& vector) const
            {
                return x*vector.x + y*vector.y + z*vector.z;
            }

            //Vector product

            Vector3 vectorProduct(const Vector3& vector) const
            {
                return Vector3(y*vector.z - z*vector.y,
                               z*vector.x - x*vector.z,
                               x*vector.y - y*vector.x);
            }
            void operator%=(const Vector3& vector)
            {
                *this = vectorProduct(vector);
            }
            Vector3 operator%(const Vector3& vector)
            {
                return Vector3(y*vector.z - z*vector.y,
                               z*vector.x - x*vector.z,
                               x*vector.y - y*vector.x);
            }

            void clear()
            {
                x = (real)0;
                y = (real)0;
                z = (real)0;
            }

    };
    
}
