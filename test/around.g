;; This is a test of going around.

(include "standard")

(set see-all true)

(area 30 30)

(area (terrain
  (by-name
    (sea 0) (shallows 1) (swamp 2) (desert 3) (plains 4)
    (forest 5) (mountains 6) (ice 7) (road 8) (river 9))
  "10e20a"
  "10e20a"
  "10e20a"
  "10e20a"
  "10e20a"
  "10ea2e7ae9a"
  "10e3ae6a2e8a"
  "10e4ae5a3e7a"
  "10eae3ae4a4e6a"
  "10e2ae3ae3a5e5a"
  "10e3ae2ae3a6e4a"
  "11e2ae2ae3a7e3a"
  "a3e3a3ea3e2ae3a8e2a"
  "2a5ea2e6ae3a9ea"
  "3a5eae6ae3a10e"
  "4a4eae6ae3a10e"
  "5ae3ae6ae3a10e"
  "6a4e6ae3a10e"
  "7a3e6ae3a10e"
  "16ae3a10e"
  "16ae3a10e"
  "16ae3a10e"
  "16ae3a10e"
  "11ae4ae3a10e"
  "12ae3ae3a10e"
  "13a4e3a10e"
  "20a10e"
  "20a10e"
  "20a10e"
  "20a10e"
))

;; Limit to two players only.

(set sides-min 2)
(set sides-max 2)

;; Two sides, no special properties needed (?).

(side 1)

(side 2)

;;; (no player for one side?)

(infantry 6 15 1)
(troop-transport 11 19 1)

;; Two bases, one for each side.

(base 5 15 1)

(base 25 15 2)

(set synthesis-methods ())

;;; This motivates the AI to go chasing after the destination.

;(scorekeeper (do last-side-wins))

(add base point-value 1)
