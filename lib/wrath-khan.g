(game-module "wrath-khan"
  (title "The Wrath of Khan")
  (version "1.0")



  (blurb "Khan!!  Khaaaaaaaaannnn!!!!!")
  (base-module "battles")

  (variants
     (sequential true)
     (world-seen true)
     (see-all false)

("Outrun Us and Outgun Us" outrun
	    "The Enterprise and Reliant have fought, now the Enterprise must make for the Motari Nebula. (Not working yet)" false
	    (true
		 (include "u-khan")

		)
	    (false
		 (include "u-khan")
		)
        )

;; OUTRUN

)
)

(set sides-min 3)
(set sides-max 3)

(side 1 kha 
  (name "Khan") (noun "Eugenicist") (adjective "Eugenic") (class "khan")
  (emblem-name "spec-blank")
)

(side 2 fed 
  (name "The Federation") (noun "Federation") (adjective "Federation") (class "federation")
  (emblem-name "spec-blank")
)

(side 3 reg 
  (name "Regula") (noun "Regula") (adjective "Abandoned") (class "regula")
  (emblem-name "spec-blank")
)

;; (side 1 (trusts (3 1)))
;; (side 2 (trusts (3 1)))
;; (side 3 (trusts (1 1) (2 1)))

;; (scorekeeper (do last-alliance-wins))


(add u* start-with 0)

(table unit-size-as-occupant
  ;; Disable occupancy by default.
	(u* u* 99)
	(all-systems-types ship-types 2)
	(all-systems-types cloaked-ship-types 2)
	(all-systems-types station-types 2)
	(all-systems-types hulk-types 2)
	(all-systems-types p-hulk-types 2)
	(fedcpc ship-types 1)

)

(add fedstation1 point-value 5)
  (include "t-khan")



