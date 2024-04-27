#pragma once

#ifdef GORAPI_EXPORTS
#define GORAPI_API _declspec(dllexport)
#else
#define GORAPI_API _declspec(dllimport)
#endif

constexpr float scr_width = 700.0f;
constexpr float scr_height = 650.0f;

constexpr int OK = 1111;
constexpr int FAIL = 1112;
constexpr int NaN = 1113;

namespace dll
{
	enum class dirs { stop = 0, up = 1, down = 2, left = 3, right = 4 };

	enum class types { no_type = 0, gorilla1 = 1, gorilla2 = 2, gorilla3 = 3, hero = 4 };

	class GORAPI_API ATOM
	{
		protected:
			float width = 0;
			float height = 0;

		public:
			float x = 0;
			float y = 0;
			float ex = 0;
			float ey = 0;

			ATOM(float fx, float fy, float fw, float fh)
			{
				x = fx;
				y = fy;
				width = fw;
				height = fh;
				ex = x + width;
				ey = y + height;
			}
			virtual ~ATOM() {};

			float GetWidth() const
			{
				return width;
			}
			float GetHeight() const
			{
				return height;
			}

			void SetWidth(float new_width)
			{
				width = new_width;
				ex = x + width;
			}
			void SetHeight(float new_height)
			{
				height = new_height;
				ey = y + height;
			}
			void SetEdges()
			{
				ex = x + width;
				ey = y + height;
			}
			void NewDims(float new_width, float new_height)
			{
				width = new_width;
				height = new_height;
				ex = x + width;
				ey = y + height;
			}
	};

	class GORAPI_API FACTORY : public ATOM
	{
		protected:

			types type = types::no_type;
			int frame = 0;
			int max_frames = 0;
			int frame_delay = 0;

			float speed = 0;

			FACTORY(types which_type, float wheretoputx, float wheretoputy) :ATOM(wheretoputx, wheretoputy, 1.0f, 1.0f)
			{
				type = which_type;
				switch (type)
				{
				case types::gorilla1:
					NewDims(100.0f, 70.0f);
					max_frames = 34;
					frame_delay = 3;
					break;

				case types::gorilla2:
					NewDims(120.0f, 100.0f);
					max_frames = 3;
					frame_delay = 10;
					break;

				case types::gorilla3:
					NewDims(120.0f, 90.0f);
					max_frames = 23;
					frame_delay = 5;
					break;

				case types::hero:
					NewDims(80.0f, 80.0f);
					max_frames = 5;
					frame_delay = 8;
					break;
				}
			}

		public:
			dirs dir = dirs::stop;
			float jump_x = 0;
			float jump_y = 0;
			float jump_ex = 0;
			float jump_ey = 0;
			dirs jump_dir = dirs::stop;

			virtual ~FACTORY() {};

			virtual void Release() = 0;

			virtual int GetFrame() = 0;
			virtual int Move(float what_speed) = 0;
			virtual int Jump() = 0;
	};

	typedef FACTORY* creat_ptr;

	extern GORAPI_API creat_ptr iFactory(types what, float where_y);

}