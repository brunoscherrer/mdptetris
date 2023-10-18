function [phi] = chainphi(basis)
  
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% Builds the phi matrix.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  
  nb_states = chainstates;
  nb_actions = chainactions;
  nb_basis = feval(basis);

  phi = zeros(nb_states * nb_actions, nb_basis);
  k = 1;
  for i = 1:nb_states
    for j = 1:nb_actions
      phi(k,:) = feval(basis, i, j)';
      k = k + 1;
    end
  end
  
  return
  
  
