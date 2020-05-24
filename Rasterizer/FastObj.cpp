#include "FastObj.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <chrono>
#include <charconv>
#include <execution>	
#include <algorithm>
#include "Tracy.hpp"

__forceinline float fast_atof(const char* p) {
	float r = 0.0;
	bool neg = false;
	if (*p == '-') {
		neg = true;
		++p;
	}
	while (*p >= '0' && *p <= '9') {
		r = (r * 10.0) + (*p - '0');
		++p;
	}
	if (*p == '.') {
		double f = 0.0;
		int n = 0;
		++p;
		while (*p >= '0' && *p <= '9') {
			f = (f * 10.0) + (*p - '0');
			++p;
			++n;
		}
		r += f / std::pow(10.0, n);
	}
	if (neg) {
		r = -r;
	}
	return r;
}
#if 1
__forceinline float parse_float(char* str, int& cursor)
{
	char* start = &str[cursor];
	//char writebuffer[30];

	//int wb = 0;

	//stream and copy the float to writebuffer
	while (str[cursor] != 0 && str[cursor] != ' ' && str[cursor] != '\n' && str[cursor] != '\r')

	{
		//writebuffer[wb] = str[cursor];
		cursor++;
		//wb++;
	}
	//null terminate
	str[cursor] = 0;

	float result  = fast_atof(start);
	//recover
	str[cursor] =' ';

	//return atof(&writebuffer[0]);
	return result;

}
#else
float parse_float(const char* str, int& cursor)
{
	char writebuffer[30];

	int wb = 0;

	//stream and copy the float to writebuffer
	while (str[cursor] != ' ')
	{
		writebuffer[wb] = str[cursor];
		cursor++;
		wb++;
	}
	//null terminate
	writebuffer[wb] = 0;


	//return atof(&writebuffer[0]);
	return fast_atof(&writebuffer[0]);

}
#endif

__forceinline int fast_atoi(const char* str, const char* end)
{
	int val = 0;
	while (str != end) {
		val = val * 10 + (*str++ - '0');
	}
	return val;
}

__forceinline int parse_int(char* str, int& cursor)
{
	char* start = &str[cursor];
	int wb = 0;

	while (str[cursor] != 0 && str[cursor] != ' ' && str[cursor] != '/' && str[cursor] != '\n' && str[cursor] != '\r')
	{		
		cursor++;
		wb++;
	}


	const char lt = str[cursor];
	str[cursor] = 0;
	int result = fast_atoi(start, &str[cursor]);
	str[cursor] = lt;
	
	return result;

}
/*__forceinline */void add_vertex(char* line, std::vector<ObjFile::VertexPos>& outVerts)
{
	//skip v space
	int cursor = 2;

	float x = parse_float(line, cursor);
	cursor++;
	float y = parse_float(line, cursor);
	cursor++;
	float z = parse_float(line, cursor);
	
	ObjFile::VertexPos newV{ x,y,z };
	outVerts.push_back(newV);
}
__forceinline void add_normal(char* line, std::vector<ObjFile::VertexNormal>& outNormals)
{
	//skip vn space
	int cursor = 3;

	float x = parse_float(line, cursor);
	cursor++;
	float y = parse_float(line, cursor);
	cursor++;
	float z = parse_float(line, cursor);

	ObjFile::VertexNormal newV{ x,y,z };
	outNormals.push_back(newV);
}
__forceinline void add_uv(char* line, std::vector<ObjFile::VertexUV>& outUVs)
{
	//skip vn space
	int cursor = 3;

	float x = parse_float(line, cursor);
	cursor++;
	float y = parse_float(line, cursor);


	ObjFile::VertexUV newV{ x,y};
	outUVs.push_back(newV);
}
__forceinline void add_face(char* line, std::vector<ObjFile::Face>& outFaces)
{
	//skip v space
	int cursor = 2;

	for (int i = 0; i < 3; i++) {
		int p = 0;
		int uv = 0;
		int n = 0;
		ObjFile::Face newFace;

		p = parse_int(line, cursor);

		cursor++;
		//check if cursor is slash, as itmeans sksip
		if (line[cursor] != '/')
		{
			uv = parse_int(line, cursor);
		}
		cursor++;

		n = parse_int(line, cursor);
		cursor++;
		newFace.normal = n;
		newFace.position = p;
		newFace.uv = uv;

		outFaces.push_back(newFace);
	}
}

struct alignas(128) LoaderWorkerData  {

	size_t buffer_start;
	size_t buffer_end;
	const char* filename;

	void load();
	void new_line(char* line)
	{
		if (line[0] == 'v')
		{
			if (line[1] == ' ')
			{
				add_vertex(line, positions);
			}
			else if (line[1] == 'n')
			{
				add_normal(line, normals);
			}
			else if (line[1] == 't')
			{
				add_uv(line, uvs);
			}
		}
		else if (line[0] == 'f')
		{
			add_face(line, faces);
		}

	}

	std::vector<ObjFile::VertexUV> uvs;
	std::vector<ObjFile::VertexPos> positions;
	std::vector<ObjFile::VertexNormal> normals;
	std::vector<ObjFile::Face> faces;
};

void LoaderWorkerData::load()
{
	size_t megab = 1048576;
	size_t sizechnk = megab * sizeof(char);
	char *chunk = new char [megab];
	char *linebuffer = new char[200];

	std::ifstream ifs(filename, std::ios_base::binary);
	if (!ifs) {
		std::cerr << "Error opeing " << filename << std::endl;
		exit(1);
	}

	positions.reserve(10000);
	normals.reserve(10000);
	faces.reserve(10000);
	uvs.reserve(10000);

	unsigned long long res = 0;
	ifs.seekg(buffer_start);

	int64_t work_left = buffer_end - buffer_start;

	int cursor = 0;
	int bfsize = sizechnk;
	if (!ifs.read(chunk, megab))
	{
		bfsize = ifs.gcount();
		
	}
	else {
		bfsize = ifs.gcount();
	}
	work_left -= bfsize;
	while (true)
	{
		int wb = 0;

		if (cursor > bfsize)
		{
			if (work_left <= 0)
			{
				return;
			}
			int read_amount = std::min((int64_t)work_left, (int64_t)megab);
			if (!ifs.read(chunk, read_amount))
			{
				bfsize = ifs.gcount();
				//EOF
				if (bfsize == 0)break;
			}
			else {
				bfsize = ifs.gcount();
			}
			work_left -= bfsize;
			
			cursor = 0;
		}

		//stream and copy the float to writebuffer
		while (chunk[cursor] != '\n' && chunk[cursor] != '\r' && cursor < bfsize)
		{
			linebuffer[wb] = chunk[cursor];
			cursor++;
			wb++;
		}
		linebuffer[wb] = 0;
		cursor++;
		new_line(linebuffer);
	}

	delete[] chunk;
	delete[] linebuffer;
}



int load_all(ObjFile& obfile, const char* filename)
{

	std::vector<LoaderWorkerData> workers;
	
	char* buffer = new char[1000];

	std::ifstream ifs(filename, std::ios_base::binary);
	if (!ifs) {
		std::cerr << "Error opeing " << filename << std::endl;
		exit(1);
	}

	//file lenght
	ifs.seekg(0, ifs.end);
	long long int length = ifs.tellg();
	ifs.seekg(0, ifs.beg);

	std::vector<size_t> cut_points;
	
	const int cut_amount =32;
	//divide the lenght in chunks
	
	for (int i = 0; i < cut_amount; i++)
	{
		cut_points.push_back((length / cut_amount) * i);
	}
	cut_points.push_back(length+1);

	{
		ZoneScopedN("Build Units");
		//find the newline at each point, index 0 and last not touched
		for (int i = 1; i < cut_amount; i++)
		{
			ZoneScopedN("Find cut");;
			size_t point = cut_points[i];

			ifs.seekg(point, ifs.beg);

			ifs.read(buffer, sizeof(char) * 1000);

			int c = 0;
			//find a newline
			while (buffer[c] != '\n' && buffer[c] != '\r')
			{
				c++;
			}
			//one more afterthe endlne
			c++;
			point += c;
			//this puts the cut-point at the first letter of a new line
			cut_points[i] = point;
		}

		for (int i = 0; i < cut_amount; i++)
		{

			LoaderWorkerData worker;
			worker.buffer_start = cut_points[i];
			worker.buffer_end = cut_points[i + 1];
			worker.filename = filename;
			workers.push_back(worker);
		}
	}

	std::for_each(std::execution::par, workers.begin(), workers.end(), [](LoaderWorkerData& worker) {

		ZoneScopedN("Work unit");
		worker.load();
	});

	size_t facecount = 0;
	size_t normalcount = 0;
	size_t uvcount = 0;
	size_t pcont = 0;

	for (auto& worker : workers)
	{
		facecount += worker.faces.size();
		normalcount += worker.normals.size();
		uvcount += worker.uvs.size();
		pcont += worker.positions.size();
	}

	{
		ZoneScopedN("Reserves");
		obfile.faces.reserve(facecount);
		obfile.normals.reserve(normalcount);
		obfile.positions.reserve(pcont);
		obfile.uvs.reserve(uvcount);
	}
	{
		ZoneScopedN("CopyBack");
		for (auto& worker : workers)
		{
			obfile.faces.insert(std::end(obfile.faces), std::begin(worker.faces), std::end(worker.faces));

			worker.faces.clear();

			obfile.normals.insert(std::end(obfile.normals), std::begin(worker.normals), std::end(worker.normals));

			worker.normals.clear();

			obfile.uvs.insert(std::end(obfile.uvs), std::begin(worker.uvs), std::end(worker.uvs));

			worker.uvs.clear();

			obfile.positions.insert(std::end(obfile.positions), std::begin(worker.positions), std::end(worker.positions));

			worker.positions.clear();
		}
	}
#if 0
	char* chunk = new char[1048576];
	char* linebuffer = new char[200];

	std::ifstream ifs(filename, std::ios_base::binary);
	if (!ifs) {
		std::cerr << "Error opeing " << filename << std::endl;
		exit(1);
	}


	
	unsigned long long res = 0;
	ifs.seekg(0);

	int cursor = 0;
	int bfsize = sizeof(chunk);
	if (!ifs.read(chunk, sizeof(chunk)))
	{
		bfsize = ifs.gcount();
	}
	while (true) 
	{
		int wb = 0;

		if (cursor > sizeof(chunk))
		{
			if (!ifs.read(chunk, sizeof(chunk)))
			{
				bfsize = ifs.gcount();
				if (bfsize == 0)break;
			}
			cursor = 0;
		}

		//stream and copy the float to writebuffer
		while (chunk[cursor] != '\n' && chunk[cursor] != '\r' && cursor < bfsize)
		{
			linebuffer[wb] = chunk[cursor];
			cursor++;
			wb++;
		}
		linebuffer[wb] = 0;
		cursor++;
		obfile.add_line(linebuffer);
	}
	delete[] chunk;
	delete[] linebuffer;
	
#endif 
	return 0;
}

void ObjFile::load_file(const char* filename)
{
	//char line[1024];

	positions.reserve(1000);
	normals.reserve(1000);
	faces.reserve(1000);
	uvs.reserve(1000);

#if 0



	std::ifstream ifs(filename, std::ios_base::binary);
	if (!ifs) {
		std::cout << "Error opeing " << filename << std::endl;
		return;
	}

	while (ifs.getline(&line[0], 1024))
	{
		add_line(line);
	}
#else
	load_all(*this, filename);
#endif
}

void ObjFile::add_line(char* line)
{
	if (line[0] == 'v')
	{
		if (line[1] == ' ')
		{
			add_vertex(line, positions);
		}
		else if (line[1] == 'n')
		{
			add_normal(line, normals);
		}
		else if (line[1] == 't')
		{
			add_uv(line, uvs);
		}
	}
	else if (line[0] == 'f')
	{
		add_face(line, faces);
	}
}
