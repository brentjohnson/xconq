(game-module "tiny"
  (blurb "A very small world")
  (notes "This is useful for debugging graphics stuff")
  (base-module "standard")
  )

(add town start-with 1)

(set sides-min 1)
(set sides-max 1)

(side 1)

(world (circumference 5))

;; The two middle rows are the interior cells; the standard game's
;; country placement needs at least 4 plains and 1 sea among them.
(area 5 4 (terrain
  "abcde"
  "aeeed"
  "aeecd"
  "abcde"
))
  