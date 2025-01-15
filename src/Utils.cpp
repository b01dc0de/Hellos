#include "Utils.h"

namespace Utils
{
#pragma pack(push, 1)
	struct BMPFileHeader
	{
		u16 Type; // Always Ascii BM
		u32 SizeInBytes; // Size (bytes) of file
		u16 Res1; // 0
		u16 Res2; // 0
		u32 OffsetBytes; // Offset (bytes) to actual pixel data
	};

	struct BMPInfoHeader
	{
		u32 StructSize; // Size (bytes) of InfoHeader
		s32 Width;
		s32 Height; // NOTE(chris): If positive, pixel data is bottom to top
		u16 Planes; // Must be 1
		u16 BitsPerPixel; // Bits-per-pixel (0, 1, 4, 8, 16, 24, 32)
		u32 Compression; // *Should* be 0
		u32 Unused_ImgSize; // Only used if Compression is weird (not 0)
		s32 HRes; // Horizontal resolution
		s32 VRes; // Vertical resolution
		u32 ColorsUsed; // 0 for our purposes
		u32 ColorsImportant; // 0 for our purposes
	};
#pragma pack(pop)

	struct BMP
	{
		BMPFileHeader FileHeader;
		BMPInfoHeader InfoHeader;
	};

    void ReadBMP(const char* InFilename, Image32& OutImage)
	{
		FILE* BMP_File = nullptr;
		fopen_s(&BMP_File, InFilename, "rb");

		if (nullptr != BMP_File)
		{
			// Get file size in bytes
			fpos_t FileSizeBytes;
			{
				int Result = fseek(BMP_File, 0, SEEK_END);
				// CKA_TODO: Assert Result == 0
				fgetpos(BMP_File, &FileSizeBytes);

				// Set curr pos to beginning of file
				Result = fseek(BMP_File, 0, SEEK_SET);
			}

			// Parse .BMP file into BMP struct
			BMP ReadBMP = {};
			fread_s(&ReadBMP, sizeof(BMP), sizeof(BMP), 1, BMP_File);

			constexpr u16 SupportedBPP = 32;
			constexpr u16 BitmapFileTypeValue = 0x4D42;
			if (SupportedBPP != ReadBMP.InfoHeader.BitsPerPixel) { DebugBreak(); }

			if (BitmapFileTypeValue == ReadBMP.FileHeader.Type)
			{
				// Extract pixel data into OutImage struct
				OutImage.Width = ReadBMP.InfoHeader.Width;
                OutImage.Height = ReadBMP.InfoHeader.Height > 0 ? ReadBMP.InfoHeader.Height : -ReadBMP.InfoHeader.Height;
				OutImage.PxCount = OutImage.Width * OutImage.Height;
				OutImage.PxBytes = u32(FileSizeBytes - sizeof(BMP));
				OutImage.PixelBuffer = (RGBA32*)(new u8[OutImage.PxBytes]);

				fread_s(OutImage.PixelBuffer, OutImage.PxBytes, sizeof(u32), OutImage.PxCount, BMP_File);

				// Unswizzle(?) the pixel data
				for (unsigned int PxIdx = 0; PxIdx < OutImage.PxCount; PxIdx++)
				{
					RGBA32& CurrPx = OutImage.PixelBuffer[PxIdx];
					CurrPx = RGBA32::Unswizzle(CurrPx);
				}
			}

			fclose(BMP_File);
		}
	}
}
