// Copyright (c) 2022 Bryan Rykowski
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

    namespace ColorType
    {
        const unsigned char GRAY = 0;
        const unsigned char RGB = 2;
        const unsigned char PALLETE = 3;
        const unsigned char GRAYALPHA = 4;
        const unsigned char RGBA = 6;
    }

	
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
	
    typedef std::array<unsigned char, 4> ChunkName;
    typedef std::array<unsigned char, 8> Signature;

    class Image
	{
	private:
		std::vector<unsigned char> p_bytes;
        PNG_IHDR ihdr;

        bool equal(const Signature& a, const Signature& b);
        bool equal(const ChunkName& a, const ChunkName& b);
        void deinterlace(std::vector<unsigned char>& interlaced_data, std::vector<unsigned char>& final_data);
        void unfilter(std::vector<unsigned char>& filtered_data, std::vector<unsigned char>& interlaced_data);
        void inflate(const std::vector<unsigned char>& in, std::vector<unsigned char>& out);
	public:
		Image(const char* path);
        unsigned char* Bytes();
        size_t nBytes();
        unsigned int Width();
        unsigned int Height();
        unsigned char BitDepth();
        unsigned char ColorType();
	};
}

#ifdef PNGIMP_IMPL

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

    FileReader::FileReader(const char* path)
    {
        // Allocate buffer and open stream to file.
        buffer = std::make_unique<std::array<unsigned char, FileReaderBuffSize>>();
        stream.open(path, std::ios::in | std::ios::binary);
    }

    // Read a single byte from the file.
    bool FileReader::readUchar(unsigned char& out)
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
    bool FileReader::readUint(unsigned int& out)
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
    bool FileReader::readChunkName(ChunkName& out)
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
    bool FileReader::readSignature(Signature& out)
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
    bool FileReader::readNbytes(std::vector<unsigned char>& destination, size_t count)
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
    bool FileReader::skipNbytes(size_t count)
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

    bool Image::equal(const Signature& a, const Signature& b)
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

    bool Image::equal(const ChunkName& a, const ChunkName& b)
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

    void Image::deinterlace(std::vector<unsigned char>& interlaced_data, std::vector<unsigned char>& final_data)
	{

	}
	
    void Image::unfilter(std::vector<unsigned char>& filtered_data, std::vector<unsigned char>& interlaced_data)
	{

	}
	
    void Image::inflate(const std::vector<unsigned char>& in, std::vector<unsigned char>& out)
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
            throw FileDataCorrupt();
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
	}

    Image::Image(const char* path)
    {
        if (path == 0)
        {
            throw FilePathNull();
        }

        FileReader file(path);

		// Validate signature
		Signature sig;
		file.readSignature(sig);

		if (!equal(sig, Signature{ 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a }))
		{
            throw FileNotPNG();
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
                throw FileDataCorrupt();
			}

			ChunkName chunk_name;
			if (!file.readChunkName(chunk_name))
			{
                throw FileDataCorrupt();
			}

			// Check if current chunk is the header.
			if (equal(chunk_name, ChunkName{ 'I', 'H', 'D', 'R' }))
			{
				if (chunk_size != 13)
				{
                    throw FileDataCorrupt();
				}

				// Read header bytes.
				file.readUint(ihdr.width);
				file.readUint(ihdr.height);
				file.readUchar(ihdr.bit_depth);
				file.readUchar(ihdr.color_type);
				file.readUchar(ihdr.compression);
				file.readUchar(ihdr.filter);
				file.readUchar(ihdr.interlace);

				// Verify image is compatible. Only 8/16 bits per sample Greyscale/RGB/RGBA.
				if (
					!(ihdr.bit_depth == 8 || ihdr.bit_depth == 16) ||
					!(ihdr.color_type == 0 || ihdr.color_type == 2 || ihdr.color_type == 6) ||
					ihdr.compression != 0 ||
					ihdr.filter != 0 ||
					!(ihdr.interlace == 0 || ihdr.interlace == 1)
					)
				{
                    throw UnsupportedFormat();
				}
			}
			// Check if current chunk is image data.
			else if (equal(chunk_name, ChunkName{ 'I', 'D', 'A', 'T' }))
			{
				// Append image data in chunk to the compressed data buffer.
				if (!file.readNbytes(buffer0, chunk_size))
				{
                    throw FileDataCorrupt();
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
                throw FileDataCorrupt();
			}

			++chunk_count;
		}

		// Decompress image data.
        inflate(buffer0, buffer1);

		if (ihdr.interlace == 1)
		{
			// Reverse the filtering applied to each scanline of the image.
			buffer0.clear();

            unfilter(buffer1, buffer0);

            deinterlace(buffer0, p_bytes);
		}
		else
		{
            unfilter(buffer1, p_bytes);
		}
	}

    unsigned char* pngimp::Image::Bytes()
    {
        return p_bytes.data();
    }

    size_t pngimp::Image::nBytes()
    {
        return p_bytes.size();
    }

    unsigned int Image::Width()
    {
        return ihdr.width;
    }

    unsigned int Image::Height()
    {
        return ihdr.height;
    }

    unsigned char Image::BitDepth()
    {
        return ihdr.bit_depth;
    }

    unsigned char Image::ColorType()
    {
        return ihdr.color_type;
    }
}

#endif
