//////////////////////////////////////////////////////////////////////////////////////////////////
// 
// File: laser.cpp
// 
// Author: Frank Luna (C) All Rights Reserved
//
// System: AMD Athlon 1800+ XP, 512 DDR, Geforce 3, Windows XP, MSVC++ 7.0 
//
// Desc: Demonstrates the PSystem::ParticleGun system.
//          
//////////////////////////////////////////////////////////////////////////////////////////////////

#include "d3dUtility.h"
#include "psystem.h"
#include "camera.h"
#include <cstdlib>
#include <ctime>
#include "Pawn.h"

//
// Globals
//

IDirect3DDevice9* Device = 0; 
ID3DXMesh*                      Mesh = 0;
std::vector<D3DMATERIAL9>       Mtrls(0);
std::vector<IDirect3DTexture9*> Textures(0);

ID3DXFont* Font = 0;

char Str[30];
const int Width  = 1024;
const int Height = 768;
POINT pt,mouse;
const int background[10][10] = {{ 1,1,1,1,1,1,1,1,1,1 },
								{ 1,0,0,0,0,0,0,0,0,1 },
								{ 1,0,0,0,0,0,1,1,0,1 },
								{ 1,0,0,1,0,1,1,1,0,1 }, 
								{ 1,0,0,0,0,0,0,0,0,1 }, 
								{ 1,0,1,1,0,1,0,1,0,1 },
								{ 1,0,1,1,0,1,0,0,0,1 }, 
								{ 1,0,1,1,0,1,1,1,0,1 }, 
								{ 1,0,0,0,0,0,0,0,0,1 }, 
								{ 1,1,1,1,1,1,1,1,1,1 } };

pawn::Tank *tank = 0;
pawn::Building *build[100] ;
pawn::enemyTank *enemyTank[3];

//
// Framework Functions
//
bool Setup()
{
	// seed random number generator.
	srand((unsigned int)time(0));
	tank = new pawn::Tank(Device);
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			if (background[i][j])
			build[i * 10 + j] = new pawn::Building(Device, -119 + 29.4 * j, 116 - 29.2 * i);
		}
	}
	for (int i = 0; i < 3; i++)
	{
		enemyTank[i] = new pawn::enemyTank(Device);
	}
	D3DXFONT_DESC fd;
	ZeroMemory(&fd, sizeof(D3DXFONT_DESC));
	fd.Width = 12;
	fd.Height = 25;
	fd.Weight = 500;
	fd.Italic = false;
	fd.CharSet = DEFAULT_CHARSET;
	fd.OutputPrecision = 0;
	fd.Quality = 0;
	fd.PitchAndFamily = 0;
	strcpy_s(fd.FaceName, "Times New Roman"); // font style

											//
											// Create an ID3DXFont based on 'fd'.
											//

	if (FAILED(D3DXCreateFontIndirect(Device, &fd, &Font)))
	{
		::MessageBox(0, "D3DXCreateFontIndirect() - FAILED", 0, 0);
		::PostQuitMessage(0);
	}

	//
	// Create a basic scene.
	//

	d3d::DrawBasicScene(Device, 1.0f);

	//
	// Set projection matrix.
	//
	D3DXMATRIX proj;
	D3DXMatrixPerspectiveFovLH(
			&proj,
			D3DX_PI / 3.0f, // 45 - degree
			(float)Width / (float)Height,
			1.0f,
			1000.0f);
	Device->SetTransform(D3DTS_PROJECTION, &proj);

	return true;
}

void Cleanup()
{
	d3d::DrawBasicScene(0, 1.0f);
}

bool Display(float timeDelta)
{
	if( Device )
	{
		tank->update(timeDelta,mouse,pt);
		for (int i = 0; i < 3; i++)
		{
			enemyTank[i]->update(timeDelta, tank);
		}
		
		static float cx = 0.0f;
		static float cy = 0.0f;
		static float cz = 0.0f;
		static float angle = 0.0f;
		static float gunAngle = 0.0f;
		//
		// Update the scene:
		//

		
		SetCursorPos(pt.x, pt.y);

		tank->getAngle(angle,gunAngle);
		tank->getTrans(cx, cy, cz);

		if (::GetAsyncKeyState('W') & 0x8000f)
		{
			tank->move(40.0f*timeDelta);
		}
		if (::GetAsyncKeyState('S') & 0x8000f)
		{
			tank->move(-40.0f*timeDelta);
		}
		if (::GetAsyncKeyState('A') & 0x8000f)
			tank->rotate(80.0f*timeDelta);
		
		if (::GetAsyncKeyState('D') & 0x8000f)
			tank->rotate(-80.0f*timeDelta);

		if (::GetAsyncKeyState(VK_SPACE) & 0x8000f)
			tank->Attack();

		if( ::GetAsyncKeyState('M') & 0x8000f )
			//TheCamera.strafe(4.0f * timeDelta);

		if( ::GetAsyncKeyState('W') & 0x8000f )
			//TheCamera.pitch(1.0f * timeDelta);

		if (::GetAsyncKeyState('S') & 0x8000f)
		{
			//TheCamera.pitch(-1.0f * timeDelta);
		}

		for (int i = 0; i < 10; i++)
		{
			for (int j = 0; j < 10; j++)
			{
				if (background[i][j])
				{
					tank->collision(build[i * 10 + j]);
				}
			}
		}
		for (int i = 0; i < 3; i++)
		{
			tank->collision(enemyTank[i]);
			enemyTank[i]->collision(tank);
		}
		D3DXVECTOR3 pos(cx + 20.0f*-(cos(gunAngle*(2 * D3DX_PI / 360))), cy+10.0f, cz + 20.0f*-(sin(gunAngle*(2 * D3DX_PI / 360))));
		D3DXVECTOR3 target(cx , cy+8.0f, cz );
		D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
		D3DXMATRIX V;
		D3DXMatrixLookAtLH(&V, &pos, &target, &up);
		Device->SetTransform(D3DTS_VIEW, &V);

		//
		// Draw the scene:
		//

		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0);
		Device->BeginScene();

		D3DXMATRIX I;
		D3DXMatrixIdentity(&I);
		Device->SetTransform(D3DTS_WORLD, &I);

		d3d::DrawBasicScene(Device, 1.0f);
		sprintf_s(Str, "x:%f  y:%f", cx,cz);
		RECT rect = { 0, 0, Width, Height };
		Font->DrawText(
			0,
			Str,
			-1, // size of string or -1 indicates null terminating string
			&rect,            // rectangle text is to be formatted to in windows coords
			DT_TOP | DT_LEFT, // draw in the top left corner of the viewport
			0xff000000);      // black text
		for (int i = 0; i < 10; i++)
		{
			for (int j = 0; j < 10; j++)
			{
				if (background[i][j])
				build[i * 10 + j]->render();
			}
		}
		for (int i = 0; i < 3; i++)
		{
			enemyTank[i]->render();
		}
		tank->render();
		Device->SetTransform(D3DTS_WORLD, &I);

		Device->EndScene();
		Device->Present(0, 0, 0, 0);
		mciSendString("", NULL, 0, NULL);
	}
	return true;
}


//
// WndProc
//
LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch( msg )
	{
	case WM_DESTROY:
		::PostQuitMessage(0);
		break;
	case WM_MOUSEMOVE:
		mouse.x = LOWORD(lParam);
		mouse.y = HIWORD(lParam);
		break;
	case WM_LBUTTONDOWN:
		tank->Attack();
		break;
	case WM_KEYDOWN:
		if( wParam == VK_ESCAPE )
			::DestroyWindow(hwnd);

		// Note: we use the message system over GetAsyncKeyState because
		// GetAsyncKeyState was adding particles to fast.  The message
		// system is slower and doesnt add them as fast.  This isn't
		// the best solution, but works for illustration purposes.
		if( wParam == VK_SPACE )
		break;
	}
	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

//
// WinMain
//
int WINAPI WinMain(HINSTANCE hinstance,
				   HINSTANCE prevInstance, 
				   PSTR cmdLine,
				   int showCmd)
{
	if(!d3d::InitD3D(hinstance,
		Width, Height, true, D3DDEVTYPE_HAL, &Device,pt))
	{
		::MessageBox(0, "InitD3D() - FAILED", 0, 0);
		return 0;
	}
		
	if(!Setup())
	{
		::MessageBox(0, "Setup() - FAILED", 0, 0);
		return 0;
	}

	d3d::EnterMsgLoop( Display );

	Cleanup();

	Device->Release();

	return 0;
}