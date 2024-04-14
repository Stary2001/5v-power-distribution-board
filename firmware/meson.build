project('power-board', ['c', 'cpp'], default_options: ['optimization=s', 'build.optimization=s'])

subdir('linker-script-as-property')

sam_project = subproject('libsam', default_options: ['samd_variant=samd21a', 'samd_device=SAMD21G17A'])
sam_library = sam_project.get_variable('sam')

# link_whole is important so newlib stubs are pulled from the library
main_exe = executable('firmware.elf',
	'main.cpp',
	'mongoose/mongoose.c',

	c_args: sam_project.get_variable('samd_defines'),
	cpp_args: sam_project.get_variable('samd_defines') + ['-fno-use-cxa-atexit'],
	include_directories: [ sam_project.get_variable('our_headers'), sam_project.get_variable('cmsis_headers'), sam_project.get_variable('samd_headers'), 'mongoose' ],
	link_whole: sam_library,
	link_args: [
		linker_script_flags
	],
	link_depends: linker_script_deps
)
