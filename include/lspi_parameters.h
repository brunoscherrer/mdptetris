#ifndef LSPI_PARAMETERS_H
#define LSPI_PARAMETERS_H

#include <string.h>
#include "common_parameters.h"
#include "feature_policy.h"
#include "rewards.h"
#include "macros.h"

/**
 * Algorithm parameters.
 */
typedef struct LSPIParameters {
  CommonParameters common_parameters;            /* board dimensions, pieces, reward function */

  int nb_samples;                                /* number of samples to generate (0 means that the samples are read from a file) */
  FeaturePolicy samples_policy;                  /* the feature policy used to generate samples */

  char samples_file_name[MAX_FILE_NAME];         /* name of the file where the samples are read */
  char samples_policy_file_name[MAX_FILE_NAME];  /* name of the file where the feature policy used to generate samples is read */
  char initial_feature_file_name[MAX_FILE_NAME]; /* name of the file where the features to optimize are read
                                                    (the weights in the file are ignored) */
  char final_feature_file_name[MAX_FILE_NAME];   /* name of the file where the features and
					            their final weights are saved */
  char statistics_file_name[MAX_FILE_NAME];      /* name of the file where the statistics for
					            each game batch are saved */

  double gamma;                                  /* Discount factor */
  double lambda;                                 /* Lambda */
  int method;                                    /* 0: fixed point, 1: Bellman residual */

  FeaturePolicy feature_policy;                  /* the feature policy computed at each iteration */

} LSPIParameters;

/**
 * Asks all parameters to the user.
 */
void ask_parameters(LSPIParameters *parameters);

/**
 * Parses the parameters from the command line.
 */
void parse_parameters(int nb_args, char **args, LSPIParameters *parameters);

/**
 * Prints a help message explaining how to use the program and exits the program.
 */
void print_usage_and_quit(void);

#endif

