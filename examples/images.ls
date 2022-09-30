; Images from book 'The craft of functional programming'

(def simple-image (join (list (convert-str-to-list "###"))
	 			  		(list (convert-str-to-list "#.#"))
						(list (convert-str-to-list "###")) ))

(defun convert-image-to-str (image) (
	   if (eq image nil)
	   	  ("")
		  (joinstr (concat-str-lst (head image)) "\n" (convert-image-to-str (tail image)))
))

(defun invert-pixel (pix) (
	   if (eq pix "#")
	   	  (".")
		  ("#")
))

(defun invert-line (line) (
	   map invert-pixel line
))

; Partial computation!
(defun invert-image (image) (
	   map (map invert-pixel) image
))

(defun flipV (image) (
	   map reverse image
))

(defun flipH (image) (
	   reverse image
))

(defun rotate (image) (
	   comp flipV flipH image
))

(def some-image (list (convert-str-to-list "#...")
	 				  (convert-str-to-list ".#..")
					  (convert-str-to-list ".##.")
					  (convert-str-to-list "#..#") ))

; In Ilispy there is no functions without arguments
; It maybe looks ugly, but it works
(defun script_main (n) (
	   progn

	   ; The strings are printed without quotes
	   (print "Converting string to list of characters: ")
	   (println (convert-str-to-list "abc"))

	   ; Not pretty output
	   (print "Image: ")
	   (println simple-image)
	   (print "\n")

       ; 'print', because there is already trailing new line character
	   (println "Original:")
	   (print (convert-image-to-str simple-image))

	   (println "Inverted:")
	   (print (convert-image-to-str (invert-image simple-image)))

	   (print "\n")
	   (println "Original:")
	   (print (convert-image-to-str some-image))
	   (println "Flipped:")
	   (print (convert-image-to-str (flipH some-image)))
))

(if (eq __name__ __main__) (script_main nil) (nil))
