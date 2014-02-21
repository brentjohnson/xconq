(game-module "cil-sides"
  (default-base-module "cil-rules")
  )

(set sides-min 7)
(set sides-max 7)

;;; All the participants and major neutrals.
;;; (should include all the "minor powers" eventually)

;;; MATERIALS: Production, Tech, Fuel, Food, Machine Parts, Gear, Ammo
;;; ADVANCES: Smooth, Rifled, Repeating, Bolt-Action, Wooden, Iron1, Iron2, Iron3, Iron4, Iron5, AC1, AC2,
;;; 	        Light1, Light2, Light3, Medium1, Medium2, Medium3, Heavy1, Heavy2, Heavy3

(side 1 us 
  (name "USA") (noun "Union") (adjective "Union")
  (emblem-name "cil-flag-usa")
;; For Testing
;;  (advances-done -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2)
;; For Real
  (advances-done -2 -2 0 0 -2 0 0 0 0 0 0 0 -2 0 0 -2 0 0 -2 0 0)
;;  (current-advance 12)
  (treasury 0 1000 400 1000 200 500 500 1000)
)

(side 2 cs (name "CSA") (adjective "Confederate") (adjective "Confederate")
  (emblem-name "cil-flag-csa")
  (advances-done -2 -2 0 0 -2 0 0 0 0 0 0 0 -2 0 0 -2 0 0 -2 0 0)
;;  (current-advance 12)
  (treasury 0 1000 300 1000 100 500 500 800)
)

(side 3 mx (name "French Mexico") (noun "Mexican") (adjective "Mexican")
  (emblem-name "cil-flag-mex")
  (advances-done -2 0 0 0 -2 0 0 0 0 0 0 0 -2 0 0 -2 0 0 0 0 0)
;;  (current-advance 12)
  (treasury 0 200 200 200 200 200 200 400)
)

(side 4 ca (name "British N. America") (noun "Canuck") (adjective "Canadian")
  (emblem-name "cil-flag-can")
  (advances-done -2 -2 0 0 -2 0 0 0 0 0 0 0 0 0 0 0 0 0 -2 0 0)
;;  (current-advance 1)
  (treasury 0 300 300 300 300 300 300 400)
)

(side 5 sx (name "Sioux") (adjective "Sioux")
  (emblem-name "cil-flag-sioux")
  (advances-done 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0)
;;  (current-advance 1)
  (treasury 0 0 0 0 0 50 100 100)
)

(side 6 cm (name "Commanche") (adjective "Commanche")
  (emblem-name "cil-flag-commanche")
  (advances-done 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0)
;;  (current-advance 1)
  (treasury 0 0 0 0 0 50 100 100)
)

(side 7 ap (name "Apache") (adjective "Apache")
  (emblem-name "cil-flag-apache")
  (advances-done 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0)
;;  (current-advance 1)
  (treasury 0 0 0 0 0 50 100 100)
)

