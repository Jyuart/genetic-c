#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "./constants.h"

#define NUM_BALLS 10
#define NUM_GENES 25

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
int game_is_running = FALSE;
float last_frame_time = 0;
int generations_count = 5;

typedef struct {
	float x;
	float y;
} position;

position goal;

typedef struct {
	position position;
	float width;
	float height;
	float last_changed_direction;
	int current_movement_index;
	position movements[NUM_GENES];
	float fitness;
} ball;

ball balls[NUM_BALLS];

int initialize_window() {
	if (SDL_Init(SDL_INIT_EVERYTHING)) {
		fprintf(stderr, "Error while initializing.\n");
		return FALSE;
	}
	window = SDL_CreateWindow(
		"Evolution",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		SDL_WINDOW_SHOWN
	);
	if (!window) {
		fprintf(stderr, "Error creating SDL Window.\n");
		return FALSE;
	}

	// the default one is -1
	int driver_index = -1;
	renderer = SDL_CreateRenderer(
		window,
		driver_index,
		0
	);
	if (!renderer) {
		fprintf(stderr, "Error creating renderer\n");
		return FALSE;
	}

	printf("All good\n");
	return TRUE;
}

void process_input() {
	SDL_Event event;
	SDL_PollEvent(&event);

	switch (event.type) {
		case SDL_QUIT:
			game_is_running = FALSE;
			break;
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE)
				game_is_running = FALSE;
			break;
	}
}

float float_rand(float min, float max) {
    float scale = rand() / (float) RAND_MAX;
    return min + scale * ( max - min );
}

void setup_ball_movements(position movements[]) {
	for (int i = 0; i < NUM_GENES; i++) {
		movements[i].x = float_rand(-1, 1) * 100;
		movements[i].y = float_rand(-1, 1) * 100;
		printf("%f, %f\n", movements[i].x, movements[i].y);
	}
	printf("\n\n");
}

void setup() {
	goal.x = WINDOW_WIDTH / 2;
	goal.y = (int)WINDOW_HEIGHT / 2;

	for (int i = 0; i < NUM_BALLS; i++) {
		balls[i].position.x = (float)WINDOW_WIDTH / 2;
		balls[i].position.y = (float)WINDOW_HEIGHT / 2;
		balls[i].width = 15;
		balls[i].height = 15;
		balls[i].last_changed_direction = 0.5f;
		balls[i].current_movement_index = 0;
		setup_ball_movements(balls[i].movements);
	}
}

void update_ball(ball *ball, float delta_time) {
	ball->last_changed_direction -= delta_time;

	if (ball->last_changed_direction < 0) {
		ball->last_changed_direction = 0.5f;
		ball->current_movement_index += 1;
	}

	ball->position.x += ball->movements[ball->current_movement_index].x * delta_time; 
	ball->position.y += ball->movements[ball->current_movement_index].y * delta_time; 
}

float calc_fitness(int x, int y) {
	return 1 - (hypot(goal.x - x, goal.y - y) / WINDOW_WIDTH);
}

void next_gen() {
	for (int i = 0; i < NUM_BALLS; i++) {
		balls[i].fitness = calc_fitness(balls[i].position.x, balls[i].position.y);
	}
	printf("\n");
}

void update() {
	// A method to cap the framerate (usually, useless if using delta_time);
	// while (!SDL_TICKS_PASSED(SDL_GetTicks(), last_frame_time + FRAME_TARGET_TIME));
	// last_frame_time = SDL_GetTicks();
	
	// A way to calc delta_time to make everything move consistently
	float delta_time = (SDL_GetTicks() - last_frame_time) / 1000.0f;
	last_frame_time = SDL_GetTicks();

	for (int i = 0; i < NUM_BALLS; i++) {
		if (balls[i].current_movement_index + 1 == NUM_GENES) {
			next_gen();
			break;
		}
		update_ball(&balls[i], delta_time);
	}
}

void render_ball(ball *ball) {
	SDL_Rect ball_rect = {
		ball->position.x,
		ball->position.y,
		ball->width,
		ball->height
	};

	SDL_SetRenderDrawColor(renderer, 123, 72, 12, 255);
	SDL_RenderFillRect(renderer, &ball_rect);
}

void render() {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);
	
	for (int i = 0; i < NUM_BALLS; i++) {
		render_ball(&balls[i]);
	}

	SDL_Rect goal_rect = {
		goal.x,
		goal.y,
		20,
		20
	};

	SDL_SetRenderDrawColor(renderer, 12, 42, 12, 255);
	SDL_RenderFillRect(renderer, &goal_rect); 

	SDL_RenderPresent(renderer);
}

void destroy_window() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}


int main() {
	game_is_running = initialize_window();

	setup();

	while (game_is_running) {
		process_input();
		update();
		render();
	}

	destroy_window();

	return 0;

}
