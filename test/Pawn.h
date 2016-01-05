
#ifndef __Pawn__
#define __Pawn__

#include "d3dUtility.h"
#include <vector>
#include <list>
#include <cstdlib>


namespace pawn
{
	struct Attribute
	{
		Attribute()
		{
			_lifeTime = 0.0f;
			_age = 0.0f;
			_isAlive = true;
		}

		D3DXVECTOR3 _position;
		D3DXVECTOR3 _velocity;
		D3DXVECTOR3 _acceleration;
		float       _lifeTime;     // how long the particle lives for before dying  
		float       _age;          // current age of the particle  
		D3DXCOLOR   _color;        // current color of the particle   
		D3DXCOLOR   _colorFade;    // how the color fades with respect to time
		bool        _isAlive;
	};

	class Pawn
	{
	public:
		Pawn();
		virtual ~Pawn();

		virtual bool init(IDirect3DDevice9* device, char* xFileName, ID3DXMesh* &mesh, std::vector<D3DMATERIAL9> &Mtrls, std::vector<IDirect3DTexture9*> &textures);

		// sometimes we don't want to free the memory of a dead particle,
		// but rather respawn it instead.
		void update(float timeDelta);
		bool buildCollision(ID3DXMesh* mesh);
		bool collisionSystem(D3DXVECTOR3 t);
		void isAttack(bool &a);
		void render();
		void destory();
		void health(int var);
		void group(int &g);
		bool scan(d3d::BoundingBox box, float range);

	protected:
		IDirect3DDevice9*       _device;
		ID3DXMesh*                      _mesh = 0;
		std::vector<D3DMATERIAL9>       _mtrls;
		std::vector<IDirect3DTexture9*> _textures;
		int						_health;
		int						_group;
		bool					_isAttack;
		bool					_canWalk;
		float					_angle;
		D3DXVECTOR3             _origin;
		d3d::BoundingBox        _boundingBox;
		IDirect3DTexture9*      _tex;
		IDirect3DVertexBuffer9* _vb;
		D3DXVECTOR3				_min;
		D3DXVECTOR3				_max;
		D3DXVECTOR3				_trans;
		D3DXVECTOR3				_ro;
		D3DXVECTOR3				_scale;


											   //
											   // Following three data elements used for rendering the p-system efficiently
											   //

		DWORD _vbSize;      // size of vb
		DWORD _vbOffset;    // offset in vb to lock   
		DWORD _vbBatchSize; // number of vertices to lock starting at _vbOffset
	};
	class enemyTank : public Pawn
	{
	public:
		enemyTank();
		void collision(Pawn c);
		void update(float timeDelta);
	private:
		ID3DXMesh*                      _mesh2 = 0;
		std::vector<D3DMATERIAL9>       _mtrls2;
		std::vector<IDirect3DTexture9*> _textures2;
		float							_gunAngle;
		
	};
	class Bullet : public Pawn
	{
	public:
		Bullet();
		void collision(Pawn c);
		void update(float timeDelta);
	private:

	};
	class Tank : public Pawn
	{
	public:
		Tank(IDirect3DDevice9* device);
		void collision(Pawn c);
		void update(float timeDelta,POINT mouse,POINT pt);
		void render();
		void getAngle(float &angle, float &gunAngle);
		void move(float axis);
		void rotate(float axis);
		void gunRotate(float axis);
		void getTrans(float &x,float &y,float &z);
	private:
		ID3DXMesh*                      _mesh2 = 0;
		std::vector<D3DMATERIAL9>       _mtrls2;
		std::vector<IDirect3DTexture9*> _textures2;
		D3DXVECTOR3						_ro2;
		float							_gunAngle;

	};
	class Building : public Pawn
	{
	public:
		Building(IDirect3DDevice9* device);
		void collision(Pawn c);
		void update(float timeDelta);
	private:

	};
}


#endif // __pSystemH__