#include <SDL.h>
#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <cmath>
#include <iostream>
#include <fstream>

void loadPerspective(float fov, float aspect, float znear, float zfar)
{

	float ymax = znear * (float)tan(fov * (M_PI / 360));
	float ymin = -ymax;
	float xmax = ymax * aspect;
	float xmin = ymin * aspect;

	float width = xmax - xmin;
	float height = ymax - ymin;

	float depth = zfar - znear;
	float q = -(zfar + znear) / depth;
	float qn = -2 * (zfar * znear) / depth;

	float w = 2 * znear / width;
	w = w / aspect;
	float h = 2 * znear / height;

	GLfloat m [16] =
	{
		w, 0, 0, 0,

		0, h, 0, 0,

		0, 0, q, -1,

		0, 0, qn, 0
	};

	glLoadMatrixf(m);
}

float Map(const float value, const float fromLow, const float fromHigh, const float toLow, const float toHigh)
{
	return ((value - fromLow) / (fromHigh - fromLow) * (toHigh - toLow)) + toLow;
}

void DrawRectangle(const float width, const float height)
{
	GLfloat halfWidth = (width * 0.5f);
	GLfloat halfHeight = (width * 0.5f);
	glBegin(GL_LINE_LOOP);
	{
		glVertex2f(-halfWidth, halfHeight);
		glVertex2f(halfWidth, halfHeight);
		glVertex2f(halfWidth, -halfHeight);
		glVertex2f(-halfWidth, -halfHeight);
	}
	glEnd();
}

void FillRectangle(const float width, const float height)
{
	GLfloat halfWidth = (width * 0.5f);
	GLfloat halfHeight = (height * 0.5f);
	glBegin(GL_TRIANGLE_FAN);
	{
		glVertex2f(-halfWidth, halfHeight);
		glVertex2f(halfWidth, halfHeight);
		glVertex2f(halfWidth, -halfHeight);
		glVertex2f(-halfWidth, -halfHeight);
	}
	glEnd();
}

void DrawRectangle(const float x, const float y, const float width, const float height)
{
	glBegin(GL_LINE_LOOP);
	{
		glVertex2f(x, y);
		glVertex2f(x + width, y);
		glVertex2f(x + width, y + height);
		glVertex2f(x , y + height);
	}
	glEnd();
}

void FillRectangle(const float x, const float y, const float width, const float height)
{
	glBegin(GL_TRIANGLE_FAN);
	{
		glVertex2f(x, y);
		glVertex2f(x + width, y);
		glVertex2f(x + width, y + height);
		glVertex2f(x, y + height);
	}
	glEnd();
}

void FillCircle(const float radius, const int resolution = 16)
{
	float step = (2 * M_PI) / resolution;
	glBegin(GL_TRIANGLE_FAN);
	{
		float angle = 0;
		for (int i = 0; i <= resolution; ++i)
		{
			glVertex2f(cos(angle) * radius, sin(angle) * radius);
			angle += step;
		}
	}
	glEnd();
}

void DrawCircle(const float radius, const int resolution = 16)
{
	glBegin(GL_LINE_LOOP);
	{
		const float step = (2 * M_PI) / resolution;
		float angle = 0;
		for (int i = 0; i <= resolution; ++i)
		{
			glVertex2f(cos(angle) * radius, sin(angle) * radius);
			angle += step;
		}
	}
	glEnd();
}

void DrawBatteryIndicator(const int percent)
{
	float scale = Map(percent, 0, 100, -0.75, 0.75);
	float shadeR = Map(percent, 0, 100, 1, 0);
	float shadeG = Map(percent, 0, 100, 0, 1);

	float height = 0.75 * 2;
	float scaledHeight = height * scale;
	float scaledDifference = height * (1.0-scale);

	glColor3f(1.0f, 1.0f, 1.0f);
	FillRectangle(1.0f, height);

	glColor3f(0.5f, 0.5f, 0.5f);
	FillRectangle(-0.5f, -0.75f, 1.0f, scaledDifference);

	glColor3f(shadeR, shadeG, 0.0f);
	FillRectangle(-0.5f, -0.75f + scaledDifference, 1.0f, scaledHeight);
}

void DrawPowerIndicator()
{
	int seconds = 0;
	int percent = 0;

	auto state = SDL_GetPowerInfo(&seconds, &percent);

	switch (state)
	{
	case SDL_PowerState::SDL_POWERSTATE_CHARGED:
		{

		}
		break;
	case SDL_PowerState::SDL_POWERSTATE_CHARGING:
		{

		}
		break;
	case SDL_PowerState::SDL_POWERSTATE_ON_BATTERY:
		{
			//if (percent != -1)
			//{
				//DrawBatteryIndicator(percent);
			//}			
		}
		break;
	case SDL_PowerState::SDL_POWERSTATE_NO_BATTERY:
		{
			//glColor3f(0.75f, 0.75f, 0.75f);
			//FillCircle(1, 32);
		}
		break;
	}
}

int main(int argn, char* args[])
{
	int s = sizeof(GLbitfield);

	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_Window * window = nullptr;
	
	window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	
	SDL_GLContext context = SDL_GL_CreateContext(window);

	int numJoysticks = SDL_NumJoysticks();

	SDL_Joystick * joystick = SDL_JoystickOpen(0);
	int axes = SDL_JoystickNumAxes(joystick);
	int hats = SDL_JoystickNumHats(joystick);
	int buttons = SDL_JoystickNumButtons(joystick);

	bool joystickMode = (numJoysticks > 0);
	
	float x = 0, y = 0;
	float angle = 0;
	Uint32 last = 0;

	float xOffset = 0, yOffset = 0;
	float pAngle = 0, mAngle = 0;

	bool running = true;
	SDL_Event e;
	while (running)
	{
		int now = SDL_GetTicks();
		int diff = now - last;
		last = now;

		float delta = diff / 1000.0f;

		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
			{
				running = false;
			}
			else if (e.type == SDL_KEYDOWN)
			{
				if (!joystickMode)
				{
					switch (e.key.keysym.sym)
					{
					case SDLK_w: yOffset = 1; break;
					case SDLK_a: xOffset = -1; break;
					case SDLK_x: yOffset = -1; break;
					case SDLK_d: xOffset = 1; break;
					case SDLK_q: pAngle = -1; break;
					case SDLK_e: mAngle = 1; break;
					}
				}
			}
			else if (e.type == SDL_KEYUP)
			{
				if (!joystickMode)
				{
					switch (e.key.keysym.sym)
					{
					case SDLK_w: yOffset = 0; break;
					case SDLK_a: xOffset = 0; break;
					case SDLK_x: yOffset = 0; break;
					case SDLK_d: xOffset = 0; break;
					case SDLK_q: pAngle = 0; break;
					case SDLK_e: mAngle = 0; break;
					}
				}
			}
			else if (e.type == SDL_JOYAXISMOTION)
			{
				if (joystickMode)
				{
					xOffset = Map(SDL_JoystickGetAxis(joystick, 0), -32768, 32767, -1, 1);
					yOffset = Map(SDL_JoystickGetAxis(joystick, 1), -32768, 32767, 1, -1);
					pAngle = Map(SDL_JoystickGetAxis(joystick, 2), -32768, 32767, 0, -1);
					mAngle = Map(SDL_JoystickGetAxis(joystick, 5), -32768, 32767, 0, 1);
				}
			}
		}

		const float dead = 0.2f;

		x += (xOffset < dead && xOffset > -dead) ? 0 : xOffset * delta;
		y += (yOffset < dead && yOffset > -dead) ? 0 : yOffset * delta;

		float pAngleOffset = (pAngle < dead && pAngle > -dead) ? 0 : pAngle * (M_PI * 2) * delta;
		float mAngleOffset = (mAngle < dead && mAngle > -dead) ? 0 : mAngle * (M_PI * 2) * delta;

		pAngleOffset *= 10;
		mAngleOffset *= 10;

		angle += pAngleOffset;
		angle += mAngleOffset;

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		{
			glViewport(0, 0, 800, 600);

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			glPushMatrix();
			glTranslatef(0.75f,-0.75f,0.0f);
			glScalef(0.25f, 0.25f, 1.0f);
			DrawPowerIndicator();
			glPopMatrix();

			glMatrixMode(GL_PROJECTION);
			loadPerspective(60.0, 800.0f / 600.0f, 0.1f, 100.0f);

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			glPushMatrix();
			
				glTranslatef(x, y, -5.0);
				glRotatef(angle, 0, 1, 0);

				/*glColor3f(1.0f, 1.0f, 1.0f);
				glBegin(GL_TRIANGLE_FAN);
				glVertex3d(0, 1, 0);
				glVertex3d(0.5, 0, 0.5);
				glVertex3d(-0.5, 0, 0.5);
				glVertex3d(-0.5, 0, -0.5);
				glVertex3d(0.5, 0, -0.5);
				glEnd();*/

				glColor3f(0.0f, 0.0f, 1.0f);

				glBegin(GL_LINE_LOOP);
					glVertex3f(0.6, 0.75, 0.6);
					glVertex3f(0.6, 0.75, -0.6);
					glVertex3f(-0.6, 0.75, -0.6);
					glVertex3f(-0.6, 0.75, 0.6);
				glEnd();

				glBegin(GL_LINE_LOOP);
					glVertex3f(0.8, 0.75, 0.8);
					glVertex3f(0.8, 0.75, -0.8);
					glVertex3f(-0.8, 0.75, -0.8);
					glVertex3f(-0.8, 0.75, 0.8);
				glEnd();

				glBegin(GL_LINE_LOOP);
					glVertex3f(0.8, 0.8, 0.8);
					glVertex3f(0.8, 0.8, -0.8);
					glVertex3f(-0.8, 0.8, -0.8);
					glVertex3f(-0.8, 0.8, 0.8);
				glEnd();
				
				glBegin(GL_LINE_LOOP);
					glVertex3f(0.6, -0.75, 0.6);
					glVertex3f(0.6, -0.75, -0.6);
					glVertex3f(-0.6, -0.75, -0.6);
					glVertex3f(-0.6, -0.75, 0.6);
				glEnd();
				
				glBegin(GL_LINE_LOOP);
					glVertex3f(0.8, -0.75, 0.8);
					glVertex3f(0.8, -0.75, -0.8);
					glVertex3f(-0.8, -0.75, -0.8);
					glVertex3f(-0.8, -0.75, 0.8);
				glEnd();
				
				
				glBegin(GL_LINE_LOOP);
					glVertex3f(0.8, -0.85, 0.8);
					glVertex3f(0.8, -0.85, -0.8);
					glVertex3f(-0.8, -0.85, -0.8);
					glVertex3f(-0.8, -0.85, 0.8);
				glEnd();

				glBegin(GL_LINES);

					glVertex3f(0.6, 0.75, 0.6);
					glVertex3f(0.6, -0.75, 0.6);

					glVertex3f(0.6, 0.75, -0.6);
					glVertex3f(0.6, -0.75, -0.6);

					glVertex3f(-0.6, 0.75, 0.6);
					glVertex3f(-0.6, -0.75, 0.6);

					glVertex3f(-0.6, 0.75, -0.6);
					glVertex3f(-0.6, -0.75, -0.6);
				
				glEnd();

				/*glVertex3d(0, 1, 0);
				glVertex3d(0.5, 0, 0.5);

				glVertex3d(0, 1, 0);
				glVertex3d(-0.5, 0, 0.5);

				glVertex3d(0, 1, 0);
				glVertex3d(-0.5, 0, -0.5);

				glVertex3d(0, 1, 0);
				glVertex3d(0.5, 0, -0.5);

				glVertex3d(0.5, 0, 0.5);
				glVertex3d(-0.5, 0, 0.5);

				glVertex3d(-0.5, 0, 0.5);
				glVertex3d(-0.5, 0, -0.5);

				glVertex3d(-0.5, 0, -0.5);
				glVertex3d(0.5, 0, -0.5);

				glVertex3d(0.5, 0, -0.5);
				glVertex3d(0.5, 0, 0.5);*/

				/*glColor3f(1.0f, 0.0f, 1.0f);
				glBegin(GL_POLYGON);
					glVertex3d(1.5, 1, -3);
					glVertex3d(0.5, 0, -3);
					glVertex3d(2.5, 0, -3);
				glEnd();*/

			glPopMatrix();

			/*glBegin(GL_QUADS);
			glVertex2f(-0.25f, -0.25f);
			glVertex2f(0.25f, -0.25f);
			glVertex2f(0.25f, 0.25f);
			glVertex2f(-0.25f, 0.25f);
			glEnd();*/
		}

		SDL_GL_SwapWindow(window);
	}

	SDL_JoystickClose(joystick);

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}