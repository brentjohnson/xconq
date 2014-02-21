;; This is a test of transport construction.

;(include "standard")
;(add aircraft acp-per-turn 0)
;(add aircraft speed 0)

(include "stdunit")
;(define plains land)

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
;(troop-transport 11 19 1)

(city 9 14 1)

;; Two bases, one for each side.

(base 5 15 1)

(base 25 15 2)

(set synthesis-methods ())

;;; This motivates the AI to go chasing after the destination.

(scorekeeper (do last-side-wins))

(add base point-value 1)
