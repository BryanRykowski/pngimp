#pragma once

#include <fstream>
#include <exception>

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
		unsigned int b0 = (unsigned int)a.bytes[3];
		unsigned int b1 = ((unsigned int)a.bytes[2] << 8);
		unsigned int b2 = ((unsigned int)a.bytes[1] << 16);
		unsigned int b3 = ((unsigned int)a.bytes[0] << 24);

		return b0 | b1 | b2 | b3;
	}
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

pngimp::BufferStruct pngimp::import(const char* path)
{
	std::ifstream infile;
	infile >> std::noskipws;
	infile.open(path, std::ios::in | std::ios::binary);

	// Validate signature
	PNG_8byte sig;
	if (!infile.read(sig.bytes, 8))
	{
		throw std::exception("");
	}

	if (!equal(sig, PNG_8byte(0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a)))
	{
		throw std::exception("");
	}

	PNG_4byte chunk_size_raw;
	if (!infile.read(chunk_size_raw.bytes, 4))
	{
		throw std::exception("");
	}

	unsigned int chunk_size = toUint(chunk_size_raw);

	PNG_4byte chunk_name_raw;
	if (!infile.read(chunk_name_raw.bytes, 4))
	{
		throw std::exception("");
	}

	return BufferStruct(0, 0, 0);
}

#endif