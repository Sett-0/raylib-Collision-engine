#include <cstdio>
#include "raylib.h"
#include "raymath.h"

#define cR 100
#define sX 10
#define sY 1
#define GRAVITY 1.0f
#define DRAG 0.9f

void resetBall(Vector2 &ccoords, Vector2 &cspeed) {
	ccoords.x = cR;
	ccoords.y = cR;
	cspeed.x = sX;
	cspeed.y = sY;
}

int main() {
	const int screenWidth = 600;
	const int screenHeight = 800;
	char xcoord[50];
	char ycoord[50];
	char xspeed[50];
	char yspeed[50];
	InitWindow(screenWidth, screenHeight, "raylib basic window");
	SetTargetFPS(60);
	Vector2 ccoords = { cR, cR };
	Vector2 cspeed  = { sX, sY };
	while (!WindowShouldClose()) {
		if (IsKeyPressed(KEY_R))
			resetBall(ccoords, cspeed);
		BeginDrawing();
		ClearBackground(RAYWHITE);
		ccoords = Vector2Add(ccoords, cspeed);
		DrawCircleV(ccoords, cR, RED);
		if (ccoords.y + cR + cspeed.y * DRAG < screenHeight)
			cspeed.y += GRAVITY;
		else 
			cspeed.y = -cspeed.y * DRAG;
		if (ccoords.x - cR < 0 || ccoords.x + cR + cspeed.x * DRAG > screenWidth)
			cspeed.x = -cspeed.x * DRAG;
		if (screenHeight - ccoords.y < cR)
			cspeed.x *= DRAG;
		
		sprintf(xcoord, "x:  %f",  ccoords.x);
		sprintf(ycoord, "y:  %f",  ccoords.y);
		sprintf(xspeed, "dx: %f", cspeed.x);
		sprintf(yspeed, "dy: %f", cspeed.y);
		DrawText(xcoord, 20, 20,  30, BLACK);
		DrawText(ycoord, 20, 50,  30, BLACK);
		DrawText(xspeed, 20, 80,  30, BLACK);
		DrawText(yspeed, 20, 110, 30, BLACK);
		EndDrawing();
		
	}
	CloseWindow();
	return 0;
}
