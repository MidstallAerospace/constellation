project('constellation', 'c', license: 'GPL-3.0-only', version: '0.1.0-prealpha')

libnet = subproject('libnet')
libnet_dep = libnet.get_variable('cnet_dep')

libvehicle = subproject('libvehicle')
libvehicle_dep = libvehicle.get_variable('libvehicle_dep')

if get_option('build_sim_ksp')
	subproject('sim-ksp', default_options: ['ksp_data_path=' + get_option('ksp_data_path')])
endif
