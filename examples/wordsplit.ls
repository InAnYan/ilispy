(defun split-str (whitespace str) (
	   split-str-impl whitespace (skip-white whitespace str)
))

(defun skip-white (whitespace str) (
	   if (eq str nil)
	   	  (nil)
		  (if (element (head str) whitespace)
	   	  	 (skip-white whitespace (tail str))
		  	 (str))
))

(defun split-str-impl (whitespace str) (
	   if (eq str nil)
	   	  (nil)
	   	  (cons (get-word whitespace str)
	   	   	    (split-str-impl whitespace (skip-white whitespace (drop-word whitespace str))) )
))

(defun get-word (whitespace str) (
	   if (eq str nil)
	   	  (nil)
	   	  (if (not (element (head str) whitespace))
	   	  	 (cons (head str) (get-word whitespace (tail str)))
			 (nil))
))

(defun drop-word (whitespace str) (
	   if (eq str nil)
	   	  (nil)
	   	  (if (not (element (head str) whitespace))
	   	  	 (drop-word whitespace (tail str))
		  	 (str))
))

(def work-str (convert-str-to-list "hi hello aloha halo bye"))
(def whitespaces (convert-str-to-list " "))

(if (eq __name__ __main__) (println (split-str whitespaces work-str)) (nil))