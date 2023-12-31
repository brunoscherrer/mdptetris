addpath ../CHAIN

printf ("Chargement échantillons");
load('exp/samples_array_200.mat', 'samples_array_200'); 

printf ("*** RBF-FP ***");
[rbf_200ech_fp_vk, rbf_200ech_fp_vpi] = exp_lambda(samples_array_200, 6, @chain_basis_rbf, 150,0.99);
save('exp/rbf/200ech/fp_vk.mat', 'rbf_200ech_fp_vk'); 
save('exp/rbf/200ech/fp_vpi.mat', 'rbf_200ech_fp_vpi'); 

printf ("*** RBF-BR ***");
[rbf_200ech_br_vk, rbf_200ech_br_vpi] = exp_lambda(samples_array_200, 8, @chain_basis_rbf, 150,0.99);
save('exp/rbf/200ech/br_vk.mat', 'rbf_200ech_br_vk');
save('exp/rbf/200ech/br_vpi.mat', 'rbf_200ech_br_vpi');


printf ("*** POL-FP ***");
[pol_200ech_fp_vk, pol_200ech_fp_vpi] = exp_lambda(samples_array_200, 6, @chain_basis_pol, 150,0.99);
save('exp/pol/200ech/fp_vk.mat', 'pol_200ech_fp_vk'); 
save('exp/pol/200ech/fp_vpi.mat', 'pol_200ech_fp_vpi'); 

printf ("*** POL-BR ***");
[pol_200ech_br_vk, pol_200ech_br_vpi] = exp_lambda(samples_array_200, 8, @chain_basis_pol, 150,0.99);
save('exp/pol/200ech/br_vk.mat', 'pol_200ech_br_vk');
save('exp/pol/200ech/br_vpi.mat', 'pol_200ech_br_vpi');
