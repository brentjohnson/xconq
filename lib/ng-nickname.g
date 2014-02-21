;;; This was too good to pass up...

;;; From: fishkin@parc.xerox.com (Ken Fishkin)
;;; Newsgroups: soc.history
;;; Date: 1 Jul 91 17:37:02 GMT
;;;
;;; For those interested, here are some honest-to-god nicknames given
;;; to Medieval European rulers:

(namer royal-adjective (random
  "Bald" "Beloved" "Black"
  "Conqueror" "Cruel"
  "Elder"
  "Fair" "Fat" "Fowler"
  "Glorius" "Good" "Great"
  "Hardy" "Harefoot" "Headstrong"
  "Just"
  "Lion"
  "Mad" "Martyr"
  "Peaceful"
  "Simple" "Sluggard" "Stammerer" "Slobberer" ; somebody mentioned this one too
  "Tall"
  "Unready"
  "Victorious"
  "Wise"
  "Younger"
))

;;; Example of use.
;;;
;;;  (include "ng-weird")
;;;  (include "ng-nickname")
;;;
;;;  (namer funny-king-names (grammar full-name
;;;    (full-name (first-name " the " royal-adjective))
;;;    (first-name generic-names)))
;;;
;;;  (unit-type king ... (namer "funny-king-names") ...)
;;;
;;; This is most sensible if some unit type represents the ruler of a
;;; side personally.


