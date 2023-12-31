/**
 * This header declares the main types defined in the modules.
 * Each header should include this one first to avoid cycling
 * dependencies other headers.
 */

#ifndef TYPES_H
#define TYPES_H

typedef struct Game Game;
typedef struct Action Action;
typedef struct Board Board;
typedef struct Piece Piece;
typedef struct PieceOrientation PieceOrientation;
typedef struct RewardDescription RewardDescription;
typedef struct Feature Feature;
typedef struct FeaturePolicy FeaturePolicy;
typedef struct GamesStatistics GamesStatistics;
typedef struct Strategy Strategy;
typedef struct CommonParameters CommonParameters;
typedef struct UctNode UctNode;

/**
 * @brief Function type for a feature.
 *
 * A feature function takes as parameter a game state
 * and returns a numeric value.
 * Note that this definition also fits for state-action feature functions,
 * as Game can be seen as a state-action pair when game->next_action is defined.
 */
typedef double (FeatureFunction)(Game *game);

#endif
