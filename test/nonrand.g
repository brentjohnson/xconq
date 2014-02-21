(game-module "nonrand"
  (title "Non-Random")
  (blurb "This is a special period that has no randomness at all.")
)

(unit-type bm (name "Blanc-Mange") (image-name "mound")
)

(unit-type kp (name "killer pancake") (image-name "saucerpad")
)

(terrain-type |outer space| (color "navy blue"))

(add u* acp-per-turn 1)

(add u* speed 1)

;;; Hits always occur.

(table hit-chance
  (u* u* 100)
)

(table damage
  (u* u* 1)
)

(set see-all true)

(add u* see-always true) ; redundant

(area 30 30)

(area (terrain
  "30a"
  "30a"
  "30a"
  "30a"
  "30a"
  "30a"
  "30a"
  "30a"
  "30a"
  "30a"
  "30a"
  "30a"
  "30a"
  "30a"
  "30a"
  "30a"
  "30a"
  "30a"
  "30a"
  "30a"
))

(side (name "BM"))

(side (name "KP"))

(bm 10 10 1)

(kp 12 12 2)
