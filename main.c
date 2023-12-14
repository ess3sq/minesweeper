#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <raylib.h>

const unsigned board_height = 1080;
const unsigned board_width = board_height;

const unsigned footer_height = 50;
const unsigned footer_width = board_width;

const unsigned nrows = 20;
const unsigned ncols = nrows;

#define CHEAT_MODE false // for debug purposes
bool show_bombs = false;

typedef struct {
	bool hidden;
	bool hasbomb;
	bool hasflag;
	unsigned bombs_close_by;
	unsigned x, y;
} Cell;

int tiles_left = 0;
int flags_set = 0;
int total_mines = 0;

Cell *get_cell(Cell *grid, unsigned row, unsigned col) {
	return &grid[ncols * row + col];
}

void draw_grid(Cell *grid, Texture2D bomb, Texture2D flag) {
	Color hintcolor[9] = {RAYWHITE, DARKGREEN, LIME, GREEN, GOLD, ORANGE, RED, PURPLE, DARKPURPLE};

	ClearBackground(DARKGRAY);
	unsigned rectwidth = board_width/ncols;
	unsigned rectheight = board_height/nrows;

	for (unsigned x = 0; x < ncols; ++x) {
		for (unsigned y = 0; y < nrows; ++y) {
			Cell *cell = get_cell(grid, y, x);
			if (show_bombs && cell->hasbomb && cell->hidden) {
				DrawRectangle(rectwidth * x, rectheight * y, rectwidth, rectheight, DARKGRAY);
				DrawTexture(bomb, rectwidth * x, rectheight * y, DARKGRAY);
			}
			if (cell->hidden && cell->hasflag) {
				DrawTexture(flag, rectwidth * x, rectheight * y, DARKGRAY);
				continue;
			}
			if (!cell->hidden) {
				DrawRectangle(rectwidth * x, rectheight * y, rectwidth, rectheight, RAYWHITE);
				if (cell->hasbomb) {
					DrawRectangle(rectwidth * x, rectheight * y, rectwidth, rectheight, RED);
					DrawTexture(bomb, rectwidth * x, rectheight * y, RED);
				} else {
					DrawRectangle(rectwidth * x, rectheight * y, rectwidth, rectheight, RAYWHITE);
					if (cell->bombs_close_by == 0) continue;
					char txtbuf[2];
					unsigned nbombs = cell->bombs_close_by > 8 ? 8 : cell->bombs_close_by;
					sprintf(txtbuf, "%u", nbombs);
					DrawText(txtbuf, rectwidth * x + rectwidth/3, rectheight * y + rectheight/3, 25, hintcolor[nbombs]);
				}
			}
		}
	}

	for (unsigned i = 0; i <= nrows; ++i) {
		int rowheight = board_height / nrows * i;
		DrawLine(0, rowheight, board_width, rowheight, BLACK);
	}

	for (unsigned j = 0; j <= ncols; ++j) {
		int colwidth = board_width / ncols * j;
		DrawLine(colwidth, 0, colwidth, board_height, BLACK);
	}
}

void draw_footer(int game_status) {
	// positive: won, zero: running, negative: lost
	if (game_status > 0) {
		DrawText("You win!", footer_height / 2, board_height + footer_height / 3, 25, GREEN);
	} else if (game_status < 0) {
		DrawText("You lose!", footer_height / 2, board_height + footer_height / 3, 25, RED);
	} else  {
		char mine_count_msg[100];
		sprintf(mine_count_msg, "Mine count: %d", total_mines - flags_set);
		DrawText(mine_count_msg, footer_height / 2, board_height + footer_height / 3, 25, GOLD);
	}

#if CHEAT_MODE
	DrawText("Cheat mode active: press R to reveal", footer_width / 2, board_height + footer_height / 3, 25, RED);
#endif
}

void choose_initial_cell(Cell *grid) {
	unsigned w = ncols - 4;
	unsigned h = nrows - 4;
	int rw = rand() % w;
	int rh = rand() % h;
	Cell *c = get_cell(grid, rh + 2, rw + 2);
	if (c->hasbomb) ++tiles_left;
	c->hidden = false;
	c->hasbomb = false;
	--tiles_left; // we are revealing the first cell
}

Cell *generate_grid() {
	tiles_left = nrows * ncols;

	Cell *grid = malloc(nrows * ncols * sizeof(Cell));
	if (!grid) {
		perror("error: allocation failed");
		exit(2);
	}

	// DEFAULT: 20% of cells are bombs // TODO: allow setting percentage
	unsigned prob_percent = 20;
	for (unsigned x = 0; x < ncols; ++x) {
		for (unsigned y = 0; y < nrows; ++y) {
			Cell *c = get_cell(grid, y, x);
			c->x = x;
			c->y = y;
			c->hidden = true;
			c->hasflag = false;
			c->hasbomb = (unsigned) rand() % 100 < prob_percent;
			if (c->hasbomb) --tiles_left;
			c->bombs_close_by = 0;
		}
	}

	choose_initial_cell(grid);

	for (unsigned x = 0; x < ncols; ++x) {
		for (unsigned y = 0; y < nrows; ++y) {
			Cell *c = get_cell(grid, y, x);
			if (!c->hasbomb) continue;
			if (x > 0) {
				if (y > 0) get_cell(grid, y-1, x-1)->bombs_close_by++;
				get_cell(grid, y, x-1)->bombs_close_by++;
				if (y < nrows - 1) get_cell(grid, y+1, x-1)->bombs_close_by++;
			}
			if (x < ncols-1) {
				if (y > 0) get_cell(grid, y-1, x+1)->bombs_close_by++;
				get_cell(grid, y, x+1)->bombs_close_by++;
				if (y < nrows - 1) get_cell(grid, y+1, x+1)->bombs_close_by++;
			}
			if (y > 0) get_cell(grid, y-1, x)->bombs_close_by++;
			if (y < nrows - 1) get_cell(grid, y+1, x)->bombs_close_by++;
		}
	}

	total_mines = nrows * ncols - tiles_left;

	return grid;
}

Cell *get_cell_at_mouse_pos(Cell *grid) {
	Vector2 mouse = GetMousePosition();
	unsigned cell_height = board_height / nrows;
	unsigned cell_row = (unsigned) (mouse.y / cell_height);
	unsigned cell_width = board_width / ncols;
	unsigned cell_col = (unsigned) (mouse.x / cell_width);
	return get_cell(grid, cell_row, cell_col);
}

void print_help() {
	fprintf(stderr, "a simple minesweeper game\n"
					"options:\n"
					"--help           display help and exit\n"
					"--seed <seed>    set random seed\n");
}

void parse_args(int argc, char **argv) {
	bool expecting_seed = false;
	char *seed = NULL;
	for (int i = 1; i < argc; ++i) {
		if (*argv[i] == '-' && expecting_seed) {
			fprintf(stderr, "error: expected seed, found option '%s'\n", argv[i]);
			exit(1);
		} else if (!strcmp(argv[i], "--help")) {
			print_help();
			exit(1);
		} else if (!strcmp(argv[i], "--seed")) {
			if (seed) {
				fprintf(stderr, "error: can only specify seed once\n");
				exit(1);
			}
			expecting_seed = true;
		} else if (*argv[i] == '-') {
			fprintf(stderr, "error: invalid option '%s'\n", argv[i]);
			exit(1);
		} else {
			// arg
			if (!expecting_seed) {
				fprintf(stderr, "error: unexpected argument '%s'\n", argv[i]);
				exit(1);	
			}
			seed = argv[i];
			expecting_seed = false;
		}
	}

	if (expecting_seed) {
		fprintf(stderr, "error: expected seed, found nothing\n");
		exit(1);
	}

	if (!seed) {
		srand(time(NULL));
	} else {
		char *endptr;
		long x = strtol(seed, &endptr, 10);
		if (x < 0 || endptr != seed + strlen(seed)) {
			fprintf(stderr, "error: --seed requires positive integer\n");
			exit(1);
		}
		srand(x % INT_MAX);
	}
}

int main(int argc, char **argv)
{
	parse_args(argc, argv);

	Cell *grid = generate_grid();
	bool exploded = false;

	InitWindow(board_width, board_height + footer_height, "Minesweeper");
	SetTargetFPS(30);
	Texture2D bomb = LoadTexture("assets/bomb.png");
	Texture2D flag = LoadTexture("assets/flag.png");

	while (!WindowShouldClose()) {
		BeginDrawing();
		draw_grid(grid, bomb, flag);
		draw_footer(exploded ? -1 : (tiles_left <= 0 ? 1 : 0));
		EndDrawing();

#if CHEAT_MODE
		if (IsKeyPressed(KEY_R)) {
			show_bombs = !show_bombs;
		}
#endif

		if (exploded || tiles_left <= 0) continue; // game over

		if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
			Cell *cell = get_cell_at_mouse_pos(grid);
			if (cell->hidden) cell->hasflag = !cell->hasflag;
			flags_set += cell->hasflag ? 1 : -1;
	
		}
		
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			Cell *cell = get_cell_at_mouse_pos(grid);

			if (cell->hasflag)  continue;
			if (cell->hidden) {
				cell->hidden = false;
				if (!cell->hasbomb) --tiles_left;
			}
			if (cell->hasbomb) {
				exploded = true;
			}
		}
}

	free(grid);
	return 0;
}

