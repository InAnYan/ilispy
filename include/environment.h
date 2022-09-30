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

#ifndef LISPY_ENVIRONMENT_H
#define LISPY_ENVIRONMENT_H

#include <common.h>

typedef struct lenv_entry
{
	char* key;
	lval* value;
} lenv_entry;

struct lenv
{
	struct lenv* parent;
	lenv_entry* entries;
	unsigned count;
};

lenv* lenv_new(lenv* parent);
void  lenv_del(lenv* env);
lenv* lenv_copy(lenv* env);

lval* lenv_get(lenv* env, lval* key);
bool  lenv_set(lenv* env, lval* key, lval* value);

void  lenv_put(lenv* env, lval* key, lval* value);
void  lenv_def(lenv* env, lval* key, lval* value);

bool  lenv_put_new(lenv* env, lval* key, lval* value);
bool  lenv_def_new(lenv* env, lval* key, lval* value);

#endif // LISPY_ENVIRONMENT_H
