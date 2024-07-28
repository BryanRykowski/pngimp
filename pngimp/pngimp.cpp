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
	Image::Image(){}

	Image::Image(SmartBuffer data, const ImageInfo& info) : m_data(std::move(data))
	{
		m_width = info.width;
		m_height = info.height;
		m_gamma = info.gamma;
		m_gamma_val = info.gamma_val;
		m_srgb = info.srgb;
		m_srgb_val = info.srgb_val;
	}

	unsigned int Image::width()
	{
		return m_width;
	}

	unsigned int Image::height()
	{
		return m_height;
	}
	unsigned int Image::gamma_val()
	{
		return m_gamma_val;
	}
	
	bool Image::gamma()
	{
		return m_gamma;
	}
	
	ImageInfo::SRGBIntent Image::srgb_val()
	{
		return m_srgb_val;
	}
	
	bool Image::srgb()
	{
		return m_srgb;
	}
	
	size_t Image::size()
	{
		return m_data->size();
	}
	
	char* Image::data()
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

	ImageInfo ReadHeader(std::ifstream& stream)
	{
		char buffer[25];

		try
		{
			stream.read(buffer, 25);
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

		ImageInfo info;
		info.width = ReadUint32(&buffer[8]);
		info.height = ReadUint32(&buffer[12]);
		memcpy(&info.bit_depth, &buffer[16], 1);
		memcpy(&info.color_type, &buffer[17], 1);
		memcpy(&info.interlaced, &buffer[20], 1);

		return info;
	}

	void ValidateInfo(const ImageInfo& info)
	{
		// 0 is not a valid PNG width or height.

		if (info.width ==  0 || info.height == 0) throw(MalformedFile(MalformedFile::Cause::BadDimension));

		// Make sure the image has a valid combination of color format and bit depth.
		// This doesn't mean we can open it, just that it's a valid PNG.

		switch (info.color_type)
		{
		case 0:
			switch (info.bit_depth)
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
			switch (info.bit_depth)
			{
			case 8:
			case 16:
				break;
			default:
				throw(MalformedFile(MalformedFile::Cause::BadBitDepth));
			}
			break;
		case 3:
			switch (info.bit_depth)
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

		// Now that we know it's a valid PNG, reject it if it's not 8 bit RGB or RGBA.

		if (info.bit_depth != 8) throw(UnsupportedOption(UnsupportedOption::Cause::BitDepth));

		switch (info.color_type)
		{
		case 2:
		case 6:
			break;
		default:
			throw(UnsupportedOption(UnsupportedOption::Cause::ColorType));
		}

		// Valid interlace values are 0 (not interlaced) and 1 (Adam-7).

		switch (info.interlaced)
		{
		case 0:
		case 1:
			break;
		default:
			throw(MalformedFile(MalformedFile::Cause::BadInterlace));
		}
	}

	SmartBuffer ReadChunks(ImageInfo& info, std::ifstream& stream)
	{
		SmartBuffer outbuffer = std::make_unique<std::vector<char>>();
		char chunkhdr[8];	
		unsigned int length = 0;
		unsigned int offset = 0;

		while (true)
		{
			try
			{
				stream.read(chunkhdr, 8);
			}
			catch (...)
			{
				throw(StreamFail(StreamFail::Cause::ReadFail));
			}

			length = ReadUint32(chunkhdr);

			if (memcmp(&chunkhdr[4], "IEND", 4) == 0)
			{
				break;
			}
			else if (memcmp(&chunkhdr[4], "IDAT", 4) == 0)
			{
				// Concatenate compressed image data to buffer.

				outbuffer->resize(outbuffer->size() + length);

				try
				{
					stream.read(reinterpret_cast<char*>(outbuffer->data() + offset), length);
				}
				catch (...)
				{
					throw(StreamFail(StreamFail::Cause::ReadFail));
				}

				offset = outbuffer->size();
			}
			else if (memcmp(&chunkhdr[4], "gAMA", 4) == 0)
			{
				if (length != 4) throw(MalformedFile(MalformedFile::Cause::BadChunkSize));
				info.gamma = true;
				char gamma_buffer[4];
				
				try
				{
					stream.read(gamma_buffer, 4);
				}
				catch (...)
				{
					throw(StreamFail(StreamFail::Cause::ReadFail));
				}

				info.gamma_val = ReadUint32(gamma_buffer);
			}
			else if (memcmp(&chunkhdr[4], "sRGB", 4) == 0)
			{
				if (length != 4) throw(MalformedFile(MalformedFile::Cause::BadChunkSize));
				info.srgb = true;
				char srgb_buffer[4];
				
				try
				{
					stream.read(srgb_buffer, 4);
				}
				catch (...)
				{
					throw(StreamFail(StreamFail::Cause::ReadFail));
				}

				switch (ReadUint32(srgb_buffer))
				{
				case 0:
					info.srgb_val = ImageInfo::SRGBIntent::Perceptual;
					break;
				case 1:
					info.srgb_val = ImageInfo::SRGBIntent::Relative;
					break;
				case 2:
					info.srgb_val = ImageInfo::SRGBIntent::Saturation;
					break;
				case 3:
					info.srgb_val = ImageInfo::SRGBIntent::Absolute;
				}
			}
			else
			{
				// Ignore all other blocks.

				try
				{
					stream.ignore(length);
				}
				catch (...)
				{
					throw(StreamFail(StreamFail::Cause::ReadFail));
				}
			}

			// Skip CRC for now.

			try
			{
				stream.ignore(4);
			}
			catch (...)
			{
				throw(StreamFail(StreamFail::Cause::ReadFail));
			}
		}

		return outbuffer;
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
