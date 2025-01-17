module = shared_module('module',
   'e_mod_main.c',
   'e_mod_keybindings.c',
   'ng_config.c',
   'ng_box.c',
   'ng_item.c',
   'ng_winlist.c',
   

   dependencies : [dep_e, dep_intl],
   install_dir: lib_install_dir,
   include_directories: config_dir,
   install: true,
   name_prefix: '',
   link_args: '-Wl,--unresolved-symbols=ignore-in-object-files'
)
