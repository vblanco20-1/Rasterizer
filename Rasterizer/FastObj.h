#pragma once
#include <vector>
struct ObjFile {
	
	struct VertexPos {
		float x, y, z;
	};
	struct VertexNormal {
		float x, y, z;
	};
	struct VertexUV {
		float u, v;
	};
	struct Face {
		int position;
		int uv;
		int normal;
	};


	void load_file(const char* filename);
	void add_line(char* line);
	std::vector<VertexUV> uvs;
	std::vector<VertexPos> positions;
	std::vector<VertexNormal> normals;
	std::vector<Face> faces;	
};