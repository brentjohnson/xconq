(game-module "cil"
  (title "Cast Iron Life")
  (version "0.01")
  (blurb "The Civil War, with the option of going into Guam, and Canada, Cuba, Mexico and the Indian Nations. By Elijah Meeks.")
;  (base-module "cil-rules")
  (variants
   (see-all false)
   (world-seen true)
   (sequential true)
;;   (real-time true)
   )
  (instructions (
   "This is an attempt to model North America circa 1860.  This file was originally the ww2-38.g and requires the cil-rules.g, cil-sides.g, cil.imf, cil-flags16x16.gif, cil-flags8x8.gif, cil.gif and an updated imf.dir that includes reference to the new graphics."
   ))
  )

(include "cil-rules")

;;; Define basic terrain.

(include "noram")

;; We want the rivers.

(include "t-e50-river")


;;; ????????
;;; (set synthesis-methods '(name-units-randomly))


(include "cil-sides")

;;; France is out of the picture now.

;;; (side 2 (active false))

;;; Define the nationalities of the people.???????

;;; (include "p-e1-1938")

;;; Define the cities.

;; (should have more British bases - for instance at Bermuda and in West Indies)

(include "cil-units")



