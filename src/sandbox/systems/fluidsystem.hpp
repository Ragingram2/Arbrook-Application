//#pragma once
//
//#include "core/core.hpp"
//#include "graphics/rendering.hpp"
//
//namespace rythe::core
//{
//#define IX(x, y) ((x) + (y * N))
//#define SIZE 32
//#define DIFFUSION .0001f
//#define VISCOSITY 0.00000001f
//
//	struct FluidCube
//	{
//		int size = SIZE;
//		float diffusion;
//		float viscosity;
//
//		float source[SIZE * SIZE];
//		float density[SIZE * SIZE];
//
//		float Velx[SIZE * SIZE];
//		float Vely[SIZE * SIZE];
//
//		float Velx0[SIZE * SIZE];
//		float Vely0[SIZE * SIZE];
//	};
//
//	struct vertex
//	{
//		math::vec3 position;
//		math::vec2 uv;
//	};
//
//	class TestSystem : public System<transform>
//	{
//	public:
//		gfx::RenderInterface* m_api;
//		FluidCube cube;
//		gfx::buffer_handle vertexHandle;
//		gfx::buffer_handle constantHandle;
//		gfx::shader_handle shader;
//		gfx::inputlayout layout;
//
//		TestSystem() = default;
//		virtual ~TestSystem() = default;
//
//		void setup();
//		void update();
//		void shutdown();
//
//		void render();
//
//		void step(int iter, float dt)
//		{
//			diffuse(1, cube.Velx0, cube.Velx, VISCOSITY, dt, iter);
//			diffuse(2, cube.Vely0, cube.Vely, VISCOSITY, dt, iter);
//
//			project(cube.Velx0, cube.Vely0, cube.Velx, cube.Vely, iter);
//
//			advect(1, cube.Velx, cube.Velx0, cube.Velx0, cube.Vely0, dt);
//			advect(2, cube.Vely, cube.Vely0, cube.Velx0, cube.Vely0, dt);
//
//			project(cube.Velx, cube.Vely, cube.Velx0, cube.Vely0, iter);
//
//			diffuse(0, cube.source, cube.density, DIFFUSION, dt, iter);
//			advect(0, cube.density, cube.source, cube.Velx, cube.Vely, dt);
//		}
//
//		void diffuse(int b, float* x, float* x0, float diff, float dt, int iter)
//		{
//			int N = cube.size;
//			float a = dt * diff * N * N;
//			linearSolve(b, x, x0, a, 1 + 4 * a, iter);
//		}
//
//		void advect(int b, float* d, float* d0, float* velX, float* velY, float dt)
//		{
//			int N = cube.size;
//
//			float i0, i1, j0, j1;
//
//			float dtx = dt * N;
//			float dty = dt * N;
//
//			float s0, s1, t0, t1;
//			float tmp1, tmp2, x, y;
//
//			float Nfloat = N;
//			float ifloat, jfloat;
//			int i, j;
//
//			for (j = 1, jfloat = 1; j < N - 1; j++, jfloat++) {
//				for (i = 1, ifloat = 1; i < N - 1; i++, ifloat++) {
//					tmp1 = dtx * velX[IX(i, j)];
//					tmp2 = dty * velY[IX(i, j)];
//					x = ifloat - tmp1;
//					y = jfloat - tmp2;
//
//					if (x < 0.5f) x = 0.5f;
//					if (x > Nfloat + 0.5f) x = Nfloat + 0.5f;
//					i0 = floor(x);
//					i1 = i0 + 1.0f;
//					if (y < 0.5f) y = 0.5f;
//					if (y > Nfloat + 0.5f) y = Nfloat + 0.5f;
//					j0 = floor(y);
//					j1 = j0 + 1.0f;
//
//					s1 = x - i0;
//					s0 = 1.0f - s1;
//					t1 = y - j0;
//					t0 = 1.0f - t1;
//
//					int i0i = int(i0);
//					int i1i = int(i1);
//					int j0i = int(j0);
//					int j1i = int(j1);
//
//					// DOUBLE CHECK THIS!!!
//					d[IX(i, j)] =
//						s0 * (t0 * d0[IX(i0i, j0i)] + t1 * d0[IX(i0i, j1i)]) +
//						s1 * (t0 * d0[IX(i1i, j0i)] + t1 * d0[IX(i1i, j1i)]);
//				}
//			}
//			setBound(b, d);
//		}
//
//		void project(float* velX, float* velY, float* p, float* div, int iter)
//		{
//			int N = cube.size;
//			for (int j = 1; j < N - 1; j++)
//			{
//				for (int i = 1; i < N - 1; i++)
//				{
//					div[IX(i, j)] = -0.5f *
//						(velX[IX(i + 1, j)]
//							- velX[IX(i - 1, j)]
//							+ velY[IX(i, j + 1)]
//							- velY[IX(i, j - 1)]) / N;
//					p[IX(i, j)] = 0;
//				}
//			}
//			setBound(0, div); setBound(0, p);
//			linearSolve(0, div, p, 1, 4, iter);
//			for (int i = 1; i <= N; i++)
//			{
//				for (int j = 1; j <= N; j++)
//				{
//					velX[IX(i, j)] -= .5 * (p[IX(i + 1, j)] - p[IX(i - 1, j)]) * N;
//					velY[IX(i, j)] -= .5 * (p[IX(i, j + 1)] - p[IX(i, j - 1)]) * N;
//				}
//			}
//
//			setBound(1, velX); setBound(2, velY);
//		}
//
//		void linearSolve(int b, float* x, float* x0, float a, float c, int iter)
//		{
//			int N = cube.size;
//			float cRecip = 1.f / c;
//			for (int k = 0; k < iter; k++)
//			{
//				for (int j = 1; j < N - 1; j++)
//				{
//					for (int i = 1; i < N - 1; i++)
//					{
//						x[IX(i, j)] = (x0[IX(i, j)] +
//							a * (x[IX(i + 1, j)]
//								+ x[IX(i - 1, j)]
//								+ x[IX(i, j + 1)]
//								+ x[IX(i, j - 1)])) * cRecip;
//					}
//				}
//				setBound(b, x);
//			}
//		}
//
//		void setBound(int b, float* x)
//		{
//			int N = cube.size;
//			for (int i = 1; i < N - 1; i++) {
//				x[IX(i, 0)] = b == 2 ? -x[IX(i, 1)] : x[IX(i, 1)];
//				x[IX(i, N - 1)] = b == 2 ? -x[IX(i, N - 2)] : x[IX(i, N - 2)];
//			}
//			for (int j = 1; j < N - 1; j++) {
//				x[IX(0, j)] = b == 1 ? -x[IX(1, j)] : x[IX(1, j)];
//				x[IX(N - 1, j)] = b == 1 ? -x[IX(N - 2, j)] : x[IX(N - 2, j)];
//			}
//
//			x[IX(0, 0)] = .5f * (x[IX(1, 0)] + x[IX(0, 1)]);
//			x[IX(0, N - 1)] = .5f * (x[IX(1, N - 1)] + x[IX(0, N - 2)]);
//			x[IX(N - 1, 0)] = .5f * (x[IX(N - 2, 0)] + x[IX(N - 1, 1)]);
//			x[IX(N - 1, N - 1)] = .5f * (x[IX(N - 2, N - 1)] + x[IX(N - 1, N - 2)]);
//		}
//
//		void addDensity(int x, int y, float amount)
//		{
//			int N = cube.size;
//			int idx = IX(x, y);
//			cube.density[idx] += amount;
//		}
//
//		void addVelocity(int x, int y, float amountX, float amountY)
//		{
//			int N = cube.size;
//			int idx = IX(x, y);
//
//			cube.Velx[idx] += amountX;
//			cube.Vely[idx] += amountY;
//		}
//	};
//}