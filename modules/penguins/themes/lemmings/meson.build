### lemmings theme ###
cmd = [ edje_cc,
        '-id', join_paths(meson.project_source_root(), 'themes/lemmings'), 
        '@INPUT@', '@OUTPUT@']

custom_target('lemmings.edj',
              input      : 'lemmings.edc',
              output     : 'lemmings.edj',
              command    : cmd,
              install_dir: join_paths(mod_install_dir, 'themes'),
              install    : true
             )
