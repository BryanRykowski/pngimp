#pragma once
#include <filesystem>
#include <vector>

namespace pngimp
{
	class Image
	{
		std::vector<unsigned char> m_data;
		int m_width = 0;
		int m_height = 0;

		Image(std::filesystem::path path);
		int width();
		int height();
		char* data();
	};
}
