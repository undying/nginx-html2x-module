
import os

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
    '-I', 'vendor/nginx/objs',
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
    '-I', 'src',
    '-isystem', '/usr/include',
]

flags_c = (
    '-std=c11',
    '-x', 'c',
)

flags_cpp = (
    '-std=c++17',
    '-x', 'c++',
)

ext_cpp = ('.cc', '.hh')


def getFlags(ext):
    if ext in ext_cpp:
        flags.extend(flags_cpp)
        return flags

    flags.extend(flags_c)
    return flags


def getFileExt(filename=None):
    file_ext = os.path.splitext(filename)
    if len(file_ext) > 1:
        return file_ext[1]
    return '.c'


# youcompleteme is calling this function to get flags
# You can also set database for flags. Check: JSONCompilationDatabase.html in
# clang-3.2-doc package
def Settings(**kwargs):
    return {
        'flags': getFlags(getFileExt(kwargs['filename'])),
        'do_cache': True
    }
