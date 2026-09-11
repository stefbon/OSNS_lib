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

    if (flags & OSNS_ARGUMENT_PROFILE) {

        long_options[index].name="profile";
        long_options[index].has_arg=required_argument;
        long_options[index].flag=NULL;
        long_options[index].val=OSNS_ARGINDEX_PROFILE;
        index++;

    }


    while (result==0) {
	int tmp = getopt_long(argc, argv, "", long_options, NULL);

	if (tmp==-1) {

            /* all arguments are parsed */
	    break;

        } else if (tmp==OSNS_ARGINDEX_HELP) {

            arguments->flags |= OSNS_ARGUMENT_HELP;

        } else if (tmp==OSNS_ARGINDEX_FORK) {

            arguments->flags |= OSNS_ARGUMENT_FORK;

        } else if (tmp==OSNS_ARGINDEX_PROFILE) {

            arguments->flags |= OSNS_ARGUMENT_PROFILE;
            arguments->startprofile=strdup(optarg);

        } else if (tmp=='?') {

	    logoutput_error("%s: error ... option %s not reckognized.", __FUNCTION__, argv[optind]);
	    result=-1;
	    break;

        }

    }

    return result;

}
