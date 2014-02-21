;;; Start with the simple game in the library.

;;; (should add a variant that controls the condition)

(include "simple")

(if 1 x)

(unit-type dragon)

(end-if x)

(area 20 20)

(area (terrain (by-name "+" plains)
  "20+"
  "20+"
  "20+"
  "20+"
  "20+"
  "20+"
  "20+"
  "20+"
  "20+"
  "20+"
  "20+"
  "20+"
  "20+"
  "20+"
  "20+"
  "20+"
  "20+"
  "20+"
  "20+"
  "20+"
))

(set see-all true)

(side 1)

(human 10 10 1)

(if 0 xxx)

(human 11 11 1)

(else xxx)

(dragon 11 11 1)

(end-if xxx)

(human 11 12 1)
