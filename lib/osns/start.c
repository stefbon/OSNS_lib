/* SPDX-License-Identifier: GLP-2.0-only */

#include "libosns-basic-system-headers.h"
#include "libosns-defaults.h"

#include "libosns-log.h"
#include "libosns-list.h"
#include "libosns-datatypes.h"
#include "libosns-path.h"
#include "libosns-threads.h"
#include "libosns-file.h"

#include "arguments.h"
#include "osns.h"
#include "modules.h"

static const char *osns_action_unknown_name="unknown";

static int osns_process_actions_list(struct osns_ctx_s *octx, struct list_element_s **p_list, unsigned char direction, unsigned char actioncode)
{
    int result=0;
    struct list_element_s *list=*p_list;

    while (list) {
        struct osns_module_s *module=(struct osns_module_s *)((char *)list - offsetof(struct osns_module_s, list));
        struct osns_ctx_action_s *action=module->action;

	/* the module has to be loaded */

	if (module->type==OSNS_MODULE_TYPE_EXTERN) {

	    if ((module->status & OSNS_MODULE_STATUS_LOADED)==0) {

		logoutput_debug("%s: module not loaded .... skip", __FUNCTION__);
		goto nextprev;

	    }

	}

	/* the action has to be defined */

	if (action==NULL) goto nextprev;

    	if (actioncode==OSNS_CTX_ACTION_CODE_DO) {

	    logoutput_debug("%s: starting module action %s", __FUNCTION__, action->name);

    	    result=(* action->manage)(octx, actioncode, action, 0);

    	    if (result==-1) {

            	logoutput_debug("%s: module action %s gives error .... skip", __FUNCTION__, action->name);
            	module->status |= OSNS_MODULE_STATUS_ERROR;
            	break;

    	    }

    	    module->status |= OSNS_MODULE_STATUS_DONE;

    	} else if (actioncode==OSNS_CTX_ACTION_CODE_UNDO) {

    	    /* only undo if done earlier and no error */

    	    if (module->status & OSNS_MODULE_STATUS_DONE) {

            	result=(* action->manage)(octx, actioncode, action, 0);
            	module->status &= ~OSNS_MODULE_STATUS_DONE;

	    }

	    if (module->type==OSNS_MODULE_TYPE_EXTERN) {

		OSNS_module_unload(module);
		module->status &= ~OSNS_MODULE_STATUS_LOADED;

	    }

        }

	nextprev:

        list=(direction==1) ? LIST_element_get_next(list) : LIST_element_get_prev(list);

    }

    *p_list=list;
    return result;

}

static void osns_process_actions_thread(void *ptr)
{
    struct osns_ctx_s *octx=(struct osns_ctx_s *) ptr;
    struct timespec_s timeout=TIME_INIT;
    struct list_element_s *list=NULL;
    int result=0;

    /* signal the thread which started this thread it's up */

    EVENT_signal_set_flag(octx->esignal, &octx->status, OSNS_CTX_STATUS_THREAD_STARTED);

    /* wait for the eventloop to start */

    TIME_add(&timeout, TIME_ADD_ZERO, 2);

    if (BEVENTLOOP_wait_to_start(&timeout)==0) {

        logoutput_debug("%s: eventloop not started .... cannot continue", __FUNCTION__);
        result=-1;
        goto out;

    }

    /* exec the actions on the actions list */

    list=LIST_header_get_first(&octx->actions);
    result=osns_process_actions_list(octx, &list, 1, OSNS_CTX_ACTION_CODE_DO);

    out:

    if (result==-1) {

        logoutput_debug("%s: ... stop eventloop by sending sigtem signal", __FUNCTION__);
        BEVENTLOOP_stop(SIGTERM);

    }

}

void OSNS_action_process_all_undo(struct osns_ctx_s *octx)
{
    struct list_element_s *list=LIST_header_get_last(&octx->actions);
    int result=osns_process_actions_list(octx, &list, 0, OSNS_CTX_ACTION_CODE_UNDO);
}

void OSNS_action_process_all_start(struct osns_ctx_s *octx)
{
    struct timespec_s expire;

    if (octx==NULL) {

        logoutput_debug("%s: not all arguments set ... cannot continue", __FUNCTION__);
        goto errorout;

    }

    if (octx->actions.count==0) {

        logoutput_debug("%s: no actions found ... no need to continue", __FUNCTION__);
        return;

    }

    LOCAL_threads_put_job(0, osns_process_actions_thread, (void *) octx);

    /* check the thread is started */

    TIME_now(&expire);
    TIME_add(&expire, TIME_ADD_ZERO, 2);

    if (EVENT_signal_wait_flag_set(octx->esignal, &octx->status, OSNS_CTX_STATUS_THREAD_STARTED, &expire)==0) {

        logoutput_debug("%s: thread for connection with system socket is not started ... cannot continue", __FUNCTION__);
        goto errorout;

    }

    return;

    errorout:
    logoutput_debug("%s: stop eventloop", __FUNCTION__);
    BEVENTLOOP_stop(SIGTERM);

}

void OSNS_action_add(struct osns_ctx_s *octx, struct osns_ctx_action_s *action)
{
    struct osns_module_s *module=NULL;

    if (action==NULL) return;

    module=malloc(sizeof(struct osns_module_s));

    if (module==NULL) {

	logoutput_debug("%s: unable to allocate %u bytes for osns module", __FUNCTION__, sizeof(struct osns_module_s));
	return;

    }

    memset(module, 0, sizeof(struct osns_module_s));
    module->type=OSNS_MODULE_TYPE_INTERN;
    module->status=0;
    LIST_element_init(&module->list, NULL);
    module->action=action;
    MODULE_init(&module->module);

    LIST_header_add_last(&octx->actions, &module->list);
}

/* read modules from start profile */

struct osns_read_startprofile_hlpr_s {
    struct osns_ctx_s *octx;
};

static unsigned char osns_read_startprofile_cb(struct dstr_s *line, void *ptr)
{
    struct osns_read_startprofile_hlpr_s *hlpr=(struct osns_read_startprofile_hlpr_s *) ptr;
    struct dstr_s part=DSTR_INIT;
    struct dstr_s tmp=DSTR_INIT;
    struct dstr_s name=DSTR_INIT;
    struct osns_ctx_s *octx=hlpr->octx;
    struct osns_module_s *module=NULL;
    struct dstr_s domainp1=DSTR_INIT;
    struct dstr_s domainp2=DSTR_INIT;
    char *sep=NULL;

    /* possible may contain the delimiter (=newline or carriage return) */

    sep=memchr(line->str, 13, line->length);
    if (sep) {

	*sep=0;
	line->length=(unsigned int)(sep - line->str);

    }

    sep=memchr(line->str, 10, line->length);
    if (sep) {

	*sep=0;
	line->length=(unsigned int)(sep - line->str);

    }

    logoutput_debug("%s: found module %.*s", __FUNCTION__, line->length, line->str);
    DSTR_set_str(&name, line, 0);

    /* check the bane:
	ir has to end on .so and only one dot
	more checks to do like first name is a domain
    */

    if (DSTR_get_last_dstr(line, '.', &part, 1, 0)==0) {

	logoutput_debug("%s: skip %.*s: no dot found", __FUNCTION__, line->length, line->str);
	return 0;

    }

    if (DSTR_cmp_bytes(&part, "so", 0, 1, 0, 0)==0) {

	logoutput_debug("%s: skip %.*s: extension %.*s is not .so", __FUNCTION__, line->length, line->str, part.length, part.str);
	return 0;

    }

    DSTR_set_str(&tmp, line, 0);

    /* get the domain parts where the name begins with
	note: the name is now in tmp, excluding the extension */

    if (DSTR_get_first_dstr(&tmp, '_', &domainp1, 1, 0)==0) {

	logoutput_debug("%s: skip %.*s: no domain p1 found", __FUNCTION__, line->length, line->str);
	return 0;

    }

    if (DSTR_get_first_dstr(&tmp, '_', &domainp2, 1, 0)==0) {

	logoutput_debug("%s: skip %.*s: no domain p1 found", __FUNCTION__, line->length, line->str);
	return 0;

    }

    /* create module */

    module=malloc(sizeof(struct osns_module_s));

    if (module==NULL) {

	logoutput_debug("%s: unable to allocate %u bytes for osns module", __FUNCTION__, sizeof(struct osns_module_s));
	return 0;

    }

    memset(module, 0, sizeof(struct osns_module_s));
    module->type=OSNS_MODULE_TYPE_EXTERN;
    module->status=0;
    LIST_element_init(&module->list, NULL);
    module->action=NULL;
    MODULE_init(&module->module);

    if (OSNS_module_load(octx, &name, module)==0) {

	logoutput_debug("%s: unable to load %.*s", __FUNCTION__, name.length, name.str);
	free(module);
	return 0;

    }

    module->status|=OSNS_MODULE_STATUS_LOADED;

    logoutput_debug("%s: loaded module %.*s .. now trying to get symbol %.*s", __FUNCTION__, name.length, name.str, tmp.length, tmp.str);

    module->action=(struct osns_ctx_action_s *) OSNS_module_get_symbolptr(module, line);

    if (module->action==NULL) {

	logoutput_debug("%s: unable to get symbo %.*s", __FUNCTION__, line->length, line->str);
	free(module);
	return 0;

    }

    logoutput_debug("%s: found symbol %.*s in module %.*s", __FUNCTION__, tmp.length, tmp.str, line->length, line->str);

    /* add to action list */

    LIST_header_add_last(&octx->actions, &module->list);
    return 0; /* do not stop */

}

int OSNS_read_start(struct osns_ctx_s *octx)
{
    struct fs_path_s path=FS_PATH_INIT;
    int result=-1;
    struct osns_read_startprofile_hlpr_s hlpr;

    if ((octx->arguments->startprofile==NULL) || (strlen(octx->arguments->startprofile)==0)) {

	logoutput_debug("%s: empty start profile, cannot continue", __FUNCTION__);
	return 0;

    }

    FS_path_append_init(&path, FS_PATH_FLAG_BUFFER_ALLOC);

    /* construct a path like /etc/osns/start/*/

    if (FS_path_append(&path, 'p', (void *) &octx->options->etcpath.value, 0)==0) goto out;
    if (FS_path_append(&path, 'c', (void *) "start", 1)==0) goto out;
    if (FS_path_append(&path, 'c', (void *) octx->arguments->startprofile, 1)==0) goto out;

    hlpr.octx=octx;

    if (FILE_parse(&path, osns_read_startprofile_cb, (void *)&hlpr)) {

	result=1;

    } else {

	logoutput_debug("%s: unable to read start profile", __FUNCTION__);

    }

    out:

    FS_path_clear(&path);
    return result;

}
