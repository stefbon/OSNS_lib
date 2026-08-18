/* SPDX-License-Identifier: GLP-2.0-only */

#include "libosns-basic-system-headers.h"
#include "libosns-defaults.h"

#include "libosns-log.h"
#include "libosns-threads.h"
#include "libosns-network.h"
#include "libosns-misc.h"
#include "libosns-list.h"
#include "libosns-datatypes.h"
#include "libosns-event.h"
#include "libosns-io.h"
#include "libosns-fs.h"

#include "osns/osns.h"

static const char *osns_action_unknown_name="unknown";

static int osns_process_actions_list(struct osns_ctx_s *octx, struct list_element_s **p_list, unsigned char direction, unsigned char actioncode)
{
    int result=0;
    struct list_element_s *list=*p_list;

    while (list) {
        struct osns_ctx_action_s *action=(struct osns_ctx_action_s *)((char *)list - offsetof(struct osns_ctx_action_s, list));

        if (actioncode==OSNS_CTX_ACTION_CODE_DO) {

            result=(* action->manage)(octx, actioncode, action);

            if (result==-1) {

                logoutput_debug("%s: action %s gives error .... cannot continue", __FUNCTION__);
                action->status |= OSNS_CTX_ACTION_STATUS_ERROR;
                break;

            }

            action->status |= OSNS_CTX_ACTION_STATUS_DONE;

        } else if (actioncode==OSNS_CTX_ACTION_CODE_UNDO) {

            /* only undo if done earlier and no error */

            if (action->status & OSNS_CTX_ACTION_STATUS_DONE) {

                result=(* action->manage)(octx, actioncode, action);
                action->status |= OSNS_CTX_ACTION_STATUS_UNDONE;

            }

        }

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

static int osns_action_manage_cb_noop(struct osns_ctx_s *octx, unsigned char actioncode, struct osns_ctx_action_s *action)
{
    return 0;
}

void OSNS_action_init(struct osns_ctx_action_s *action)
{
    memset(action, 0, sizeof(struct osns_ctx_action_s));
    LIST_element_init(&action->list, NULL);
    action->name=osns_action_unknown_name;
    action->manage=osns_action_manage_cb_noop;
}

void OSNS_action_add(struct osns_ctx_s *octx, struct osns_ctx_action_s *action)
{
    LIST_header_add_last(&octx->actions, &action->list);
}
