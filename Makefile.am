ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS =
sbin_PROGRAMS =
lib_LIBRARIES =
lib_LTLIBRARIES =

# extra_CFLAGS = -fasynchronous-unwind-tables -fexceptions -fstack-clash-protection -arch x86_64 -Wall -Wextra -Wconversion -Wformat-security 
extra_CFLAGS = -fexceptions -fstack-clash-protection -arch x86_64 -Wall -Wextra -Wconversion -Wformat-security 
debug_CFLAGS = -ggdb -Og
# debug_CFLAGS =

# library

shared_lib_connection_sources = lib/connection/connection.c
shared_lib_datatypes_sources = lib/datatypes/dstr.c

shared_lib_dnssd_sources = lib/dnssd/dnssd.c lib/dnssd/mdns-interface.c lib/dnssd/mdns-socket.c lib/dnssd/mdns-socket-ctx.c lib/dnssd/mdns-utils.c lib/dnssd/addrs.c

shared_lib_event_sources = lib/event/signal.c lib/event/lock.c lib/event/set.c
shared_lib_eventloop_sources = lib/eventloop/backend.c lib/eventloop/bae.c lib/eventloop/bevent.c lib/eventloop/ctx.c lib/eventloop/loop.c lib/eventloop/queue.c
shared_lib_file_sources = lib/file/file.c 
shared_lib_fs_sources = lib/fs/createremove.c lib/fs/fs.c lib/fs/open.c lib/fs/readwrite.c lib/fs/realpath.c lib/fs/set.c lib/fs/stat.c lib/fs/stat-utils.c lib/fs/statvfs.c lib/fs/symlink.c lib/fs/utils.c
shared_lib_hashtable_sources = lib/hashtable/table.c lib/hashtable/unique-id.c

shared_lib_io_sources = lib/io/backend.c lib/io/buffer.c lib/io/io.c lib/io/utils.c lib/io/connection/address.c lib/io/connection/cb.c lib/io/connection/compare.c lib/io/connection/copy.c lib/io/connection/init.c lib/io/connection/set.c lib/io/connection/utils.c
shared_lib_io_sources += lib/io/system/copy.c lib/io/system/init.c
shared_lib_ldap_sources = lib/ldap/ldap.c
shared_lib_list_sources = lib/list/header.c lib/list/insert.c lib/list/list.c lib/list/lock.c lib/list/next.c lib/list/remove.c lib/list/utils.c
shared_lib_lock_sources = lib/lock/lock.c
shared_lib_main_sources = lib/main/error.c lib/main/localthreads.c lib/main/logging.c
shared_lib_misc_sources = lib/misc/txtutils.c lib/misc/numutils.c
shared_lib_module_sources = lib/module/module.c
shared_lib_mountinfo_sources = lib/mountinfo/export.c lib/mountinfo/monitor.c lib/mountinfo/read.c lib/mountinfo/utils.c
shared_lib_network_sources = lib/network/address-list.c lib/network/address.c lib/network/dns.c lib/network/host.c lib/network/services.c
shared_lib_osns_sources = lib/osns/arguments.c lib/osns/config.c lib/osns/event.c lib/osns/interface.c lib/osns/modules.c lib/osns/osns.c lib/osns/pidfile.c lib/osns/start.c lib/osns/utils.c
shared_lib_path_sources = lib/path/append.c lib/path/compare.c lib/path/filename.c lib/path/path.c lib/path/seperator.c lib/path/utils.c
shared_lib_pid_sources = lib/pid/pid.c
shared_lib_sl_sources = lib/sl/init.c lib/sl/lock.c lib/sl/sl.c
shared_lib_socket_sources = lib/socket/socket.c lib/socket/listen.c lib/socket/ctx.c

shared_lib_system_sources = lib/system/fork.c lib/system/mount.c lib/system/signal.c
shared_lib_user_sources = lib/user/user.c lib/user/group.c
shared_lib_time_sources = lib/time/time.c


lib_LTLIBRARIES += libosns.la

libosns_la_SOURCES = $(shared_lib_connection_sources) $(shared_lib_datatypes_sources) $(shared_lib_dnssd_sources) $(shared_lib_eventloop_sources) $(shared_lib_event_sources) $(shared_lib_file_sources) $(shared_lib_fs_sources) $(shared_lib_hashtable_sources) $(shared_lib_io_sources) $(shared_lib_list_sources) $(shared_lib_lock_sources) $(shared_lib_ldap_sources) $(shared_lib_main_sources) \
                    $(shared_lib_misc_sources) $(shared_lib_module_sources) $(shared_lib_mountinfo_sources) $(shared_lib_network_sources) $(shared_lib_osns_sources) $(shared_lib_path_sources) $(shared_lib_pid_sources) $(shared_lib_sl_sources) $(shared_lib_socket_sources) $(shared_lib_system_sources) $(shared_lib_user_sources) $(shared_lib_time_sources)

# libosns_system_la_SOURCES = $(shared_lib_mountinfo_sources) $(shared_lib_dnssd_sources)

#

#libosns_la_LDFLAGS = -module -avoid-version -version-info 0:0:0

libosns_la_LDFLAGS = -avoid-version
libosns_la_CFLAGS = $(CFLAGS) $(debug_CFLAGS) -D_FILE_OFFSET_BITS=64 -lz -lpthread -lm -fPIC -shared -I . -I include -I ./lib -Wall -Wno-unused-but-set-variable -Wno-uninitialized -Wno-unused-variable -Wno-unused-label -Wno-unused-function

##libosns_system_la_LDFLAGS = -avoid-version
##libosns_system_la_CFLAGS = $(CFLAGS) $(debug_CFLAGS) -D_FILE_OFFSET_BITS=64 -lz -lpthread -lm -fPIC -shared -I . -I include -I ./lib -Wall -Wno-unused-but-set-variable -Wno-uninitialized -Wno-unused-variable -Wno-unused-label -Wno-unused-function
