/*
 * The MIT License
 *
 * Copyright 2022 Ruslan Popov <ruslanpopov1512@gmail.com>.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef LISPY_COMMON_H
#define LISPY_COMMON_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

#define DIE_IF_NULL(what) do {							\
		if ((what) == NULL)								\
		{												\
			printf("internal error: out of memory\n");	\
			assert(false);								\
			exit(1);									\
		}												\
	} while (false)

#define MAX_ERR_LENGTH 1024
// From https://stackoverflow.com/questions/8257714/how-to-convert-an-int-to-string-in-c
#define MAX_INT_STR_LENGTH ((CHAR_BIT * sizeof(long) - 1) / 3 + 2)

#define PRELUDE_FILE_NAME "prelude.ls"

typedef struct lenv lenv;
typedef struct lval lval;

#endif // LISPY_COMMON_H
