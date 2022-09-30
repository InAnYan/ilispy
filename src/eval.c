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

#include <eval.h>
#include <builtins.h>

lval* eval_func_call(lenv* env, lval* func, lval* args);
lval* eval_lval_expr(lenv* env, lval* v);

lval* eval_lval(lenv* env, lval* v)
{
	if (IS_SYM(v))
	{
		lval* got = lenv_get(env, v);
		lval_del(v);
		return got;
	}
	
	if (IS_LIST(v) && v->count != 0)
		return eval_lval_expr(env, v);

	if (IS_QUOTE(v))
		return lval_unquote(v);
	
	return v;
}

lval* eval_lval_expr(lenv* env, lval* v)
{
	assert(IS_LIST(v));
	
	v->cells[0] = eval_lval(env, v->cells[0]);
	if (IS_MACRO(v->cells[0]) && v->count != 1)
	{
		lval* macro = list_pop(v, 0);
		lval* expanded = eval_macro_expand(macro, v);
		lval_del(v);
		lval_del(macro);
		lval* evaluated = eval_lval(env, expanded);
		return evaluated;
	}
	
	for (unsigned i = 1; i < v->count; i++)
	{
		v->cells[i] = eval_lval(env, v->cells[i]);
	}
	
	for (unsigned i = 0; i < v->count; i++)
	{
		if (IS_ERR(v->cells[i]))
			return list_take(v, i);
	}

	if (v->count == 0) return v;
	if (v->count == 1) return list_take(v, 0);

	lval* f = list_pop(v, 0);
	
	lval* result = eval_func_call(env, f, v);
	
	return result;
}

lval* eval_func_call(lenv* env, lval* func, lval* args)
{
	if (!(IS_LAMBDA(func) || IS_BUILTIN(func)))
	{
		lval_del(func);
		lval_del(args);
		return lval_err("attempt to call non-callable value");
	}
	
	if (IS_BUILTIN(func))
	{
		lbuiltin_func builtin = func->builtin;
		lval_del(func);
		return builtin(env, args);
	}

	if (IS_LAMBDA(func))
	{
		while (args->count)
		{
			if (func->formals->count == 0)
			{
				lval_del(func);
				lval_del(args);
				return lval_err("passed too many arguments to function");
			}

			lval* formal = list_pop(func->formals, 0);

			if (strcmp(formal->sym, "&") == 0)
			{
				// assuming that there is no parameters after rest parameter
				// builtin "lambda" should guarantee this safety
				lval_del(formal);
				lval* rest = list_pop(func->formals, 0);
				lenv_put(func->env, rest, builtin_list(env, args));
				lval_del(rest);
				break;
			}
			
			lval* actual = list_pop(args, 0);
			lenv_put(func->env, formal, actual);
			lval_del(formal);
			lval_del(actual);
		}

		if (func->formals->count != 0 && strcmp(func->formals->cells[0]->sym, "&") == 0)
		{
			lval_del(list_pop(func->formals, 0));

			lval* key = list_pop(func->formals, 0);
			lval* val = lval_list();

			lenv_put(func->env, key, val);

			lval_del(key);
			lval_del(val);
		}
		
		lval_del(args);
		func->env->parent = env;

		if (func->formals->count != 0)
		{
			return func;
		}
		else
		{
			lval* bodyCopy = lval_copy(func->body);
			lval* res = eval_lval(func->env, bodyCopy);
			lval_del(func);
			return res;
		}
	}

	assert(false && "Unreachable");
}

lval* eval_macro_expand(lval* macro, lval* args)
{
	if (macro->formals->count < args->count)
		return lval_err("macro passed too much arguments");
	else if (macro->formals->count > args->count)
		return lval_err("macro passed not enough arguments");
	
	lval* expr = macro->body;
	macro->body = NULL;
	
	while (macro->formals->count)
	{
		lval* formal = list_pop(macro->formals, 0);
		lval* actual = list_pop(args, 0);
		eval_macro_replace(expr, formal, actual);
		lval_del(formal);
		lval_del(actual);
	}
	
	return expr;
}

void eval_macro_replace(lval* expr, lval* formal, lval* actual)
{
	assert(expr != NULL);
	assert(formal != NULL);
	assert(actual != NULL);
	assert(IS_SYM(formal));
	
	if (IS_SYM(expr) && strcmp(expr->sym, formal->sym) == 0)
	{
		assert(false && "Got uncaught symbol. It should be replaced in upper call frame");
	}
	
	switch (expr->type)
	{
	case LVAL_NUM:
	case LVAL_BUILTIN:
	case LVAL_ERR:
	case LVAL_STR:
	case LVAL_MACRO: // TODO: Is this possible?
	case LVAL_BOOL:
		break;
	case LVAL_LAMBDA:
		eval_macro_replace(expr->formals, formal, actual);
		eval_macro_replace(expr->body, formal, actual);
		break;
	case LVAL_LIST:
		for (unsigned i = 0; i < expr->count; i++)
		{
			if (IS_SYM(expr->cells[i]) 
					&& strcmp(expr->cells[i]->sym, formal->sym) == 0)
			{
				lval_del(expr->cells[i]);
				expr->cells[i] = lval_copy(actual);
			}
			else eval_macro_replace(expr->cells[i], formal, actual);
		}
		break;
	case LVAL_QUOTE:
		if (IS_SYM(expr->quoted) 
				&& strcmp(expr->quoted->sym, formal->sym) == 0)
		{
			lval_del(expr->quoted);
			expr->quoted = lval_copy(actual);
		}
		else eval_macro_replace(expr->quoted, formal, actual);
		break;
	case LVAL_SYM:
		if(strcmp(expr->sym, formal->sym) == 0)
			assert(false && "Got uncaught symbol. It should be replaced in upper call frame");
		break;
	}
}