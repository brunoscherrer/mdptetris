function [v, q1, q2, q] = chainsolve(policy)
  
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
% [v, q1, q2, q] = chainsolve(policy)
% v: V(s)
% q1: Q(s, 'left')
% q2: Q(s, 'right')
% q: max(q1, q2)
%
% Evaluates the input policy by solving the model of the chain
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  
  
  [s, r, e, PR, R] = chain_simulator; % PR is the whole transition matrix of the MDP

  dim = length(R);
  
  PA{1} = squeeze( PR(:,1,:) ); % transition probabilities of action 'left'
  PA{2} = squeeze( PR(:,2,:) ); % transition probabilities of action 'right'
  
  P = zeros(dim, dim);
  for i=1:dim
    a = policy_function(policy, i);
    P(i,:) = PA{a}(i,:); % P: transition probabilities of the policy
  end
  
  v = (eye(dim) - policy.discount * P) \ R;
  
  q1 = R + policy.discount * PA{1} * v;
  q2 = R + policy.discount * PA{2} * v;
  q = max(q1,q2);
  
  return
  
  
