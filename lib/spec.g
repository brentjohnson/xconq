(game-module "spec"
  (title "Specula")
  (version "1.0")
  (blurb "Fanatasy game by Elijah Meeks.")
  (variants
   (see-all false)
   (world-seen true)
   (sequential true)
;;   (real-time true)
   ("Alternate economy" model1
    "Use a different algorithm for moving materials among units."
    (true (set backdrop-model 1)))
   ("AltEcon uses doctrine" model1doctrine
    "When using alternate economy, don't drain units below doctrine levels."
    (true (set backdrop-ignore-doctrine 0))
    (false (set backdrop-ignore-doctrine 1)))
   )
  (instructions (
   "The research is set up like Master of Magic, but you can always research new books. They're expensive, much more expensive than spells. The AI only likes summoning spells."
   ))
  )

(include "spec-rules")

;;; Define basic terrain.

(include "opal")

(include "spec-sides")

(include "spec-units")
