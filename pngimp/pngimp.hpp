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

namespace pngimp
{
	class Image
	{
	private:
		int m_width = 0;
		int m_height = 0;
		std::unique_ptr<std::vector<unsigned char>> m_data;
	public:
		Image(std::vector<unsigned char>* data, int width, int height);
		Image();
		int width();
		int height();
		size_t size();
		unsigned char* data();
	};

	class ImageRGB8 : Image {};
	class ImageRGBA8 : Image {};

	ImageRGB8 OpenRGB8(const std::string& filepath);
	ImageRGBA8 OpenRGBA8(const std::string& filepath);
}
