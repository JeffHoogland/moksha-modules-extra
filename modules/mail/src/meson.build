includes = [
   'e_mod_main.c',
   'e_mod_config.c',
   'e_mod_config_box.c',
   'mdir.c', 
   'mbox.c', 
   'pop.c',
]

if imap2 == true
   includes += 'imap2.c'
else
   includes += 'imap.c'
endif

module = shared_module('module',
   includes,
   dependencies : [dep_e, dep_intl],
   install_dir: lib_install_dir,
   include_directories: config_dir,
   install: true,
   name_prefix: '',
   link_args: '-Wl,--unresolved-symbols=ignore-in-object-files'
)
