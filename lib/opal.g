(game-module "opal"
  (title "Opal")
  (version "2.04")
  (blurb "A large fantasy game, similar to Master of Magic.  Many units and a crude spell system.")
  (base-module "opal-rules")
  (variants
   (see-all false)
   (world-seen true)
   (sequential true)

        ("Classic" classic
	    "Predefined map and units, very big and cumbersome and almost impossible to finish." false
	    (true

(set initial-date "Jan 973")

(set sides-min 15)
(set sides-max 15)

(side 1 al 
  (name "Aristocracy Loyalists") (noun "Loyalist") (adjective "Loyalist") (class "loyal")
  (emblem-name "spec-al")
  (treasury 100 50 50 50 3500 50)
  (advances-done 0 0 -2 0 0 0 0 0)

)

(side 2 ar 
  (name "Aristocracy Rebels") (noun "Rebel") (adjective "Rebellious") (class "rebel")
  (emblem-name "spec-ar")
  (advances-done 0 0 0 -2 0 0 0 0)
  (treasury 100 50 50 50 3500 50)
)

(side 3 con 
  (name "Confederacy") (noun "Confederate") (adjective "Confederate") (class "confed")
  (emblem-name "spec-con")
  (treasury 100 50 50 50 3500 50)
;;  (advances-done 0 0 0 0 0 0 0 0 0 0 -2 -2 -2 0 0 -2 -2 0 0 0 0 0 0 0 0 0 0)

)

(side 4 sho 
  (name "Shogunate") (noun "Shogunate") (adjective "Shogunate") (class "shogunate")
  (emblem-name "spec-sho")
  (treasury 100 50 50 50 3500 50)
  (advances-done 0 -2 0 0 0 0 0 0)

)

(side 5 oni 
  (name "Oni") (noun "Oni") (adjective "Oni") (class "oni")
  (emblem-name "spec-oni")
  (treasury 100 50 50 50 3500 50)
  (advances-done 0 0 0 0 0 -2 0 0)
)

(side 6 emp 
  (name "Iriken") (noun "Irikani") (adjective "Iriken") (class "empire")
  (emblem-name "spec-emp")
  (treasury 100 50 50 50 3500 50)
  (advances-done -2 0 0 0 0 0 0 0)
)

(side 7 bed 
  (name "Bedouin") (noun "Bedouin") (adjective "Bedouin") (class "bedouin")
  (emblem-name "spec-bed")
  (treasury 100 0 50 50 3500 50)
  (advances-done 0 0 0 0 0 -2 0 0)
)

(side 8 cit 
  (name "City-States") (noun "Dwarf") (adjective "Dwarven") (class "dwarf")
  (emblem-name "spec-cit")
  (treasury 100 50 50 50 3500 50)
  (advances-done 0 0 0 0 -2 0 0 0)
)

(side 9 und 
  (name "The Undead") (noun "Undead") (adjective "Undead") (class "undead")
  (emblem-name "spec-und")
  (treasury 100 0 50 50 5000 0)
  (advances-done 0 0 0 0 0 0 0 0 0 -2 -2)
)

(side 10 liz 
  (name "The Lizardmen") (noun "Lizardman") (adjective "Reptillian") (class "lizard")
  (emblem-name "spec-liz")
  (treasury 100 50 50 50 3500 50)
  (advances-done 0 0 0 0 0 0 0 -2)

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
  (advances-done 0 0 0 0 0 0 0 0 0 0 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2
    -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2
    -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2)
  (treasury 100 0 5000 5000 3000 0)

)





(side 1 (trusts (3 1) (4 1) (5 1)))
(side 2 (trusts (6 1)))
(side 3 (trusts (1 1) (4 1) (5 1) (8 1)))
(side 4 (trusts (1 1) (3 1) (5 1)))
(side 5 (trusts (1 1) (3 1) (4 1)))
(side 6 (trusts (2 1)))
(side 7 (trusts (1 1) (3 1) (5 1)))
(side 8 (trusts (3 1)))


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

		(include "u-opal")

(add u* start-with 0)

(scorekeeper 8
  (title "Main Players Only")
  (keep-score false)
  (applies-to (not "independent" "glass" "white" "wild" "kraken"))
  (do last-alliance-wins)
)

(set scorefile-name "8")


)


	(false

(set sides-min 2)
(set sides-max 8)

;; X999 so that the last researched is immediately discovered (otherwise you end up annoying the player)
;;  (advances-done -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 0 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 ;; -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 ;; -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2)


(side 1 al 
  (name "Aristocracy Loyalists") (noun "Loyalist") (adjective "Loyalist") (class "loyal")
  (emblem-name "spec-al")
  (treasury 5000 5000 5000 5000 3500 5000)
;;  (advances-done 0 0 -2 0 0 0 0 0)
;;  (advances-done 0 0 -2 0 0 0 0 0 0)
  (advances-done 0 0 0 0 0 0 0 0 0 0 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2
    -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2
    -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2)


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
(add sword start-with 2)
(add halberd start-with 1)
(add bow start-with 1)
(add cavalry start-with 2)
(add city3 start-with 2)
(add mage start-with 2)
(add hero start-with 1)
(add wizard start-with 1)
(add tower start-with 2)
(add scout start-with 3)
(add catapult start-with 1)

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
  (city2 (land steppe forest mountains hills desert) (25 25 25 50 50 50))
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


	   )
	)


        ("Tolkien Style" tolk
	    "Map is presented in a pencil-drawn way reminiscent of J. R. R. Tolkien's maps of Middle-Earth." false
	    (true
                (add sea image-name "tolk-ocean")
                (add shallows image-name "tolk-lake")
                (add swamp image-name "tolk-swamp")
                (add land image-name "tolk-plains")
                (add mountains image-name "tolk-mountains")
                (add hills image-name "tolk-hills")
                (add ice image-name "tolk-ice")
                (add jungle image-name "tolk-jungle")
                (add desert image-name "tolk-dunes")
                (add forest image-name "tolk-forest")
		(add steppe image-name "tolk-steppe")
		(add wasteland image-name "tolk-wasteland")
;;		(add river image-name "tolk-river")

	    )
	 )


;;   (real-time true)
   )
  (instructions (
   "Units gain experience and turn into heroes, rangers, wizards or knights, depending on their unit type.  These units can turn into Epic Heroes which can themselves turn into Legendary Heroes.  Otherwise, it's just your everyday fantasy-genre game of conquering the world."
   ))
  )

;;; Define basic terrain.

