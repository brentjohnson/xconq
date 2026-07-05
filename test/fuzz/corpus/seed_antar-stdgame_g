(game-module "antar-stdgame"
  (title "Antarctica")
  (default-base-module "standard")
  (blurb "Something like the standard game, set in Antarctica.")
  (variants
    ("Alternate economy" model1
     "Use a different algorithm for moving materials among units."
     (true (set backdrop-model 1)))
    ("AltEcon uses doctrine" model1doctrine
     "When using alternate economy, don't drain units below doctrine levels."
     (true (set backdrop-ignore-doctrine 0))
     (false (set backdrop-ignore-doctrine 1)))
  )
)

(set see-all true)

(include "antar-stdterr")
(include "antar-map-50km")
