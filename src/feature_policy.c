#include <stdlib.h>
#include <stdio.h>
#include "config.h"
#include "feature_policy.h"
#include "feature_functions.h"
#include "game.h"
#include "games_statistics.h"
#include "brick_masks.h"
#include "file_tools.h"
#include "macros.h"



/**
 * @brief Returns whether or a feature policy contains a given feature.
 * @param feature_policy the features
 * @param feature_id id of the feature to search
 * @return 1 if this feature is present, 0 otherwise
 */
int contains_feature(const FeaturePolicy *feature_policy, FeatureID feature_id) {

  int i;
  int found = 0;
  for (i = 0; i < feature_policy->nb_features && !found; i++) {
    found = (feature_policy->features[i].feature_id == feature_id);
  }

  return found;
}

/**
 * @brief Evaluates a game state using a set of features.
 * @param game the current game state or state-action pair
 * @param feature_policy the feature-based policy
 * @return the evaluation of the state with the features
 */
double evaluate_features(Game *game, const FeaturePolicy *feature_policy) {
  double rating = -1; /* unknown */
  int i, nb_features;
  Feature *feature;

  if (game->game_over && feature_policy->gameover_evaluation == 0) {
      rating = 0;
  }
  else if (game->game_over && feature_policy->gameover_evaluation == -1) {
      rating = -TETRIS_INFINITE;
  }
  else {               /* general case: evaluate with the features */
    if (feature_policy->update_column_heights_needed) {
      board_update_column_heights(game->board);
    }
    if (feature_policy->compute_next_action_needed) {
      features_compute_next_action(game, feature_policy);

      if (game->next_state->game_over && feature_policy->gameover_evaluation == 0) {
	rating = 0;
      }
      else if (game->next_state->game_over && feature_policy->gameover_evaluation == -1) {
	rating = -TETRIS_INFINITE;
      }
    }
    
    if (rating == -1) {
      nb_features = feature_policy->nb_features;
      rating = 0;
      for (i = 0; i < nb_features; i++) {
	feature = &feature_policy->features[i];
	rating += feature->get_feature_rating(game) * feature->weight;
      }
    }
  }

  return rating;
}

/**
 * @brief Returns the values of each feature in a state.
 *
 * The function returns an array with the values of each feature for the current state.
 * The game should not be over.
 *
 * @param game the current game state or state-action pair
 * @param feature_policy the feature-based policy
 * @return the value of each feature in this state
 */
double *get_feature_values(Game *game, const FeaturePolicy *feature_policy) {

  int i;
  double *feature_values;

  MALLOCN(feature_values, double, feature_policy->nb_features);
  
  if (feature_policy->update_column_heights_needed) {
    board_update_column_heights(game->board);
  }

  if (feature_policy->compute_next_action_needed) {
    features_compute_next_action(game, feature_policy);
  }

/*   printf("\n---------\n"); */
/*   print_board(stdout, game->board); */

  for (i = 0; i < feature_policy->nb_features; i++) {
    feature_values[i] = feature_policy->features[i].get_feature_rating(game);
/*     printf("feature %d (id = %d): %f\n", i, feature_policy->features[i].feature_id, feature_values[i]); */
  }
/*   getchar(); */

  return feature_values;
}

/**
 * @brief Chooses the best action in a state.
 *
 * The action is chosen with respect to a feature set and a reward function.
 * Every action is tried, then each resulting state is evaluated.
 * This evaluation is added to the reward obtained. The action that
 * maximizes this value is selected.
 *
 * If the features are defined over the state-action space, this function just calls features_get_best_action_sa.
 * Thus, you can call this function in both cases.
 *
 * @param game the current game state 
 * @param feature_policy the feature based policy
 * @param best_action pointer to store the best action found
 */
void features_get_best_action(Game *game, const FeaturePolicy *feature_policy, Action *best_action) {
  int nb_possible_orientations, nb_possible_columns, i, j;
  double evaluation, best_evaluation;
  Action action;

  if (feature_policy->compute_next_action_needed) {
    features_get_best_action_sa(game, feature_policy, best_action);
    return;
  }

  best_evaluation = -TETRIS_INFINITE;
  best_action->orientation = 0;
  best_action->column = 1;

  /* try every possible action */
  nb_possible_orientations = game_get_nb_possible_orientations(game);
  for (i = 0; i < nb_possible_orientations; i++) {
    action.orientation = i;
    nb_possible_columns = game_get_nb_possible_columns(game, i);
    for (j = 1; j <= nb_possible_columns; j++) {

      action.column = j;

      /* make the action */
      game_drop_piece(game, &action, 1, feature_policy);
      
      /* compute immediate reward + evaluation of next state */

      /* debug
      if (feature_policy->reward_description.reward_function(game) != 0) {
	printf("oups: reward = %d\n", feature_policy->reward_description.reward_function(game));
      }
      */

      evaluation = feature_policy->reward_description.reward_function(game) + evaluate_features(game, feature_policy);
      
      /*board_print(stdout, game->board);*/
      /*game_print_features(game, feature_policy);*/
      /*printf("orientation %d, column %d: evaluation = %e\n", action.orientation, action.column, evaluation);*/

      

      if (DOUBLE_GREATER_THAN(evaluation,best_evaluation)) {
	best_evaluation = evaluation;
	best_action->orientation = i;
	best_action->column = j;
      }
      game_cancel_last_move(game);
    }
  }
}

/**
 * @brief Chooses the best action in a state.
 *
 * The action is chosen with respect to a feature set where the feature functions are defined on state-action pairs.
 *
 * @param game the current game state
 * @param feature_policy the feature based policy
 * @param best_action pointer to store the best action found
 */
void features_get_best_action_sa(Game *game, const FeaturePolicy *feature_policy, Action *best_action) {

  int nb_possible_orientations, nb_possible_columns, i, j;
  double evaluation, best_evaluation;

  best_evaluation = -TETRIS_INFINITE;
  best_action->orientation = 0;
  best_action->column = 1;

  /* try every possible action */
  nb_possible_orientations = game_get_nb_possible_orientations(game);
  for (i = 0; i < nb_possible_orientations; i++) {
    game->next_action.orientation = i;
    nb_possible_columns = game_get_nb_possible_columns(game, i);
    for (j = 1; j <= nb_possible_columns; j++) {
      game->next_action.column = j;
      evaluation = evaluate_features(game, feature_policy);
      
      if (DOUBLE_GREATER_THAN(evaluation, best_evaluation)) {
	best_evaluation = evaluation;
	best_action->orientation = i;
	best_action->column = j;
      }
    }
  }
}

/**
 * @brief Plays a game with a feature policy.
 * @param feature_policy the feature policy
 * @param game a game object (will be reseted at the beginning of the game)
 */
void feature_policy_play_game(const FeaturePolicy *feature_policy, Game *game) {

  Action action;
  game_reset(game);

  while (!game->game_over) {
    /* debug 
    game_print(stdout, game);
    game_print_features(game, feature_policy);
      */

    /* search the best move */
    features_get_best_action(game, feature_policy, &action);

    /* play the best move found earlier */
    game_drop_piece(game, &action, 0, feature_policy);

  }
}

/**
 * @brief Play some games to evaluate a feature policy.
 * @param feature_policy the feature policy
 * @param nb_games number of games to play
 * @param game a game object (will be reseted at the beginning of each game)
 * @param stats a statistics object, to save the results (NULL to save nothing)
 * @param print_scores 1 to print the score of each game on the standard output, 0 otherwise
 * @return the mean score of the games
 */
double feature_policy_play_games(const FeaturePolicy *feature_policy, int nb_games, Game *game,
				 GamesStatistics *stats, int print_scores) {

  double mean_score;
  int i;

  mean_score = 0;
  for (i = 0; i < nb_games; i++) {
    feature_policy_play_game(feature_policy, game);
    if (print_scores) {
      printf("%d ", game->score);
      fflush(stdout);
    }

    if (stats != NULL) {
      games_statistics_add_game(stats, game->score);
    }
    else {
      mean_score += game->score;
    }
  }
  
  if (stats != NULL) {
    mean_score = stats->mean;
    games_statistics_end_episode(stats, feature_policy);
  }
  else {
    mean_score /= nb_games;
  }

  return mean_score;  
}

/**
 * This function is called when the features need to compute the state resulting to an action.
 * @param game the current state and action
 * @param feature_policy the features
 */
void features_compute_next_action(Game *game, const FeaturePolicy *feature_policy) {

  Game *next_state = game->next_state;

  /*printf("compute next state for action: orientation = %d, column = %d\n", game->next_action.orientation, game->next_action.column);*/


  /* we have to fill the next_state field */
  if (game->next_state == NULL) {
    /* allocate memory only once */
    next_state = new_game_copy(game);
  }
  else {
    game->next_state = NULL;
    game_copy(game, next_state);
  }

  /* do the action */
  game->next_state = next_state;
  game_drop_piece(game->next_state, &game->next_action, 0, feature_policy);

  /* also compute the column heights for the next state, if needed */
  if (feature_policy->update_column_heights_needed) {
    board_update_column_heights(game->next_state->board);
  }

/*  printf("done\n");*/
}

/**
 * @brief Loads the feature based policy and the initial weights from a file.
 * @param feature_file_name file to read (this file will be searched in the
 * current directory and then in the data directory)
 * @param feature_policy object to store the information read
 * @see save_feature_policy(), unload_feature_policy()
 */
void load_feature_policy(const char *feature_file_name, FeaturePolicy *feature_policy) {
  FILE *feature_file;
  int reward_function_id, gameover_evaluation;
  int i, nb_features;
  int update_column_heights_needed, compute_next_action_needed;
  int feature_id;
  double weight;
  Feature *features;

  feature_file = open_data_file(feature_file_name, "r");

  if (feature_file == NULL) {
    DIE1("Cannot read the feature file '%s'", feature_file_name);
  }

  /* read the reward function id */
  FSCANF(feature_file, "%d", &reward_function_id);

  /* read the gameover evaluation value */
  FSCANF(feature_file, "%d", &gameover_evaluation);


  /* read the number of features */
  FSCANF(feature_file, "%d", &nb_features);
  MALLOCN(features, Feature, nb_features);
  
  /* read each feature and its weight */
  update_column_heights_needed = 0;
  compute_next_action_needed = 0;
  for (i = 0; i < nb_features; i++) {
    FSCANF2(feature_file, "%d %lf", &feature_id, &weight);
    features[i].feature_id = feature_id;
    features[i].weight = weight;

    /* see whether we need to compute the column heights */
    switch (feature_id) {
    case NEXT_COLUMN_HEIGHT:
    case NEXT_COLUMN_HEIGHT_DIFFERENCE:
    case MAX_HEIGHT_DIFFERENCE:
    case HEIGHT_DIFFERENCES_SUM:
    case MEAN_HEIGHT:
    case SA_HEIGHT_DIFFERENCES_SUM_CHANGE:
    case SA_MEAN_HEIGHT_CHANGE:
    case SA_WALL_HEIGHT_CHANGE:
    case NEXT_WALL_DISTANCE_TO_TOP:
    case NEXT_COLUMN_HEIGHT_DIFFERENCE2:
    case NEXT_COLUMN_HEIGHT2:
    case DIVERSITY:
      update_column_heights_needed = 1;
      break;

    default:
      break;
    }

    /* see whether we need to compute a next state */
    switch (feature_id) {
    case SA_HEIGHT_DIFFERENCES_SUM_CHANGE:
    case SA_MEAN_HEIGHT_CHANGE:
    case SA_WALL_HEIGHT_CHANGE:
    case SA_HOLES_CHANGE:
    case SA_LINES:
      compute_next_action_needed = 1;

    default:
      break;
    }


  }
  
  fclose(feature_file);

  /* load the feature functions */
  for (i = 0; i < nb_features; i++) {
    feature_id = features[i].feature_id;
    features[i].get_feature_rating = feature_function(feature_id);
  }

  feature_policy->gameover_evaluation = gameover_evaluation;
  feature_policy->reward_description.reward_function_id = reward_function_id;
  feature_policy->reward_description.reward_function = all_reward_functions[reward_function_id];
  feature_policy->features = features;
  feature_policy->nb_features = nb_features;
  feature_policy->update_column_heights_needed = update_column_heights_needed;
  feature_policy->compute_next_action_needed = compute_next_action_needed;

  /* initialize the features system for these feature functions */
  features_initialize(feature_policy);
}

/**
 * @brief Saves the feature based policy and their weights into a file.
 * @param feature_file_name the file to write (the file name is relative to current directory)
 * @param feature_policy the feature_policy to save into the file
 * @see load_feature_policy()
 */
void save_feature_policy(const char *feature_file_name, const FeaturePolicy *feature_policy) {
  FILE *feature_file;
  int i;

  feature_file = fopen(feature_file_name, "w");

  if (feature_file == NULL) {
    DIE1("Unable to write the feature file '%s'", feature_file_name);
  }

  /* write the reward function id */
  fprintf(feature_file, "%d\n", feature_policy->reward_description.reward_function_id);

  /* write the gameover evaluation value */
  fprintf(feature_file, "%d\n", feature_policy->gameover_evaluation);

  /* write the number of features */
  fprintf(feature_file, "%d\n", feature_policy->nb_features);

  /* write each feature and its weight */
  for (i = 0; i < feature_policy->nb_features; i++) {
    fprintf(feature_file, "%d %e\n", feature_policy->features[i].feature_id, feature_policy->features[i].weight);
  }

  fclose(feature_file);
}

/**
 * @brief Unloads a feature policy.
 *
 * This function frees the memory used by the internal fields of the FeaturePolicy structure.
 * The feature_policy pointer is not deleted, as load_feature_policy() does not allocate memory for it.
 *
 * @param feature_policy the feature_policy to unload
 * @see load_feature_policy()
 */
void unload_feature_policy(FeaturePolicy *feature_policy) {

  FREE(feature_policy->features);
}

/**
 * @brief Displays the weights of the features.
 *
 * A feature is represented on a line with two values: the feature id and the weight.
 *
 * @param feature_policy the feature policy
 */
void print_features(const FeaturePolicy *feature_policy) {
  int i;

  for (i = 0; i < feature_policy->nb_features; i++) {
    printf("%d->%f ", feature_policy->features[i].feature_id, feature_policy->features[i].weight);
  }
  printf("\n");
}

/**
 * @brief Displays the values of the features in a given state.
 *
 * This function is useful for debugging/watching.
 *
 * @param game the current game state
 * @param feature_policy the feature policy to use
 */
void game_print_features(Game *game, FeaturePolicy *feature_policy) {
  int i;
  double *feature_values;

  printf("Features: ");
  
  feature_values = get_feature_values(game, feature_policy);

  for (i = 0; i < feature_policy->nb_features; i++) {
    printf("%i->%f ",feature_policy->features[i].feature_id , feature_values[i]);
  }
  printf("=> Value: %f\n", evaluate_features(game, feature_policy));

  FREE(feature_values);
}
