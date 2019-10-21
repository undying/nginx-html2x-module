
flags = [
    '-Wall',
    '-Wextra',
    '-Werror',
    '-pedantic',
    '-I', 'lib/libharu/include',
    '-I', 'vendor/wkhtmltopdf/src/image',
    '-I', 'vendor/wkhtmltopdf/src/lib',
    '-I', 'vendor/wkhtmltopdf/src/pdf',
    '-I', 'vendor/wkhtmltopdf/src/shared',
    '-I', 'vendor/nginx/src/core',
    '-I', 'vendor/nginx/src/event',
    '-I', 'vendor/nginx/src/event/modules',
    '-I', 'vendor/nginx/src/http',
    '-I', 'vendor/nginx/src/http/v2',
    '-I', 'vendor/nginx/src/http/modules',
    '-I', 'vendor/nginx/src/http/modules/perl',
    '-I', 'vendor/nginx/src/mail',
    '-I', 'vendor/nginx/src/misc',
    '-I', 'vendor/nginx/src/os/unix',
    '-I', 'vendor/nginx/src/stream',
    '-I', 'vendor/nginx/objs',
    '-I', 'src',
    # std is required
    # clang won't know which language to use compiling headers
    '-isystem', '/usr/include',
    '-std=c11',
    # '-x' and 'c++' also required
    # use 'c' for C projects
    '-x', 'c',
]
 
# youcompleteme is calling this function to get flags
# You can also set database for flags. Check: JSONCompilationDatabase.html in
# clang-3.2-doc package
def FlagsForFile(filename):
  return {
    'flags': flags,
    'do_cache': True
}

