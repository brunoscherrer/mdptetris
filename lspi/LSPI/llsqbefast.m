function [w, A, b] = llsqbefast(samples, policy, new_policy, firsttime)
  
  
  persistent Phihat;
  persistent Rhat;
  
  
  %%% Initialize variables
  howmany = length(samples);
  k = feval(new_policy.basis);
  A = zeros(k, k);
  b = zeros(k, 1);
  PiPhihat = zeros(howmany,k);
  mytime = cputime;
  
  
  %%% Precompute Phihat and Rhat for all subsequent iterations
  if firsttime == 1
    
    Phihat = zeros(howmany,k);
    Rhat = zeros(howmany,1);
    
    for i=1:howmany
      phi = feval(new_policy.basis, samples(i).state, samples(i).action);
      Phihat(i,:) = phi';
      Rhat(i) = samples(i).reward;
    end
    
  end
  
  
  %%% Loop through the samples 
  for i=1:howmany
    
    %%% Make sure the nextstate is not an absorbing state
    if ~samples(i).absorb
      
      %%% Compute the policy and the corresponding basis at the next state 
      nextaction = policy_function(policy, samples(i).nextstate);
      nextphi = feval(new_policy.basis, samples(i).nextstate, nextaction);
      PiPhihat(i,:) = nextphi';
      
    end
     
    %%% Make sure the nextstate 2 is not an absorbing state
    if ~samples(i).absorb2
      
      %%% Compute the policy and the corresponding basis at the next state 
      nextaction2 = policy_function(policy, samples(i).nextstate2);
      nextphi2 = feval(new_policy.basis, samples(i).nextstate2, nextaction2);
      PiPhihat2(i,:) = nextphi2';
      
    end

  end
  
  
  %%% Compute the matrices A and b
  A = (Phihat - policy.lambda * new_policy.discount * PiPhihat2)' * ...
      (Phihat - policy.lambda * new_policy.discount * PiPhihat);
  b = (Phihat - policy.lambda * new_policy.discount * PiPhihat2)' * ...
      (Rhat + (1 - policy.lambda) * new_policy.discount * PiPhihat * policy.weights);
  
  phi_time = cputime - mytime;
  disp(['CPU time to form A and b : ' num2str(phi_time)]);
  mytime = cputime;
  
  
  %%% Solve the system to find w
  rankA = rank(A);
  
  rank_time = cputime - mytime;
  disp(['CPU time to find the rank of A : ' num2str(phi_time)]);
  mytime = cputime;
  
  disp(['Rank of matrix A : ' num2str(rankA)]);
  if rankA==k
    disp('A is a full rank matrix!!!');
    w = A\b;
  else
    disp(['WARNING: A is lower rank!!! Should be ' num2str(k)]);
    w = pinv(A)*b;
  end
  
  solve_time = cputime - mytime;
  disp(['CPU time to solve Aw=b : ' num2str(solve_time)]);
  
  return
  
