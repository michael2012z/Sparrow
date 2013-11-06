#include <stdio.h>
#include <stdlib.h>

int sudoku[9][9];

void sudoku_init();
void sudoku_generate();
int check(int x, int y, int c);
int try(int x, int y, int *candidates[9][9]);
void sudoku_resolve();
void sudoku_dump();


void main() {
  printf("\nSudoku generator and resolver demo\n");
  sudoku_init();
  printf("\ngenerating ... \n");
  sudoku_generate();
  printf("\ngenerated:\n");
  sudoku_dump();
  printf("\nresolving ... \n");
  sudoku_resolve();
  printf("\nresolved: \n");
  sudoku_dump();
  exit(0);
}

void sudoku_dump() {
  int i, j;

  for (i = 0; i < 9; i++) {
    if (0 == i%3)
      printf("+---+---+---+\n");
    for(j = 0; j < 9; j++) {
      if (0 == j%3)
	printf("|");
      printf("%d", sudoku[i][j]);
    }
    printf("|\n");
  }
  printf("+---+---+---+\n\n");
}

void sudoku_init() {
  int i, j;
  for (i = 0; i < 9; i++)
	for(j = 0; j < 9; j++)
	  sudoku[i][j] = 0;
}

void sudoku_generate() {
  int i, j, loc;
  for(i = 1; i <= 9; i++) {
	loc = random()%9;
	for (j = 0; j < 9; j++)
	  if (sudoku[0][(loc+j)%9] == 0) {
		sudoku[0][(loc+j)%9] = i;
		break;
	  } else
		continue;
  }

  sudoku_resolve();

  for (i = 0; i < 9; i++) {
    for(j = 0; j < 9; j++) {
      if ((random()%100) > 20)
	sudoku[i][j] = 0;
    }
  }

}

int check(int x, int y, int c) {
  int i, j;
  //printf("%s: x = %d, y = %d, c= %d\n", __func__, x, y, c);
  for (i = 0; i < 9; i++)
	if (c == sudoku[x][i])
	  return 0;

  //  //printf("%s: flag 1\n", __func__);
  
  for (i = 0; i < 9; i++)
	if (c == sudoku[i][y])
	  return 0;

  //  //printf("%s: flag 2\n", __func__);

  for (i = 0; i < 3; i++)
	for (j = 0; j < 3; j++)
	  if (c == sudoku[(x/3)*3 + i][(y/3)*3 + j])
		return 0;

  //  //printf("%s: flag 3\n", __func__);

  return 1;
}

int try(int x, int y, int *candidates[9][9]) {
  int k, m;
  int ok;
  int next_x = x, next_y = (y + 1)%9;

  //printf("%s:%d x = %d, y = %d, candidates = ", __func__, __LINE__, x, y);
  //  for (k = 0; k < 9; k++)
    //printf("%d", candidates[x][y][k]);
  //printf("\nsudoku:\n");
  //  sudoku_dump();

  if (0 != sudoku[x][y]) {
	for (k = 0; k < 9; k++)
	  candidates[x][y][k] = 0;
	if (0 == next_y)
	  next_x ++;
	if (9 == next_x)
	  return 1;
	for (m = 0; m < 9; m++)
	  candidates[next_x][next_y][m] = m+1;
	return try(next_x, next_y, candidates);
  } else {
	for (k = 0; k < 9; k++) {
	  if (0 == candidates[x][y][k])
		continue;
	  ok = check(x, y, candidates[x][y][k]);
	  if (0 == ok) {
	    //printf("%s:%d x = %d, y = %d, '%d' not possible here, k = %d\n", __func__, __LINE__, x, y, candidates[x][y][k], k);
		continue;
	  } else {
	    //printf("%s:%d x = %d, y = %d, '%d' possible here, k = %d\n", __func__, __LINE__, x, y, candidates[x][y][k], k);
		sudoku[x][y] = candidates[x][y][k];
		candidates[x][y][k] = 0;
		if (0 == next_y)
		  next_x ++;
		if (9 == next_x)
		  return 1;
		for (m = 0; m < 9; m++)
		  candidates[next_x][next_y][m] = m+1;
		ok = try(next_x, next_y, candidates);
		if (0 == ok) {
		  //printf("%s:%d x = %d, y = %d, try '%d' failed, k = %d \n", __func__, __LINE__, x, y, sudoku[x][y], k);
		  sudoku[x][y] = 0;
		  continue;
		} else
		  return 1;
	  }
	}
	return 0;
  }
}

void sudoku_resolve() {
  int *candidates[9][9];
  int i, j, k;
  for (i = 0; i < 9; i++)
	for(j = 0; j < 9; j++) {
	  candidates[i][j] = (int *)malloc(9*sizeof(int));
	  if (0 == candidates[i][j]) {
		printf("malloc failure\n");
		exit(0);
	  } else {
		for (k = 0; k < 9; k++)
		  candidates[i][j][k] = k+1;
	  }
	}
  
  k = try(0, 0, candidates);

  for (i = 0; i < 9; i++)
    for(j = 0; j < 9; j++) {
      free(candidates[i][j]);
    }
}
