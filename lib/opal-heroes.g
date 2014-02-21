(game-module "opal-heroes"
  (title "Opal Heroic")
  (version "1.0")
  (blurb "Variants focusing on starting with just heroes.")
  (base-module "opal-rules")
  (variants
   (see-all false)
   (world-seen true)
   (sequential true)

        ("Classic" classic
	    "The classic map and sides, except only the nations are true sides and the remaining sides are individual heroes." false
	    (true

(set initial-date "Jan 973")

(set sides-min 15)
(set sides-max 15)


(side 1 ranger1 
  (name "The Ranger") (noun "Ranger") (adjective "Ranger") (class "ranger1")
  (emblem-name "spec-blank")
  (treasury 0 0 0 0 0 0)
)

(side 2 wizard1 
  (name "The Good Wizard") (noun "Wizard") (adjective "Wizardly") (class "wizard1")
  (emblem-name "spec-blank")
  (treasury 0 0 0 0 3500 0)
)

(side 3 hero1 
  (name "The Evil Hero") (noun "Evil Hero") (adjective "Evil Heroic") (class "hero1")
  (emblem-name "spec-blank")
  (treasury 0 0 0 0 0 0)
  (advances-done 0 0 0 -2 0 0 0 0 0 -2 -2)
)

(side 4 hero2 
  (name "The Good Hero") (noun "Good Hero") (adjective "Good Heroic") (class "hero2")
  (emblem-name "spec-blank")
  (treasury 0 0 0 0 0 0)
  (advances-done 0 0 0 0 0 0 0 0)
)

(side 5 knight1 
  (name "The White Knight") (noun "Knight") (adjective "white") (class "knight1")
  (emblem-name "spec-blank")
  (treasury 0 0 0 0 0 0)
  (advances-done 0 0 -2 0 0 0 0 0)
)

(side 6 knight2 
  (name "The Black Knight") (noun "Black Knight") (adjective "Black") (class "knight2")
  (emblem-name "spec-blank")
  (treasury 0 0 0 0 0 0)
  (advances-done 0 0 -2 0 0 0 0 0)
)

(side 7 wizard2 
  (name "The Evil Wizard") (noun "Evil Wizard") (adjective "Evil Wizardly") (class "wizard2")
  (emblem-name "spec-blank")
  (treasury 0 0 0 0 3500 0)
)

(side 8 epic1 
  (name "The Epic Hero") (noun "Epic Hero") (adjective "Epick") (class "epic1")
  (emblem-name "spec-blank")
  (treasury 0 0 0 0 0 0)
)


(side 9 arch1 
  (name "The Archmage") (noun "Archmage") (adjective "Archmagickal") (class "arch1")
  (emblem-name "spec-blank")
  (treasury 0 0 0 0 7000 0)
)

(side 10 al 
  (name "Aristocracy Loyalists") (noun "Loyalist") (adjective "Loyalist") (class "loyal")
  (emblem-name "spec-al")
  (treasury 100 50 50 50 3500 50)
  (advances-done 0 0 -2 0 0 0 0 0)

)

(side 11 ar 
  (name "Aristocracy Rebels") (noun "Rebel") (adjective "Rebellious") (class "rebel")
  (emblem-name "spec-ar")
  (advances-done 0 0 0 -2 0 0 0 0)
  (treasury 100 50 50 50 3500 50)
)

(side 12 con 
  (name "Confederacy") (noun "Confederate") (adjective "Confederate") (class "confed")
  (emblem-name "spec-con")
  (treasury 100 50 50 50 3500 50)
;;  (advances-done 0 0 0 0 0 0 0 0 0 0 -2 -2 -2 0 0 -2 -2 0 0 0 0 0 0 0 0 0 0)

)

(side 13 sho 
  (name "Shogunate") (noun "Shogunate") (adjective "Shogunate") (class "shogunate")
  (emblem-name "spec-sho")
  (treasury 100 50 50 50 3500 50)
  (advances-done 0 -2 0 0 0 0 0 0)

)

(side 14 oni 
  (name "Oni") (noun "Oni") (adjective "Oni") (class "oni")
  (emblem-name "spec-oni")
  (treasury 100 50 50 50 3500 50)
  (advances-done 0 0 0 0 0 -2 0 0)
)

(side 15 emp 
  (name "Iriken") (noun "Irikani") (adjective "Iriken") (class "empire")
  (emblem-name "spec-emp")
  (treasury 100 50 50 50 3500 50)
  (advances-done -2 0 0 0 0 0 0 0)
)






;;(side 10 (trusts (12 1) (4 1) (5 1)))
;;(side 11 (trusts (6 1)))
;;(side 12 (trusts (1 1) (4 1) (5 1)))
;;(side 13 (trusts (1 1) (3 1) (5 1)))
;;(side 14 (trusts (1 1) (3 1) (4 1)))
;;(side 15 (trusts (2 1)))


(table independent-density
  (banditseed (land steppe forest semi-desert) (65 50 50 25))
  (pirateseed (sea shallows) (50 100))
  (orcseed (jungle) (75))
  (fortseed (hills steppe forest semi-desert) (50 25 25 15))
  (dragonseed (mountains hills forest steppe) (30 15 10 5))
  (heroseed (land steppe forest semi-desert desert jungle swamp) (5 5 5 5 5 5 5))
  (archseed (land steppe forest semi-desert desert hills swamp) (8 8 8 8 8 8 8))
  (rangerseed (land steppe forest semi-desert desert jungle swamp) (5 5 5 5 5 5 5))
  (knightseed (land steppe forest semi-desert desert hills swamp) (5 5 5 5 5 5 5))
  (riplegend (hills forest desert mountains swamp jungle) (1 1 1 1 1 1))

)

		(include "t-opal")

		(include "u-opal-heroes")

(add u* start-with 0)

(scorekeeper noindy
  (title "Monsters Don't Win")
  (keep-score false)
  (applies-to (not "independent"))
  (do last-alliance-wins)
)

(set scorefile-name "noindy")


)


	(false

(set sides-min 2)
(set sides-max 8)

;; X999 so that the last researched is immediately discovered (otherwise you end up annoying the player)

(side 0
(advances-done -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 0 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2
-2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2)
)

(side 1 al 
  (name "Aristocracy Loyalists") (noun "Loyalist") (adjective "Loyalist") (class "loyal")
  (emblem-name "spec-al")
  (treasury 100 100 100 100 3500 100)
;;  (advances-done 0 0 -2 0 0 0 0 0)
  (advances-done 0 0 -2 0 0 0 0 0 0)

;;  (advances-done 0 0 0 0 0 0 0 0 0 0 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2
;;    -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2
;;    -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2)


)


(side 2 ar 
  (name "Aristocracy Rebels") (noun "Rebel") (adjective "Rebellious") (class "rebel")
  (emblem-name "spec-ar")
;;  (advances-done 0 0 0 -2 0 0 0 0)
  (treasury 100 100 100 100 3500 100)
  (advances-done 0 0 0 -2 0 0 0 0 0 0)
)


(side 3 con 
  (name "Confederacy") (noun "Confederate") (adjective "Confederate") (class "confed")
  (emblem-name "spec-con")
  (treasury 100 100 100 100 3500 100)

  (advances-done 0 0 0 0 0 0 0 0 0 0)
)

(side 4 sho 
  (name "Shogunate") (noun "Shogunate") (adjective "Shogunate") (class "shogunate")
  (emblem-name "spec-sho")
  (treasury 100 100 100 100 3500 100)
;;  (advances-done 0 -2 0 0 0 0 0 0)
  (advances-done 0 -2 0 0 0 0 0 0 0 0)
)

(side 5 oni 
  (name "Oni") (noun "Oni") (adjective "Oni") (class "oni")
  (emblem-name "spec-oni")
  (treasury 100 100 100 100 3500 100)
;;  (advances-done 0 0 0 0 0 -2 0 0)
  (advances-done 0 0 0 0 0 0 0 -2)
)

(side 6 emp 
  (name "Iriken") (noun "Irikani") (adjective "Iriken") (class "empire")
  (emblem-name "spec-emp")
  (treasury 100 100 100 100 3500 100)
;;  (advances-done -2 0 0 0 0 0 0 0)
  (advances-done -2 0 0 0 0 0 0 0 0)
)

(side 7 bed 
  (name "Bedouin") (noun "Bedouin") (adjective "Bedouin") (class "bedouin")
  (emblem-name "spec-bed")
  (treasury 100 100 100 100 3500 100)
;;  (advances-done 0 0 0 0 0 -2 0 0)
  (advances-done 0 0 0 0 0 -2 0 0 0 0 0)
)

(side 8 cit 
  (name "City-States") (noun "Dwarf") (adjective "Dwarven") (class "dwarf")
  (emblem-name "spec-cit")
  (treasury 100 100 100 100 3500 100)
;;  (advances-done 0 0 0 0 -2 0 0 0)
  (advances-done 0 0 0 0 -2 0 0 0 0)
)

(set country-radius-max 40)
(set country-separation-min 10)
(set edge-terrain ice)

(add u* start-with 0)
(add hero start-with 1)
(add wizard start-with 1)
(add ranger start-with 1)
(add knight start-with 1)

(table independent-density
  (faultline (land steppe hills mountains) (25 25 25 25))
  (banditseed (land steppe forest semi-desert) (65 50 50 25))
  (fortseed (hills steppe forest semi-desert) (50 25 25 15))
  (dragonseed (mountains hills forest steppe) (30 15 10 5))
  (heroseed (land steppe forest semi-desert desert jungle swamp) (5 5 5 5 5 5 5))
  (archseed (land steppe forest semi-desert desert hills swamp) (8 8 8 8 8 8 8))
  (rangerseed (land steppe forest semi-desert desert jungle swamp) (5 5 5 5 5 5 5))
  (knightseed (land steppe forest semi-desert desert hills swamp) (5 5 5 5 5 5 5))

  (riplegend (hills forest desert mountains swamp jungle) (1 1 1 1 1 1))

  (city1 (land steppe forest mountains hills desert) (50 50 50 25 25 25))
  (city2 (land steppe forest mountains hills desert) (50 50 50 25 25 25))
  (city3 (land steppe forest mountains hills desert) (15 15 15 15 15 15))
  (city4 (land steppe forest mountains hills desert) (25 25 25 25 25 25))
  (ohamlet (land steppe forest mountains hills desert) (15 15 15 15 15 15))
  (ovillage (land steppe forest mountains hills desert) (15 15 15 15 15 15))
  (citadel (land steppe forest mountains hills desert) (15 15 15 15 15 15))
  (lab (land steppe forest mountains hills desert) (20 20 20 20 20 20))

  (enode mountains 75)
  (cnode desert 75)
  (snode steppe 75)
  (lnode land 75)
  (dnode (forest hills swamp) (75 75 75))
  (temple (mountains desert steppe forest hills swamp) (20 20 20 20 20 20))

  (goldmine mountains 150)
  (ironmine mountains 150)
  (quarry (mountains hills) (150 150))
  (lumber forest 150)

  (deadcity (land steppe forest mountains hills desert) (8 8 8 8 8 8))
  (dino1 (land steppe forest mountains hills) (7 7 7 7 7))
  (dino2 (land steppe forest mountains hills) (7 7 7 7 7))
  (fheart forest 10)
  (jheart forest 10)
  (lair mountains 8)
  (pit sea 8)

  (thurastes sea 3)
  (squid sea 8)
  (dajawyrm sea 8)
  (dajaturtle sea 4)
  (gibambi sea 4)
  (churambi sea 4)
  (ulitar sea 4)
  (plesiosaur sea 4)
  (leviathan sea 4)
  (mold sea 4)
  (sludge sea 4)
  (tentacle sea 5)
  
  (zombie (desert forest) (30 30))
  (spectre (desert forest) (20 20))
  (vampyre (desert forest) (8 8))
  (lich (desert forest) (5 5))

  (iblis desert 3)
  (chaoscreature desert 3)
  (demon desert 8)

  (phantombeast steppe 4)
  (phantomwarrior steppe 8)

  (bears forest 60)
  (wolves forest 60)
  (tribesman forest 20)
  (ferak forest 60)
  (royalferak forest 60)


  (tyrannosaur (forest swamp) (5 10))
  (apatosaur (forest swamp) (7 12))
  (pterodactyl (forest swamp) (10 15))

)


(table favored-terrain
  (land-types land-t* 100)
  (deep-types sea 100)
  (amphibious-types sea 100)
  (water-types sea 100)
  (air-types t* 100)

	)


	   )
	)



;;   (real-time true)
   )
  (instructions (
   "Quick, conquer the cities around you and build an army!"
   ))
  )

;;; Define basic terrain.

