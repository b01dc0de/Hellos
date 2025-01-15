#ifndef GRAPHICS_DX11_H
#define GRAPHICS_DX11_H

#include "Hellos.h"

namespace Graphics_DX11
{
	struct v2f
	{
		float X;
		float Y;
	};
	struct v3f
	{
		float X;
		float Y;
		float Z;
	};
	struct v4f
	{
		float X;
		float Y;
		float Z;
		float W;
	};

	struct VertexColor
	{
		v4f Position;
		v4f Color;
	};
	
	int CompileShaderHelper(LPCWSTR SourceFileName, LPCSTR EntryPointFunction, LPCSTR Profile, ID3DBlob** ShaderBlob);
	int InitGraphics();
	void UpdateGraphicsState();
	void Draw();
}

#endif // GRAPHICS_DX11_H