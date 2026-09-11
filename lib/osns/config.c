/* SPDX-License-Identifier: GLP-2.0-only */

#include "libosns-basic-system-headers.h"
#include "libosns-defaults.h"

#include "libosns-misc.h"
#include "libosns-log.h"
#include "libosns-path.h"
#include "libosns-file.h"
#include "libosns-user.h"

#include "osns.h"
#include "config.h"

static void osns_option_fs_path_set(struct osns_option_fs_path_s *oofp, unsigned char type, void *ptr, unsigned char origin, unsigned char alloc)
{
    oofp->origin=origin;
    int tmp=FS_path_copy(&oofp->value, type, ptr, alloc);
}

static void osns_option_dstr_set(struct osns_option_dstr_s *ood, char *value, unsigned char origin, unsigned char alloc)
{
    unsigned int length=0;

    ood->origin=origin;
    DSTR_clear(&ood->value);
    if ((value==NULL) || ((length=strlen(value))==0)) return;

    if (alloc) {

        if (DSTR_create(&ood->value, length, 0)) length=DSTR_copy_bytes(&ood->value, value, length);

    } else {

        DSTR_set_bytes(&ood->value, value, length, 0);

    }

}

static void osns_option_dstr_append(struct dstr_s *config, char *value, unsigned char origin)
{

    if (DSTR_append(config, ',', value, 0, 1, 0)==0) {

	logoutput_warning("%s: unable to append %s", __FUNCTION__, value);

    }

}

static void osns_option_uint_set(struct osns_option_uint_s *oou, uint64_t value, unsigned char origin, unsigned char how, unsigned int flags)
{
    if (how==OSNS_OPTION_HOW_SET) {

        oou->origin=origin;
        oou->value=value;

    } else if (how==OSNS_OPTION_HOW_UNSET) {

        oou->value&=~value;

    } else if (how==OSNS_OPTION_HOW_OR) {

        if (oou->value==0) oou->origin=origin;
        oou->value|=value;

    } else if (how==OSNS_OPTION_HOW_AND) {

        oou->value&=value;

    }

}

void OSNS_config_init(struct osns_options_s *options, unsigned char role)
{

    memset(options, 0, sizeof(struct osns_options_s));

    osns_option_fs_path_set(&options->runpath, 'c', OSNS_DEFAULT_RUNPATH, OSNS_OPTION_ORIGIN_DEFAULT, 0);
    osns_option_fs_path_set(&options->etcpath, 'c', OSNS_DEFAULT_ETCPATH, OSNS_OPTION_ORIGIN_DEFAULT, 0);
    osns_option_fs_path_set(&options->execpath, 'c', OSNS_DEFAULT_EXECPATH, OSNS_OPTION_ORIGIN_DEFAULT, 0);
    osns_option_fs_path_set(&options->ldappath, 'c', OSNS_DEFAULT_LDAPPATH, OSNS_OPTION_ORIGIN_DEFAULT, 0);

    osns_option_dstr_set(&options->group, OSNS_DEFAULT_UNIXGROUP, OSNS_OPTION_ORIGIN_DEFAULT, 0);
    osns_option_uint_set(&options->maxthreads, OSNS_DEFAULT_MAXTHREADS, OSNS_OPTION_ORIGIN_DEFAULT, OSNS_OPTION_HOW_SET, 0);
    osns_option_uint_set(&options->argumentflags, 0, OSNS_OPTION_ORIGIN_DEFAULT, OSNS_OPTION_HOW_SET, 0);

    if (role==OSNS_CTX_ROLE_CLIENT) {

        DSTR_init(&options->role.client.services);
        options->role.client.fuse_default_blocksize=4096;
        options->role.client.protocols=NULL;

    }

}

static void osns_option_set_uint_hlpr(struct osns_option_uint_s *oou, struct dstr_s *value, uint64_t param, unsigned char origin, unsigned int flags)
{
    long numericvalue=DSTR_convert_str_to_long(value);
    unsigned char how=(numericvalue>0) ? OSNS_OPTION_HOW_SET : OSNS_OPTION_HOW_UNSET;

    osns_option_uint_set(oou, param, origin, how, flags);

}

struct cb_read_osns_conf_hlpr_s {
    unsigned char               role;
    unsigned char               origin;
    struct osns_options_s       *options;
};

static unsigned char cb_read_osns_conf(struct dstr_s *line, void *ptr)
{
    struct cb_read_osns_conf_hlpr_s *hlpr=(struct cb_read_osns_conf_hlpr_s *) ptr;
    struct osns_options_s *options=hlpr->options;
    struct dstr_s option=DSTR_INIT;

    if (DSTR_get_first_dstr(line, '=', &option, 1, 0)==0) return 0;

    if (DSTR_cmp_bytes(&option, "main.maxthreads", 0, 1, 0, 0)) {
    	long numericvalue=DSTR_convert_str_to_long(line);

        osns_option_uint_set(&options->maxthreads, numericvalue, hlpr->origin, OSNS_OPTION_HOW_SET, 0);
        logoutput_debug("%s: found option main.maxthreads, value %lu", __FUNCTION__, numericvalue);

    } else if (DSTR_cmp_bytes(&option, "client.", 0, 0, 0, 0)) {

        if (hlpr->role != OSNS_CTX_ROLE_CLIENT) return 0;

        if (DSTR_cmp_bytes(&option, "client.services", 0, 1, 0, 0)) {

            /* parse a comma seperated list */

            struct dstr_s service=DSTR_INIT;

            logoutput_debug("%s: found option client.services %.*s", __FUNCTION__, line->length, line->str);

            while (DSTR_get_first_dstr(line, ',', &service, 1, 1)) {

                if (DSTR_cmp_bytes(&service, "fuse", 0, 1, 0, 0)) {

		    osns_option_dstr_append(&options->role.client.services, "fuse", hlpr->origin);
                    logoutput_debug("%s: found option client.services enabling fuse", __FUNCTION__);

                } else if (DSTR_cmp_bytes(&service, "connector", 0, 1, 0, 0)) {

		    osns_option_dstr_append(&options->role.client.services, "connector", hlpr->origin);
                    logoutput_debug("%s: found option client.services enabling connector", __FUNCTION__);

		}

            }

        }

    }

    return 0;

}

static void osns_config_read_configfile(struct fs_path_s *path, struct osns_options_s *options, unsigned char origin, unsigned char role)
{
    struct cb_read_osns_conf_hlpr_s hlpr;

    logoutput_debug("%s: look for config file %.*s", __FUNCTION__, path->start.length, path->start.str);

    hlpr.role=role;
    hlpr.origin=origin;
    hlpr.options=options;

    if (FILE_parse(path, cb_read_osns_conf, (void *) &hlpr)==0) {

        logoutput_debug("%s: unable to read config file %.*s error %s", __FUNCTION__);

    }

}

void OSNS_config_read_config(struct user_s *user, struct osns_options_s *options, unsigned char role)
{
    struct fs_path_s path=FS_PATH_INIT;
    const char *name=OSNS_get_name_from_role(role);
    struct cb_read_osns_conf_hlpr_s hlpr;

    if (role==0) return;
    hlpr.role=role;
    hlpr.options=options;

    FS_path_append_init(&path, FS_PATH_FLAG_BUFFER_ALLOC);

    if (options->etcpath.value.start.length) {

        if (FS_path_append(&path, 'p', (void *) &options->etcpath.value, 0)==0) goto out;

    } else {

        if (FS_path_append(&path, 'c', (void *) OSNS_DEFAULT_ETCPATH, 0)==0) goto out;

    }

    /* first read the main osns.conf */

    if (FS_path_append(&path, 'c', (void *) "osns.conf", 1)==0) goto out;
    osns_config_read_configfile(&path, options, OSNS_OPTION_ORIGIN_SYSTEM, role);

    if (FS_path_get_filename(&path, NULL, 1)) {
        unsigned int size=32 + strlen(name);
        char roleconfigfile[size];
        int tmp=snprintf(roleconfigfile, size, "%s.conf", name);

        if (FS_path_append(&path, 'c', roleconfigfile, 1)==0) goto out;
        osns_config_read_configfile(&path, options, OSNS_OPTION_ORIGIN_SYSTEM, role);

    }

    FS_path_clear(&path);
    FS_path_append_init(&path, FS_PATH_FLAG_BUFFER_ALLOC);

    if (USER_is_desktop_user(user)) {
        struct user_directory_s ud;

        USER_directory_init(&ud);

        if (USER_get_directory(user, USER_DIRECTORY_TYPE_HOME, &ud, USER_DIRECTORY_FLAG_COPY)) {
            unsigned int size=64 + strlen(name);
            char roleconfigfile[size];
            int tmp=snprintf(roleconfigfile, size, ".config/osns/%s.conf", name);

            if (FS_path_append(&path, 'p', (void *) &ud.path, 0)==0) goto out;
            if (FS_path_append(&path, 'c', roleconfigfile, 1)==1) osns_config_read_configfile(&path, options, OSNS_OPTION_ORIGIN_SYSTEM, role);

        }

        USER_directory_clear(&ud);

    }

    out:

    FS_path_clear(&path);

}

void OSNS_config_free_options(struct osns_options_s *options, unsigned char role)
{

    FS_path_clear(&options->runpath.value);
    FS_path_clear(&options->etcpath.value);
    FS_path_clear(&options->execpath.value);
    FS_path_clear(&options->ldappath.value);

    DSTR_clear(&options->group.value);

    if (role==OSNS_CTX_ROLE_CLIENT) {

	DSTR_clear(&options->role.client.services);

    }
}
