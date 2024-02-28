project('power-board', ['cpp'], default_options: ['optimization=s', 'build.optimization=s'])

subdir('linker-script-as-property')

sam_project = subproject('libsam', default_options: ['samd_variant=samd21a', 'samd_device=SAMD21G17A'])
sam_library = sam_project.get_variable('sam')

# link_whole is important so newlib stubs are pulled from the library
main_exe = executable('firmware.elf',
	'main.cpp',
'w5500/src/Utility/CRC16.cpp',
'w5500/src/Protocols/DNS.cpp',
'w5500/src/Protocols/NTP.cpp',
'w5500/src/Protocols/DHCP.cpp',
'w5500/src/UdpSocket.cpp',
'w5500/src/Socket.cpp',
'w5500/src/TcpSocket.cpp',
'w5500/src/W5500.cpp',

	c_args: sam_project.get_variable('samd_defines'),
	cpp_args: sam_project.get_variable('samd_defines'),
	include_directories: [ sam_project.get_variable('our_headers'), sam_project.get_variable('cmsis_headers'), sam_project.get_variable('samd_headers'), 'w5500/include' ],
	link_whole: sam_library,
	link_args: [
		linker_script_flags
	],
	link_depends: linker_script_deps
)
