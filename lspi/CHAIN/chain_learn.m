function [final_policy, all_policies, samples, deltas_vk, deltas_vpi] = chain_learn(maxiterations, ...
						  epsilon, samples, ...
						  maxepisodes, ...
						  maxsteps, discount, lambda, ...
						  basis, algorithm, policy)
  
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% Copyright 2000-2002 
%
% Michail G. Lagoudakis (mgl@cs.duke.edu)
% Ronald Parr (parr@cs.duke.edu)
%
% Department of Computer Science
% Box 90129
% Duke University, NC 27708
% 
%
% [final_policy, all_policies, samples] = chain_learn(maxiterations, ...
%						  epsilon, samples, ...
%						  maxepisodes, ...
%						  maxsteps, discount, ...
%						  basis, algorithm, policy)
%
% Runs LSPI for the chain domain
%
% Input:
%
% maxiterations - An integer indicating the maximum number of
%                 LSPI iterations (default = 8)
% 
% epsilon - A small real number used as the termination
%           criterion. LSPI converges if the distance between
%           weights of consequtive iterations is less than
%           epsilon (default = 10^-5)
%
% samples - The sample set. This should be an array where each
%            entry samples(i) has the following form: 
%
%            samples(i).state     : Arbitrary description of state
%            samples(i).action    : An integer in [1,|A|]
%            samples(i).reward    : A real value
%            samples(i).nextstate : Arbitrary description
%            samples(i).absorb    : Absorbing nextstate? (0 or 1)
%
%            (default = [] - empty)
%
% maxepisodes - An integer indicating the maximum number of
%               episodes from which (additional) samples will be
%               collected.
%               (default = 10, if samples is empty, 0 otherwise)
% 
% maxsteps - An integer indicating the maximum number of steps of each
%            episode (an episode may finish earlier if an absorbing
%            state is encountered). 
%            (default = 50)
%
% discount - A real number in (0,1] to be used as the discount factor
%            (default = 0.9)
%
% basis - The function that computes the basis for a given pair
%         (state, action) given as a function handle
%         (e.g. @chain_phi) or as a string (e.g. 'chain_phi')
%         (default = 'chain_basis_pol')
% 
% algorithm - This is a number that indicates which evaluation
%             algorithm should be used (see the paper):
%
%             1-lsq       : The regular LSQ (incremental)
%             2-lsqfast   : A fast version of LSQ (uses more space)
%             3-lsqbe     : LSQ with Bellman error minimization 
%             4-lsqbefast : A fast version of LSQBE (more space)
%
%             LSQ is the evaluation algorithm for regular
%             LSPI. Use lsqfast in general, unless you have
%             really big sample sets. LSQBE is provided for
%             comparison purposes and completeness.
%             (default = 2)
%
% policy - (optional argument) A policy to be used for collecting the
%          (additional) samples and as the initial policy for LSPI. It
%          should be given as a struct with the following fields (at
%          least):
%
%          explore  : Exploration rate (real number)
%          discount : Discount factor (real number)
%          actions  : Total numbers of actions, |A|
%          basis    : The function handle for the basis
%                     associated with this policy
%          weights  : A column array of weights 
%                     (one for each basis function)
%
%          If a policy is not provided, samples will be collected by a
%          purely random policy initialized with "explore"=1.0,
%          "discount" and "basis" some dummy values, and "actions" and
%          "weights" as suggested by the chain domain (in the
%          chain_initialize_policy function. Notice that the
%          "basis" used by this policy can be different from the
%          "basis" above that is used for the LSPI iteration. 
%
%
% Output:
%
% final_policy - The learned policy (same struct as above)
% 
% all_policies - A cell array of size (iterations+1) containing
%                all the intermediate policies at each LSPI
%                iteration, including the initial policy. 
%
% samples     - The set of all samples used for this run. Each entry
%               samples(i) has the following form:
%
%               samples(i).state     : Arbitrary description of state
%               samples(i).action    : An integer in [1,|A|]
%               samples(i).reward    : A real value
%               samples(i).nextstate : Arbitrary description
%               samples(i).absorb    : Absorbing nextstate? (0 or 1)
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  
  clear functions
  
  %%% Print some info
  disp('*************************************************');
  disp('LSPI : Least-Squares Policy Iteration');
  disp('-------------------------------------------------');
  
  domain = 'chain';
  disp(['Domain : ' domain]);
  
  
  if nargin < 1
    maxiterations = 8;
  end
  disp(['Max LSPI iterations : ' num2str(maxiterations)]);
  
  
  if nargin < 2
    epsilon = 10^(-5);
  end
  disp(['Epsilon : ' num2str(epsilon)]);
  
  
  if nargin < 3
    samples = [];
  end
  disp(['Samples in the initial set : ' num2str(length(samples))]);
  
  
  if nargin < 4
    if isempty(samples)
      maxepisodes = 10;
    else
      maxepisodes = 0;
    end
  end
  disp(['Episodes for sample collection : ' num2str(maxepisodes)]);
  
  
  if nargin < 5
    maxsteps = 500;
  end
  disp(['Max steps in each episode : ' num2str(maxsteps)]);
  
  
  if nargin < 6
    discount = 0.9;
  end
  disp(['Discount factor : ' num2str(discount)]);
   
  if nargin < 7
    lambda = 1.0;
  end
  disp(['Lambda: ' num2str(lambda)]);
  
  if nargin < 8
    basis = 'chain_basis_pol';
  end
  disp('Basis : ');
  disp(basis);
  
  
  %%% Set the evaluation algorithm
  if nargin < 9
    algorithm = 2;
  end
  algorithms = ['lsq      '; 'lsqfast  '; 'lsqbe    '; 'lsqbefast'; 'llsq     '; 'llsqfast '; 'llsqbe   '; 'llsqbefast'];
  disp(['Selected evaluation algorithm : ' algorithms(algorithm,: )]);
  
  
  if nargin < 10
    disp('No initial policy provided');
    policy = chain_initialize_policy(0.0, discount, lambda, basis);
  else
    disp(['Initial policy exploration rate : ' num2str(policy.explore)]);
    disp('Initial policy basis : ');
    disp(policy.basis);
    disp(['Initial policy number of weights : ', ...
	  num2str(length(policy.weights))]);
  end
  
  
  %%% Collect (additional) samples if requested
  if maxepisodes>0
    disp('-------------------------------------------------');
    disp('Collecting samples ...');
    if nargin < 10
      disp('... using a purely random policy');
      new_samples = collect_samples(domain, maxepisodes, maxsteps);
    else
      disp('... using the policy provided');
      new_samples = collect_samples(domain, maxepisodes, maxsteps, policy);
    end
    samples = [samples new_samples];
    clear new_samples;
  end
  
  
  %%% Ready to go - Display the total number of samples
  disp('-------------------------------------------------');
  disp(['Total number of samples : ' num2str(length(samples))]);
    
  %%% Compute the optimal value to compare the values computed by the algorithm to it
  optimal_policy = chain_optimal_policy(discount);
  optimal_value = chainsolve(optimal_policy);
  phi = chainphi(basis);
  
  %%% Run LSPI
  disp('*************************************************');
  disp('Starting LSPI ...');
  [final_policy, all_policies] = lspi(domain, algorithm, maxiterations, ...
				      epsilon, samples, basis, ...
				      discount, lambda, policy);
  
  %%% Compare the values of each policy with the optimal value
  deltas_vk = zeros(length(all_policies), 1);
  deltas_vpi = zeros(length(all_policies), 1);
  estimated_value = zeros(chainstates, 1);
  for i = 1:length(all_policies)
    policy = all_policies{i};
    estimated_qvalue = phi * policy.weights;
    for j = 1:chainstates
      estimated_value(j,1) = estimated_qvalue(1 + (j-1) * chainactions + (policy_function(policy, j) - 1));
    end
    real_value = chainsolve(policy);

    difference_vk = estimated_value - optimal_value;
    deltas_vk(i,1) = norm(difference_vk, inf);
    difference_vpi = real_value - optimal_value;
    deltas_vpi(i,1) = norm(difference_vpi, inf);

  end

