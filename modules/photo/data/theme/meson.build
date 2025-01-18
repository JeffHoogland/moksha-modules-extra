### photo theme ###
cmd = [ edje_cc,
        '-id', join_paths(meson.project_source_root(), 'data/theme/images'), 
        '@INPUT@', '@OUTPUT@']

custom_target('photo.edj',
              input      : 'photo.edc',
              output     : 'photo.edj',
              command    : cmd,
              install_dir: mod_install_dir,
              install    : true
             )
