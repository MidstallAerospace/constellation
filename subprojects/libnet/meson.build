project('constellation-net', 'c', license: 'GPL-3.0-only', version: '0.1.0-prealpha')

subdir('include')

libnet = both_libraries('constellation-net', ['src/packet.c'],
	include_directories: ['include'],
	install: true)
libnet_dep = declare_dependency(link_with: [libnet],
	include_directories: ['include'])
