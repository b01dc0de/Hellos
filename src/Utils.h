#ifndef UTILS_H
#define UTILS_H

#include "Hellos.h"

namespace Utils
{
	struct RGBA32
	{
		u8 R;
		u8 G;
		u8 B;
		u8 A;

		u32 SwizzleAsU32()
		{
			return u32((A << 24) | (B << 16) | (G << 8) | (R << 0));
		}
        static RGBA32 Unswizzle(RGBA32 InVal)
		{
			RGBA32 Result = {};
			Result.R = InVal.B;
			Result.G = InVal.G;
			Result.B = InVal.R;
			Result.A = InVal.A;
			return Result;
		}
	};

    	struct Image32
	{
		u32 Width;
		u32 Height;
		u32 PxCount;
		u32 PxBytes;
		RGBA32* PixelBuffer;
	};

	void ReadBMP(const char* InFilename, Image32& OutImage);
}

#endif // UTILS_H