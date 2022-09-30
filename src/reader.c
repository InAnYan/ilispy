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


#include <reader.h>

lval* read_lval_num(mpc_ast_t* node)
{
	errno = 0;
	long x = strtol(node->contents, NULL, 10);
	return errno != ERANGE ? lval_num(x) : lval_err("invalid number");
}

lval* read_lval_str(mpc_ast_t* node)
{
	node->contents[strlen(node->contents) - 1] = '\0';
	
	char* unescaped = malloc(strlen(node->contents + 1) + 1);
	strcpy(unescaped, node->contents + 1);
	
	unescaped = mpcf_unescape(unescaped);
	
	lval* str = lval_str(unescaped);
	free(unescaped);
	
	return str;
}

lval* read_lval(mpc_ast_t* node)
{
#define CHECK_NODE(str) (strstr(node->tag, str))

	if (CHECK_NODE("quote"))  
	{
		return lval_quote(read_lval(node->children[1]));
	}
	
	if (CHECK_NODE("number"))  return read_lval_num(node);
	if (CHECK_NODE("string"))  return read_lval_str(node);
	
	if (CHECK_NODE("symbol"))
	{
		if (strcmp(node->contents, "true")  == 0) return lval_bool(true);
		if (strcmp(node->contents, "false") == 0) return lval_bool(false);
		else return lval_sym(node->contents);
	}
	
	lval* x = NULL;
	if (strcmp(node->tag, ">") == 0) x = lval_list();
	if (CHECK_NODE("list")) x = lval_list();
	
	for (unsigned i = 0; i < node->children_num; i++)
	{
		if (strcmp(node->children[i]->contents, "(") == 0)  continue;
		if (strcmp(node->children[i]->contents, ")") == 0)  continue;
		if (strcmp(node->children[i]->contents, "{") == 0)  continue;
		if (strcmp(node->children[i]->contents, "}") == 0)  continue;
		if (strcmp(node->children[i]->tag, "regex")  == 0)  continue;
		if (strstr(node->children[i]->tag, "comment"))      continue;
		x = list_add(x, read_lval(node->children[i]));
	}
	
	assert(x != NULL);
	return x;
	
#undef CHECK_NODE
}