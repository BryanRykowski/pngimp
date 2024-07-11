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

namespace pngimp
{
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

	ImageRGB8 OpenRGB8(const std::string& filepath)
	{
		return ImageRGB8();
	}

	ImageRGBA8 OpenRGBA8(const std::string& filepath)
	{
		return ImageRGBA8();
	}
}
