##### edje_cc binary compiler tool
edje_cc = get_option('edje-cc')
if edje_cc == ''
edje_cc = find_program('edje_cc')
endif

cmd = [ edje_cc,
        '-id', join_paths(meson.project_source_root(), 'data/theme/images'),
        '@INPUT@', '@OUTPUT@']
mod_edc = join_paths(meson.project_source_root(), 'data/theme/module.edc')

custom_target('e-module-news.edj',
              input      : mod_edc,
              output     : 'e-module-news.edj',
              command    : cmd,
              install_dir: mod_install_dir,
              install    : true
             )