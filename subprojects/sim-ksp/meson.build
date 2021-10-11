project('constellation-sim-ksp', 'cs', license: 'GPL-3.0-only', version: '0.1.0-prealpha')

# Imports
fs = import('fs')

# Options
ksp_data_path = get_option('ksp_data_path')
if ksp_data_path == ''
	error('Invalid directory for KSP')
endif

# Variables
do_host = target_machine.cpu_family() == 'avr'

ksp_path = fs.parent(ksp_data_path)
ksp_gamedata_path = ksp_path / 'GameData'
ksp_asm_path = ksp_data_path / 'Managed'
ksp_moddir = ksp_gamedata_path / 'ConstellationSim'

# Dependencies
AssemblyCSharp_dep = declare_dependency(link_args: ['-reference:' + ksp_asm_path / 'Assembly-CSharp.dll'])
System_dep = declare_dependency(link_args: ['-reference:' + ksp_asm_path / 'System.dll'])
System_Core_dep = declare_dependency(link_args: ['-reference:' + ksp_asm_path / 'System.Core.dll'])
UnityEngine_dep = declare_dependency(link_args: ['-reference:' + ksp_asm_path / 'UnityEngine.dll'])
UnityEngine_CoreModule_dep = declare_dependency(link_args: ['-reference:' + ksp_asm_path / 'UnityEngine.CoreModule.dll'])
UnityEngine_IMGUIModule_dep = declare_dependency(link_args: ['-reference:' + ksp_asm_path / 'UnityEngine.IMGUIModule.dll'])
UnityEngine_UI_dep = declare_dependency(link_args: ['-reference:' + ksp_asm_path / 'UnityEngine.UI.dll'])
mscorlib_dep = declare_dependency(link_args: ['-reference:' + ksp_asm_path / 'mscorlib.dll'])

summary({
	'Kerbal Space Program': ksp_path,
	'Kerbal Space Program GameData': ksp_gamedata_path,
	'Kerbal Space Program Assemblies': ksp_asm_path,
	'Constellation Simulator Directory': ksp_moddir,
	'Compiling on host': do_host,
}, section: 'Configuration')

shared_library('ConstellationSimMod', ['src/mod.cs'],
	dependencies: [AssemblyCSharp_dep, mscorlib_dep, UnityEngine_dep, UnityEngine_CoreModule_dep,
		UnityEngine_UI_dep, UnityEngine_IMGUIModule_dep, System_Core_dep],
	cs_args: ['-nostdlib'],
	link_args: ['-nostdlib'],
	install: true,
	install_dir: ksp_moddir / 'Plugins',
	native: not do_host)

install_subdir('data', install_dir: ksp_moddir, strip_directory: true)
