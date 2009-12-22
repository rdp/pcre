require 'mkmf'
inc, lib = dir_config('pcre', '/usr/local')
have_header('pcre.h')
case RUBY_PLATFORM
when /bccwin32|mswin32|mingw/
  have_library('libpcre')
else
  $LIBS += ' -lpcre'
end

create_makefile("pcre")

