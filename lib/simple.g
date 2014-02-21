(game-module "simple"
  (blurb "trivial game")
  )

;;; This game definition is about as simple as you
;;; can get and still have a working game.

(terrain-type plains (char "+"))

(unit-type human (image-name "person") (char "@")
  (start-with 1)
  (acp-per-turn 4)
  )
