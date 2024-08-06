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

// pngimp_inflate.cpp

#include "pngimp.hpp"
#include <zlib.h>

#ifndef PNGIMP_CHUNK_SIZE
#define PNGIMP_CHUNK_SIZE 262144 // Initial size of destination buffer and size to grow it each time.
#endif

namespace pngimp
{
	void InflateData(SmartBuffer& from, SmartBuffer& to)
	{
		if (to.get() == nullptr) to = std::make_unique<std::vector<char>>();
		to->resize(0);

		// Get the adler32 appended to the end of the Zlib stream.
		unsigned int adler = ReadUint32(from->data() + (from->size() - 4));

		// Initialize the Zlib stream struct.
		z_stream stream;
		stream.zalloc = Z_NULL;
		stream.zfree = Z_NULL;
		stream.opaque = Z_NULL;
		int ret = inflateInit(&stream);
		if (ret != Z_OK) throw(StreamFail(StreamFail::Cause::ZlibInit));

		// Set Zlib to read from the input buffer.
		stream.avail_in = from->size();
		stream.next_in = reinterpret_cast<unsigned char*>(from->data());

		do
		{
			// Decompress the input buffer, writing PNGIMP_CHUNK_SIZE bytes to the output buffer at a time.
			size_t size = to->size();
			to->resize(to->size() + PNGIMP_CHUNK_SIZE);
			stream.next_out = reinterpret_cast<unsigned char*>(to->data() + size);
			stream.avail_out = PNGIMP_CHUNK_SIZE;
			ret = inflate(&stream, Z_NO_FLUSH);

			if (ret == Z_STREAM_END) break;
			else if (ret != Z_OK)
			{
				inflateEnd(&stream);
				throw(MalformedFile(MalformedFile::Cause::BadZlibStream));
			}
		}
		while (stream.avail_out == 0);

		// Make sure the adler32 of the inflated data matches the value appended to the stream.
		if (stream.adler != adler) throw(MalformedFile(MalformedFile::Cause::BadZlibStream));

		// Shrink the output buffer to the actual data written to it.
		to->resize(stream.total_out);

		// Clean up the Zlib state.
		inflateEnd(&stream);
	}
}
