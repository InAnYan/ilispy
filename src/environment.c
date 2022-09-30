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

#include <environment.h>

#include <value.h>

lenv* lenv_new(lenv* parent)
{
	lenv* env = malloc(sizeof(lenv));

	DIE_IF_NULL(env);
	env->entries = NULL;
	env->parent = parent;
	env->count = 0;
	
	return env;
}

void lenv_del(lenv* env)
{
	if (env->entries != NULL)
	{
		for (unsigned i = 0; i < env->count; i++)
		{
			free(env->entries[i].key);
			lval_del(env->entries[i].value);
		}
	}

	free(env->entries);
	free(env);
}

lenv* lenv_copy(lenv* env)
{
	assert(env != NULL);
	
	lenv* res = lenv_new(env->parent);

	for (unsigned i = 0; i < env->count; i++)
	{
		lval key;
		key.type = LVAL_SYM;
		key.sym = env->entries[i].key;
		lenv_put(res, &key, env->entries[i].value);
	}

	return res;
}

lval* lenv_get(lenv* env, lval* key)
{
	assert(env != NULL);
	assert(IS_SYM(key));

	for (unsigned i = 0; i < env->count; i++)
	{
		if (strcmp(env->entries[i].key, key->sym) == 0)
		{
			return lval_copy(env->entries[i].value);
		}
	}
	
	if (env->parent != NULL) return lenv_get(env->parent, key);
	else return lval_err("symbol '%s' is not bound to anything",
						 key->sym);
}

bool lenv_set(lenv* env, lval* key, lval* value)
{
	assert(env != NULL);
	assert(IS_SYM(key));

	for (unsigned i = 0; i < env->count; i++)
	{
		if (strcmp(env->entries[i].key, key->sym) == 0)
		{
			lval_del(env->entries[i].value);
			env->entries[i].value = lval_copy(value);
			return true;
		}
	}

	if (env->parent != NULL) return lenv_set(env->parent, key, value);
	else return false;
}

void lenv_internal_put(lenv* env, lval* key, lval* value)
{
	env->count++;

	env->entries = realloc(env->entries, sizeof(lenv_entry) * env->count);
	DIE_IF_NULL(env->entries);

	env->entries[env->count - 1].key = malloc(strlen(key->sym) + 1);
	DIE_IF_NULL(env->entries[env->count - 1].key);
	strcpy(env->entries[env->count - 1].key, key->sym);
	env->entries[env->count - 1].value = lval_copy(value);
}

void lenv_put(lenv* env, lval* key, lval* value)
{
	assert(env != NULL);
	assert(IS_SYM(key));

	for (unsigned i = 0; i < env->count; i++)
	{
		if (strcmp(env->entries[i].key, key->sym) == 0)
		{
			lval_del(env->entries[i].value);
			env->entries[i].value = lval_copy(value);
			return;
		}
	}

	lenv_internal_put(env, key, value);
}

void lenv_def(lenv* env, lval* key, lval* value)
{
	assert(env != NULL);
	
	while (env->parent) env = env->parent;

	lenv_put(env, key, value);
}

bool lenv_put_new(lenv* env, lval* key, lval* value)
{
	assert(env != NULL);
	assert(IS_SYM(key));

	for (unsigned i = 0; i < env->count; i++)
	{
		if (strcmp(env->entries[i].key, key->sym) == 0)
		{
			return false;
		}
	}

	lenv_internal_put(env, key, value);

	return true;
}

bool lenv_def_new(lenv* env, lval* key, lval* value)
{
	assert(env != NULL);
	
	while (env->parent != NULL) env = env->parent;

	return lenv_put_new(env, key, value);
}
