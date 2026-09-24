#ifndef PANACKELTY_PLATFORM_H
#define PANACKELTY_PLATFORM_H
/* Select POSIX interfaces before any system header in host translation units. */
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#ifdef __APPLE__
#ifndef _DARWIN_C_SOURCE
#define _DARWIN_C_SOURCE
#endif
#endif
#endif
