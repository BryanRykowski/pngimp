#pragma once

#include <fstream>
#include <cstdint>
#include <exception>
#include <vector>
#include <array>
#include <memory>

namespace pngimp
{
	struct PNG_IHDR
	{
		unsigned int width;
		unsigned int height;
		unsigned char bit_depth;
		unsigned char color_type;
		unsigned char compression;
		unsigned char filter;
		unsigned char interlace;
	};

	enum BufferFormat : char
	{
		RGB8 = 0,
		RGBA8
	};
	
	class BufferStruct
	{
	private:
		std::vector<unsigned char> _data;
		int _width;
		int _height;
		BufferFormat _format;
	public:
		BufferStruct(PNG_IHDR& ihdr, std::vector<unsigned char>& data);
		const unsigned char* data();
		const int width();
		const int height();
		BufferFormat format();
	};
}

pngimp::BufferStruct::BufferStruct(PNG_IHDR& ihdr, std::vector<unsigned char>& data)
{
	std::copy(data.begin(), data.end(), std::back_inserter(_data));
	_width = ihdr.width;
	_height = ihdr.height;

	if (ihdr.color_type == 2)
	{
		_format = BufferFormat::RGB8;
	}
	else
	{
		_format = BufferFormat::RGBA8;
	}
}

const unsigned char* pngimp::BufferStruct::data()
{
	return _data.data();
}

const int pngimp::BufferStruct::width()
{
	return _width;
}

const int pngimp::BufferStruct::height()
{
	return _height;
}
pngimp::BufferFormat pngimp::BufferStruct::format()
{
	return _format;
}

namespace pngimp
{
	BufferStruct import(const char* path);
}

#ifdef PNGIMP_IMPL

namespace pngimp
{
	typedef std::array<unsigned char, 4> ChunkName;
	typedef std::array<unsigned char, 8> Signature;

	bool equal(const Signature& a, const Signature& b)
	{
		bool eq = true;

		for (char i = 0; i < 8; i++)
		{
			if (a[i] != b[i])
			{
				eq = false;
			}
		}

		return eq;
	}

	bool equal(const ChunkName& a, const ChunkName& b)
	{
		bool eq = true;

		for (char i = 0; i < 4; i++)
		{
			if (a[i] != b[i])
			{
				eq = false;
			}
		}

		return eq;
	}
}

namespace pngimp
{
	constexpr size_t FileReaderBuffSize = 1024 * 4;
	
	class FileReader
	{
	private:
		std::unique_ptr<std::array<unsigned char, FileReaderBuffSize>> buffer;
		std::ifstream stream;
	public:
		FileReader(const char* path);
		bool readUchar(unsigned char& out);
		bool readUint(unsigned int& out);
		bool readChunkName(ChunkName& out);
		bool readSignature(Signature& out);
		bool readNbytes(std::vector<unsigned char>& destination, size_t count);
		bool skipNbytes(size_t count);
	};
}

pngimp::FileReader::FileReader(const char* path)
{
	buffer = std::make_unique<std::array<unsigned char, FileReaderBuffSize>>();
	stream.open(path, std::ios::in | std::ios::binary);
}

bool pngimp::FileReader::readUchar(unsigned char& out)
{
	char c;
	if (stream.read(&c, 1))
	{
		out = (unsigned char)c;
		return true;
	}
	else
	{
		return false;
	}
}

bool pngimp::FileReader::readUint(unsigned int& out)
{
	char i[4];
	if (stream.read(i, 4))
	{
		out = (unsigned int)(unsigned char)i[0] << 24 | (unsigned int)(unsigned char)i[1] << 16 | (unsigned int)(unsigned char)i[2] << 8 | (unsigned int)(unsigned char)i[3];
		return true;
	}
	else
	{
		return false;
	}
}

bool pngimp::FileReader::readChunkName(ChunkName& out)
{
	if (stream.read((char*)out.data(), 4))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool pngimp::FileReader::readSignature(Signature& out)
{
	char s[8];
	if (stream.read(s, 8))
	{
		std::copy(std::begin(s), std::end(s), out.begin());
		return true;
	}
	else
	{
		return false;
	}
}

bool pngimp::FileReader::readNbytes(std::vector<unsigned char>& destination, size_t count)
{
	while (count > 0)
	{
		if (count > FileReaderBuffSize)
		{
			if (!stream.read((char*)buffer->data(), FileReaderBuffSize))
			{
				return false;
			}

			std::copy( buffer->begin(), buffer->end(), std::back_inserter(destination));
			count -= FileReaderBuffSize;
		}
		else
		{
			if (!stream.read((char*)buffer->data(), count))
			{
				return false;
			}

			std::copy(buffer->begin(), buffer->begin() + count, std::back_inserter(destination));
			count = 0;
		}
	}

	return true;
}

bool pngimp::FileReader::skipNbytes(size_t count)
{
	while (count > 0)
	{
		if (count > FileReaderBuffSize)
		{
			if (!stream.read((char*)buffer->data(), FileReaderBuffSize))
			{
				return false;
			}

			count -= FileReaderBuffSize;
		}
		else
		{
			if (!stream.read((char*)buffer->data(), count))
			{
				return false;
			}

			count = 0;
		}
	}

	return true;
}

namespace pngimp
{
	void deinterlace(PNG_IHDR& ihdr, std::vector<unsigned char>& interlaced_data, std::vector<unsigned char>& final_data)
	{

	}
	
	void unfilter( PNG_IHDR& ihdr,std::vector<unsigned char>& filtered_data, std::vector<unsigned char>& interlaced_data)
	{

	}
	
	void inflate(PNG_IHDR& ihdr,const std::vector<unsigned char>& in, std::vector<unsigned char>& out)
	{
		size_t width = static_cast<size_t>(ihdr.width) + 1;
		size_t height = static_cast<size_t>(ihdr.height);
		if (ihdr.color_type == 2)
		{
			out.reserve(width * height * 3);
		}
		else
		{
			out.reserve(width * height * 4);
		}

		struct
		{
			unsigned char cm;
			unsigned char cinfo;
			bool fdict;
		}zhdr;
		
		zhdr.cm = in[0] & 0x0f;
		zhdr.cinfo = in[0] >> 4;

		zhdr.fdict = (in[1] & 0b00100000) >> 5;

		unsigned short fcheck_verify = (unsigned short)in[0] << 8 | (unsigned short)in[1];

		if (
			fcheck_verify % 31 != 0 ||
			zhdr.cm != 8 ||
			zhdr.cinfo > 7
			)
		{
			throw std::exception("");
		}

		size_t in_pos;
		
		if (zhdr.fdict)
		{
			in_pos = 6;
		}
		else
		{
			in_pos = 2;
		}

		while (in_pos < in.size())
		{
			++in_pos;
		}
	}
}

pngimp::BufferStruct pngimp::import(const char* path)
{
	FileReader file(path);

	// Validate signature
	Signature sig;
	file.readSignature(sig);

	if (!equal(sig, Signature{ 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a }))
	{
		throw std::exception();
	}

	PNG_IHDR ihdr;
	std::vector<unsigned char> buffer0;
	std::vector<unsigned char> buffer1;
	int chunk_count = 0;
	int IDAT_count = 0;

	bool done = false;
	while (!done)
	{
		unsigned int chunk_size;
		if (!file.readUint(chunk_size))
		{
			throw std::exception();
		}

		ChunkName chunk_name;
		if (!file.readChunkName(chunk_name))
		{
			throw std::exception();
		}

		if (equal(chunk_name, ChunkName{ 'I', 'H', 'D', 'R' }))
		{
			if (chunk_size != 13)
			{
				throw std::exception();
			}

			file.readUint(ihdr.width);
			file.readUint(ihdr.height);
			file.readUchar(ihdr.bit_depth);
			file.readUchar(ihdr.color_type);
			file.readUchar(ihdr.compression);
			file.readUchar(ihdr.filter);
			file.readUchar(ihdr.interlace);

			// Verify image is compatible. Only 8 bits per sample RGB or RGBA.
			if (
				ihdr.bit_depth != 8 ||
				!(ihdr.color_type == 2 || ihdr.color_type == 6) ||
				ihdr.compression != 0 ||
				ihdr.filter != 0 ||
				!(ihdr.interlace == 0 || ihdr.interlace == 1)
				)
			{
				throw std::exception("");
			}
		}
		else if (equal(chunk_name, ChunkName{ 'I', 'D', 'A', 'T' }))
		{
			if (!file.readNbytes(buffer0, chunk_size))
			{
				throw std::exception();
			}
			++IDAT_count;
		}
		else if (equal(chunk_name, ChunkName{ 'I', 'E', 'N', 'D' }))
		{
			done = true;
		}
		else
		{
			file.skipNbytes(chunk_size);
		}
		
		// Discard CRC for now
		unsigned int crc;
		if (!file.readUint(crc))
		{
			throw std::exception();
		}

		++chunk_count;
	}

	inflate(ihdr, buffer0, buffer1);

	buffer0.clear();

	unfilter(ihdr, buffer1, buffer0);

	std::vector<unsigned char>& outBuffer = buffer0;
	
	if (ihdr.interlace == 1)
	{
		buffer1.clear();
		deinterlace(ihdr, buffer0, buffer1);
		outBuffer = buffer1;
	}

	return BufferStruct(ihdr, outBuffer);
}
#endif