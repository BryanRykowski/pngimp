#pragma once

#include <fstream>
#include <cstdint>
#include <exception>
#include <vector>

namespace pngimp
{
	class BufferStruct
	{
	private:
		const char* _data;
		int _width;
		int _height;
	public:
		BufferStruct(const char* data, const int width, const int height);
		~BufferStruct();
		const char* data();
		const int width();
		const int height();
	};
	
	BufferStruct import(const char* path);
}

#ifdef PNGIMP_IMPL

namespace pngimp
{
	struct PNG_8byte
	{
		char bytes[8] = {0};
		PNG_8byte() {};
		PNG_8byte(char b0, char b1, char b2, char b3, char b4, char b5, char b6, char b7)
		{
			bytes[0] = b0;
			bytes[1] = b1;
			bytes[2] = b2;
			bytes[3] = b3;
			bytes[4] = b4;
			bytes[5] = b5;
			bytes[6] = b6;
			bytes[7] = b7;
		}
	};

	struct PNG_4byte
	{
		char bytes[4] = {0};
		PNG_4byte() {};
		PNG_4byte(char b0, char b1, char b2, char b3)
		{
			bytes[0] = b0;
			bytes[1] = b1;
			bytes[2] = b2;
			bytes[3] = b3;
		}
	};

	struct PNG_IHDR
	{
		unsigned int width;
		unsigned int height;
		char bit_depth;
		char color_type;
		char compression;
		char filter;
		char interlace;
	};

	bool equal(const PNG_8byte &a, const PNG_8byte &b)
	{
		bool eq = true;

		for (char i = 0; i < 8; i++)
		{
			if (a.bytes[i] != b.bytes[i])
			{
				eq = false;
			}
		}

		return eq;
	}

	bool equal(const PNG_4byte& a, const PNG_4byte& b)
	{
		bool eq = true;

		for (char i = 0; i < 4; i++)
		{
			if (a.bytes[i] != b.bytes[i])
			{
				eq = false;
			}
		}

		return eq;
	}

	unsigned int toUint(const PNG_4byte& a)
	{
		unsigned int b0 = (unsigned int)(unsigned char)(a.bytes[3]);
		unsigned int b1 = ((unsigned int)(unsigned char)a.bytes[2] << 8);
		unsigned int b2 = ((unsigned int)(unsigned char)a.bytes[1] << 16);
		unsigned int b3 = ((unsigned int)(unsigned char)a.bytes[0] << 24);

		return b0 | b1 | b2 | b3;
	}

	class FileBuffer
	{
	public:
		unsigned int pos = 0;
		std::vector<char> bytes;
		FileBuffer(){};
		bool read4Bytes(PNG_4byte& out);
		bool read8Bytes(PNG_8byte& out);
		bool readChar(char& out);
	};
}

pngimp::BufferStruct::BufferStruct(const char* data, const int width, const int height)
{
	_data = data;
	_width = width;
	_height = height;
}

pngimp::BufferStruct::~BufferStruct()
{
	delete[] _data;
}

const char* pngimp::BufferStruct::data()
{
	return _data;
}

const int pngimp::BufferStruct::width()
{
	return _width;
}

const int pngimp::BufferStruct::height()
{
	return _height;
}

bool pngimp::FileBuffer::read4Bytes(PNG_4byte& out)
{
	for (unsigned int i = 0; i < 4; ++i)
	{
		if (pos < bytes.size())
		{
			out.bytes[i] = bytes[pos];
			++pos;
		}
		else
		{
			return false;
		}
	}

	return true;
}

bool pngimp::FileBuffer::read8Bytes(PNG_8byte& out)
{
	for (unsigned int i = 0; i < 8; ++i)
	{
		if (pos < bytes.size())
		{
			out.bytes[i] = bytes[pos];
			++pos;
		}
		else
		{
			return false;
		}
	}

	return true;
}

bool pngimp::FileBuffer::readChar(char& out)
{
	if (pos < bytes.size())
	{
		out = bytes[pos];
		++pos;
		return true;
	}
	else
	{
		return false;
	}
}

pngimp::BufferStruct pngimp::import(const char* path)
{
	FileBuffer file;

	{
		std::ifstream ifs(path, std::ios::in | std::ios::binary);
		char c;
		while (ifs.read(&c, 1))
		{
			file.bytes.push_back(c);
		}
	}

	// Validate signature
	PNG_8byte sig;
	file.read8Bytes(sig);

	if (!equal(sig, PNG_8byte(0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a)))
	{
		throw std::exception("");
	}

	PNG_IHDR ihdr;
	std::vector<char> chunk_data;
	std::vector<char> compressed_data;
	std::vector<char> filtered_data;
	std::vector<char> final_data;

	bool done = false;
	while (!done)
	{
		PNG_4byte chunk_size_raw;
		if (!file.read4Bytes(chunk_size_raw))
		{
			throw std::exception("");
		}

		unsigned int chunk_size = toUint(chunk_size_raw);

		PNG_4byte chunk_name_raw;
		if (!file.read4Bytes(chunk_name_raw))
		{
			throw std::exception("");
		}

		for (int i = 0; i < chunk_size; ++i)
		{
			char c;
			if (!file.readChar(c))
			{
				throw std::exception("");
			}
			chunk_data.push_back(c);
		}

		// Discard CRC for now
		PNG_4byte crc;
		if (!file.read4Bytes(crc))
		{
			throw std::exception("");
		}

		if (equal(chunk_name_raw, PNG_4byte('I', 'H', 'D', 'R')))
		{
			if (chunk_size != 13)
			{
				throw std::exception("");
			}

			ihdr.width = toUint(PNG_4byte(chunk_data[0], chunk_data[1], chunk_data[2], chunk_data[3]));
			ihdr.height = toUint(PNG_4byte(chunk_data[4], chunk_data[5], chunk_data[6], chunk_data[7]));
			ihdr.bit_depth = chunk_data[8];
			ihdr.color_type = chunk_data[9];
			ihdr.compression = chunk_data[10];
			ihdr.filter = chunk_data[11];
			ihdr.interlace = chunk_data[12];
		}
		else if (equal(chunk_name_raw, PNG_4byte('I', 'D', 'A', 'T')))
		{
			for (int i = 0; i < chunk_data.size(); ++i)
			{
				compressed_data.push_back(chunk_data[i]);
			}
		}
		else if (equal(chunk_name_raw, PNG_4byte('I', 'E', 'N', 'D')))
		{
			done = true;
		}

		chunk_data.clear();
	}

	return BufferStruct(0, 0, 0);
}

#endif