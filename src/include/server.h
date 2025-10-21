#ifndef SERVER_H
#define SERVER_H

#include <cstdlib>
#include <iostream>
#include <vector>
#include <queue>

/*
 * You may need to define some global variables for the information of the game map here.
 * Although we don't encourage to use global variables in real cpp projects, you may have to use them because the use of
 * class is not taught yet. However, if you are member of A-class or have learnt the use of cpp class, member functions,
 * etc., you're free to modify this structure.
 */
int rows;         // The count of rows of the game map. You MUST NOT modify its name.
int columns;      // The count of columns of the game map. You MUST NOT modify its name.
int total_mines;  // The count of mines of the game map. You MUST NOT modify its name. You should initialize this
                  // variable in function InitMap. It will be used in the advanced task.
int game_state;  // The state of the game, 0 for continuing, 1 for winning, -1 for losing. You MUST NOT modify its name.

// Internal game state
static std::vector<std::vector<bool>> mine_grid;         // true if mine
static std::vector<std::vector<int>> adjacent_mines;     // number of adjacent mines
static std::vector<std::vector<bool>> visited_grid;      // visited status
static std::vector<std::vector<bool>> marked_grid;       // marked status
static int visited_non_mine_count = 0;                   // number of visited non-mine cells
static int marked_correct_mines_count = 0;               // number of correctly marked mines

static inline bool InBounds(int r, int c) {
  return r >= 0 && r < rows && c >= 0 && c < columns;
}

static inline void RecomputeGameWinState() {
  const int total_non_mines = rows * columns - total_mines;
  if (visited_non_mine_count == total_non_mines) {
    game_state = 1;
  }
}

static void FloodVisitFrom(int r0, int c0) {
  // Visit a non-mine cell; expand if zero using BFS
  std::queue<std::pair<int, int>> bfs_queue;
  auto try_visit = [&](int r, int c) {
    if (!InBounds(r, c)) return;
    if (mine_grid[r][c]) return;          // never visit mines here
    if (visited_grid[r][c]) return;
    if (marked_grid[r][c]) return;        // marked cells are not auto-visited
    visited_grid[r][c] = true;
    ++visited_non_mine_count;
    if (adjacent_mines[r][c] == 0) {
      bfs_queue.emplace(r, c);
    }
  };

  try_visit(r0, c0);
  while (!bfs_queue.empty()) {
    auto [r, c] = bfs_queue.front();
    bfs_queue.pop();
    static const int dr[8] = {-1,-1,-1,0,0,1,1,1};
    static const int dc[8] = {-1,0,1,-1,1,-1,0,1};
    for (int k = 0; k < 8; ++k) {
      int nr = r + dr[k], nc = c + dc[k];
      if (!InBounds(nr, nc)) continue;
      if (mine_grid[nr][nc]) continue;
      if (visited_grid[nr][nc]) continue;
      if (marked_grid[nr][nc]) continue;
      visited_grid[nr][nc] = true;
      ++visited_non_mine_count;
      if (adjacent_mines[nr][nc] == 0) {
        bfs_queue.emplace(nr, nc);
      }
    }
  }
  RecomputeGameWinState();
}

/**
 * @brief The definition of function InitMap()
 *
 * @details This function is designed to read the initial map from stdin. For example, if there is a 3 * 3 map in which
 * mines are located at (0, 1) and (1, 2) (0-based), the stdin would be
 *     3 3
 *     .X.
 *     ...
 *     ..X
 * where X stands for a mine block and . stands for a normal block. After executing this function, your game map
 * would be initialized, with all the blocks unvisited.
 */
void InitMap() {
  std::cin >> rows >> columns;
  mine_grid.assign(rows, std::vector<bool>(columns, false));
  adjacent_mines.assign(rows, std::vector<int>(columns, 0));
  visited_grid.assign(rows, std::vector<bool>(columns, false));
  marked_grid.assign(rows, std::vector<bool>(columns, false));
  visited_non_mine_count = 0;
  marked_correct_mines_count = 0;
  total_mines = 0;
  game_state = 0;

  // Read map lines
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      char ch;
      std::cin >> ch;
      mine_grid[i][j] = (ch == 'X');
      if (mine_grid[i][j]) ++total_mines;
    }
  }

  // Precompute adjacent mine counts
  static const int dr[8] = {-1,-1,-1,0,0,1,1,1};
  static const int dc[8] = {-1,0,1,-1,1,-1,0,1};
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      int cnt = 0;
      for (int k = 0; k < 8; ++k) {
        int ni = i + dr[k], nj = j + dc[k];
        if (InBounds(ni, nj) && mine_grid[ni][nj]) ++cnt;
      }
      adjacent_mines[i][j] = cnt;
    }
  }
}

/**
 * @brief The definition of function VisitBlock(int, int)
 *
 * @details This function is designed to visit a block in the game map. We take the 3 * 3 game map above as an example.
 * At the beginning, if you call VisitBlock(0, 0), the return value would be 0 (game continues), and the game map would
 * be
 *     1??
 *     ???
 *     ???
 * If you call VisitBlock(0, 1) after that, the return value would be -1 (game ends and the players loses) , and the
 * game map would be
 *     1X?
 *     ???
 *     ???
 * If you call VisitBlock(0, 2), VisitBlock(2, 0), VisitBlock(1, 2) instead, the return value of the last operation
 * would be 1 (game ends and the player wins), and the game map would be
 *     1@1
 *     122
 *     01@
 *
 * @param r The row coordinate (0-based) of the block to be visited.
 * @param c The column coordinate (0-based) of the block to be visited.
 *
 * @note You should edit the value of game_state in this function. Precisely, edit it to
 *    0  if the game continues after visit that block, or that block has already been visited before.
 *    1  if the game ends and the player wins.
 *    -1 if the game ends and the player loses.
 *
 * @note For invalid operation, you should not do anything.
 */
void VisitBlock(int r, int c) {
  if (game_state != 0) return;          // game already ended
  if (!InBounds(r, c)) return;          // invalid operation
  if (visited_grid[r][c]) return;       // already visited
  if (marked_grid[r][c]) return;        // marked has no effect
  if (mine_grid[r][c]) {
    // Visiting a mine ends the game immediately
    visited_grid[r][c] = true;
    game_state = -1;
    return;
  }
  FloodVisitFrom(r, c);
}

/**
 * @brief The definition of function MarkMine(int, int)
 *
 * @details This function is designed to mark a mine in the game map.
 * If the block being marked is a mine, show it as "@".
 * If the block being marked isn't a mine, END THE GAME immediately. (NOTE: This is not the same rule as the real
 * game) And you don't need to
 *
 * For example, if we use the same map as before, and the current state is:
 *     1?1
 *     ???
 *     ???
 * If you call MarkMine(0, 1), you marked the right mine. Then the resulting game map is:
 *     1@1
 *     ???
 *     ???
 * If you call MarkMine(1, 0), you marked the wrong mine(There's no mine in grid (1, 0)).
 * The game_state would be -1 and game ends immediately. The game map would be:
 *     1?1
 *     X??
 *     ???
 * This is different from the Minesweeper you've played. You should beware of that.
 *
 * @param r The row coordinate (0-based) of the block to be marked.
 * @param c The column coordinate (0-based) of the block to be marked.
 *
 * @note You should edit the value of game_state in this function. Precisely, edit it to
 *    0  if the game continues after visit that block, or that block has already been visited before.
 *    1  if the game ends and the player wins.
 *    -1 if the game ends and the player loses.
 *
 * @note For invalid operation, you should not do anything.
 */
void MarkMine(int r, int c) {
  if (game_state != 0) return;          // game already ended
  if (!InBounds(r, c)) return;          // invalid operation
  if (visited_grid[r][c]) return;       // already visited -> no effect
  if (marked_grid[r][c]) return;        // already marked -> no effect

  if (mine_grid[r][c]) {
    marked_grid[r][c] = true;
    ++marked_correct_mines_count;
    // marking does not change win condition directly
  } else {
    // Marking a non-mine causes immediate failure
    marked_grid[r][c] = true; // keep for rendering 'X'
    game_state = -1;
  }
}

/**
 * @brief The definition of function AutoExplore(int, int)
 *
 * @details This function is designed to auto-visit adjacent blocks of a certain block.
 * See README.md for more information
 *
 * For example, if we use the same map as before, and the current map is:
 *     ?@?
 *     ?2?
 *     ??@
 * Then auto explore is available only for block (1, 1). If you call AutoExplore(1, 1), the resulting map will be:
 *     1@1
 *     122
 *     01@
 * And the game ends (and player wins).
 */
void AutoExplore(int r, int c) {
  if (game_state != 0) return;
  if (!InBounds(r, c)) return;
  if (!visited_grid[r][c]) return;      // only for visited non-mine cells
  if (mine_grid[r][c]) return;

  // Count marked neighbors and compare with the number on this cell
  int required = adjacent_mines[r][c];
  int marked_neighbors = 0;
  static const int dr[8] = {-1,-1,-1,0,0,1,1,1};
  static const int dc[8] = {-1,0,1,-1,1,-1,0,1};
  for (int k = 0; k < 8; ++k) {
    int nr = r + dr[k], nc = c + dc[k];
    if (!InBounds(nr, nc)) continue;
    if (marked_grid[nr][nc]) ++marked_neighbors;
  }
  if (marked_neighbors != required) return;  // not eligible

  // Visit all non-mine neighbors (not marked, not visited)
  for (int k = 0; k < 8; ++k) {
    int nr = r + dr[k], nc = c + dc[k];
    if (!InBounds(nr, nc)) continue;
    if (mine_grid[nr][nc]) continue;
    if (visited_grid[nr][nc]) continue;
    if (marked_grid[nr][nc]) continue;
    FloodVisitFrom(nr, nc);
    if (game_state != 0) return;  // may win here
  }
}

/**
 * @brief The definition of function ExitGame()
 *
 * @details This function is designed to exit the game.
 * It outputs a line according to the result, and a line of two integers, visit_count and marked_mine_count,
 * representing the number of blocks visited and the number of marked mines taken respectively.
 *
 * @note If the player wins, we consider that ALL mines are correctly marked.
 */
void ExitGame() {
  if (game_state == 1) {
    std::cout << "YOU WIN!" << std::endl;
    std::cout << visited_non_mine_count << " " << total_mines << std::endl;
  } else if (game_state == -1) {
    std::cout << "GAME OVER!" << std::endl;
    std::cout << visited_non_mine_count << " " << marked_correct_mines_count << std::endl;
  } else {
    // Should not happen in normal flow, but keep a fallback
    std::cout << "GAME OVER!" << std::endl;
    std::cout << visited_non_mine_count << " " << marked_correct_mines_count << std::endl;
  }
  exit(0);  // Exit the game immediately
}

/**
 * @brief The definition of function PrintMap()
 *
 * @details This function is designed to print the game map to stdout. We take the 3 * 3 game map above as an example.
 * At the beginning, if you call PrintMap(), the stdout would be
 *    ???
 *    ???
 *    ???
 * If you call VisitBlock(2, 0) and PrintMap() after that, the stdout would be
 *    ???
 *    12?
 *    01?
 * If you call VisitBlock(0, 1) and PrintMap() after that, the stdout would be
 *    ?X?
 *    12?
 *    01?
 * If the player visits all blocks without mine and call PrintMap() after that, the stdout would be
 *    1@1
 *    122
 *    01@
 * (You may find the global variable game_state useful when implementing this function.)
 *
 * @note Use std::cout to print the game map, especially when you want to try the advanced task!!!
 */
void PrintMap() {
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      char out_char = '?';
      if (game_state == 1) {
        // Victory: reveal all mines as '@', others are their digits
        if (mine_grid[i][j]) {
          out_char = '@';
        } else {
          out_char = static_cast<char>('0' + adjacent_mines[i][j]);
        }
      } else {
        // Ongoing or failure
        if (visited_grid[i][j]) {
          if (mine_grid[i][j]) {
            out_char = 'X';
          } else {
            out_char = static_cast<char>('0' + adjacent_mines[i][j]);
          }
        } else if (marked_grid[i][j]) {
          // Marked but not visited
          if (mine_grid[i][j]) {
            out_char = '@';
          } else {
            out_char = 'X';  // wrong mark appears as X
          }
        } else {
          out_char = '?';
        }
      }
      std::cout << out_char;
    }
    std::cout << std::endl;
  }
}

#endif
