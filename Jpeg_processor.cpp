#define _CRT_SECURE_NO_WARNINGS

#include "Jpeg_processor.h"

void Jpeg_processor::horizontal_mirror(const char *input_name, const char *output_name)
{
	std::vector<unsigned char> raw_image;

	jpeg_decompress_struct jpeg_input{};				//������, ����������� �� �������� �����

	read(input_name, &jpeg_input, &raw_image);

	std::vector<std::vector<unsigned char>> raw_matrix;		//�������� ������� ����������� (��� ������������ ��������������)
	raw_matrix.resize(jpeg_input.image_height);
	for(int i = 0; i < raw_matrix.size(); i++)
	{
		raw_matrix[i].resize(jpeg_input.image_width * jpeg_input.output_components);
	}
	
	for(int i = 0; i < raw_matrix.size(); i++)				//������ ����������� � �������.
	{
		for(int j = 0; j < raw_matrix[i].size(); j++)
		{
			raw_matrix[i][j] = raw_image[i * jpeg_input.image_width * jpeg_input.output_components + j];
		}
	}

	for(unsigned int i = 0; i < (raw_matrix.size() / 2); i++)												//��������������, ����� �������������� ������� �����
	{
		std::vector<unsigned char> temp = raw_matrix[i];
		raw_matrix[i] = raw_matrix[raw_matrix.size() - 1 - i];
		raw_matrix[jpeg_input.image_height - 1 - i] = temp;
	}

	jpeg_compress_struct  jpeg_output{};					//������ ��� ������ � �������� ����.

	jpeg_output.image_height = jpeg_input.image_height;		//�������� ���������� �����������.
	jpeg_output.image_width = jpeg_input.image_width;
	jpeg_output.input_components = jpeg_input.output_components;
	jpeg_output.in_color_space = jpeg_input.out_color_space;

	for(unsigned int i = 0; i < raw_matrix.size(); i++)											//���������� raw image ������������� ������������.
	{
		for(unsigned int j = 0; j < raw_matrix[i].size(); j++)
		{
			raw_image[i * jpeg_output.image_width * jpeg_output.input_components + j] = raw_matrix[i][j];
		}
	}


	write(output_name, &jpeg_output, &raw_image);
}

void Jpeg_processor::read(const char *input_name, jpeg_decompress_struct *jpeg_info, std::vector<unsigned char> *raw_image)
{
	FILE *input_file;												//��������� �� ���� ��������� �����������

	if((input_file = fopen(input_name, "rb")) == NULL)				//�������� �� ����������� ��� �������.
	{
		fprintf(stderr, "can't open %s\n", input_name);
		exit(1);
	}

	jpeg_error_mgr jpeg_err;					//�������� ������ ���������� libjpeg

	jpeg_info->err = jpeg_std_error(&jpeg_err);

	jpeg_create_decompress(jpeg_info);

	jpeg_stdio_src(jpeg_info, input_file);			//�������� ������ �� �����.

	jpeg_read_header(jpeg_info, TRUE);
	jpeg_start_decompress(jpeg_info);

	raw_image->resize(jpeg_info->image_width * jpeg_info->image_height * jpeg_info->output_components);

	unsigned char* scr = (unsigned char*)malloc(jpeg_info->image_width * jpeg_info->image_height * jpeg_info->output_components);

	JSAMPROW row_pointer[1];						//���������� ����� �����������.
	row_pointer[0] = (unsigned char *)malloc(jpeg_info->image_width * jpeg_info->output_components);		//��������� ������ (����� ����� ������ ������, ������� �������� ���� ������ �������� �����������.

	unsigned long location = 0;						//���������� ����� �������.

	while(jpeg_info->output_scanline < jpeg_info->image_height)			//������ ����������� � ������ (����� ���������� ���������)
	{
		jpeg_read_scanlines(jpeg_info, row_pointer, 1);
		for(unsigned int i = 0; i < jpeg_info->image_width * jpeg_info->output_components; i++)
		{
			scr[location++] = row_pointer[0][i];
		}
	}

	for(int i = 0; i < jpeg_info->image_width * jpeg_info->image_height * jpeg_info->output_components; i++)
	{
		(*raw_image)[i] = scr[i];
	}

	jpeg_finish_decompress(jpeg_info);				//��������� ��������� �����������.
	jpeg_destroy_decompress(jpeg_info);
	fclose(input_file);								//�������� �����.

	free(scr);
	free(row_pointer[0]);
}

void Jpeg_processor::write(const char *output_name, jpeg_compress_struct *jpeg_info, std::vector<unsigned char> *raw_image)
{
	FILE *output_file;												//��������� �� ���� ����������� �����������

	if((output_file = fopen(output_name, "wb")) == NULL)				//�������� �� ����������� ��� �������.
	{
		fprintf(stderr, "can't open %s\n", output_name);
		exit(1);
	}
	jpeg_error_mgr jpeg_err;					//�������� ������ ���������� libjpeg


	jpeg_compress_struct compress;				//������ ��� ������ � �������� ���� 
	//jpeg_info ������������ �������� ������, ��� ��� ��� ���������� jpeg_create_compress �������� ���������� � ������� ������.

	compress.err = jpeg_std_error(&jpeg_err);

	jpeg_create_compress(&compress);

	jpeg_stdio_dest(&compress, output_file);

	compress.image_width = jpeg_info->image_width;
	compress.image_height = jpeg_info->image_height;
	compress.input_components = jpeg_info->input_components;
	compress.in_color_space = jpeg_info->in_color_space;
	
	jpeg_set_defaults(&compress);
	jpeg_start_compress(&compress, TRUE);

	JSAMPROW row_pointer[1];																		//���������� ����� �����������.
	row_pointer[0] = (unsigned char *)malloc(compress.image_width * compress.input_components);		//��������� ������ (����� ����� ������ ������, ������� �������� ���� ������ �������� �����������.

	unsigned char *scr = (unsigned char *)malloc(compress.image_height * compress.image_width * compress.input_components);		//��������� ������� unsigned char � ������� ������ �� �������
	for(int i = 0; i < (compress.image_height * compress.image_width * compress.input_components); i++)							//���������� libjpeg ����� �������� � �������� �����������.
	{
		scr[i] = (*raw_image)[i];
	}

	while(compress.next_scanline < compress.image_height)												//���������� ������ ����������� � ����
	{
		row_pointer[0] = (JSAMPLE *)(scr + (compress.next_scanline * (compress.image_width * compress.input_components)));
		jpeg_write_scanlines(&compress, row_pointer, 1);
	}

	jpeg_finish_compress(&compress);
	jpeg_destroy_compress(&compress);
	fclose(output_file);								//�������� �����.

	free(scr);											//������������ ������.
}