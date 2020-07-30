/* Copyright (C) 2019-2020 Trevor Last
 * See LICENSE file for copyright and license details.
 *
 *  Debugging/logging macros
 */

#ifndef _LOGGING_H
#define _LOGGING_H

#include <stdio.h>


extern FILE *G_logfile;

#if DEBUG_LEVEL != 0
#define debug(fmt, ...)                 \
    ({                                  \
        fprintf(                        \
            G_logfile,                  \
            "Debug: %s -- " fmt "\n",   \
            __func__,                   \
            ##__VA_ARGS__);             \
    })
#else
#define debug(fmt, ...)                             \
    ({                                              \
        snprintf(nullptr, 0, fmt, ##__VA_ARGS__);   \
    })
#endif

#define warn(fmt, ...)                  \
    ({                                  \
        fprintf(                        \
            G_logfile,                  \
            "Warning: %s -- " fmt "\n", \
            __func__,                   \
            ##__VA_ARGS__);             \
    })

#define error(fmt, ...)                 \
    ({                                  \
        fprintf(                        \
            G_logfile,                  \
            "Error: %s -- " fmt "\n",   \
            __func__,                   \
            ##__VA_ARGS__);             \
    })

#define fatal(fmt, ...)                 \
    ({                                  \
        fprintf(                        \
            G_logfile,                  \
            "Fatal: %s -- " fmt "\n",   \
            __func__,                   \
            ##__VA_ARGS__);             \
        exit(EXIT_FAILURE);             \
    })


#endif

