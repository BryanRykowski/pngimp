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

	enum ErrorType : int
	{
		NoError,
		FilePathNull,
		FileNotExist,
		FileNotPNG,
		UnsupportedFormat,
		FileDataCorrupt
	};

	enum ColorType : unsigned char
	{
		GRAY = 0,
		RGB = 2,
		PALLETE = 3,
		GRAYALPHA = 4,
		RGBA = 6
	};
	
	struct ImageStruct
	{
		unsigned char* bytes;
		size_t nBytes;
		unsigned int width;
		unsigned int height;
		unsigned char bitDepth;
		ColorType colorType;
	};
	
	bool Import(const char* path, ErrorType& errorType, ImageStruct& imageStruct);
	
#ifndef PNGIMP_NOEXCEPT

	class FilePathNull : std::exception
	{

	};

	class FileNotExist : std::exception
	{

	};

	class FileNotPNG : std::exception
	{

	};

	class UnsupportedFormat : std::exception
	{

	};

	class FileDataCorrupt : std::exception
	{

	};
	
	class Image
	{
	private:
		std::vector<unsigned char> p_bytes;
		unsigned int p_width;
		unsigned int p_height;
		unsigned char p_bitDepth;
		ColorType p_colorType;
	public:
		Image(const char* path);
		const unsigned char* Bytes();
		const size_t nBytes();
		const unsigned int Width();
		const unsigned int Height();
		const unsigned char BitDepth();
		const ColorType ColorType();
	};
}
#endif

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
	// Read file in 4KB chunks.
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
	// Allocate buffer and open stream to file.
	buffer = std::make_unique<std::array<unsigned char, FileReaderBuffSize>>();
	stream.open(path, std::ios::in | std::ios::binary);
}

// Read a single byte from the file.
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

// Read 4 bytes from the file. Interpret them as a big endian unsigned integer.
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

// Read 4 bytes from the file. Interpret them as a ChunkName (array of 4 chars).
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

// Read 8 bytes from the file. Interpret them as a Signature (array of 8 chars).
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

// Read an arbitrary number of bytes from the file.
bool pngimp::FileReader::readNbytes(std::vector<unsigned char>& destination, size_t count)
{
	while (count > 0)
	{
		if (count > FileReaderBuffSize)
		{
			// Read a buffer's worth of data and append it to the output vector.
			if (!stream.read((char*)buffer->data(), FileReaderBuffSize))
			{
				return false;
			}

			std::copy( buffer->begin(), buffer->end(), std::back_inserter(destination));
			count -= FileReaderBuffSize;
		}
		else
		{
			// Read the remaining data into the buffer and append that amount to the output vector.
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

// Read and discard an arbitrary number of bytes from the file.
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
	bool deinterlace(PNG_IHDR& ihdr, std::vector<unsigned char>& interlaced_data, std::vector<unsigned char>& final_data)
	{
		return false;
	}
	
	bool unfilter( PNG_IHDR& ihdr,std::vector<unsigned char>& filtered_data, std::vector<unsigned char>& interlaced_data)
	{
		return false;
	}
	
	bool inflate(PNG_IHDR& ihdr,const std::vector<unsigned char>& in, std::vector<unsigned char>& out)
	{
		// Pre-allocate the output buffer to the appropriate size depending on whether the color format is RGB or RGBA.
		// Each scanline is one byte wider since the buffer will be filled with filtered data. The first byte of each
		// scanline indicates the filter method used.
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
		
		// Bitwise operations to extract the compression method, compression info, and dictionary flag from the zlib header.
		zhdr.cm = in[0] & 0x0f;
		zhdr.cinfo = in[0] >> 4;

		zhdr.fdict = (in[1] & 0b00100000) >> 5;

		// fcheck_verify is built from the first 2 bytes of the compressed data block, interpreted as an unsigned 16 bit
		// big endian integer. This value should be a multiple of 31 if these bytes are intact.
		unsigned short fcheck_verify = (unsigned short)in[0] << 8 | (unsigned short)in[1];

		if (
			fcheck_verify % 31 != 0 ||
			zhdr.cm != 8 ||
			zhdr.cinfo > 7
			)
		{
			// Throw error
			return false;
		}

		size_t in_pos;
		
		// Skip over dictionary if present.
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

		return false;
	}

	bool read(const char* path, ErrorType& errorType, PNG_IHDR& ihdr, std::vector<unsigned char>& bytes)
	{
		FileReader file(path);

		// Validate signature
		Signature sig;
		file.readSignature(sig);

		if (!equal(sig, Signature{ 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a }))
		{
			errorType = ErrorType::FileNotPNG;
			return false;
		}

		// Buffers to ping-pong data back and forth between.
		// Likely procedure is:
		// 					[compressed data]	-> buffer0
		//		buffer0 ->	[decompress]		-> buffer1
		//		buffer1 ->	[unfilter]			-> buffer0
		// 		buffer0 ->	[deinterlace]		-> buffer1
		std::vector<unsigned char> buffer0;
		std::vector<unsigned char> buffer1;
		int chunk_count = 0;
		int IDAT_count = 0;

		// Loop through chunks.
		bool done = false;
		while (!done)
		{
			unsigned int chunk_size;
			if (!file.readUint(chunk_size))
			{
				errorType = ErrorType::FileDataCorrupt;
				return false;
			}

			ChunkName chunk_name;
			if (!file.readChunkName(chunk_name))
			{
				errorType = ErrorType::FileDataCorrupt;
				return false;
			}

			// Check if current chunk is the header.
			if (equal(chunk_name, ChunkName{ 'I', 'H', 'D', 'R' }))
			{
				if (chunk_size != 13)
				{
					errorType = ErrorType::FileDataCorrupt;
					return false;
				}

				// Read header bytes.
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
					errorType = ErrorType::UnsupportedFormat;
					return false;
				}
			}
			// Check if current chunk is image data.
			else if (equal(chunk_name, ChunkName{ 'I', 'D', 'A', 'T' }))
			{
				// Append image data in chunk to the compressed data buffer.
				if (!file.readNbytes(buffer0, chunk_size))
				{
					errorType = ErrorType::FileDataCorrupt;
					return false;
				}
				++IDAT_count;
			}
			// Check if current chunk is final chunk.
			else if (equal(chunk_name, ChunkName{ 'I', 'E', 'N', 'D' }))
			{
				// Stop looping though chunks.
				done = true;
			}
			// Check if current chunk is any other chunk.
			else
			{
				// Skip over current chunk data.
				file.skipNbytes(chunk_size);
			}
		
			// Read and discard CRC for now.
			unsigned int crc;
			if (!file.readUint(crc))
			{
				errorType = ErrorType::FileDataCorrupt;
				return false;
			}

			++chunk_count;
		}

		// Decompress image data.
		if (!inflate(ihdr, buffer0, buffer1))
		{
			errorType = ErrorType::FileDataCorrupt;
			return false;
		}


		if (ihdr.interlace == 1)
		{
			// Reverse the filtering applied to each scanline of the image.
			buffer0.clear();

			if (!unfilter(ihdr, buffer1, buffer0))
			{
				errorType = ErrorType::FileDataCorrupt;
				return false;
			}
			
			if (!deinterlace(ihdr, buffer0, bytes))
			{
				errorType = ErrorType::FileDataCorrupt;
				return false;
			}
		}
		else
		{
			if (!unfilter(ihdr, buffer1, bytes))
			{
				errorType = ErrorType::FileDataCorrupt;
				return false;
			}
		}

		return true;
	}
}

bool pngimp::Import(const char* path, ErrorType& errorType, ImageStruct& imageStruct)
{
	if (path == 0)
	{
		errorType = ErrorType::FilePathNull;
		return false;
	}

	PNG_IHDR ihdr;
	std::vector<unsigned char> bytes_vec;

	if (!read(path, errorType, ihdr, bytes_vec))
	{
		return false;
	}

	imageStruct.width = ihdr.width;
	imageStruct.height = ihdr.height;
	imageStruct.bitDepth = ihdr.bit_depth;

	switch (ihdr.color_type)
	{
		case 0: imageStruct.colorType = ColorType::GRAY; break;
		case 2: imageStruct.colorType = ColorType::RGB; break;
		case 3: imageStruct.colorType = ColorType::PALLETE; break;
		case 4: imageStruct.colorType = ColorType::GRAYALPHA; break;
		case 6: imageStruct.colorType = ColorType::RGBA; break;
		default: break;
	}

	unsigned char *bytes = new unsigned char[bytes_vec.size()]();

	std::copy(bytes_vec.begin(), bytes_vec.end(), bytes);

	return true;
}

#ifndef PNGIMP_NOEXCEPT
pngimp::Image::Image(const char* path)
{
	if (path == 0)
	{
		throw FilePathNull;
	}

	ErrorType errorType = ErrorType::NoError;
	PNG_IHDR ihdr;

	read(path, errorType, ihdr, p_bytes);

	switch (errorType)
	{
		case pngimp::FileNotExist:
			throw FileNotExist;
			break;
		case pngimp::FileNotPNG:
			throw FileNotPNG;
			break;
		case pngimp::UnsupportedFormat:
			throw UnsupportedFormat;
			break;
		case pngimp::FileDataCorrupt:
			throw FileDataCorrupt;
			break;
		default:
			break;
	}

	p_width = ihdr.width;
	p_height = ihdr.height;
	p_bitDepth = ihdr.bit_depth;

	switch (ihdr.color_type)
	{
		case 0: p_colorType = ColorType::GRAY; break;
		case 2: p_colorType = ColorType::RGB; break;
		case 3: p_colorType = ColorType::PALLETE; break;
		case 4: p_colorType = ColorType::GRAYALPHA; break;
		case 6: p_colorType = ColorType::RGBA; break;
		default: break;
	}
}

const unsigned char* pngimp::Image::Bytes()
{
	return p_bytes.data();
}

const size_t pngimp::Image::nBytes()
{
	return p_bytes.size();
}

const unsigned int pngimp::Image::Width()
{
	return p_width;
}

const unsigned int pngimp::Image::Height()
{
	return p_height;
}

const unsigned char pngimp::Image::BitDepth()
{
	return p_bitDepth;
}

const pngimp::ColorType pngimp::Image::ColorType()
{
	return p_colorType;
}

#endif

#endif
