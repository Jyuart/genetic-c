#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <sodium/core.h>
#include <sodium/randombytes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sodium.h>
#include <SDL2/SDL.h>
#include "./constants.h"

#define NUM_BALLS 100
#define NUM_GENES 20
#define MUTATION_RATE 0.05


SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture *texture, *text_texture;
TTF_Font* font;
int game_is_running = FALSE;
float last_frame_time = 0;
int generations_count = 1;
float one_direction_move_duration = 0.5f;

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
ball candidates[NUM_BALLS * 100];
int candidates_len = 0;

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

float float_rand(int min, int max) {
	unsigned int r = randombytes_random();
	float normalized = (float) r / (float) UINT32_MAX;
	return min + normalized * (max - min);
}

void setup_ball_movements(position movements[]) {
	for (int i = 0; i < NUM_GENES; i++) {
		movements[i].x = float_rand(-1, 1) * 100;
		movements[i].y = float_rand(-1, 1) * 100;
	}
}

void setup(bool setup_random_movements) {
	goal.x = WINDOW_WIDTH - 20;
	goal.y = (int)(WINDOW_HEIGHT / 2);

	for (int i = 0; i < NUM_BALLS; i++) {
		balls[i].position.x = (float)WINDOW_WIDTH / 2;
		balls[i].position.y = (float)WINDOW_HEIGHT / 2;
		balls[i].width = 15;
		balls[i].height = 15;
		balls[i].last_changed_direction = one_direction_move_duration;
		balls[i].current_movement_index = 0;
		if (setup_random_movements) {
			setup_ball_movements(balls[i].movements);
		}
	}
}

void update_ball(ball *ball, float delta_time) {
	ball->last_changed_direction -= delta_time;

	ball->position.x += ball->movements[ball->current_movement_index].x * delta_time; 
	ball->position.y += ball->movements[ball->current_movement_index].y * delta_time; 
	
	if (ball->last_changed_direction < 0) {
		ball->last_changed_direction = one_direction_move_duration;
		ball->current_movement_index += 1;
	}
}

float calc_fitness(int x, int y) {
	return 1 - (hypot(goal.x - x, goal.y - y) / WINDOW_WIDTH);
}

void crossover(int idx) {
	int mom_idx = randombytes_uniform(candidates_len+1);
	int dad_idx = randombytes_uniform(candidates_len+1);
	for (int i = 0; i < NUM_GENES; i++) {
		if (i % 2 == 0) {
			balls[idx].movements[i] = candidates[mom_idx].movements[i];
		}
		else {
			balls[idx].movements[i] = candidates[dad_idx].movements[i];
		}
	}

}

void mutate(int idx) {
	for (int i = 0; i < NUM_GENES; i++) {
		if (float_rand(0, 1) < MUTATION_RATE) {
			printf("mutated");
			balls[idx].movements[i].x = float_rand(-1, 1) * 100;
			balls[idx].movements[i].y = float_rand(-1, 1) * 100;
		}
	}
}

void next_gen() {
	int first_empty_idx = 0;
	for (int i = 0; i < NUM_BALLS; i++) {
		balls[i].fitness = calc_fitness(balls[i].position.x, balls[i].position.y);
		int rounded_fintess = (int)round(balls[i].fitness * 100);
		for (int j = first_empty_idx; j < first_empty_idx + rounded_fintess; j++) {
			candidates[j] = balls[i];
			candidates_len += 1;
		}
		first_empty_idx += rounded_fintess;
	}

	setup(false);
	for (int i = 0; i < NUM_BALLS; i++) {
		crossover(i);
		mutate(i);
	}
	generations_count += 1;

	candidates_len = 0;
}

void update() {
	// A method to cap the framerate (usually, useless if using delta_time);
	// while (!SDL_TICKS_PASSED(SDL_GetTicks(), last_frame_time + FRAME_TARGET_TIME));
	// last_frame_time = SDL_GetTicks();
	
	// A way to calc delta_time to make everything move consistently
	float delta_time = (SDL_GetTicks() - last_frame_time) / 1000.0f;
	last_frame_time = SDL_GetTicks();

	for (int i = 0; i < NUM_BALLS; i++) {
		update_ball(&balls[i], delta_time);
	}
	if (balls[0].current_movement_index == NUM_GENES) {
		next_gen();
	}
}

void render_text(char text[]) {
	SDL_Rect dest;
	SDL_Color foreground = { 234, 12, 42 };
	SDL_Surface* text_surface = TTF_RenderText_Solid(font, text, foreground);
	text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
	dest.x = 10;
	dest.y = 10;
	dest.w = text_surface->w;
	dest.h = text_surface->h;
	SDL_RenderCopy(renderer, text_texture, NULL, &dest);

	SDL_DestroyTexture(text_texture);
	SDL_FreeSurface(text_surface);
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

	SDL_SetRenderDrawColor(renderer, 12, 42, 12, 255);
	char gen_text[3];
	sprintf(gen_text, "%d", generations_count);
	render_text(gen_text);

	SDL_RenderPresent(renderer);
}

void destroy_window() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}


int main() {
	game_is_running = initialize_window();
	setup(true);
	if (TTF_Init() < 0) {
		fprintf(stderr, "Error initializing SDL_ttf.\n");
	}
	font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 24);
	if (!font) {
		fprintf(stderr, "Error initializing font.\n");
	}

	if (sodium_init() < 0) {
		// panic! the library couldn't be initialized, it is not safe to use
		return 0;
	}
	while (game_is_running) {
		process_input();
		update();
		render();
	}

	destroy_window();

	return 0;

}
