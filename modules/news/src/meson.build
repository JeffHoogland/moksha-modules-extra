module = shared_module('module',
   'e_mod_main.c',                      'news_feed.c',
   'news_config.c',                     'news_item.c',
   'news_config_dialog.c',              'news_menu.c',
   'news_config_dialog_category.c',     'news_parse.c',
   'news_config_dialog_feed.c',         'news_popup.c',
   'news_config_dialog_feeds.c',        'news_theme.c',
   'news_config_dialog_item.c',         'news_utils.c',
   'news_config_dialog_item_content.c', 'news_viewer.c',
   'news_config_dialog_langs.c',


   dependencies : [dep_e, dep_intl],
   install_dir: lib_install_dir,
   include_directories: config_dir,
   install: true,
   name_prefix: '',
   link_args: '-Wl,--unresolved-symbols=ignore-in-object-files'
)
