#pragma once
#include <filesystem>
#include <vector>

namespace pngimp
{
	class PngimpException : public std::exception
	{
		virtual const char* what()
		{
			return "pngimp exception";
		}
	};

	class InvalidState : PngimpException
	{
		virtual const char* what()
		{
			return "image already opened or no image opened";
		}
	};

	class BadSignature : PngimpException
	{
		virtual const char* what()
		{
			return "invalid signature";
		}
	};

	class BadHeader : PngimpException
	{
		virtual const char* what()
		{
			return "invalid header";
		}
	};

	class UnsupportedFormat : PngimpException
	{
		virtual const char* what()
		{
			return "unsupported color type or bit depth";
		}
	};

	class ZFail : PngimpException
	{
		virtual const char* what()
		{
			return "zlib inflate operation failed";
		}
	};

	class CorruptFile : PngimpException
	{
		virtual const char* what()
		{
			return "a value was wrong or a checksum did not match";
		}
	};

	class Image
	{
		std::vector<unsigned char> m_data;
		int m_width = 0;
		int m_height = 0;

	public:
		Image();
		Image(std::filesystem::path path);
		void Open(std::filesystem::path);
		int width();
		int height();
		char* data();
	};
}
