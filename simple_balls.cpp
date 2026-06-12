#include <ctime>
#include <vector>

#include "raylib.h"
#include "raymath.h"

#define BALL_DENSITY 1.0f
#define GRAVITY 1
#define DRAG 0.9f
#define WIDTH  800
#define HEIGHT 800
#define GRIDSIZE_X 4
#define GRIDSIZE_Y 4
#define ITERATIONS 4

struct BallInitData {
	float R;
	float mass;
	Vector2 coord;
	Vector2 speed;
	Color color;
};

class Ball {
public:
	float R;
	float mass;
	Vector2 coord;
	Vector2 speed;
	Color color; 

	Ball(float r, Vector2 cd, Vector2 sp, Color clr) : R(r), coord(cd), speed(sp), color(clr) {
		mass = BALL_DENSITY * PI * r*r;
		ball = BallInitData { r, mass, cd, sp, clr };
	}
	
	Ball() = default;
	
	void resetBall() {
		this->R     = ball.R;
		this->coord = ball.coord;
		this->speed = ball.speed;
		this->color = ball.color;
	}
	
	void shiftColor() {
		unsigned char shiftVal = 15;
		this->color.r += shiftVal;
		this->color.g += shiftVal;
		this->color.b += shiftVal;
	}

private:
	BallInitData ball;
};

struct CollisionGrid {
	std::vector<Ball*> grid[GRIDSIZE_X][GRIDSIZE_Y];
};

Ball makeRandomBall() {
	float R  = GetRandomValue(10, 100);
	float x  = GetRandomValue(R, WIDTH - R);
	float y  = GetRandomValue(0, HEIGHT/2);
	float dx = GetRandomValue(0, 20);
	float dy = GetRandomValue(-200, 200) / 200.0f;
	unsigned char r = GetRandomValue(0, 255);
	unsigned char g = GetRandomValue(0, 255);
	unsigned char b = GetRandomValue(0, 255);
	Vector2 coord = { x,  y };
	Vector2 speed = { dx, dy };
	Color color = Color { r, g, b, 255 };
	return Ball(R, coord, speed, color);
}

std::vector<Ball> makeRandomBalls(const int N) {
	std::vector<Ball> balls(N);
	for (size_t i = 0; i < balls.size(); i++) {
		balls[i] = makeRandomBall();
	}
	return balls;
}

void clearGrid(CollisionGrid &collisionGrid) {
	for (size_t x = 0; x < GRIDSIZE_X; x++)
		for (size_t y = 0; y < GRIDSIZE_Y; y++)
			collisionGrid.grid[x][y].clear();
}

void populateGrid(CollisionGrid &collisionGrid, std::vector<Ball> &balls) {
	for (Ball &ball : balls) {
		int cellX = ball.coord.x / (WIDTH  / GRIDSIZE_X);
		int cellY = ball.coord.y / (HEIGHT / GRIDSIZE_Y);
		collisionGrid.grid[cellX][cellY].push_back(&ball);
		
		// Checking if the ball touches neighboring cells
		int lCell = (ball.coord.x - ball.R) / (WIDTH  / GRIDSIZE_X);
		int rCell = (ball.coord.x + ball.R) / (WIDTH  / GRIDSIZE_X);
		int tCell = (ball.coord.y - ball.R) / (HEIGHT / GRIDSIZE_Y);
		int bCell = (ball.coord.y + ball.R) / (HEIGHT / GRIDSIZE_Y);
		// Top-left
		if (lCell > 0 && tCell > 0 && lCell < cellX && tCell < cellY) { collisionGrid.grid[lCell][tCell].push_back(&ball); }
		// Top
		if (tCell > 0 && tCell < cellY) { collisionGrid.grid[cellX][tCell].push_back(&ball); }
		// Top-right
		if (rCell < GRIDSIZE_X && tCell > 0 && rCell > cellX && tCell < cellY) { collisionGrid.grid[rCell][tCell].push_back(&ball); }
		// Right
		if (rCell < GRIDSIZE_X && rCell > cellX) { collisionGrid.grid[rCell][cellY].push_back(&ball); }
		// Bottom-right
		if (rCell < GRIDSIZE_X && bCell < GRIDSIZE_Y && rCell > cellX && bCell > cellY) { collisionGrid.grid[rCell][bCell].push_back(&ball); }
		// Bottom
		if (bCell < GRIDSIZE_Y && bCell > cellY) { collisionGrid.grid[cellX][bCell].push_back(&ball); }
		// Bottom-left
		if (lCell > 0 && bCell < GRIDSIZE_Y && lCell < cellX && bCell > cellY) { collisionGrid.grid[lCell][bCell].push_back(&ball); }
		// Left
		if (lCell > 0 && lCell < cellX) { collisionGrid.grid[lCell][cellY].push_back(&ball); }
	}
}

void onCollision(float &coord, float &speed) {
	speed  = -speed;
	coord +=  speed;
	speed  =  speed * DRAG;
}

void onCollision(Vector2 &coord, Vector2 &speed) {
	speed = Vector2Negate(speed);
	coord = Vector2Add(coord, speed);
	speed = Vector2Scale(speed, DRAG);
}

void resolveCollisions(CollisionGrid &collisionGrid) {
	// Window bounds
	Vector2 lBound[] = { { 0,     0      }, { 0,     HEIGHT } };
	Vector2 rBound[] = { { WIDTH, 0      }, { WIDTH, HEIGHT } };
	Vector2 tBound[] = { { 0,     0      }, { WIDTH, 0      } };
	Vector2 bBound[] = { { 0,     HEIGHT }, { WIDTH, HEIGHT } };			
	for (size_t i = 0; i < ITERATIONS; i++) {
		for (size_t x = 0; x < GRIDSIZE_X; x++) {
			for (size_t y = 0; y < GRIDSIZE_Y; y++) {
				auto &bucket = collisionGrid.grid[x][y];
				for (size_t i = 0; i < bucket.size(); i++) {
					// Collision type: Ball : lBound
					if (x == 0) {
						if (CheckCollisionCircleLine(bucket[i]->coord, bucket[i]->R, lBound[0], lBound[1])) {
							onCollision(bucket[i]->coord.x, bucket[i]->speed.x);
						}
					}
					
					// Collision type: Ball : rBound
					if (x == GRIDSIZE_X-1) {
						if (CheckCollisionCircleLine(bucket[i]->coord, bucket[i]->R, rBound[0], rBound[1])) {
							onCollision(bucket[i]->coord.x, bucket[i]->speed.x);
						}
					}
					
					// Collision type: Ball : tBound
					if (y == 0) {
						if (CheckCollisionCircleLine(bucket[i]->coord, bucket[i]->R, tBound[0], tBound[1])) {
							onCollision(bucket[i]->coord.y, bucket[i]->speed.y);
						}
					}
					
					// Collision type: Ball : bBound
					if (y == GRIDSIZE_Y-1) {
						if (CheckCollisionCircleLine(bucket[i]->coord, bucket[i]->R, bBound[0], bBound[1])) {
							onCollision(bucket[i]->coord.y, bucket[i]->speed.y);
						}
					}
					
					if (bucket.size() > 1) {
						// Collision type: Ball : Ball
						for (size_t j = i + 1; j < bucket.size(); j++) {
							if (bucket[i] == bucket[j]) continue;
							if (CheckCollisionCircles(bucket[i]->coord, bucket[i]->R, bucket[j]->coord, bucket[j]->R)) {
								onCollision(bucket[i]->coord, bucket[i]->speed);
								onCollision(bucket[j]->coord, bucket[j]->speed);
							}
						}
					}
				}
			}
		}
	}
}

int main() {
	InitWindow(WIDTH, HEIGHT, "raylib Balling");
	SetTargetFPS(60);
	SetRandomSeed(std::time(0));
	CollisionGrid grid{};
	
	//auto balls = makeRandomBalls(10);
	Ball ball1 = Ball(100, { WIDTH/2, HEIGHT/2 }, { 0, 0 }, RED);
	Ball ball2 = Ball(100, { WIDTH/2, HEIGHT-101 }, { 0, 0 }, BLUE);
	std::vector<Ball> balls = { ball1, ball2 };
	
	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(RAYWHITE);
		
		// Drawing the collison grid
		for (size_t x = 1; x < GRIDSIZE_X; x++) {
			for (size_t y = 1; y < GRIDSIZE_Y; y++) {
				Vector2 startPosRow = { 0,     HEIGHT / GRIDSIZE_Y * (float)y };
				Vector2 endPosRow   = { WIDTH, HEIGHT / GRIDSIZE_Y * (float)y };
				DrawLineDashed(startPosRow, endPosRow, 5, 5, LIGHTGRAY);
				
				Vector2 startPosCol = { WIDTH / GRIDSIZE_X * (float)x, 0      };
				Vector2 endPosCol   = { WIDTH / GRIDSIZE_X * (float)x, HEIGHT };
				DrawLineDashed(startPosCol, endPosCol, 5, 5, LIGHTGRAY);
			}
		}
		
		for (Ball &ball : balls) {
			// Reset balls option
			if (IsKeyPressed(KEY_R)) {
				for (Ball &ball : balls) {
					ball.resetBall();
				}
				continue;
			}
			
			// Updating ball
			ball.speed.y += GRAVITY;
			ball.coord = Vector2Add(ball.coord, ball.speed);
			
			// Collision handling
			clearGrid(grid);
			populateGrid(grid, balls);
			resolveCollisions(grid);
			
			// Drawing ball
			DrawCircleV(ball.coord, ball.R, ball.color);
			
			// Extra logic 
			// if (ball.coord.y + ball.R >= HEIGHT && FloatEquals(ball.speed.y, 0))
			// {
				// ball.speed.y = 0;
				// ball.coord.y = HEIGHT - ball.R;
			// }
			// if (HEIGHT - ball.coord.y < ball.R) {
				// ball.speed.x *= DRAG;
			// }
		}
		
		DrawText(TextFormat(" x: %s%f", balls[0].coord.x >= 0 ? " " : "", balls[0].coord.x), 20,           20,  30, BLACK);
		DrawText(TextFormat(" y: %s%f", balls[0].coord.y >= 0 ? " " : "", balls[0].coord.y), 20,           50,  30, BLACK);
		DrawText(TextFormat("dx: %s%f", balls[0].speed.x >= 0 ? " " : "", balls[0].speed.x), 20,           80,  30, BLACK);
		DrawText(TextFormat("dy: %s%f", balls[0].speed.y >= 0 ? " " : "", balls[0].speed.y), 20,           110, 30, BLACK);
		DrawText(TextFormat(" x: %s%f", balls[1].coord.x >= 0 ? " " : "", balls[1].coord.x), WIDTH-220-20, 20,  30, BLACK);
		DrawText(TextFormat(" y: %s%f", balls[1].coord.y >= 0 ? " " : "", balls[1].coord.y), WIDTH-220-20, 50,  30, BLACK);
		DrawText(TextFormat("dx: %s%f", balls[1].speed.x >= 0 ? " " : "", balls[1].speed.x), WIDTH-220-20, 80,  30, BLACK);
		DrawText(TextFormat("dy: %s%f", balls[1].speed.y >= 0 ? " " : "", balls[1].speed.y), WIDTH-220-20, 110, 30, BLACK);
		DrawText(TextFormat("Color: %d %d %d", balls[0].color.r, balls[0].color.g, balls[0].color.b), 20,           140, 30, BLACK);
		DrawText(TextFormat("Color: %d %d %d", balls[1].color.r, balls[1].color.g, balls[1].color.b), WIDTH-220-20, 140, 30, BLACK);
		EndDrawing();
	}
	CloseWindow();
	return 0;
}
