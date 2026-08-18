/* SPDX-License-Identifier: GLP-2.0-only */

#include "libosns-basic-system-headers.h"
#include "libosns-defaults.h"

#include <getopt.h>

#include "libosns-main.h"
#include "libosns-datatypes.h"
#include "osns/osns.h"
#include "arguments.h"

int OSNS_parse_arguments(int argc, char *argv[], struct osns_arguments_s *arguments, unsigned int flags)
{
    static struct option long_options[] = {
	{"help", 		no_argument, 		        0, OSNS_ARGINDEX_HELP},
	{NULL,0,0,0},
	{NULL,0,0,0},
	{NULL,0,0,0},
	{NULL,0,0,0},
	{NULL,0,0,0},
    };
    int result=0;
    unsigned int index=1;

    if (flags & OSNS_ARGUMENT_FORK) {

        long_options[index].name="fork";
        long_options[index].has_arg=no_argument;
        long_options[index].flag=NULL;
        long_options[index].val=OSNS_ARGINDEX_FORK;
        index++;

    }

    if (flags & OSNS_ARGUMENT_SOCKETACTIVATION) {

        long_options[index].name="socketactivation";
        long_options[index].has_arg=no_argument;
        long_options[index].flag=NULL;
        long_options[index].val=OSNS_ARGINDEX_SOCKETACTIVATION;
        index++;

    }

    if (flags & OSNS_ARGUMENT_PIDFILE) {

        long_options[index].name="pidfile";
        long_options[index].has_arg=no_argument;
        long_options[index].flag=NULL;
        long_options[index].val=OSNS_ARGINDEX_PIDFILE;
        index++;

    }

    if (flags & OSNS_ARGUMENT_PPID) {

        long_options[index].name="ppid";
        long_options[index].has_arg=required_argument;
        long_options[index].flag=NULL;
        long_options[index].val=OSNS_ARGINDEX_PPID;
        index++;

    }


    while (result==0) {
	int tmp = getopt_long(argc, argv, "", long_options, NULL);

        logoutput_debug("%s: result getopt %i", __FUNCTION__, tmp);

	if (tmp==-1) {

            /* all arguments are parsed */
	    break;

        } else if (tmp==OSNS_ARGINDEX_HELP) {

            arguments->flags |= OSNS_ARGUMENT_HELP;

        } else if (tmp==OSNS_ARGINDEX_FORK) {

            arguments->flags |= OSNS_ARGUMENT_FORK;

        } else if (tmp==OSNS_ARGINDEX_SOCKETACTIVATION) {

            arguments->flags |= OSNS_ARGUMENT_SOCKETACTIVATION;

        } else if (tmp==OSNS_ARGINDEX_PIDFILE) {

            arguments->flags |= OSNS_ARGUMENT_PIDFILE;

        } else if (tmp==OSNS_ARGINDEX_PPID) {

            arguments->flags |= OSNS_ARGUMENT_PPID;
            arguments->pid=atoi(optarg);

        } else if (tmp=='?') {

	    logoutput_error("%s: error ... option %s not reckognized.", __FUNCTION__, argv[optind]);
	    result=-1;
	    break;

        }

    }

    return result;

}
