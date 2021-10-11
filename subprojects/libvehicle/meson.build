project('constellation-vehicle', 'c', license: 'GPL-3.0-only', version: '0.1.0-prealpha')

libnet = subproject('libnet')
libnet_dep = libnet.get_variable('cnet_dep')

thread_dep = dependency('threads')

subdir('include')

libvehicle = both_libraries('constellation-vehicle', ['src/net.c', 'src/vehicle.c'],
	dependencies: [libnet_dep, thread_dep],
	include_directories: ['include'],
	install: true)

libvehicle_dep = declare_dependency(link_with: [libvehicle],
	dependencies: [libnet_dep, thread_dep],
	include_directories: ['include'])
