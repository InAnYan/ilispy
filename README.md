# Ilispy
Ilispy - Lisp dialect and interpreter.  
It was written, while reading a book "Build Your Own Lisp" by Daniel Holden [https://www.buildyourownlisp.com/].  
This Lisp dialect is similar to the original and to the one in book.  
Ilispy has quoted expressions and macros. It doesn't have Q-exprs.  
Untested fully, quality is not guaranteed.  

# Usage and building
	./ilispy [filename]
While no `filename` is provided, the program will start in interpreter mode, where you can type expressions from command line.  
If there is a `filename` the program will interpret this file.  
  
To build this program, create directories `bin` and `obj` and type `make` in the root directory of the project.  
IMPORTANT: edit the Makefile and fill `DEFINES` according to your system (`LISPY_COMPILE_LINUX`, `LISPY_COMPILE_OSX` or `LISPY_COMPILE_OTHER`).  

# Values
There are n types of value:

* __Number__ - signed integer. `13`, `42`, etc.
* __Boolean__ - contains true or false. `true` or `false`.
* __Symbol__ - like a Lisp symbol. `node-type`, `number?`, it's like identifier in other languages, but it can contain a lot of different characters.
* __List__ - list of Ilispy values. `'(1 2 3)` or `(first second third)`.
* __String__ - sequence of characters.
* __Lambda__ - unnamed function.
* __Macro__ - unnamed macro.
* __Builtin__ - function, that written in C, but executes in Lispy.
* __Error__ - error string.

The last 4 cannot be typed directly, they are results of builtin functions. Also, there is `Quoted type`, it contains value, which evaluation is delayed (details in [Evaluation] section).

Ilispy value type and its equivalent in C
| Ilispy value type  | Type in C                                                             |
|--------------------|-----------------------------------------------------------------------|
| Ilispy value       | `lval`                                                                |
| Ilispy environment | `lenv`                                                                |
| Number             | `long`                                                                |
| Boolean            | `bool`                                                                |
| Symbol             | `char*`                                                               |
| List               | ``` struct {     unsigned count;     struct lval** cells; } ```       |
| String             | `char*`                                                               |
| Lambda             | ``` struct {     lenv* env;     lval* formals;     lval* body; }  ``` |
| Builtin            | `lval* (*lbuiltin_func)(lenv* e, lval* a)`                            |
| Macro              | ``` struct {     lval* formals;     lval* body; } ```                 |

# Evaluation
Ilispy program - is a List. A List, that under evaluation, is named `S-expr`. By default, the first member of S-expr is a function and the rest are arguments of function (like code mode in Lisp). For example:

    (+ 1 2 3)

It returns sum of 1, 2 and 3. This method of writing expressions is similar to Polish notation.  
Because List can store any Ilispy value, the Lists can be nested:  

	(+ 10 (- 5 9))

It's similar to Lisp, Scheme, Common Lisp, etc.  
Before a function call all S-expr elements are evaluated. This is problematic in a lot of cases. For instance:  

	(head (4 5 6))

I have List of 3 elements, I want to take the first element of the List. Because all elements are evaluated, the interpreter will try to call `4` with arguments `5` and `6`. To prevent this, Ilispy uses Quoted type:  

	(head '(4 5 6))

Quoted type is simply a wrapper on Ilispy value. It's purpose is "to delay" evaluation (it's really a "delay", it isn't a switchig to data mode). This means, that evaluation of a Quoted type return wrapped value, it doesn't try to evaluate wrapped value. Look at the example (`->` means evaluation pass):  

	(+ 1 2 3) -> 6
	'(+ 1 2 3) -> (+ 1 2 3)
	(head (+ 2 3)) -> (head 6) -> error
	(head '(+ 2 3)) -> +

Again, it's similar to Lisp. But in Ilispy you can type, probably, funny things:  

	'1 -> 1
	''(1 2 3) -> '(1 2 3) -> (1 2 3)

Quotes can be nested and be placed on any type.  
Yes, that's not a `Q-expr`, and maybe `Q-expr` is a better concept because it doesn't have those strange cases.    

The rules of evalutaion of Ilispy value:

1. If it is a Symbol, then retrieve value from current environment by symbol.  
2. If it is a List, then evaluate List as S-expr.  
3. Otherwise, just return it.  

The rules of evaluation of S-expr:

1. Evaluate first element of the List.  
2. If the result is a macro, then expand macro and evaluate the expanded expression.
3. Otherwise, evaluate the rest of the List and use it as arguments.  
2. Take the first element:
	* If it's a Builtin, then call it with arguments.  
	* If it's a Lambda, then call it with arguments.  
	* Otherwise, return an error.  

# Macros
I don't know how macros work in Lisp.  
Macros in Ilispy iare very simple: macros contain formal and actual arguments, like a lambda. A macro tries to find symbol in its body, which equals to formal, and replaces it with actual. Look at the example:  

	(defmacro head! (lst) (head 'lst))
	(head! (1 2 3)) -> (head '(1 2 3))

All it have done is replaced all occurences of `lst` in macro body with `(1 2 3)`.  
The existence of macros gives ability to a lot of things. One of the usage is:  

	(defmacro defun (name params body) (def 'name (\ 'params 'body)))
	(defun inc (n) (+ n 1))

Without macros, we have to do this:  

	(def 'inc (\ '(n) '(+ n 1)))
	
# End
The rest of the language is similar to Lisp and Lispy. Look at the files in examples directory.
