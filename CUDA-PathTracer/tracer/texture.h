#pragma once
#include "common.h"
#include "..\tracer\imageio.h"

//using namespace std;

class Texture{
public:
	std::vector<uchar4> data;
	int width, height;

public:
	Texture(const char* file){
		std::vector<float4> image;
		ImageIO::LoadTexture(file, width, height, true, image);
		data.resize(width*height);
		for (int i = 0; i < width * height; ++i){
			float4 rgba = image[i];
			unsigned char r = unsigned char(rgba.x * 255);
			unsigned char g = unsigned char(rgba.y * 255);
			unsigned char b = unsigned char(rgba.z * 255);
			unsigned char a = unsigned char(rgba.w * 255);
			data[i] = make_uchar4(r, g, b, a);
		}
	}
};