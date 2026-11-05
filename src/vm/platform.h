#ifndef PANACKELTY_PLATFORM_H
#define PANACKELTY_PLATFORM_H
/* Select POSIX interfaces before any system header in host translation units. */
#ifdef __linux__
/* Atomic directory exchange uses the Linux syscall interface. Keep feature
 * selection here so force-included test headers see the same declarations. */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#ifdef __APPLE__
#ifndef _DARWIN_C_SOURCE
#define _DARWIN_C_SOURCE
#endif
#endif
#endif
