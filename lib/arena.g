(game-module "arena"
  (title "The Arena")
  (version "1.00")
  (blurb "Just a test of item units.")
  )

(set terrain-seen true)
(set indepside-has-ai true)
(set self-required true)



;;; Types -- Heroes must be self-units or unused weapons will live on

(unit-type hero (name "Hero") (image-name "ang-pirate-red-cap")
  (acp-per-turn 6) (hp-max 50) (vision-range 3)
  (point-value 10) (can-be-self true) (self-resurrects true)
  (help "A tough guy, stuck in the Arena, just trying to make a buck, or something."))

(unit-type fodder (name "Fodder") (image-name ("ang-hunchback-buff" "ang-jester-orange-bell" "ang-man-brown-drunk" "ang-man-cap-pitchfork" "ang-man-brown-hypnotized"))
  (acp-per-turn 6) (hp-max 10) (vision-range 2)
  (point-value 10)
  (help "Not so tough at all, in fact, they're just good for testing sword sharpness."))

(unit-type sword (name "Sword") (image-name "ang-scroll-sword-green")
  (acp-per-turn 2) (hp-max 1) (vision-range 0)  
  (point-value 10)
  (help "A sword, for killing people."))

(unit-type dagger (name "Dagger") (image-name "ang-scroll-sword-hit")
  (acp-per-turn 3) (hp-max 1) (vision-range 0)  
  (point-value 10)
  (help "A knife, for stabbing people and thereby pissing them off."))

(unit-type claymore (name "Claymore") (image-name "ang-scroll-sword-red")
  (acp-per-turn 2) (hp-max 1) (vision-range 0)  
  (point-value 10)
  (help "A big sword, for killing many, many people."))

(unit-type bow (name "Bow") (image-name "ang-arrow-red-right")
  (acp-per-turn 2) (hp-max 1) (vision-range 0) (acp-to-fire 1) (range 5)  
  (point-value 10)
  (help "A bow, for shooting people."))

(unit-type crossbow (name "Crossbow") (image-name "ang-arrow-green-up")
  (acp-per-turn 1) (hp-max 1) (vision-range 0) (acp-to-fire 1) (range 5)  
  (point-value 10)
  (help "A crossbow, for shooting less people but killing more of the people you hit."))

(unit-type leatherarmor (name "Leather Armor") (image-name "ang-scroll-armor-blue")
  (acp-independent true) (hp-max 1) (vision-range 0)  
  (point-value 10)
  (help "Thin, light armor."))

(unit-type chainarmor (name "Chain Mail Armor") (image-name "ang-scroll-armor-green")
  (acp-independent true) (hp-max 1) (vision-range 0)  
  (point-value 10)
  (help "Thickish, heavyish armor."))

(unit-type platearmor (name "Plate Mail Armor") (image-name "ang-scroll-armor-red")
  (acp-independent true) (hp-max 1) (vision-range 0)  
  (point-value 10)
  (help "Thick, heavy armor."))

(unit-type smallshield (name "Small Shield") (image-name "ang-chest-small-wood")
  (acp-independent true) (hp-max 1) (vision-range 0)  
  (point-value 10)
  (help "A wee, little shield."))

(unit-type largeshield (name "Large Shield") (image-name "ang-chest-large-metal")
  (acp-independent true) (hp-max 1) (vision-range 0)  
  (point-value 10)
  (help "A big shield that really helps against arrows and such."))


(define people-types (hero fodder))

(define item-types (sword dagger claymore bow crossbow leatherarmor chainarmor platearmor smallshield largeshield))

(define melee-weapon-types (sword dagger claymore))

(define ranged-weapon-types (bow crossbow))

(define armor-types (leatherarmor chainarmor platearmor))

(define shield-types (smallshield largeshield))

;; TERRAIN

(terrain-type sea (char ".")
  (help "deep water"))
(terrain-type shallows (char ",")
  (help "shallow coastal water and lakes"))
(terrain-type swamp (char "="))
(terrain-type desert (image-name "adv-desert") (char "~")
  (help "dry open terrain"))
(terrain-type land (image-name "adv-plain") (char "+")
  (help "open flat or rolling country"))
(terrain-type forest (image-name "adv-forest") (char "%"))
(terrain-type mountains (image-name "adv-mountains") (char "^"))
(terrain-type ice  (image-name "adv-ice") (char "_"))
(terrain-type neutral (image-name "gray") (char "-"))
(terrain-type hills (image-name "adv-hills"))
(terrain-type semi-desert (image-name "adv-semi-desert"))
(terrain-type steppe (image-name "adv-steppe"))
(terrain-type deepforest (image-name "adv-forest"))
(terrain-type wasteland (image-name "gray"))
(terrain-type river (char "<")
  (subtype border) (subtype-x river-x))
(terrain-type beach (char "|")
  (subtype border))
(terrain-type cliffs (image-name "cliffs")
  (subtype border))
(terrain-type road (char ">")
  (subtype connection) (subtype-x road-x))
(terrain-type mud (image-name "brown")
  (subtype coating))
(terrain-type snow (image-name "ice")
  (subtype coating))
(terrain-type jungle (image-name "adv-forest"))

(terrain-type p-land (name "Phantom Plains") (image-name "blue") (char "+")
  (help "Wispy plains."))
(terrain-type p-forest (name "Phantom Forest") (image-name "blue") (char "%"))
(terrain-type p-mountains (name "Phantom Mountains") (image-name "blue") (char "^"))
(terrain-type p-hills (name "Phantom Hills") (image-name "blue"))
(terrain-type p-river (name "Phantom River") (image-name "blue")
  (subtype border) (subtype-x river-x))
(terrain-type p-cliffs (name "Phantom Cliffs") (image-name "blue")
  (subtype border))



(add (sea shallows) liquid true)

(define sea-t* (sea shallows))

(define land-t* (jungle deepforest wasteland desert semi-desert hills steppe land forest mountains river))

(define phantom-t* (p-land p-forest p-mountains p-hills p-river p-cliffs))

;;; Unit-unit capacities.


(add hero capacity 4)

(table unit-size-as-occupant
  ;; Disable occupancy by default.
	(u* u* 99)
	(item-types hero 1)
	(ranged-weapon-types hero 2)
)

(table occupant-max
	(fodder item-types 0)
	
)

;;; Unit-terrain capacities.

(table unit-size-in-terrain
  (u* t* 25)
)

(add t* capacity 100)


;;; Movement.

(table mp-to-enter-terrain
  (u* t* 2)
  (people-types sea-t* 99)
  (people-types cliffs 99)
  (people-types river 3)
  (people-types (hills forest semi-desert swamp jungle) 3)
  (people-types (desert wasteland deepforest) 4)
  (people-types mountains 6)
  (item-types t* 99)
)

(table mp-to-leave-terrain
  (people-types sea-t* 99)
)




(add hero hp-recovery 5)

(set combat-model 0)


(table acp-to-attack
  (u* u* 0)
  (hero people-types 3)
  (fodder people-types 3)
  (melee-weapon-types people-types 1)
  (fodder item-types 3)
)

(table acp-to-defend
   (u* u* 99)
)



(table fire-hit-chance
	(u* u* 0)
	(ranged-weapon-types u* 75)
)


(table fire-damage
	(bow people-types 1d8)
	(crossbow people-types 2d6)
	(ranged-weapon-types item-types 0)

)





(table hit-chance
	(u* u* 0)
	(people-types people-types 75)
	(melee-weapon-types u* 75)
	(fodder item-types 100)
)



(table damage
	(u* u* 0)
	(people-types people-types 1d3)
	(sword people-types 1d10)
	(dagger people-types 1d6)
	(claymore people-types 3d6)
	(melee-weapon-types item-types 0)
)

(table capture-chance
	(fodder item-types 100)
)

(table ferry-on-entry
	(u* u* 0)
	(people-types item-types over-all)
)

(table ferry-on-departure
	(u* u* 0)
;;	(people-types item-types over-all)
)

(table mp-to-leave-unit
	(item-types people-types 99)
)

(table protection
  ;; Ground units protect cities.

  (leatherarmor people-types 20)
  (chainarmor people-types 40)
  (platearmor people-types 60)
  (smallshield people-types 10)
  (largeshield people-types 20)
  (ranged-weapon-types people-types 0)
  (melee-weapon-types people-types 0)


  (people-types leatherarmor 80)
  (people-types chainarmor 60)
  (people-types platearmor 40)
  (people-types smallshield 90)
  (people-types largeshield 80)
  (people-types ranged-weapon-types 100)
  (people-types melee-weapon-types 100)


)



(table occupant-combat
	(u* u* 0)
	(melee-weapon-types people-types 100)
	(ranged-weapon-types people-types 100)
)



(add u* lost-vanish-chance 0)

;;; The world.

(world 360 (year-length 12) (axial-tilt 22))


(set calendar '(usual month))

(set season-names
  ;; 0 is January, 3 is April, 6 is July, 9 is October
  ((0 2 "winter") (3 5 "spring") (6 8 "summer") (9 11 "autumn")))


;;; River generation.

(add (land forest mountains) river-chance (10.00 25.00 25.00))

(set river-sink-terrain shallows)

;;; Road generation.

(table road-into-chance
  (t* land 100)
  (land (desert forest mountains) (50 40 20))
  )

(define cell-t* (steppe sea swamp desert forest land mountains ice))

;                               		sea 	lak	swa 	des 	pla 	for 	mou 	ice
; ---------------------------------------------------------------------

(add cell-t* alt-percentile-min 	(  	0  	27 	30  	30  	30	30  	90  	99	))
(add cell-t* alt-percentile-max 	( 	28  	30  	33  	90  	90	89  	99 	100	))
(add cell-t* wet-percentile-min 	(  	0   	0  	20   	0  	20	70   	0   	0	))
(add cell-t* wet-percentile-max 	(	100 	100 	100	20  	70	100 	100 	100	))

;;;	Some defns for the fractal percentile generator.

(set alt-blob-density 10000)
(set alt-blob-height 500)
(set alt-blob-size 100)
(set alt-smoothing 4)
(set wet-blob-density 2000)
(set wet-blob-size 100)


(table favored-terrain
  (u* t* 0)
  (people-types land-t* 100)
  (item-types land-t* 100)

	)


(set edge-terrain ice)


(add hero start-with 3)
(add fodder start-with 10)
(add item-types start-with 1)

(scorekeeper (do last-side-wins))





(game-module (design-notes
  "It is ambitious"
  ))

(imf "ang-hunchback-buff" ((32 32) (file "dg_monster132.gif" std 0 0)))

(imf "ang-scroll-sword-red" ((32 32) (file "dg_misc32.gif" std 1 15)))
(imf "ang-scroll-sword-hit" ((32 32) (file "dg_misc32.gif" std 2 11)))
(imf "ang-scroll-sword-green" ((32 32) (file "dg_misc32.gif" std 2 15)))

(imf "ang-scroll-armor-red" ((32 32) (file "dg_misc32.gif" std 3 14)))
(imf "ang-scroll-armor-green" ((32 32) (file "dg_misc32.gif" std 4 11)))
(imf "ang-scroll-armor-blue" ((32 32) (file "dg_misc32.gif" std 5 11)))

(imf "ang-arrow-red-right" ((32 32) (file "dg_effects32.gif" std 1 9)))
(imf "ang-arrow-green-up" ((32 32) (file "dg_effects32.gif" std 8 9)))
(imf "ang-jester-orange-bell" ((32 32) (file "dg_people32.gif" std 0 1)))
(imf "ang-man-brown-drunk" ((32 32) (file "dg_people32.gif" std 0 2)))
(imf "ang-man-cap-pitchfork" ((32 32) (file "dg_people32.gif" std 1 0)))
(imf "ang-man-brown-hypnotized" ((32 32) (file "dg_people32.gif" std 1 1)))

(imf "ang-pirate-red-cap" ((32 32) (file "dg_abyss.gif" std 3 0)))
(imf "ang-chest-small-wood" ((32 32) (file "dg_misc32.gif" std 0 0)))
(imf "ang-chest-large-metal" ((32 32) (file "dg_misc32.gif" std 5 0)))

