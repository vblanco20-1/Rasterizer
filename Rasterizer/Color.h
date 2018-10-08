#pragma once
#include <algorithm>
struct Color {

	char r;
	char g;
	char b;
	char a;

	char FloatToColor(float f)
	{
		f *= 256.0;
		f = std::min(256.f, f);
		f = std::max(0.f, f);
		return (char)f;
	}

	Color(char _r, char _g, char _b): r(_r),g(_g),b(_b),a((char)255){	}
	Color(float _r, float _g, float _b):a((char)255) {
		r = FloatToColor(_r);
		g = FloatToColor(_g);
		b = FloatToColor(_b);

	}
};