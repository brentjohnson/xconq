(game-module "colonizer"
  (title "Planet Colonization")
  (version "0.5")
  (blurb "Futuristic planet colonization game by Lincoln Peters.")
  (instructions
"Find a spot where you can build your first few cities and facilities, and then grow out from there.
If the planet is inhabited, you will have to fight the native population in order to establish yourself there.")
  
  (notes
"This game is modeled on what it might be like if, in the (somewhat) near future, a team of settlers landed on an Earth-like planet and built up a civilization there.")
  (variants
    (world-seen true)
    (see-all false)
    (world-size (60 30 60))
    ("Already populated" already-populated
     "Planet has a population that will fight over resources"
     (true
       (set indepside-has-ai true)
       (table independent-density add
         (city plain 1.00)
	 (city steppe 1.00)
	 (city forest 1.00)
       )))
    ("Last side wins" last-side-wins
     "The game won't end until only one player remains"
     (true
       (scorekeeper (do last-side-wins))))
    ("AI may not resign" ai-may-not-resign
     "Don't allow the AI player to give up"
     (true
       (set ai-may-resign false)))
;; Doesn't yet give realistic maps...
    ("M-class world" m-class-world
     "Land on an M-class (Earth-like) planet"
     (true
     		; Try to adjust the percentiles so that the world is 30% land
	        
     ;                               		sea 	lak	swa 	sal 	des 	sed	ste	pla 	for 	mou 	ice
; ---------------------------------------------------------------------

(add cell-t* alt-percentile-min 	(  	0  	69 	70  	70	71  	71  	71	71	71  	93  	99	))
(add cell-t* alt-percentile-max 	( 	69  	70  	71  	71	93  	93  	93	93	93  	99 	100	))
(add cell-t* wet-percentile-min 	(  	0   	0  	50   	50	0  	5	15  	20	80   	0   	0	))
(add cell-t* wet-percentile-max 	(	100 	100 	100	100	5  	15	20 	80	100 	100 	100	))

     ))
    ("N-class world" n-class-world
     "Land on an N-class (entirely water) planet"
     (true
     		; Try to adjust the precentiles so that the world is 1% land

     ;                               		sea 	lak	swa 	sal 	des 	sed	ste	pla 	for 	mou 	ice
; ---------------------------------------------------------------------

(add cell-t* alt-percentile-min 	(  	0  	89 	0  	20	21  	21  	21	21	21  	90  	99	))
(add cell-t* alt-percentile-max 	( 	90  	90  	0  	23	90  	90  	90	90	89  	99 	100	))
(add cell-t* wet-percentile-min 	(  	0   	0  	40   	40	0  	10	20  	30	70   	0   	0	))
(add cell-t* wet-percentile-max 	(	100 	100 	100	100	10  	20	30 	70	100 	100 	100	))

     ))
    ("L-class world" l-class-world
     "Land on an L-class (geologically inactive) planet"
     (true
     		; Try to adjust the percentiles so that the world is 30% land, but no plains or forest
     ;                               		sea 	lak	swa 	sal 	des 	sed	ste	pla 	for 	mou 	ice
; ---------------------------------------------------------------------

(add cell-t* alt-percentile-min 	(  	0  	89 	0  	20	21  	21  	21	21	21  	90  	99	))
(add cell-t* alt-percentile-max 	( 	60  	90  	0  	23	90  	90  	90	90	89  	99 	100	))
(add cell-t* wet-percentile-min 	(  	0   	0  	40   	40	0  	10	20  	30	70   	0   	0	))
(add cell-t* wet-percentile-max 	(	100 	100 	100	100	10  	20	30 	70	100 	100 	100	))

      ))
    ("Monsters" monsters
     "Generate a world populated with monsters"
     (true
       (set indepside-has-ai true)
       (table independent-density add
         (orc-hole mountain 1.00)
         (troll-cave mountain 1.00)
         (wolf-den mountain 1.00)
         (spider-nest mountain 1.00)
         (dragon-lair mountain 1.00)
       )))
    ("Alternate economy" model1
     "Use a different algorithm for moving materials among units."
     (true (set backdrop-model 1)))
    ("AltEcon uses doctrine" model1doctrine
     "When using alternate economy, don't drain units below doctrine levels."
     (true (set backdrop-ignore-doctrine 0))
     (false (set backdrop-ignore-doctrine 1)))

  )
)

(include "advterr")
(include "town-names")

(set sides-min 1)

(imf "sheaf" ((16 16) (file "civmisc.gif" 36 2)))
(imf "shield" ((16 16) (file "civmisc.gif" 54 2)))
(imf "arrows" ((16 16) (file "civmisc.gif" 72 2)))

(define water (sea lake))

(define land (swamp salt-marsh desert semi-desert steppe plain forest mountain))

;                               			 	sea 	lak	swa 	sal 	des 	sed	ste	pla 	for 	mou 	ice
; ---------------------------------------------------------------------

(add cell-t* alt-percentile-min 	(  	0  	89 	0  	20	21  	21  	21	21	21  	90  	99	))
(add cell-t* alt-percentile-max 	( 	20  	90  	0  	23	90  	90  	90	90	89  	99 	100	))
(add cell-t* wet-percentile-min 	(  	0   	0  	20   	20	0  	10	20  	30	70   	0   	0	))
(add cell-t* wet-percentile-max 	(	100 	100 	100	100	10  	20	30 	70	100 	100 	100	))

;; City types
(unit-type city (image-name "ancient-city") (point-value 5)
  (hp-max 20) (cp 10)
  (help "A typical city where sentient lifeforms live"))
(unit-type arcology (image-name "ancient-tower") (point-value 10)
  (hp-max 30) (cp 20)
  (help "A huge city stacked into a very tall building"))
(unit-type floating-city (image-name "ancient-barge") (point-value 7)
  (hp-max 15) (cp 15)
  (help "Smaller than a land city, but makes oceans inhabitable"))
  
;; Small facilities
(unit-type radar (image-name "radar") (point-value 2)
  (hp-max 1) (cp 20)
  (help "Sees things far away"))

;; Large facilities
(unit-type spaceport (image-name "airbase") (point-value 3)
  (hp-max 10) (cp 15)
  (help "Builds aircraft"))

;; Ground units
(unit-type settler (image-name "worker") (point-value 2)
  (hp-max 1) (acp-per-turn 2) (acp-min -1) (cp 10)
  (help "Builds cities"))
(unit-type engineer (image-name "tractor") (point-value 1)
  (hp-max 1) (acp-per-turn 2) (acp-min -1) (cp 15)
  (help "Build big facilities"))
(unit-type infantry (image-name "soldiers") (point-value 1)
  (hp-max 1) (acp-per-turn 2) (acp-min -1) (cp 10)
  (help "Foot soldiers that capture and/or defend worlds"))
(unit-type armor (image-name "tank") (point-value 2)
  (hp-max 2) (acp-per-turn 3) (acp-min -1) (cp 20)
  (help "Like infantry, but with tanks"))
(unit-type rover (image-name "jeep") (point-value 0)
  (hp-max 1) (acp-per-turn 4) (acp-min -1) (cp 15)
  (help "Good for ground support, reconnaisance"))
(unit-type bus (image-name "halftrack")
  (hp-max 1) (acp-per-turn 6) (acp-min -2) (cp 15)
  (help "Moves units on land quickly and cheaply, but only across roads"))

;; Naval units
(unit-type transport (image-name "ancient-dromund") (point-value 0)
  (hp-max 2) (acp-per-turn 2) (cp 20)
  (help "The most cost-effective (but slowest) way to cross oceans"))

;; Air units
(unit-type mothership (image-name "mothership") (point-value 1)
  (hp-max 10) (acp-per-turn 4) (cp 40)
  (help "Large spacecraft for landing on worlds"))
(unit-type saucer (image-name "saucer") (point-value 0)
  (hp-max 1) (acp-per-turn 8) (cp 15)
  (help "Small and fast; moves land units"))
(unit-type fighter (image-name "delta") (point-value 0)
  (hp-max 2) (acp-per-turn 6) (cp 15)
  (help "Moves fast and fights anything"))
  
;; Just to make it interesting...
(unit-type orcs (image-name "monsters-orc1")
  (hp-max 1) (acp-per-turn 2) (cp 10)
  (help "Primitive, warmongering humanoids"))
(unit-type trolls (image-name "monsters-club-troll2")
  (hp-max 2) (acp-per-turn 1) (cp 10)
  (help "Big, slow, unintelligent humanoids"))
(unit-type spiders (image-name "monsters-alien-spider")
  (hp-max 2) (acp-per-turn 3) (cp 10)
  (help "Gigantic and fast insects"))
(unit-type wolves (image-name "monsters-wolves")
  (hp-max 1) (acp-per-turn 4) (cp 10)
  (help "Very fast (but weak) canine creatures"))
(unit-type dragons (image-name "monsters-green-dragon")
  (hp-max 3) (acp-per-turn 5) (cp 10)
  (help "Fast, tough, reptilian demon"))
  
(unit-type orc-hole (image-name "ancient-ruins")
  (hp-max 15) (acp-per-turn 1) (cp 25)
  (help "Where the orcs come from"))
(unit-type troll-cave (image-name "ancient-burrow")
  (hp-max 20) (acp-per-turn 1) (cp 25)
  (help "Where the trolls come from"))
(unit-type spider-nest (image-name "ancient-black-castle")
  (hp-max 15) (acp-per-turn 1) (cp 25)
  (help "Where the spiders come from"))
(unit-type wolf-den (image-name "ancient-burrow")
  (hp-max 15) (acp-per-turn 1) (cp 25)
  (help "Where the wolves come from"))
(unit-type dragon-lair (image-name "ancient-black-castle")
  (hp-max 15) (acp-per-turn 1) (cp 25)
  (help "Where the dragons live"))

;; Is this interesting enough for you???

  
;; Materials
(material-type food (image-name "sheaf") (resource-icon 1)
  (help "Feeds cities"))
(material-type ores (image-name "shield") (resource-icon 2)
  (help "Construction material"))
(material-type fuel (image-name "arrows") (resource-icon 3)
  (help "Feeds machines"))
(material-type ammo
  (help "Essential for combat"))
  
(define cities (city arcology floating-city))
(define small-facilities (radar))
(define large-facilities (spaceport))
(define ground (settler engineer infantry armor rover bus))
(define naval (transport))
(define air (mothership saucer fighter))
(define military (infantry armor fighter transport))

(define monsters (orcs trolls spiders wolves dragons))
(define dens (orc-hole troll-cave spider-nest wolf-den dragon-lair))

(add cities advanced true)
(add large-facilities advanced true)
(add cities acp-independent true)
(add large-facilities acp-independent true)
(add cities acp-per-turn 0)
(add large-facilities acp-per-turn 0)
(add cities speed 0)
(add large-facilities speed 0)

(add dens speed 0)

#|
(add floating-city advanced false)
|#
(add floating-city acp-independent false)
(add floating-city acp-per-turn 2)
(add floating-city speed 100)


(add u* free-acp 1)

(add city reach 2)
(add arcology reach 3)
(add floating-city reach 0)

(add city ai-minimal-size-goal 4)
(add arcology ai-minimal-size-goal 6)
(add floating-city ai-minimal-size-goal 2)

(add large-facilities reach 1)

(add floating-city speed 100)

(add cities namer "random-town-names")
(add large-facilities namer "junky")

(add mothership start-with 1)
(add saucer start-with 2)
(add settler start-with 3)
(add engineer start-with 1)
(add fighter start-with 4)
(add infantry start-with 2)
(add rover start-with 1)

(set country-separation-min 32)

(add cities see-always true)
(add large-facilities see-always true)

(table favored-terrain
  (u* t* 0)
  (mothership land 100)
)

(table mp-to-enter-terrain
  (air t* 1)
  (ground water 99)
  (bus land 3)
  (naval land 99)
  (air river 0)
  
  (floating-city land 99)
  
  (monsters water 99)
  (dragons water 1)
  (dragons river 0)
)

(table mp-to-leave-terrain
  ;; Don't let units jump from a road on water to land unless the road goes all the way
  (ground water 99)
)

(table mp-to-traverse
  (u* road 0)
  (u* ford 0)
)

(table vanishes-on
  (ground water true)
  (naval land true)
  (floating-city land true)
)

(table production-from-terrain
  (sea m* (3 1 10 0))
  (lake m* (3 1 20 0))
  (swamp m* (1 1 20 0))
  (salt-marsh m* (1 1 30 0))
  (desert m* (0 2 20 0))
  (semi-desert m* (1 2 20 0))
  (steppe m* (2 2 10 0))
  (plain m* (3 1 10 0))
  (forest m* (2 2 20 0))
  (mountain m* (0 3 10 0))
  
  (river m* (1 1 0 0))
)

(table unit-storage-x
  (cities m* 9999)
  (city fuel 600)
  (arcology fuel 800)
  (floating-city fuel 300)
  
  (city ammo 400)
  (arcology ammo 600)
  (floating-city ammo 350)

  (spaceport (ores fuel ammo) (9999 400 200))
  
  (mothership fuel 200)
  (saucer fuel 32)
  (fighter fuel 24)
  
  (mothership ammo 100)
  (infantry ammo 10)
  (armor ammo 15)
  (fighter ammo 5)
  (transport ammo 40)
)

(table unit-initial-supply
  (air m* 9999)
  (ground m* 9999)
)

(table base-production
  (mothership fuel 2)

  (city ammo 15)
  (arcology ammo 25)
  (floating-city ammo 10)

  (large-facilities ammo 5)

  ;; (floating-city food 3)
)

(table base-consumption
  (air fuel 1)

  ;; (floating-city food 1)
)

(table consumption-as-occupant
  (u* m* 0)
)

(table material-to-move
  (air fuel 1)
)

(table consumption-per-move
  (air fuel 1)
)

(table hp-per-starve
  (air fuel 99)
  (mothership fuel 1)	;; Mothership is supposed to be able to stay airborne without extra fuel

  ;; (floating-city food 9999)
)

(table unit-consumption-per-size
  (cities food 1)
  ;; (floating-city food 0)
)

(table unit-consumption-to-grow
  (cities food 20)
  (large-facilities ores 20)
  ;; (floating-city food 0)
)

#|
(table productivity
  ;; Needed for floating-city because they aren't advanced, but must depend on the land.
  ;; Floating cities are unlikely to ever occupy land tiles, but they could be built on land and then pushed into the water.
  ;; (of course, they should be next to a water tile!!!)
  (floating-city swamp 0.33)
  (floating-city salt-marsh 0.33)
  (floating-city desert 0.00)
  (floating-city semi-desert 0.33)
  (floating-city steppe 0.67)
  (floating-city forest 0.67)
  (floating-city mountain 0.00)
)
|#

;;; CONSTRUCTION

(table can-create
  (settler cities 1)
  (cities small-facilities 1)
  (engineer large-facilities 1)
  (spaceport air 1)
  (cities ground 1)
  (cities naval 1)

  (orc-hole orcs 1)
  (troll-cave trolls 1)
  (spider-nest spiders 1)
  (wolf-den wolves 1)
  (dragon-lair dragons 1)
)

(table can-build
  (settler cities 1)
  (cities small-facilities 1)
  (engineer large-facilities 1)
  (spaceport air 1)
  (cities ground 1)
  (cities naval 1)

  (orc-hole orcs 1)
  (troll-cave trolls 1)
  (spider-nest spiders 1)
  (wolf-den wolves 1)
  (dragon-lair dragons 1)
)

(table acp-to-create
  (settler cities 1)
  (cities small-facilities 1)
  (engineer large-facilities 1)
  (spaceport air 1)
  (cities ground 1)
  (cities naval 1)

  (orc-hole orcs 1)
  (troll-cave trolls 1)
  (spider-nest spiders 1)
  (wolf-den wolves 1)
  (dragon-lair dragons 1)
)

(table acp-to-build
  (settler cities 1)
  (cities small-facilities 1)
  (engineer large-facilities 1)
  (spaceport air 1)
  (cities ground 1)
  (cities naval 1)

  (orc-hole orcs 1)
  (troll-cave trolls 1)
  (spider-nest spiders 1)
  (wolf-den wolves 1)
  (dragon-lair dragons 1)
)

(table create-range
  (settler floating-city 1)
)

(table build-range
  (settler floating-city 1)
)

(table hp-to-garrison
  (settler cities 1)
  (engineer large-facilities 1)
)

(table consumption-per-cp
  (ground ores 1)
  (air ores 1)
  (naval ores 1)
  (small-facilities ores 1)
)

;;; ENGINEERING

;; Allow engineers to build roads
(table acp-to-add-terrain
  (engineer road 2)
)

(table acp-to-remove-terrain
  (engineer road 2)
)

;;; CAPACITANCE

(table unit-capacity-x
  (mothership (settler engineer infantry armor rover saucer fighter) (6 4 6 2 2 8 6))
)

(add cities capacity 32)
(add large-facilities capacity 16)
(add rover capacity 1)
(add bus capacity 2)
(add saucer capacity 1)
(add transport capacity 2)

(add dens capacity 16)

(table unit-size-as-occupant
  (u* u* 999)
  
  (settler rover 1)
  (infantry rover 1)
  
  (ground bus 1)
  (engineer bus 2)
  (armor bus 2)
  (bus bus 999)
  
  (ground saucer 1)
  (ground transport 1)
  
  (air spaceport 1)
  (ground spaceport 1)

  (u* cities 1)
  (cities cities 999)
  
  (monsters dens 1)
)

(table occupant-max
  (cities small-facilities 1)
)

(table unit-size-in-terrain
  (cities t* 16)
  (large-facilities t* 16)
)

(table terrain-capacity-x
  (bus road 1)
)

;;; VISION
(add air vision-range 2)
(add mothership vision-range 3)
(add cities vision-range 3)
(add large-facilities vision-range 2)
(add radar vision-range 8)
;; ...and to be sure that settlers & engineers can see attackers coming...
(add settler vision-range 2)
(add engineer vision-range 2)


(add u* can-see-behind false)
(add radar can-see-behind true)

;;; COMBAT

(table acp-to-attack
  (u* u* 0)
  (military u* 1)
  (monsters u* 1)
)

(add mothership acp-to-fire 1)
(add mothership range 4)

(table hit-chance
  (u* u* 50)
  (cities u* 20)
)

(table damage
  (u* u* 1)
  (monsters cities 3)	;; They sack instead of capturing!
)

(table material-to-attack
  (u* ammo 1)
  (monsters ammo 0)
)

(table consumption-per-attack
  (u* ammo 1)
  (monsters ammo 0)
)

(table hit-by
  (u* ammo 1)
)

(table capture-chance
  (infantry cities 50)
  (infantry large-facilities 75)
  (armor cities 60)
  (armor large-facilities 85)
)

(table protection
  (infantry cities 0.00)
  (infantry large-facilities 0.00)
  (armor cities 0.00)
  (armor large-facilities 0.00)
  (mothership u* 0.00)
  (mothership cities 0.50)
  (mothership large-facilities 0.50)
  
  (cities infantry 0.25)
  (cities armor 0.50)
)

;;; REPAIR

(table auto-repair
  (mothership u* 1.00)
  (cities u* 1.00)
  (spaceport air 1.00)
  (engineer u* 1.00)
)

(add cities hp-recovery 1.00)
(add large-facilities hp-recovery 0.50)

;;; DOCTRINES
(doctrine build-once
  (construction-run (u* 1)))
  
(side-defaults
  (doctrines
    (settler build-once)
    (engineer build-once)))

