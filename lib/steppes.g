(game-module "steppes"
  (title "Steppes")
  (version "1.0")
  (blurb "The wide open spaces. Based on the standard game.")
  (base-module "standard")
  (variants (see-all false) (world-seen true) (sequential false))
  )

(add t* country-terrain-min 0)
(add t* country-terrain-max 1000)

;;; Uniform land (matches anything with a type called "plains").

(area 60 30)

(area (terrain (by-name "+" plains)
  "60+"
  "60+"
  "60+"
  "30+a29+"  ; works around a bug in AI code?
  "60+"
  "60+"
  "60+"
  "60+"
  "60+"
  "60+"
  "60+"
  "60+"
  "60+"
  "60+"
  "60+"
  "60+"
  "60+"
  "60+"
  "60+"
  "60+"
  "60+"
  "60+"
  "60+"
  "60+"
  "60+"
  "60+"
  "60+"
  "60+"
  "60+"
  "60+"
  ))
