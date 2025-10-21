#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <utility>
#include <vector>
#include <string>

extern int rows;         // The count of rows of the game map.
extern int columns;      // The count of columns of the game map.
extern int total_mines;  // The count of mines of the game map.

// You MUST NOT use any other external variables except for rows, columns and total_mines.

/**
 * @brief The definition of function Execute(int, int, bool)
 *
 * @details This function is designed to take a step when player the client's (or player's) role, and the implementation
 * of it has been finished by TA. (I hope my comments in code would be easy to understand T_T) If you do not understand
 * the contents, please ask TA for help immediately!!!
 *
 * @param r The row coordinate (0-based) of the block to be visited.
 * @param c The column coordinate (0-based) of the block to be visited.
 * @param type The type of operation to a certain block.
 * If type == 0, we'll execute VisitBlock(row, column).
 * If type == 1, we'll execute MarkMine(row, column).
 * If type == 2, we'll execute AutoExplore(row, column).
 * You should not call this function with other type values.
 */
void Execute(int r, int c, int type);

/**
 * @brief The definition of function InitGame()
 *
 * @details This function is designed to initialize the game. It should be called at the beginning of the game, which
 * will read the scale of the game map and the first step taken by the server (see README).
 */
// Client-side observed map of the current game state
static std::vector<std::string> observed_map;

void InitGame() {
  // Initialize all client-side global states
  observed_map.assign(rows, std::string(columns, '?'));
  int first_row, first_column;
  std::cin >> first_row >> first_column;
  Execute(first_row, first_column, 0);
}

/**
 * @brief The definition of function ReadMap()
 *
 * @details This function is designed to read the game map from stdin when playing the client's (or player's) role.
 * Since the client (or player) can only get the limited information of the game map, so if there is a 3 * 3 map as
 * above and only the block (2, 0) has been visited, the stdin would be
 *     ???
 *     12?
 *     01?
 */
// Client-side observed map of the current game state
void ReadMap() {
  observed_map.resize(rows);
  for (int r = 0; r < rows; ++r) {
    std::string line;
    std::cin >> line;
    // Defensive: ensure size matches columns
    if (static_cast<int>(line.size()) < columns) {
      line.resize(columns, '?');
    }
    observed_map[r] = line.substr(0, columns);
  }
}

/**
 * @brief The definition of function Decide()
 *
 * @details This function is designed to decide the next step when playing the client's (or player's) role. Open up your
 * mind and make your decision here! Caution: you can only execute once in this function.
 */
static inline bool in_bounds(int r, int c) { return r >= 0 && r < rows && c >= 0 && c < columns; }

static void neighbors(int r, int c, std::vector<std::pair<int, int>> &out) {
  static const int dr[8] = {-1,-1,-1,0,0,1,1,1};
  static const int dc[8] = {-1,0,1,-1,1,-1,0,1};
  out.clear();
  for (int k = 0; k < 8; ++k) {
    int nr = r + dr[k], nc = c + dc[k];
    if (in_bounds(nr, nc)) out.emplace_back(nr, nc);
  }
}

void Decide() {
  // Strategy: one action per Decide.
  // 1) For any revealed number cell, if marked neighbors equals the number -> auto-explore it
  // 2) If for any revealed number, unknown + marked == number -> mark an unknown neighbor
  // 3) Otherwise, visit the first unknown cell we find

  std::vector<std::pair<int, int>> nbrs;

  // Step 1: try auto-explore on any satisfied number cell
  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < columns; ++c) {
      char ch = observed_map[r][c];
      if (ch >= '0' && ch <= '8') {
        int number_required = ch - '0';
        neighbors(r, c, nbrs);
        int marked_count = 0;
        int unknown_count = 0;
        for (auto [nr, nc] : nbrs) {
          char v = observed_map[nr][nc];
          if (v == '@') ++marked_count;
          else if (v == '?') ++unknown_count;
        }
        if (marked_count == number_required && unknown_count > 0) {
          Execute(r, c, 2);  // auto-explore
          return;
        }
      }
    }
  }

  // Step 2: try to mark an obvious mine
  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < columns; ++c) {
      char ch = observed_map[r][c];
      if (ch >= '0' && ch <= '8') {
        int number_required = ch - '0';
        neighbors(r, c, nbrs);
        int marked_count = 0;
        int unknown_count = 0;
        for (auto [nr, nc] : nbrs) {
          char v = observed_map[nr][nc];
          if (v == '@') ++marked_count;
          else if (v == '?') ++unknown_count;
        }
        if (marked_count + unknown_count == number_required && unknown_count > 0) {
          // Mark the first unknown neighbor
          for (auto [nr, nc] : nbrs) {
            if (observed_map[nr][nc] == '?') {
              Execute(nr, nc, 1);
              return;
            }
          }
        }
      }
    }
  }

  // Step 3: visit a cell. Prefer unknown adjacent to a revealed zero, else any unknown.
  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < columns; ++c) {
      if (observed_map[r][c] == '0') {
        neighbors(r, c, nbrs);
        for (auto [nr, nc] : nbrs) {
          if (observed_map[nr][nc] == '?') {
            Execute(nr, nc, 0);
            return;
          }
        }
      }
    }
  }

  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < columns; ++c) {
      if (observed_map[r][c] == '?') {
        Execute(r, c, 0);
        return;
      }
    }
  }
}

#endif