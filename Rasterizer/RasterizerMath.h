#pragma once
#include "Constants.h"

#include <algorithm>
#include <functional>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>



struct Point3D {
	float x, y,z;
	Point3D(glm::vec3 vec)
	{
		x = vec.x;
		y = vec.y;
		z = vec.z;
	}
};
struct ScreenCoord {
	int x, y;
};


struct Triangle {
	glm::vec4 Positions[3];
	glm::vec3 Colors[3];
	glm::vec3 Normals[3];

	Triangle() {

	}
	Triangle(const Triangle& other)
	{
		Positions[0] = other.Positions[0];
		Positions[1] = other.Positions[1];
		Positions[2] = other.Positions[2];

		Colors[0] = other.Colors[0];
		Colors[1] = other.Colors[1];
		Colors[2] = other.Colors[2];

		Normals[0] = other.Normals[0];
		Normals[1] = other.Normals[1];
		Normals[2] = other.Normals[2];
	}
	 
	void RandomizeColors() {

		Colors[0].r = ((float)rand()) / (float)RAND_MAX;
		Colors[0].g = ((float)rand()) / (float)RAND_MAX;
		Colors[0].b = ((float)rand()) / (float)RAND_MAX;
		//Colors[0].a = 1.f;

		Colors[1].r = ((float)rand()) / (float)RAND_MAX;
		Colors[1].g = ((float)rand()) / (float)RAND_MAX;
		Colors[1].b = ((float)rand()) / (float)RAND_MAX;
		//Colors[1].a = 1.f;

		Colors[2].r = ((float)rand()) / (float)RAND_MAX;
		Colors[2].g = ((float)rand()) / (float)RAND_MAX;
		Colors[2].b = ((float)rand()) / (float)RAND_MAX;
		//Colors[2].a = 1.f;
	}

	__forceinline Triangle GetMultipliedByMatrix(glm::mat4 matrix)
	{
		Triangle newtri(*this);
		for (int i = 0; i < 3; i++)
		{
			newtri.Positions[i] = matrix * Positions[i];//glm::vec4(Positions[i],1.0f);
		}
		return newtri;
	}
};



//float orient2d(const Point3D& a, const Point3D& b, const ScreenCoord& c);



struct FramebufferTile;
void drawTri_Tile(const Triangle& Tri, FramebufferTile * Tile);