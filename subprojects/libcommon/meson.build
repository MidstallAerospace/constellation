project('constellation-commons', 'c', license: 'GPL-3.0-only', version: '0.1.0-prealpha')

subdir('include')

libcommon = both_libraries('constellation-common', ['src/event.c', 'src/list.c'],
	include_directories: ['include'],
	install: true)
libcommon_dep = declare_dependency(link_with: [libcommon],
	include_directories: ['include'])
