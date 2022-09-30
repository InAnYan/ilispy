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

#include <builtins.h>

#include <value.h>
#include <environment.h>
#include <eval.h>
#include <parser.h>

#include "reader.h"

#define LASSERT(args, cond, fmt, ...)    \
	do {           \
		if (!(cond))        \
		{           \
			lval_del(args);       \
			return lval_err(fmt, ##__VA_ARGS__); \
		}           \
	} while (false)

#define LASSERT_COUNT(val, must, name)        \
	do {                \
		if (val->count < must)           \
		{                \
			LASSERT(val, false,           \
					"function '" name "' passed not enough arguments"); \
		}                \
		else if (val->count > must)          \
		{                \
			LASSERT(val, false,           \
					"function '" name "' passed too much arguments"); \
		}                \
	} while (false)

#define LASSERT_ELEMENT_TYPE__(args, checkargs, i, exptype, funcname, elementname)  \
	do                 \
	{                 \
		lval_type got = checkargs[i]->type;        \
		LASSERT(args, checkargs[i]->type == exptype,     \
				"function '" funcname "' " elementname "passed incorrect type for argument %i. " \
				"Got %s, expected %s", i + 1,       \
				lval_type_str(got), lval_type_str(exptype));   \
	}                 \
	while (false)

#define LASSERT_ELEMENT_TYPE(args, checkargs, i, exptype, funcname, elementname) \
	LASSERT_ELEMENT_TYPE__(args, checkargs, i, exptype, funcname, elementname " ")

#define LASSERT_TYPE(args, i, exptype, funcname) \
	LASSERT_ELEMENT_TYPE__(args, args->cells, i, exptype, funcname, "")

#define LASSERT_TYPE2(args, i, exptype1, exptype2, funcname) \
	do                 \
	{                 \
		lval_type got = args->cells[i]->type;        \
		LASSERT(args, args->cells[i]->type == exptype1 || args->cells[i]->type == exptype2,     \
				"function '" funcname "' passed incorrect type for argument %i. " \
				"Got %s, expected %s or %s", i + 1,       \
				lval_type_str(got), lval_type_str(exptype1), lval_type_str(exptype2));   \
	}                 \
	while (false)

void add_builtins(lenv* e)
{
	add_builtin(e, "list", builtin_list);
	add_builtin(e, "head", builtin_head);
	add_builtin(e, "tail", builtin_tail);
	add_builtin(e, "eval", builtin_eval);
	add_builtin(e, "join", builtin_join);
	
	add_builtin(e, "headstr", builtin_headstr);
	add_builtin(e, "tailstr", builtin_tailstr);
	add_builtin(e, "joinstr", builtin_joinstr);

	add_builtin(e, "+", builtin_add);
	add_builtin(e, "-", builtin_sub);
	add_builtin(e, "*", builtin_mul);
	add_builtin(e, "/", builtin_div);

	add_builtin(e, "__def", builtin_def);
	add_builtin(e, "__set", builtin_set);
	add_builtin(e, "__let", builtin_let);

	add_builtin(e, "eq", builtin_eq);
	add_builtin(e, "less", builtin_less);
	add_builtin(e, "not", builtin_not);

	add_builtin(e, "cond", builtin_cond);

	add_builtin(e, "\\", builtin_lambda);

	add_builtin(e, "show", builtin_show);
	add_builtin(e, "print", builtin_print);
	add_builtin(e, "println", builtin_println);
	add_builtin(e, "error", builtin_error);

	add_builtin(e, "load", builtin_load);
	add_builtin(e, "exit", builtin_exit);

	add_builtin(e, "typeq", builtin_typeq);
	add_builtin(e, "__macro", builtin_macro_internal);
	add_builtin(e, "macroexpand", builtin_macroexpand);
}

lval* builtin_op(lval* a, const char* op)
{
	for (unsigned i = 0; i < a->count; i++)
		LASSERT_TYPE(a, i, LVAL_NUM, "builtin arithmetic");

	lval* x = list_pop(a, 0);

	if ((strcmp(op, "-") == 0) && a->count == 0)
		x->num = -(x->num);

	while (a->count > 0)
	{
		lval* y = list_pop(a, 0);

		if (strcmp(op, "+") == 0) x->num += y->num;
		if (strcmp(op, "-") == 0) x->num -= y->num;
		if (strcmp(op, "*") == 0) x->num *= y->num;
		if (strcmp(op, "/") == 0)
		{
			if (y->num == 0)
			{
				lval_del(x);
				lval_del(y);
				x = lval_err("division by zero");
				break;
			}

			x->num /= y->num;
		}

		lval_del(y);
	}

	lval_del(a);
	return x;
}

lval* builtin_add(lenv* e, lval* a)
{
	return builtin_op(a, "+");
}

lval* builtin_sub(lenv* e, lval* a)
{
	return builtin_op(a, "-");
}

lval* builtin_div(lenv* e, lval* a)
{
	return builtin_op(a, "/");
}

lval* builtin_mul(lenv* e, lval* a)
{
	return builtin_op(a, "*");
}

lval* builtin_head(lenv* e, lval* a)
{
	LASSERT_COUNT(a, 1, "head");
	LASSERT_TYPE(a, 0, LVAL_LIST, "head");
	
	LASSERT(a, a->cells[0]->count != 0, "function 'head' passed ()");

	lval* lst = list_take(a, 0);
	
	while (lst->count > 1)
	{
		lval_del(list_pop(lst, 1));
	}
	
	return list_take(lst, 0);
}

lval* builtin_tail(lenv* e, lval* a)
{
	LASSERT_COUNT(a, 1, "tail");

	LASSERT_TYPE(a, 0, LVAL_LIST, "tail");
	LASSERT(a, a->cells[0]->count != 0,
			"function 'tail' passed {}");

	lval* v = list_take(a, 0);
	lval_del(list_pop(v, 0));
	return v;
}

lval* builtin_list(lenv* e, lval* a)
{
	return a;
}

lval* builtin_eval(lenv* e, lval* a)
{
	LASSERT_COUNT(a, 1, "eval");

	return eval_lval(e, list_take(a, 0));
}

lval* builtin_join(lenv* e, lval* a)
{
	for (unsigned i = 0; i < a->count; i++)
		LASSERT_TYPE(a, i, LVAL_LIST, "join");

	lval* x = list_pop(a, 0);

	while (a->count)
	{
		x = list_join(x, list_pop(a, 0));
	}

	lval_del(a);
	return x;
}

void add_builtin(lenv* env, const char* name, lbuiltin_func func)
{
	lval* key = lval_sym(name);
	lval* value = lval_builtin(func);
	lenv_put(env, key, value);
	lval_del(key);
	lval_del(value);
}

lval* builtin_def(lenv* env, lval* a)
{
	LASSERT_COUNT(a, 2, "def");
	LASSERT_TYPE(a, 0, LVAL_SYM, "def");

	lenv_def(env, a->cells[0], a->cells[1]);

	return list_take(a, 1);
}

lval* builtin_let(lenv* env, lval* a)
{
	LASSERT_COUNT(a, 2, "let");
	LASSERT_TYPE(a, 0, LVAL_SYM, "let");

	lenv_put(env, a->cells[0], a->cells[1]);

	return list_take(a, 1);
}

lval* builtin_set(lenv* e, lval* a)
{
	LASSERT_COUNT(a, 2, "set");
	LASSERT_TYPE(a, 0, LVAL_SYM, "set");

	if (!lenv_set(e, a->cells[0], a->cells[1]))
	{
		lval* res = lval_err("symbol '%s' is not bound to anything",
							a->cells[0]->sym);
		lval_del(a);
		return res;
	}

	return list_take(a, 1);
}

lval* builtin_lambda(lenv* e, lval* a)
{
	LASSERT_COUNT(a, 2, "\\");

	LASSERT_TYPE(a, 0, LVAL_LIST, "\\");
	LASSERT_TYPE(a, 1, LVAL_LIST, "\\");

	for (unsigned i = 0; i < a->cells[0]->count; i++)
		LASSERT_ELEMENT_TYPE(a, a->cells[0]->cells, i, LVAL_SYM, "\\", "parameters");

	lval* formals = list_pop(a, 0);

	// checking for proper use of '&'
	for (unsigned i = 0; i < formals->count; i++)
	{
		if (strcmp(formals->cells[i]->sym, "&") == 0)
		{
			if (!(formals->count == 3 && i == 1))
			{
				lval_del(formals);
				lval_del(a);
				return lval_err("inappropriate usage of '&'");
			}
		}
	}

	lval* body = list_pop(a, 0);
	lval_del(a);

	return lval_lambda(lenv_new(NULL), formals, body);
}

lval* builtin_exit(lenv* e, lval* a)
{
	if (a->count > 1)
	{
		printf("runtime error: function 'exit' passed too many arguments");
		exit(1);
	}
	else if (a->count == 1)
	{
		if (!IS_NUM(a->cells[0]))
		{
			printf("runtime error: function 'exit' passed non-number argument");
			exit(1);
		}
		else
		{
			exit(a->cells[0]->num);
		}
	}
	else
	{
		exit(0);
	}
}

lval* builtin_eq(lenv* e, lval* a)
{
	LASSERT_COUNT(a, 2, "eq");

	lval* res = lval_bool(lval_eq(a->cells[0], a->cells[1]));

	lval_del(a);
	return res;
}

lval* builtin_less(lenv* e, lval* a)
{
	LASSERT_COUNT(a, 2, "less");
	LASSERT_TYPE(a, 0, LVAL_NUM, "less");
	LASSERT_TYPE(a, 1, LVAL_NUM, "less");

	lval* res = lval_bool((a->cells[0]->num < a->cells[1]->num));
	lval_del(a);
	return res;
}

lval* builtin_not(lenv* e, lval* a)
{
	LASSERT_COUNT(a, 1, "not");
	LASSERT_TYPE(a, 0, LVAL_BOOL, "not");

	lval* res = lval_bool(!a->cells[0]->boolean);
	;
	lval_del(a);
	return res;
}

lval* builtin_cond(lenv* e, lval* a)
{
	for (unsigned i = 0; i < a->count; i++)
		LASSERT_TYPE(a, i, LVAL_LIST, "cond");
	
	lval* res = lval_list();
	
	unsigned i = 0;
	while (a->count)
	{
		lval* lst = list_pop(a, 0);
		if (lst->count == 0)
		{
			lval_del(res);
			lval_del(lst);
			LASSERT(a, false, "function 'cond' argument %i is nil", i);
		}
		
		lval* test = eval_lval(e, list_pop(lst, 0));
		if (!IS_BOOL(test))
		{
			lval_del(lst);
			lval_del(res);
			if (IS_ERR(test))
			{
				lval_del(a);
				return test;
			}
			else
			{
				LASSERT(a, false, "function 'cond' test result for argument %i is not a Boolean", i);
			}
		}
		
		bool cond = test->boolean;
		lval_del(test);
		
		if (cond)
		{
			while (lst->count)
			{
				lval_del(res);
				res = eval_lval(e, list_pop(lst, 0));
			}
			
			lval_del(lst);
			lval_del(a);
			return res;
		}
		
		lval_del(lst);
		
		i++;
	}
	
	return res;
}

lval* builtin_if(lenv* e, lval* a)
{
	LASSERT_COUNT(a, 3, "if");
	LASSERT_TYPE(a, 0, LVAL_BOOL, "if");

	lval* res = NULL;

	// a->cells[1]->type = LVAL_LIST;
	// a->cells[2]->type = LVAL_LIST;

	if (a->cells[0]->boolean)
		res = eval_lval(e, list_pop(a, 1));
	else
		res = eval_lval(e, list_pop(a, 2));

	lval_del(a);
	return res;
}

lval* builtin_load(lenv* e, lval* a)
{
	return builtin_load_impl(e, a, false);
}

lval* builtin_load_impl(lenv* e, lval* a, bool isMain)
{
	LASSERT_COUNT(a, 1, "load");
	LASSERT_TYPE(a, 0, LVAL_STR, "load");

	mpc_result_t r;
	if (mpc_parse_contents(a->cells[0]->str,
						(mpc_parser_t*) get_parser_lispy(), &r))
	{
		lval* expr = read_lval(r.output);
		mpc_ast_delete(r.output);

		if (IS_ERR(expr))
		{
			lval_del(a);
			return expr;
		}

		lval nameKey; nameKey.type = LVAL_SYM; nameKey.sym = "__name__";
		lval* oldName = lenv_get(e, &nameKey);

		{
			lval nameValue; nameValue.type = LVAL_SYM; nameValue.sym = isMain ? "__main__" : "__load__";
			lenv_put(e, &nameKey, &nameValue);
		}
		
		while (expr->count)
		{
			lval* x = eval_lval(e, list_pop(expr, 0));
			if (IS_ERR(x))
			{
				lval_del(expr);
				lval_del(a);
				lenv_put(e, &nameKey, oldName);
				lval_del(oldName);
				return x;
			}
			lval_del(x);
		}

		lval_del(expr);
		lval_del(a);

		lenv_put(e, &nameKey, oldName);
		lval_del(oldName);

		return lval_list();
	}
	else
	{
		char* errMsg = mpc_err_string(r.error);
		mpc_err_delete(r.error);
		errMsg[strlen(errMsg) - 1] = '\0';
		lval* err = lval_err("could not load file '%s'", errMsg);
		free(errMsg);
		lval_del(a);

		return err;
	}
}

lval* builtin_print(lenv* e, lval* a)
{
	for (unsigned i = 0; i < a->count; i++)
		lval_print(a->cells[i]);

	lval_del(a);
	return lval_list();
}

lval* builtin_println(lenv* e, lval* a)
{
	for (unsigned i = 0; i < a->count; i++)
		lval_print(a->cells[i]);

	printf("\n");
	lval_del(a);
	return lval_list();
}

lval* builtin_error(lenv* e, lval* a)
{
	LASSERT_COUNT(a, 1, "error");
	LASSERT_TYPE(a, 0, LVAL_STR, "error");

	lval* res = lval_err(a->cells[0]->str);
	lval_del(a);
	return res;
}

lval* builtin_typeq(lenv* e, lval* a)
{
	LASSERT_COUNT(a, 2, "typeq");

	lval* res = lval_bool(a->cells[0]->type == a->cells[1]->type);
	lval_del(a);
	return res;
}

lval* builtin_macro_internal(lenv* e, lval* a)
{
	LASSERT_COUNT(a, 2, "__macro");
	LASSERT_TYPE(a, 0, LVAL_LIST, "__macro");
	LASSERT_TYPE(a, 1, LVAL_LIST, "__macro");

	for (unsigned i = 0; i < a->cells[0]->count; i++)
		LASSERT_ELEMENT_TYPE(a, a->cells[0]->cells, i, LVAL_SYM, "__macro", "parameters");

	lval* formals = list_pop(a, 0);
	lval* body = list_pop(a, 0);
	lval_del(a);

	return lval_macro(formals, body);
}

lval* builtin_macroexpand(lenv* e, lval* a)
{
	LASSERT_COUNT(a, 1, "macroexpand");
	LASSERT_TYPE(a, 0, LVAL_LIST, "macroexpand");

	lval* expr = list_pop(a, 0);
	lval_del(a);

	lval* sym = list_pop(expr, 0);
	if (!IS_SYM(sym))
	{
		lval_del(sym);
		lval_del(expr);
		return lval_err("function 'macroexpand' passed non-macro");
	}

	lval* macro = lenv_get(e, sym);
	lval_del(sym);
	if (!IS_MACRO(macro))
	{
		lval_del(macro);
		lval_del(expr);
		return lval_err("function 'macroexpand' passed non-macro");
	}

	lval* expanded = eval_macro_expand(macro, expr);
	lval_del(macro);
	lval_del(expr);

	return builtin_println(e, list_add(lval_list(), expanded));
}

lval* builtin_headstr(lenv* e, lval* a)
{
	LASSERT_COUNT(a, 1, "headstr");
	LASSERT_TYPE(a, 0, LVAL_STR, "headstr");
	
	LASSERT(a, a->cells[0]->str[0] != '\0', "function 'headstr' passed empty string");
	
	lval* v = list_take(a, 0);
	
	char r = v->str[0];
	lval_del(v);
	
	return lval_str_char(r);
}

lval* builtin_tailstr(lenv* e, lval* a)
{
	LASSERT_COUNT(a, 1, "tailstr");
	LASSERT_TYPE(a, 0, LVAL_STR, "tailstr");
	
	LASSERT(a, a->cells[0]->str[0] != '\0', "function 'tailstr' passed empty string");
	
	lval* str = list_take(a, 0);
	lval* res = lval_str_null();
	
	res->str = malloc(strlen(str->str));
	DIE_IF_NULL(res->str);
	
	strcpy(res->str, str->str + 1);
	
	lval_del(str);
	return res;
}

lval* builtin_joinstr(lenv* e, lval* a)
{
	for (unsigned i = 0; i < a->count; i++)
		LASSERT_TYPE(a, i, LVAL_STR, "joinstr");
	
	lval* res = lval_str("");
	
	while (a->count)
	{
		lval* str = list_pop(a, 0);
		
		if (str->str[0] == '\0')
		{
			lval_del(str);
			continue;
		}
		
		res->str = realloc(res->str, strlen(res->str) + strlen(str->str) + 1);
		DIE_IF_NULL(res->str);
		strcat(res->str, str->str);
		lval_del(str);
	}
	
	lval_del(a);
	return res;
}

lval* builtin_show(lenv* e, lval* a)
{
	LASSERT_COUNT(a, 1, "show");

	lval* x = list_take(a, 0);
	lval* res = lval_to_str(x);
	lval_del(x);
	
	return res;
}
