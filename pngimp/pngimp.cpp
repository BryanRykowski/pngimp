#include "pngimp.hpp"

namespace pngimp
{
	Image::Image(std::filesystem::path path)
	{

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
		return (char*)m_data.data();
	}
}

