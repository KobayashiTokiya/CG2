#pragma once
#include "../../externals/DirectXTex/DirectXTex.h"

struct Vector4
{
    float x;
    float y;
    float z;
    float w;
};

struct Vector3 {
    float x;
    float y;
    float z;

    //足し算
    Vector3 operator+(const Vector3& other)const
    {
        return{ x + other.x, y + other.y, z + other.z };
    }

    //引き算
    Vector3 operator-(const Vector3& other)const
    {
        return{ x - other.x, y - other.y, z - other.z };
    }

    //掛け算(Vector3同士)
    Vector3 operator*(const Vector3& other)const
    {
        return{ x * other.x, y * other.y, z * other.z };
	}

    //掛け算(Vector3とfloat)
    Vector3 operator*(float scalar)const
    {
        return{ x * scalar, y * scalar, z * scalar };
	}

    //割り算(Vector3同士)
    Vector3 operator/(const Vector3& other)const
    {
        return{ x / other.x, y / other.y, z / other.z };
    }

	//割り算(Vector3とfloat)
    Vector3 operator/(float scalar)const
    {
        if (scalar!=0.0f)
        {
            return{ x / scalar,y / scalar,z / scalar };
        }
		return *this;
    }

    // 加算代入 (+=)
    Vector3& operator+=(const Vector3& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    // 減算代入 (-=)
    Vector3& operator-=(const Vector3& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }
};

struct  Vector2 {
    float x;
    float y;
};