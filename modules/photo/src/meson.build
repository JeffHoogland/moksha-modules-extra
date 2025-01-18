module = shared_module('module',
   'e_mod_main.c',
   'photo_item.c',
   'photo_picture_net.c',
   'photo_config.c',
   'photo_menu.c',
   'photo_popup_info.c',
   'photo_config_dialog.c',
   'photo_picture.c',
   'photo_popup_warn.c',
   'photo_config_dialog_dir.c',
   'photo_picture_histo.c',
   'photo_utils.c',
   'photo_config_dialog_item.c',
   'photo_picture_local.c',


   dependencies : [dep_e, dep_intl],
   install_dir: lib_install_dir,
   include_directories: config_dir,
   install: true,
   name_prefix: '',
   link_args: '-Wl,--unresolved-symbols=ignore-in-object-files'
)
