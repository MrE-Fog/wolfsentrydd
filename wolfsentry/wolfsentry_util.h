/*
 * wolfsentry_util.h
 *
 * Copyright (C) 2021-2023 wolfSSL Inc.
 *
 * This file is part of wolfSentry.
 *
 * wolfSentry is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * wolfSentry is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335, USA
 */

#ifndef WOLFSENTRY_UTIL_H
#define WOLFSENTRY_UTIL_H

#ifndef offsetof
/* gcc and clang define this in stddef.h to use sanitizer-safe builtins. */
#define offsetof(structure, element) ((uintptr_t)&(((structure *)0)->element))
#endif
#ifndef sizeof_field
#define sizeof_field(structure, element) sizeof(((structure *)0)->element)
#endif
#ifndef container_of
#define container_of(ptr, container_type, member_name) ((container_type *)(void *)(((byte *)(ptr)) - offsetof(container_type, member_name)))
#endif
#ifndef length_of_array
#define length_of_array(x) (sizeof (x) / sizeof (x)[0])
#endif
#ifndef end_ptr_of_array
#define end_ptr_of_array(x) (&(x)[length_of_array(x)])
#endif

#ifndef popcount32
#ifdef __GNUC__
#define popcount32(x) __builtin_popcount(x)
#else
#error Must supply binding for popcount32() on non-__GNUC__ targets.
#endif
#endif

#if defined(__GNUC__) && !defined(WOLFSENTRY_NO_BUILTIN_CLZ)
#ifndef LOG2_32
#define LOG2_32(x) (31 - __builtin_clz((unsigned int)(x)))
#endif
#ifndef LOG2_64
#define LOG2_64(x) ((sizeof(unsigned long long) * 8ULL) - (unsigned long long)__builtin_clzll((unsigned long long)(x)) - 1ULL)
#endif
#endif

#define streq(vs,fs,vs_len) (((vs_len) == strlen(fs)) && (memcmp(vs,fs,vs_len) == 0))
#define strcaseeq(vs,fs,vs_len) (((vs_len) == strlen(fs)) && (strncasecmp(vs,fs,vs_len) == 0))

#define WOLFSENTRY_BYTE_STREAM_DECLARE_STACK(buf, bufsiz) static const size_t buf ## siz = (bufsiz); unsigned char (buf)[bufsiz], *buf ## _p; size_t buf ## spc
#define WOLFSENTRY_BYTE_STREAM_DECLARE_HEAP(buf, bufsiz) static const size_t buf ## siz = (bufsiz); unsigned char *(buf), *buf ## _p; size_t buf ## spc
#define WOLFSENTRY_BYTE_STREAM_INIT_HEAP(buf) ((buf) = (unsigned char *)WOLFSENTRY_MALLOC(buf ## siz))
#define WOLFSENTRY_BYTE_STREAM_FREE_HEAP(buf) WOLFSENTRY_FREE(buf)
#define WOLFSENTRY_BYTE_STREAM_RESET(buf) do { (buf ## _p) = (buf); (buf ## spc) = (buf ## siz); } while (0)
#define WOLFSENTRY_BYTE_STREAM_LEN(buf) ((buf ## siz) - (buf ## spc))
#define WOLFSENTRY_BYTE_STREAM_HEAD(buf) (buf)
#define WOLFSENTRY_BYTE_STREAM_PTR(buf) (&(buf ## _p))
#define WOLFSENTRY_BYTE_STREAM_SPC(buf) (&(buf ## spc))

#define MAX_UINT_OF(x) ((((uint64_t)1 << ((sizeof(x) * (uint64_t)BITS_PER_BYTE) - (uint64_t)1)) - (uint64_t)1) | ((uint64_t)1 << ((sizeof(x) * (uint64_t)BITS_PER_BYTE) - (uint64_t)1)))
#define MAX_SINT_OF(x) ((int64_t)((((uint64_t)1 << ((sizeof(x) * (uint64_t)BITS_PER_BYTE) - (uint64_t)2)) - (uint64_t)1) | ((uint64_t)1 << ((sizeof(x) * (uint64_t)BITS_PER_BYTE) - (uint64_t)2))))

#define WOLFSENTRY_SET_BITS(enumint, bits) ((enumint) |= (bits))
#define WOLFSENTRY_CHECK_BITS(enumint, bits) (((enumint) & (bits)) == (bits))
#define WOLFSENTRY_CLEAR_BITS(enumint, bits) ((enumint) &= ~(uint32_t)(bits))
#define WOLFSENTRY_MASKIN_BITS(enumint, bits) ((enumint) & (bits))
#define WOLFSENTRY_MASKOUT_BITS(enumint, bits) ((enumint) & ~(uint32_t)(bits))
#define WOLFSENTRY_CLEAR_ALL_BITS(enumint) ((enumint) = 0)

#ifndef BITS_PER_BYTE
#define BITS_PER_BYTE 8
#endif

#define WOLFSENTRY_BITS_TO_BYTES(x) (((x) + 7U) >> 3U)

/* helpers for stringifying the expanded value of a macro argument rather than its literal text: */
#define _qq(x) #x
#define _q(x) _qq(x)

#ifdef WOLFSENTRY_THREADSAFE

#ifdef WOLFSENTRY_HAVE_GNU_ATOMICS

#define WOLFSENTRY_ATOMIC_INCREMENT(i, x) __atomic_add_fetch(&(i),x,__ATOMIC_SEQ_CST)
#define WOLFSENTRY_ATOMIC_INCREMENT_BY_ONE(i) WOLFSENTRY_ATOMIC_INCREMENT(i, 1)
#define WOLFSENTRY_ATOMIC_DECREMENT(i, x) __atomic_sub_fetch(&(i),x,__ATOMIC_SEQ_CST)
#define WOLFSENTRY_ATOMIC_DECREMENT_BY_ONE(i) WOLFSENTRY_ATOMIC_DECREMENT(i, 1)
#define WOLFSENTRY_ATOMIC_POSTINCREMENT(i, x) __atomic_fetch_add(&(i),x,__ATOMIC_SEQ_CST)
#define WOLFSENTRY_ATOMIC_POSTDECREMENT(i, x) __atomic_fetch_sub(&(i),x,__ATOMIC_SEQ_CST)
#define WOLFSENTRY_ATOMIC_STORE(i, x) __atomic_store_n(&(i), x, __ATOMIC_RELEASE)
#define WOLFSENTRY_ATOMIC_LOAD(i) __atomic_load_n(&(i), __ATOMIC_CONSUME)

/* caution, _TEST_AND_SET() alters arg2 (and returns false) on failure. */
#define WOLFSENTRY_ATOMIC_TEST_AND_SET(i, expected, intended)           \
    __atomic_compare_exchange_n(                                        \
        &(i),                                                           \
        &(expected),                                                    \
        intended,                                                       \
        0 /* weak */,                                                   \
        __ATOMIC_SEQ_CST /* success_memmodel */,                        \
        __ATOMIC_SEQ_CST /* failure_memmodel */);

#define WOLFSENTRY_ATOMIC_UPDATE_FLAGS(i, set_i, clear_i, pre_i, post_i)\
do {                                                                    \
    *(pre_i) = (i);                                                     \
    for (;;) {                                                          \
        *(post_i) = (*(pre_i) | (set_i)) & ~(clear_i);                  \
        if (*(post_i) == *(pre_i))                                      \
            break;                                                      \
        if (__atomic_compare_exchange_n(                                \
                &(i),                                                   \
                (pre_i),                                                \
                *(post_i),                                              \
                0 /* weak */,                                           \
                __ATOMIC_SEQ_CST /* success_memmodel */,                \
                __ATOMIC_SEQ_CST /* failure_memmodel */))               \
            break;                                                      \
    }                                                                   \
} while (0)

#define WOLFSENTRY_ATOMIC_RESET(i, pre_i)                               \
do {                                                                    \
    *(pre_i) = (i);                                                     \
    for (;;) {                                                          \
        if (*(pre_i) == 0)                                              \
            break;                                                      \
        if (__atomic_compare_exchange_n(                                \
                &(i),                                                   \
                (pre_i),                                                \
                0,                                                      \
                0 /* weak */,                                           \
                __ATOMIC_SEQ_CST /* success_memmodel */,                \
                __ATOMIC_SEQ_CST /* failure_memmodel */))               \
            break;                                                      \
    }                                                                   \
} while (0)

#define WOLFSENTRY_ATOMIC_INCREMENT_UNSIGNED_SAFELY(i, x, out)          \
do {                                                                    \
    __typeof__(i) _pre_i = (i);                                         \
    __typeof__(i) _post_i = _pre_i;                                     \
    for (;;) {                                                          \
        if (MAX_UINT_OF(i) - _pre_i < (x)) {                            \
            _post_i = 0;                                                \
            break;                                                      \
        }                                                               \
        _post_i = (__typeof__(i))(_pre_i + (x));                        \
        if (_post_i == _pre_i)                                          \
            break;                                                      \
        if (__atomic_compare_exchange_n(                                \
                &(i),                                                   \
                &_pre_i,                                                \
                _post_i,                                                \
                0 /* weak */,                                           \
                __ATOMIC_SEQ_CST /* success_memmodel */,                \
                __ATOMIC_SEQ_CST /* failure_memmodel */))               \
            break;                                                      \
    }                                                                   \
    (out) = _post_i;                                                    \
} while(0)

#define WOLFSENTRY_ATOMIC_INCREMENT_UNSIGNED_SAFELY_BY_ONE(i, out)      \
    WOLFSENTRY_ATOMIC_INCREMENT_UNSIGNED_SAFELY(i, 1U, out)

#define WOLFSENTRY_ATOMIC_DECREMENT_UNSIGNED_SAFELY(i, x, out)          \
do {                                                                    \
    __typeof__(i) _pre_i = (i);                                         \
    __typeof__(i) _post_i = _pre_i;                                     \
    for (;;) {                                                          \
        if (_pre_i < (x)) {                                             \
            _post_i = MAX_UINT_OF(i);                                   \
            break;                                                      \
        }                                                               \
        _post_i = (__typeof__(i))(_pre_i - (x));                        \
        if (_post_i == _pre_i)                                          \
            break;                                                      \
        if (__atomic_compare_exchange_n(                                \
                &(i),                                                   \
                &_pre_i,                                                \
                _post_i,                                                \
                0 /* weak */,                                           \
                __ATOMIC_SEQ_CST /* success_memmodel */,                \
                __ATOMIC_SEQ_CST /* failure_memmodel */))               \
            break;                                                      \
    }                                                                   \
    (out) = _post_i;                                                    \
} while(0)

#define WOLFSENTRY_ATOMIC_DECREMENT_UNSIGNED_SAFELY_BY_ONE(i, out)      \
    WOLFSENTRY_ATOMIC_DECREMENT_UNSIGNED_SAFELY(i, 1U, out)

#endif /* WOLFSENTRY_HAVE_GNU_ATOMICS */

#else /* !WOLFSENTRY_THREADSAFE */

#define WOLFSENTRY_ATOMIC_INCREMENT(i, x) ((i) += (x))
#define WOLFSENTRY_ATOMIC_INCREMENT_BY_ONE(i) (++(i))
#define WOLFSENTRY_ATOMIC_DECREMENT(i, x) ((i) -= (x))
#define WOLFSENTRY_ATOMIC_DECREMENT_BY_ONE(i) (--(i))
#define WOLFSENTRY_ATOMIC_STORE(i, x) ((i)=(x))
#define WOLFSENTRY_ATOMIC_LOAD(i) (i)

#define WOLFSENTRY_ATOMIC_UPDATE_FLAGS(i, set_i, clear_i, pre_i, post_i)\
do {                                                                    \
    *(pre_i) = (i);                                                     \
    *(post_i) = (*(pre_i) | (set_i)) & ~(clear_i);                      \
    if (*(post_i) != *(pre_i))                                          \
        (i) = *(post_i);                                                \
} while (0)

#define WOLFSENTRY_ATOMIC_RESET(i, pre_i) do { *(pre_i) = (i); (i) = 0; } while (0)

#define WOLFSENTRY_ATOMIC_INCREMENT_UNSIGNED_SAFELY(i, x, out)          \
    do {                                                                \
        if (((x) > MAX_UINT_OF(i)) || ((MAX_UINT_OF(i) - (i) < (x))))   \
            (out) = 0U;                                                 \
        else                                                            \
            (out) = (i) = (__typeof__(i))((i) + (x));                   \
    } while (0)

#define WOLFSENTRY_ATOMIC_INCREMENT_UNSIGNED_SAFELY_BY_ONE(i, out)      \
    WOLFSENTRY_ATOMIC_INCREMENT_UNSIGNED_SAFELY(i, 1U, out)

#define WOLFSENTRY_ATOMIC_DECREMENT_UNSIGNED_SAFELY(i, x, out)          \
    do {                                                                \
        if (((x) > MAX_UINT_OF(i)) || ((i) < (x)))                      \
            (out) = MAX_UINT_OF(i);                                     \
        else                                                            \
            (out) = (i) = (__typeof__(i))((i) - (x));                   \
    } while (0)

#define WOLFSENTRY_ATOMIC_DECREMENT_UNSIGNED_SAFELY_BY_ONE(i, out)      \
    WOLFSENTRY_ATOMIC_DECREMENT_UNSIGNED_SAFELY(i, 1U, out)

#endif /* WOLFSENTRY_THREADSAFE */

#ifdef WOLFSENTRY_CPPCHECK
    /* work around internalAstError */
    #undef WOLFSENTRY_ATOMIC_INCREMENT_UNSIGNED_SAFELY
    #define WOLFSENTRY_ATOMIC_INCREMENT_UNSIGNED_SAFELY(i, x, out) do { (out) = ((i) += (x)); } while (0)
    #undef WOLFSENTRY_ATOMIC_DECREMENT_UNSIGNED_SAFELY
    #define WOLFSENTRY_ATOMIC_DECREMENT_UNSIGNED_SAFELY(i, x, out) do { (out) = ((i) += (x)); } while (0)
#endif

#endif /* WOLFSENTRY_UTIL_H */
