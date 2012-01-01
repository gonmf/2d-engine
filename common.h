#define _POSIX_C_SOURCE 200112L
//200809L

#if (defined(__unix__)) || (defined(__hpux)) || (defined(__APPLE__)) && (defined(__MACH__)) || (defined(__sun)) || (defined(__SVR4)) || (defined(__CYGWIN__))
#define SEPARATOR '/'
#define _SEPARATOR '\\'
#endif
#if (defined(_WIN64)) || (defined(_WIN32))
#define SEPARATOR '\\'
#define _SEPARATOR '/'
#endif

#ifndef __game_pragma_once_
#define __game_pragma_once_

typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;
typedef char i8;
typedef short int i16;
typedef int i32;

// ANSI C
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// X
#include <X11/Xlib.h>

#include "clock.h"
#include "sprite.h"
#include "engine.h"

#endif