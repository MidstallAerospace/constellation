project('constellation-net', 'c', license: 'GPL-3.0-only', version: '0.1.0-prealpha')

libcommon = subproject('libcommon')
libcommon_dep = libcommon.get_variable('libcommon_dep')

subdir('include')

libnet = both_libraries('constellation-net', ['src/packet.c'],
	dependencies: [libcommon_dep],
	include_directories: ['include'],
	install: true)
libnet_dep = declare_dependency(link_with: [libnet],
	dependencies: [libcommon_dep],
	include_directories: ['include'])
