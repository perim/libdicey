#include <assert.h>
#include "dice.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "stb_image_write.h"

static void write_screenshot(const char* filename, int w, int h, const uint8_t* mono)
{
        int r = stbi_write_png(filename, w, h, 1, mono, 0);
	assert(r != 0);
}

/// Visually test randomness generation
int main()
{
	const int size = 400;
	seed s(1111);
	uint8_t* pixels = new uint8_t[size * size];

	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			pixels[i * size + j] = s.roll(0, 255);
		}
	}
	write_screenshot("test_base.png", size, size, pixels);

	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			pixels[i * size + j] = s.derive(j * size + i).roll(0, 255);
		}
	}
	write_screenshot("test_x.png", size, size, pixels);

	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			pixels[i * size + j] = s.derive(i, j).roll(0, 255);
		}
	}
	write_screenshot("test_xy.png", size, size, pixels);

	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			pixels[i * size + j] = s.derive(i, i, j).roll(0, 255);
		}
	}
	write_screenshot("test_xyz.png", size, size, pixels);

	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			pixels[i * size + j] = s.derive(i, j, i, j).roll(0, 255);
		}
	}
	write_screenshot("test_xyzw.png", size, size, pixels);

	delete [] pixels;
	return 0;
}
