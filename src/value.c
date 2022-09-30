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

#include <value.h>

#include <environment.h>

const char* lval_type_str(lval_type type)
{
	switch (type)
	{
	case LVAL_BUILTIN: return "Builtin function";
	case LVAL_ERR:     return "Error";
	case LVAL_LAMBDA:  return "Lambda";
	case LVAL_NUM:     return "Number";
	case LVAL_LIST:    return "List";
	case LVAL_SYM:     return "Symbol";
	case LVAL_STR:     return "String";
	case LVAL_BOOL:    return "Boolean";
	case LVAL_QUOTE:   return "Quoted type";
	case LVAL_MACRO:   return "Macros";
	}

	assert(false);
}

lval* alloc_lval(lval_type type)
{
	lval* v = malloc(sizeof(lval));
	DIE_IF_NULL(v);
	v->type = type;
	return v;
}

lval* lval_num(long x)
{
	lval* v = alloc_lval(LVAL_NUM);
	v->num = x;
	return v;
}

lval* lval_bool(bool x)
{
	lval* v = alloc_lval(LVAL_BOOL);
	v->boolean = x;
	return v;
}

lval* lval_err(const char* fmt, ...)
{
	va_list lst;
	va_start(lst, fmt);
	lval* res = lval_verr(fmt, lst);
	va_end(lst);

	return res;
}

lval* lval_verr(const char* fmt, va_list lst)
{
	lval* v = alloc_lval(LVAL_ERR);
	
	char buffer[MAX_ERR_LENGTH];
	vsnprintf(buffer, sizeof(buffer), fmt, lst);
	
	v->err = malloc(strlen(buffer) + 1);
	DIE_IF_NULL(v->err);
	strcpy(v->err, buffer);
	return v;
}

lval* lval_sym(const char* sym)
{
	lval* v = alloc_lval(LVAL_SYM);
	v->sym = malloc(strlen(sym) + 1);
	DIE_IF_NULL(v->sym);
	strcpy(v->sym, sym);
	return v;
}

lval* lval_str(const char* str)
{
	lval* v = alloc_lval(LVAL_STR);
	v->str = malloc(strlen(str) + 1);
	DIE_IF_NULL(v->str);
	strcpy(v->str, str);
	return v;

}

lval* lval_str_char(const char ch)
{
	lval* v = alloc_lval(LVAL_STR);
	v->str = malloc(2);
	DIE_IF_NULL(v->str);
	v->str[0] = ch;
	v->str[1] = '\0';
	return v;
}

lval* lval_str_null()
{
	lval* v = alloc_lval(LVAL_STR);
	v->str = NULL;
	return v;
}

lval* lval_list()
{
	lval* v = alloc_lval(LVAL_LIST);
	v->count = 0;
	v->cells = NULL;
	return v;
}

lval* lval_lambda(lenv* env, lval* formals, lval* body)
{
	lval* v = alloc_lval(LVAL_LAMBDA);
	v->env = env;
	v->formals = formals;
	v->body = body;
	return v;
}

lval* lval_macro(lval* formals, lval* body)
{
	lval* v = alloc_lval(LVAL_MACRO);
	v->formals = formals;
	v->body = body;
	return v;
}

lval* lval_builtin(lbuiltin_func func)
{
	lval* v = alloc_lval(LVAL_BUILTIN);
	v->builtin = func;
	return v;
}

lval* lval_quote(lval* x)
{
	lval* v = alloc_lval(LVAL_QUOTE);
	v->quoted = x;
	return v;
}

lval* lval_copy(lval* a)
{
	lval* v = NULL;
	switch (a->type)
	{
	case LVAL_BUILTIN:
		v = lval_builtin(a->builtin);
		break;
	case LVAL_ERR:
		v = lval_err(a->err);
		break;
	case LVAL_STR:
		v = lval_str(a->str);
		break;
	case LVAL_BOOL:
		v = lval_bool(a->boolean);
		break;
	case LVAL_QUOTE:
		v = lval_quote(lval_copy(a->quoted));
		break;
	case LVAL_LAMBDA:
		v = lval_lambda(lenv_copy(a->env),
						lval_copy(a->formals),
						lval_copy(a->body));
		break;
	case LVAL_MACRO:
		v = lval_macro(lval_copy(a->formals),
					   lval_copy(a->body));
		break;
	case LVAL_NUM:
		v = lval_num(a->num);
		break;
	case LVAL_LIST:
		v = lval_list();
		v->count = a->count;
		v->cells = malloc(sizeof(lval*) * a->count);
		DIE_IF_NULL(v->cells);
		for (unsigned i = 0; i < a->count; i++)
			v->cells[i] = lval_copy(a->cells[i]);
		break;
	case LVAL_SYM:
		v = lval_sym(a->sym);
		break;
	}

	assert(v != NULL);
	return v;
}

void lval_del(lval* v)
{
	if (v == NULL) return;
	
	switch (v->type)
	{
	case LVAL_BOOL:
	case LVAL_BUILTIN:
	case LVAL_NUM: break;

	case LVAL_ERR: free(v->err); break;
	case LVAL_SYM: free(v->sym); break;
	case LVAL_STR: free(v->str); break;

	case LVAL_QUOTE: if (v->quoted != NULL) lval_del(v->quoted); break;
	
	case LVAL_LIST:
		for (unsigned i = 0; i < v->count; i++)
			lval_del(v->cells[i]);
		free(v->cells);
		break;
		
	case LVAL_LAMBDA:
		if (v->env     != NULL) lenv_del(v->env);
		if (v->formals != NULL) lval_del(v->formals);
		if (v->body    != NULL) lval_del(v->body);
		break;
	
	case LVAL_MACRO:
		if (v->formals != NULL) lval_del(v->formals);
		if (v->body    != NULL) lval_del(v->body);
		break;
	}

	free(v);
}

lval* list_add(lval* v, lval* x)
{
	assert(v != NULL);
	assert(x != NULL);
	assert(IS_LIST(v));
	
	v->count++;
	v->cells = realloc(v->cells, sizeof(lval*) * (v->count));
	DIE_IF_NULL(v->cells);
	v->cells[v->count - 1] = x;

	return v;
}

lval* list_take(lval* v, unsigned i)
{
	lval* x = list_pop(v, i);
	lval_del(v);
	return x;
}

lval* list_pop(lval* v, unsigned i)
{
	assert(IS_LIST(v));
	assert(i < v->count);
	
	lval* x = v->cells[i];

	memmove(&v->cells[i],
			&v->cells[i + 1],
			sizeof(lval*) * (v->count - i - 1));

	v->count--;

	v->cells = realloc(v->cells, sizeof(lval*) * v->count);
	if (v->count != 0) DIE_IF_NULL(v->cells);
	return x;
}

lval* list_join(lval* x, lval* y)
{
	while (y->count)
		x = list_add(x, list_pop(y, 0));

	lval_del(y);
	return x;
}

lval* lval_to_str(lval* a)
{
	assert(a != NULL);

	lval* res = NULL;

	switch (a->type)
	{
	case LVAL_ERR:
		res = lval_str(a->err);
		break;
	case LVAL_SYM:
		res = lval_str(a->sym);
		break;
	case LVAL_STR:
		res = lval_str(a->str);
		break;
	case LVAL_NUM:
	{
		char* str = malloc(MAX_INT_STR_LENGTH);
		sprintf(str, "%ld", a->num);
		res = lval_str_null();
		res->str = str;
		break;
	}
	case LVAL_BOOL:
		res = lval_str(a->boolean ? "true" : "false");
		break;
	case LVAL_LAMBDA:
		res = lval_str("<lambda>");
		break;
	case LVAL_BUILTIN:
		res = lval_str("<builtin>");
		break;
	case LVAL_MACRO:
		res = lval_str("<macro>");
		break;
	case LVAL_QUOTE:
	{
		lval* quoted = lval_to_str(a->quoted);
		res = lval_str_null();
		res->str = malloc(strlen(quoted->str) + 1 + 1);
		DIE_IF_NULL(res->str);
		res->str[0] = '\'';
		res->str[1] = '\0';
		strcat(res->str, quoted->str);
		lval_del(quoted);
		break;
	}
	case LVAL_LIST:
		res = list_to_str(a);
		break;
	}

	assert(res != NULL);
	return res;
}

lval* list_to_str(lval* v)
{
	assert(v != NULL);

	lval* res = lval_str("(");
	bool inStart = true;

	for (unsigned i = 0; i < v->count; i++)
	{
		lval* x = lval_to_str(v->cells[i]);
		res->str = realloc(res->str, strlen(res->str) + strlen(x->str) + 1 + (inStart ? 0 : 1));
		DIE_IF_NULL(res->str);
		if (!inStart)
		{
			size_t size = strlen(res->str);
			res->str[size] = ' ';
			res->str[size + 1] = '\0';
		} else inStart = false;
		strcat(res->str, x->str);
		lval_del(x);
	}

	size_t size = strlen(res->str);
	res->str = realloc(res->str, size + 1 + 1);
	res->str[size] = ')';
	res->str[size + 1] = '\0';

	return res;
}

void lval_print(lval* v)
{
	assert(v != NULL);

	if (IS_ERR(v))
	{
		printf("runtime error: %s", v->err);
	}
	else
	{
		lval* str = lval_to_str(v);
		printf("%s", str->str);
		lval_del(str);
	}
}

void lval_println(lval* v)
{
	lval_print(v);
	putchar('\n');
}

bool lval_eq(lval* a, lval* b)
{
	assert(a != NULL);
	assert(b != NULL);

	if (a->type != b->type) return false;

	switch (a->type)
	{
	case LVAL_NUM: return a->num == b->num;
	case LVAL_BOOL: return a->boolean == b->boolean;
	
	case LVAL_SYM: return strcmp(a->sym, b->sym) == 0;
	case LVAL_ERR: return strcmp(a->err, b->err) == 0;
	case LVAL_STR: return strcmp(a->str, b->str) == 0;
	
	case LVAL_BUILTIN: return a->builtin == b->builtin;
	case LVAL_LAMBDA: return lval_eq(a->formals, b->formals) && lval_eq(a->body, b->body);
	case LVAL_MACRO: return lval_eq(a->formals, b->formals) && lval_eq(a->body, b->body);

	case LVAL_LIST:
		if (a->count != b->count) return false;
		for (unsigned i = 0; i < a->count; i++)
			if (!lval_eq(a->cells[i], b->cells[i])) return false;
		return true;
	default:
		return false;
	}
}

lval* lval_unquote(lval* a)
{
	assert(IS_QUOTE(a));
	assert(a->quoted != NULL);
	
	lval* v = a->quoted;
	a->quoted = NULL;
	lval_del(a);
	
	return v;
}
