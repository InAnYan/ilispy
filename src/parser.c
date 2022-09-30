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

#include <parser.h>

#define o(name, rule) static mpc_parser_t* name;
// Parsers global variables
PARSERS
#undef o

#define o(name, rule)							\
	const mpc_parser_t* get_parser_##name ()	\
	{ return name; }
// Parsers' getters definitions
PARSERS
#undef o

#define o(name, rule)  PARSER_##name ,
#undef of
#define of(name, rule) PARSER_##name = 0 ,

typedef enum parsers_enum
{
	// Parsers' enum (used for calculating count of parsers)
	PARSERS
	PARSERS_COUNT
} parsers_enum;

#undef o
#undef of
#define of(a1, a2) o(a1, a2) // Restoring defaults

static bool inited = false;

void init_parsers()
{
	assert(!inited);

#define o(name, rule) name = mpc_new(#name);
	// Parsers' constructors
	PARSERS
#undef o

#define o(name, rule) #name ":" rule ";"
#define p(name, rule) name ,
#undef pl
#define pl(name, rule) name

// The most hard part, that requires additional macro
		mpca_lang(MPCA_LANG_DEFAULT,
				  PARSERS,
				  PARSERS_ADDITIONAL);
	
#undef o
#undef p
#undef pl
#define pl(a1, a2) p(a1, a2)
	
	inited = true;
}

void free_parsers()
{
	assert(inited);

#define o(name, rule) name ,
#undef ol
#define ol(name, rule) name
  
	mpc_cleanup(PARSERS_COUNT, PARSERS);

#undef o
#undef ol
#define ol(a1, a2) o(a1, a2)
	
	inited = false;
}
