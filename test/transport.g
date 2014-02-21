;; This is a test of transport usage.

(include "standard")

(set see-all true)

(area 30 30)

(area (terrain (by-name ".+" sea plains)
  "10+10.10+"
  "10+10.10+"
  "10+10.10+"
  "10+10.10+"
  "10+10.10+"
  "10+10.10+"
  "10+10.10+"
  "10+10.10+"
  "10+10.10+"
  "10+10.10+"
  "10+10.10+"
  "10+10.10+"
  "10+10.10+"
  "10+10.10+"
  "10+10.10+"
  "10+10.10+"
  "10+10.10+"
  "10+10.10+"
  "10+10.10+"
  "10+10.10+"
  "10+10.10+"
  "10+10.10+"
  "10+10.10+"
  "10+10.10+"
  "10+10.10+"
  "10+10.10+"
  "10+10.10+"
  "10+10.10+"
  "10+10.10+"
  "10+10.10+"
))

;; Limit to two players only.

(set sides-min 2)
(set sides-max 2)

;; Two sides, no special properties needed (?).

(side 1)

(side 2)

;;; (no player for one side?)

(infantry 6 15 1)
;(infantry 6 16 1)
(troop-transport 11 19 1)

;; We should be able to do in 20 turns, but be generous.

(set last-turn 70)

;(set ai-badtask-remove-chance 1)
;(set ai-badtask-max-retries 20)

;; Two bases, one for each side.

(base 5 15 1)

(base 25 15 2)

(set synthesis-methods ())

(add base point-value 1)
