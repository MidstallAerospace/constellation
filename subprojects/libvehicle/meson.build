project('constellation-vehicle', 'c', license: 'GPL-3.0-only', version: '0.1.0-prealpha')

libnet = subproject('libnet')
libnet_dep = libnet.get_variable('libnet_dep')

libcommon = subproject('libcommon')
libcommon_dep = libcommon.get_variable('libcommon_dep')

thread_dep = dependency('threads')
libmath = meson.get_compiler('c').find_library('m')

subdir('include')

libvehicle = both_libraries('constellation-vehicle', ['src/net.c', 'src/program.c', 'src/vehicle.c'],
	dependencies: [libnet_dep, libcommon_dep, thread_dep, libmath],
	include_directories: ['include'],
	install: true)

libvehicle_dep = declare_dependency(link_with: [libvehicle],
	dependencies: [libnet_dep, libcommon_dep, thread_dep, libmath],
	include_directories: ['include'])
