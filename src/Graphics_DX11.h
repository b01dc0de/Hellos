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
	struct m4f
	{
		v4f r0;
		v4f r1;
		v4f r2;
		v4f r3;
	};

	struct VertexColor
	{
		v4f Position;
		v4f Color;
	};

	struct VertexTexture
	{
		v4f Position;
		v2f TexUV;
	};

	struct WVPData
	{
		m4f World;
		m4f View;
		m4f Proj;
	};
	
	int CompileShaderHelper(LPCWSTR SourceFileName, LPCSTR EntryPointFunction, LPCSTR Profile, ID3DBlob** ShaderBlob, const D3D_SHADER_MACRO* Defines = nullptr);
    int InitGraphics();
	void UpdateAndDraw();
	void Draw();
}

#endif // GRAPHICS_DX11_H