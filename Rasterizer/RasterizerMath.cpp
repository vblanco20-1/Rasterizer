#include "RasterizerMath.h"

#include "Screen.h"
#include "Globals.h"

#include <execution>

__forceinline float orient2d(const Point3D& a, const Point3D& b, const ScreenCoord& c)
{
	float p = (b.x - a.x)*(c.y - a.y);
	float z = (b.y - a.y)*(c.x - a.x);
	return p - z;
}



void drawTri_Tile(const Triangle& Tri, FramebufferTile * Tile)
{
	const Point3D v0 = Point3D(Tri.Positions[0]);
	const Point3D v1 = Point3D(Tri.Positions[1]);
	const Point3D v2 = Point3D(Tri.Positions[2]);


	// Compute triangle bounding box
	int minX = (int)std::min(Tri.Positions[0].x, std::min(Tri.Positions[1].x, Tri.Positions[2].x));
	int minY = (int)std::min(Tri.Positions[0].y, std::min(Tri.Positions[1].y, Tri.Positions[2].y));
	int maxX = (int)std::max(Tri.Positions[0].x, std::max(Tri.Positions[1].x, Tri.Positions[2].x));
	int maxY = (int)std::max(Tri.Positions[0].y, std::max(Tri.Positions[1].y, Tri.Positions[2].y));

	// Clip against tile bounds
	minX = std::max(minX, (int)Tile->minX);
	minY = std::max(minY, (int)Tile->minY);
	maxX = std::min(maxX, (int)Tile->maxX - 1);
	maxY = std::min(maxY, (int)Tile->maxY - 1);

	// Rasterize
	//ScreenCoord p;

	//const int dy = maxY - minY;
	
	
	for (short y = minY; y <= maxY; y++) {
		
		for (short x = minX; x <= maxX; x++) {
			ScreenCoord p;
			p.x = (float)x;
			p.y = (float)y;
		
			// Determine barycentric coordinates
			const float w0 = orient2d(v1, v2, p);
			const float w1 = orient2d(v2, v0, p);
			const float w2 = orient2d(v0, v1, p);
			const float area = /*0.5 */(w0 + w1 + w2);

			// If p is on or inside all edges, render pixel.
			if (w0 >= 0 && w1 >= 0 && w2 >= 0)
			{

				const float nw0 = w0 / area;
				const float nw1 = w1 / area;
				const float nw2 = w2 / area;
				const glm::vec3 color = nw0 * Tri.Normals[0] + nw1 * Tri.Normals[1] + nw2 * Tri.Normals[2];
				//color = Tri.Colors[0];//w0 * Tri.Colors[0] + w1 * Tri.Colors[1] + w2 * Tri.Colors[2];
				const float depth = 1.0 / (nw0 * Tri.Positions[0].z + nw1 * Tri.Positions[1].z + nw2 * Tri.Positions[2].z);
				
				//g_Framebuffer->SetPixel(p.x, p.y, Color(color.r, color.g, color.b), depth);
				Tile->SetPixel(p.x, p.y, Color(color.r, color.g, color.b), depth);
			}
				
		}
		
	}
}
