#include <cstdio>
#include "raylib.h"
#include "raymath.h"

#define GRAVITY 1.0f
#define DRAG 0.9f
#define WIDTH  600
#define HEIGHT 800

struct Ball {
	float R = 100;
	Vector2 coord = { R, R };
	Vector2 speed = { 10, 1 };
	Color color = RED; 
	
	void resetBall() {
		*this = Ball{}; 
	}
	
	void shiftColor() {
		unsigned char shiftVal = 15;
		this->color.r += shiftVal;
		this->color.g += shiftVal;
		this->color.b += shiftVal;
	}
};


int main() {
	InitWindow(WIDTH, HEIGHT, "raylib Balling");
	SetTargetFPS(60);
	Ball ball{};
	while (!WindowShouldClose()) {
		if (IsKeyPressed(KEY_R))
			ball.resetBall();
		BeginDrawing();
		ClearBackground(RAYWHITE);
		ball.coord = Vector2Add(ball.coord, ball.speed);
		DrawCircleV(ball.coord, ball.R, ball.color);
		if (ball.coord.y + ball.R + ball.speed.y * DRAG < HEIGHT)
			ball.speed.y += GRAVITY;
		else if (ball.coord.y + ball.R >= HEIGHT && FloatEquals(ball.speed.y, 0))
		{
			ball.speed.y = 0;
			ball.coord.y = HEIGHT - ball.R;
		}
		else {
			ball.speed.y = -ball.speed.y * DRAG;
			ball.shiftColor();
		}
		if (ball.coord.x - ball.R + ball.speed.x * DRAG < 0 || ball.coord.x + ball.R + ball.speed.x * DRAG > WIDTH)
			ball.speed.x = -ball.speed.x * DRAG;
		if (HEIGHT - ball.coord.y < ball.R) {
			ball.speed.x *= DRAG;
			ball.shiftColor();
		}
		
		DrawText(TextFormat("x:  %f", ball.coord.x), 20, 20,  30, BLACK);
		DrawText(TextFormat("y:  %f", ball.coord.y), 20, 50,  30, BLACK);
		DrawText(TextFormat("dx: %f", ball.speed.x), 20, 80,  30, BLACK);
		DrawText(TextFormat("dy: %f", ball.speed.y), 20, 110, 30, BLACK);
		DrawText(TextFormat("Color: %d %d %d", ball.color.r, ball.color.g, ball.color.b), 20, 140, 30, BLACK);
		EndDrawing();
	}
	CloseWindow();
	return 0;
}
