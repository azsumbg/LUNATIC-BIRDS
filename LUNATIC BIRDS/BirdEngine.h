#pragma once

#ifdef BIRDENGINE_EXPORTS
#define ENGINE_API _declspec(dllexport)
#else 
#define ENGINE_API _declspec(dllimport)
#endif

enum class dirs { stop = 0, left = 1, right = 2, up = 3, down = 4 };

enum class birds { bomb = 0, gray = 1, red = 2, yellow = 3 };

enum class pigs { pig = 0, big_pig = 1 };

enum class fields { background = 0, field = 1, h_board = 2, v_board = 3, prem_h_board = 4, 
	prem_v_board = 5, sling = 6, rock = 7 };

constexpr float scr_width = 600.0f;
constexpr float scr_height = 550.0f;


namespace dll
{
	
	class ENGINE_API ITEM
	{
		protected:
			float width = 0;
			float height = 0;
			
		public:
			float x = 0;
			float y = 0;
			float ex = 0;
			float ey = 0;

			ITEM(float start_x, float start_y, float start_width = 1.0f, float start_height = 1.0f)
			{
				x = start_x;
				y = start_y;
				width = start_width;
				height = start_height;
				ex = x + width;
				ey = y + height;
			}

			virtual ~ITEM() {};

			float GetW()const
			{
				return width;
			}
			float GetH()const
			{
				return height;
			}

			void SetW(float new_width)
			{
				width = new_width;
				ex = x + width;
			}
			void SetH(float new_height)
			{
				width = new_height;
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

	class ENGINE_API BASIC_FIELD :public ITEM
	{
		protected:
			fields type = fields::background;
			int frame = 0;
			int frame_delay = 0;
			int max_frame_delay = 0;
			int max_frames = 0;

			BASIC_FIELD(fields what, float sx, float sy) :ITEM(sx, sy)
			{
				type = what;

				switch (type)
				{
				case fields::background:
					NewDims(600.0f, 550.0f);
					lifes = 0;
					max_frames = 9;
					max_frame_delay = 5;
					break;

				case fields::sling:
					NewDims(170.0f, 150.0f);
					lifes = 0;
					max_frames = 5;
					max_frame_delay = 9;
					break;

				case fields::field:
					NewDims(600.0f, 100.0f);
					lifes = 0;
					break;

				case fields::rock:
					NewDims(100.0f, 100.0f);
					lifes = 10000;
					break;

				case fields::h_board:
					NewDims(100.0f, 45.0f);
					lifes = 100;
					break;

				case fields::v_board:
					NewDims(100.0f, 75.0f);
					lifes = 100;
					break;

				case fields::prem_h_board:
					NewDims(150.0f, 95.0f);
					lifes = 200;
					break;

				case fields::prem_v_board:
					NewDims(100.0f, 115.0f);
					lifes = 200;
					break;
				}
			}

		public:
			int lifes = 0;

			virtual ~BASIC_FIELD() {};
			
			fields GetType() const
			{
				return type;
			}
			void SetType(fields what)
			{
				type = what;
			}

			int GetFrame()
			{
				frame_delay++;
				if (frame_delay > max_frame_delay)
				{
					frame_delay = 0;
					frame++;
					if (frame > max_frames)frame = 0;
				}
				return frame;
			}

			void InitFrame()
			{
				frame = 0;
				frame_delay = 0;
			}

			void Release()
			{
				delete this;
			}

			ENGINE_API friend BASIC_FIELD* CreateFieldItem(fields what_field, float where_x, float where_y);
	};

	class ENGINE_API BASIC_PIG :public ITEM
	{
		protected:
			pigs type = pigs::pig;

			int frame = 0;
			int max_frames = 0;
			int frame_delay = 0;

			BASIC_PIG(pigs what, float sx, float sy) :ITEM(sx, sy)
			{
				type = what;
				switch (type)
				{
				case pigs::pig:
					NewDims(90.0f, 110.0f);
					max_frames = 11;
					frame_delay = 5;
					lifes = 100;
					break;

				case pigs::big_pig:
					NewDims(100.0f, 84.0f);
					max_frames = 14;
					frame_delay = 3;
					lifes = 200;
					break;
				}
			}

		public:
			int lifes = 0;

			virtual ~BASIC_PIG() {};

			ENGINE_API friend BASIC_PIG* CreatePig(float first_x, float first_y, pigs what);

			void Release()
			{
				delete this;
			}

			int GetFrame() 
			{
				frame_delay--;
				if (frame_delay < 0)
				{
					switch (type)
					{
					case pigs::pig:
						frame_delay = 5;
						frame++;
						break;

					case pigs::big_pig:
						frame_delay = 3;
						frame++;
						break;
					}
				}
				if (frame > max_frames)frame = 0;
				return frame;
			}

			pigs GetType() const
			{
				return type;
			}
	};

	class ENGINE_API BASIC_BIRD :public ITEM
	{
		protected:
			birds type = birds::red;
			dirs dir = dirs::right;

			float slope = 0;
			float intercept = 0;
			
			float initial_x = 0;
			float initial_y = 0;
			float destination_x = 0;
			float destination_y = 0;

			bool vertical_line = false;
			bool horizontal_line = false;
			bool go_up = true;

			int frame = 0;
			int max_frames = 0;
			int frame_delay = 0;

			int damage = 0;

			BASIC_BIRD(birds what, float sx, float sy) :ITEM(sx, sy)
			{
				type = what;

				switch (type)
				{
				case birds::red:
					NewDims(75.0f, 60.0f);
					max_frames = 6;
					frame_delay = 8;
					damage = 60;
					break;

				case birds::bomb:
					NewDims(61.0f, 80.0f);
					max_frames = 1;
					frame_delay = 15;
					damage = 80;
					break;

				case birds::gray:
					NewDims(60.0f, 50.0f);
					max_frames = 11;
					frame_delay = 8;
					damage = 50;
					break;

				case birds::yellow:
					NewDims(50.0f, 48.0f);
					max_frames = 8;
					frame_delay = 5;
					damage = 55;
					break;
				}
			}

			void SetJumpData(float start_x, float start_y, float end_x, float end_y)
			{
				vertical_line = false;
				horizontal_line = false;
				if (start_x < end_x)dir = dirs::right;
				else if (start_x > end_x)dir = dirs::left;
				else dir = dirs::stop;

				if (end_x - start_x == 0)vertical_line = true;
				else
				{
					slope = (end_y - start_y) / (end_x - start_x);
					intercept = start_y - slope * start_x;
				}

				if (end_y == start_y)horizontal_line = true;
			}

		public:
			bool flying = false;

			virtual ~BASIC_BIRD() {};

			void Release()
			{
				delete this;
			}

			int GetDamage() const
			{
				if (go_up) return damage;
				else return (int)(damage * 1.5);
			}

			int GetFrame()
			{
				frame_delay--;
				if (frame_delay < 0)
				{
					switch (type)
					{
					case birds::red:
						frame_delay = 12;
						frame++;
						break;

					case birds::bomb:
						frame_delay = 20;
						frame++;
						break;

					case birds::gray:
						frame_delay = 8;
						frame++;
						break;

					case birds::yellow:
						frame_delay = 10;
						frame++;
						break;
					}
				}
				if (frame > max_frames)frame = 0;
				return frame;
			}

			birds GetType() const
			{
				return type;
			}

			bool Shoot(float sx,float sy,float dest_x,float dest_y)
			{
				if (!flying)
				{
					flying = true;
					
					initial_x = sx;
					initial_y = sy;
					destination_x = dest_x;
					destination_y = dest_y;

					if (initial_y == destination_y)horizontal_line = true;

					SetJumpData(sx, sy, dest_x, dest_y);
				}
				else
				{
					if (go_up)
					{
						if (horizontal_line)
						{
							if (dir == dirs::right)
							{
								x++;
								SetEdges();
								if (x <= scr_width)return true;
								else return false;
							}
							if (dir == dirs::left)
							{
								x--;
								SetEdges();
								if (ex >= 0)return true;
								else return false;
							}
						}
						if (vertical_line)
						{
							if (y > destination_y)
							{
								y--; 
								SetEdges();
								return true;
							}
							else 
							{
								destination_x = x + (x - initial_x);
								initial_x = x;
								initial_y = y;
								destination_y = scr_height - 40.0f;
								go_up = false;

								SetJumpData(initial_x, initial_y, destination_x, destination_y);
								return true;
							}
							
						}
						else
						{
							if (ey < 50)return false;
							if (dir == dirs::right)
							{
								if (x < destination_x)
								{
									x++;
									y = slope * x + intercept;
									SetEdges();
									if (y < destination_y)
									{
										destination_x = x + (x - initial_x);
										initial_x = x;
										initial_y = y;
										destination_y = scr_height - 40.0f;
										go_up = false;

										SetJumpData(initial_x, initial_y, destination_x, destination_y);
										return true;
									}
									return true;
								}
								else
								{
									y--;
									SetEdges(); 
									if (y < destination_y)
									{
										destination_x = x + (x - initial_x);
										initial_x = x;
										initial_y = y;
										destination_y = scr_height - 40.0f;
										go_up = false;

										SetJumpData(initial_x, initial_y, destination_x, destination_y);
										return true;
									}
									return true;
								}
							}
							if (dir == dirs::left)
							{
								if (x > destination_x)
								{
									x--;
									y = slope * x + intercept;
									SetEdges();
									if (y < destination_y)
									{
										destination_x = x + (x - initial_x);
										initial_x = x;
										initial_y = y;
										destination_y = scr_height - 40.0f;
										go_up = false;

										SetJumpData(initial_x, initial_y, destination_x, destination_y);
										return true;
									}
									return true;
								}
								else
								{
									y--;
									SetEdges();
									if (y < destination_y)
									{
										destination_x = x + (x - initial_x);
										initial_x = x;
										initial_y = y;
										destination_y = scr_height - 40.0f;
										go_up = false;

										SetJumpData(initial_x, initial_y, destination_x, destination_y);
										return true;
									}
									return true;
								}
							}
						}
					}
					else
					{
						if (vertical_line)
						{
							if (y < destination_y)
							{
								y++;
								SetEdges();
								return true;
							}
							else return false;
							
						}
						else
						{
							if (dir == dirs::right)
							{
								if (x < destination_x)
								{
									x++;
									y = slope * x + intercept;
									SetEdges();
									if (y >= destination_y) return false;
									return true;
								}
								else
								{
									y--;
									SetEdges();
									if (y >= destination_y)return false;
									return true;
								}
							}
							if (dir == dirs::left)
							{
								if (x > destination_x)
								{
									x--;
									y = slope * x + intercept;
									SetEdges();
									if (y >= destination_y) return false;
									
									return true;
								}
								else
								{
									y--;
									SetEdges();
									if (y >= destination_y) return false;
									
									return true;
								}
							}
						}
					}

				}

				return true;
			}

	
			ENGINE_API friend BASIC_BIRD* CreateBird(float first_x, float first_y, birds what);
	};

	/////////////////////////////////////////////////////////////

	typedef BASIC_FIELD* FieldItem;
	typedef BASIC_PIG* Pig;
	typedef BASIC_BIRD* Bird;

	ENGINE_API BASIC_FIELD* CreateFieldItem(fields what_field, float where_x, float where_y);
	ENGINE_API BASIC_PIG* CreatePig(float first_x, float first_y, pigs what);
	ENGINE_API BASIC_BIRD* CreateBird(float first_x, float first_y, birds what);
	
}

