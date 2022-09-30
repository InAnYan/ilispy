; Evaluator (?)
; Evaluates AST, which in form of list
; 
; Actually, I don't know how to write lexical and parsers 
; in functional style, expect this tiny program

; Node types
; Absolutely don't know how to program this correctly
; I know, these are unsafe enums

(def lit-node-type 1)
(def bin-node-type 2)

(def add-bin-op 1)
(def sub-bin-op 2)
(def mul-bin-op 3)
(def div-bin-op 4)

; node format: (node-type ...)
; literal: (lit-node-type number)
; binary operators: (bin-node-type bin-op node1 node2)

; It's like getters
(defun node-type   (node) (fst node))
(defun node-bin-op (node) (snd node))
(defun node-bin-a  (node) (nth node 2))
(defun node-bin-b  (node) (nth node 3))

; Helpers
(defun node-type-is   (is node) (eq (node-type   node) is))
(defun node-bin-op-is (is node) (eq (node-bin-op node) is))

(defun lit? (node) (node-type-is lit-node-type node))
(defun bin? (node) (node-type-is bin-node-type node))

(defun add? (node) (node-bin-op-is add-bin-op node))
(defun sub? (node) (node-bin-op-is sub-bin-op node))
(defun mul? (node) (node-bin-op-is mul-bin-op node))
(defun div? (node) (node-bin-op-is div-bin-op node))

(def program (list bin-node-type add-bin-op (list lit-node-type 1) (list lit-node-type 2)))

; Cond is not a macro, and its arguments should be quoted
; That's because macros cannot have variable arguments count

(defun evaluate (node) (
	cond
		'((lit? node) (snd node))
		'((bin? node) (
			cond
				'((add? node) (+ (evaluate (node-bin-a node)) (evaluate (node-bin-b node))))
				'((sub? node) (- (evaluate (node-bin-a node)) (evaluate (node-bin-b node))))
				'((mul? node) (* (evaluate (node-bin-a node)) (evaluate (node-bin-b node))))
				'((div? node) (/ (evaluate (node-bin-a node)) (evaluate (node-bin-b node))))
				'(true (error "unrecognized binary operation"))
		))
		'(true (error "unrecognized node type"))
))

(if (eq __name__ __main__)
	(println (evaluate program))
	(nil)
)