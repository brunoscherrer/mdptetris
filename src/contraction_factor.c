#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "macros.h"

int main(int argc, char **argv) {
  FILE *delta_file;
  char *line;
  unsigned int n, read;
  int iteration;
  double delta, previous_delta, gamma, max_gamma;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s delta_file_name\n", argv[0]);
    exit(1);
  }
  
  delta_file = fopen(argv[1], "r");
  line = NULL;
  read = 0;
  max_gamma = 0;
  previous_delta = 0;
  read = getline(&line, &n, delta_file);
  while (read != -1) {
    if (line[0] != '#') {
      sscanf(line, "%d %lf", &iteration, &delta);
/*       printf("delta = %f\n", delta); */
      
      if (previous_delta != 0) {
	gamma = delta / previous_delta;
/* 	printf("gamma = %f\n", gamma); */
	if (gamma > max_gamma) {
	  printf("new max gamma = %f, delta = %f\n", gamma, delta);
	}
	max_gamma = MAX(max_gamma, gamma);
      }

      previous_delta = delta;
    }
    read = getline(&line, &n, delta_file);
  }
  fclose(delta_file);

  printf("max_gamma = %f\n", max_gamma);
  printf("1 - max_gamma = %f\n", 1 - max_gamma);
  
  return 0;
}
