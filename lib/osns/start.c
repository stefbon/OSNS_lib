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

	    if ((module->status & OSNS_MODULE_STATUS_LOADED)==0) goto nextprev;

	}

	/* the action has to be defined */

	if (action==NULL) goto nextprev;

    	if (actioncode==OSNS_CTX_ACTION_CODE_DO) {

    	    result=(* action->manage)(octx, actioncode, action, 0);

    	    if (result==-1) {

            	logoutput_debug("%s: action %s gives error .... cannot continue", __FUNCTION__);
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

		MODULE_unload(&module->module);
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
    struct osns_ctx_s *octx=hlpr->octx;
    struct osns_module_s *module=NULL;

    logoutput_debug("%s: found module %.*s", __FUNCTION__, line->length, line->str);

    DSTR_set_str(&tmp, line, 0);

    /* check the bane:
	ir has to end on .so and only one dot  */

    if ((DSTR_get_first_dstr(&tmp, '.', &part, 1, 0)==0) || (DSTR_cmp_bytes(&part, "so", 0, 1, 0, 0)==0)) {

	logoutput_debug("%s: skip %.*s: no dot found", __FUNCTION__, line->length, line->str);
	return 0;

    }

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

    if (OSNS_module_load(octx, line, module)==0) {

	logoutput_debug("%s: unable to load %.*s", __FUNCTION__, line->length, line->str);
	free(module);
	return 0;

    }

    logoutput_debug("%s: loaded module %.*s", __FUNCTION__, line->length, line->str);

    module->action=(struct osns_ctx_action_s *) MODULE_get_symbolptr(&module->module, "osns_ctx_action");

    if (module->action==NULL) {

	logoutput_debug("%s: unable to get symbol osns_ctx_action form %.*s", __FUNCTION__, line->length, line->str);
	free(module);
	return 0;

    }

    logoutput_debug("%s: found symbol osns_ctx_action in  module %.*s", __FUNCTION__, line->length, line->str);

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
