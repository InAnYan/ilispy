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


#include <common.h>

#include <mpc/mpc.h>

#include <value.h>
#include <environment.h>
#include <eval.h>
#include <builtins.h>
#include <parser.h>
#include <reader.h>

#define MAX_INPUT_LENGTH 2048

#if defined(LISPY_COMPILE_LINUX) || defined(LISPY_COMPILE_OSX)

#include <editline/readline.h>
#include <editline/history.h>

#else

static char buffer[MAX_INPUT_LENGTH];

char* readline(const char* prompt)
{
	printf(prompt);
	fgets(buffer, MAX_INPUT_LENGTH, stdin);
	char* cpy = malloc(strlen(buffer) + 1);
	DIE_IF_NULL(cpy);
	strcpy(cpy, buffer);
	cpy[strlen(cpy) - 1] = '\0';
	return cpy;
}

void add_history(char* unused) {}

void clear_history() {}

#endif

void read_file(int argc, char** argv);
void repl();
bool load_prelude(bool isSilence);

static lenv* globalEnv;

int main(int argc, char** argv)
{
	init_parsers();
	
	globalEnv = lenv_new(NULL);
	add_builtins(globalEnv);

	{
		lval nameKey; nameKey.type = LVAL_SYM; nameKey.sym = "__name__";
		lval nameValue; nameValue.type = LVAL_SYM; nameValue.sym = "__main__";
		lenv_put(globalEnv, &nameKey, &nameValue);
	}
	
	if (!load_prelude(argc != 1) && argc == 1)
		printf("warning: proceeding without prelude\n\n");
	
	if (argc != 1)
		read_file(argc, argv);
	else
		repl();

	lenv_del(globalEnv);
	clear_history();
	free_parsers();
	
	return 0;
}

void repl()
{
	printf("Ilispy v0.1.0\n");
	printf("Press Ctrl+C or type \":quit\" (without quotes) to exit\n\n");
	
	char* input;

	for (;;)
	{
		input = readline("ilispy> ");
		add_history(input);
		
		if (memcmp(input, ":quit", 5) == 0)
		{
			break;
		}
		
		mpc_result_t r;
		if (mpc_parse("<stdin>", input, (mpc_parser_t*) get_parser_lispy(), &r))
		{
			lval* program = read_lval(r.output);

			lval* result = eval_lval(globalEnv, program);

			lval_println(result);
			
			lval_del(result);
			mpc_ast_delete(r.output);
		}
		else
		{
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}

		free(input);
	}

	free(input);
}

void read_file(int argc, char** argv)
{
	// TODO: argv, argc into lisp
	
	lval* args = lval_list();
	list_add(args, lval_str(argv[1]));
	lval* res = builtin_load_impl(globalEnv, args, true);
	if (IS_ERR(res))
		printf("runtime error: %s\n", res->err);
	lval_del(res);
}

bool load_prelude(bool isSilence)
{
	lval* args = lval_list();
	list_add(args, lval_str(PRELUDE_FILE_NAME));
	lval* res = builtin_load(globalEnv, args);
	if (IS_ERR(res))
	{
		if (!isSilence)
			printf("runtime error: %s\n", res->err);
		lval_del(res);
		return false;
	}
	lval_del(res);
	
	return true;
}
