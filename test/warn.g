;;; This file should tweak all the warnings in the kernel,
;;; but not cause any fatal errors.

(game-module "xyz"
  (variants
   a-valid-variant
   304 ; a bogus variant
   (201) ; another bogus variant
   (another-valid-variant 5)
   )
  )

;;; Warn about setting a non-symbol.

(set 45 92)

;;; Warn about attempted modification of a constant.

(set true false)

;;; Warn about an undefined table.

(table foo)

;;; Warn about too long of a string.

(define str2 "
0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789
")

(print str2)

;;; Warn about unbound symbol.

(if expr1)

(end-if)

;;; Warn about invalid dice specs.

(define dice1 8d4+2)
(define dice2 4d16+8)
(define dice3 3d4+200)

;;; The following definitions give us some types to work with,
;;; they prevent Xconq from erroring out completely, and they
;;; prevent attempts to load a default game, which can happen
;;; as part of trying to make sense of some top-level forms.

(unit-type u1)
(unit-type u2)
(unit-type u3)

(terrain-type t1)
(terrain-type t2)

;;; Nonsensical top-level forms.

12345

(burp)

;;; Including non-modules.

(include)

(include 0)

;;; Warn about basic syntax of property add.

(add)
(add u1)
(add u1 4)

;;; Warn about non-matching lists.

(add u* speed (0 100))

;;; Warn about trying to fill an empty list.

(define xxx nil)

(add xxx speed nil)

;;; Warn about incorrect table fill-in.

(table acp-to-develop
  (u* (u1 u2) (1 2 3 4 5 6))
  )

(table acp-to-toolup
  (u* (u1 u2) ((1 2) (3 4) (5 6)))
  )

(table acp-to-create ("string" 56 89))

(table acp-to-create (-3 56 89))

(table acp-to-create (u1 -3 89))

(table acp-to-create (u1 u2 "foo"))

;;; Warn about setting a non-symbol.

(set 0 2)

;;; Warn about overwriting an existing definition.

(define u1 u2)

(set synthesis-methods nil)

(area 10 5 (terrain
  "10a"
  "10b"
  "4a2c4b"
  "10b"
  "10a"
))

(side 1
  ;; Warn about too many numbers in the list (short array reader).
  (gain-counts 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
	       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
	       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9)
  ;; Warn about too many numbers in the list (long array reader).
  (attack-stats (u2 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9))
  )

;;; Warn about namer syntax.

(set synthesis-methods '(name-units-randomly))

(add u1 namer "not-a-namer")
(add u3 namer "bad-names")

(namer bad-names (grammar root 9
  (root (or ((foo) hey there) (hey there real real)))
  (hey (reject))
  (there (capitalize))
  (real (reject (or a b c d e f) a b c d e f))
  )))

(u1 3 2 0)
(u3 4 2 0)
(u3 5 2 0)
(u3 6 2 0)
(u3 7 2 0)

;;; Warn about unmatched parens.

)))

;;; Warn about unclosed parens.

(a b c (d e

;;; Warn about unclosed comment.  (This needs to be the last thing
;;; in the file.)

#|
