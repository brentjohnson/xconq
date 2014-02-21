(game-module "3rd-age"
  (title "Third Age Middle Earth")
  (blurb "Base module for Middle Earth during the Third Age. Uses the Civ2 combat model.")
  (version "1.0")
  (variants
    (world-seen true)
    (see-all true)
    (sequential true)
    (world-size (60 35 360))
    ("Alternate economy" model1
     "Use a different algorithm for moving materials among units."
     (true (set backdrop-model 1)))
    ("AltEcon uses doctrine" model1doctrine
     "When using alternate economy, don't drain units below doctrine levels."
     (true (set backdrop-ignore-doctrine 0))
     (false (set backdrop-ignore-doctrine 1)))
  )
)



;;;	TERRAIN TYPES


(include "advterr")

(define water (sea lake))

(define land (swamp salt-marsh desert semi-desert steppe plain forest mountain))

;                               			 	sea 	lak	swa 	sal 	des 	sed	ste	pla 	for 	mou 	ice
; ---------------------------------------------------------------------

(add cell-t* alt-percentile-min 	(  	0  	89 	0  	20	21  	21  	21	21	21  	90  	99	))
(add cell-t* alt-percentile-max 	( 	20  	90  	0  	23	90  	90  	90	90	89  	99 	100	))
(add cell-t* wet-percentile-min 	(  	0   	0  	20   	20	0  	10	20  	30	70   	0   	0	))
(add cell-t* wet-percentile-max 	(	100 	100 	100	100	10  	20	30 	70	100 	100 	100	))



;;;	UNIT TYPES


(unit-type colonizers (name "Settlers") (image-name "ancient-colonizer")
  (acp-per-turn 1) (attack 0) (defend 1) (hp-max 2) (cp 8))
(unit-type faramir (name "Prince Faramir") (image-name "heroes-man4")
  (acp-per-turn 3) (attack 9) (defend 1) (hp-max 4) (cp 1))
(unit-type radagast (name "Wizard Radagast") (image-name "heroes-brown-wizard")
  (acp-per-turn 10) (attack 10) (defend 1) (hp-max 5) (cp 1))
(unit-type saruman (name "Wizard Saruman") (image-name "heroes-priest1")
  (acp-per-turn 15) (attack 12) (defend 8) (hp-max 7) (cp 1))
(unit-type archers (name "Archers") (image-name "ancient-archer")
  (acp-per-turn 1) (attack 3) (defend 2) (hp-max 1) (cp 3))
(unit-type merry (name "Hobbit Merry") (image-name "heroes-swordman2")
  (acp-per-turn 1) (attack 8) (defend 4) (hp-max 4) (cp 1))
(unit-type pikemen (name "Pikemen") (image-name "ancient-spearman")
  (acp-per-turn 1) (attack 1) (defend 6) (hp-max 1) (cp 2))
(unit-type elves (name "Elves") (image-name "heroes-elf3")
  (acp-per-turn 2) (attack 6) (defend 3) (hp-max 4) (cp 7))
(unit-type orcs (name "Orcs") (image-name "monsters-orc1")
  (acp-per-turn 1) (attack 5) (defend 2) (hp-max 2) (cp 2))
(unit-type legolas (name "Elf Legolas") (image-name "heroes-elf2")
  (acp-per-turn 3) (attack 9) (defend 6) (hp-max 4) (cp 1))
(unit-type hobbits (name "Hobbits") (image-name "heroes-hobbits")
  (acp-per-turn 1) (attack 5) (defend 4) (hp-max 5) (cp 10))
(unit-type elrond (name "Lord Elrond") (image-name "heroes-man1")
  (acp-per-turn 3) (attack 10) (defend 1) (hp-max 5) (cp 1))
(unit-type galadriel (name "Queen Galadriel") (image-name "heroes-elf-lady")
  (acp-per-turn 3) (attack 12) (defend 1) (hp-max 5) (cp 1))
(unit-type spiders (name "Spiders") (image-name "monsters-alien-spider")
  (acp-per-turn 2) (attack 4) (defend 4) (hp-max 3) (cp 5))
(unit-type easterlings (name "Easterlings") (image-name "ancient-knight")
  (acp-per-turn 3) (attack 7) (defend 2) (hp-max 2) (cp 6))
(unit-type wainriders (name "Wainriders") (image-name "ancient-chariot")
  (acp-per-turn 2) (attack 6) (defend 3) (hp-max 2) (cp 6))
(unit-type mumakil (name "Mumakil") (image-name "ancient-elephant")
  (acp-per-turn 2) (attack 6) (defend 4) (hp-max 3) (cp 6))
(unit-type haradrim (name "Haradrim") (image-name "ancient-rider")
  (acp-per-turn 3) (attack 6) (defend 3) (hp-max 2) (cp 6))
(unit-type knights (name "Knights") (image-name "heroes-mounted8")
  (acp-per-turn 2) (attack 4) (defend 2) (hp-max 2) (cp 6))
(unit-type dunedain (name "Dunedain") (image-name "heroes-mounted6")
  (acp-per-turn 3) (attack 6) (defend 3) (hp-max 2) (cp 10))
(unit-type riders (name "Riders") (image-name "heroes-mounted2")
  (acp-per-turn 2) (attack 6) (defend 3) (hp-max 2) (cp 4))
(unit-type shelob (name "Monster Shelob") (image-name "monsters-shelob")
  (acp-per-turn 1) (attack 8) (defend 6) (hp-max 3) (cp 1))
(unit-type gwaihir (name "Eagle Gwaihir") (image-name "heroes-eagle-rider")
  (acp-per-turn 10) (attack 10) (defend 6) (hp-max 3) (cp 1))
(unit-type trolls (name "Trolls") (image-name "monsters-club-troll2")
  (acp-per-turn 1) (attack 8) (defend 3) (hp-max 3) (cp 10))
(unit-type olog-hai (name "Olog-Hai") (image-name "monsters-club-troll3")
  (acp-per-turn 1) (attack 10) (defend 5) (hp-max 4) (cp 25))
(unit-type uruk-hai (name "Uruk-Hai") (image-name "monsters-orc3")
  (acp-per-turn 2) (attack 7) (defend 3) (hp-max 2) (cp 8))
(unit-type witch-king (name "Witch King") (image-name "monsters-blue-king")
  (acp-per-turn 12) (attack 14) (defend 10) (hp-max 7) (cp 1))
(unit-type gandalf (name "Wizard Gandalf") (image-name "heroes-gray-wizard")
  (acp-per-turn 15) (attack 12) (defend 1) (hp-max 7) (cp 1))
(unit-type eagle (name "Eagle") (image-name "heroes-eagle")
  (acp-per-turn 8) (attack 8) (defend 5) (hp-max 3) (cp 12))
(unit-type nazgul (name "Nazgul") (image-name "monsters-mounted-nazgul1")
  (acp-per-turn 10) (attack 12) (defend 7) (hp-max 6) (cp 1))
(unit-type longboat (name "Longboat") (image-name "ancient-longship")
  (acp-per-turn 8) (attack 0) (defend 1) (hp-max 1) (cp 4))
(unit-type theoden (name "King Theoden") (image-name "heroes-mounted1")
  (acp-per-turn 1) (attack 9) (defend 5) (hp-max 4) (cp 1))
(unit-type balrog (name "Balrog") (image-name "monsters-balrog1")
  (acp-per-turn 3) (attack 15) (defend 8) (hp-max 5) (cp 1))
(unit-type gollum (name "Creep Gollum") (image-name "monsters-gollum")
  (acp-per-turn 3) (attack 0) (defend 5) (hp-max 3) (cp 1))
(unit-type spy-birds (name "Spy Birds") (image-name "heroes-eagles")
  (acp-per-turn 20) (attack 0) (defend 1) (hp-max 1) (cp 5))
(unit-type frodo (name "Hobbit Frodo") (image-name "heroes-discoverer")
  (acp-per-turn 1) (attack 0) (defend 3) (hp-max 5) (cp 1))
(unit-type sam (name "Hobbit Sam") (image-name "heroes-hobbit1")
  (acp-per-turn 1) (attack 9) (defend 5) (hp-max 4) (cp 1))
(unit-type dwarves (name "Dwarves") (image-name "heroes-dwarves")
  (acp-per-turn 2) (attack 6) (defend 4) (hp-max 3) (cp 9))
(unit-type gimli (name "Dwarf Gimli") (image-name "heroes-dwarf1")
  (acp-per-turn 3) (attack 9) (defend 6) (hp-max 3) (cp 1))
(unit-type aragorn (name "King Aragorn") (image-name "heroes-bluecape")
  (acp-per-turn 2) (attack 12) (defend 6) (hp-max 5) (cp 1))
(unit-type boromir (name "Prince Boromir") (image-name "heroes-prince2")
  (acp-per-turn 4) (attack 10) (defend 4) (hp-max 3) (cp 1))
(unit-type eomer (name "Prince Eomer") (image-name "heroes-mounted11")
  (acp-per-turn 2) (attack 10) (defend 4) (hp-max 3) (cp 1))
(unit-type undead (name "Undead") (image-name "monsters-undead")
  (acp-per-turn 7) (attack 15) (defend 0) (hp-max 3) (cp 1))
(unit-type ring (name "Ring of Power") (image-name "ancient-ring")
  (acp-per-turn 3) (attack 0) (defend 0) (hp-max 1) (cp 1))
(unit-type pippin (name "Hobbit Pippin") (image-name "heroes-hobbit2")
  (acp-per-turn 1) (attack 8) (defend 4) (hp-max 4) (cp 1))
(unit-type wormtongue (name "Spy Wormtongue") (image-name "monsters-wormtongue")
  (acp-per-turn 2) (attack 0) (defend 4) (hp-max 3) (cp 1))
(unit-type eowyn (name "Princess Eowyn") (image-name "heroes-swordwoman2")
  (acp-per-turn 2) (attack 8) (defend 5) (hp-max 4) (cp 1))
(unit-type sauron (name "Lord Sauron") (image-name "monsters-dark-lord")
  (acp-per-turn 5) (attack 30) (defend 15) (hp-max 7) (cp 1))
(unit-type dragon (name "Dragon") (image-name "monsters-green-dragon")
  (acp-per-turn 4) (attack 12) (defend 6) (hp-max 4) (cp 50))
(unit-type wolves (name "Wolves") (image-name "monsters-wolves")
  (acp-per-turn 3) (attack 8) (defend 1) (hp-max 2) (cp 10))
(unit-type battering-ram (name "Battering Ram") (image-name "ancient-battering-ram")
  (acp-per-turn 2) (attack 8) (defend 1) (hp-max 1) (cp 8))
(unit-type spy (name "Spy") (image-name "heroes-man2")
  (acp-per-turn 2) (attack 0) (defend 0) (hp-max 1) (cp 5))
(unit-type galley (name "Galley") (image-name "ancient-bireme")
  (acp-per-turn 1) (attack 0) (defend 3) (hp-max 5) (cp 6))
(unit-type rangers (name "Rangers") (image-name "heroes-mounted11")
  (acp-per-turn 2) (attack 5) (defend 2) (hp-max 2) (cp 10))
(unit-type treebeard (name "Ent-Lord Treebeard") (image-name "heroes-ent2")
  (acp-per-turn 1) (attack 12) (defend 6) (hp-max 5) (cp 1))
(unit-type ents (name "Ents") (image-name "heroes-ent1")
  (acp-per-turn 1) (attack 10) (defend 5) (hp-max 4) (cp 15))
(unit-type dromund (name "Dromund") (image-name "ancient-dromund")
  (acp-per-turn 5) (attack 7) (defend 3) (hp-max 2) (cp 6))
(unit-type catapult (name "Catapult") (image-name "ancient-onager")
  (acp-per-turn 1) (attack 8) (defend 1) (hp-max 1) (cp 6))
(unit-type grond (name "Grond") (image-name "ancient-ram-tower")
  (acp-per-turn 2) (attack 20) (defend 10) (hp-max 3) (cp 30))

;	Advanced Units

(unit-type mines (name "Mines")(image-name "ancient-burrow"))
(unit-type village (name "Village")(image-name "ancient-village1"))
(unit-type gondor-city (name "City")(image-name "ancient-yellow-castle"))
(unit-type town (name "City")(image-name "ancient-city"))
(unit-type mordor-city (name "City")(image-name "ancient-black-castle"))
(unit-type elven-city (name "City")(image-name "ancient-blue-city"))
(unit-type tree-city (name "City")(image-name "ancient-tree-city"))
(unit-type ruins (name "Ruins")(image-name "ancient-ruins"))
(unit-type tower (name "Tower")(image-name "ancient-tower"))

;	Facility Units

(unit-type stockade (name "Stockade")(image-name "stockade"))
(unit-type city-walls (name "City Walls")(image-name "walltown"))
(unit-type temple (name "Temple")(image-name "parthenon"))
(unit-type workshops (name "Workshops")(image-name "village"))
(unit-type granary (name "Granary")(image-name "granary"))
(unit-type marketplace (name "Marketplace")(image-name "camp"))
(unit-type forum (name "Forum")(image-name "parthenon"))
(unit-type palace (name "Palace")(image-name "parthenon"))
(unit-type barracks (name "Barracks")(image-name "parthenon"))
(unit-type harbor (name "Harbor")(image-name "anchor"))

(define persons
	(gandalf radagast saruman 
	  legolas elrond galadriel
	  aragorn boromir faramir
	  eomer eowyn theoden
	  frodo sam merry pippin
	  gimli treebeard gwaihir
	  sauron gollum wormtongue
	  witch-king balrog shelob
	  nazgul))

(define unarmed
	(colonizers spy))

(define infantry
	(archers pikemen
	  elves hobbits dwarves ents
	  orcs uruk-hai 
	  trolls olog-hai
	  spiders))
	
(define mounted
	(riders rangers dunedain
	  knights mumakil wolves
	  easterlings haradrim wainriders
	  nazgul undead))
	
(define siege
	(battering-ram catapult grond))

(define naval
  	(longboat galley dromund))

(define air
	(gwaihir eagle spy-birds dragon))

(define ground
	(append persons unarmed infantry mounted siege))

(define movers
	(append naval air ground))

(define armed
	(append persons infantry mounted siege naval air))

(define city
	(mines gondor-city town mordor-city elven-city tree-city tower))
	
(define places
	(mines village gondor-city town mordor-city elven-city tree-city ruins tower))

(define facilities
	(granary marketplace temple workshops forum stockade city-walls palace barracks harbor))



;;;	MATERIAL TYPES

(material-type food (name "Food")(image-name "sheaf")(resource-icon 1))
(material-type ores (name "Ores")(image-name "shield")(resource-icon 2))
(material-type cash (name "Cash")(image-name "arrows")(resource-icon 3))



;;;	STARTUP CONDITIONS


(table independent-density 
	(town plain 400)
)

(table favored-terrain 
	(colonizers land 100)
	(ground land 100)
	(places plain 100)
)

(table vanishes-on 
	(u* t* false)
	(places (sea lake ice) true)
	(colonizers (sea lake ice) true)
)

(set sides-min 4)
(set advantage-min 1)
(set advantage-default 1)
(set advantage-max 5)
(set country-separation-min 21)
(set country-separation-max 60)
(set country-radius-min 5)
(add places initial-seen-radius 2)
(add places already-seen 1)
(add places see-always 1)
(add colonizers start-with 3)
(scorekeeper (do last-side-wins))
(set indepside-has-ai true)
(set ai-may-resign false)			;	Turn off automatic resignation.
 


;;;	GENERAL UNIT PROPERTIES


(add village ai-minimal-size-goal 8)	;	Don't settle in places that cannot support a size 8 village.
(add city ai-minimal-size-goal 12)		;	Don't settle in places that cannot support a size 12 city.
(add ruins ai-minimal-size-goal 0)		;	Ruin sizes do not matter.

(add places ai-peace-garrison 0)		;	Minimal number of defenders assigned to a city.
(add places ai-war-garrison 2)		;	Minimal number of defenders assigned to a city.

(add village reach 1)
(add city reach 2)
(add city vision-range 2)

(add u* free-acp 1)				;	Makes defense possible.

(add places advanced true)			;	Places are advanced units.
(add places acp-independent true)		;	And they are also acp-independent.
(add ruins acp-independent false)		;	But ruins are not.



;;;	 VOLUME AND NUMBER RESTRICTIONS


(table unit-size-as-occupant	;	Basic size of unit as occ.
	(u* u* 1)				;	Each unit takes up 1 capacity slot as occupant.
	(facilities places 0)		;	Facilities do not compete for space.
	(u* persons 99)			;	Persons can't carry any other units.
	(ring persons 1)		;	except The Ring.
)

(add places capacity 24)		;	Each city has room for 24 occs.
(add village capacity 8)		;	Each village has room for 8 occs.
(add longboat capacity 6)
(add galley capacity 3)
(add dromund capacity 3)
(add persons capacity 1)

(add places occupant-total-max 24)	;	Max 20 occupants.
(add village occupant-total-max 8)	;	Max 8 occupants.
	
(add places facility-total-max 12)	;	Max 10 facilities.
(add village facility-total-max 4)	;	Max 2 facilities.
	
(add places mobile-total-max 12)	;	Max 10 mobiles.
(add village mobile-total-max 4)		;	Max 6 mobiles.
	
(table occupant-max			;	Extra limits on top of capacity!
	(u* places 0)			;	Places can't be occupants at all.
	(u* facilities 0)			;	Facilities can't be occupants in general.	
	(places facilities 1)		;	But places have room for exactly one facility of each type.	
	(u* ring 0)			;	The Ring can't be an occupant in general.	
	(persons ring 1)		;	But persons have room for exactly one Ring.	
	(naval naval 0)			;	Don't let ships ship ships!
)

(table unit-size-in-terrain
	(u* t* 1)				;	Each unit takes up 1 capacity slot in terrain. 
	(places t* 4)			;	But places cover the entire cell.
	(naval land 99)			;	No room for ships on land (except within places).
	(ground water 99)		;	No room for ground units at sea (except within transports).
						;	These land-water restrictions prevent creation of ships in 
						;	adjacent land cells when a city is full.
)

(add t* capacity 4)			;	Limits terrain capacity to 4 units or 1 place per cell.			



;;;	 UNIT MOVEMENT


(add places acp-per-turn 0)	;	Zero value since they are acp-independent.

(add places speed 0)					;	Cities and facilities are immobile.
(add facilities speed 0)

(table mp-to-enter-terrain 
	(u* t* 99)
	(ground land 1)
	(ground river 3)
	(ground ford 1)
	(ground forest 1)
	(ground mountain 2)
	(ground swamp 2)
	(ground salt-marsh 2)
  	(naval water 1)
  	(air t* 1)
)

(table ferry-on-entry
	(u* u* 0)
	((append places harbor) naval over-border)
	(persons ring 1)		;	The Ring can freely enter a bearer.
)

(table ferry-on-departure
	(u* u* 0)
	(persons ring 1)		;	The Ring can freely leave a bearer.
)

(table mp-to-traverse
	(u* (river road) 99)
	(naval river 1)
	(ground road 1)
)

(add u* free-mp 1)		;	Let units with only 1 mp cross rivers etc.



;;; 	COMBAT


(set combat-model 1)		;	Use the Civ-like combat model.

(add armed cxp-max 1)	;	Any armed unit can be a veteran. 

(table damage
	(u* u* 1)			;	Units lose one hp for each hit.
	(u* places 0)		;	Places are not damaged. (But they can be captured!)
	(u* facilities 0)		;	Nor are facilities. (But they can be captured!)
)

(add catapult acp-to-fire 1)

(add catapult range 2)

(add catapult range-min 1)

(table acp-to-attack		;	This refers to normal attacks, not fire or capture. However,
	(catapult u* 0)		;	setting it to null blocks capture attempts through one_attack!
	(facilities u* 0)
	(u* facilities 0)
	(places u* 0)
	(ground naval 0)
	(naval ground 0)
	(unarmed u* 0)
	(u* ring 0)

)

; (table acp-to-capture		;	Meaningless in combat model 1 games.
;	(u* u* 0)
;	(battering-ram places 1)
;	(grond places 1)
;	(persons ring 1)
; )

(table hit-chance	;	Note: zero by default!
	  (u* u* 100)
	  (unarmed u* 0)
	  (places u* 0)
	  (facilities u* 0)
	  (u* facilities 0)
	; (ground naval 0)	;	Why not?
	; (naval ground 0)	;	Why not?
)

;;	The protection table has no effect on model 1 combat, but is still important for the capture chance.
;;	By setting protection (armed places 0) all occupants must be destroyed before a city is captured.
;;	Without such protection, the city is captured when the first occupant is destroyed, and all other
;;	occupants perish. 

(table protection					;	100 (no protection) by default!
	(armed places 0)			;	Places can't be captured as long as they have armed defenders.
	(naval places 100)			;	But ships in harbor don't count as defenders.
	(places facilities 0)			;	Facilities can't be captured as long as they are in a friendly place.	
)

(table transport-affects-defense
	(places u* 150)				;	Cities increase occs def value by 50%
)

(table neighbour-affects-defense		
	(stockade u* 150)			;	Stockade increases occs def value by 100%
	(city-walls u* 200)			;	City walls increase occs def value by 100%
)

(table occupant-affects-attack
	(ring persons 1000)			;	The Ring makes its bearer 10 times more powerful.
)

(table constructor-absorbed-by
    (colonizers places true)
)

(table hp-to-garrison				;	Defaults to zero!
	(colonizers places 2)			;	Colonizer is consumed when founding a city.
)

(table capture-chance				;	independent-capture-chance defaults to this!
	(ground places 100)
	(persons ring 100)
)

(table auto-repair
	(barracks ground 100.00)
	(harbor naval 100.00)
  )



;;;	CONSTRUCTION


(add places cp 1)			;	Must be 1 to permit colonizers to build them.
(add facilities cp 20)			;	Facilities cost 20 cp.

(table can-create
	(places u* 1)			;	Places can create units.
	(ruins u* 0)			;	But ruins cannot.
	(places places 0)		;	But not other places.
	(places persons 0)		;	or persons.
	(places undead 0)		;	or undead.
	(places ring 0)			;	or the Ring.
	(colonizers city 1)		;	Colonizers can only create city.
)

(table can-build
	(places u* 1)			;	Places can build units.
	(ruins u* 0)			;	But ruins cannot.
	(places places 0)		;	But not other places.
	(places persons 0)		;	or persons.
	(places undead 0)		;	or undead.
	(places ring 0)			;	or the Ring.
	(colonizers city 1)		;	Colonizers can only build city.
)

(table acp-to-create			;	Defaults to zero!
	(places u* 1)			;	Places can create units.
	(ruins u* 0)			;	But ruins cannot.
	(places places 0)		;	But not other places.
	(places persons 0)		;	or persons.
	(places undead 0)		;	or undead.
	(places ring 0)			;	or the Ring.
	(colonizers city 1)		;	Colonizers can only create city.
)

(table acp-to-build			;	Defaults to zero!
	(places u* 1)			;	Places can build units.
	(ruins u* 0)			;	But ruins cannot.
	(places places 0)		;	But not other places.
	(places persons 0)		;	or persons.
	(places undead 0)		;	or undead.
	(places ring 0)			;	or the Ring.
	(colonizers city 1)		;	Colonizers can only build city.
)

(table create-range			;	Defaults to zero!
	(places u* 0)			;	Places cannot create units in adjacent cells.
)

(table build-range			;	Defaults to zero!
	(places u* 0)			;	Places cannot build units in adjacent cells.
)



;;;	MATERIAL PRODUCTION AND CONSUMPTION


(add cash treasury true)			;	Cash is globally available in the treasury.

(table gives-to-treasury
	(places cash true)
)

(table unit-storage-x				;	Only city can store materials.
	(u* m* 0)
	(places m* 9999)
)

(table production-from-terrain
	(t* m* 0)
	((desert semi-desert) ores 1) 
	((steppe plain) ores 2)
	((forest mountain) ores 3)
	((sea lake semi-desert) food 1)
	((steppe forest) food 2)
	((plain) food 3)
	((sea lake) cash 3)
)

(table unit-consumption-per-size	;	Defaults to zero!
	(places food 1)				;	Maintenance costs 1 food unit per size unit.
)

(table unit-consumption-to-grow	;	Defaults to zero!
	(places food 10)				;	Size increase costs 10 stored food units.
)

(table consumption-per-cp		;	Defaults to zero!
	(u* ores 1)				;	1 cp costs 1 ores unit.
	(places m* 0)
)

(table occupant-multiply-production
	(temple food 150)			;	Temple increases food production by 50%.
	(granary food 200)			;	Granary increases food production by 100%
	(workshops ores 150)		;	Workshops increase ores production by 50%.
	(palace ores 200)			;	Palace increase ores production by 100%.
	(marketplace cash 150)		;	Marketplace increases cash production by 50%
	(forum cash 200)			;	Forum increases cash production by 100%
)



;;;	SIDES


(set side-library '(
  ((name "Gondor") (plural-noun "Gondorians") (adjective "Gondor's") (class "gondor"))        
  ((name "Mordor") (plural-noun "Sauron's creatures") (adjective "Mordor's") (class "mordor"))
  ((name "Isengard") (plural-noun "Saruman's slaves") (adjective "Isengard's") (class "isengard"))
  ((name "Arnor") (plural-noun "Dunedain") (adjective "Arnor's") (class "arnor"))
  ((name "Rohan") (plural-noun "Rohirrim") (adjective "Rohan's") (class "rohan"))    
  ((name "Elves") (plural-noun "Elves") (adjective "Elven") (class "elves"))
  ((name "Dwarves") (plural-noun "Dwarves") (adjective "Dwarfish") (class "dwarves"))
  ((name "Ents") (plural-noun "Ents")	(adjective "Entish") (class "ents"))    
  ((name "Eagles") (plural-noun "Eagles") (adjective "Eagles'") (class "eagles"))    
  ((name "Shire") (plural-noun "Hobbits") (adjective "Shire's") (class "shire"))
  ((name "Harad") (plural-noun "Haradrim") (adjective "Harad's") (class "harad"))    
  ((name "Rhun") (plural-noun "Easterlings") (adjective "Rhun's") (class "rhun"))    
  ((name "Umbar") (plural-noun "Pirates") (adjective "Pirate") (class "umbar"))
  ((name "Monsters") (plural-noun "Monsters") (adjective "Monsters'") (class "monsters")))
)

(add archers possible-sides (or "gondor" (or "arnor" (or "rohan" (or "harad" (or "rhun" "umbar"))))))
(add pikemen possible-sides (or "gondor" (or "arnor" (or "rohan" (or "harad" (or "rhun" "umbar"))))))
(add elves possible-sides "elves")
(add orcs possible-sides (or "mordor" (or "isengard" "monsters")))
(add hobbits possible-sides "shire")
(add spiders possible-sides (or "mordor" (or "isengard" "monsters")))
(add easterlings possible-sides "rhun")
(add wainriders possible-sides "rhun")
(add mumakil possible-sides "harad")
(add haradrim possible-sides "harad")
(add knights possible-sides "gondor")
(add dunedain possible-sides "arnor")
(add riders possible-sides "rohan")
(add trolls possible-sides (or "mordor" (or "isengard" "monsters")))
(add olog-hai possible-sides (or "mordor" (or "isengard" "monsters")))
(add uruk-hai possible-sides (or "mordor" (or "isengard" "monsters")))
(add eagle possible-sides "eagles")
(add dwarves possible-sides "dwarves")
(add dragon possible-sides (or "mordor" (or "isengard" "monsters")))
(add wolves possible-sides (or "mordor" (or "isengard" "monsters")))
(add spy possible-sides (or "gondor" (or "arnor" (or "rohan" (or "harad" (or "rhun" "umbar"))))))
(add rangers possible-sides "arnor")
(add ents possible-sides "ents")
(add grond possible-sides "mordor")



;;;	NAMES


;;  Use short generic names.

(include "ng-weird")
(set default-namer "short-generic-names")
(add places namer "short-generic-names")



;;;	IMAGES


;;	Use some Civ2 type images.

(imf "sheaf" ((16 16) (file "civmisc.gif" 36 2)))
(imf "shield" ((16 16) (file "civmisc.gif" 54 2)))
(imf "arrows" ((16 16) (file "civmisc.gif" 72 2)))
