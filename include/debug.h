#pragma once
/**
 * @file debug.h
 */
#ifndef DEBUG_H
#define DEBUG_H

#ifdef DEBUG
#include <stdio.h>
#define debug_print(fmt, args ...) printf(fmt, ##args)
#else
#define debug_print(fmt, args ...)
#endif

#endif // DEBUG_H
