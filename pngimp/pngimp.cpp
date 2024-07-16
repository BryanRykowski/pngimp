// Copyright (c) 2022-2024 Bryan Rykowski
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this softwareand associated documentation files
// (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge,
// publish, distribute, sublicense, and /or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, 
// subject to the following conditions :
//
// The above copyright noticeand this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE 
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF 
// CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
// DEALINGS IN THE SOFTWARE.

// pngimp.cpp

#include "pngimp.hpp"
#include <fstream>
#include <cstring>

namespace pngimp
{
	struct Header
	{
		unsigned int width = 0;
		unsigned int height = 0;
		unsigned char bit_depth = 0;
		unsigned char color_type = 0;
		bool interlaced = false;
	};

	Image::Image(){}

	Image::Image(std::vector<unsigned char>* data, int width, int height)
	{
		m_data.reset(data);
		m_width = width;
		m_height = height;
	}

	int Image::width()
	{
		return m_width;
	}

	int Image::height()
	{
		return m_height;
	}
	
	size_t Image::size()
	{
		return m_data->size();
	}
	
	unsigned char* Image::data()
	{
		return m_data->data();
	}

	void ReadSignature(std::ifstream& stream)
	{
		const char control[8] = {char(0x89), 'P', 'N', 'G', char(0x0d), char(0x0a), char(0x1a), char(0x0a)};
		char buffer[8];

		try
		{
			stream.read(buffer, 8);
		}
		catch(...)
		{
			throw(StreamFail(StreamFail::Cause::ReadFail));
		}

		if (std::memcmp(control, buffer, 8))
		{
			throw(MalformedFile(MalformedFile::Cause::BadSignature));
		}
	}

	unsigned int ReadUint32(char* data)
	{
		unsigned int i = 0;

		i |= (static_cast<unsigned char>(data[0]) << 24);
		i |= (static_cast<unsigned char>(data[1]) << 16);
		i |= (static_cast<unsigned char>(data[2]) << 8);
		i |= static_cast<unsigned char>(data[3]);

		return i;
	}

	Header ReadHeader(std::ifstream& stream)
	{
		char buffer[21];

		try
		{
			stream.read(buffer, 21);
		}
		catch(...)
		{
			throw(StreamFail(StreamFail::Cause::ReadFail));
		}

		// The IHDR chunk should always start with a u32 equaling 13 (the length of the chunk) and four chars "IHDR".

		if (ReadUint32(&buffer[0]) != 13) throw(MalformedFile(MalformedFile::Cause::BadHeader));

		const char control[4] = {'I', 'H', 'D', 'R'};

		if (memcmp(control, &buffer[4], 4)) throw(MalformedFile(MalformedFile::Cause::BadHeader));

		// The compression and filter types of a PNG must always be 0.

		if (buffer[18]) throw(MalformedFile(MalformedFile::Cause::BadCompression));
		if (buffer[19]) throw(MalformedFile(MalformedFile::Cause::BadFilter));

		Header hdr;
		hdr.width = ReadUint32(&buffer[8]);
		hdr.height = ReadUint32(&buffer[12]);
		memcpy(&hdr.bit_depth, &buffer[16], 1);
		memcpy(&hdr.color_type, &buffer[17], 1);

		// 0 is not a valid PNG width or height.

		if (hdr.width ==  0 || hdr.height == 0) throw(MalformedFile(MalformedFile::Cause::BadDimension));

		// Make sure the image has a valid combination of color format and bit depth.
		// This doesn't mean we can open it, just that it's a valid PNG.

		switch (hdr.color_type)
		{
		case 0:
			switch (hdr.bit_depth)
			{
			case 1:
			case 2:
			case 4:
			case 8:
			case 16:
				break;
			default:
				throw(MalformedFile(MalformedFile::Cause::BadBitDepth));
			}
			break;
		case 2:
		case 4:
		case 6:
			switch (hdr.bit_depth)
			{
			case 8:
			case 16:
				break;
			default:
				throw(MalformedFile(MalformedFile::Cause::BadBitDepth));
			}
			break;
		case 3:
			switch (hdr.bit_depth)
			{
			case 1:
			case 2:
			case 4:
			case 8:
				break;
			default:
				throw(MalformedFile(MalformedFile::Cause::BadBitDepth));
			}
			break;
		default:
			throw(MalformedFile(MalformedFile::Cause::BadColorType));
		}

		// Valid interlace values are 0 (not interlaced) and 1 (Adam-7).

		if (buffer[20] == 0)
		{
			hdr.interlaced = false;
		}
		else if (buffer[20] == 1)
		{
			hdr.interlaced = true;
		}
		else 
		{
			throw(MalformedFile(MalformedFile::Cause::BadInterlace));
		}

		// Now that we know it's a valid PNG, reject it if it's not 8 bit RGB or RGBA.

		if (hdr.bit_depth != 8) throw(UnsupportedOption(UnsupportedOption::Cause::BitDepth));

		switch (hdr.color_type)
		{
		case 2:
		case 6:
			break;
		default:
			throw(UnsupportedOption(UnsupportedOption::Cause::ColorType));
		}

		return hdr;
	}

	ImageRGB8 OpenRGB8(const std::string& filepath)
	{
		return ImageRGB8();
	}

	ImageRGBA8 OpenRGBA8(const std::string& filepath)
	{
		return ImageRGBA8();
	}
}
