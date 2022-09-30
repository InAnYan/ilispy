; lval format: (val-type x)
; node format: (node-type ...)
; list-node: (list-node-type ...)
; number-node (number-node-type number)
; symbol-node: (symbol-node-type string)
; quoted-node: (quoted-node-type quoted)

(def list-node-type   1)
(def number-node-type 2)
(def symbol-node-type 3)
(def quoted-node-type 4)
(def builtin-node-type 5)
(def lambda-node-type 6)

(defun node-type    (node) (fst node))
(defun node-number  (node) (snd node))
(defun node-symbol  (node) (snd node))
(defun node-quoted  (node) (snd node))
(defun node-builtin (node) (snd node))

(defun node-type-is (is node) (eq (node-type node) is))

(defun symbol?  (node) (node-type-is symbol-node-type  node))
(defun number?  (node) (node-type-is number-node-type  node))
(defun list?    (node) (node-type-is list-node-type    node))
(defun quoted?  (node) (node-type-is quoted-node-type  node))
(defun lambda?  (node) (node-type-is lambda-node-type  node))
(defun builtin? (node) (node-type-is builtin-node-type node))

; I didn't add this to prelude, because I doubt about its proper syntax
(defun __switch (value cases) (
	   if (eq (length cases) 1)

	   	  (eval (head cases))
		  
		  (if (eq (eval (head (head cases))) value)
	   	  	  (eval (tail (head cases)))
		  	  (__switch value (tail cases)) )
))

(defmacro switch (value cases) (__switch value 'cases))

(def lisp-env (list
	 (list "+" (lambda (env lst) (list env foldl + 0 lst)) )
 	 (list "-" (lambda (env lst) (list env foldl - 0 lst)) )
 	 (list "*" (lambda (env lst) (list env foldl * 1 lst)) )		
 	 (list "/" (lambda (env lst) (list env foldl / 1 lst)) )
  	 (list "def" (lambda (env lst) (set lisp-env (env-def env (nth 0 lst) (nth 1 lst)))) )
))

(defun env-contains (env x) (
	   if (eq env nil)
	   	  (false)
		  (if (eq (nth 0 (head env)) x)
		  	  (true)
			  (env-contains (tail env) x))
))

(defun env-get (env x) (
	   if (eq env nil)
	   	  (error (joinstr "unbound symbol: " (show x)))
		  (if (eq (nth (head env) 0) x)
		  	  (nth (head env) 1)
			  (env-get (tail env) x) )
))

(defun env-put (env key val) (
	   if (env-contains env key)
	   	  (env-set env key value)
		  (cons (list key val) env)
))

(defun env-set (env key val) (
	   if (eq (nth 0 (head env)) key)
	   	  (cons (list key val) env)
		  (cons (head env) (env-set (tail env) key val))
))

; I don't know how to implement environments properly
(defun evaluate (node) (
	 switch (node-type node) (
	 		(symbol-node-type (
			    
			))
			(number-node-type (
			    list env (node-number node)
			))
			(list-node-type (
				 (nth (evaluate env (head (tail node))) 1) env (map (evaluate env) (tail (tail node)))
			))
			(quoted-node-type (list env (node-quoted node)))
			(error (joinstr "unrecognized node type: " (show (node-type node))))
	 )
))

(defun evaluate-n (env nodes) (
	   if (eq nodes nil)
	   	  (env)
		  (evaluate-n (head (evaluate env (head nodes))) (tail nodes))
))

(load "../examples/wordsplit.ls")

(defun tokenize (str) (
	   split-str (convert-str-to-list " \n\t") (convert-str-to-list str)
))

(defun create-ast (tokens) (
	   if (eq tokens nil)
	   	  (nil)
		  (cons (get-parsed tokens) (create-ast (drop-parsed tokens)))
))

(defun get-parsed (tokens) (
	   cond
	   		'((eq (head tokens) '("(")) (cons list-node-type (get-parsed-list (tail tokens))))
			'((eq (head tokens) '("'")) (list quoted-node-type (get-parsed (tail tokens))))
			'((str-number? (head tokens)) (list number-node-type (str-to-num (head tokens))))
			'((otherwise) (list symbol-node-type (head (head tokens))))
))

(defun drop-parsed (tokens) (
	   cond
	   		'((eq (head tokens) '("(")) (drop-parsed-list (tail tokens)))
			'((eq (head tokens) '("'")) (drop-parsed (tail tokens)))
			'((otherwise) (tail tokens))
))

(defun get-parsed-list (tokens) (
	   if (eq (head tokens) '(")"))
	   	  (nil)
		  (cons-no-nil (get-parsed tokens) (get-parsed-list (drop-parsed tokens)))
))

(defun drop-parsed-list (tokens) (
	   if (eq (head tokens) '(")"))
	   	  (tail tokens)
		  (drop-parsed-list (tail tokens))
))

(defun str-number? (str) (
	   foldl and true (map (lambda (x) (element x numbers-list)) str)
))

(def str-to-num-map '(
	 ("0" 0)
	 ("1" 1)
	 ("2" 2)
	 ("3" 3)
	 ("4" 4)
	 ("5" 5)
	 ("6" 6)
	 ("7" 7)
	 ("8" 8)
	 ("9" 9)
))

(defun str-to-num (str) (
	   concat-digit-list (map (find-digit str-to-num-map) str)
))

(defun find-digit (map digit) (
	   if (eq digit (head (head map)))
	   	  (head (tail (head map)))
		  (find-digit (tail map) digit)
))

(defun find-digit (map digit) (
	   if (eq (nth (head map) 0) digit)
	   	  	  (nth (head map) 1)
		  	  (find-digit (tail map) digit)
))

(defun concat-digit-list-impl (lst) (
	   if (eq lst nil)
	   	  (0)
	   	  (+ (head lst) (* 10 (concat-digit-list-impl (tail lst))))
))

(defun concat-digit-list (lst) (concat-digit-list-impl (reverse lst)))

(def numbers-list '("0" "1" "2" "3" "4" "5" "6" "7" "8" "9"))

(def program-str " ( + 42 7 ) " )

(println (tail (evaluate lisp-env (head (create-ast (tokenize program-str))))))
;(println (create-ast (tokenize program-str)))