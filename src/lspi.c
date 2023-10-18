/**
 * This module executes the least-squares policy iteration algorithm.
 *
 *
 * You can stop the algorithm with Ctrl-C. The current iteration
 * is then terminated. The policy is saved
 * into the feature based policy file you specified in the command line.
 *
 * This feature file can be used to continue the execution later
 * or to play in the simulator (./tetris).
 */

#include <stdio.h>
#include <math.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_linalg.h>
#include "lspi_parameters.h"
#include "random.h"
#include "feature_policy.h"
#include "feature_functions.h"
#include "games_statistics.h"
#define NB_GAMES_TO_EVALUATE 100

/**
 * Information about one sample (s, a, r, s') or (s, a) generated.
 */
typedef struct Sample {

  Game *state;            /* s */
  Action action;          /* a */
  gsl_vector *features;   /* \phi(s,a) */
  int next_piece_index;   /* next piece obtained (useful to compute the next state) */

  /*
  gsl_vector *features;    the feature values in this state 
  Action action;           the action played 
  int reward;              the reward obtained by simulation 

  Game *next_state;        copy of the full game state obtained after playing action a in state s 
  */

} Sample;

/* functions to start / end the algorithm */
static void initialize_lspi(LSPIParameters *parameters);
static void exit_lspi(LSPIParameters *parameters);

/* the algorithm itself */
static void lspi(LSPIParameters *parameters);
static void lstdq(LSPIParameters *parameters, Sample *samples, const gsl_vector *weights, gsl_vector *next_weights);
static void update_matrices_fp(LSPIParameters *parameters, const Sample *sample, Game *game,
    const gsl_vector *weights, gsl_matrix *a, gsl_vector *b);
static void update_matrices_br(LSPIParameters *parameters, const Sample *sample, Game *game,
    const gsl_vector *weights, gsl_matrix *a, gsl_vector *b);

static void feature_policy_set_weights(FeaturePolicy *feature_policy, const gsl_vector *weights);
static void feature_policy_get_weights(const FeaturePolicy *feature_policy, gsl_vector *weights);
static void feature_policy_get_values(FeaturePolicy *feature_policy, gsl_vector *weights, Game *game);
static double get_distance(const gsl_vector *a, const gsl_vector *b);
static double dot_product(const gsl_vector *a, const gsl_vector *b);

static Sample * generate_samples(LSPIParameters *parameters);
static Sample * load_samples(LSPIParameters *parameters);
static void save_samples(LSPIParameters *parameters, Sample *samples);

/**
 * Initializes the algorithm.
 * This function has to be called before lspi() is called.
 * The parameters should have been set first.
 */
static void initialize_lspi(LSPIParameters *parameters) {

  /* initialize the random number generator */
  initialize_random_generator(parameters->common_parameters.random_generator_seed);
  
  /* load the policy */
  load_feature_policy(parameters->samples_policy_file_name, &parameters->samples_policy);

  /* initialize the feature functions system */
  features_initialize(&parameters->samples_policy);
}

/**
 * Frees the memory.
 */
static void exit_lspi(LSPIParameters *parameters) {
  exit_random_generator();
  features_exit();
  unload_feature_policy(&parameters->feature_policy);
}

/**
 * Executes the algorithm.
 */
static void lspi(LSPIParameters *parameters) {

  Sample *samples;
  gsl_vector *weights, *next_weights;
  int i, n;
  Game *game;
  double mean_score;
/*  double epsilon = 0.001;  */
  GamesStatistics *games_statistics;

  /* initialize the statistics */
  games_statistics = games_statistics_new(parameters->statistics_file_name, NB_GAMES_TO_EVALUATE, NULL);

  /* load or generate samples */
  if (parameters->nb_samples != 0) {
    printf("Generating samples... ");
    fflush(stdout);
    samples = generate_samples(parameters);
    printf("Done.\n");
    save_samples(parameters, samples);

    /* debug 
    tmp_samples = load_samples(parameters);
     now tmp_samples and samples should be identical 

    for (i = 0; i < parameters->nb_samples; i++) {
      if (
	  tmp_samples[i].action.orientation == samples[i].action.orientation && 
	  tmp_samples[i].action.column == samples[i].action.column && 
	  tmp_samples[i].next_piece_index == samples[i].next_piece_index &&
	  tmp_samples[i].features->data[0] == samples[i].features->data[0] &&
	  tmp_samples[i].features->data[1] == samples[i].features->data[1] &&
	  tmp_samples[i].features->data[2] == samples[i].features->data[2] &&
	  tmp_samples[i].features->data[3] == samples[i].features->data[3] &&
	  tmp_samples[i].features->data[4] == samples[i].features->data[4] &&
	  tmp_samples[i].features->data[5] == samples[i].features->data[5] &&
	  tmp_samples[i].features->data[6] == samples[i].features->data[6] &&
	  tmp_samples[i].features->data[7] == samples[i].features->data[7] && 
	  tmp_samples[i].features->data[8] == samples[i].features->data[8] &&
	  tmp_samples[i].features->data[9] == samples[i].features->data[9] &&
	  tmp_samples[i].state->board->rows[0] == samples[i].state->board->rows[0] &&
	  tmp_samples[i].state->board->rows[1] == samples[i].state->board->rows[1] &&
	  tmp_samples[i].state->board->rows[2] == samples[i].state->board->rows[2] &&
	  tmp_samples[i].state->board->rows[3] == samples[i].state->board->rows[3] &&
	  tmp_samples[i].state->board->rows[4] == samples[i].state->board->rows[4] &&
	  tmp_samples[i].state->board->rows[5] == samples[i].state->board->rows[5] &&
	  tmp_samples[i].state->board->rows[6] == samples[i].state->board->rows[6] &&
	  tmp_samples[i].state->board->rows[7] == samples[i].state->board->rows[7] &&
	  tmp_samples[i].state->board->rows[8] == samples[i].state->board->rows[8] &&
	  tmp_samples[i].state->board->rows[9] == samples[i].state->board->rows[9] &&
	  tmp_samples[i].state->board->rows[10] == samples[i].state->board->rows[10] &&
	  tmp_samples[i].state->board->rows[11] == samples[i].state->board->rows[11] &&
	  tmp_samples[i].state->board->rows[12] == samples[i].state->board->rows[12] &&
	  tmp_samples[i].state->board->rows[13] == samples[i].state->board->rows[13] &&
	  tmp_samples[i].state->board->rows[14] == samples[i].state->board->rows[14] &&
	  tmp_samples[i].state->board->rows[15] == samples[i].state->board->rows[15] &&
	  tmp_samples[i].state->board->rows[16] == samples[i].state->board->rows[16] &&
	  tmp_samples[i].state->board->rows[17] == samples[i].state->board->rows[17] &&
	  tmp_samples[i].state->board->rows[18] == samples[i].state->board->rows[18] &&
	  tmp_samples[i].state->board->rows[19] == samples[i].state->board->rows[19] 
	  
	  ) {
      }
      else {
        printf("sample %d differs!\n", i);

	printf("original: \n");
	printf("action = (%d,%d) = (%d,%d)\n", samples[i].action.orientation, samples[i].action.column,
	    samples[i].state->next_action.orientation, samples[i].state->next_action.column);
	game_print(stdout, samples[i].state);
	game_print_features(samples[i].state, &parameters->feature_policy);
	
	printf("\n\nreloaded:\n");
	printf("action = (%d,%d) = (%d,%d)\n", samples[i].action.orientation, samples[i].action.column,
	    tmp_samples[i].state->next_action.orientation, tmp_samples[i].state->next_action.column);
	game_print(stdout, tmp_samples[i].state);
	game_print_features(tmp_samples[i].state, &parameters->feature_policy);
      }
    }
     end debug */

  }
  else {
    printf("Loading samples... ");
    fflush(stdout);
    samples = load_samples(parameters);
    printf("Done.\n");
  }

  /* perform the algorithm */
  weights = gsl_vector_calloc(parameters->feature_policy.nb_features);
  next_weights = gsl_vector_calloc(parameters->feature_policy.nb_features);
  feature_policy_get_weights(&parameters->feature_policy, next_weights);

  n = 0;
  game = new_game_from_parameters(&parameters->common_parameters);
  do {
    gsl_vector_memcpy(weights, next_weights);
    lstdq(parameters, samples, weights, next_weights);
    printf("Distance: %f\n", get_distance(next_weights, weights));


    mean_score = feature_policy_play_games(&parameters->feature_policy, NB_GAMES_TO_EVALUATE,
					    game, games_statistics, 1);
 
    printf("\nMean score: %f\n\n", mean_score);
    

    n++;
  }
/*  while (get_distance(next_weights, weights) > epsilon); TODO stop after N iterations */
/*  while (n < 50 && get_distance(next_weights, weights) > epsilon); */
  while (n < 300); /* TODO: parameter for this + parameter to fix the policy */

  free_game(game);

  /* play some games with the learned policy */
  feature_policy_set_weights(&parameters->feature_policy, next_weights);
  save_feature_policy(parameters->final_feature_file_name, &parameters->feature_policy);
  /*
  game = new_standard_game();
  mean_score = feature_policy_play_games(&parameters->feature_policy, 100, game, NULL, 1);
  free_game(game);
  printf("\nMean score: %f\n", mean_score);
*/

  /* free the memory */
  for (i = 0; i < parameters->nb_samples; i++) {
    free_game(samples[i].state);
    gsl_vector_free(samples[i].features);
  }
  FREE(samples);
  gsl_vector_free(weights);
  gsl_vector_free(next_weights);
}

/**
 * Performs the LSTD-Q algorithm.
 * @param parameters the parameters of LSPI
 * @param samples the samples to use to evaluate the policy
 * @param weights some feature weights defining a policy to evaluate
 * @param next_weights pointer to a vector where the action-state value function weights will be stored
 */
static void lstdq(LSPIParameters *parameters, Sample *samples, const gsl_vector *weights, gsl_vector *next_weights) {

  gsl_matrix *a;
  gsl_vector *b;
  gsl_matrix *v;
  gsl_vector *s, *unused;
  int i;
  int nb_features;
  int status;
  int reward;

  Game *game;

  FeaturePolicy *feature_policy = &parameters->feature_policy;
  nb_features = parameters->feature_policy.nb_features;
  a = gsl_matrix_calloc(nb_features, nb_features);
  b = gsl_vector_calloc(nb_features);

  /* initialize A 
  gsl_matrix_set_identity(a);
  gsl_matrix_scale(a, 0.001);
  */

  feature_policy_set_weights(feature_policy, weights);
  printf("Starting LSTDQ with weights: ");
  print_features(feature_policy);

  game = new_standard_game();
  for (i = 0; i < parameters->nb_samples; i++) {
   
    /*
    printf("\n\ntaking sample %d, will do action (%d,%d) on this board: \n", i, samples[i].action.orientation, samples[i].action.column);

    if (i <= 5) {
      game_print_features(game, &parameters->feature_policy);
      game_print(stdout, game);
    }
    */

    game_copy(samples[i].state, game);
    game_drop_piece(game, &samples[i].action, 1, NULL);
    reward = parameters->common_parameters.reward_description.reward_function(game);

    /* bug !!!
    if (game->game_over) {
      continue;
    }
    */ 

    if (parameters->method != 1) {
      /* fixed point */
      update_matrices_fp(parameters, &samples[i], game, weights, a, b);
    }
    else {
      /* Bellman residual */
      update_matrices_br(parameters, &samples[i], game, weights, a, b);
    }
    /*   game_set_current_piece_index(game, samples[i].next_piece_index); */


    /*
    if (i == 9) {
      printf("A: \n");
      gsl_matrix_fprintf(stdout, a, "%f");
      printf("b: \n");
      gsl_vector_fprintf(stdout, b, "%f");
    }
    */
  }

  /* solve the Aw = b system */
  v = gsl_matrix_alloc(nb_features, nb_features);
  s = gsl_vector_alloc(nb_features);
  unused = gsl_vector_alloc(nb_features);
  status = gsl_linalg_SV_decomp(a, v, s, unused);
  gsl_vector_set_zero(next_weights);
  status = gsl_linalg_SV_solve(a, v, s, b, next_weights);

  gsl_matrix_free(a);
  gsl_vector_free(b);

  printf("LSTDQ finished. ");
  feature_policy_set_weights(feature_policy, next_weights);
  print_features(feature_policy);

  free_game(game);
}

/**
 * For a sample given, updates A and b with respect to the fixed point method.
 * @param parameters the parameters of LSPI
 * @param sample the sample (s,a) to take into account 
 * @param game the new game state after taking action a in state s
 * @param weights the old weights (used when lambda < 1)
 * @param a the A matrix to update
 * @param b the b vector to update
 */
static void update_matrices_fp(LSPIParameters *parameters, const Sample *sample, Game *game,
    const gsl_vector *weights, gsl_matrix *a, gsl_vector *b) {

  Action action;
  int j;  
  gsl_vector *tmp, *next_phis[7], *next_phi;
  gsl_matrix *left, *right;
  double sum;
  FeaturePolicy *feature_policy = &parameters->feature_policy;
  int nb_features = feature_policy->nb_features;
  const double gamma = parameters->gamma;
  const double lambda = parameters->lambda;
  int reward;

  tmp = gsl_vector_alloc(nb_features);
  next_phi = gsl_vector_alloc(nb_features);
  left = gsl_matrix_alloc(nb_features, nb_features);
  right = gsl_matrix_alloc(nb_features, nb_features);

  /* update A */
  gsl_vector_memcpy(tmp, sample->features);
  for (j = 0; j < 7; j++) {

    next_phis[j] = gsl_vector_alloc(nb_features);

    if (game->game_over) {
      gsl_vector_set_zero(next_phis[j]);
    }
    else {
      game_set_current_piece_index(game, j);

      /* make the move indicated by the policy to evaluate */
      features_get_best_action(game, feature_policy, &action);

      /* debug
	 if (j == 0 && i < 10) {
	 printf("sample %d: best action = %d,%d, evaluation = %f\n  ", i, action.column, action.orientation, evaluate_features(game, feature_policy));
	 game->next_action = action;
	 game_print_features(game, feature_policy);
	 }
	 */

      game->next_action = action;
      feature_policy_get_values(feature_policy, next_phis[j], game);
    }

    gsl_vector_memcpy(next_phi, next_phis[j]);
    gsl_vector_scale(next_phi, lambda * gamma / 7.0);

    /* debug
       if (j == 0 && i < 10) {
       gsl_vector_fprintf(stdout, next_phi, "%f");
       }
       */

    gsl_vector_sub(tmp, next_phi);

    /* debug
       if (j == 0 && i < 10) {
       printf("tmp: \n");
       gsl_vector_fprintf(stdout, tmp, "%f");
       }
       */
  }

  for (j = 0; j < nb_features; j++) {
    gsl_matrix_set_col(left, j, sample->features);
    gsl_matrix_set_row(right, j, tmp);
  }
  gsl_matrix_mul_elements(left, right);
  gsl_matrix_add(a, left);

  /* update b */
  reward = parameters->common_parameters.reward_description.reward_function(game);
  gsl_vector_memcpy(tmp, sample->features);

  sum = 0.0;
  for (j = 0; j < 7; j++) {
    sum += reward + (1 - lambda) * gamma * dot_product(next_phis[j], weights);
  }

  gsl_vector_scale(tmp, sum / 7.0);
  gsl_vector_add(b, tmp);

  for (j = 0; j < 7; j++) {
    gsl_vector_free(next_phis[j]);
  }
  gsl_vector_free(tmp);
  gsl_vector_free(next_phi);
  gsl_matrix_free(left);
  gsl_matrix_free(right);
}

/**
 * For a sample given, updates A and b with respect to the Bellman residual minimization method.
 * @param parameters the parameters of LSPI
 * @param sample the sample (s,a) to take into account 
 * @param game the new game state after taking action a in state s
 * @param weights the old weights (used when lambda < 1)
 * @param a the A matrix to update
 * @param b the b vector to update
 */
static void update_matrices_br(LSPIParameters *parameters, const Sample *sample, Game *game,
    const gsl_vector *weights, gsl_matrix *a, gsl_vector *b) {

  FeaturePolicy *feature_policy = &parameters->feature_policy;
  int nb_features = feature_policy->nb_features;
  const double gamma = parameters->gamma;
  const double lambda = parameters->lambda;
  Action action;
  int reward;

  int j;
  double sum;
  gsl_vector *tmp, *next_phi, *next_phis[7];
  gsl_matrix *left, *right;

  tmp = gsl_vector_alloc(nb_features);
  next_phi = gsl_vector_alloc(nb_features);
  left = gsl_matrix_alloc(nb_features, nb_features);
  right = gsl_matrix_alloc(nb_features, nb_features);

  /* update A */
  gsl_vector_memcpy(tmp, sample->features);
  for (j = 0; j < 7; j++) {

    next_phis[j] = gsl_vector_alloc(nb_features);
    if (game->game_over) {
      gsl_vector_set_zero(next_phis[j]);
    }
    else {
      game_set_current_piece_index(game, j);
      features_get_best_action(game, feature_policy, &action);
      game->next_action = action;
      feature_policy_get_values(feature_policy, next_phis[j], game);
    }

    gsl_vector_memcpy(next_phi, next_phis[j]);
    gsl_vector_scale(next_phi, lambda * gamma / 7.0);
    gsl_vector_sub(tmp, next_phi);
  }

  for (j = 0; j < nb_features; j++) {
    gsl_matrix_set_col(left, j, tmp);
    gsl_matrix_set_row(right, j, tmp);
  }
  gsl_matrix_mul_elements(left, right);
  gsl_matrix_add(a, left);

  /* update b */
  reward = parameters->common_parameters.reward_description.reward_function(game);
 
  sum = 0.0;
  for (j = 0; j < 7; j++) {
    sum += reward + (1 - lambda) * gamma * dot_product(next_phis[j], weights);
  }

  gsl_vector_scale(tmp, sum / 7.0);
  gsl_vector_add(b, tmp);

  for (j = 0; j < 7; j++) {
    gsl_vector_free(next_phis[j]);
  }
  gsl_vector_free(tmp);
  gsl_vector_free(next_phi);
  gsl_matrix_free(left);
  gsl_matrix_free(right);
}

/**
 * Generates some samples by playing a game.
 *
 */
static Sample * generate_samples(LSPIParameters *parameters) {

  Sample *samples;
  Game *game;
  Action action;
  int i;
  int nb_features;
  double *values;

  MALLOCN(samples, Sample, parameters->nb_samples);
  game = new_game_from_parameters(&parameters->common_parameters);

  /* draw nb_samples samples with the samples feature policy */
  for (i = 0; i < parameters->nb_samples; i++) {

    samples[i].state = new_game_copy(game);
    features_get_best_action(game, &parameters->samples_policy, &action); /* use the (s,a) feature functions to choose the move */
    samples[i].state->next_action = action;
    samples[i].action = action;
    samples[i].next_piece_index = game->current_piece_index;

    /* debug 
    if (i == parameters->nb_samples - 1) {
      printf("Sample %d action = (%d,%d), initial state:\n", i, action.orientation, action.column);
      game->next_action = action;
      game_print(stdout, game);
      game_print_features(game, &parameters->samples_policy);
    }
     */

    /* play the best move found (<=> go to the next state) */
    game_drop_piece(game, &action, 0, &parameters->samples_policy);

    /* debug 
    if (i == parameters->nb_samples - 1) {
      printf("Sample %d next state:\n", i);
      game_print(stdout, game);
      getchar();
    }
     */


    if (game->game_over) {
      game_reset(game);
      printf("game over\n");
    }
  }

  /* initialize the feature system again with the features we will use now */
  features_exit();
  load_feature_policy(parameters->initial_feature_file_name, &parameters->feature_policy);
  features_initialize(&parameters->feature_policy);

  nb_features = parameters->feature_policy.nb_features;

  /* compute the feature values of each sample */
  for (i = 0; i < parameters->nb_samples; i++) {

    game_copy(samples[i].state, game);
    samples[i].features = gsl_vector_alloc(nb_features);
    values = get_feature_values(game, &parameters->feature_policy);
    MEMCPY(samples[i].features->data, values, double, nb_features);
    FREE(values); /* TODO avoid this */

    game_drop_piece(game, &game->next_action, 0, NULL);

    /* store the reward (remark: the reward is known after the move) */
    /*    samples[i].reward = parameters->common_parameters.reward_description.reward_function(game);
remark: the reward obtained can be different from the reward function used to choose the move! (this is useful when the initial policy is based on a different criterion) */

    /*
       printf("reward: %d\n", game->last_move_info.removed_lines);
       */

    if (game->game_over) {
      game_reset(game);
    }
    game_set_current_piece_index(game, samples[i].next_piece_index);
  }

  /*  free_game(game); */
  return samples;
}

/**
 * Loads the samples from a file.
 *
 */
static Sample * load_samples(LSPIParameters *parameters) {

  /* load the samples */
  int i;
  FILE *f = open_data_file(parameters->samples_file_name, "rb");
  Sample *samples;
  Game *game;
  int nb_features;
  double *values;

  int width;
  int height;
  int allow_lines_after_overflow;
  char *piece_file_name = NULL;
  int n;
  int piece_index;

  /* load the game configuration */
  FREAD(&width, sizeof(int), 1, f);
  FREAD(&height, sizeof(int), 1, f);
  FREAD(&allow_lines_after_overflow, sizeof(int), 1, f);
  FREAD(&n, sizeof(int), 1, f);
  MALLOCN(piece_file_name, char, n);
  FREAD(piece_file_name, sizeof(char), n, f);

  game = new_game(0, width, height, allow_lines_after_overflow, piece_file_name, NULL);

  /* load the samples */
  FREAD(&parameters->nb_samples, sizeof(int), 1, f);
  MALLOCN(samples, Sample, parameters->nb_samples);
  for (i = 0; i < parameters->nb_samples; i++) {
    /* state, action, next piece index */
    game = new_game_copy(game);
    FREAD(&game->score, sizeof(int), 1, f);
    FREAD(&piece_index, sizeof(int), 1, f);
    game_set_current_piece_index(game, piece_index);
    FREAD(&game->board->wall_height, sizeof(int), 1, f);
    FREAD(game->board->rows, sizeof(uint16_t), height, f);
    FREAD(&samples[i].action, sizeof(Action), 1, f);
    game->next_action = samples[i].action;
    FREAD(&samples[i].next_piece_index, sizeof(int), 1, f);
    samples[i].state = game;

    /*
    if (i <= 5) {
      printf("Loading sample %d: piece = %d, action = (%d,%d), next piece = %d\n", i, game->current_piece_index,
	samples[i].action.orientation, samples[i].action.column, samples[i].next_piece_index);
      game_print(stdout, game);
    }
    */
    
  }

  /*
  printf("loaded the %d samples\n", parameters->nb_samples);
*/

  fclose(f);

  load_feature_policy(parameters->initial_feature_file_name, &parameters->feature_policy);
  features_initialize(&parameters->feature_policy);
  nb_features = parameters->feature_policy.nb_features;

  /* compute the feature values of each sample */
  for (i = 0; i < parameters->nb_samples; i++) {

    samples[i].features = gsl_vector_alloc(nb_features);
    values = get_feature_values(samples[i].state, &parameters->feature_policy);
    MEMCPY(samples[i].features->data, values, double, nb_features);
    FREE(values); /* TODO avoid this */

    /*
    if (i <= 5) {
      printf("  computed features for sample %d\n", i);
      game_print_features(samples[i].state, &parameters->feature_policy);
    }
    */
  }

  return samples;
}


/**
 * Saves the samples into a file.
 */
static void save_samples(LSPIParameters *parameters, Sample *samples) {

  int i;
  FILE *f = open_data_file(parameters->samples_file_name, "wb");
  Sample *sample;

  Game *game = samples[0].state;
  int width = game->board->width;
  int height = game->board->height;
  int allow_lines_after_overflow = game->board->allow_lines_after_overflow;
  char *piece_file_name = parameters->common_parameters.piece_file_name;
  int n = strlen(piece_file_name) + 1;

  /* save the game configuration */
  FWRITE(&width, sizeof(int), 1, f);
  FWRITE(&height, sizeof(int), 1, f);
  FWRITE(&allow_lines_after_overflow, sizeof(int), 1, f);
  FWRITE(&n, sizeof(int), 1, f);
  FWRITE(piece_file_name, sizeof(char), n, f);

  /* save the samples */
  FWRITE(&parameters->nb_samples, sizeof(int), 1, f);
  for (i = 0; i < parameters->nb_samples; i++) {
    sample = &samples[i];
    game = sample->state;
    /* state, action, next piece index */
    FWRITE(&game->score, sizeof(int), 1, f);
    FWRITE(&game->current_piece_index, sizeof(int), 1, f);
    FWRITE(&game->board->wall_height, sizeof(int), 1, f);
    FWRITE(game->board->rows, sizeof(uint16_t), height, f);
    FWRITE(&sample->action, sizeof(Action), 1, f);
    FWRITE(&sample->next_piece_index, sizeof(int), 1, f);

    /*
    if (i <= 5) {
      printf("writing sample %d: piece = %d, action = (%d,%d), next piece = %d\n", i, game->current_piece_index,
	sample->action.orientation, sample->action.column, sample->next_piece_index);
      game_print_features(game, &parameters->feature_policy);
      game_print(stdout, game);
    }
    */
  }

  fclose(f);
}

/**
 * Gets the feature values of a state-action pair into a gsl vector.
 * @param feature_policy the feature weights to use
 * @param values_vector the vector to fill
 * @param game a state-action pair
 */
static void feature_policy_get_values(FeaturePolicy *feature_policy, gsl_vector *values_vector, Game *game) {

  double *values = get_feature_values(game, feature_policy); /* TODO avoid allocating memory */
  int i;
  for (i = 0; i < feature_policy->nb_features; i++) {
    gsl_vector_set(values_vector, i, values[i]);
  }
  FREE(values);
}

/**
 * Sets the weights of a feature policy from a gsl vector.
 *
 */
static void feature_policy_set_weights(FeaturePolicy *feature_policy, const gsl_vector *weights) {

  int i;
  for (i = 0; i < feature_policy->nb_features; i++) {
    feature_policy->features[i].weight = gsl_vector_get(weights, i);
  }
}

/**
 * Gets the weights of a feature policy into a gsl vector.
 *
 */
static void feature_policy_get_weights(const FeaturePolicy *feature_policy, gsl_vector *weights) {

  int i;
  for (i = 0; i < feature_policy->nb_features; i++) {
    gsl_vector_set(weights, i, feature_policy->features[i].weight);
  }
}


/**
 * Returns the maximum-norm distance between two vectors.
 */
static double get_distance(const gsl_vector *a, const gsl_vector *b) {

  double max = 0;
  double diff;
  int i;

  for (i = 0; i < a->size; i++) {

    diff = fabs(gsl_vector_get(a, i) - gsl_vector_get(b, i));
    if (diff > max) {
      max = diff;
    }
  }

  return max;
}

/**
 * Returns the dot product between two vectors.
 */
static double dot_product(const gsl_vector *a, const gsl_vector *b) {

  int i;
  double result = 0.0;

  for (i = 0; i < a->size; i++) {
    result += gsl_vector_get(a, i) * gsl_vector_get(b, i);
  }

  return result;
}

/**
 * Main function.
 * Usage: ./lspi [parameters]
 * 
 */
int main(int argc, char **argv) {

  LSPIParameters parameters;

  if (argc == 1) {
    ask_parameters(&parameters);
  }
  else {
    parse_parameters(argc - 1, argv + 1, &parameters);
  }

  initialize_lspi(&parameters);
  lspi(&parameters);
  exit_lspi(&parameters);

  return 0;
}

