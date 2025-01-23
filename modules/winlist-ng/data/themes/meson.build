### default theme ###
cmd = [ edje_cc,
        '-id', join_paths(meson.project_source_root(), 'data/themes/'), 
        '@INPUT@', '@OUTPUT@']

custom_target('ng.edj',
              input      : 'ng.edc',
              output     : 'ng.edj',
              command    : cmd,
              install_dir: mod_install_dir,
              install    : true
             )
