#include <windows.h>
#include <array>
#include <algorithm>
#include <vector>
#include <cstdlib>
#include <time.h>
#include "congl.h"

using namespace ConGL;
using namespace eng2D;
using namespace std;

#define MENU_STATE 0
#define GAME_STATE 1
#define SOLVE_STATE 2

Screen screen;
Layout frame(&screen);

int state = GAME_STATE;
int health = 3;
bool solved = false;

COORD selected = {0, 0};

array<array<int, 9>, 9> numbers;
array<array<int, 9>, 9> numbers_post;
array<array<int, 9>, 9> numbers_pre;
array<array<int, 9>, 9> numbers_wrong;
array<array<vector<int>, 9>, 9> psblt;


void renderNum(int num, COORD pos, COLOR col) {
	PIXEL px;
	px.ch = num ? to_wstring(num)[0] : L' ';
	px.pos = pos;
	px.col = col;

	screen.setPX(px);
}

void renderGrid() {
	PIXEL cross, vline, hline, hp;
	cross.ch = L'+';
	vline.ch = L'|';
	hline.ch = L'-';

	hp.ch = L'#';
	hp.col = colors::FG_RED;
	for (short x = 0; x < health; ++x) {
		hp.pos = {x, 0};
		screen.setPX(hp);
	}

	short yoff = 2;

	for (short y = 0; y < 10; ++y) {
		short ysk = y / 3;
		for (short x = 0; x < 10; ++x) {
			short xsk = x / 3;

			if (y <= 8 && x <= 8) {
				bool sl = selected.X == x && selected.Y == y;
				COORD pos = {x + xsk + 1, y + ysk + 1 + yoff};
				COLOR col = 0;
				
				if (sl)
					if (numbers_post[y][x]) {
						if (numbers_wrong[y][x]) col = colors::BG_RED;
						else col = colors::BG_GREEN;
					} else col = colors::BG_WHITE;				


				if (numbers_pre[y][x] || numbers_post[y][x]) {
					if (!col) {
						col = numbers_pre[y][x] ? colors::FG_WHITE : colors::FG_GREEN;
						col = numbers_wrong[y][x] ? colors::FG_RED : col;
					}
										
					int num = numbers_pre[y][x] ? numbers_pre[y][x] : numbers_post[y][x];
					renderNum(num, pos, col);			
				} else renderNum(0, pos, col);
			}
			
			if (x % 3 == 0 && y % 3 == 0) {
				cross.pos = {x + xsk, y + ysk + yoff};
				screen.setPX(cross);	
			}
			
			if (x % 3 == 0) {
				vline.pos = {x + xsk, y + ysk + 1 + yoff};
				screen.setPX(vline);
			}

			if (y % 3 == 0) {
				hline.pos = {x + xsk + 1, y + ysk + yoff};
				screen.setPX(hline);
			}
		}
	}
}

void calcPsblt() {
	for (int x = 0; x < 9; ++x) {
		int mx = x / 3;
		for (int y = 0; y < 9; ++y) {
			int my = y / 3;
			psblt[y][x].clear();

			if (numbers[y][x]) continue;

			for (int n = 1; n <= 9; ++n) {
				bool possible = true;
				
				for (int tx = 0; tx < 9; ++tx)
					if (numbers[y][tx] == n) {
						possible = false;
						break;
					}

				for (int ty = 0; ty < 9; ++ty) 
					if (numbers[ty][x] == n) {
						possible = false;
						break;
					}

				for (int tx = 0; tx < 3; ++tx) {
					if (false) br: break;
					int ttx = tx + mx * 3;
					for (int ty = 0; ty < 3; ++ty) {
						int tty = ty + my * 3;

						if (numbers[tty][ttx] == n) {
							possible = false;
							goto br;
						}
					}
				}

				if (!possible) continue;
				psblt[y][x].push_back(n);
			}
		}
	}
}

void initSolver() {
	health = 0;
	solved = false;

	for (int x = 0; x < 9; ++x)
		for (int y = 0; y < 9; ++y) {
			numbers[x][y] = 0;
			numbers_post[x][y] = 0;
			numbers_pre[x][y] = 0;
			numbers_wrong[x][y] = 0;
		} 
}

void clearSet() {
	for (int x = 0; x < 9; ++x) 
		for (int y = 0; y < 9; ++y) 
			if (numbers_post[y][x]) {
				numbers[y][x] = 0;
				numbers_post[y][x] = 0;
			}
}

void clearPost() {
	for (int x = 0; x < 9; ++x) 
		for (int y = 0; y < 9; ++y) 
			if (numbers_post[y][x])
				numbers_post[y][x] = 0;
}

bool canSolve() {
	for (int x = 0; x < 9; ++x) {
		int mx = x / 3;
		for (int y = 0; y < 9; ++y) {
			int my = y / 3;

			if (!numbers[y][x]) continue;
			int n = numbers[y][x];
			bool possible = true;				

			for (int tx = 0; tx < 9; ++tx) {
				if (tx == x) continue;
				if (numbers[y][tx] == n) {
					possible = false;
					break;
				}
			}

			for (int ty = 0; ty < 9; ++ty) { 
				if (ty == y) continue;
				if (numbers[ty][x] == n) {
					possible = false;
					break;
				}
			}

			for (int tx = 0; tx < 3; ++tx) {
				if (false) br: break;
				int ttx = tx + mx * 3;
				for (int ty = 0; ty < 3; ++ty) {
					int tty = ty + my * 3;
					if (ttx == x && tty == y) continue;
					if (numbers[tty][ttx] == n) {
						possible = false;
						goto br;
					}
				}
			}
			
			if (!possible) return false;
		}
	}

	return true;
}

void solve() {
	solved = true;

	r:
	calcPsblt();

	while(true) {
		COORD pmin = {-1, -1};
		size_t min = 9;

		for (short x = 0; x < 9; ++x) { 
			if (false) br: break;
			for (short y = 0; y < 9; ++y) { 
				vector<int>* v = &psblt[y][x];
				if (v->size() == 0) continue;
				if (v->size() < min) {
					min = v->size();
					pmin = {x, y};
					if (min == 1) goto br;
				}
			}
		}
		
		if (pmin.X == -1) {
			for (int x = 0; x < 9; ++x) 
				for (int y = 0; y < 9; ++y)
					if (!numbers[y][x]) {
						clearSet();
						goto r;
					}
			break;
		}

		int len = psblt[pmin.Y][pmin.X].size();
		int num = psblt[pmin.Y][pmin.X][rand() % len];
		numbers_post[pmin.Y][pmin.X] = num;
		numbers[pmin.Y][pmin.X] = num;
			
		calcPsblt();
	}
}

bool isEmpty() {
	for (int x = 0; x < 9; ++x) 
		for (int y = 0; y < 9; ++y)
			if (numbers[y][x]) return false;	

	return true;
}

void initGame() {
	health = 3;
	solved = false;

	int rx = rand() % 9, ry = rand() % 9;
	int rn = rand() % 9;	

	for (int x = 0; x < 9; ++x)
		for (int y = 0; y < 9; ++y) {
			numbers[x][y] = 0;
			numbers_post[x][y] = 0;
			numbers_pre[x][y] = 0;
			numbers_wrong[x][y] = 0;
		} 

	numbers[ry][rx] = rn;
	calcPsblt();
	solve();
	clearPost();
	solved = false;

	for (int n = 0; n < 25; ++n) {
		r: 
		rx = rand() % 9;
		ry = rand() % 9;
		if (numbers_pre[ry][rx]) goto r;

		numbers_pre[ry][rx] = numbers[ry][rx];
	}
}

void revealGame() {
	solved = true;
	
	for (int x = 0; x < 9; ++x) 
		for (int y = 0; y < 9; ++y) 
			if (!numbers_pre[y][x]) {
				numbers_post[y][x] = numbers[y][x];
				numbers_wrong[y][x] = 0;
			}
}

int main() {
	// init 
	screen.setSurfaceSize({13, 15});
	screen.setFont({20, 20});
	screen.toggleAutosize(true);
	screen._getHScreen()->threaded = false;

	srand(time(NULL));

	initGame();

	while (true) {
		// controls
		if (keys::released(VK_ESCAPE)) break;

		PIXEL ipx;
		ipx.pos = {12, 0};
		ipx.ch = state == GAME_STATE ? L'G' : L'S';
		
		if (state == GAME_STATE && keys::released(0x53)) {
			state = SOLVE_STATE;
			initSolver();
		}
		if (state == SOLVE_STATE && keys::released(0x47)) {
			state = GAME_STATE;
			initGame();	
		}
	
		short x = selected.X, y = selected.Y;
			
		if (x > 0 && keys::released(VK_LEFT)) --selected.X;
		if (x < 8 && keys::released(VK_RIGHT)) ++selected.X;
		if (y > 0 && keys::released(VK_UP)) --selected.Y;
		if (y < 8 && keys::released(VK_DOWN)) ++selected.Y;		
		
		switch (state) {
		case GAME_STATE:
			if (!health) initGame();			
			
			solved = true;
			for (int x = 0; x < 9; ++x) 
				for (int y = 0; y < 9; ++y)
					if (!numbers_post[y][x] && !numbers_pre[y][x]) solved = false;

			if (keys::released(VK_SPACE)) solved ? initGame() : revealGame();

			for (int n = 0; n < 10; ++n) {
				if (!keys::released(0x30 + n)) continue;
				if (numbers_pre[y][x] > 0) continue;
				if (!numbers_wrong[y][x] && numbers_post[y][x] > 0) continue;
				
				numbers_post[y][x] = n;
				
				if (n == 0 && numbers_wrong[y][x]) {
					numbers_wrong[y][x] = 0;
				} else if (numbers[y][x] == n) {
					numbers_wrong[y][x] = 0;
				} else {
					numbers_wrong[y][x] = n;
					--health;	
				}
				
			}

			break;
		case SOLVE_STATE: 
			if (keys::released(VK_SPACE)) {
				if (isEmpty());
				else if (solved) initSolver();
				else {
					if (canSolve()) solve();
					else {
						solved = true;
						for (int x = 0; x < 9; ++x) 
							for (int y = 0; y < 9; ++y) 
								if (numbers_pre[y][x]) {
									numbers_wrong[y][x] = 1;
								}
					}
				}
			}
			
			if (solved) break;	
		
			for (int n = 0; n < 9; ++n) {
				if (!keys::released(0x31 + n)) continue;
				int m = n + 1;

				numbers[y][x] = m;
				numbers_pre[y][x] = m;
			}

			if (keys::released(0x30)) {
				numbers[y][x] = 0;
				numbers_pre[y][x] = 0;
			}

		}

		renderGrid();
		screen.setPX(ipx);
		screen.render();
	}

	return 0;
}
