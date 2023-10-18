/**
 * This module contains functions to parse the parameters from the command line
 * or to ask them to the user.
 */

#include "config.h"
#include "lspi_parameters.h"
#include "file_tools.h"

/**
 * Prints a help message explaining how to use the program.
 */
static void print_usage(void);

/**
 * Asks all parameters to the user.
 */
void ask_parameters(LSPIParameters *parameters) {
  char *read;
  char line[MAX_FILE_NAME];

  /* first ask the common parameters */
  ask_common_parameters(&parameters->common_parameters);

  
  /* gamma */
  printf("Gamma discount factor (default 1.0): ");
  read = fgets(line, MAX_LENGTH, stdin);
  if (read == NULL || line[0] == '\n') {
    parameters->gamma = 1.0;
  }
  else {
    sscanf(line, "%lf", &parameters->gamma);
  }
  
  /* lambda */
  printf("Lambda (default 1.0): ");
  read = fgets(line, MAX_LENGTH, stdin);
  if (read == NULL || line[0] == '\n') {
    parameters->lambda = 1.0;
  }
  else {
    sscanf(line, "%lf", &parameters->lambda);
  }

  /* method */
  printf("Method (0: fixed point, 1: Bellman residual, default 0): ");
  read = fgets(line, MAX_LENGTH, stdin);
  if (read == NULL || line[0] == '\n') {
    parameters->method = 0;
  }
  else {
    sscanf(line, "%d", &parameters->method);
  }

  /* number of samples */
  printf("Number of samples to generate (default 10000, set to 0 to read them from a file): ");
  read = fgets(line, MAX_LENGTH, stdin);
  if (read == NULL || line[0] == '\n') {
    parameters->nb_samples = 10000;
  }
  else {
    sscanf(line, "%d", &parameters->nb_samples);
  }

  if (parameters->nb_samples == 0) {

    /* load existing samples from a binary file */
    printf("Binary file containing the samples to load (default samples.dat): ");
    read = fgets(line, MAX_LENGTH, stdin);
    if (read == NULL || line[0] == '\n') {
      strcpy(parameters->samples_file_name, "samples.dat");
    }
    else {
      line[strlen(line)-1] = '\0'; 
      strcpy(parameters->samples_file_name, line);
    }
  }
  else {

    /* generate the samples with a policy (a feature file) */
    printf("Feature file describing the policy to use to generate samples (default features/lagoudakis_initial.dat): ");
    read = fgets(line, MAX_LENGTH, stdin);
    if (read == NULL || line[0] == '\n') {
      strcpy(parameters->samples_policy_file_name, "features/lagoudakis_initial.dat");
    }
    else {
      line[strlen(line)-1] = '\0'; 
      strcpy(parameters->samples_policy_file_name, line);
    }
    strcpy(parameters->samples_file_name, "samples.dat");
  }

  /* list of features to optimize (a feature file, again) - the weights are ignored */
  printf("Feature file containing the list of features to optimize (default features/lagoudakis_initial.dat): ");
  read = fgets(line, MAX_LENGTH, stdin);
  if (read == NULL || line[0] == '\n') {
    strcpy(parameters->initial_feature_file_name, "features/lagoudakis_initial.dat");
  }
  else {
    line[strlen(line)-1] = '\0'; 
    strcpy(parameters->initial_feature_file_name, line);
  }

  /* final feature file */
  printf("File to save the features with the final weights (default lagoudakis.dat): ");
  read = fgets(line, MAX_LENGTH, stdin);
  if (read == NULL || line[0] == '\n') {
    strcpy(parameters->final_feature_file_name, "lagoudakis.dat");
  }
  else {
    line[strlen(line)-1] = '\0'; 
    strcpy(parameters->final_feature_file_name, line);
  }

  /* statistics file */
  printf("File to save the game statistics (default lspi_statistics.dat): ");
  read = fgets(line, MAX_LENGTH, stdin);
  if (read == NULL || line[0] == '\n') {
    strcpy(parameters->statistics_file_name, "lspi_statistics.dat");
  }
  else {
    line[strlen(line)-1] = '\0'; 
    strcpy(parameters->statistics_file_name, line);
  }
}

/**
 * Parses the parameters from the command line.
 */
void parse_parameters(int nb_args, char **args, LSPIParameters *parameters) {
  int i, nb_read, recognized;
  int samples_mode = -1; /* -1: not specified yet, 0: generate, 1: load */

  /* first, load the default parameters */
  load_default_parameters(&parameters->common_parameters);
  parameters->gamma = 1.0;
  parameters->lambda = 1.0;
  parameters->method = 0;
  parameters->nb_samples = 10000;
  strcpy(parameters->samples_file_name, "samples.dat");
  strcpy(parameters->samples_policy_file_name, "features/lagoudakis_initial.dat");
  strcpy(parameters->initial_feature_file_name, "features/lagoudakis_initial.dat");
  strcpy(parameters->final_feature_file_name, "lagoudakis.dat");
  strcpy(parameters->statistics_file_name, "lspi_statistics.dat");

  /* then parse the specified paramaters */
  i = 0;
  while (i < nb_args) {
    recognized = 0;

    /* gamma */
    if (!strcmp(args[i], "-gamma")) {
      i++;
      parameters_assert(i < nb_args, "Missing argument to parameter -gamma", print_usage);
      parameters_assert(sscanf(args[i], "%lf", &parameters->gamma) == 1,
			"Incorrect argument for parameter -gamma", print_usage);
      recognized = 1;
    }

    /* lambda */
    if (!strcmp(args[i], "-lambda")) {
      i++;
      parameters_assert(i < nb_args, "Missing argument to parameter -lambda", print_usage);
      parameters_assert(sscanf(args[i], "%lf", &parameters->lambda) == 1,
			"Incorrect argument for parameter -lambda", print_usage);
      recognized = 1;
    }

    /* method */
    if (!strcmp(args[i], "-method")) {
      i++;
      parameters_assert(i < nb_args, "Missing argument to parameter -method", print_usage);
      parameters_assert(sscanf(args[i], "%d", &parameters->method) == 1,
			"Incorrect argument for parameter -method", print_usage);
      recognized = 1;
    }

    /* load samples */
    if (!strcmp(args[i], "-load_samples")) {
      parameters_assert(samples_mode == -1, "Cannot load samples from a file since you chose to generate them", print_usage);
      i++;
      parameters_assert(i < nb_args, "Missing argument to parameter -load_samples", print_usage);
      parameters_assert(sscanf(args[i], "%s", parameters->samples_file_name) == 1,
			"Incorrect argument for parameter -load_samples", print_usage);
      samples_mode = 1;
      parameters->nb_samples = 0;
      recognized = 1;
    }

    /* generate samples */
    if (!strcmp(args[i], "-generate_samples")) {
      parameters_assert(samples_mode == -1, "Cannot generate samples since you chose to load them from a file", print_usage);
      i++;
      parameters_assert(i < nb_args, "Missing argument 1 to parameter -generate_samples", print_usage);
      parameters_assert(sscanf(args[i], "%d", &parameters->nb_samples) == 1,
			"Incorrect argument 1 for parameter -generate_samples", print_usage);
      i++;
      parameters_assert(i < nb_args, "Missing argument 2 to parameter -generate_samples", print_usage);
      parameters_assert(sscanf(args[i], "%s", parameters->samples_policy_file_name) == 1,
			"Incorrect argument 2 for parameter -generate_samples", print_usage);
      samples_mode = 0;
      recognized = 1;
    }

    /* initial feature file */
    if (!strcmp(args[i], "-initial_features")) {
      i++;
      parameters_assert(i < nb_args, "Missing argument to parameter -initial_features", print_usage);
      parameters_assert(sscanf(args[i], "%s", parameters->initial_feature_file_name) == 1,
			"Incorrect argument for parameter -initial_features", print_usage);
      recognized = 1;
    }

    /* final feature file */
    if (!strcmp(args[i], "-final_features")) {
      i++;
      parameters_assert(i < nb_args, "Missing argument to parameter -final_features", print_usage);
      parameters_assert(sscanf(args[i], "%s", parameters->final_feature_file_name) == 1,
			"Incorrect argument for parameter -final_features", print_usage);
      recognized = 1;
    }

    /* statistics file */
    if (!strcmp(args[i], "-statistics_file")) {
      i++;
      parameters_assert(i < nb_args, "Missing argument to parameter -statistics_file", print_usage);
      parameters_assert(sscanf(args[i], "%s", parameters->statistics_file_name) == 1,
			"Incorrect argument to parameter -statistics_file", print_usage);
      recognized = 1;
    }

    /* if the parameter is none of these ones, it should be a common parameter */
    if (!recognized) {
      nb_read = parse_common_parameter(&parameters->common_parameters, nb_args - i, &args[i], print_usage);
      recognized = (nb_read > 0);
      parameters_assert(recognized, "Unknown parameter", print_usage);
      
      i += nb_read;
    }

    else {
      i++;
    }
  }
}

/**
 * Prints a help message explaining how to use the program.
 */
static void print_usage(void) {

  /* the whole string is too long for ISO C89 compilers */

  fprintf(stderr,
	  "Usage: ./lspi [parameters]\n"
	  "\n"
	  "Without arguments, all parameters are asked to the user.\n"
	  "If there is at least one argument, the missing parameters take their default values.\n\n");

  common_parameters_print_usage();

  fprintf(stderr,
	  "-gamma x                       gamma discount factor (default 1.0)\n"
	  "-lambda x                      lambda (default 1.0)\n"
	  "-method m                      0: fixed point, 1: Bellman residual (default 0)\n"
	  "-generate_samples n file_name  generate n samples using the policy in the feature file file_name\n"
	  "                               (default 10000 from features/lagoudakis_initial.dat) \n");
  fprintf(stderr,
	  "-load_samples file_name        load the samples from the binary file file_name instead of generating them \n"
	  "-initial_features file_name    feature file describing the features to optimize with LSPI (the weights specified are ignored)\n"
	  "-final_features file_name      feature file where the final weights will be saved\n"
	  "-statistics_file file_name     file where the game statistics at each iteration will be saved\n\n");

}

