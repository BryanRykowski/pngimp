#include "pngimp.hpp"
#include <cstdint>
#include <fstream>
#include <cstring>
#include <zlib.h>

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

void ReadChunks(ImageInfo& info, std::ifstream& stream, std::vector<char>& buffer)
{
	buffer.clear();
	stream.exceptions(std::ios::failbit);

	char chunkheader[8];

	while (!stream.eof())
	{
		stream.read(chunkheader, 8);

		if (std::memcmp(&chunkheader[4], "IEND", 4) == 0)
		{
			break;
		}
		else if (std::memcmp(&chunkheader[4], "IDAT", 4) == 0)
		{
			uint_least32_t chunksize = ReadU32(chunkheader);
			size_t offset = buffer.size();
			buffer.resize(buffer.size() + chunksize);
			stream.read(&buffer[offset], chunksize);
		}
		else if (std::memcmp(&chunkheader[4], "gAMA", 4) == 0)
		{
			char gamma[4];
			stream.read(gamma, 4);
			info.gamma_val = ReadU32(gamma);
			info.gamma = true;
		}
		else if (std::memcmp(&chunkheader[4], "sRGB", 4) == 0)
		{
			char intent;
			stream.read(&intent, 1);
			info.srgb_intent = static_cast<uint_least8_t>(intent);
			info.srgb = true;
		}
		else
		{
			// Ignore all other chunks.
			stream.ignore(ReadU32(chunkheader));
		}

		stream.ignore(4); // Ignore CRC for now.
	}
}

void InflateData(std::vector<char>& in, std::vector<char>& out)
{
#if defined (PNGIMP_ZCHUNK_SIZE)
	constexpr size_t zchunksize = PNGIMP_ZCHUNK_SIZE;
#else
	constexpr size_t zchunksize = 262144;
#endif

	out.clear();

	// Get the adler32 appended to the end of the Zlib stream.
	uint_least32_t adler = ReadU32(&in[in.size() - 4]);

	// Initialize the Zlib stream struct.
	z_stream stream;
	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;
	int ret = inflateInit(&stream);
	if (ret != Z_OK) throw (pngimp::ZFail());

	// Set Zlib to read from the input buffer.
	stream.avail_in = in.size();
	stream.next_in = (unsigned char*)in.data();

	do
	{
		// Decompress the input buffer, writing [zchunksize] bytes to the output buffer at a time.
		size_t size = out.size();
		out.resize(out.size() + zchunksize);
		stream.next_out = (unsigned char*)&out[size];
		stream.avail_out = zchunksize;
		ret = inflate(&stream, Z_NO_FLUSH);

		if (ret == Z_STREAM_END)
		{
			break;
		}
		else if (ret != Z_OK)
		{
			inflateEnd(&stream);
			throw (pngimp::ZFail());
		}
	}
	while (stream.avail_out == 0);

	// Make sure the adler32 of the inflated data matches the value appended to the stream.
	if (stream.adler != adler) throw (pngimp::CorruptFile());

	// Shrink the output buffer to the actual data written to it.
	out.resize(stream.total_out);
	
	// Clean up the Zlib state.
	inflateEnd(&stream);
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

		std::vector<char> buffer_a;
		ReadChunks(info, stream, buffer_a);
		std::vector<char> buffer_b;
		InflateData(buffer_a, buffer_b);
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

