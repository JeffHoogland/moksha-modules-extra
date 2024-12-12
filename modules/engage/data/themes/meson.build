cmd = [ edje_cc,
        '-id', join_paths(meson.project_source_root(), 'data/themes'),
        '@INPUT@', '@OUTPUT@']
custom_target('engage.edj',
              input      : 'engage.edc',
              output     : 'engage.edj',
              command    : cmd,
              install_dir: mod_install_dir,
              install    : true
             )
