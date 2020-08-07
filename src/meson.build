subdir('shader')
subdir('reshade')

vkBasalt_src = [
    'basalt.cpp',
    'basalt_device.cpp',
    'basalt_instance.cpp',
    'logger.cpp',
]

x11_dep = dependency('x11')

shared_library(meson.project_name().to_lower(), 
    vkBasalt_src, shader_include,
    include_directories : vkBasalt_include_path,
    dependencies : [x11_dep, reshade_dep],
    install : lib_dir)
