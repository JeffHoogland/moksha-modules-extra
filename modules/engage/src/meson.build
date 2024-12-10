module = shared_module('module',
   'e_mod_main.c',
    'ng_config.c',
	   'ng_item.c',
	   'ng_launcher.c',
	   'ng_taskbar.c',
	   'ng_config_instances.c',
	   'ng_box.c',
	   'ng_border_menu.c', 
	   'ng_gadcon.c',

   dependencies : [dep_e, dep_intl],
   install_dir: lib_install_dir,
   include_directories: config_dir,
   install: true,
   name_prefix: '',
   link_args: '-Wl,--unresolved-symbols=ignore-in-object-files'
)
