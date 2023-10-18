#include <stdio.h>
#include <math.h>
#include <stdlib.h>
/* #include <values.h> */
#include <sys/time.h>
#include <time.h>
#include <unistd.h>


#include "random.h"
#include "feature_functions.h"
#include "feature_policy.h"
#include "game.h"
#include "cmaes_interface.h"
#include "common_parameters.h"

#define _FREE(x) if (x!=NULL) { free(x); x=NULL;} \
else {fprintf(stderr, "Problem freeing memmory\n"); exit(1);}
#define _ALLOC(p,nb,typ) if ((p=(typ*)calloc(nb,sizeof(typ)))==NULL) \
{fprintf(stderr, "Memory full\n"); exit(1);}

#define __TIMEIT

#ifdef __TIMEIT
#define _ST_CHRONO_S(T) gettimeofday(&(T), NULL);
#define _SP_CHRONO_S(T0, T1, DT)\
gettimeofday(&(T1), NULL);\
DT=((T1).tv_sec - (T0).tv_sec) + ((double)((T1).tv_usec - (T0).tv_usec)) * 1.0e-6;
#else
#define _ST_CHRONO_S(T)
#define _SP_CHRONO_S(T0, T1, DT)
#endif

static int num_evals =0;


/* Evaluation function */
double eval_fixed_seed(double *x, int n, int nb_games, FeaturePolicy *fp, 
		       Game *game, long *seeds){
    int i;
    double score;

    /* copy the vector in a feature struct */
    for(i=0; i <n; i++){
        fp->features[i].weight = x[i];
    }
    
    score = 0.0;
    for(i=0; i <nb_games; i++){
	initialize_random_generator(seeds[i]);
	feature_policy_play_game(fp, game);
	score += game->score;
	num_evals++;
    }
    score /= (double) nb_games;

    return score;
}





double eval(double *x, int n, int nb_games, FeaturePolicy *fp, Game *game){
    int i;
    double score;

    /* copy the vector in a feature struct */
    for(i=0; i <n; i++){
        fp->features[i].weight = x[i];
    }
    
    score = 0.0;
    for(i=0; i <nb_games; i++){
	feature_policy_play_game(fp, game);
	score += game->score;
    }
    score /= (double) nb_games;
    return score;
}






int main(int argc, char *argv[]){
    int            i=0,j=0,k=0;
   
    /* CMA-ES Variables */
    char          *cma_parameter_file="incmaes.par"; /* cma parameter files */
    cmaes_t        evo;                     /* CMA struct contains everything*/
    double        *rgFunVal = NULL;         /* pointer to population fitness */
    double *const *pop;                     /* pointer to sample population */
    double        *xinit=NULL;              /* initial feature vector */
    double        *xbest=NULL;              /* Best feature vector so far */
    double         fbest=0;                 /* Fitness of the best vector */
    double        *mean=NULL;               /* Mean of all feature vectors */
    double         fmean=0;                 /* fitness of the mean vector */
    
    /* MDP Tetris Variables */
    int              board_width;
    int              board_height;
    char            *feature_file_name;    /* Initial feature file name */
    FeaturePolicy    feature_policy;
    Game            *game;

    int   nb_games=10;      /* Number of games to evaluate each vector */
    int   nb_games_mean=10; /* Number of games to evaluate the mean vector */
    int   gen;              /* Maximum number of generations for CMA ES */

    /* stats */
    int   print = 1; /* print stats or not, argument 7 in command line*/
    char *outf;      /* the file name to print to if "print" is true */
    FILE *fp=NULL;   
  
    /* timing vars */
    struct timeval    t0, tc;
    double            dtc =0;
    double            elapsed_time=0.0;
    double            time_sample=0.0, time_eval_pop=0.0;
    double            time_adapt=0.0, time_eval_mean=0.0;

    /* random game seed */
    long  *seeds=NULL;






    /* parse the command line */
    if (argc != 9 ) {
	fprintf(stderr, "Usage: PROG feature_file nb_games board_width board_height gens print nb_games_mean outfile\n");
	exit(1);
    }
    feature_file_name    = argv[1];
    nb_games             = atoi(argv[2]);
    board_width          = atoi(argv[3]);
    board_height         = atoi(argv[4]);
    gen                  = atoi(argv[5]);
    print                = atoi(argv[6]);
    nb_games_mean        = atoi(argv[7]);
    outf                 = argv[8];

    /* open outfile */
    if(print){
	fp = fopen(outf, "w");
	if(!fp){
	    fprintf(stderr, "could not create out file %s\n", outf);
	    exit(1);
	}
    }
		
    /* initialize mdptetris */
    initialize_random_generator(time(NULL)); 
    load_feature_policy(feature_file_name, &feature_policy);
    features_initialize(&feature_policy);
    game = new_game(0, board_width, board_height, 1, "pieces4-hard.dat", NULL);
    
    
    /* copy  initial vector */
    _ALLOC(xinit, feature_policy.nb_features, double);
    for(i=0; i <feature_policy.nb_features; i++){
	xinit[i] = feature_policy.features[i].weight;
    }
    
    /* holds nb_games random seeds */
    _ALLOC(seeds, nb_games, long);
   
    /* init CMA */
    rgFunVal = cmaes_init(&evo, 
			  feature_policy.nb_features, 
			  xinit , 
			  NULL, 0, 0, cma_parameter_file);

    /* print column titles of statistics */
    if(print){
	fprintf(fp,"#%s\n", cmaes_SayHello(&evo));
	fprintf(fp,"# (1)evals (2)best(nb_games) (3)best(nb_games_mean) (4)mean(nb_games_mean) (5)eigenRatio (6)stddevRatio (7)sigma (8)MeanEvalTime [(9-%d)axisLength] [(%d-%d) stdDevAxis] [mean] [best] time_sample time_eval_pop time_adapt time_eval_mean\n", 
		feature_policy.nb_features+8, feature_policy.nb_features+10, feature_policy.nb_features+10+feature_policy.nb_features);
    }
    
    /* gen loop */
    for(i=1; i<=gen; i++) {
	
	/* sample lambda vectors according to the current distribution */  
	time_sample=0.0;
	_ST_CHRONO_S(t0);
	pop = cmaes_SamplePopulation(&evo);
	/* ------- normalize vectors */
	for (j=0; j < cmaes_Get(&evo, "lambda"); ++j){
	    double sum=0.0;
	    for(k=0; k <(int) cmaes_Get(&evo, "dim"); k++)
		sum += pop[j][k] * pop[j][k];
	    sum =sqrt(sum);
	    for(k=0; k <(int) cmaes_Get(&evo, "dim"); k++)
		pop[j][k] /= sum;
	}
	/* ------- initialize nb_games random seeds */
	for(k=0; k < nb_games ; k++){
	    seeds[k] =  random_uniform(1, time(NULL));
	}
	_SP_CHRONO_S(t0, tc, dtc);
	time_sample = dtc;


	/* evaluate the sample */
	elapsed_time = 0.0;
	time_eval_pop = 0.0;
	_ST_CHRONO_S(t0);
	for (j=0; j < cmaes_Get(&evo, "lambda"); ++j)
	    rgFunVal[j] = -eval_fixed_seed(pop[j], 
					   (int) cmaes_Get(&evo,"dim"), 
					   nb_games, 
					   &feature_policy, 
					   game, 
					   seeds);
	_SP_CHRONO_S(t0, tc, dtc);
	elapsed_time = dtc / ((double) cmaes_Get(&evo, "lambda")) ;
	time_eval_pop = dtc;

	

	/* update the distribution */
	time_adapt=0.0;
	_ST_CHRONO_S(t0);
	cmaes_UpdateDistribution(&evo, rgFunVal);  
	_SP_CHRONO_S(t0, tc, dtc);
	time_adapt = dtc;



	/* eval and copy mean of the distribution and the best */
	time_eval_mean = 0.0;
	 _ST_CHRONO_S(t0);
	mean  = cmaes_GetInto(&evo, "xmean", mean);
	fmean = eval(mean, (int) cmaes_Get(&evo, "dim"), nb_games_mean,
		     &feature_policy, game);
	xbest = cmaes_GetInto(&evo, "xbest", xbest);
	/* fbest = eval(xbest, (int) cmaes_Get(&evo, "dim"), nb_games_mean, */
	/* &feature_policy, game);*/
	_SP_CHRONO_S(t0, tc, dtc);
	time_eval_mean = dtc;


	/* print stats */
	if(print){
	    fprintf(fp, "%7d %.5e %.5e %.5e %.5e %.5e %.5e %.5e ", 
		    num_evals,                      /* 1 # of evaluations */ 
		    -cmaes_Get(&evo, "fctvalue"),   /* 2 best this gen after nb_games evaluations */
		    fbest,                          /* 3 best this gen after nb_games_mean evaluations */
		    fmean,                          /* 4 mean after nb_games_mean evaluations*/
		    cmaes_Get(&evo, "maxaxislen") / /* 5 ratio of eigen axis*/
		    cmaes_Get(&evo, "minaxislen"),
		    cmaes_Get(&evo, "maxstddev") /  /* 6 ratio stddev */ 
		    cmaes_Get(&evo, "minstddev"),
		    cmaes_Get(&evo, "sigma"),       /* 7 sigma */
		    elapsed_time);                  /* 8 mean eval time/vector */

	    /* principal axis lengths sqrt(eigenvalues) */
	    fprintf(fp,"[ ");
	    for(j=0; j <(int) cmaes_Get(&evo, "dim"); j++)
		fprintf(fp, "%.5e ", sqrt(evo.rgD[j]));
	    fprintf(fp,"] ");

	    /* standard dev of all axis sigma*sqrt(diag(C)) */
	    fprintf(fp,"[ ");
	    for(j=0; j <(int) cmaes_Get(&evo, "dim"); j++)
		fprintf(fp, "%.5e ", sqrt(evo.C[j][j])*cmaes_Get(&evo,"sigma"));
	    fprintf(fp,"] ");

	    /* mean */ 
	    fprintf(fp,"[(%f) ", fmean);
	    for (j=0; j < (int) cmaes_Get(&evo, "dim"); j++)
		fprintf(fp,"%.3e ", mean[j]);
	    fprintf(fp,"] ");
	    
	    /* xbest */
	    fprintf(fp,"[(%f) ", fbest);
	    for (j=0; j < (int) cmaes_Get(&evo, "dim"); j++)
		fprintf(fp,"%.3e ", xbest[j]);
	    fprintf(fp,"] ");
	    

	    /* print timings */
	    fprintf(fp, "%.5e %.5e %.5e %.5e",  
		    time_sample, 
		    time_eval_pop, 
		    time_adapt, 
		    time_eval_mean);
	    

	    /* end the line */
	    fprintf(fp,"\n"); 
	    fflush(fp);

	} /* print stats */
	
    } /* gens loop */



    /* clean up and exit */ 
    exit_random_generator();
    free_game(game);
    _FREE(xinit);
    _FREE(mean);
    _FREE(xbest);
    _FREE(seeds);
    cmaes_exit(&evo);
    if(print)
	fclose(fp);
    exit(0);
}

