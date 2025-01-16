### default theme ###
cmd = [ edje_cc,
        '-id', join_paths(meson.project_source_root(), 'themes/default'), 
        '@INPUT@', '@OUTPUT@']

custom_target('default.edj',
              input      : 'default.edc',
              output     : 'default.edj',
              command    : cmd,
              install_dir: join_paths(mod_install_dir, 'themes'),
              install    : true
             )
