(game-module "omaha"
  (title "Omaha Beach Landings")
  (version "0.1")
  (blurb "Allied Omaha Beach landing in Normandy on D-Day.")
  (base-module "ww2-bn")
  (variants
   (world-seen true)
   (see-all false)
   (sequential true)
   )
  (instructions (
   "Don't try to play this, it's not finished yet."
   ))
  )

(scorekeeper (do last-side-wins))

(set initial-date "0:00 6 Jun 1944")

(set initial-day-part 0.50)

(set last-turn 9)

(add u* start-with 0)

(side 1 (name "Allies") (adjective "Allied") (class "allied")
   (color "blue") (emblem-name "white-star"))

(side 2 (name "Germany") (adjective "German") (class "german")
   (color "black") (emblem-name "german-cross"))

(set sides-min 2)
(set sides-max 2)

(area 40 19 (restrict 186 159 55 80))

(include "t-normandy") ; has to go here so towns get on German side

(unit-defaults (s 1))

(landing-ship 90 90 (sym ls1))
(landing-ship 89 91 (sym ls2))
(landing-ship 88 92 (sym ls3))

(inf-mot 90 90 (in ls1))
(inf-mot 89 91 (in ls2))
(inf-mot 88 92 (in ls3))
