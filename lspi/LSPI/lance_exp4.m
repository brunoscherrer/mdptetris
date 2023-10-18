addpath ../CHAIN


printf ("Chargement échantillons");
load('exp/samples_array_100.mat', 'samples_array_100'); 

printf ("*** RBF-FP ***");
[rbf_100ech_fp_vk, rbf_100ech_fp_vpi] = exp_lambda(samples_array_100, 6, @chain_basis_rbf, 150,0.95);
save('exp/rbf/100ech/fp_vk_g0.95.mat', 'rbf_100ech_fp_vk'); 
save('exp/rbf/100ech/fp_vpi_g0.95.mat.mat', 'rbf_100ech_fp_vpi'); 

printf ("*** RBF-BR ***");
[rbf_100ech_br_vk, rbf_100ech_br_vpi] = exp_lambda(samples_array_100, 8, @chain_basis_rbf, 150,0.95);
save('exp/rbf/100ech/br_vk_g0.95.mat.mat', 'rbf_100ech_br_vk');
save('exp/rbf/100ech/br_vpi_g0.95.mat.mat', 'rbf_100ech_br_vpi');


printf ("*** POL-FP ***");
[pol_100ech_fp_vk, pol_100ech_fp_vpi] = exp_lambda(samples_array_100, 6, @chain_basis_pol, 150,0.95);
save('exp/pol/100ech/fp_vk_g0.95.mat.mat', 'pol_100ech_fp_vk'); 
save('exp/pol/100ech/fp_vpi_g0.95.mat.mat', 'pol_100ech_fp_vpi'); 

printf ("*** POL-BR ***");
[pol_100ech_br_vk, pol_100ech_br_vpi] = exp_lambda(samples_array_100, 8, @chain_basis_pol, 150,0.95);
save('exp/pol/100ech/br_vk_g0.95.mat.mat', 'pol_100ech_br_vk');
save('exp/pol/100ech/br_vpi_g0.95.mat.mat', 'pol_100ech_br_vpi');



printf ("Chargement échantillons");
load('exp/samples_array_50.mat', 'samples_array_50'); 

printf ("*** RBF-FP ***");
[rbf_50ech_fp_vk, rbf_50ech_fp_vpi] = exp_lambda(samples_array_50, 6, @chain_basis_rbf, 150,0.95);
save('exp/rbf/50ech/fp_vk_g0.95.mat', 'rbf_50ech_fp_vk'); 
save('exp/rbf/50ech/fp_vpi_g0.95.mat.mat', 'rbf_50ech_fp_vpi'); 

printf ("*** RBF-BR ***");
[rbf_50ech_br_vk, rbf_50ech_br_vpi] = exp_lambda(samples_array_50, 8, @chain_basis_rbf, 150,0.95);
save('exp/rbf/50ech/br_vk_g0.95.mat.mat', 'rbf_50ech_br_vk');
save('exp/rbf/50ech/br_vpi_g0.95.mat.mat', 'rbf_50ech_br_vpi');


printf ("*** POL-FP ***");
[pol_50ech_fp_vk, pol_50ech_fp_vpi] = exp_lambda(samples_array_50, 6, @chain_basis_pol, 150,0.95);
save('exp/pol/50ech/fp_vk_g0.95.mat.mat', 'pol_50ech_fp_vk'); 
save('exp/pol/50ech/fp_vpi_g0.95.mat.mat', 'pol_50ech_fp_vpi'); 

printf ("*** POL-BR ***");
[pol_50ech_br_vk, pol_50ech_br_vpi] = exp_lambda(samples_array_50, 8, @chain_basis_pol, 150,0.95);
save('exp/pol/50ech/br_vk_g0.95.mat.mat', 'pol_50ech_br_vk');
save('exp/pol/50ech/br_vpi_g0.95.mat.mat', 'pol_50ech_br_vpi');


