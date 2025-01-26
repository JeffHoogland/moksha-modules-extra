#### Project ####
project('calendar', 'c',
        version: '0.0.1',
        meson_version: '>= 0.58.0')

add_project_arguments('-D__UNUSED__=__attribute__((unused))', language: 'c')

gettext_domain = 'e-module-' + meson.project_name()

#### Enlightenemnt ####
dep_e = dependency('enlightenment')
e_modules_dir = dep_e.get_variable('modules')
e_release = dep_e.get_variable('release')
e_module_arch = dep_e.get_variable('module_arch')

#### Install dirs ####
mod_install_dir = join_paths(e_modules_dir, meson.project_name())
lib_install_dir = join_paths(mod_install_dir, e_module_arch)

#### config.h ####
config_h = configuration_data()
config_h.set_quoted('PACKAGE', meson.project_name())
config_h.set_quoted('LOCALEDIR', join_paths(get_option('prefix'), get_option('localedir')))
config_h.set_quoted('LOCALEDOMAIN', gettext_domain)
config_h.set('__UNUSED__', '__attribute__((unused))')
config_dir = include_directories('.')

#### i18n ####
dep_intl = []
if get_option('nls')
   config_h.set('HAVE_GETTEXT', '1')
   cc = meson.get_compiler('c')
   dep_intl = cc.find_library('intl', required : false)
   subdir('po')
endif

#### module.desktop ####
i18n.merge_file(
  input: 'module.desktop.in',
  output: 'module.desktop',
  type: 'desktop',
  po_dir: 'po',
  install: true,
  install_dir: mod_install_dir
)

#### Edje cc ####
edje_cc = find_program('edje_cc')

#### module sources ####
subdir('src')

#### write config file ####
configure_file(output: 'config.h', configuration: config_h)

cmd = [ edje_cc,
        '-id', join_paths(meson.project_source_root(), 'images'),
        '@INPUT@', '@OUTPUT@']
custom_target('e-module-calendar.edj',
              input      : 'e-module-calendar.edc',
              output     : 'e-module-calendar.edj',
              command    : cmd,
              install_dir: mod_install_dir,
              install    : true
             )
custom_target('calendar.edj',
              input      : 'calendar.edc',
              output     : 'calendar.edj',
              command    : cmd,
              install_dir: mod_install_dir,
              install    : true
             )

#### Report ####
if meson.version().version_compare('>=0.53')
   summary({'install': lib_install_dir,
           }, section: 'Directories')
endif
