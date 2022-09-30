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

#ifndef LISPY_VALUE_H
#define LISPY_VALUE_H

#include <common.h>

#include <mpc/mpc.h>

typedef enum
{
    LVAL_ERR,
    LVAL_NUM,
    LVAL_SYM,
    LVAL_LIST,
    LVAL_QUOTE,
    LVAL_LAMBDA,
    LVAL_BUILTIN,
    LVAL_MACRO,
    LVAL_STR,
    LVAL_BOOL
} lval_type;

const char* lval_type_str(lval_type type);

typedef lval* (*lbuiltin_func)(lenv* env, lval* args);

struct lval
{
    lval_type type;

    union
    {
        long num;
        char* err;
        char* sym;
        char* str;
        lval* quoted;
        bool boolean;
        lbuiltin_func builtin;

        struct
        {
            unsigned count;
            struct lval** cells;
        };

        struct
        {
            lenv* env;
            lval* formals;
            lval* body;
        };
    };
};

#define IS_ERR(val)     (val->type == LVAL_ERR)
#define IS_NUM(val)     (val->type == LVAL_NUM)
#define IS_SYM(val)     (val->type == LVAL_SYM)
#define IS_STR(val)     (val->type == LVAL_STR)
#define IS_LIST(val)    (val->type == LVAL_LIST)
#define IS_LAMBDA(val)  (val->type == LVAL_LAMBDA)
#define IS_BUILTIN(val) (val->type == LVAL_BUILTIN)
#define IS_BOOL(val)    (val->type == LVAL_BOOL)
#define IS_QUOTE(val)   (val->type == LVAL_QUOTE)
#define IS_MACRO(val)   (val->type == LVAL_MACRO)

lval* lval_num(long x);
lval* lval_bool(bool x);
lval* lval_err(const char* fmt, ...);
lval* lval_verr(const char* fmt, va_list lst);
lval* lval_sym(const char* sym);
lval* lval_str(const char* str);
lval* lval_str_null();
lval* lval_str_char(const char ch);
lval* lval_list();
lval* lval_quote(lval* x);
lval* lval_lambda(lenv* env, lval* formals, lval* body);
lval* lval_macro(lval* formals, lval* body);
lval* lval_builtin(lbuiltin_func func);

lval* lval_copy(lval* a);
void  lval_del(lval* v);

lval* list_add(lval* v, lval* x);
lval* list_take(lval* v, unsigned i);
lval* list_pop(lval* v, unsigned i);
lval* list_join(lval* x, lval* y);
lval* list_to_str(lval* v);

lval* lval_unquote(lval* a);
bool  lval_eq(lval* a, lval* b);

lval* lval_to_str(lval* a);
void  lval_print(lval* v);
void  lval_println(lval* v);

#endif // LISPY_VALUE_H
