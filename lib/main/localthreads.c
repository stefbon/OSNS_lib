/*
  2010, 2011, 2012 Stef Bon <stefbon@gmail.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "libosns-basic-system-headers.h"

#include <sys/syscall.h>
#include <sys/wait.h>

#include "libosns-log.h"
#include "libosns-misc.h"
#include "libosns-list.h"
#include "libosns-error.h"
#include "libosns-event.h"

#include "localthreads.h"

#include <pthread.h>

static pthread_mutex_t mutex_threads=PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond_threads=PTHREAD_COND_INITIALIZER;
static struct event_shared_signal_s esignal_threads;
static unsigned char initdone=0;

struct local_thread_job_s {
    void 						                (* cb)(void *ptr);
    void 						                *ptr;
    struct list_element_s 				                list;
};

struct local_thread_s {
    pthread_t 						                threadid;
    struct list_element_s				                list;
    struct list_element_s				                wlist;
};

#define LOCAL_THREADS_MANAGER_STATUS_FINISH                             1

struct local_threads_manager_s {
    unsigned int                                                        status;
    struct list_header_s 				                threads;
    struct list_header_s				                joblist;
    unsigned int 					                maxnr;
    void                                                                (* put_job)(struct local_threads_manager_s *manager, int timeout, void (*cb)(void *ptr), void *ptr);
    void                                                                (* stop)(struct local_threads_manager_s *manager);
    void                                                                (* start)(struct local_threads_manager_s *manager);
};

struct local_threads_manager_s default_manager;

static void THREAD_module_init()
{
    struct event_shared_signal_s *esignal=EVENT_signal_get_default();

    logoutput_debug("%s", __FUNCTION__);

    if (EVENT_signal_lock(esignal)==0) {

	if (initdone==0) {

	    initdone=1;
	    EVENT_signal_set_custom(&esignal_threads, &mutex_threads, &cond_threads);

	}

	EVENT_signal_unlock(esignal);

    }

}

static struct local_threads_manager_s *local_thread_get_manager(struct local_thread_s *thread)
{
    struct list_header_s *h=thread->list.h;
    return (h) ? ((struct local_threads_manager_s *)((char *)h - offsetof(struct local_threads_manager_s, threads))) : NULL;
}

static struct local_thread_job_s *local_thread_get_job(struct local_threads_manager_s *manager)
{
    struct list_element_s *list=LIST_header_remove_first(&manager->joblist);
    return (list) ? ((struct local_thread_job_s *)((char *) list - offsetof(struct local_thread_job_s, list))) : NULL;
}

static void process_local_thread(void *ptr)
{
    struct local_thread_s *lthread=NULL;
    struct local_threads_manager_s *manager=NULL;
    struct list_element_s *list=NULL;
    sigset_t emptyset;

    if (ptr==NULL) return;
    lthread=(struct local_thread_s *) ptr;
    manager=local_thread_get_manager(lthread);
    if (manager==NULL) goto exitthread;
    lthread->threadid=pthread_self();

    sigemptyset(&emptyset);
    pthread_sigmask(SIG_BLOCK, &emptyset, NULL);

    /* thread can be cancelled any time */

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    checkandwait:

    EVENT_signal_lock(&esignal_threads);

    if (manager->status & LOCAL_THREADS_MANAGER_STATUS_FINISH) {

        EVENT_signal_unlock(&esignal_threads);
        goto exitthread;

    }

    /* wait till this thread has to do some work */

    list=LIST_header_remove_first(&manager->joblist);

    if (list) {
        struct local_thread_job_s *job=(struct local_thread_job_s *) ((char *) list - offsetof(struct local_thread_job_s, list));

        EVENT_signal_unlock(&esignal_threads);
        logoutput_debug("%s: process job tid %u", __FUNCTION__, gettid());
	(* job->cb)(job->ptr);
	free(job);
        goto checkandwait;

    }

    int tmp=EVENT_signal_wait(&esignal_threads, NULL);
    EVENT_signal_unlock(&esignal_threads);
    goto checkandwait;

    exitthread:

    LIST_element_remove(&lthread->list);
    free(lthread);
    pthread_exit(NULL);

}

static struct local_thread_s *local_thread_create(struct local_threads_manager_s *manager)
{
    pthread_attr_t attr;
    struct local_thread_s *lthread=malloc(sizeof(struct local_thread_s));
    unsigned int errcode=0;

    errcode=ENOMEM;
    if (lthread==NULL) goto errorout;

    memset(lthread, 0, sizeof(struct local_thread_s));
    lthread->threadid=0;
    LIST_element_init(&lthread->list, NULL);

    errcode=(unsigned int) pthread_attr_init(&attr);
    if (errcode) goto errorout;

    errcode=(unsigned int) pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (errcode) goto out_detachstate;

    LIST_header_add_last(&manager->threads, &lthread->list);
    errcode=(unsigned int) pthread_create(&lthread->threadid, &attr, (void *) process_local_thread, (void *) lthread);

    out_detachstate:

    pthread_attr_destroy(&attr);

    out:

    if (errcode==0) return lthread;

    errorout:

    if (lthread) free(lthread);
    logoutput_debug("%s: error %u (%s)", __FUNCTION__, errcode, strerror(errcode));
    return NULL;

}

static int local_thread_add(struct local_threads_manager_s *manager)
{
    struct local_thread_s *lthread=local_thread_create(manager);
    if (lthread) LIST_header_add_last(&manager->threads, &lthread->list);
    return (lthread ? 0 : -1);
}

static void local_threads_put_job(struct local_threads_manager_s *manager, int timeout, void (*cb)(void *ptr), void *ptr)
{
    struct local_thread_job_s *ljob=NULL;

    if (manager->status & LOCAL_THREADS_MANAGER_STATUS_FINISH) return;

    ljob=malloc(sizeof(struct local_thread_job_s));
    if (! ljob) return;

    ljob->cb=cb;
    ljob->ptr=ptr;
    LIST_element_init(&ljob->list, NULL);
    LIST_header_add_last(&manager->joblist, &ljob->list);
    EVENT_signal_broadcast_locked(&esignal_threads);

}

static void local_threads_stop(struct local_threads_manager_s *manager)
{

    EVENT_signal_lock(&esignal_threads);
    manager->status |= LOCAL_THREADS_MANAGER_STATUS_FINISH;
    EVENT_signal_broadcast(&esignal_threads);
    EVENT_signal_unlock(&esignal_threads);

}

void local_threads_start(struct local_threads_manager_s *manager)
{
    logoutput_debug("%s", __FUNCTION__);
    for (unsigned int i=0; i<manager->maxnr; i++) local_thread_add(manager);
}

static void local_threads_put_job_init(struct local_threads_manager_s *manager, int timeout, void (*cb)(void *ptr), void *ptr)
{}

static void local_threads_startstop_init(struct local_threads_manager_s *manager)
{}

struct local_threads_manager_s default_manager = {
    .status                                                     = 0,
//    .threads                                                    = LIST_HEADER_INIT,
//    .joblist                                                    = LIST_HEADER_INIT,
    .maxnr                                                      = 6,
    .put_job                                                    = local_threads_put_job_init,
    .stop                                                       = local_threads_startstop_init,
    .start                                                      = local_threads_startstop_init,
};

void LOCAL_threads_init()
{
    struct local_threads_manager_s *manager=&default_manager;

    THREAD_module_init();

    manager->status=0;
    LIST_header_init(&manager->threads, 0);
    LIST_header_init(&manager->joblist, 0);
    if (manager->maxnr==0) manager->maxnr=6;
    manager->put_job=local_threads_put_job;
    manager->stop=local_threads_stop;
    manager->start=local_threads_start;

    logoutput_debug("%s: out", __FUNCTION__);

}

void LOCAL_threads_set_maxnr(unsigned maxnr)
{
    struct local_threads_manager_s *manager=&default_manager;

    manager->maxnr=maxnr;
}

void LOCAL_threads_start()
{
    struct local_threads_manager_s *manager=&default_manager;

    logoutput_debug("%s", __FUNCTION__);

    (* manager->start)(manager);
}

void LOCAL_threads_stop()
{
    struct local_threads_manager_s *manager=&default_manager;

    (* manager->stop)(manager);
}

void LOCAL_threads_put_job(int timeout, void (*cb)(void *ptr), void *ptr)
{
    struct local_threads_manager_s *manager=&default_manager;

    (* manager->put_job)(manager, timeout, cb, ptr);
}
