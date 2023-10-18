#include <math.h>
#include <stdlib.h>
#include "config.h"
#include "brick_masks.h"
#include "macros.h"
#include "simple_tetris.h"

#define ASSERT(b) do { if (!(b)) { printf("Assertion failed, file %s line %d\n", __FILE__, __LINE__); exit(1); }} while(0);

static void check_code(Game *game, uint32_t code) {
  set_game_state(game, code);
  ASSERT(code == get_game_code(game));
}

int main(int argc, char **argv) {
  int i;
  uint32_t s;
  Game *game;

  for (i = 0; i < 16; i++) {
    ASSERT(pow(2, i) == brick_masks[15 - i]);
    ASSERT((brick_masks[i] ^ brick_masks_inv[i]) == 0xFFFF);
  }

  game = new_game(WIDTH, HEIGHT, 0, "pieces_melax.dat", NULL);
  check_code(game, get_game_code(game));

  for (s = 0; s < NB_STATES; s++) {
    check_code(game, s);
  }

  return 0;
}
