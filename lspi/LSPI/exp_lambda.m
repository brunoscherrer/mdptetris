function [all_mean_deltas_vk, all_mean_deltas_vpi] = exp_lambda(samples, algorithm, basis, max_iterations, gamma, lambdas)

  if (nargin < 5)
    gamma = 0.9
  end

  if (nargin < 6)
    lambdas = [ 0.0 0.1 0.3 0.5 0.7 0.8 0.9 0.95 1.0 ];
  end

  it = 1;
  for lambda = lambdas
    disp(['lambda = ' num2str(lambda)]);

    mean_deltas_vk = zeros(max_iterations + 1, 1);
    mean_deltas_vpi = zeros(max_iterations + 1, 1);
    for i = [1:10]
      [pol, allpol, s, deltas_vk, deltas_vpi] = chain_learn(max_iterations, 10^-3, samples{i}, 0, 0, gamma, lambda, basis, algorithm);
      deltas_vk(length(deltas_vk) + 1 : max_iterations + 1) = deltas_vk(length(deltas_vk));
      deltas_vpi(length(deltas_vpi) + 1 : max_iterations + 1) = deltas_vpi(length(deltas_vpi));
      mean_deltas_vk = mean_deltas_vk + deltas_vk;
      mean_deltas_vpi = mean_deltas_vpi + deltas_vpi;
    end
    mean_deltas_vk = mean_deltas_vk / max_iterations;
    mean_deltas_vpi = mean_deltas_vpi / max_iterations;

    figure(4);
    plot(mean_deltas_vk);
    text(15, mean_deltas_vk(15), ['\leftarrow \lambda = ' num2str(lambda)]);
    hold on;

    figure(5);
    plot(mean_deltas_vpi);
    text(4, mean_deltas_vpi(4), ['\leftarrow \lambda = ' num2str(lambda)]);
    hold on;

    all_mean_deltas_vk{it} = mean_deltas_vk;
    all_mean_deltas_vpi{it} = mean_deltas_vpi;
    it = it + 1;
  end

  figure(4);
  hold off;
  figure(5);
  hold off;
