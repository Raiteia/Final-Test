#include "d3dUtility.h"
#include <vector>
#include <list>
#include <cstdlib>
#include "Pawn.h"
#include <ctime>

using namespace pawn;

Pawn::Pawn()
{
	_device = 0;
	_vb = 0;
	_tex = 0;
	_health = 50;
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
	if (_health > 0)
	{
		D3DXMATRIX T, Rx, P, S, Ry, Rz;
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
	}
	
}

bool Pawn::scan(Pawn* box,float range)
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
		if (box->collisionSystem(t))
		{
			return true;
		}
	}
	return false;
}

void Pawn::update(float timeDelta)
{
	_boundingBox._max.x = _max.x*_scale.x + _trans.x;
	_boundingBox._max.y = _max.y*_scale.y + _trans.y;
	_boundingBox._max.z = _max.z*_scale.z + _trans.z;
	_boundingBox._min.x = _min.x*_scale.x + _trans.x;
	_boundingBox._min.y = _min.y*_scale.y + _trans.y;
	_boundingBox._min.z = _min.z*_scale.z + _trans.z;
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
	_trans.y = -999.0f;
	_canWalk = 0;
}

void Pawn::group(int &g)
{
	g = _group;
}

void Pawn::health(int var)
{
	_health += var;
}
void Pawn::getTrans(float &x, float &y, float &z)
{
	x = _trans.x;
	y = _trans.y;
	z = _trans.z;
}

bool Pawn::canWalk()
{
	if (_canWalk)
		return true;
	else
		return false;
}
void Pawn::collision(Pawn* c)
{
	0;
}

void Pawn::getBullet(Pawn* &b,int i)
{
	0;
}
void Pawn::setDir(float angle,D3DXVECTOR3 trans)
{
	0;
}

//*****************************************************************************
// enemyTank
//****************

enemyTank::enemyTank(IDirect3DDevice9* device)
{
	srand(rand());
	_health = 50;
	int a = rand() % 4;
	_isAttack = false;
	_canAttack = 1;
	_canWalk = 1;
	_group = 2;
	_bulletCount = 0;
	_isDead = 0;
	switch (a)
	{
	case 0:
		_trans = D3DXVECTOR3(116.0f, 4.0f, 88.0f);
		break;
	case 1:
		_trans = D3DXVECTOR3(116.0f, 4.0f, -117.0f);
		break;
	case 2:
		_trans = D3DXVECTOR3(-92.0f, 4.0f, 88.0f);
		break;
	case 3:
		_trans = D3DXVECTOR3(-92.0f, 4.0f, -117.0f);
		break;
	}
	_scale = D3DXVECTOR3(0.14f, 0.14f, 0.14f);
	enemyTank::init(device, "enemyTank.X", _mesh, _mtrls, _textures);
	enemyTank::buildCollision(_mesh);
	for (int i = 0; i < 3; i++)
	{
		_bullet[i] = new Bullet(device, _group);
	}
}

void enemyTank::collision(Pawn *c)
{
	int group;
	bool attack;
	D3DXVECTOR3 otherTrans;
	Pawn* b;
	float x, y, z;
	c->getTrans(x, y, z);
	otherTrans.x = x;
	otherTrans.y = y;
	otherTrans.z = z;
	c->isAttack(attack);
	c->group(group);
	if (group == 1)
	{
		
		for (int i = 0; i < 3; i++)
		{
			c->getBullet(b,i);
			b->getTrans(x, y, z);
			otherTrans.x = x;
			otherTrans.y = y;
			otherTrans.z = z;
			if (this->collisionSystem(otherTrans))
			{
				b->destory();
				_health =0;
			}
		}

	}

}

void enemyTank::update(float timeDelta,Pawn* t)
{
	_canAttack += timeDelta;
	int count = 0;
	count = rand() % 4;
	if (_canWalk)
	{
		if (floor(_trans.x) == 116 && floor(_trans.z) == 88)
		{
			if (count < 2)
			{
				_moveState = 2;
				_trans.z += -1;
			}

			else
			{
				_moveState = 3;
				_trans.x += -1;
			}
		}
		else if (floor(_trans.x) == 116 && floor(_trans.z) == 0)
		{
			if (count < 2)
			{
				_moveState = 3;
				_trans.x += -1;
			}
			else if (count == 2)
			{
				_moveState = 2;
				_trans.z += -1;
			}
			else
			{
				_moveState = 1;
				_trans.z += -1;
			}
		}
		else if (floor(_trans.x) == 116 && floor(_trans.z) == -117)
		{
			if (count < 2)
			{
				_moveState = 1;
				_trans.z += 1;
			}
			else
			{
				_moveState = 2;
				_trans.x += -1;
			}
		}
		else if (floor(_trans.x) == 0 && floor(_trans.z) == 88)
		{
			if (count < 2)
			{
				_moveState = 2;
				_trans.z += -1;
			}
			else if (count == 2)
			{
				_moveState = 3;
				_trans.x += -1;
			}
			else
			{
				_moveState = 4;
				_trans.x += 1;
			}
		}
		else if (floor(_trans.x) == 0 && floor(_trans.z) == 0)
		{
			if (count == 0)
			{
				_moveState = 1;
				_trans.z += 1;
			}
			else if (count == 1)
			{
				_moveState = 2;
				_trans.z += -1;
			}
			else if (count == 2)
			{
				_moveState = 3;
				_trans.x += -1;
			}
			else
			{
				_moveState = 4;
				_trans.x += 1;
			}
		}
		else if (floor(_trans.x) == 0 && floor(_trans.z) == -117)
		{
			if (count < 2)
			{
				_moveState = 1;
				_trans.z += 1;
			}
			else if (count == 2)
			{
				_moveState = 3;
				_trans.x += -1;
			}
			else
			{
				_moveState = 4;
				_trans.x += 1;
			}
		}
		else if (floor(_trans.x) == -92 && floor(_trans.z) == 88)
		{
			if (count < 2)
			{
				_moveState = 2;
				_trans.z += -1;
			}
			else
			{
				_moveState = 4;
				_trans.x += 1;
			}
		}
		else if (floor(_trans.x) == -92 && floor(_trans.z) == 0)
		{
			if (count < 2)
			{
				_moveState = 4;
				_trans.x += 1;
			}
			else if (count == 2)
			{
				_moveState = 2;
				_trans.z += -1;
			}
			else
			{
				_moveState = 1;
				_trans.z += 1;
			}
		}
		else if (floor(_trans.x) == -92 && floor(_trans.z) == -117)
		{
			if (count < 2)
			{
				_moveState = 1;
				_trans.z += 1;
			}
			else
			{
				_moveState = 4;
				_trans.x += 1;
			}
		}

		if (scan(t, 100))
		{
			if (_canAttack >= 1.5)
			{
				if (_bulletCount >= 3)
					_bulletCount = 0;
				if (!(_bullet[_bulletCount]->canWalk()))
				{
					D3DXVECTOR3 a;
					a = _trans;
					a.y = _trans.y ;
					_bullet[_bulletCount]->setDir(_angle, a);
					_bulletCount++;
					_canAttack = 0;
				}
			}
		}
		else
		{
			switch (_moveState)
			{
			case 1:
				_angle = 90;
				_trans.z += 30 * timeDelta;
				break;
			case 2:
				_angle = 270;
				_trans.z += -30 * timeDelta;
				break;
			case 3:
				_angle = 180;
				_trans.x += -30 * timeDelta;
				break;
			case 4:
				_angle = 0;
				_trans.x += 30 * timeDelta;
				break;
			}
		}
	}
	if (_isDead == 0 && _health <= 0)
	{
		this->dead();
	}
	for (int i = 0; i < 3; i++)
	{
		_bullet[i]->update(timeDelta);
	}
	_ro.y = _angle / 180;
	_boundingBox._max.x = _max.x*_scale.x + _trans.x;
	_boundingBox._max.y = _max.y*_scale.y + _trans.y;
	_boundingBox._max.z = _max.z*_scale.z + _trans.z;
	_boundingBox._min.x = _min.x*_scale.x + _trans.x;
	_boundingBox._min.y = _min.y*_scale.y + _trans.y;
	_boundingBox._min.z = _min.z*_scale.z + _trans.z;
}

void enemyTank::getBullet(Pawn* &b,int i)
{
	b = _bullet[i];
}

void enemyTank::render()
{
	D3DXMATRIX T, Rx, P, S, Ry, Rz;
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
	for (int i = 0; i < 3; i++)
	{
		_bullet[i]->render();
	}
}

void enemyTank::dead()
{
	_canWalk = 0;
	_isDead = 1;
}
//*****************************************************************************
// Bullet
//****************
Bullet::Bullet(IDirect3DDevice9* device,int g)
{
	_isAttack = true;
	_group = g;
	if (_group == 1)
	{
		init(device, "bullet1.X", _mesh, _mtrls, _textures);
	}
	else if (_group == 2)
	{
		init(device, "bullet2.X", _mesh, _mtrls, _textures);
	}
	Bullet::buildCollision(_mesh);
	_trans = D3DXVECTOR3(0.0f, -999.0f, 0.0f);
	_scale = D3DXVECTOR3(1.35f, 0.35f, 0.35f);
	_canWalk = 0;
	_group = 0;
}

void Bullet::collision(Pawn c)
{

}

void Bullet::update(float timeDelta)
{
	_ro.y = _angle / 180;
	_boundingBox._max.x = _max.x*_scale.x + _trans.x;
	_boundingBox._max.y = _max.y*_scale.y + _trans.y;
	_boundingBox._max.z = _max.z*_scale.z + _trans.z;
	_boundingBox._min.x = _min.x*_scale.x + _trans.x;
	_boundingBox._min.y = _min.y*_scale.y + _trans.y;
	_boundingBox._min.z = _min.z*_scale.z + _trans.z;
	if (_canWalk)
	{
		_trans.x += 200 * cos(_angle*D3DX_PI / 180)*timeDelta;
		_trans.z += 200 * sin(_angle*D3DX_PI / 180)*timeDelta;
		_trans.y += -5 * timeDelta;
	}
	if (_trans.y < 0)
		destory();
}

void Bullet::setDir(float angle, D3DXVECTOR3 trans)
{
	_angle = angle;
	_trans.x = trans.x + 8*cos(_angle*D3DX_PI / 180);
	_trans.z = trans.z + 8*sin(_angle*D3DX_PI / 180);
	_trans.y = trans.y;
	_canWalk = 1;
}

//*****************************************************************************
// Tank
//****************
Tank::Tank(IDirect3DDevice9* device)
{
	_health = 50;
	_gunAngle = 0;
	_isAttack = false;
	_group = 1;
	Tank::init(device, "tank.X", _mesh, _mtrls, _textures);
	Tank::init(device, "gun.X", _mesh2, _mtrls2, _textures2);
	Tank::buildCollision(_mesh);
	_trans = D3DXVECTOR3(0.0f, 4.0f, 0.0f);
	_scale = D3DXVECTOR3(0.14f, 0.14f, 0.14f);
	_ro2 = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	_canAttack = 1;
	_isDead = 0;
	_canWalk = 1;
	for (int i = 0; i < 3; i++)
	{
		_bullet[i] = new Bullet(device,_group);
	}
}

void Tank::collision(Pawn *c)
{
	int group;
	bool attack;
	D3DXVECTOR3 otherTrans,ct;
	Pawn* b;
	float x, y, z;
	c->getTrans(x,y,z);
	otherTrans.x = x;
	otherTrans.y = y;
	otherTrans.z = z;
	c->isAttack(attack);
	c->group(group);
	if (group==2)
	{
		
		for (int i = 0; i < 3; i++)
		{
			c->getBullet(b,i);
			b->getTrans(x, y, z);
			otherTrans.x = x;
			otherTrans.y = y;
			otherTrans.z = z;
			if (collisionSystem(otherTrans))
			{
				b->destory();
				_health -= 50;
			}
		}
		
	}
	else
	{
		ct.x = _trans.x + 12 * cos(_angle * D3DX_PI / 180);
		ct.z = _trans.z + 12 * sin(_angle * D3DX_PI / 180);
		ct.y = 0;
		if (c->collisionSystem(ct))
		{
			_trans = _ptrans;
		}
		for (int i = 0; i < 3; i++)
		{
			c->collision(_bullet[i]);
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
	_boundingBox._max.x = _max.x*_scale.x + _trans.x;
	_boundingBox._max.y = _max.y*_scale.y + _trans.y;
	_boundingBox._max.z = _max.z*_scale.z + _trans.z;
	_boundingBox._min.x = _min.x*_scale.x + _trans.x;
	_boundingBox._min.y = _min.y*_scale.y + _trans.y;
	_boundingBox._min.z = _min.z*_scale.z + _trans.z;
	for (int i = 0; i < 3; i++)
	{
		_bullet[i]->update(timeDelta);
	}
	_canAttack += timeDelta;
	if (_isDead == 0 && _health <= 0)
	{
		Tank::dead();
	}
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
	for (int i = 0; i < 3; i++)
	{
		_bullet[i]->render();
	}
}

void Tank::getAngle(float &angle, float &gunAngle)
{
	angle = _angle;
	gunAngle = _gunAngle;
}
void Tank::move(float axis)
{
	if (_canWalk)
	{
		_ptrans = _trans;
		_trans.x = _trans.x + axis * cos(_angle * 2 * D3DX_PI / 360);
		_trans.z = _trans.z + axis * sin(_angle * 2 * D3DX_PI / 360);
	}
}

void Tank::rotate(float axis)
{
	if(_canWalk)
	_angle += axis;
}

void Tank::gunRotate(float axis)
{
	if (_canWalk)
	_gunAngle += axis;
}

void Tank::Attack()
{
	if (_canWalk)
	{
		if (_canAttack> 1)
		{
			if (_bulletCount >= 3)
				_bulletCount = 0;
			if (!(_bullet[_bulletCount]->canWalk()))
			{
				D3DXVECTOR3 a;
				a = _trans;
				a.y = _trans.y + 30.0f*_scale.y;
				_bullet[_bulletCount]->setDir(_gunAngle, a);
				_bulletCount++;
				_canAttack = 0;
			}
		}

	}
	
}

void Tank::getBullet(Pawn* &b,int i)
{
	b = _bullet[i];
}

void Tank::dead()
{
	_canWalk = 0;
	_isDead = 1;
}

//*****************************************************************************
// Building
//****************
Building::Building(IDirect3DDevice9* device,float x,float z)
{
	srand(rand());
	int s = rand() % 2;
	_isAttack = false;
	if (s)
	{
		Building::init(device, "build1.X", _mesh, _mtrls, _textures);
	}
	else
	{
		Building::init(device, "build2.X", _mesh, _mtrls, _textures);
	}
	Building::buildCollision(_mesh);
	_trans = D3DXVECTOR3(x, 0.0f, z);
	_scale = D3DXVECTOR3(0.4f, 0.4f, 0.4f);
	_boundingBox._max.x = _max.x*_scale.x + _trans.x;
	_boundingBox._max.y = _max.y*_scale.y + _trans.y;
	_boundingBox._max.z = _max.z*_scale.z + _trans.z;
	_boundingBox._min.x = _min.x*_scale.x + _trans.x;
	_boundingBox._min.y = _min.y*_scale.y + _trans.y;
	_boundingBox._min.z = _min.z*_scale.z + _trans.z;
	_max = _max;
	_min = _min;
	
}

void Building::collision(Pawn* c)
{
	bool attack;
	c->isAttack(attack);
	D3DXVECTOR3 otherTrans;
	float x, y, z;
	c->getTrans(x, y, z);
	otherTrans.x = x;
	otherTrans.y = y;
	otherTrans.z = z;
	if (collisionSystem(otherTrans))
	{
		if (attack)
		{
			c->destory();
		}
	}
}

void Building::update(float timeDelta)
{

}