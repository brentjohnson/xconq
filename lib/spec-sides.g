(game-module "spec-sides"
  (default-base-module "spec-rules")
  )


(set sides-min 15)
(set sides-max 15)


;;; MATERIALS: Production, Tech, Fuel, Food, Machine Parts, Gear, Ammo
;;; ADVANCES: Smooth, Rifled, Repeating, Bolt-Action, Wooden, Iron1, Iron2, Iron3, Iron4, Iron5, AC1, AC2,
;;; 	        Light1, Light2, Light3, Medium1, Medium2, Medium3, Heavy1, Heavy2, Heavy3

;;  (advances-done 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0)
;;  (current-advance 1)

;; X999 so that the last researched is immediately discovered (otherwise you end up annoying the player)


(side 1 al 
  (name "Aristocracy Loyalists") (noun "Loyalist") (adjective "Loyalist") (class "loyal")
  (emblem-name "spec-al")
  (treasury 100 50 50 50 6500 50)

)

(side 2 ar 
  (name "Aristocracy Rebels") (noun "Rebel") (adjective "Rebellious") (class "rebel")
  (emblem-name "spec-ar")
  (treasury 100 50 50 50 4500 50)
)

(side 3 con 
  (name "Confederacy") (noun "Confederate") (adjective "Confederate") (class "confed")
  (emblem-name "spec-con")
  (treasury 100 50 50 50 6500 50)
;;  (advances-done 0 0 0 0 0 0 0 0 0 0 -2 -2 -2 0 0 -2 -2 0 0 0 0 0 0 0 0 0 0)

)

(side 4 sho 
  (name "Shogunate") (noun "Shogunate") (adjective "Shogunate") (class "shogunate")
  (emblem-name "spec-sho")
  (treasury 100 50 50 50 6500 50)
;;  (advances-done -2 -2 0 0 0 -2 -2 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -2 0)

)

(side 5 oni 
  (name "Oni") (noun "Oni") (adjective "Oni") (class "oni")
  (emblem-name "spec-oni")
  (treasury 100 50 50 50 6500 50)
;;  (advances-done -2 -2 -2 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 -2)
)

(side 6 emp 
  (name "Iriken") (noun "Irikani") (adjective "Iriken") (class "empire")
  (emblem-name "spec-emp")
  (treasury 100 50 50 50 6500 50)
;;  (advances-done -2 -2 0 0 0 -2 -2 -2 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0)
)

(side 7 bed 
  (name "Bedouin") (noun "Bedouin") (adjective "Bedouin") (class "bedouin")
  (emblem-name "spec-bed")
  (treasury 100 0 50 50 6500 50)
;;  (advances-done -2 0 0 0 0 -2 0 0 0 0 -2 0 0 0 0 -2 0 0 0 0 -2 0 0 0 0 0 0)
)

(side 8 cit 
  (name "City-States") (noun "Dwarf") (adjective "Dwarven") (class "dwarf")
  (emblem-name "spec-cit")
  (treasury 100 50 50 50 6500 50)
;;  (advances-done 0 0 0 0 0 -2 0 0 0 0 -2 -2 0 0 0 -2 -2 0 0 0 0 0 0 0 0 -2 0)
)

(side 9 und 
  (name "The Undead") (noun "Undead") (adjective "Undead") (class "undead")
  (emblem-name "spec-und")
  (treasury 100 0 50 50 5000 0)
)

(side 10 liz 
  (name "The Lizardmen") (noun "Lizardman") (adjective "Reptillian") (class "lizard")
  (emblem-name "spec-liz")
  (treasury 100 50 50 50 6500 50)
;;  (advances-done 0 0 0 0 0 0 0 0 0 0 -2 -2 -2 0 0 0 0 0 0 0 0 0 0 0 0 0 -2)
)

(side 11 kra 
  (name "The Kraken") (noun "Kraken") (adjective "Kraken") (class "kraken")
  (emblem-name "spec-kra")
  (treasury 100 0 50 50 5000 0)
)

(side 12 wil 
  (name "The Wilderness") (noun "Beast") (adjective "Wild") (class "wild")
  (emblem-name "spec-blank")
  (treasury 0 0 50 50 3000 0)
)

(side 13 sul 
  (name "The White Sea") (noun "Sea Monster") (adjective "Foul") (class "white")
  (emblem-name "spec-blank")
  (treasury 100 0 50 50 0 0)
)

(side 14 gla 
  (name "The Glass Sea") (noun "Terrible Creature") (adjective "Hideous") (class "glass")
  (emblem-name "spec-blank")
  (treasury 100 0 50 50 0 0)
)

(side 15 dra 
  (name "Dragonkind") (noun "Wyrm") (adjective "Draconian") (class "dragon")
  (emblem-name "spec-blank")
  (treasury 100 0 50 50 3000 0)
)



(side 1 (trusts (3 1) (4 1) (5 1)))
(side 2 (trusts (6 1)))
(side 3 (trusts (1 1) (4 1) (5 1) (8 1)))
(side 4 (trusts (1 1) (3 1) (5 1)))
(side 5 (trusts (1 1) (3 1) (4 1)))
(side 6 (trusts (2 1)))
(side 7 (trusts (1 1) (3 1) (5 1)))
(side 8 (trusts (3 1)))
