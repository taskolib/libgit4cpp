# Test sources
test_src = files(
    'test_Error.cc',
    'test_main.cc',
    'test_Remote.cc',
    'test_Repository.cc',
)

# The tests are executed in the build dir to avoid pollution of the git repository with
# the test executable's output files
test('all',
    executable(meson.project_name() + '_test',
        test_src,
        dependencies : [
            dependency('catch2', version : '>=3.4.1',
                fallback : [ 'catch2', 'catch2_dep' ]),
            libgit4cpp_dep,
        ],
    ),
    workdir : meson.current_build_dir(),
    timeout : 10,
)

######
# Test if all headers are self-contained (Core Guidelines SF.11)
# This is automated over all public_headers

message('Generating self-containment tests')
all_header_checks = [ ]
loop_number = 0
foreach header_to_check : public_headers
    loop_number += 1
    if header_to_check.endswith('.h')
        header_name = header_to_check.split('.h')[0]

        header_conf = configuration_data()
        header_conf.set('HEADER_TO_INCLUDE', meson.project_name() + '/' + header_to_check)
        header_conf.set('DUMMY_FUNCTION_NAME', 'a@0@'.format(loop_number))

        header_check = configure_file(
            input : 'test_self_containment.cc.in',
            output : 'incl_' + header_name.split('/')[-1] + '.cc',
            configuration : header_conf,
        )
        all_header_checks += [ header_check ]
    endif
endforeach

static_library(
    '_ignore_me', all_header_checks,
    dependencies : libgit4cpp_dep,
    install : false,
)
