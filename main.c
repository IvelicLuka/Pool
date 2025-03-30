#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "raylib.h"

#define FACTOR 120
#define WIDTH (16*FACTOR)
#define HEIGHT (9*FACTOR)
#define BOARD_WIDTH (0.75*WIDTH)
#define BOARD_HEIGHT (0.5*BOARD_WIDTH)
#define RAD 25
#define HOLE 60
#define HOLE_C 6
#define TRENJE 0.0005f
#define SHOT_SPEED 1.5f
#define JUMP 0.1f
#define STICK_DISTANCE 300
#define PTR_LEN 500
#define BALL_SPACING (RAD*1.5)
#define THICKNESS 10
#define POINT_SIZE 15
#define COLS 5
#define BALLS_DISPLAY ((COLS)*(COLS+1)/2)
#define BALL_C (BALLS_DISPLAY+1)
#define DISPLAY_WIDTH (2*RAD*BALLS_DISPLAY)
#define DISPLAY_MARGIN ((HEIGHT-BOARD_HEIGHT)/4.0 - RAD)
#define PHYSICS_FRAMES 10


Rectangle board = {
		WIDTH/2 - BOARD_WIDTH/2,
		HEIGHT/2 - BOARD_HEIGHT/2,
		BOARD_WIDTH,
		BOARD_HEIGHT	
};

Rectangle display_box = {
		WIDTH/2 - DISPLAY_WIDTH/2,
		DISPLAY_MARGIN,
		DISPLAY_WIDTH,
		2*RAD	
};

Vector2 holes[] = {
	{0, 0},
	{0, BOARD_HEIGHT},
	{BOARD_WIDTH, 0},
	{BOARD_WIDTH/2, 0},
	{BOARD_WIDTH, BOARD_HEIGHT},
	{BOARD_WIDTH/2, BOARD_HEIGHT}
};

typedef struct {
	Vector2 pos;
	Vector2 vel;
	int pune;
	Color c;
	int active;
} ball;

void BallCollision(Vector2 *pos1, Vector2 *pos2, Vector2 *vel1, Vector2 *vel2) {
	Vector2 dv = {vel1->x - vel2->x, vel1->y - vel2->y};
	Vector2 dx = {pos1->x - pos2->x, pos1->y - pos2->y};
	
	Vector2 nv1 = {
		vel1->x - ((dv.x*dx.x+dv.y*dx.y)/(dx.x*dx.x+dx.y*dx.y))*dx.x,
		vel1->y - ((dv.x*dx.x+dv.y*dx.y)/(dx.x*dx.x+dx.y*dx.y))*dx.y
	};
	dv.x *= -1;dv.y *= -1;
	dx.x *= -1;dx.y *= -1;
	Vector2 nv2 = {
		vel2->x - ((dv.x*dx.x+dv.y*dx.y)/(dx.x*dx.x+dx.y*dx.y))*dx.x,
		vel2->y - ((dv.x*dx.x+dv.y*dx.y)/(dx.x*dx.x+dx.y*dx.y))*dx.y
	};
//	pos1->x -= vel1->x;
//	pos1->y -= vel1->y;
//	pos2->x -= vel2->x;
//	pos2->y -= vel2->y;
	
	pos1->x -= dx.x/(2*RAD);
	pos1->y -= dx.y/(2*RAD);
	pos2->x -= -dx.x/(2*RAD);
	pos2->y -= -dx.y/(2*RAD);
	*vel1 = nv1;
	*vel2 = nv2;
}

void EdgeCollisions(Vector2* ball, Vector2 *ballvel) {
	for (int i = 0; i < HOLE_C; i++) {
		if (CheckCollisionCircles(*ball, RAD, holes[i], HOLE - RAD)) return;
	}
	int h = 0;
	if(ball->y < RAD)   				{if(!h){h=1;ball->x -= ballvel->x;ball->y -= ballvel->y;}ballvel->y*=-1;ball->y -= -JUMP;}
	if(ball->x < RAD) 					{if(!h){h=1;ball->x -= ballvel->x;ball->y -= ballvel->y;}ballvel->x*=-1;ball->x -= -JUMP;}
	if(ball->y > board.height - RAD)	{if(!h){h=1;ball->x -= ballvel->x;ball->y -= ballvel->y;}ballvel->y*=-1;ball->y -=  JUMP;}
	if(ball->x > board.width  - RAD)	{if(!h){h=1;ball->x -= ballvel->x;ball->y -= ballvel->y;}ballvel->x*=-1;ball->x -=  JUMP;}
}

void DrawBall(float x, float y, float r, Color c, int pune) {
	DrawCircle(x + board.x, y + board.y, r, c);
	if (pune == 1) DrawBall(x, y, RAD/2, WHITE, 0);
}
void DrawDisplayed(float x, float y, float r, Color c, int pune) {
	DrawCircle(x + display_box.x + RAD, y + display_box.y + RAD, r, c);
	if (pune == 1) DrawDisplayed(x, y, RAD/2, WHITE, 0);
}

int main(void) {
	srand(time(NULL));
	InitWindow(WIDTH, HEIGHT, "pool");
	ball balls[BALL_C] = { 0 };
	Vector2 start = {board.width*0.75, board.height/2};
    Vector2 whitestart = (Vector2){ board.width/5, board.height/2 };
	ball display[BALLS_DISPLAY] = { 0 };
	int removed = 0;
#if 0
	for (int i = 0; i < BALL_C; i++) {
		balls[i].pos.x = (float) rand() / RAND_MAX * (board.width-2*RAD) + RAD;
		balls[i].pos.y = (float) rand() / RAND_MAX * (board.height-2*RAD)+ RAD;
		balls[i].vel.x = ((float) rand() / RAND_MAX - 0.5) * SHOT_SPEED;
		balls[i].vel.y = ((float) rand() / RAND_MAX - 0.5) * SHOT_SPEED;
		balls[i].c     = ColorFromHSV((float) rand() / RAND_MAX * 360, 1, 1);
		balls[i].active = 1;
	}
#endif
	SetTargetFPS(60);		
	int standing, playing;
	Vector2 mouse;
	while(!WindowShouldClose()) {
		for (int physics_repeat = 0; physics_repeat < PHYSICS_FRAMES; physics_repeat++) {
			standing = 1;
			playing = 0;
			for (int i = 0; i < BALL_C; i++) {
				if (!balls[i].active) continue;
				if (i != 0) playing = 1;
				if (balls[i].vel.x != 0 || balls[i].vel.y != 0) standing = 0;
				balls[i].pos.x += balls[i].vel.x;
				balls[i].pos.y += balls[i].vel.y;
				EdgeCollisions(&balls[i].pos, &balls[i].vel);
			}
			for (int i = 0; i < BALL_C; i++) {
				for (int j = i+1; j < BALL_C; j++) {
					if (!balls[i].active) continue;
					if (!balls[j].active) continue;
					if (CheckCollisionCircles(balls[i].pos, RAD, balls[j].pos, RAD)) {
						BallCollision(&balls[i].pos, &balls[j].pos, &balls[i].vel, &balls[j].vel);
					}
				}
			}
			for (int i = 0; i < BALL_C; i++) {
				for (int j = 0; j < HOLE_C; j++) {
					if (CheckCollisionCircles(balls[i].pos, RAD, holes[j], 10)) {
						if (!balls[i].active) continue;
						balls[i].active = 0;
						if (i == 0) continue;
						display[removed++] = balls[i];
					}
				}
			}
			if (!balls[0].active && standing) {
				balls[0].active = 1;
				balls[0].pos = whitestart; 
				balls[0].vel = (Vector2){ 0, 0 };	
			}

			mouse = GetMousePosition();
			if (standing) {
				if (IsKeyPressed(KEY_SPACE)) {
					mouse = GetMousePosition();
					balls[0].vel.x = (balls[0].pos.x + board.x - mouse.x) / STICK_DISTANCE; 
					balls[0].vel.y = (balls[0].pos.y + board.y - mouse.y) / STICK_DISTANCE;
					float len = sqrtf(balls[0].vel.x*balls[0].vel.x + balls[0].vel.y*balls[0].vel.y);
					if (len < 1) len = 1;

					balls[0].vel.x /= len;
					balls[0].vel.y /= len;

					balls[0].vel.x *= SHOT_SPEED;
					balls[0].vel.y *= SHOT_SPEED;
				}
				//balls[0].vel.x = (rand()%6-1) * SHOT_SPEED;
				//balls[0].vel.y = (rand()%6-1) * SHOT_SPEED;
			}
#if 1
			for (int i = 0; i < BALL_C; i++) {
				if (!balls[i].active) continue;
				float mag = sqrtf(balls[i].vel.x*balls[i].vel.x + balls[i].vel.y*balls[i].vel.y);
				if (mag > TRENJE) {
					balls[i].vel.x -= TRENJE*balls[i].vel.x/mag/2;
					balls[i].vel.y -= TRENJE*balls[i].vel.y/mag/2;
				}
				if (-TRENJE <= balls[i].vel.x && balls[i].vel.x <= TRENJE && -TRENJE <= balls[i].vel.y && balls[i].vel.y <= TRENJE) {
					balls[i].vel.x = 0.0f;
					balls[i].vel.y = 0.0f;
				}
			}
#endif
#if 0
			Vector2 avgvel;
			avgvel.x = 0;
			avgvel.y = 0;
			for (int i = 0; i < BALL_C; i++) {
				if (!balls[i].active) continue;
				avgvel.x += balls[i].vel.x;
				avgvel.y += balls[i].vel.y;
			}
			printf("%f %f %f\n", avgvel.x, avgvel.y, sqrtf(avgvel.x*avgvel.x+avgvel.y*avgvel.y)/BALL_C);
#endif
			if (!playing) {
				balls[0].active = 1;
				balls[0].pos = whitestart; 
				balls[0].vel = (Vector2){ 0, 0 };	
				int k = 1;
				for (int i = 0; i < COLS; i++) {
					for (int j = 0; j < i+1; j++) {
						balls[k].pos.x = start.x + i * BALL_SPACING;
						balls[k].pos.y = start.y + (2*j-i) * BALL_SPACING*0.75;
						balls[k].vel.x = 0.0f;
						balls[k].vel.y = 0.0f;
						balls[k].c     = ColorFromHSV((float) rand() / RAND_MAX * 360, 1, 1);
						balls[k].active = 1;
						balls[k].pune   = rand()%2 + 1;
						k++;
					}
				}
				removed = 0;
			}
		}
		BeginDrawing();
			ClearBackground(RAYWHITE);
			DrawRectangleRec(board, LIME);
			DrawBall(whitestart.x, whitestart.y, POINT_SIZE, BLACK, 0);
			for (int i = 0; i < HOLE_C; i++) {
				DrawBall(holes[i].x, holes[i].y, HOLE, GRAY, 0);
				DrawBall(holes[i].x, holes[i].y, HOLE-RAD*2, BLACK, 0);
			}
			if (standing) {
				DrawBall(balls[0].pos.x, balls[0].pos.y, STICK_DISTANCE, PINK, 0);
				float dx = balls[0].pos.x + board.x - mouse.x;
				float dy = balls[0].pos.y + board.y - mouse.y;
				float d  = sqrtf(dx*dx + dy*dy);
				if (d < 1) d = 1;
				dx /= d; dy /= d;
				dx *= PTR_LEN; dy *= PTR_LEN;
				DrawLineEx((Vector2){balls[0].pos.x+board.x, balls[0].pos.y+board.y}, (Vector2){balls[0].pos.x + board.x + dx, balls[0].pos.y + board.y + dy}, THICKNESS, BLACK);
			}
			for (int i = 0; i < BALL_C; i++) {
				if (!balls[i].active) continue;
				if (i != 0) {
					DrawBall(balls[i].pos.x, balls[i].pos.y, RAD, balls[i].c, balls[i].pune);
					//if (balls[i].pune == 1) DrawBall(balls[i].pos.x, balls[i].pos.y, RAD/2, WHITE);
				}
				else 
				DrawBall(balls[i].pos.x, balls[i].pos.y, RAD, WHITE, 0);
			}

			DrawRectangleRec(display_box, BLACK);
			for (int i = 0; i < removed; i++) {
				DrawDisplayed(i * 2*RAD, 0, RAD, display[i].c, display[i].pune);
			}
		EndDrawing();
	}
	CloseWindow();
}
