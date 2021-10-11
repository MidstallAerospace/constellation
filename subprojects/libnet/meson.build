project('constellation-net', 'c', license: 'GPL-3.0-only', version: '0.1.0-prealpha')

subdir('include')

cnet = both_libraries('constellation-net', ['src/packet.c'],
	include_directories: ['include'],
	install: true)
cnet_dep = declare_dependency(link_with: [cnet],
	include_directories: ['include'])
