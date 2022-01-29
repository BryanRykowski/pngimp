#pragma once

#include <fstream>
#include <cstdint>
#include <exception>
#include <vector>
#include <array>
#include <memory>

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

namespace pngimp
{
	BufferStruct import(const char* path);
}

#ifdef PNGIMP_IMPL

namespace pngimp
{
	struct PNG_8byte
	{
		char bytes[8] = { 0 };
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
		char bytes[4] = { 0 };
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
		unsigned int b = (unsigned int)(unsigned char)(a.bytes[3]);
		b |= ((unsigned int)(unsigned char)a.bytes[2] << 8);
		b |= ((unsigned int)(unsigned char)a.bytes[1] << 16);
		b |= ((unsigned int)(unsigned char)a.bytes[0] << 24);

		return b;
	}
}

namespace pngimp
{
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

namespace pngimp
{
	void deinterlace(PNG_IHDR& ihdr, std::vector<char>& interlaced_data, std::vector<char>& final_data)
	{

	}
	
	void unfilter( PNG_IHDR& ihdr,std::vector<char>& filtered_data, std::vector<char>& interlaced_data)
	{

	}
	
	void inflate(std::vector<char>& compressed_data, std::vector<char>& filtered_data)
	{

	}
}

pngimp::BufferStruct pngimp::import(const char* path)
{
	FileBuffer file;
	
	{
		constexpr int buffSize = 1024 * 32;
		auto buff = std::make_unique<std::array<char, buffSize>>();
		
		std::ifstream ifs(path, std::ios::in | std::ios::binary);
		
		bool done = false;
		while (!done)
		{
			if (ifs.read(buff->data(), buffSize))
			{
				file.bytes.insert(file.bytes.end(), buff->begin(), buff->end());
			}
			else
			{
				if (ifs.gcount() > 0)
				{
					file.bytes.insert(file.bytes.end(), buff->data(), buff->data() + ifs.gcount());
				}
				done = true;
			}
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
	std::vector<char> buffer0;
	std::vector<char> buffer1;
	int chunk_count = 0;
	int IDAT_count = 0;

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

		if (equal(chunk_name_raw, PNG_4byte('I', 'H', 'D', 'R')))
		{
			if (chunk_size != 13)
			{
				throw std::exception("");
			}

			std::vector<char> temp_data = { file.bytes.begin() + file.pos, file.bytes.begin() + file.pos + chunk_size };
			file.pos += chunk_size;

			ihdr.width = toUint(PNG_4byte(temp_data[0], temp_data[1], temp_data[2], temp_data[3]));
			ihdr.height = toUint(PNG_4byte(temp_data[4], temp_data[5], temp_data[6], temp_data[7]));
			ihdr.bit_depth = temp_data[8];
			ihdr.color_type = temp_data[9];
			ihdr.compression = temp_data[10];
			ihdr.filter = temp_data[11];
			ihdr.interlace = temp_data[12];
		}
		else if (equal(chunk_name_raw, PNG_4byte('I', 'D', 'A', 'T')))
		{
			std::copy(file.bytes.begin() + file.pos, file.bytes.begin() + file.pos + chunk_size, std::back_inserter(buffer0));
			file.pos += chunk_size;
			++IDAT_count;
		}
		else if (equal(chunk_name_raw, PNG_4byte('I', 'E', 'N', 'D')))
		{
			done = true;
		}
		else
		{
			file.pos += chunk_size;
		}
		
		// Discard CRC for now
		PNG_4byte crc;
		if (!file.read4Bytes(crc))
		{
			throw std::exception("");
		}

		++chunk_count;
	}

	file.bytes.clear();
	file.bytes.shrink_to_fit();

	inflate(buffer0, buffer1);

	buffer0.clear();

	unfilter(ihdr, buffer1, buffer0);

	buffer1.clear();

	deinterlace(ihdr, buffer0, buffer1);

	buffer0.clear();

	return BufferStruct(0, 0, 0);
}
#endif