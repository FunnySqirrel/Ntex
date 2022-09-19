#define _CRT_SECURE_NO_WARNINGS

#include "Jpeg_processor.h"

int main() 
{
	Jpeg_processor::horizontal_mirror("in.jpg", "out.jpg");
	return 0;
}