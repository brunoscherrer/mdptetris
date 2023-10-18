/**
 * Strategy evaluating a position with some features defined in a file.
 */

#ifndef STRATEGY_UCT_H
#define STRATEGY_UCT_H

#include "types.h"
#include "strategy.h"
#include "rewards.h"

Strategy *new_strategy_uct(char *feature_file);

#endif
