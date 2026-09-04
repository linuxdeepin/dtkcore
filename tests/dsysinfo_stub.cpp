// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

// LD_PRELOAD stub that redirects the specific DSysInfo fixture files
// (/etc/os-version, /etc/lsb-release, /etc/os-release, /etc/deepin-version,
// /usr/lib/os-release) to /tmp/etc/ and /tmp/usr/lib/ so that DSysInfo reads
// mock fixture files instead of real system files.
//
// The redirect is intentionally NARROW (exact-path only), NOT all of /etc/:
// a broad /etc/ redirect breaks unrelated accesses the tests rely on —
//   * getpwuid()'s /etc/passwd read -> redirected to a non-existent
//     /tmp/etc/passwd -> null passwd entry -> SEGFAULT in the empty-HOME
//     DLogManager tests;
//   * QSaveFile's O_TMPFILE-backed writes (mode arg dropped) -> EACCES in the
//     DCI FileEngine tests.
//
// This stub is compiled as a shared library and loaded via LD_PRELOAD
// (configured in gtest_discover_tests PROPERTIES ENVIRONMENT).
// It complements the OBJECT-library approach: the OBJECT copy of
// dsysinfo.cpp already reads from /tmp/etc/ via DSYSINFO_PREFIX="/tmp",
// while this stub catches any /etc/ reads originating from the shared
// library's internal code paths.

#define _GNU_SOURCE
#include <dlfcn.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

// Exact-path redirects only. Returns either `path` or a pointer to a string
// literal (immutable, reentrant, thread-safe) — never a shared mutable buffer.
static const char *redirect_path(const char *path)
{
    if (!path)
        return path;

    if (strcmp(path, "/etc/os-version") == 0)
        return "/tmp/etc/os-version";
    if (strcmp(path, "/etc/lsb-release") == 0)
        return "/tmp/etc/lsb-release";
    if (strcmp(path, "/etc/os-release") == 0)
        return "/tmp/etc/os-release";
    if (strcmp(path, "/etc/deepin-version") == 0)
        return "/tmp/etc/deepin-version";
    if (strcmp(path, "/usr/lib/os-release") == 0)
        return "/tmp/usr/lib/os-release";

    return path;
}

typedef int (*orig_open_t)(const char *, int, ...);
typedef int (*orig_open64_t)(const char *, int, ...);
typedef int (*orig_access_t)(const char *, int);
typedef FILE *(*orig_fopen_t)(const char *, const char *);
typedef FILE *(*orig_fopen64_t)(const char *, const char *);
typedef int (*orig_stat_t)(const char *, struct stat *);
typedef int (*orig_lstat_t)(const char *, struct stat *);
typedef int (*orig_xstat_t)(int, const char *, struct stat *);

extern "C" int open(const char *path, int flags, ...)
{
    mode_t mode = 0;
    // mode is significant for O_CREAT AND O_TMPFILE (O_TMPFILE does not set
    // O_CREAT; dropping it made QSaveFile create 0000-mode files -> EACCES).
    if (flags & (O_CREAT | O_TMPFILE)) {
        va_list args;
        va_start(args, flags);
        mode = va_arg(args, mode_t);
        va_end(args);
    }
    orig_open_t orig = (orig_open_t)dlsym(RTLD_NEXT, "open");
    return orig(redirect_path(path), flags, mode);
}

extern "C" int open64(const char *path, int flags, ...)
{
    mode_t mode = 0;
    if (flags & (O_CREAT | O_TMPFILE)) {
        va_list args;
        va_start(args, flags);
        mode = va_arg(args, mode_t);
        va_end(args);
    }
    orig_open64_t orig = (orig_open64_t)dlsym(RTLD_NEXT, "open64");
    return orig(redirect_path(path), flags, mode);
}

extern "C" int access(const char *path, int mode)
{
    orig_access_t orig = (orig_access_t)dlsym(RTLD_NEXT, "access");
    return orig(redirect_path(path), mode);
}

extern "C" FILE *fopen(const char *path, const char *mode)
{
    orig_fopen_t orig = (orig_fopen_t)dlsym(RTLD_NEXT, "fopen");
    return orig(redirect_path(path), mode);
}

extern "C" FILE *fopen64(const char *path, const char *mode)
{
    orig_fopen64_t orig = (orig_fopen64_t)dlsym(RTLD_NEXT, "fopen64");
    return orig(redirect_path(path), mode);
}

extern "C" int stat(const char *path, struct stat *buf)
{
    orig_stat_t orig = (orig_stat_t)dlsym(RTLD_NEXT, "stat");
    return orig(redirect_path(path), buf);
}

extern "C" int lstat(const char *path, struct stat *buf)
{
    orig_lstat_t orig = (orig_lstat_t)dlsym(RTLD_NEXT, "lstat");
    return orig(redirect_path(path), buf);
}

extern "C" int __xstat(int ver, const char *path, struct stat *buf)
{
    orig_xstat_t orig = (orig_xstat_t)dlsym(RTLD_NEXT, "__xstat");
    return orig(ver, redirect_path(path), buf);
}

extern "C" int __lxstat(int ver, const char *path, struct stat *buf)
{
    orig_xstat_t orig = (orig_xstat_t)dlsym(RTLD_NEXT, "__lxstat");
    return orig(ver, redirect_path(path), buf);
}
