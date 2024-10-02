#include "pngimp.hpp"
#include <cstdint>
#include <fstream>
#include <cstring>

struct ImageInfo
{
	uint_least32_t width;
	uint_least32_t height;
	uint_least32_t gamma_val;
	uint_least8_t color_type;
	uint_least8_t bit_depth;
	uint_least8_t srgb_intent;
	uint_least8_t interlaced;
	bool srgb = false;
	bool gamma = false;
};

uint_least32_t ReadU32(char* in)
{
	uint_least32_t i = 0;
	i |= (static_cast<unsigned char>(in[0]) << 24);
	i |= (static_cast<unsigned char>(in[1]) << 16);
	i |=  (static_cast<unsigned char>(in[2]) << 8);
	i |=         static_cast<unsigned char>(in[3]);
	return i;
}

void ReadHeader(ImageInfo& info, std::ifstream& stream)
{
	const char compare[8] = {char(0x89), 'P', 'N', 'G', char(0x0d), char(0x0a), char(0x1a), char(0x0a)};
	char buffer[33];

	stream.exceptions(std::ios::failbit | std::ios::eofbit);
	stream.read(buffer, 33);

	if (std::memcmp(compare, buffer, 8)) throw (pngimp::BadSignature());

	// The IHDR chunk should always start with a u32 equaling 13 (the length of the chunk) and four chars "IHDR".

	if (ReadU32(&buffer[8]) != 13) throw(pngimp::BadHeader());

	const char control[] = {'I', 'H', 'D', 'R'};

	if (std::memcmp(control, &buffer[12], 4)) throw(pngimp::BadHeader());

	// The compression and filter types of a PNG must always be 0.

	if (buffer[26]) throw(pngimp::BadHeader());
	if (buffer[27]) throw(pngimp::BadHeader());

	info.width = ReadU32(&buffer[16]);
	info.height = ReadU32(&buffer[20]);
	info.bit_depth = static_cast<uint_least8_t>(buffer[24]);
	info.color_type = static_cast<uint_least8_t>(buffer[25]);
	info.interlaced = static_cast<uint_least8_t>(buffer[28]);
}

namespace pngimp
{
	Image::Image() {}
	
	Image::Image(std::filesystem::path path)
	{
		Open(path);
	}

	void Image::Open(std::filesystem::path path)
	{
		std::ifstream stream;
		stream.exceptions(std::ios::failbit | std::ios::eofbit);
		stream.open(path);

		ImageInfo info;
		ReadHeader(info, stream);

		if ((info.color_type != 2) && (info.color_type != 6)) throw (UnsupportedFormat());
		if (info.bit_depth != 8) throw (UnsupportedFormat());

		m_width = info.width;
		m_height = info.height;
	}

	int Image::width()
	{
		return m_width;
	}

	int Image::height()
	{
		return m_height;
	}

	char* Image::data()
	{
		if (m_data.size() == 0) throw (InvalidState());
		return (char*)m_data.data();
	}
}

