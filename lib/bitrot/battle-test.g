(game-module "battle-test"
  (title "Star Fleet Battles Test")
  (version "1.0")



  (blurb "Just a testbed, okay.")
  (base-module "battles")

  (variants
     (sequential true)
     (world-seen true)
     (see-all false)

("Fleet Action" fleet-action
	    "Each Player has two heavy and four light cruisers." true
	    (true
		 (include "u-battles-2")
		 (add ship-types already-seen 50)

		)
	    (false
		 (include "u-battles")
		)
        )

;; FLEET ACTION

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
  (name "The Klingon Empire") (noun "Klingon") (adjective "Klingon") (class "klingon")
  (emblem-name "spec-blank")
)

(side 4 kha 
  (name "Khan") (noun "Eugenicist") (adjective "Eugenic") (class "khan")
  (emblem-name "spec-blank")
)


  (add u* start-with 0)
  (include "t-battles")



