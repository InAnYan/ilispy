; print - prints its arguments, separated by space
; println - same, as print, but adds new line character at the end
(println "Hello, World!")

; nil - shorthand for '()
; () equals '()
(defun isort (lst) (
	   if (eq lst nil)
	   	  ('())
		  (insert (head lst) (isort (tail lst)))
))

(defun insert (x lst) (
	   if (eq lst '())
	   	  (list x) ; Actually, I wrote '(x), but that means, that x won't be evaluated
	   	  (if (less x (head lst))
	   	  	 (cons x lst)
		  	 (cons (head lst) (insert x (tail lst))))
))

(println (isort '(1 5 172 95 2)))