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

#ifndef LISPY_PARSER_H
#define LISPY_PARSER_H

#include <common.h>

#include <mpc/mpc.h>

// Thanks for Bisqwit for this technique
// Added first and last functions
// Added second parsers' macro
// of - first transform
// o  - middle transform
// ol - last transform

#define PARSERS_REAL(first, middle, last)                \
   first(comment, "/;[^\\r\\n]*/")                       \
    middle(number, "/-?[0-9]+/")                         \
    middle(symbol, "/[a-zA-Z0-9_+\\-*\\/\\\\=<>!?&]+/")  \
    middle(string, "/\"(\\\\.|[^\"])*\"/")               \
    middle(list, "'(' <expr>* ')'")                      \
    middle(quote, " '\'' <expr> ")                       \
    middle(expr, "<string> | <number> | <symbol> "       \
                 "| <list> | <comment> | <quote>")       \
    last(lispy, "/^/ <expr>* /$/")

#define PARSERS PARSERS_REAL(of, o, ol)
#define PARSERS_ADDITIONAL PARSERS_REAL(pf, p, pl)

// Defaults
#define of(name, rule) o(name, rule)
#define ol(name, rule) o(name, rule)
#define pf(name, rule) p(name, rule)
#define pl(name, rule) p(name, rule)

#define o(name, rule) const mpc_parser_t* get_parser_##name();
// Parsers' getters declaration
    PARSERS
#undef o

void init_parsers();
void free_parsers();

#endif // LISPY_PARSER_H
