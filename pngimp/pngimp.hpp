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

// pngimp.hpp

#pragma once
#include <cstddef>
#include <vector>
#include <memory>
#include <exception>

namespace pngimp
{
	class MalformedFile : public std::exception
	{
	public:
		enum class Cause
		{
			BadSignature,		// The first 8 bytes of the file are wrong
			BadHeader,			// The IHDR size is not 13 or bytes 4-7 are not "IHDR"
			BadDimension,		// The height and/or width is 0
			BadBitDepth,		// Invalid bit depth for a given color type
			BadColorType,		// Invalid color type (not 1,2,4,8,16)
			BadCompression,		// Compression is not 0
			BadFilter,			// Filter is not 0
			BadInterlace,		// Interlace is not 0 or 1
			BadChunkSize		// A gAMA or sRGB chunk is the wrong size
		} cause;

		virtual const char* what()
		{
			return "The file has a non-conforming attribute";
		}

		MalformedFile(Cause c) : cause{c} {}
	};

	class StreamFail : public std::exception
	{
	public:
		enum class Cause
		{
			OpenFail,			// The file does not exist or permissions are wrong
			ReadFail			// The file stopped being available or ended unexpectedly
		} cause;

		virtual const char* what()
		{
			return "There was an error accessing the file";
		}

		StreamFail(Cause c) : cause{c} {}
	};

	class UnsupportedOption : public std::exception
	{
	public:
		enum class Cause
		{
			ColorType,			// The color is not type 2 (RGB) or type 6 (RGBA)
			BitDepth			// The bit depth is not 8
		} cause;

		virtual const char* what()
		{
			return "The color type or bit depth of the image are not supported by this decoder";
		}

		UnsupportedOption(Cause c) : cause{c} {}
	};

	typedef std::unique_ptr<std::vector<char>> SmartBuffer;

	struct ImageInfo
	{
		unsigned int width = 0;
		unsigned int height = 0;
		unsigned char bit_depth = 0;
		unsigned char color_type = 0;
		unsigned char interlaced = 0;
		unsigned int gamma_val = 0;
		bool srgb = false;
		bool gamma = false;
		
		enum class SRGBIntent
		{
			Perceptual,
			Relative,
			Saturation,
			Absolute
		} srgb_val;
	};

	class Image
	{
	private:
		SmartBuffer m_data;
		unsigned int m_width = 0;
		unsigned int m_height = 0;
		unsigned int m_gamma_val = 0;
		bool m_srgb = false;
		bool m_gamma = false;
		ImageInfo::SRGBIntent m_srgb_val;
	public:
		Image(SmartBuffer data, const ImageInfo& info);
		Image();
		unsigned int width();
		unsigned int height();
		unsigned int gamma_val();
		bool gamma();
		ImageInfo::SRGBIntent srgb_val();
		bool srgb();
		size_t size();
		char* data();
	};

	class ImageRGB8 : public Image
	{
		using Image::Image;
	};

	class ImageRGBA8 : public Image
	{
		using Image::Image;
	};

	ImageRGB8 OpenRGB8(const std::string& filepath);
	ImageRGBA8 OpenRGBA8(const std::string& filepath);
}
