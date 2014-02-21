(game-module "korea-2006"
  (title "Korean War 2006")
  (version "1.14")

;; From .1:
;; Brought version number in line, added supply rules.
;; From 1.11: Added two cities for China and removed an armor group.  The added cities makes the Chinese AI far more aggressive, which is part of the reason why the armor group was pulled out.
;; From 1.11a added the aggressive-china option, which places many more Chinese units in the area.
;; From 1.12 removed ZOC option, since ZOC is now standard
;; Placed the US Pacific Fleet closer, so that the AI uses the Nimitz
;; From 1.14 streamlined side handling


  (blurb "The Second Korean War and the Sino-Japanese Intervention. Near future game by Elijah Meeks.")
  (base-module "awls-rules")
  (variants
     (sequential true)
     (world-seen true)
     (see-all false)

("Game Graphics" game-units
	    "Units are artistic, instead of military symbols." true
	    (true
                (add corps-types image-name "ko-corps-2")
                (add army-types image-name "trident-alpine-troops")
                (add armor-types image-name "trident-armor")
                (add fighter-types image-name "trident-fighter")
                (add tech2-carrier-types image-name "trident-carrier")
                (add tech1-carrier-types image-name "ko-carrier2")
                (add tech0-carrier-types image-name "ko-carrier1")
                (add surface-types image-name "trident-battleship")
                (add squadron-types image-name "ko-advanced-fighter")
                (add coastal image-name "trident-aegis-cruiser")
                (add coastsub image-name "trident-submarine")
                (add specops image-name "trident-paratroopers")
                (add specheli image-name "trident-helicopter")
                (add milair image-name "trident-bomber")
                (add civair image-name "trident-bomber")
                (add spysat image-name "ko-sat")
                (add miltran image-name "trident-transport")
                (add civtran image-name "trident-transport")
                (add para-types image-name "trident-partisans")
                (add adneng image-name "trident-engineers")
                (add adn-types image-name "ba-big-sam")
                (add sub image-name "trident-submarine")
	    	(add brigade-types image-name "ko-advanced-tank")
	    	(add coastsub image-name "ko-coastsub")
		(add carrier-fighter-types image-name "ko-carrier-wing")


            )
        )


("Enable Supply" enable-supply
	    "Units must have minimum amounts of supplies to attack and and move, keeping them close to their cities." true
	    (true
	          	(table material-to-move
			   (u* supply 1)
			   (satellite-types supply 0)
	    	    	)
			
			(table material-to-attack
			   (u* supply 2)
			)

			(table material-to-fire
			   (u* supply 2)
			)

		)
        )

;; SUPPLY


("Three Year War" three-year-war
	    "Imposes a thirty-six turn limit.  After this, the realism of the war seriously breaks down." true
	    (true
	          	(set last-turn 36)
		)
        )


;; THREE YEAR

("Modified AI" different-ai
	    "An attempt to make the AI deal better with these rules." true
	    (true

	          	(set ai-may-resign false)
;;	          	(set units-may-go-into-reserve false)
	          	(set ai-advanced-unit-separation 0)
	          	(set ai-badtask-max-retries 3)
 	          	(set ai-badtask-remove-chance 20)
	          	(set ai-badtask-reserve-chance 6)
;;	          	(set minimal-sea-for-docks 50)





		)
        )

;; AI

("Varyag" add-varyag
	    "Assumes the Chinese manage to get the Russian carrier Varyag up and running." true
	    (true

			(unit-defaults)
			(wreckedcarrier-1 42 51 ch (n "Varyag"))
			(carrierwing 42 51 ch (in "Varyag"))



		)
        )

;; VARYAG

("US Pacific Fleet" add-pacfleet
	    "Gives the US player an aircraft carrier and surface forces from the 3rd Fleet." true
	    (true

			(unit-defaults)
			(reinforcedsurface 81 21 us)
			(reinforcedsurface 80 21 us)
			(restoredcarrier-2 79 21 us (n "Nimitz"))
			(crk-carrierwing 79 21 us (in "Nimitz"))


		)
        )

;; USPAC

("Aggressive China" aggressive-china
	    "Gives the Chinese player a vast array of forces lined up on the China/DPRK border." false
	    (true
		 (include "u-korea-2")


		)
	    (false
		 (include "u-korea")
		)
        )

;; TOUGH CHINA



("No China" remove-china
	    "China provides support for DPRK, but does not take active part in the war."
	    (true
			
			(side 4 (active false))
			(unit-defaults)
			(int-aid 59 58 nk)

;;;;;
(area (aux-terrain peace1-political-border
  "140a"
  "79acy59a"
  "80ahq58a"
  "80agL58a"
  "80agL58a"
  "80acT58a"
  "81ahq57a"
  "81agL57a"
  "81agL57a"
  "80aeoL57a"
  "80agM<57a"
  "79aeoL58a"
  "79agM<58a"
  "78aeoL59a"
  "77aeoM<59a"
  "76aeoM<60a"
  "75aeoM<61a"
  "75agM<62a"
  "72ae2moL63a"
  "68ae3moM2=<63a"
  "67aeoM3=<66a"
  "59ae3mi2aeoM<70a"
  "58aeoM2=?yeoM<71a"
  "57aeoM<3ad:M<72a"
  "57agM<5ab<73a"
  "56aeoL81a"
  "56agM<81a"
  "56agL82a"
  "56agL82a"
  "56agL82a"
  "56acT82a"
  "57ahq81a"
  "57acT81a"
  "58ahq80a"
  "58acT80a"
  "59ahq79a"
  "59agL79a"
  "59agL79a"
  "59agL79a"
  "59agL79a"
  "59acT79a"
  "60ahq78a"
  "60acT78a"
  "61ady77a"
  "62ahq76a"
  "62acT76a"
  "63ahq75a"
  "63acT75a"
  "64ahq74a"
  "64acT74a"
  "65ahq73a"
  "65agL73a"
  "65agL73a"
  "64aeoL73a"
  "64agM<73a"
  "63aeoL74a"
  "63agM<74a"
  "62aeoL75a"
  "62agM<75a"
  "62agL76a"
  "62agL76a"
  "62agL76a"
  "61aeoL76a"
  "60aeoM<76a"
  "60agM<77a"
  "60agL78a"
  "59aeoL78a"
  "58aeoM<78a"
  "57aeoM<79a"
  "56aeoM<80a"
  "32a16mi7agM<81a"
  "32ab15=?y4aemoL82a"
  "49ady2aeoM=<82a"
  "50ad}moM<84a"
  "51ab2=<85a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  ))
)
)

;; NO CHINA


("No Japan" remove-japan
	    "Japan provides support for ROK, but does not take active part in the war." true
	    (true


			(side 5 (active false))
			(unit-defaults)
			(int-aid 71 43 sk)


(area (aux-terrain peace2-political-border
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "87ae4mi47a"
  "85aemoM3=?}mi44a"
  "85agM=<4ab=?y43a"
  "85agL7aempq42a"
  "85agL7agM=<42a"
  "85acT7agL44a"
  "86ahq6agL44a"
  "86acT6agL44a"
  "87ahq5agL44a"
  "87agL5agL44a"
  "86aeoL5acT44a"
  "86agM<6ahq43a"
  "85aeoL7agL43a"
  "84aeoM<7agL43a"
  "84agM<8agL43a"
  "84agL9agL43a"
  "84acT9agL43a"
  "84aepq8acT43a"
  "83aeoM<9ahq42a"
  "77ae5moM<10acT42a"
  "75aemoM5=<12ahq41a"
  "74aeoM=<18agL41a"
  "74agM<20agL41a"
  "74agL14ae6moL41a"
  "74agL13aeoM6=<41a"
  "73aeoL13agM<48a"
  "73agM<13agL49a"
  "73agL12aemoL49a"
  "73agL11aeoM=<49a"
  "73acT4aei4aeoM<51a"
  "74adyaemoO}3moM<52a"
  "75ad}oM=<b4=<53a"
  "76ab=<61a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  "140a"
  ))

		)
        )

;; NO JAPAN


)
  (instructions (
   "Escalating tension on the Korean peninsula erupts in war, drawing in all the major powers but Russia."
   ))
  )


(set sides-min 2)
(set sides-max 6)

;;  ALL BUT SPECOPS (OLD GAME)
;;  (advances-done -2 -2 -2 -2 -2 -2 -2 0 0 0 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2)



(side 1 nk (name "North Korea") (adjective "North Korean")
  (treasury 300 7 30 250)
  (emblem-name "flag-north-korea")
  (advances-done -2 0 0 -2 -2 -2 0 -2 -2 -2 -2 -2 -2 -2 -2 -2 0 0 -2 -2 -2  )
;;  (current-advance 8)
)
(side 2 us (name "USA") (adjective "American")
  (treasury 500 100 40 250)
  (emblem-name "flag-usa")
  (advances-done -2 0 0 -2 -2 -2 0 -2 -2 -2 -2 -2 -2 -2 -2 -2 0 0 -2 -2 -2  )
;;  (current-advance 8)
)
(side 3 sk (name "South Korea") (adjective "South Korean")
  (treasury 400 50 50 250)
  (emblem-name "flag-korea")
  (advances-done -2 0 0 -2 -2 -2 0 -2 -2 -2 -2 -2 -2 -2 -2 -2 0 0 -2 -2 -2  )
;;  (current-advance 8)
)

(side 4 ch (name "China") (noun "Chinese") (adjective "Chinese")
  (treasury 400 50 100 250)
  (emblem-name "flag-china")
  (advances-done -2 0 0 -2 -2 -2 0 -2 -2 -2 -2 -2 -2 -2 -2 -2 0 0 -2 -2 -2  )
;;  (current-advance 8)
)

(side 5 jp (name "Japan") (adjective "Japanese")
  (treasury 500 250 15 500)
  (emblem-name "flag-japan")
  (advances-done -2 0 0 -2 -2 -2 0 -2 -2 -2 -2 -2 -2 -2 -2 -2 0 0 -2 -2 -2  )
;;  (current-advance 8)
)

(side 6 ru (name "Russia") (adjective "Russian")
  (treasury 800 0 80 50)
  (emblem-name "flag-russia")
  (advances-done -2 0 0 -2 -2 -2 0 -2 -2 -2 -2 -2 -2 -2 -2 -2 0 0 -2 -2 -2  )
;;  (current-advance 8)
)

(side 1 (trusts (4 1)))
(side 2 (trusts (3 1)(5 1)))
(side 3 (trusts (2 1)(5 1)))
(side 4 (trusts (1 1)))
(side 5 (trusts (2 1)(3 1)))

(side 1 (advances-done -2 0 0 0 0 0 0 0 0 0 -2 -2 -2 -2 -2 -2 0 0 -2 -2 -2))
(side 2 (advances-done -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2))
(side 3 (advances-done -2 0 0 0 -2 -2 0 0 0 0 -2 -2 -2 -2 -2 -2 -2 0 -2 -2 -2))
(side 4 (advances-done -2 0 0 -2 -2 -2 0 0 0 0 -2 -2 -2 -2 -2 0 0 -2  -2 -2 -2))
(side 5 (advances-done -2 0 0 0 -2 -2 0 0 0 0 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2 -2))


(side 1 (treasury 100 0 30 0))
(side 2 (treasury 1000 100 40 0))
(side 3 (treasury 200 50 100 0))
(side 4 (treasury 500 50 500 0))
(side 5 (treasury 500 250 200 0))


(side 6 (active false))


;;; Define basic terrain.

(include "t-korea")

(set synthesis-methods '(name-units-randomly))

(set initial-date "Apr 2006")





