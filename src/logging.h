/*
 * DESCRIPTION
 *
 */

#ifndef _LOGGING_H
#define _LOGGING_H

#include <stdio.h>


#if !DEBUG_LEVEL == 0
#define debug(fmt, ...)     ({ fprintf (stderr,\
                                        "Debug: %s -- " fmt "\n",\
                                        __func__, ##__VA_ARGS__); })
#else
#define debug(fmt, ...)     ({ snprintf (NULL, 0, fmt, ##__VA_ARGS__); })
#endif

#define error(fmt, ...)     ({ fprintf (stderr,\
                                        "Error: %s -- " fmt "\n",\
                                        __func__, ##__VA_ARGS__); })
#define fatal(fmt, ...)     ({ fprintf (stderr,\
                                        "Fatal: %s -- " fmt "\n",\
                                        __func__, ##__VA_ARGS__);\
                               exit (EXIT_FAILURE); })


#endif

