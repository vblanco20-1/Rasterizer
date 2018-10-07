#pragma once
#include "Constants.h"
#include <algorithm>
#include <functional>
#include <glm/vec3.hpp>

struct Point3D {
	float x, y,z;
};




float orient2d(const Point3D& a, const Point3D& b, const Point3D& c)
{
	float p = (b.x - a.x)*(c.y - a.y);
	float z = (b.y - a.y)*(c.x - a.x);
	return p - z;
}


void drawTri(const Point3D& v0, const Point3D& v1, const Point3D& v2, std::function<void(Point3D, float, float, float)> putpixel)
{
	// Compute triangle bounding box
	int minX = std::min(v0.x, std::min(v1.x, v2.x));
	int minY = std::min(v0.y, std::min(v1.y, v2.y));
	int maxX = std::max(v0.x, std::max(v1.x, v2.x));
	int maxY = std::max(v0.y, std::max(v1.y, v2.y));

	// Clip against screen bounds
	minX = std::max(minX, 0);
	minY = std::max(minY, 0);
	maxX = std::min(maxX, ScreenWidth - 1);
	maxY = std::min(maxY, ScreenHeight - 1);

	// Rasterize
	Point3D p;

	
	for (p.y = minY; p.y <= maxY; p.y++) {
		for (p.x = minX; p.x <= maxX; p.x++) {
			// Determine barycentric coordinates
			const float w0 = orient2d(v1, v2, p);
			const float w1 = orient2d(v2, v0, p);
			const float w2 = orient2d(v0, v1, p);
			float area = /*0.5 */(w0+ w1+ w2);

			// If p is on or inside all edges, render pixel.
			if (w0 >= 0 && w1 >= 0 && w2 >= 0)
				putpixel(p, w0/ area, w1/ area, w2/ area);
		}
	}
}