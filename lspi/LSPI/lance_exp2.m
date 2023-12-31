addpath ../CHAIN

printf ("Chargement échantillons");
load('exp/samples_array_500.mat', 'samples_array_500'); 

printf ("*** RBF-FP ***");
[rbf_500ech_fp_vk, rbf_500ech_fp_vpi] = exp_lambda(samples_array_500, 6, @chain_basis_rbf, 150,0.99);
save('exp/rbf/500ech/fp_vk_g0.99.mat', 'rbf_500ech_fp_vk'); 
save('exp/rbf/500ech/fp_vpi_g0.99.mat', 'rbf_500ech_fp_vpi'); 

printf ("*** RBF-BR ***");
[rbf_500ech_br_vk, rbf_500ech_br_vpi] = exp_lambda(samples_array_500, 8, @chain_basis_rbf, 150,0.99);
save('exp/rbf/500ech/br_vk_g0.99.mat', 'rbf_500ech_br_vk');
save('exp/rbf/500ech/br_vpi_g0.99.mat', 'rbf_500ech_br_vpi');


printf ("*** POL-FP ***");
[pol_500ech_fp_vk, pol_500ech_fp_vpi] = exp_lambda(samples_array_500, 6, @chain_basis_pol, 150,0.99);
save('exp/pol/500ech/fp_vk_g0.99.mat', 'pol_500ech_fp_vk'); 
save('exp/pol/500ech/fp_vpi_g0.99.mat', 'pol_500ech_fp_vpi'); 

printf ("*** POL-BR ***");
[pol_500ech_br_vk, pol_500ech_br_vpi] = exp_lambda(samples_array_500, 8, @chain_basis_pol, 150,0.99);
save('exp/pol/500ech/br_vk_g0.99.mat', 'pol_500ech_br_vk');
save('exp/pol/500ech/br_vpi_g0.99.mat', 'pol_500ech_br_vpi');
