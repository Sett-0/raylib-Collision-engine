#include <ctime>
#include <vector>

#include "raylib.h"
#include "raymath.h"

#define FPS 60
#define WIDTH  1600
#define HEIGHT 800
#define GRIDWIDTH  400
#define GRIDHEIGHT 400
#define GRIDSIZE_X (WIDTH  / GRIDWIDTH)
#define GRIDSIZE_Y (HEIGHT / GRIDHEIGHT)
#define ITERATIONS 4

#define NUM_BALLS 15
#define MIN_R 20
#define MAX_R 200

#define GRAVITY 0.0f
#define BALL_DENSITY 0.001f
#define SLOP 0.05f
#define K_SLOP (float)(1.0f / std::sqrt(ITERATIONS))
#define RESTITUTION 1.0f
#define RESTING_THRESHOLD (GRAVITY * 1.5f)

struct BallInitData {
	Vector2 coord;
	Vector2 speed;
	float R;
	float mass;
	Color color;
	bool isImmovable;
};

class Ball {
public:
	Vector2 coord;
	Vector2 speed;
	float R;
	float mass;
	Color color; 
	bool isImmovable;
	bool showParentGridCell;

	Ball(Vector2 cd, Vector2 sp, float r, Color clr, bool immovable=false, bool showCell=false) {
		R = r;
		coord = cd;
		speed = sp;
		color = clr;
		isImmovable = immovable;
		mass = isImmovable ? 0.0f : (BALL_DENSITY * PI * r*r);
		showParentGridCell = showCell;
		ball = BallInitData { cd, sp, r, mass, clr, immovable };
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
	CollisionGrid() {
		size_t averageR = (MIN_R + MAX_R) / 2;
		size_t reserveSize = (size_t)((GRIDWIDTH * GRIDHEIGHT) / ((2*averageR) * (2*averageR)));
		for (size_t x = 0; x < GRIDSIZE_X; x++)
			for (size_t y = 0; y < GRIDSIZE_Y; y++)
				grid[x][y].reserve(reserveSize);
	}
};

Ball makeRandomBall(const std::vector<Ball>& balls) {
	size_t attempts = 0; 
    const size_t MAX_ATTEMPTS = 1000;
	float R;
	Vector2 coord;
	bool overlapping;
    do {
        overlapping = false;
        attempts++;

        R = GetRandomValue(MIN_R, MAX_R);
        coord = { (float)GetRandomValue(R+1, WIDTH-R-1), (float)GetRandomValue(R+1, HEIGHT-R-1) };
        for (const auto &ball : balls)
			if (CheckCollisionCircles(coord, R, ball.coord, ball.R)) {
				overlapping = true;
				break;
			}
        
		if (attempts > MAX_ATTEMPTS) {
			R = -1.0f; // dummy ball indicator
			break; 
		}
    } while (overlapping);

    float dx = GetRandomValue(-15, 15);
    float dy = GetRandomValue(-150, 150) / 15.0f;
    Vector2 speed = { dx, dy };
	
    unsigned char r = GetRandomValue(0, 255);
    unsigned char g = GetRandomValue(0, 255);
    unsigned char b = GetRandomValue(0, 255);
    Color color = Color { r, g, b, 255 };

    return Ball(coord, speed, R, color);
}

std::vector<Ball> makeRandomBalls(const int N) {
	std::vector<Ball> balls;
	for (size_t i = 0; i < N; i++) {
		Ball newBall = makeRandomBall(balls);
		if (newBall.R != -1.0f) { 
			balls.push_back(newBall); 
		}
		else { 
			continue; 
		}
	}
	return balls;
}

void clearGrid(CollisionGrid &cGrid) {
	for (size_t x = 0; x < GRIDSIZE_X; x++)
		for (size_t y = 0; y < GRIDSIZE_Y; y++)
			cGrid.grid[x][y].clear();
}

void populateGrid(CollisionGrid &cGrid, std::vector<Ball> &balls) {
	for (Ball &ball : balls) {
		size_t lCell = (size_t)Clamp((ball.coord.x - ball.R) / GRIDWIDTH,  0, GRIDSIZE_X-1);
		size_t rCell = (size_t)Clamp((ball.coord.x + ball.R) / GRIDWIDTH,  0, GRIDSIZE_X-1);
		size_t tCell = (size_t)Clamp((ball.coord.y - ball.R) / GRIDHEIGHT, 0, GRIDSIZE_Y-1);
		size_t bCell = (size_t)Clamp((ball.coord.y + ball.R) / GRIDHEIGHT, 0, GRIDSIZE_Y-1);
		
		for (size_t cellX = lCell; cellX <= rCell; cellX++) {
			for (size_t cellY = tCell; cellY <= bCell; cellY++) {
				cGrid.grid[cellX][cellY].push_back(&ball);
			}
		}
	}
}

void onCollision(Ball *b, Vector2 point) {
	Vector2 dist   = Vector2Subtract(point, b->coord);
	float distLen  = Vector2Length(dist);
	float pen      = b->R - distLen;
	Vector2 normal = Vector2Scale(dist, 1.0f / distLen);
	
	// Apply penetration slop 
	float correctionCoeff = std::max(0.0f, pen - SLOP) * K_SLOP;
	Vector2 correction = Vector2Scale(normal, correctionCoeff);
	
	if (!(b->mass)) return; // both the ball and the bound are immovable objects
	
	b->coord = Vector2Subtract(b->coord, correction);
	
	// Resolve speed based on masses and elastisity
	float normalVel = Vector2DotProduct(b->speed, normal);
	if (normalVel < 0.0f) return; // Do nothing if ball is are already moving away from the bound
	
	float impulseChange;
	if (normalVel > RESTING_THRESHOLD) {
		impulseChange = (1 + RESTITUTION) * normalVel;
	} 
	else { // RESTITUTION == 0.0f, meaning balls are resting and should not bounce
		impulseChange = normalVel;
	}
	
	b->speed = Vector2Subtract(b->speed, Vector2Scale(normal, impulseChange));
}

void onCollision(Ball *b1, Ball *b2) {
	float maxDist  = b1->R + b2->R;
	Vector2 dist   = Vector2Subtract(b2->coord, b1->coord);
	float distLen  = Vector2Length(dist);
	float pen      = maxDist - distLen;
	Vector2 normal = Vector2Scale(dist, 1.0f / distLen);
	
	// Apply penetration slop 
	float correctionCoeff = std::max(0.0f, pen - SLOP) * K_SLOP;
	Vector2 correction = Vector2Scale(normal, correctionCoeff);
	
	float invMass1 = b1->mass > 0.0f ? 1.0f / b1->mass : 0.0f;
	float invMass2 = b2->mass > 0.0f ? 1.0f / b2->mass : 0.0f;
	float totalInvMass = invMass1 + invMass2;
	if (!totalInvMass) return; // both balls are immovable objects 
	
	b1->coord = Vector2Subtract(b1->coord, Vector2Scale(correction, invMass1 / totalInvMass));
	b2->coord = Vector2Add     (b2->coord, Vector2Scale(correction, invMass2 / totalInvMass));
	
	// Resolve speed based on masses and elastisity
	Vector2 relVel = Vector2Subtract(b1->speed, b2->speed);
	float normalVel = Vector2DotProduct(relVel, normal);
	if (normalVel < 0.0f) return; // Do nothing if balls are already moving apart
	
	float impulseChange;
	if (normalVel > RESTING_THRESHOLD) {
		impulseChange = (1.0f / totalInvMass) * (1 + RESTITUTION) * normalVel;
	} 
	else { // RESTITUTION == 0.0f, meaning balls are resting and should not bounce
		impulseChange = (1.0f / totalInvMass) * normalVel;
	}
	
	b1->speed = Vector2Subtract(b1->speed, Vector2Scale(normal, invMass1 * impulseChange));
	b2->speed = Vector2Add     (b2->speed, Vector2Scale(normal, invMass2 * impulseChange));
}

void resolveCollisions(CollisionGrid &cGrid) {
	// Window bounds
	Vector2 lBound[] = { { 0,     0      }, { 0,     HEIGHT } };
	Vector2 rBound[] = { { WIDTH, 0      }, { WIDTH, HEIGHT } };
	Vector2 tBound[] = { { 0,     0      }, { WIDTH, 0      } };
	Vector2 bBound[] = { { 0,     HEIGHT }, { WIDTH, HEIGHT } };			
	for (size_t iter = 0; iter < ITERATIONS; iter++) {
		for (size_t x = 0; x < GRIDSIZE_X; x++) {
			for (size_t y = 0; y < GRIDSIZE_Y; y++) {
				auto &bucket = cGrid.grid[x][y];
				for (size_t i = 0; i < bucket.size(); i++) {
					// Collision type: Ball : lBound
					if (x == 0) {
						if (CheckCollisionCircleLine(bucket[i]->coord, bucket[i]->R, lBound[0], lBound[1])) {
							onCollision(bucket[i], { 0.0f, bucket[i]->coord.y });
						}
					}

					// Collision type: Ball : rBound
					if (x == GRIDSIZE_X-1) {
						if (CheckCollisionCircleLine(bucket[i]->coord, bucket[i]->R, rBound[0], rBound[1])) {
							onCollision(bucket[i], { WIDTH, bucket[i]->coord.y });
						}
					}
					
					// Collision type: Ball : tBound
					if (y == 0) {
						if (CheckCollisionCircleLine(bucket[i]->coord, bucket[i]->R, tBound[0], tBound[1])) {
							onCollision(bucket[i], { bucket[i]->coord.x, 0.0f });
						}
					}
					
					// Collision type: Ball : bBound
					if (y == GRIDSIZE_Y-1) {
						if (CheckCollisionCircleLine(bucket[i]->coord, bucket[i]->R, bBound[0], bBound[1])) {
							onCollision(bucket[i], { bucket[i]->coord.x, HEIGHT });
						}
					}
					
					if (bucket.size() > 1) {
						// Collision type: Ball : Ball
						for (size_t j = i + 1; j < bucket.size(); j++) {
							if (bucket[i] == bucket[j]) continue;
							if (CheckCollisionCircles(bucket[i]->coord, bucket[i]->R, bucket[j]->coord, bucket[j]->R)) {
								onCollision(bucket[i], bucket[j]);
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
	SetTargetFPS(FPS);
	SetRandomSeed(std::time(0));
	CollisionGrid cGrid{};
	
	Ball ball1 = Ball({ WIDTH/2,   HEIGHT/2   }, {   0, 0 }, 100, RED);
	Ball ball2 = Ball({ WIDTH/2,   HEIGHT-101 }, {   0, 0 }, 100, BLUE);
	Ball ball3 = Ball({ 101,       HEIGHT-351 }, {   8, 0 }, 100, YELLOW);
	Ball ball4 = Ball({ WIDTH-101, 101        }, { -10, 0 }, 50,  PURPLE);
	Ball ball5 = Ball({ 300,       300        }, {   0, 0 }, 75,  GRAY, true);
	std::vector<Ball> balls = { ball1, ball2, ball3, ball4, ball5 };
	balls = makeRandomBalls(NUM_BALLS);
	//for (auto &ball : balls) ball.showParentGridCell = true;
	
	while (!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(RAYWHITE);
		
		// Drawing the collison grid
		for (size_t x = 1; x <= GRIDSIZE_X; x++) {
			for (size_t y = 1; y <= GRIDSIZE_Y; y++) {
				Vector2 rowStart = { 0,     GRIDHEIGHT * (float)y };
				Vector2 rowEnd   = { WIDTH, GRIDHEIGHT * (float)y };
				DrawLineDashed(rowStart, rowEnd, 5, 5, LIGHTGRAY);
				
				Vector2 colStart = { GRIDWIDTH * (float)x, 0      };
				Vector2 colEnd   = { GRIDWIDTH * (float)x, HEIGHT };
				DrawLineDashed(colStart, colEnd, 5, 5, LIGHTGRAY);
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
			ball.speed.y += ball.isImmovable ? 0.0f : GRAVITY;
			ball.coord = Vector2Clamp(
				Vector2Add(ball.coord, ball.speed), 
				{ ball.R, ball.R }, 
				{ float(WIDTH-ball.R), float(HEIGHT-ball.R) }
			);
		}
		
		// Collision handling
		clearGrid(cGrid);
		populateGrid(cGrid, balls);
		resolveCollisions(cGrid);
		
		// Drawing balls
		for (Ball &ball : balls) {
			if (ball.showParentGridCell) {
				for (size_t x = 0; x < GRIDSIZE_X; x++) {
					for (size_t y = 0; y < GRIDSIZE_Y; y++) {
						const auto &bucket = cGrid.grid[x][y];
						for (const auto &el : bucket) {
							if (&ball == el) {
								int   w = GRIDWIDTH;
								int   h = GRIDHEIGHT;
								Color c = (c = ball.color, c.a = 64, c);
								DrawRectangle(x*w, y*h, w, h, c);
							}
						}
					}
				}
			}
			DrawCircleV(ball.coord, ball.R, ball.color);
		}
		
		// DrawText(TextFormat(" x: %s%f", balls[0].coord.x >= 0 ? " " : "", balls[0].coord.x), 20,           20,  30, BLACK);
		// DrawText(TextFormat(" y: %s%f", balls[0].coord.y >= 0 ? " " : "", balls[0].coord.y), 20,           50,  30, BLACK);
		// DrawText(TextFormat("dx: %s%f", balls[0].speed.x >= 0 ? " " : "", balls[0].speed.x), 20,           80,  30, BLACK);
		// DrawText(TextFormat("dy: %s%f", balls[0].speed.y >= 0 ? " " : "", balls[0].speed.y), 20,           110, 30, BLACK);
		EndDrawing();
	}
	CloseWindow();
	return 0;
}
