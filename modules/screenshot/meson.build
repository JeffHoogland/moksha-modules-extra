#### Project ####
project('screenshot', 'c',
        version: '0.1.1',
        meson_version: '>= 0.58.0')

add_project_arguments('-D__UNUSED__=__attribute__((unused))', language: 'c')

gettext_domain = 'e-module-' + meson.project_name()

### Checks for library functions.
cc = meson.get_compiler('c')

if not cc.has_function('strdup')
  error('The required function strdup() is not available.')
endif

### Check for needed headers ###
have_sys_time_h = cc.has_header('sys/time.h')
have_time_h = cc.has_header('time.h')
have_locale_h = cc.has_header('locale.h')

if not have_time_h
  error('Missing required header: sys/time.h')
endif

#### Enlightenment ####
dep_e = dependency('enlightenment')
e_modules_dir = dep_e.get_variable('modules')
e_module_arch = dep_e.get_variable('module_arch')

#### Install dirs ####
mod_install_dir = join_paths(e_modules_dir, meson.project_name())
lib_install_dir = join_paths(mod_install_dir, e_module_arch)

### Runtime Dependency ###
emprint = find_program('emprint', required: true)

#### config.h ####
config_h = configuration_data()
config_h.set_quoted('PACKAGE', meson.project_name())
config_h.set_quoted('LOCALEDIR', join_paths(get_option('prefix'), get_option('localedir')))
config_h.set_quoted('LOCALEDOMAIN', gettext_domain)

if have_sys_time_h and have_time_h
  config_h.set('TIME_WITH_SYS_TIME', 1)
else
  config_h.set('TIME_WITH_SYS_TIME', 0)
endif

if have_sys_time_h
  config_h.set('HAVE_SYS_TIME_H', 1)
else
  config_h.set('HAVE_SYS_TIME_H', 0)
endif

if have_locale_h
  config_h.set('HAVE_LOCALE_H', 1)
else
  config_h.set('HAVE_LOCALE_H', 0)
endif

config_h.set('__UNUSED__', '__attribute__((unused))')
config_dir = include_directories('.')

#### i18n ####
dep_intl = []
if get_option('nls')
   config_h.set('HAVE_GETTEXT', '1')
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

##### edje_cc binary compiler tool
edje_cc = get_option('edje-cc')
if edje_cc == ''
edje_cc = find_program('edje_cc')
endif

#### module sources ####
subdir('src')

#### write config file ####
configure_file(output: 'config.h', configuration: config_h)

cmd = [ edje_cc,
        '-id', join_paths(meson.project_source_root(), 'images'),
        '@INPUT@', '@OUTPUT@']
custom_target('e-module-screenshot.edj',
              input      : 'e-module-screenshot.edc',
              output     : 'e-module-screenshot.edj',
              command    : cmd,
              install_dir: mod_install_dir,
              install    : true
             )

#### Report ####
if meson.version().version_compare('>=0.53')
   summary({'install': lib_install_dir,
           }, section: 'Directories')
endif
