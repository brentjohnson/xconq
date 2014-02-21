(game-module "conquest"
  (title "SFB Conquest")
  (version "1.0")



  (blurb "Territorial control.")
  (base-module "battles")

  (variants
     (sequential true)
     (world-seen true)
     (see-all false)

("Khan Map" khan-map
	    "Play on the map for The Wrath of Khan." false
	    (true
		 (include "t-khan")

		)
	    (false
		  (include "t-battles")
		)
        )

;; KHAN MAP

("New Ships" builders
	    "Research Stations are equipped with construction bays to create new starships." false
	    (true
		 (add constructbay start-with 1)

		)
	    (false
		)
        )

;; BUILDERS

("Fast Construction" extra
	    "Construction bays produce at double the rate." false
	    (true
		(table base-production
			(damagecontrol repair 5)
			(damdamagecontrol repair 3)
			(kodamagecontrol repair 2)

		;;	(ship-types si 5)

			(fedcpc online 0)

			(oberth online 2)

			(constructbay construction 20)
		)


		)
	    (false
		)
        )

;; EXTRA

("Facility and Colonizer Test" faccol
	    "True: Fac-only.  False: fac and col for hulls" true
	    (true
		(add all-systems-types facility true)
		(set ai-badtask-max-retries 5)
		(set ai-badtask-remove-chance 100)
		(add constructbay minimal-sea-for-docks 0)


		)
	    (false
		(add all-systems-types facility true)
		(add hull-types colonizer true)

		)
        )

;; FACCOL


)
)

(set sides-min 2)
(set sides-max 4)

(side 1 fed 
  (name "The Federation") (noun "Human") (adjective "Federation") (class "federation")
  (emblem-name "spec-blank")
)

(side 2 emp 
  (name "The Terran Empire") (noun "Barbarian") (adjective "Imperial") (class "empire")
  (emblem-name "spec-blank")
)

(side 3 tra 
  (name "The Traitors") (noun "Traitor") (adjective "Mutinous") (class "traitor")
  (emblem-name "spec-blank")
)

(side 4 kha 
  (name "Khan") (noun "Eugenicist") (adjective "Eugenic") (class "khan")
  (emblem-name "spec-blank")
)

(table independent-density
  (victorypoint (deepspace nebula) (150 150))
)

;;  (add u* start-with 0)
  (add victorypoint start-with 1)
  (set country-separation-min 20)

;; (scorekeeper 
;;   (title "Test Baby")
;;   (when (after-turn 5))
;;   (do (if (>= (sum-uprop victorypoint point-value) 15) win))
;; )

;; (set scorefile-name "Test Baby")


