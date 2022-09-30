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

#ifndef LISPY_BUILTINS_H
#define LISPY_BUILTINS_H

#include <common.h>

#include <value.h>
#include <environment.h>

lval* builtin_head(lenv* e, lval* a);
lval* builtin_tail(lenv* e, lval* a);
lval* builtin_list(lenv* e, lval* a);
lval* builtin_eval(lenv* e, lval* a);
lval* builtin_join(lenv* e, lval* a);

lval* builtin_headstr(lenv* e, lval* a);
lval* builtin_tailstr(lenv* e, lval* a);
lval* builtin_joinstr(lenv* e, lval* a);

lval* builtin_add(lenv* e, lval* a);
lval* builtin_sub(lenv* e, lval* a);
lval* builtin_div(lenv* e, lval* a);
lval* builtin_mul(lenv* e, lval* a);

lval* builtin_def(lenv* e, lval* a);
lval* builtin_let(lenv* e, lval* a);
lval* builtin_set(lenv* e, lval* a);

lval* builtin_lambda(lenv* e, lval* a);

lval* builtin_eq(lenv* e, lval* a);
lval* builtin_less(lenv* e, lval* a);
lval* builtin_not(lenv* e, lval* a);

lval* builtin_cond(lenv* e, lval* a);

lval* builtin_show(lenv* e, lval* a);
lval* builtin_print(lenv* e, lval* a);
lval* builtin_println(lenv* e, lval* a);
lval* builtin_error(lenv* e, lval* a);

lval* builtin_load_impl(lenv* e, lval* a, bool isMain);
lval* builtin_load(lenv* e, lval* a);
lval* builtin_exit(lenv* e, lval* a);

lval* builtin_typeq(lenv* e, lval* a);

lval* builtin_macro_internal(lenv* e, lval* a);
lval* builtin_macroexpand(lenv* e, lval* a);

void add_builtin(lenv* env, const char* name, lbuiltin_func func);
void add_builtins(lenv* e);

#endif // LISPY_BUILTINS_H
