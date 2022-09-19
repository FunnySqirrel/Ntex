#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <vector>

#include "jpeglib.h"


class Jpeg_processor
{
private:
	static void read(const char *input_name, jpeg_decompress_struct *jpeg_info, std::vector<unsigned char> *raw_image);		//������� ������ jpeg �����
	static void write(const char *output_name, jpeg_compress_struct *jpeg_info, std::vector<unsigned char> *raw_image);										//������� ������ jpeg �����
public:
	static void horizontal_mirror(const char *input_name, const char *output_name);	//������� �������������� �����������.
};

