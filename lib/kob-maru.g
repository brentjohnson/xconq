(game-module "kob-maru"
  (title "The Kobayashi Maru")
  (version "1.0")



  (blurb "You can't win.")
  (base-module "battles")

  (variants
     (sequential true)
     (world-seen true)
     (see-all false)

("Vaguely Winnable" vague
	    "Only three Klingon D7-class cruisers." true
	    (true
		 (include "u-kmaru")
;;		 (add ship-types already-seen 50)

		)
	    (false
		 (include "u-kmaru2")
		)
        )

;; FLEET ACTION

)
)

(set sides-min 2)
(set sides-max 2)

(side 1 fed 
  (name "The Federation") (noun "Human") (adjective "Federation") (class "federation")
  (emblem-name "spec-blank")
)

(side 2 kli 
  (name "The Klingon Empire") (noun "Klingon") (adjective "Klingon") (class "klingon")
  (emblem-name "spec-blank")
)


(add u* start-with 0)

  (include "t-battles")



