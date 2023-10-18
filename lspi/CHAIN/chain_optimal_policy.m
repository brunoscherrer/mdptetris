function policy = chain_optimal_policy(discount)
  
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% policy = chain_optimal_policy(discount)
%
% Creates and initializes an optimal policy for the chain problem.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  
  
  policy.explore = 0.0;
  
  policy.discount = discount;
  policy.lambda = 1.0;

  policy.actions = 2;
  
  policy.basis = @chain_basis_exact;
  
  k = feval(policy.basis); % calling the basis function without paramater returns the number of basis
  
  policy.weights = [ones(k/4,1) ; zeros(k/2,1) ; ones(k/4,1)]; % this is the optimal policy for the chain problem

