#include "d3dUtility.h"
#include <vector>
#include <list>
#include <cstdlib>
#include "Pawn.h"

using namespace pawn;

Pawn::Pawn()
{
	_device = 0;
	_vb = 0;
	_tex = 0;
	_health = 0;
	_isAttack = 0;
	_group = 0;
	_canWalk = 0;
	_angle = 0.0f;
	_trans = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	_ro = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	_scale = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
}

Pawn::~Pawn()
{
	d3d::Release<IDirect3DVertexBuffer9*>(_vb);
	d3d::Release<IDirect3DTexture9*>(_tex);
}

bool Pawn::init(IDirect3DDevice9* device, char* xFileName, ID3DXMesh* &mesh, std::vector<D3DMATERIAL9> &Mtrls, std::vector<IDirect3DTexture9*> &textures)
{
	// vertex buffer's size does not equal the number of particles in our system.  We
	// use the vertex buffer to draw a portion of our particles at a time.  The arbitrary
	// size we choose for the vertex buffer is specified by the _vbSize variable.

	_device = device; // save a ptr to the device
	HRESULT hr = 0;

	//
	// Load the XFile data.
	//

	ID3DXBuffer* adjBuffer = 0;
	ID3DXBuffer* mtrlBuffer = 0;
	DWORD        numMtrls = 0;

	hr = D3DXLoadMeshFromX(
		xFileName,
		D3DXMESH_MANAGED,
		device,
		&adjBuffer,
		&mtrlBuffer,
		0,
		&numMtrls,
		&mesh);

	if (FAILED(hr))
	{
		::MessageBox(0, "D3DXLoadMeshFromX() - FAILED", 0, 0);
		return false;
	}

	//
	// Extract the materials, and load textures.
	//

	if (mtrlBuffer != 0 && numMtrls != 0)
	{
		D3DXMATERIAL* mtrls = (D3DXMATERIAL*)mtrlBuffer->GetBufferPointer();

		for (int i = 0; i < numMtrls; i++)
		{
			// the MatD3D property doesn't have an ambient value set
			// when its loaded, so set it now:
			mtrls[i].MatD3D.Ambient = mtrls[i].MatD3D.Diffuse;

			// save the ith material
			Mtrls.push_back(mtrls[i].MatD3D);

			// check if the ith material has an associative texture
			if (mtrls[i].pTextureFilename != 0)
			{
				// yes, load the texture for the ith subset
				IDirect3DTexture9* tex = 0;
				D3DXCreateTextureFromFile(
					device,
					mtrls[i].pTextureFilename,
					&tex);

				// save the loaded texture
				textures.push_back(tex);
			}
			else
			{
				// no texture for the ith subset
				textures.push_back(0);
			}
		}
	}
	d3d::Release<ID3DXBuffer*>(mtrlBuffer); // done w/ buffer

											//
											// Optimize the mesh.
											//

	hr = mesh->OptimizeInplace(
		D3DXMESHOPT_ATTRSORT |
		D3DXMESHOPT_COMPACT |
		D3DXMESHOPT_VERTEXCACHE,
		(DWORD*)adjBuffer->GetBufferPointer(),
		0, 0, 0);

	d3d::Release<ID3DXBuffer*>(adjBuffer); // done w/ buffer

	if (FAILED(hr))
	{
		::MessageBox(0, "OptimizeInplace() - FAILED", 0, 0);
		return false;
	}
	

	return true;
}

bool Pawn::buildCollision(ID3DXMesh* mesh)
{
	HRESULT hr = 0;
	BYTE* v = 0;
	mesh->LockVertexBuffer(0, (void**)&v);

	hr = D3DXComputeBoundingBox(
		(D3DXVECTOR3*)v,
		mesh->GetNumVertices(),
		D3DXGetFVFVertexSize(_mesh->GetFVF()),
		&_min,
		&_max);
	mesh->UnlockVertexBuffer();

	if (FAILED(hr))
		return false;
}

void Pawn::render()
{
	D3DXMATRIX T, R, P, S;
	D3DXMatrixTranslation(&T, _trans.x, _trans.y, _trans.z);
	D3DXMatrixScaling(&S, _scale.x, _scale.y, _scale.z);
	D3DXMatrixRotationX(&R, -D3DX_PI * _ro.x);
	D3DXMatrixRotationY(&R, -D3DX_PI * _ro.y);
	D3DXMatrixRotationZ(&R, -D3DX_PI * _ro.z);
	P = S * R * T;
	_device->SetTransform(D3DTS_WORLD, &P);
	for (int i = 0; i < _mtrls.size(); i++)
	{
		_device->SetMaterial(&_mtrls[i]);
		_device->SetTexture(0, _textures[i]);
		_mesh->DrawSubset(i);
	}
}

bool Pawn::scan(d3d::BoundingBox box,float range)
{
	float x, y;
	x = 0.0f;
	y = 0.0f;
	D3DXVECTOR3 t;
	for (int i = 0; i < range; i++)
	{
		x = _trans.x + i*cos(_angle*D3DX_PI / 180);
		y = _trans.z + i*sin(_angle*D3DX_PI / 180);
		t.x = x;
		t.y = _trans.y;
		t.z = y;
		if (box.isPointInside(t))
		{
			return true;
		}
	}
	return false;
}

void Pawn::update(float timeDelta)
{
	_boundingBox._max.x = _max.x*_ro.x + _trans.x;
	_boundingBox._max.y = _max.y*_ro.y + _trans.y;
	_boundingBox._max.z = _max.z*_ro.z + _trans.z;
	_boundingBox._min.x = _min.x*_ro.x + _trans.x;
	_boundingBox._min.y = _min.y*_ro.y + _trans.y;
	_boundingBox._min.z = _min.z*_ro.z + _trans.z;
}

bool Pawn::collisionSystem(D3DXVECTOR3 t)
{
	if (_boundingBox.isPointInside(t))
		return true;
	else
		return false;
}
void Pawn::isAttack(bool &a)
{
	a = _isAttack;
}
void Pawn::destory()
{
	_trans.y = -999999999.0f;
}

void Pawn::group(int &g)
{
	g = _group;
}

void Pawn::health(int var)
{
	_health += var;
}

//*****************************************************************************
// enemyTank
//****************

enemyTank::enemyTank()
{
	_isAttack = false;
}

void enemyTank::collision(Pawn c)
{
	int group;
	bool attack;
	c.isAttack(attack);
	if (c.collisionSystem(_trans))
	{
		if (attack)
		{
			c.group(group);
			c.destory();
			if (group == 1)
			{
				_health -= 50;
			}
		}
	}
}

void enemyTank::update(float timeDelta)
{

}
//*****************************************************************************
// Bullet
//****************
Bullet::Bullet()
{
	_isAttack = true;
}

void Bullet::collision(Pawn c)
{

}

void Bullet::update(float timeDelta)
{

}

//*****************************************************************************
// Tank
//****************
Tank::Tank(IDirect3DDevice9* device)
{
	_gunAngle = 0;
	_isAttack = false;
	Tank::init(device, "tank.X", _mesh, _mtrls, _textures);
	Tank::init(device, "gun.X", _mesh2, _mtrls2, _textures2);
	Tank::buildCollision(_mesh);
	_trans = D3DXVECTOR3(0.0f, 2.0f, 0.0f);
	_scale = D3DXVECTOR3(0.07f, 0.07f, 0.07f);
	_ro2 = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
}

void Tank::collision(Pawn c)
{
	int group;
	bool attack;
	c.isAttack(attack);
	if (c.collisionSystem(_trans))
	{
		if (attack)
		{
			c.group(group);
			c.destory();
			if (group == 1)
			{
				_health -= 50;
			}
		}
	}
}

void Tank::update(float timeDelta,POINT mouse,POINT pt)
{
	_gunAngle -= (mouse.x - pt.x) / 20;
	if (_angle > 360)
		_angle = 0;
	else if (_angle < 0)
		_angle = 360;
	if (_gunAngle > 360)
		_gunAngle = 0;
	else if (_gunAngle < 0)
		_gunAngle = 360;
	_ro.y = _angle/180;
	_ro2.y = _gunAngle/180;
	_boundingBox._max.x = _max.x*_ro.x + _trans.x;
	_boundingBox._max.y = _max.y*_ro.y + _trans.y;
	_boundingBox._max.z = _max.z*_ro.z + _trans.z;
	_boundingBox._min.x = _min.x*_ro.x + _trans.x;
	_boundingBox._min.y = _min.y*_ro.y + _trans.y;
	_boundingBox._min.z = _min.z*_ro.z + _trans.z;

	
}

void Tank::render()
{
	D3DXMATRIX T, Rx, P, S,Ry,Rz;
	D3DXMatrixTranslation(&T, _trans.x, _trans.y, _trans.z);
	D3DXMatrixScaling(&S, _scale.x, _scale.y, _scale.z);
	D3DXMatrixRotationX(&Rx, -D3DX_PI * _ro.x);
	D3DXMatrixRotationY(&Ry, -D3DX_PI * _ro.y);
	D3DXMatrixRotationZ(&Rz, -D3DX_PI * _ro.z);
	P = S * Rx*Ry*Rz * T;
	_device->SetTransform(D3DTS_WORLD, &P);
	for (int i = 0; i < _mtrls.size(); i++)
	{
		_device->SetMaterial(&_mtrls[i]);
		_device->SetTexture(0, _textures[i]);
		_mesh->DrawSubset(i);
	}

	D3DXMatrixTranslation(&T, _trans.x, _trans.y+30.0f*_scale.y, _trans.z);
	D3DXMatrixScaling(&S, _scale.x, _scale.y, _scale.z);
	D3DXMatrixRotationX(&Rx, -D3DX_PI * _ro2.x);
	D3DXMatrixRotationY(&Ry, -D3DX_PI * _ro2.y);
	D3DXMatrixRotationZ(&Rz, -D3DX_PI * _ro2.z);
	P = S * Rx*Ry*Rz * T;
	_device->SetTransform(D3DTS_WORLD, &P);
	for (int i = 0; i < _mtrls2.size(); i++)
	{
		_device->SetMaterial(&_mtrls2[i]);
		_device->SetTexture(0, _textures2[i]);
		_mesh2->DrawSubset(i);
	}
}

void Tank::getAngle(float &angle, float &gunAngle)
{
	angle = _angle;
	gunAngle = _gunAngle;
}
void Tank::move(float axis)
{
	_trans.x = _trans.x + axis * cos(_angle * 2 * D3DX_PI / 360);
	_trans.z = _trans.z + axis * sin(_angle * 2 * D3DX_PI / 360);
}

void Tank::rotate(float axis)
{
	_angle += axis;
}

void Tank::gunRotate(float axis)
{
	_gunAngle += axis;
}
void Tank::getTrans(float &x, float &y, float &z)
{
	x = _trans.x;
	y = _trans.y;
	z = _trans.z;
}


//*****************************************************************************
// Building
//****************
Building::Building(IDirect3DDevice9* device)
{
	_isAttack = false;
	Building::init(device, "build1.X", _mesh, _mtrls, _textures);
	Building::buildCollision(_mesh);
	_trans = D3DXVECTOR3(10.0f, 0.0f, 10.0f);
	_scale = D3DXVECTOR3(0.2f, 0.2f, 0.2f);
}

void Building::collision(Pawn c)
{
	bool attack;
	c.isAttack(attack);
	if (c.collisionSystem(_trans))
	{
		if (attack)
		{
			c.destory();
		}
	}
}

void Building::update(float timeDelta)
{

}