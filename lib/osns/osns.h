/* SPDX-License-Identifier: GLP-2.0-only */

#ifndef OSNS_OSNS_H
#define OSNS_OSNS_H

#include "libosns-fs.h"
#include "libosns-connection.h"
#include "libosns-socket.h"
#include "libosns-pid.h"
#include "libosns-module.h"

#ifdef HAVE_LDAP

#include <ldap.h>

#endif

struct osns_ctx_s;

// #include "osns-protocol.h"

#define OSNS_DATA_LINK_TYPE_CONTEXT					1
#define OSNS_DATA_LINK_TYPE_ID						2
#define OSNS_DATA_LINK_TYPE_SPECIAL_ENTRY				3
#define OSNS_DATA_LINK_TYPE_SYMLINK					4
#define OSNS_DATA_LINK_TYPE_DATA					5
#define OSNS_DATA_LINK_TYPE_DIRECTORY					6
#define OSNS_DATA_LINK_TYPE_CACHE					7

union data_link_u {
    void								*ptr;
    uint64_t								id;
};

struct osns_data_link_s {
    unsigned char							type;
    union data_link_u							link;
};

#define OSNS_DATA_LINK_INIT						{0}

/* database */

#define OSNS_DB_TYPE_FS							0
#define OSNS_DB_TYPE_LDAP						1
#define OSNS_DB_TYPE_SQLITE						2

struct osns_db_handle_s {
    unsigned char							type;
    struct list_element_s						list;
    struct osns_ctx_s							*octx;
    union osns_sb_hamdle_u {

#ifdef HAVE_LDAP

	LDAP								*ld;

#endif

#ifdef HAVE_SQLITE

	sqlite3								*sql;

#endif

	struct fs_object_s						*fso;

    } db;

};

union osns_db_base_u {
    char								*name;
    char								*ldap;
};

#define OSNS_DB_CTX_LOCK_HANDLES					1

struct osns_db_ctx_s {
    unsigned char							type;
    unsigned int							lock;
    struct list_header_s						handles;
    union osns_db_ctx_u {
	struct fs_path_s						root;
	const char							*bn;
    } db;
};

struct osns_value_s {
    unsigned char							type;
    void								*ptr;
};

#define OSNS_DNSSD_MODUS_REMOVE_TREE					-2
#define OSNS_DNSSD_MODUS_REMOVE						-1
#define OSNS_DNSSD_MODUS_IGNORE						0
#define OSNS_DNSSD_MODUS_INSERT_OR_REPLACE				1
#define OSNS_DNSSD_MODUS_INSERT_OR_IGNORE				2

#define OSNS_OPTION_ORIGIN_INIT                                         0
#define OSNS_OPTION_ORIGIN_DEFAULT                                      1
#define OSNS_OPTION_ORIGIN_SYSTEM                                       2
#define OSNS_OPTION_ORIGIN_USER                                         3
#define OSNS_OPTION_ORIGIN_ARGUMENT                                     4

#define OSNS_OPTION_HOW_SET                                             0
#define OSNS_OPTION_HOW_UNSET                                           1
#define OSNS_OPTION_HOW_OR                                              2
#define OSNS_OPTION_HOW_AND                                             3

struct osns_option_fs_path_s {
    struct fs_path_s                                                    value;
    unsigned char                                                       origin;
};

struct osns_option_dstr_s {
    struct dstr_s                                                       value;
    unsigned char                                                       origin;
};

struct osns_option_uint_s {
    uint64_t                                                            value;
    unsigned char                                                       origin;
};

#define OSNS_OPTIONS_MAIN_MAXTHREADS					20

struct osns_client_options_s {
    struct dstr_s							services;
};

struct osns_options_s {
    struct osns_option_fs_path_s                                        runpath;
    struct osns_option_fs_path_s                                        etcpath;
    struct osns_option_fs_path_s                                        execpath;
    struct osns_option_fs_path_s                                        ldappath;
    struct osns_option_dstr_s                                           group;
    struct osns_option_uint_s			                        maxthreads;
    struct osns_option_uint_s                                           argumentflags;
    union osns_role_options_u {
        struct osns_client_options_s                                    client;
    } role;
};

struct osns_param_s {
    unsigned char                                                       type;
    struct dstr_s                                                       name;
    struct dstr_s                                                       value;
};

struct osns_ctx_s;

/* event subscription (internal ipc) */

#define OSNS_EVENT_TYPE_MOUNTINFO                                       0
#define OSNS_EVENT_TYPE_SIGNAL                                          1

#define OSNS_EVENT_MASK_MOUNTINFO                                       (1 << OSNS_EVENT_TYPE_MOUNTINFO)
#define OSNS_EVENT_MASK_SIGNAL                                          (1 << OSNS_EVENT_TYPE_SIGNAL)

struct osns_event_mountinfo_s {
    unsigned char                                                       added;
    struct mountinfo_export_s                                           *mexport;
};

struct osns_event_system_signal_s {
    unsigned int                                                       	code;
    pid_t                                                              	pid;
    union system_signal_type_u                                          *type;
};

struct osns_event_s {
    unsigned int                                                        type;
    union osns_event_u {
        struct osns_event_mountinfo_s                                   mountinfo;
        struct osns_event_system_signal_s                               signal;
    } event;
};

struct osns_event_subscription_s {
    char                                                                *name;
    struct list_element_s                                               list;
    unsigned int                                                        mask;
    void                                                                *ptr;
    void                                                               	(* cb)(struct osns_ctx_s *octx, unsigned int mask, struct osns_event_s *event, void *ptr);
};

#define OSNS_EVENT_CTX_LOCK_SUBSCRIPTIONS                               4

struct osns_event_ctx_s {
    unsigned int                                                       lock;
    struct list_header_s                                                *subscriptions;
    unsigned int                                                        mask; /* total mask of all subscribers */
};

#define OSNS_PROCESS_STATUS_INIT                                       	1
#define OSNS_PROCESS_STATUS_RUNNING                                     2

struct osns_process_s {
    unsigned int                                                        status;
    unsigned char                                                       role2launched;
    struct pid_info_s                                                   pinfo;
    struct list_element_s                                               list;
};

#define OSNS_PROCESS_CTX_LOCK_PROCESSES                                 1

struct osns_process_ctx_s {
    unsigned int                                                        lock;
    struct list_header_s                                                processes;
};

/* FUSE ctx */

struct osns_fuse_ctx_s {
    struct list_header_s						modules;
    struct list_header_s						interfaces;
    struct list_header_s						workspaces;
};

#define _IO_OPTION_TYPE_INT				1
#define _IO_OPTION_TYPE_PCHAR				2
#define _IO_OPTION_TYPE_PVOID				3
#define _IO_OPTION_TYPE_BUFFER				4

#define _IO_OPTION_FLAG_ERROR				1
#define _IO_OPTION_FLAG_ALLOC				2
#define _IO_OPTION_FLAG_NOTDEFINED			4
#define _IO_OPTION_FLAG_DEFAULT				8
#define _IO_OPTION_FLAG_VALID				16

struct io_option_s {
    unsigned char					type;
    unsigned char					flags;
    union {
	unsigned int					integer;
	char						*name;
	void						*ptr;
	struct io_option_buffer_s {
	    char					*ptr;
	    unsigned int				size;
	    unsigned int				len;
	} buffer;
    } value;
    void						(* free)(struct io_option_s *o);
};

#define CONTEXT_INTERFACE_TYPE_FS					1
#define CONTEXT_INTERFACE_TYPE_CONNECTOR				2

#define CONTEXT_INTERFACE_GROUP_NETWORK					1
#define CONTEXT_INTERFACE_GROUP_BACKUP					2
#define CONTEXT_INTERFACE_GROUP_DEVICE					3
#define CONTEXT_INTERFACE_GROUP_SOCKET					4

#define CONTEXT_INTERFACE_FLAG_PRIMARY					(1 << 0)
#define CONTEXT_INTERFACE_FLAG_SECONDARY				(1 << 1)
#define CONTEXT_INTERFACE_FLAG_1TON					(1 << 2)
#define CONTEXT_INTERFACE_FLAG_1TO1					(1 << 3)
#define CONTEXT_INTERFACE_FLAG_1OFN					(1 << 4)

#define CONTEXT_INTERFACE_STATUS_INIT					1
#define CONTEXT_INTERFACE_STATUS_CONNECT				2
#define CONTEXT_INTERFACE_STATUS_UP					3
#define CONTEXT_INTERFACE_STATUS_DOWN					4
#define CONTEXT_INTERFACE_STATUS_CLOSING				5
#define CONTEXT_INTERFACE_STATUS_CLOSED					6
#define CONTEXT_INTERFACE_STATUS_CLEAR					7

struct context_interface_s {
    unsigned char							type;
    unsigned char							group;
    unsigned int							flags;
    unsigned int							status;
    unsigned int							lock;
    void								*ptr;
    int 								(* connect)(struct context_interface_s *i);
    char								*(* get_interface_buffer)(struct context_interface_s *i);
    struct context_interface_s                          		*(* get_primary)(struct context_interface_s *i);
    void                                                		(* set_primary)(struct context_interface_s *i, struct context_interface_s *p, unsigned char role);
    void                                                		(* set_secondary)(struct context_interface_s *i, struct context_interface_s *s, unsigned char role);
    struct context_interface_iocmd_s {
	int								(* in)(struct context_interface_s *i, const char *what, struct io_option_s *option, struct context_interface_s *s, unsigned int type);
	int								(* out)(struct context_interface_s *i, const char *what, struct io_option_s *option, struct context_interface_s *s, unsigned int type);
    } iocmd;
    union interface_link_u {
	struct context_interface_secondary_s {
	    struct context_interface_s					*primary;
	} secondary;
	struct context_interface_primary_s {
	    union context_interface_primary_u {
		unsigned int						refcount;
		struct context_interface_s				*secondary;
	    } relation;
	} primary;
    } link;

    /* buffer for interface specific data like SSH session, SFTP client, SMB client etc */

    unsigned int							size;
    char								buffer[];
};

struct context_interface_ops_s {
    char								*name;
    int									(* init_buffer)(struct context_interface_s *i, struct context_interface_ops_s *ops, struct context_interface_s *primary);
    void								(* clear_buffer)(struct context_interface_s *i);
};

#define SERVICE_CTX_TYPE_DUMMY						0
#define SERVICE_CTX_TYPE_WORKSPACE					1
#define SERVICE_CTX_TYPE_BROWSE						2
#define SERVICE_CTX_TYPE_FILESYSTEM					3
#define SERVICE_CTX_TYPE_SHARED						4

#define SERVICE_CTX_NAME_LENGTH                 			32
#define SERVICE_FILESYSTEM_NAME_LENGTH          			64

/* TO DO:
    - CTX TYPE ID: fs based upon id's provided by remote server */

#define SERVICE_CTX_FLAG_ALLOC						( 1 << 0 )
#define SERVICE_CTX_FLAG_LOCKED						( 1 << 1 )
#define SERVICE_CTX_FLAG_TOBEDELETED		 			( 1 << 2 )
#define SERVICE_CTX_FLAG_WLIST						( 1 << 3 )
#define SERVICE_CTX_FLAG_CLIST						( 1 << 4 )
#define SERVICE_CTX_FLAG_FS                     			( 1 << 5 )
#define SERVICE_CTX_FLAG_THREAD                 			( 1 << 6 )
#define SERVICE_CTX_FLAG_REFRESH                			( 1 << 7 )

#define SERVICE_WORKSPACE_FLAG_PATH             			1
#define SERVICE_WORKSPACE_FLAG_INODES           			2
#define SERVICE_WORKSPACE_FLAG_DIRECTORIES      			4
#define SERVICE_WORKSPACE_FLAG_SYMLINKS         			8

#define SERVICE_BROWSE_TYPE_NETWORK					1
#define SERVICE_BROWSE_TYPE_NETGROUP					2
#define SERVICE_BROWSE_TYPE_NETHOST					3
#define SERVICE_BROWSE_TYPE_NETSOCKET					4

#define SERVICE_BROWSE_FLAG_REFRESH_LOOKUP      			1
#define SERVICE_BROWSE_FLAG_REFRESH_OPENDIR     			2

#define SERVICE_OP_TYPE_LOOKUP						0
#define SERVICE_OP_TYPE_LOOKUP_EXISTING					1
#define SERVICE_OP_TYPE_LOOKUP_NEW					2

#define SERVICE_OP_TYPE_GETATTR						3
#define SERVICE_OP_TYPE_SETATTR						4
#define SERVICE_OP_TYPE_READLINK					5

#define SERVICE_OP_TYPE_MKDIR						6
#define SERVICE_OP_TYPE_MKNOD						7
#define SERVICE_OP_TYPE_SYMLINK						8
#define SERVICE_OP_TYPE_CREATE						9

#define SERVICE_OP_TYPE_UNLINK						10
#define SERVICE_OP_TYPE_RMDIR						11

#define SERVICE_OP_TYPE_RENAME						12

#define SERVICE_OP_TYPE_OPEN						13
#define SERVICE_OP_TYPE_READ						14
#define SERVICE_OP_TYPE_WRITE						15
#define SERVICE_OP_TYPE_FLUSH						16
#define SERVICE_OP_TYPE_FSYNC						17
#define SERVICE_OP_TYPE_RELEASE						18
#define SERVICE_OP_TYPE_FGETATTR					19
#define SERVICE_OP_TYPE_FSETATTR					20

#define SERVICE_OP_TYPE_GETLOCK						21
#define SERVICE_OP_TYPE_SETLOCK						22
#define SERVICE_OP_TYPE_SETLOCKW					23
#define SERVICE_OP_TYPE_FLOCK						24

#define SERVICE_OP_TYPE_OPENDIR						25
#define SERVICE_OP_TYPE_READDIR						26
#define SERVICE_OP_TYPE_READDIRPLUS					27
#define SERVICE_OP_TYPE_RELEASEDIR					28
#define SERVICE_OP_TYPE_FSYNCDIR					29

#define SERVICE_OP_TYPE_GETXATTR					30
#define SERVICE_OP_TYPE_SETXATTR					31
#define SERVICE_OP_TYPE_LISTXATTR					32
#define SERVICE_OP_TYPE_REMOVEXATTR					33

#define SERVICE_OP_TYPE_STATFS						34

/*
    TODO:
    - git
    - Google Drive
    - Microsoft One Drive
    - Amazon
    - Nextcloud
    - backup

*/

struct browse_service_fs_s;
struct path_service_fs_s;
struct fuse_path_s;

struct service_context_s {
    unsigned int							type;
    uint32_t								flags;
    char								name[SERVICE_CTX_NAME_LENGTH];
    struct osns_data_link_s						link;
    union {
	struct workspace_context_s {
	    unsigned int                                		status;
	    struct event_shared_signal_s				*esignal;
	    struct list_header_s					header;
	    struct browse_service_fs_s					*fs; 		/* the fs used for browsing the network map */
            void							*rootinode;
            uint64_t 							nrinodes;
            struct list_header_s					directories;
            struct list_header_s					symlinks;
            struct list_header_s					forget;
            uint16_t                                    		pathmax;
	} workspace;
	struct browse_context_s {
	    unsigned int                                		status;
	    struct browse_service_fs_s					*fs; 		/* the fs used for browsing the network map */
	    struct list_element_s					clist;		/* the list of the parent: part of a tree */
	    unsigned int						type;		/* type: network, netgroup or nethost or netsocket */
	    struct list_header_s					header;		/* has children */
	    struct timespec_s						refresh_lookup;	/* time of latest refresh: to keep cache and contexes in sync */
	    struct timespec_s						refresh_opendir;
	    uint64_t							unique;
	    unsigned int						service;
	    pthread_t							threadid;
	} browse;
	struct filesystem_context_s {
	    unsigned int                                		status;
	    struct path_service_fs_s					*fs; 		/* the path based fs used for this service: sftp, webdav, smb ... */
	    struct list_element_s					clist;		/* the list of the parent: part of a tree */
	    struct inode_s 						*inode; 	/* the inode the service is attached to */
	    struct shared_signal_s					*signal;
	    struct timespec_s						refresh;	/* time of latest refresh: to keep cache and contexes in sync */
	    char							name[SERVICE_FILESYSTEM_NAME_LENGTH];
	    unsigned int						service;
	    struct list_header_s					pathcaches;
	} filesystem;
	struct shared_context_s {
	    struct timespec_s						refresh;	/* time of latest refresh: to keep cache and contexes in sync */
	    uint64_t							unique;
	    unsigned int						service;
	    unsigned int						transport;
	} shared;
    } service;
    struct list_element_s						wlist;
    struct context_interface_s						interface;
};

#define OSNS_WORKSPACE_INODE_HASHTABLE_SIZE				512

#define OSNS_WORKSPACE_TYPE_FS						CONTEXT_INTERFACE_TYPE_FS
#define OSNS_WORKSPACE_TYPE_CONNECTOR					CONTEXT_INTERFACE_TYPE_CONNECTOR

#define OSNS_WORKSPACE_GROUP_NETWORK					CONTEXT_INTERFACE_GROUP_NETWORK
#define OSNS_WORKSPACE_GROUP_BACKUP					CONTEXT_INTERFACE_GROUP_BACKUP
#define OSNS_WORKSPACE_GROUP_DEVICE					CONTEXT_INTERFACE_GROUP_DEVICE
#define OSNS_WORKSPACE_GROUP_SOCKET					CONTEXT_INTERFACE_GROUP_SOCKET

#define OSNS_WORKSPACE_FLAG_ALLOC					1

#define OSNS_WORKSPACE_LOCK_PATHMAX					(1 << 0)
#define OSNS_WORKSPACE_LOCK_CONTEXES					(1 << 1)
#define OSNS_WORKSPACE_LOCK_INODES					(1 << 2)
#define OSNS_WORKSPACE_LOCK_DELETE_INODES_THREAD			(1 << 3)
#define OSNS_WORKSPACE_STATUS_LOCK_FORGET				(1 << 4)
#define OSNS_WORKSPACE_STATUS_LOCK_SYMLINK				(1 << 5)

#define OSNS_WORKSPACE_STATUS_INIT					1
#define OSNS_WORKSPACE_STATUS_CLEAR					2

struct osns_workspace_s {
    unsigned char 							type;
    unsigned char							group;
    unsigned int							flags;
    unsigned int							status;
    struct event_shared_signal_s					*esignal;
    struct timespec_s							syncdate;
    struct list_header_s						contexes;
    struct list_element_s						list;
};

#define OSNS_MODULE_TYPE_FUSE						1

struct osns_module_s {
    char								*name;
    unsigned int							type;
    struct list_element_s						list;
    union osns_module_u {
	struct osns_fuse_module_s {
	    unsigned int						flags;
	} fuse;
    } interface;
    unsigned int							(* populate)(struct osns_ctx_s *octx, struct osns_module_s *omod, struct context_interface_ops_s *ops);
    struct module_s							module;
};

#define OSNS_CTX_ROLE_NOTSET                                            0
#define OSNS_CTX_ROLE_SYSTEM                                            1
#define OSNS_CTX_ROLE_CLIENT                                            2
#define OSNS_CTX_ROLE_APP                                               3
#define OSNS_CTX_ROLE_HLPR                                              4
#define OSNS_CTX_ROLE_UNKNOWN                                           5

#define OSNS_CTX_STATUS_THREAD_STARTED                                  1
#define OSNS_CTX_STATUS_CONNECTED                                       2
#define OSNS_CTX_STATUS_SETUP                                           4
#define OSNS_CTX_STATUS_REMOVED                                       	8

#define OSNS_CTX_LOCK_MODULES                                           1
#define OSNS_CTX_LOCK_ACTIONS                                           2

struct osns_arguments_s;

struct osns_ctx_s {
    unsigned char                                                       role;
    unsigned int                                                        status;
    unsigned int                                                        lock;
    struct osns_arguments_s                                             *arguments;
    struct event_shared_signal_s                                        *esignal;
    struct osns_options_s                                               *options;
    struct pid_info_s                                                   *pinfo;
    struct user_s                                                       *user;
    struct dstr_s                                                       *program;
    struct osns_event_ctx_s                                             *event_ctx;
    struct osns_process_ctx_s                                           *process_ctx;
    struct osns_fuse_ctx_s						*fuse_ctx;
    struct osns_db_ctx_s						*db_ctx;
    struct list_header_s                                                actions;
};

#define OSNS_CTX_ACTION_CODE_DO                                         1
#define OSNS_CTX_ACTION_CODE_UNDO                                       2

#define OSNS_CTX_ACTION_STATUS_DONE                                     1
#define OSNS_CTX_ACTION_STATUS_UNDONE                                   2
#define OSNS_CTX_ACTION_STATUS_ERROR                                    4

struct osns_ctx_action_s {
    struct list_element_s                                               list;
    const char                                                          *name;
    unsigned int                                                        status;
    int                                                                 (* manage)(struct osns_ctx_s *octx, unsigned char actioncode, struct osns_ctx_action_s *action);
    void                                                                *ptr;
};

/* prototypes */

unsigned int OSNS_utils_create_version(unsigned int major, unsigned int minor);
unsigned int OSNS_utils_get_major(unsigned int version);
unsigned int OSNS_utils_get_minor(unsigned int version);

const char *OSNS_get_name_from_role(unsigned char role);
const unsigned char OSNS_get_role_from_name(struct dstr_s *name);

void OSNS_ctx_init(struct osns_ctx_s *octx, unsigned char role, struct event_shared_signal_s *esignal, struct osns_options_s *options, struct user_s *user, struct pid_info_s *pinfo, struct dstr_s *program, struct osns_arguments_s *arg);

#endif
