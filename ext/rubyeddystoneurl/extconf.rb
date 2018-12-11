require 'mkmf'

#have_library 'stdc++'
extension_name = 'rubyeddystoneurl/rubyeddystoneurl'
dir_config extension_name

$CPPFLAGS << " -I/usr/include/bluetooth"
$LOCAL_LIBS << " -lbluetooth"

create_makefile extension_name