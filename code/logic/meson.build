dir = include_directories('.')

subdir('fossil')

fossil_network_lib = library(
    'fossil_network',
    files(
        'socket.c'
    ),
    install: true,
    dependencies: platform_deps,
    include_directories: dir
)

fossil_network_dep = declare_dependency(
    link_with: [fossil_network_lib],
    include_directories: dir
)

meson.override_dependency('fossil-network', fossil_network_dep)