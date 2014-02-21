(game-module "valhalla-beach"
  (title "Invasion")
  (version ".1")

;; From .1: Glen

  (blurb "A three-pronged amphibious assault.")
  (base-module "valhalla")
  (variants
     (sequential true)
     (world-seen true)
     (see-all false)


("6 Sides" add-pacfleet
	    "All split up." false
	    (true

(set sides-min 6)
(set sides-max 6)


(side 1 inva (name "Invasion Group North") (adjective "Northern Invasion")
)

(side 2 defa (name "Defense Group North") (adjective "Northern Defender")
)

(side 3 invb (name "Invasion Group Central") (adjective "Central Invasion")
)

(side 4 defb (name "Defense Group Central") (adjective "Central Defender")
)

(side 5 invc (name "Invasion Group South") (adjective "Southern Invasion")
)

(side 6 defc (name "Defense Group South") (adjective "Southern Defender")
)



(side 1 (trusts (3 1) (5 1)))
(side 2 (trusts (4 1) (6 1)))
(side 3 (trusts (1 1) (5 1)))
(side 4 (trusts (2 1) (6 1)))
(side 5 (trusts (1 1) (3 1)))
(side 6 (trusts (2 1) (4 1)))

(include "u-val-beach")

		)

	   (false
(set sides-min 2)
(set sides-max 2)


(side 1 inv (name "Invasion Group") (adjective "Invasion")
)

(side 2 def (name "Defense Group") (adjective "Defender")
)

(include "u-val-beach-2")

		)
        )

;; SIXER




)
  (instructions (
   "Invading player must control at least six Victory Points by turn 20."
   "After that, control of ten Victory Points gives victory."
   "Recon flights are no different than normal Fighters."

   ))
  )



(add hanger see-always false) 

;;; Define basic terrain.

(include "t-val-beach")

(set synthesis-methods '(name-units-randomly))

(scorekeeper 
   (title "Test Baby")
   (when (after-turn 20))
   (do (if (>= (sum-uprop active-vp point-value) 250) win))
)

(set scorefile-name "Test Baby")



