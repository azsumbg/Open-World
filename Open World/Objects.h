#pragma once

#ifdef OBJECTS_EXPORTS
#define OBJECTS_API _declspec(dllexport)
#else
#define OBJECTS_API _declspec(dllimport)
#endif

#define scr_width 516.0f    //SCREEN 500 x 750 - cell 50 x 50
#define scr_height 686.0f

#define DL_OK 5001
#define DL_FAIL 5002
#define DL_NOT_SUPPORTED 5003

enum class dirs { stop = 0, up = 1, down = 2, left = 3, right = 4 };
enum class grids { not_used = 0, empty = 1, tree = 2, rock = 3, end_tile = 4 };
enum class creatures { creep = 0, walk = 1, fly = 2, hero = 3, no_type = 4, killed = 5 };

struct CELL
{
	float x = 0;
	float y = 0;
	float ex = 0;
	float ey = 0;
	int number = -1;
	int row = -1;
	int col = -1;
	grids type = grids::not_used;
};

class OBJECTS_API PROTON
{
	protected:
		float width = 0;
		float height = 0;

	public:
		float x = 0;
		float y = 0;
		float ex = 0;
		float ey = 0;

		PROTON(float _x, float _y, float _width, float _height)
		{
			x = _x;
			y = _y;
			width = _width;
			height = _height;
			ex = x + width;
			ey = y + height;
		}
		virtual ~PROTON(){}
		void SetEdges()
		{
			ex = x + width;
			ey = y + height;
		}
		void NewDims(float _width, float _height)
		{
			width = _width;
			height = _height;
			ex = x + width;
			ey = y + height;
		}

		virtual void Release()
		{
			delete this;
		}

};
class OBJECTS_API CREATURE :public PROTON
{
	protected:
		creatures type = creatures::no_type;
		float speed = 1.0f;
		int max_frames = 0;
		int frame_delay = 0;
		int frame = 0;
		int attack_rate = 0;
		int max_attack_rate = 0;
		int killed_timer = 200;
		int my_grid_number = -1;

	public:
		int lifes = 100;
		dirs dir = dirs::stop;
		int strenght = 0;
		bool horizontal_move = true;
		bool killed = false;

		CREATURE(float _x, float _y, float _width, float _height, creatures _type) :PROTON(_x, _y, _width, _height)
		{
			int temp_col = (int)(x / 50);
			int temp_row = (int)((y - 50) / 50);
			
			type = _type;
			my_grid_number = temp_row * 10 + temp_col;

			switch (type)
			{
			case creatures::walk:
				lifes = 100;
				max_attack_rate = 8;
				max_frames = 11;
				frame_delay = 7;
				speed = 0.9f;
				strenght = 20;
				break;

			case creatures::creep:
				lifes = 110;
				max_attack_rate = 10;
				max_frames = 3;
				frame_delay = 15;
				speed = 0.4f;
				strenght = 30;
				break;

			case creatures::fly:
				lifes = 120;
				max_attack_rate = 6;
				max_frames = 27;
				frame_delay = 3;
				speed = 1.0f;
				strenght = 10;
				break;

			case creatures::hero: 
				lifes = 120;
				max_attack_rate = 4;
				max_frames = 21;
				frame_delay = 5;
				speed = 0.9f;
				strenght = 40;
				break;

			}
		}
		virtual ~CREATURE(){}
		
		void SetType(creatures _which_type)
		{
			type = _which_type;
			switch (type)
			{
			case creatures::walk:
				NewDims(40.0f, 35.0f);
				lifes = 100;
				max_attack_rate = 8;
				max_frames = 11;
				frame_delay = 7;
				speed = 0.9f;
				strenght = 20;
				break;

			case creatures::creep:
				NewDims(40.0f, 30.0f);
				lifes = 110;
				max_attack_rate = 10;
				max_frames = 3;
				frame_delay = 15;
				speed = 0.4f;
				strenght = 30;
				break;

			case creatures::fly:
				NewDims(30.0f, 40.0f);
				lifes = 90;
				max_attack_rate = 6;
				max_frames = 27;
				frame_delay = 3;
				speed = 1.0f;
				strenght = 10;
				break;

			case creatures::hero:
				NewDims(32.0f, 40.0f);
				lifes = 120;
				max_attack_rate = 4;
				max_frames = 21;
				frame_delay = 5;
				speed = 0.9f;
				strenght = 40;
				break;
			}
		}
		creatures GetType()const
		{
			return type;
		}
		
		int Attack()
		{
			attack_rate++;
			if (attack_rate > max_attack_rate)
			{
				attack_rate = 0;
				return strenght;
			}
			return 0;
		}
		BOOL Killed()
		{
			killed_timer--;
			if (killed_timer < 0)return DL_FAIL;
			return DL_OK;
		}
		int GetCellNum()
		{
			int temp_col = (int)(x / 50);
			int temp_row = (int)((y - 50) / 50);
			my_grid_number = temp_row * 10 + temp_col;
			return my_grid_number;
		}

		virtual void Release() = 0;
		virtual int GetFrame() = 0;
		virtual BOOL Move(float edge_x = 0, float edge_y = 50.0f, float edge_ex = scr_width, 
			float edge_ey = scr_height, int _gear = 1) = 0;
};

typedef PROTON* prot_ptr;
typedef CREATURE* cre_ptr;

extern OBJECTS_API void InitGrid(float _firstx, float _firsty, CELL (&_NewGrid)[10][10]);
extern OBJECTS_API cre_ptr CreatureFactory(creatures _type, float _x, float _y);