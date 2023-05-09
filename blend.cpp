// g++ --std=c++0x blend.cpp -o blend -lpng

#include <png.h>
#include <zlib.h>

#include <algorithm>
#include <iostream>
#include <vector>

using namespace std;

uint32_t *Read(const char *path, int *width, int *height);
void Write(const char *path, uint32_t *buffer, int width, int height);



int main(int argc, char *argv[])
{
	if(argc != 4)
	{
		cout << "Usage: $ blend <opaque image> <additive image> <result image>" << endl;
		return 1;
	}

	int ow = 0, oh = 0;
	uint32_t *op = Read(argv[1], &ow, &oh);
	if(!op)
	{
		cerr << "Unable to read image: " << argv[1] << endl;
		return 1;
	}

	int aw = 0, ah = 0;
	uint32_t *ap = Read(argv[2], &aw, &ah);
	if(!op)
	{
		cerr << "Unable to read image: " << argv[2] << endl;
		delete [] op;
		return 1;
	}

	if(ow != aw || oh != ah)
	{
		cerr << "Images are different sizes: " << ow << "x" << oh << " versus "
			<< aw << "x" << ah << "." << endl;
		delete [] op;
		delete [] ap;
		return 1;
	}

	for(int y = 0; y < oh; ++y)
	{
		uint32_t *oit = op + y * ow;
		uint32_t *ait = ap + y * aw;

		for(uint32_t *oend = oit + ow; oit != oend; ++oit, ++ait)
		{
			uint64_t oA = (*oit >> 24) & 0xFF;
			uint64_t oR = (*oit >> 16) & 0xFF;
			uint64_t oG = (*oit >> 8) & 0xFF;
			uint64_t oB = (*oit >> 0) & 0xFF;

			uint64_t aA = (*ait >> 24) & 0xFF;
			uint64_t aR = (*ait >> 16) & 0xFF;
			uint64_t aG = (*ait >> 8) & 0xFF;
			uint64_t aB = (*ait >> 0) & 0xFF;

			oR = min(uint64_t(255), (oR * oA) / 255 + (aR * aA) / 255);
			oG = min(uint64_t(255), (oG * oA) / 255 + (aG * aA) / 255);
			oB = min(uint64_t(255), (oB * oA) / 255 + (aB * aA) / 255);

			*oit = static_cast<uint32_t>((oA << 24) + (oR << 16) + (oG << 8) + (oB << 0));
		}
	}

	Write(argv[3], op, ow, oh);

	return 0;
}



uint32_t *Read(const char *path, int *width, int *height)
{
	FILE *file = fopen(path, "rb");
	if(!file)
		return nullptr;

	// Set up libpng.
	png_struct *png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if(!png)
		return nullptr;

	png_info *info = png_create_info_struct(png);
	if(!info)
	{
		png_destroy_read_struct(&png, nullptr, nullptr);
		return nullptr;
	}

	uint32_t *buffer = nullptr;
	if(setjmp(png_jmpbuf(png)))
	{
		png_destroy_read_struct(&png, &info, nullptr);
		delete buffer;
		return nullptr;
	}

	png_init_io(png, file);
	png_set_sig_bytes(png, 0);

	png_read_info(png, info);
	*width = png_get_image_width(png, info);
	*height = png_get_image_height(png, info);
	if(!*width || !*height)
		return nullptr;

	// Adjust settings to make sure the result will be a BGRA file.
	int colorType = png_get_color_type(png, info);
	int bitDepth = png_get_bit_depth(png, info);

	png_set_strip_16(png);
	png_set_packing(png);
	if(colorType == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png);
	if(colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8)
		png_set_expand_gray_1_2_4_to_8(png);
	if(colorType & PNG_COLOR_MASK_COLOR)
		png_set_bgr(png);
	png_read_update_info(png, info);

	// Read the file.
	buffer = new uint32_t[*width * *height];
	vector<png_byte *> rows;
	for(int y = 0; y < *height; ++y)
		rows.push_back(reinterpret_cast<png_byte *>(buffer + y * *width));

	png_read_image(png, &rows.front());

	// Clean up.
	png_destroy_read_struct(&png, &info, nullptr);
	fclose(file);

	return buffer;
}



void Write(const char *path, uint32_t *buffer, int width, int height)
{
	// Set up libpng.
	png_struct *png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if(!png)
		return;

	png_info *info = png_create_info_struct(png);
	if(!info)
	{
		png_destroy_read_struct(&png, nullptr, nullptr);
		return;
	}

	if(setjmp(png_jmpbuf(png)))
	{
		png_destroy_read_struct(&png, &info, nullptr);
		return;
	}

	FILE *file = fopen(path, "wb");
	png_init_io(png, file);
	png_set_compression_level(png, Z_BEST_COMPRESSION);

	png_set_IHDR(png, info, width, height, 8,
		PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	png_set_bgr(png);
	png_write_info(png, info);

	vector<png_byte *> rows;
	for(int y = 0; y < height; ++y)
		rows.push_back(reinterpret_cast<png_byte *>(buffer + y * width));

	png_write_image(png, &rows.front());
	png_write_end(png, NULL);

	// Clean up.
	png_destroy_write_struct(&png, &info);
	fclose(file);
}
