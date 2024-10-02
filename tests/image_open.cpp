#include <cstdio>
#include <cstring>
#include <fstream>
#include "pngimp.hpp"

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::fprintf(stderr, "less than 1 arg\n");
		return 1;
	}

	std::vector<char> raw;

	if (argc == 3)
	{
		std::ifstream raw_stream;
		raw_stream.exceptions(std::ios::failbit);
		raw_stream.open(argv[2], std::ios::binary);

		raw_stream.seekg(0, std::ios::end);
		raw.resize(raw_stream.tellg());
		raw_stream.clear();
		raw_stream.seekg(0, std::ios::beg);

		raw_stream.read(raw.data(), raw.size());
	}

	pngimp::Image image;

	try
	{
		image.Open(argv[1]);

		if (argc == 3)
		{
			int image_size = image.width() * image.height() * 4;
			if (image_size != raw.size())
			{
				std::printf("image size differs. should be %zd, not %d\n", raw.size(), image_size);
				return 1;
			}
			
			if (std::memcmp(image.data(), raw.data(), image.width() * image.height() * 4))
			{
				std::printf("image data differs\n");
				return 1;
			}
		}
	}
	catch (pngimp::BadHeader& e)
	{
		std::printf("BadHeader thrown\n");
		return 1;
	}
	catch (pngimp::BadSignature& e)
	{
		std::printf("BadSignature thrown\n");
		return 1;
	}
	catch (pngimp::InvalidState& e)
	{
		std::printf("InvalidState thrown\n");
		return 1;
	}
	catch (pngimp::UnsupportedFormat& e)
	{
		std::printf("UnsupportedFormat thrown\n");
		return 1;
	}

	return 0;
}
