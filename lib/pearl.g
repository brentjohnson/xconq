(game-module "pearl"
  (title "Pearl Harbor")
  (version "1.0")
  (blurb "Japan falls upon Pearl Harbor unexpectedly.")
  (picture-name "pearl-harbor")
  (base-module "ww2-div-pac")
  (variants 
   (world-seen true)
   (see-all true)
   (sequential false))
  )

(game-module (instructions (
  "This module is an introduction to the division/theater-level"
  "Pacific game.  The Japanese fall upon Pearl Harbor and sink"
  "as many ships as possible, while the Americans sit there and"
  "take it."
  )))

(set sides-min 3)
(set sides-max 3)

(side 3 (active false))

(set initial-date "7 Dec 1941")

(scorekeeper (do last-side-wins))

; (set last-turn 2)

(set synthesis-methods nil)

;; (should use restriction of Pacific map)

(area 30 20 (terrain
  "30a"
  "30a"
  "30a"
  "30a"
  "30a"
  "30a"
  "30a"
  "30a"
  "30a"
  "9ac20a"
  "13a2c15a"
  "16ac13a"
  "18ac11a"
  "20ac9a"
  "20a2c8a"
  "21ac8a"
  "30a"
  "30a"
  "30a"
  "30a"
  ))

(town 14 9 1 (n "Honolulu") (sym honolulu))
(airbase 14 9 1 (in honolulu))
(bb 14 9 1 (n "Arizona") (in honolulu) (acp 0))
(bb 14 9 1 (n "California") (in honolulu) (acp 0))
(bb 14 9 1 (n "Maryland") (in honolulu) (acp 0))
(bb 14 9 1 (n "Nevada") (in honolulu) (acp 0))
(bb 14 9 1 (n "Oklahoma") (in honolulu) (acp 0))
(bb 14 9 1 (n "Pennsylvania") (in honolulu) (acp 0))
(bb 14 9 1 (n "Tennessee") (in honolulu) (acp 0))
(bb 14 9 1 (n "West Virginia") (in honolulu) (acp 0))

(ca 8 18 2 (n "Tone"))
(dd 8 18 2 (n "Kagero"))
(cv 8 18 2 (n "Akagi"))
(cv 8 18 2 (n "Hiryu"))
(cv 8 18 2 (n "Kaga"))
(cv 8 18 2 (n "Shokaku"))
(cv 8 18 2 (n "Soryu"))
(cv 8 18 2 (n "Zuikaku"))
(bb 8 18 2 (n "Hiei"))
(bb 8 18 2 (n "Kirishima"))
