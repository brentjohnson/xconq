(game-module "advances"
  (title "Advances Through History")
  (blurb "Advanced empire-building and research game by Hans Ronne.")
  (version "1.0")
  (instructions (
    "Research scientific advances in order to produce more powerful units."
    "Make colonizers that can build new cites and colonize all available land. "
    "Do not forget to make military units, or suffer the consequences!"
  ))



;;;	THE WORLD


  (variants
    (world-seen true)
    (see-all true)
    (sequential true)
    (world-size (60 35 360))
#|
    (people false)
    (economy false)
    (supply false)
    ("Mostly Water" dummy-symbol
      (false
	;	Adjust so that sea is 20% instead of 50% of the world.
	(add sea alt-percentile-max 20)
	(add lake alt-percentile-min 20)
	(add lake alt-percentile-max 21)
	(add swamp alt-percentile-min 21)
	(add swamp alt-percentile-max 23)
	(add (desert semi-desert plain forest) alt-percentile-min 21)
       )
    )
|#
    ("Alternate economy" model1
     "Use a different algorithm for moving materials among units."
     (true (set backdrop-model 1)))
    ("AltEcon uses doctrine" model1doctrine
     "When using alternate economy, don't drain units below doctrine levels."
     (true (set backdrop-ignore-doctrine 0))
     (false (set backdrop-ignore-doctrine 1)))
  )
)

(set indepside-has-ai true)

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


;	Unarmed Units

(unit-type colonizers (name "Colonizers") (help "Settler Unit")
	(notes ("Colonizers made up of retired military units were used from Sargon the Great to the Romans"
		    "to build colonies and settle new land.")) 
)
(unit-type corvees (name "Corvees") (help "Forced Labour")
	(notes ("The Sumerian Dusu or Corvees consisted of pesants who did unpaid work for the City State as"
		    "a form of tax. Corvee workers were also used in Egypt to build the Pyramids."))
)
(unit-type sappers (name "Sappers") (help "Construction Crew")
	(notes ("Sappers as a separate branch of the armed services first appeared with Alexander and was"
		    "fully developed in Roman times. They were used both for military siege work and civilian"
		    "tasks such as road and bridge construction."))
)
(unit-type donkeys (name "Donkey Train") (help "Caravan of Donkeys")
	(notes ("Overland transport during the third and second millenium BC was mostly handled by Donkey"
		    "Trains. The Assyrians used to send heavily guarded trains of several thousand animals to their"
		    "trade factories in Anatolia."))
)
(unit-type caravan (name "Caravan") (help "Camel Train")
	(notes ("In the first millenium BC the recently domesticated Camel gradually replaced the Donkey as"
		    "freight animal due to its hardiness and superior strength."))
)

;	Infantry Units

(unit-type warriors (name "Warrior Band") (help "Ancient Fighters")
	(notes ("The Warrior Band was the first organized fighting unit. Poorly equipped, poorly commanded,"
		    "and poorly trained."))
)
( unit-type spearmen (name "Spearmen") (help "Spears with Shields")
	(notes ("The first infantry units to fight in close formation are shown on the Sumerian Stele of the"
		    "Vultures. They were armed with spears, leather helmets and large rectangular shields."))
)
(unit-type swordmen (name "Swordmen") (help "Men with Iron Swords")
	(notes ("Around 1200 BC the Middle East was overrun by the Sea Peoples, invaders from the North. "
		    "They were armed with a new superior weapon, the iron sword, and used a new tactic that"
		    "neutralized the Chariots."))
)
(unit-type slingers (name "Slingers") (help "Sling Throwers")
	(notes ("In the third millenium BC the main projectile weapon was the sling rather than the bow. "
		    "Lightly armed Slingers were more agile but also more vulnerable than the Spearmen."))    
)
(unit-type archers (name "Archers") (help "Bowmen")
	(notes ("The invention of the composite bow meant that Archers became an important military force"
		    "which quickly replaced the older Slinger units."))    
)

;	Mounted Units

(unit-type horsemen (name "Horsemen") (help "Archers on Horseback")
	(notes ("Prior to invention of the bit, Horsemen armed with bows were used as scouts and to harass"
		    "enemy troops. They operated in pairs, where one horseman would hold both reins while his"
		    "companion used the bow.")) 
)
(unit-type cavalry (name "Cavalry") (help "Horsemen with Lances")
	(notes ("The bit made it easier to control horses, and the Cavalry was born. Under Alexander, they"
	 	    "were armed with lances and were used to break up infantry lines in frontal charges."))
)
(unit-type elephants (name "Elephants") (help "Archer Platform")
	(notes ("Elephants were used in large numbers by the Carthagians. They carried towers with Archers "
		    "on their backs. Very effective against Cavalry, since most horses will panic at the sight of an"
		    "Elephant."))
)	
(unit-type war-carts (name "War Carts") (help "Javelin Platform")
	(notes ("Primitive War Carts are shown on the Sumerian Standard of Ur. They were heavy wagons with"
		    "solid wheels drawn by a team of four onagers, and served as platforms for javelin throwers."))    
)
(unit-type chariots (name "Chariots") (help "Archers on Wheels")
	(notes ("Horse-drawn Chariots dominated the battlefield in the second millenium BC. They served"
		    "as highly mobile platforms for archers equipped with powerful composite bows."))
)

;	Siege Units

(unit-type battering-ram (name "Battering Ram") (help "Gate Breaker")
	(notes ("The emergence of fortified cities necessitated the Battering Ram, which was used from the"
		    "time of Sargon the Great to break down city gates."))    
) 
(unit-type siege-tower (name "Siege Tower") (help "Tower on Wheels")
	(notes ("The Wheeled Siege Tower was developed by the Assyrians. Its main function was to protect"
	 	    "the Battering-Ram that was suspended within it."))    
) 

(unit-type ballista (name "Ballista") (help "Giant Crossbow")
	(notes ("The ballista was a gigantic crossbow mounted on a mobile platform. It was extensively used by" 
		    "Philip II during his wars in Greece."))  
)
(unit-type onager (name "Onager") (help "Big Catapult")
	(notes ("The Roman Onager was a catapult that could fire large rocks over long distances."
		    " Given enough time, it would reduce the strongest fortifications to rubble."))
)	             
(unit-type siege-ramp (name "Siege Ramp") (help "Artificial Mountain")
	(notes ("A Siege Ramp was sometimes the only way to conquer a heavily fortified position. The"
	             "gigantic Siege Ramp that the Romans built to take the clifftop fortress of Masada can still"
	             "be seen today."))
)	             

;	Naval Units

(unit-type reed-boat (name "Reed Boat") (help "Primitive Ship")
	(notes ("The keelless Reed Boat was used for river and coastal transport in Egypt and Mesopotamia."
		    " It had limited seaworthiness and could carry a small cargo besides its crew."))
)
(unit-type longship (name "Longship") (help "Ship of Ulysses")
	(notes ("The greek Longship or Pentekonter was a keeled seagoing vessel that could use both sails and"
		    " oars for propulsion. It typically had 50 oars, hence its name."))
) 	              
(unit-type barge (name "Barge") (help "Transport Ship")
	(notes ("The Barge was used for troop transport. It was slow and vulnerable."))
) 	              
(unit-type bireme (name "Bireme") (help "Two Rows of Oars")
	(notes ("The Bireme was a development of the Longship that had two rows of oars on each side. The"
		    "top row was supported by an outrigger."))
) 	              
(unit-type trireme (name "Trireme") (help "Three Rows of Oars")
	(notes ("The Bireme was soon replaced by the Trireme, with three rows of oars on each side. More"
		    "rows were impractical, so Triremes dominated naval warfare for several centuries."))
) 	              

;	Complex Units

(unit-type phalanx (name "Phalanx") (help "Heavy Infantry")
	(notes ("The greek Phalanx was made up of Hoplites, heavy infantrymen equipped with long pikes,"
		    "and more lightly armed Peltasts."))
)
(unit-type legion (name "Legion") (help "Roman Legion")
	(notes ("The immense success of the Roman Legion was due to its balanced composition of Infantry,"
		    "Cavalry and Engineers. It was the first truly integrated military unit that could carry out"
		    "both offensive, defensive and constructive tasks."))
)

;	Advanced Units

(unit-type tribe (name "Tribe")(image-name "tribe")(help "Nomadic Community")
	(notes ("The first human communities were nomadic tribes.")) 
)
(unit-type village (name "Village")(image-name "village")(help "Minor Settlement")
	(notes ("The first villages appeared with agriculture, which quickly transformed nomadic tribes into"
		    "settled communities. This process started in the Fertile Crescent around 8,000 BC.")) 
)
(unit-type city (name "City") (help "Major Settlement")
	(notes ("The first real City in human history was Sumerian Uruk, whose walls were built by its"
		    "legendary ruler Gilgamesh. Uruk is believed to have had as many as 50,000 inhabitants"
		    "at the start of the third millenium BC.")) 
)

;	Facility Units

(unit-type stockade (name "Stockade")(image-name "stockade"))
(unit-type city-walls (name "City Walls")(image-name "walltown"))
(unit-type temple (name "Temple")(image-name "parthenon"))
(unit-type workshops (name "Workshops")(image-name "village"))
(unit-type granary (name "Granary")(image-name "granary"))
;(unit-type marketplace (name "Marketplace")(image-name "camp"))
(unit-type school (name "Scribal School")(image-name "parthenon"))
;(unit-type forum (name "Forum")(image-name "parthenon"))
(unit-type ziggurat (name "Ziggurat")(image-name "nest"))
(unit-type palace (name "Palace")(image-name "parthenon"))

(define unarmed
	(colonizers corvees sappers donkeys caravan))

(define infantry
	(warriors spearmen swordmen slingers archers))
	
(define mounted
	(horsemen cavalry elephants war-carts chariots))
	
(define siege
	(battering-ram siege-tower ballista onager siege-ramp))

(define complex
	(phalanx legion))

(define places
	(tribe village city))

(define naval
  	(reed-boat longship barge bireme trireme))

(define ground
	(append unarmed infantry mounted siege complex))

(define movers
	(append naval ground))

(define armed
	(append infantry mounted siege complex naval))

(define facilities
	(granary temple school workshops stockade city-walls ziggurat palace
	; forum marketplace
	))



;;;	MATERIAL TYPES


(material-type food (name "Food")(image-name "sheaf")(resource-icon 1))
(material-type ores (name "Ores")(image-name "shield")(resource-icon 2))
(material-type ideas (name "Ideas")(image-name "arrows")(resource-icon 3))
; The idea behind cash is to add code for buying units etc. like in
; Civ2. But that wont happen anytime soon, so comment it out for now.
;(material-type cash (name "Cash")(resource-icon 4))



;;;	ADVANCE TYPES


; Note that many advances don't yet do anything.  This is intended to
; be a temporary state of affairs, to be fixed by adding units which
; use those advances.  The intent of this game is to have lots of
; advances (even more than now, especially at high levels).
; 
; Part of the challenge with this game is to figure out what advances
; (and units) are really important and what are dispensable. Just like
; in real life!

;	1st generation advances.

(advance-type barley (name "Barley")(help "Ancient Cereal")
	(notes ("Barley and Einkorn Wheat were the first two cereal crops cultivated by mankind. While Einkorn Wheat"
		    "quickly was replaced by tetraploid and hexaploid wheats, Barley has remained an important crop until"
		    "modern times."))
)
(advance-type carpentry (name "Carpentry")(help "Woodcraft")
	(notes ("Carpentry was one of the first human crafts that required specially trained artisans."))
)
(advance-type cattle (name "Cattle"))
(advance-type charcoal (name "Charcoal Burning"))
(advance-type clay-token (name "Clay Tokens")(help "Inscribed Clay Beads")
	(notes ("The clay token was used to keep track of sheep and other livestock in the fourth millenium BC. It was the"
		    "single most important invention in human history, for two reasons. First, the inscribed symbols on these"
		    "tokens evolved directly into the Sumerian pictograms, the first writing system. Second, clay tokens made"
		    "barter by distance possible, from which trade and other economic activities soon developed."))    
)		    

(advance-type einkorn (name "Einkorn Wheat")(help "Diplod Wheat")
	(notes ("Man's first crop was Einkorn Wheat which is a variant of the diploid wild wheat that grows in the Zagros"
		    "and Taurus mountains. It has a poor yield and is not used anywhere today."))
)
	
(advance-type elders-council (name "Elders Council"))
(advance-type flax (name "Flax"))
(advance-type handicraft (name "Handicraft"))
(advance-type hemp (name "Hemp"))
(advance-type idolatry (name "Idolatry"))
(advance-type leatherworking (name "Leatherworking"))
(advance-type mud-brick (name "Mud Bricks"))
(advance-type pottery (name "Pottery"))
(advance-type shaman (name "The Shaman"))
(advance-type sheep (name "The Sheep"))
(add (barley carpentry cattle charcoal clay-token einkorn
      elders-council flax handicraft hemp idolatry leatherworking
      mud-brick pottery shaman sheep) rp 10)

;	2nd generation advances.

(advance-type barter (name "Barter"))
(advance-type beer (name "Beer"))
(advance-type bellows (name "Bellows"))
(advance-type construction (name "Construction"))
(advance-type donkey (name "The Donkey"))
(advance-type emmer (name "Emmer Wheat")(help "Tetraploid Wheat")
	(notes ("Tetraploid Emmer Wheat originated as a species hybrid between Emmer Wheat and Goat Grass. It has"
		    "mostly been replaced by Modern Wheat today but a variant of Emmer Wheat still survives as Durum"
		    "Wheat which is used to make pasta."))
)
		    
(advance-type gardening (name "Gardening"))
(advance-type harness (name "The Harness"))
(advance-type kiln (name "The Kiln"))
(advance-type loom (name "The Loom"))
(advance-type pictograms (name "Pictograms"))
(advance-type potash (name "Potash"))
(advance-type potters-wheel (name "Potter's Wheel"))
(advance-type priesthood (name "Priesthood"))
(advance-type ropemaking (name "Ropemaking"))
(advance-type spindle (name "The Spindle"))
(advance-type tribal-law (name "Tribal Law"))
(advance-type the-village (name "The Village"))
(advance-type yoke (name "The Yoke"))
(add (barter beer bellows construction donkey emmer gardening harness
      kiln loom potash
      pictograms potters-wheel priesthood ropemaking spindle
      the-village tribal-law yoke) rp 20)

;	3rd generation advances.

(advance-type artisanry (name "Artisanry"))
(advance-type assembly (name "The Assembly"))
(advance-type boatbuilding (name "Boatbuilding"))
(advance-type canvas (name "Canvas"))
(advance-type ceramics (name "Ceramics"))
(advance-type cloth (name "Cloth"))
(advance-type festival (name "Festival"))
(advance-type furnace (name "The Furnace"))
(advance-type herbal-lore (name "Herbal Lore"))
(advance-type horse (name "The Horse"))
(advance-type irrigation (name "Irrigation"))
(advance-type market (name "The Market"))
(advance-type mining (name "Mining"))
(advance-type numbers (name "Numbers"))
(advance-type pulley (name "The Pulley"))
(advance-type shrine (name "The Shrine"))
(advance-type sledge (name "The Sledge"))
(advance-type syllabary (name "Syllabary"))
(advance-type trance (name "Trance"))
; Maybe we want to set advance-multiply-production
; and/or advance-add-production on food?
(advance-type wheat (name "Modern Wheat")(help "Hexaploid Wheat")
	(notes ("A second species hybridization between Emmer Wheat and Goat Grass created the hexaploid Modern"
		    "Wheat which is used in agriculture all over the world. It has a much better yield and hardiness than"
		    "the earlier diploid and tetraploid wheat species."))  
)
(advance-type wooden-plow (name "Wooden Plow"))
(advance-type solid-wheel (name "Solid Wheel"))
(add (artisanry assembly boatbuilding canvas ceramics cloth festival
      furnace herbal-lore horse irrigation market mining numbers pulley shrine
      sledge syllabary solid-wheel trance wheat wooden-plow) rp 40)

;	4th generation advances.

(advance-type astronomy (name "Astronomy"))
(advance-type camel (name "The Camel"))
(advance-type copper (name "Copper Melting"))
(advance-type cylinder-seal (name "Cylinder Seal"))
(advance-type divination (name "Divination"))
(advance-type dyeing (name "Dyeing"))
(advance-type glassmaking (name "Glassmaking"))
(advance-type joinery (name "Joinery"))
(advance-type landsurvey (name "Land Survey"))
(advance-type lime (name "Lime"))
(advance-type merchant (name "The Merchant"))
(advance-type metalworking (name "Metalworking"))
(advance-type river-transport (name "River Transport"))
(advance-type saddle (name "The Saddle"))
(advance-type sailing (name "Sailing"))
(advance-type stonecutting (name "Stonecutting"))
(advance-type temple-estate (name "The Temple Estate"))
(advance-type till-farming (name "Till-Farming"))
(advance-type township (name "Township"))
(advance-type wagon (name "The Wagon"))
(advance-type writing (name "Writing"))
(add (astronomy camel copper cylinder-seal divination dyeing glassmaking
      joinery landsurvey lime merchant
      metalworking river-transport saddle sailing stonecutting
      temple-estate till-farming township wagon writing) rp 80)

;	5th generation advances.

(advance-type arithmetic (name "Arithmetic"))
(advance-type astrology (name "Astrology"))
(advance-type bookkeeping (name "Book Keeping"))
(advance-type the-border (name "The Border"))
(advance-type bronze (name "Bronze"))
(advance-type calendar (name "Calendar"))
(advance-type contract (name "The Contract"))
(advance-type elephant (name "The Elephant"))
(advance-type ensi (name "The Ensi"))
(advance-type forge (name "The Forge"))
(advance-type leasehold (name "The Leasehold"))
(advance-type mapmaking (name "Map Making"))
(advance-type masonry (name "Masonry"))
; medicine would seem like a logical candidate for size-limit-without-advance
(advance-type medicine (name "Medicine"))
(advance-type navigation (name "Navigation"))
(advance-type omen-books (name "Omen Books"))
(advance-type scribe (name "The Scribe"))
(advance-type spoked-wheel (name "The Spoked Wheel"))
(add (arithmetic astrology bookkeeping
      the-border bronze calendar contract elephant ensi
      forge leasehold mapmaking masonry medicine navigation
      omen-books scribe spoked-wheel) rp 160)

;	6th generation advances.

(advance-type agriculture (name "Agriculture"))
(advance-type city-state (name "The City State"))
(advance-type code-of-laws (name "Code of Laws"))
(advance-type engineering (name "Engineering"))
(advance-type foundry (name "The Foundry"))
(advance-type iron (name "Iron"))
(advance-type letter (name "The Letter"))
(advance-type mathematics (name "Mathematics"))
(advance-type military (name "Military"))
(advance-type trade (name "Trade"))
(add (agriculture city-state code-of-laws
      engineering foundry iron letter mathematics
      military trade) rp 320)

;	7th generation advances.

(advance-type court (name "Court of Law"))
(advance-type diplomacy (name "Diplomacy"))
(advance-type geometry (name "Geometry"))
(advance-type kingship (name "Kingship"))
(advance-type literature (name "Literature"))
(advance-type road-building (name "Road Building"))
(advance-type taxation (name "Taxation"))
(advance-type the-bit (name "The Bit") (help "Advanced bridle for horses"))
(add (court diplomacy geometry kingship literature road-building taxation
      the-bit) rp 640)

;	8th generation advances.

(advance-type architecture (name "Architecture"))
(advance-type currency (name "Currency"))
(advance-type government (name "Government"))
(advance-type philosophy (name "Philosophy"))
(add (architecture currency government philosophy) rp 1280)

;	9th generation advances.

(advance-type elected-council (name "Elected Council"))
(advance-type judiciary (name "Judiciary"))
(add (elected-council judiciary) rp 2560)

;	10th generation advances.

; The sheer number of prerequisites for this one might mean that
; 5120 is a bit excessive.  But let's stick with the pattern (more
; of a sense of accomplishment if you actually do achieve it :-)).
(advance-type republic (name "The Republic"))
(add republic rp 5120)

;;; Next Research Goals
; Get Handicraft first. Slingers are useful for blasting defenders.
(set ai-initial-research-goals (handicraft))
; Always go to Leatherworking next. Spearmen are good defenders.
(add handicraft ai-next-goal (leatherworking)) 
; Always go to The Saddle next. Horsemen are decent mobile attackers.
(add leatherworking ai-next-goal (saddle))
; Always go to Joinery. Archers are more mobile and accurate than Slingers.
(add saddle ai-next-goal (joinery))
; Always go to Elephant. Elephants are powerful mobile attackers.
(add joinery ai-next-goal (elephant))
; Always go to Construction.
; This gives us City Walls, and paves the way for other advances.
(add elephant ai-next-goal (construction))
; Construction branches out.
; Temple Estate will give us Granaries (food) and Temples (ideas).
; Engineering will give us Siege Towers, War Carts, Chariots, Swordmen, 
;   and Onagers (excellent siege engines, range > 1).
(add construction ai-next-goal (temple-estate engineering))
; Temple Estate always goes to Military.
; Military will give us Swordmen (via Bronze),
;   and The Ensi, which is common to other important advances.
(add temple-estate ai-next-goal (military))
; Engineering always goes to Temple Estate.
; (We are now playing catch-up with the Construction -> Temple Estate path.)
(add engineering ai-next-goal (temple-estate))
; Military always goes to Taxation. Palaces increase Ores productivity by 100%.
(add military ai-next-goal (taxation))
; Taxation always goes to Government. Phalanxes are very tough and mobile.
(add taxation ai-next-goal (government))
; Government always goes to The Republic. Legions are ultimate.
; Siege Ramps also are enabled, but need Sappers to build them.
(add government ai-next-goal (republic))
; The Republic always goes to Road Building.
; This will allow us to go back and pick up Engineering, if necessary.
; This enables Sappers, so that Siege Ramps can be constructed.
(add republic ai-next-goal (road-building))
; Road Building branches out.
; The Bit gives us Cavalry.
; Navigation gives us Biremes.
(add road-building ai-next-goal (the-bit navigation))
; The Bit always goes to Navigation.
(add the-bit ai-next-goal (navigation))
; Navigation always goes to Trade. Triemes are the best naval vessel.
(add navigation ai-next-goal (trade))
; Trade always goes to The Bit.
(add trade ai-next-goal (the-bit))
; After this, the AI can clean up loose ends as it sees fit.

;;;	STARTUP CONDITIONS

; (add colonizers independent-near-start 5)


(table independent-density 
	(city plain 400)
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
(add places already-seen 100)
(add places already-seen-independent 100)
;(add places see-always 1)
(add colonizers start-with 3)
(scorekeeper (do last-side-wins))

(set ai-may-resign false)			;	Turn off automatic resignation.
(set side-can-research true)		;	Enable side-based research.
 

;;;	GENERAL UNIT PROPERTIES


(add tribe ai-minimal-size-goal 2)		;	Don't settle in places that cannot support a size 2 tribe.
(add village ai-minimal-size-goal 8)	;	Don't settle in places that cannot support a size 8 village.
(add city ai-minimal-size-goal 12)		;	Don't settle in places that cannot support a size 12 city.

(add places ai-peace-garrison 0)		;	Minimal number of defenders assigned to a city.
(add places ai-war-garrison 2)		;	Minimal number of defenders assigned to a city.

(add tribe reach 0)
(add village reach 1)
(add city reach 2)
(add city vision-range 2)

(add u* free-acp 1)				;	Makes defense possible.

(add places advanced true)			;	Places are advanced units.
(add places acp-independent true)		;	And they are also acp-independent.


;;;	 VOLUME AND NUMBER RESTRICTIONS


(table unit-size-as-occupant	;	Basic size of unit as occ.
	(u* u* 1)				;	Each unit takes up 1 capacity slot as occupant.
	(facilities places 0)		;	Facilities do not compete for space.
)

(add city capacity 24)			;	Each city has room for 24 occs.
(add village capacity 8)		;	Each village has room for 8 occs.
(add tribe capacity 2)			;	Each tribe has room for 2 occ.
(add reed-boat capacity 1)
(add barge capacity 4)

(add city occupant-total-max 24)		;	Max 20 occupants.
(add village occupant-total-max 8)	;	Max 8 occupants.
(add tribe occupant-total-max 2)		;	Max 2 occupants.
	
(add city facility-total-max 12)		;	Max 10 facilities.
(add village facility-total-max 4)	;	Max 2 facilities.
(add tribe facility-total-max 0)		;	Max 0 facilities.
	
(add city mobile-total-max 12)		;	Max 10 mobiles.
(add village mobile-total-max 4)		;	Max 6 mobiles.
(add tribe mobile-total-max 2)		;	Max 2 mobiles.
	
(table occupant-max			;	Extra limits on top of capacity!
	(u* places 0)			;	Places can't be occupants at all.
	(u* facilities 0)			;	Facilities can't be occupants in general.	
	(places facilities 1)		;	But places have room for exactly one facility of each type.	
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

(add colonizers acp-per-turn 1)
(add corvees acp-per-turn 1)
(add sappers acp-per-turn 2)
(add donkeys acp-per-turn 1)
(add caravan acp-per-turn 2)

(add warriors acp-per-turn 1)
(add spearmen acp-per-turn 2)
(add swordmen acp-per-turn 3)
(add slingers acp-per-turn 2)
(add archers acp-per-turn 3)

(add horsemen acp-per-turn 3)
(add cavalry acp-per-turn 4)
(add elephants acp-per-turn 3)
(add war-carts acp-per-turn 3)
(add chariots acp-per-turn 4)

(add battering-ram acp-per-turn 1)
(add siege-tower acp-per-turn 2)
(add ballista acp-per-turn 2)
(add onager acp-per-turn 3)
(add siege-ramp acp-per-turn 3)

(add phalanx acp-per-turn 4)
(add legion acp-per-turn 6)

(add reed-boat acp-per-turn 1)
(add longship acp-per-turn 1)
(add barge acp-per-turn 1)
(add bireme acp-per-turn 2)
(add trireme acp-per-turn 3)


(add u* speed 0)					;	Cities and facilities are immobile.

(add colonizers speed 100)
(add corvees speed 100)
(add sappers speed 100)
(add donkeys speed 200)
(add caravan speed 300)

(add warriors speed 100)
(add spearmen speed 100)
(add swordmen speed 100)
(add slingers speed 200)
(add archers speed 200)

(add horsemen speed 300)
(add cavalry speed 300)
(add elephants speed 200)
(add war-carts speed 200)
(add chariots speed 300)

(add battering-ram speed 100)
(add siege-tower speed 100)
(add ballista speed 100)
(add onager speed 100)
(add siege-ramp speed 0)

(add phalanx speed 200)
(add legion speed 200)

(add reed-boat speed 100)
(add longship speed 200)
(add barge speed 100)
(add bireme speed 300)
(add trireme speed 400)

(table mp-to-enter-terrain 
	(u* t* 99)
	(ground land 1)
	(ground river 1)
	(ground forest 1)
	(ground mountain 2)
	(ground swamp 2)
	(ground salt-marsh 2)
  	(naval water 1)
)

(table mp-to-traverse
	(u* (river road) 99)
	(naval river 2)
	(ground road 1)
)

(add u* free-mp 1)			;	Let units with only 1 mp cross rivers etc.



;;; 	COMBAT


(add u* hp-max 1)

(add colonizers hp-max 1)
(add corvees hp-max 1)
(add sappers hp-max 2)
(add donkeys hp-max 1)
(add caravan hp-max 1)

(add warriors hp-max 1)
(add spearmen hp-max 3)
(add swordmen hp-max 3)
(add slingers hp-max 2)
(add archers hp-max 2)

(add horsemen hp-max 2)
(add cavalry hp-max 3)
(add elephants hp-max 4)
(add war-carts hp-max 2)
(add chariots hp-max 3)

(add battering-ram hp-max 1)
(add siege-tower hp-max 2)
(add ballista hp-max 1)
(add onager hp-max 2)
(add siege-ramp hp-max 4)

(add phalanx hp-max 6)
(add legion hp-max 8)

(add reed-boat hp-max 1)
(add longship hp-max 2)
(add bireme hp-max 3)
(add trireme hp-max 4)

(table damage
	(u* u* 1)			;	Units lose one hp for each hit.
	(u* places 0)		;	Places are not damaged. (But they can be captured!)
	(u* facilities 0)		;	Nor are facilities. (But they can be captured!)
)

(add ballista acp-to-fire 1)
(add onager acp-to-fire 1)

(add ballista range 1)
(add onager range 2)

(add ballista range-min 1)
(add onager range-min 1)

(table acp-to-attack		;	This refers to normal attacks, not fire or capture. However,
	(ballista u* 0)		;	setting it to null blocks capture attempts through one_attack!
	(onager u* 0)		;	This is because check_attack_action checks acp_to_attack.	
	(facilities u* 0)
	(places u* 0)
	(ground naval 0)
	(naval ground 0)
	(unarmed u* 0)

)

(table acp-to-capture
	(u* u* 0)
	(battering-ram places 1)
	(siege-tower places 1)
	(siege-ramp places 1)
;	(ballista places 1)
;	(onager places 1)
)


(table hit-chance	;	Note: zero by default!

;              					col	cor	sap	don	car	war	spe	swo	sli	arc	hor	cav	ele	crt	cha	bat	tow	cat	eng	rmp	pha	leg
;  ------------------------------------------------------------------------------------------------------------------

  (warriors ground 		(	20	20	10	20	20	20	10	10	10	10	10	10	5	10	5	20	10	20	20	0	5	5	))
  (spearmen ground 		(	30	30	20	30	30	30	20	20	20	20	20	20	10	20	10	30	20	30	30	0	10	10	))
  (swordmen ground 		(	40	40	30	40	40	40	30	30	30	30	30	30	20	50	40	40	30	40	40	0	20	20	))
  (slingers ground 		(	30	30	20	30	30	30	20	20	20	20	20	20	10	20	10	30	20	30	30	0	10	10	))
  (archers ground 		(	40	40	30	40	40	40	30	30	30	30	30	30	20	30	20	40	30	40	40	0	20	20	))

  (horsemen ground 		(	50	50	40	50	50	50	40	40	40	40	40	40	5	40	40	50	40	50	50	0	10	10	))
  (cavalry ground 		(	60	60	50	60	60	60	50	50	50	50	50	50	10	50	50	60	50	60	60	0	20	20	))
  (elephants ground 		(	70	70	60	70	70	70	60	60	60	60	60	60	40	60	60	70	60	70	70	0	30	30	))
  (war-carts ground 		(	40	40	30	40	40	40	30	20	30	30	30	30	5	30	30	40	30	30	30	0	20	20	))
  (chariots ground 		(	60	60	50	60	60	60	50	30	50	50	50	50	10	50	50	60	50	50	50	0	30	30	))

  (ballista ground 		(	30	30	30	30	30	30	30	30	30	30	30	30	30	30	30	30	30	30	30	10	30	30	))
  (onager ground 			(	40	40	40	40	40	40	40	40	40	40	40	40	40	40	40	40	40	40	40	20	40	40	))

  (phalanx ground 		(	80	80	70	80	80	80	70	60	70	70	70	70	60	70	60	80	70	80	80	10	70	60	))
  (legion ground 			(	90	90	80	90	90	90	80	70	80	80	80	80	70	80	70	90	80	90	90	20	80	70	))

;  ------------------------------------------------------------------------------------------------------------------

 
; 					ree	bar	lon	bir	tri
;  ------------------------------------------

  (reed-boat naval	(	40	50	30	20	10	))
  (barge naval		(	0	0	0	0	0	))
  (longship naval		(	50	60	40	30	20	))
  (bireme naval		(	60	70	50	40	30	))
  (trireme naval		(	70	80	60	50	40	))

;  ------------------------------------------ 
 
 
  (armed places 100)
  (armed facilities 100)
  (unarmed u* 0)
  (sappers siege-ramp 40)
  (places u* 0)
  (facilities u* 0)
  (battering-ram movers 5)
  (siege-tower movers 10)
  (siege-ramp movers 20)
  (battering-ram places 30)
  (siege-tower places 50)
  (siege-ramp places 80)
  (battering-ram city-walls 20)
  (siege-tower city-walls 40)
  (siege-ramp city-walls 60)
  (ground naval 0)
  (naval ground 0)
)

(table cellwide-protection-for		;	100 (no protection) by default!
	(armed places 0)			;	Places can't be captured as long as they have armed defenders.
	(naval places 100)			;	But ships in harbor don't count as defenders.
	(places facilities 0)			;	Facilities can't be captured as long as they are in a friendly place.	
)

(table cellwide-protection-against	;	100 (no protection) by default!
	(stockade u* 75)			;	Stockade reduces hit chance for unit to 75%.
	(stockade ballista 100)		;	But they are not effective against ballistas.
	(stockade onager 100)			;	Nor against siege engines.
	(city-walls u* 50)			;	City walls reduces hit chance for unit to 50%.
	(city-walls ballista 100)		;	But they are not effective against ballistas.
	(city-walls onager 100)		;	Nor against siege engines.
)

(table constructor-absorbed-by
    (colonizers places true)
)
			
(table hp-to-garrison				;	Defaults to zero!
	(colonizers places 1)			;	Colonizer is consumed when founding a city.
)

(table capture-chance				;	independent-capture-chance defaults to this!

	(warriors places 20)
	(spearmen places 20)
	(swordmen places 20)
	(slingers places 20)
	(archers places 20)

	(horsemen places 20)
	(cavalry places 20) 
	(elephants places 20)
	(war-carts places 20)
	(chariots places 20)

	(battering-ram places 30)
	(siege-tower places 50)
	(siege-ramp places 80)
	(ballista places 30)
	(onager places 50)

	(reed-boat places 20)
	(barge places 0)
	(longship places 20)
	(bireme places 20)
	(trireme places 20)

	(phalanx places 40)
	(legion places 60)
)



;;;	BUILDING AND CONSTRUCTION


; This did not work well -it took forever to make the first few units. HR.

#|
(add u* cp 50)  ; Most units are this much
(add horsemen cp 100)
(add (cavalry war-carts chariots) cp 200)
(add elephants cp 400)
(add phalanx cp 600)
(add legion cp 800)

(add places cp 1)			;	Must be 1 to permit colonizers to build them.
(add (stockade granary temple workshops) cp 100) ; marketplace
(add (city-walls ziggurat school palace) cp 300) ; forum
|#

(add u* cp 10)				;	Most units cost 10 cp.
(add places cp 1)			;	Must be 1 to permit colonizers to build them.
(add facilities cp 20)			;	Facilities cost 20 cp.

(table advance-needed-to-build

	(colonizers a* false)
	(warriors a* false)
	(tribe a* false)
	(village a* false)
	(city a* false)

	(slingers handicraft true)
	(spearmen leatherworking true)
	(donkeys donkey true)
	(battering-ram ropemaking true)
	(reed-boat boatbuilding true)
	(ballista pulley true)
	(siege-tower sledge true)
	(caravan camel true)
	(archers joinery true)
	(barge river-transport true)
	(horsemen saddle true)
	(longship sailing true)
	(war-carts wagon true)
	(elephants joinery true)
	(elephants elephant true)
	(swordmen bronze true)
	(bireme navigation true)	
	(cavalry saddle true)
	(chariots spoked-wheel true)
	(onager engineering true)
	; Requiring navigation for the trireme is a bit of a hack,
	; a more elegant solution would be a higher level advance
	; which follows on from navigation.   But we need this for
	; now, trade doesn't require canvas, sailing, &c.
	(trireme navigation true)
	(trireme trade true)
	(siege-ramp geometry true)
	(sappers road-building true)
	(cavalry the-bit true)
	(phalanx government true)
	(legion republic true)

	(stockade a* false)
	(workshops artisanry true)
;	(marketplace market true)
;	(forum architecture true)
	(temple shrine true)
	(ziggurat stonecutting true)
	(granary temple-estate true)
	(school scribe true)
	(city-walls construction true)
	(palace taxation true)
)

(table can-create
	(places u* 1)			;	Places can create units.
	(places places 0)		;	But not other places.
	(places siege-ramp 0)	; 	Or siege ramps.
	(colonizers city 1)		;	Colonizers can only create city.
	(sappers siege-ramp 1)	;	Sappers can create siege ramps.
)

(table can-build
	(places u* 1)			;	Places can build units.
	(places places 0)		;	But not other places.
	(places siege-ramp 0)	; 	Or siege ramps
	(colonizers city 1)		;	Colonizers can only build city.
	(sappers siege-ramp 1)	;	Sappers can build siege ramps.
)

(table acp-to-create			;	Defaults to zero!
	(places u* 1)			;	Places can create units.
	(places places 0)		;	But not other places.
	(places siege-ramp 0)	; 	Or siege ramps.
	(colonizers city 1)		;	Colonizers can only create city.
	(sappers siege-ramp 1)	;	Sappers can create siege ramps.
)

(table acp-to-build			;	Defaults to zero!
	(places u* 1)			;	Places can build units.
	(places places 0)		;	But not other places.
	(places siege-ramp 0)	; 	Or siege ramps
	(colonizers city 1)		;	Colonizers can only build city.
	(sappers siege-ramp 1)	;	Sappers can build siege ramps.
)

(table create-range			;	Defaults to zero!
	(places u* 1)			;	Places can create units also in adjacent cells.
	(places facilities 0)		;	But facilities have to be in the same cell.
)

(table build-range			;	Defaults to zero!
	(places u* 1)			;	Places can build units also in adjacent cells.
	(places facilities 0)		;	But facilities have to be in the same cell.
)

(table can-change-type-to 0
    (tribe village true)
    (village city true)
)

(table acp-to-change-type 
    (u* u* 0)
    (tribe village 1)
    (village city 1)
)



;;;	MATERIAL PRODUCTION AND CONSUMPTION


;(add cash treasury true)			;	Cash is globally available in the treasury.
(add ideas treasury true)			;	Necessary for side-based research.
	
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
;	((plain) cash 2)
	((plain) ideas 2)
	((sea lake) ideas 3)
)

; Don't set this to 2, or the AI won't create any cities.
(table unit-consumption-per-size	;	Defaults to zero!
	; Maintenance costs this many food units per size unit.
	(places food 1)
)

(table unit-consumption-to-grow	;	Defaults to zero!
	; Size increase costs this many stored food units per size unit.
	(places food 15)
)

(table consumption-per-cp		;	Defaults to zero!
	(u* ores 1)				;	1 cp costs 1 ores unit.
	(city m* 0)
)

(table advance-consumption-per-rp	;	Defaults to zero!
	(a* ideas 1)				;	1 rp costs 1 ideas unit.
)

(table occupant-multiply-production
	(granary food 150)			;	Temple increases food production by 50%.
	(ziggurat food 200)			;	Ziggurat increases food production by 100%
	(temple ideas 150)			;	School increases ideas production by 50%.
	(school ideas 200)			;	School increases ideas production by 100%.
	(workshops ores 150)		;	Workshops increase ores production by 50%.
	(palace ores 200)			;	Palace increase ores production by 100%.
;	(marketplace cash 150)		;	Marketplace increases cash production by 50%
;	(forum cash 200)			;	Forum increases cash production by 100%
)



;;;	RESEARCH


(table advance-needed-to-research

	(a* a* false)

;	1st generation advances (16).

	(barley a* false)
	(carpentry a* false)
	(cattle a* false)
	(charcoal a* false)
	(clay-token a* false)
	(einkorn a* false)
	(elders-council a* false)
	(flax a* false)
	(handicraft a* false)						;	slingers
	(hemp a* false)
	(idolatry a* false)
	(leatherworking a* false)					;	spearmen
	(mud-brick a* false)
	(pottery a* false)
	(shaman a* false)
	(sheep a* false)

;	2nd generation advances (19).

	(barter (clay-token sheep) true)
	(beer (barley pottery) true)
	(bellows (carpentry leatherworking) true)
	(construction (carpentry mud-brick) true)		;	city-wall
	(donkey (cattle) true)						;	donkeys
	(emmer (einkorn) true)
	(gardening (flax hemp) true)
	(harness (cattle leatherworking) true)
	(kiln (charcoal mud-brick) true)
	(loom (carpentry sheep) true)
	(pictograms (clay-token shaman) true)
	(potash (charcoal pottery) true)
	(potters-wheel (carpentry pottery) true)
	(priesthood (idolatry shaman) true)
	(ropemaking (handicraft hemp) true)			;	battering-ram
	(spindle (handicraft sheep) true)
	(the-village (elders-council mud-brick) true)
	(tribal-law (elders-council idolatry) true)
	(yoke (carpentry cattle) true)

;	3rd generation advances (22).

	(artisanry (barter the-village) true)			;	workshops
	(assembly (tribal-law the-village) true)
	(boatbuilding (carpentry ropemaking) true)		;	reed-boat
	(canvas (flax loom) true)
	(ceramics (kiln potters-wheel) true)
	(cloth (spindle loom) true)
	(festival (beer priesthood) true)
	(furnace (bellows kiln) true)
	(herbal-lore (gardening shaman) true)
	(horse (donkey) true)	
	(irrigation (construction gardening) true)
	(market (barter construction) true)			;	marketplace
	(mining (construction ropemaking) true)
	(numbers (barter pictograms) true)
	(pulley (spindle ropemaking) true)			;	ballista
	(shrine (construction priesthood) true)		; 	temple
	(sledge (yoke ropemaking) true)				;	siege-tower
	(syllabary (pictograms priesthood) true)
	(trance (beer hemp) true)
	(wheat (emmer) true)
	(wooden-plow (harness yoke) true)
	(solid-wheel (donkey potters-wheel) true)

;	4th generation advances (21).

	(astronomy (shrine syllabary) true)
	(camel (horse) true)						;	caravan
	(copper (furnace mining) true)
	(cylinder-seal (ceramics syllabary) true)
	(divination (priesthood trance) true)
	(dyeing (cloth herbal-lore) true)
	(glassmaking (ceramics potash) true)
	(joinery (artisanry carpentry) true)			;	archers
	(landsurvey (numbers ropemaking) true)
	(lime (kiln mining) true)
	(merchant (cloth market) true)
	(metalworking (artisanry furnace) true)
	(river-transport (boatbuilding pulley) true)	;	barge
	(saddle (horse leatherworking) true)			;	horsemen			
	(sailing (boatbuilding canvas) true)			;	longship
	(stonecutting (artisanry mining) true)			;	ziggurat
	(temple-estate (festival shrine) true)			;	granary
	(till-farming (irrigation wooden-plow) true)
	(township (assembly market) true)
	(wagon (sledge solid-wheel) true)			;	war cart
	(writing (artisanry syllabary) true)

;	5th generation advances (18).
	
	(arithmetic (numbers writing) true)
	(astrology (astronomy divination) true)
	(bookkeeping (numbers merchant) true)
	(the-border (landsurvey township) true)
	(bronze (copper metalworking) true)
	(calendar (astronomy writing) true)
	(contract (cylinder-seal writing) true)
	(elephant (camel) true)					;	elephants
	(ensi (temple-estate township) true)
	(forge (kiln metalworking) true)
	(leasehold (till-farming landsurvey) true)
	(mapmaking (landsurvey writing) true)
	(masonry (lime stonecutting) true)
	(medicine (divination herbal-lore) true)
	(navigation (astronomy sailing) true)			;	bireme
	(omen-books (divination writing) true)
	(scribe (temple-estate writing) true)			;	school
	(spoked-wheel (joinery wagon) true)			;	chariots	

;	6th generation advances (10).

	(agriculture (calendar leasehold) true)
	(city-state (the-border ensi) true)
	(code-of-laws (ensi contract) true)
	(engineering (bronze spoked-wheel) true)		;	onager
	(foundry (bronze forge) true)
	(iron (bronze forge) true)
	(letter (calendar writing) true)
	(mathematics (arithmetic calendar) true)		
	(military (bronze ensi) true)				;	swordmen
	(trade (contract mapmaking) true)			;	trireme

;	7th generation advances (8).

	(court (city-state code-of-laws) true)
	(diplomacy (letter military) true)
	(geometry (mapmaking mathematics) true)		;	siege-ramp
	(kingship (city-state military) true)
	(literature (letter scribe) true)
	(road-building (engineering military) true)		;	sappers
	(taxation (city-state military) true)			;	palace
	(the-bit (iron saddle) true)				;	cavalry

;	8th generation advances (4).

	(architecture (engineering kingship) true)		;	forum
	(currency (kingship trade) true)
	(government (kingship taxation) true)			;	phalanx
	(philosophy (geometry literature) true)
	
;	9th generation advances (2).

	(elected-council (government philosophy) true)
	(judiciary (philosophy court) true)

;	10th generation advances (1).

	(republic (elected-council judiciary) true)		;	legion

)


;;;	SIDES


(set side-library '(

  ((name "Akkad") 	
    (plural-noun "Akkadians")	
    (adjective "Akkadian")	
    (unit-namers (tribe "akkadian-names")
    			(village "akkadian-names")
    			(city "akkadian-names")))
        
  ((name "Assyria") 	
    (plural-noun "Assyrians")	
    (adjective "Assyrian")	
    (unit-namers (tribe "assyrian-names")
    			(village "assyrian-names")
    			(city "assyrian-names")))
    
  ((name "the Aztecs") 	
    (plural-noun "Aztecs")	
    (adjective "Aztec")	
    (unit-namers (tribe "aztec-names")
    			(village "aztec-names")
    			(city "aztec-names")))
    
  ((name "Babylonia") 	
    (plural-noun "Babylonians")	
    (adjective "Babylonian")	
    (unit-namers (tribe "babylonian-names")
    			(village "babylonian-names")
    			(city "babylonian-names")))
    
  ((name "Elam") 	
    (plural-noun "Elamites")	
    (adjective "Elamite")	
    (unit-namers (tribe "elamite-names")
    			(village "elamite-names")
    			(city "elamite-names")))

  ((name "Achaea")
    (plural-noun "Achaeans")		
    (adjective "Achaean")		
    (unit-namers (tribe "greek-names")
    			(village "greek-names")
    			(city "greek-names")))

  ((name "Hatti") 	
    (plural-noun "Hittites")	
    (adjective "Hittite")	
    (unit-namers (tribe "hittite-names")
    			(village "hittite-names")
    			(city "hittite-names")))
    
  ((name "Lower Egypt")		
    (plural-noun "Egyptians")		
    (adjective "Egyptian")		
    (unit-namers (tribe "lower-egyptian-names")
    			(village "lower-egyptian-names")
    			(city "lower-egyptian-names")))

  ((name "the Mayas") 	
    (plural-noun "Mayas")	
    (adjective "Mayan")	
    (unit-namers (tribe "mayan-names")
    			(village "mayan-names")
    			(city "mayan-names")))
    
  ((name "Mitanni") 	
    (plural-noun "Mitannis")	
    (adjective "Mitanni")	
    (unit-namers (tribe "mitanni-names")
    			(village "mitanni-names")
    			(city "mitanni-names")))
    
  ((name "Persia") 	
    (plural-noun "Persians")	
    (adjective "Persian")	
    (unit-namers (tribe "persian-names")
    			(village "persian-names")
    			(city "persian-names")))
    
  ((name "Phoenicia")		
    (plural-noun "Phoenicians")		
    (adjective "Phoenician")		
    (unit-namers (tribe "phoenician-names")
    			(village "phoenician-names")
    			(city "phoenician-names")))

  ((name "Sumer") 	
    (plural-noun "Sumerians")	
    (adjective "Sumerian")	
    (unit-namers (tribe "sumerian-names")
    			(village "sumerian-names")
    			(city "sumerian-names")))
  
  ((name "Upper Egypt")		
    (plural-noun "Egyptians")		
    (adjective "Egyptian")		
    (unit-namers (tribe "upper-egyptian-names")
    			(village "upper-egyptian-names")
    			(city "upper-egyptian-names"))) 

))


;;;	NAMING


(namer akkadian-names (random
  "Akkad" "Kazallu" "Borsippa" "Sippar" "Akshak" "Larak" "Dilbat" "Me-Turan"
  "Awal" "Tutub" "Neribtum" "Mashkan-Shapir" "Marad" "Eresh" "Lagaba" "Girtab"))

(namer assyrian-names (random
  "Assur" "Nineveh" "Arbil" "Kalhu" "Arrapha" "Nuzi" "Dur-Sharrukin" "Ekallatum"
  "Apku" "Shubat-Enlil" "Tarbisu" "Imgur-Bel" "Kakzu" "Simurrum" "Sinjar" "Samsat"))

(namer aztec-names (random
  "Tenochtitlan" "Teotihuacan" "Tollan" "Toluca" "Cuetlaxtlan" "Zaachila" "Yagul" "Xochonocho"
  "Zapatitlan" "Utatlan" "Cuzcatlan" "Tlaxcala" "Cempoallan" "Chalco" "Atitlan" "Cholua"))

(namer babylonian-names (random
  "Babil" "Isin" "Mari" "Eshnunna" "Rapiqu" "Malgium" "Dur-Kurigalzu" "Larsa"
  "Shaduppum" "Tuttul" "Kutha" "Haradum" "Ana" "Terqa" "Idu" "Apiak"))

(namer elamite-names (random
  "Susa" "Anshan" "Awan" "Hamazi" "Kabnak" "Dur-Untash" "Adamdum" "Aratta"
  "Urua" "Hupshen" "Der" "Alman" "Zahara" "Harhar" "Tulaspid" "Kurangan"))

(namer hittite-names (random
  "Hattusas" "Kushara" "Kanish" "Ankuwa" "Purushkanda" "Ataniya" "Tuwana" "Komana"
  "Tilgarim" "Melid" "Urshum" "Durhumit" "Shamuha" "Ulama" "Hubishna" "Tuhpiya"))

(namer mayan-names (random
  "Tikal" "Palenque" "Copan" "Chichen-Itza" "Calakmul" "Peten" "Uxmal" "Seibal"
  "Yaxchilan" "Coba" "Uaxactun" "Quirigua" "Bonampak" "Mayapan" "Sayil" "Tulum"))

(namer mitanni-names (random
  "Washukanni" "Ebla" "Harran" "Kargamish" "Ugarit" "Alalakh" "Qadesh" "Tadmor"
  "Yamhad" "Qatna" "Ashnakkum" "Emar" "Urkish" "Nihriya" "Elahut" "Abattum"))

(namer upper-egyptian-names (random
  "Waset" "Abedju" "Nekhen" "Gebtu" "Khmun" "Ta-Senet" "Nekheb" "Miam"
  "Napata" "Nubt" "Iunu" "Tantere" "Ipu" "Qis" "Buhen" "Pselqet"))

(namer lower-egyptian-names (random
  "Mennufer" "Bast" "Djannet" "Zau" "Busiris" "Per-Wadjit" "Khem" "Imu"
  "Tjebnutjer" "Per-Banebdjedet" "Behdet" "Mefket" "Rakote" "Tjeku" "Bah" "Hebyt"))

(namer persian-names (random
  "Pasargadae" "Persepolis" "Ekbatana" "Hormuz" "Marakanda" "Taxila" "Phrada" "Bagistane"
  "Gabae" "Thabsacus" "Rhegae" "Margiane" "Kyreschata" "Zariaspa" "Aornos" "Tabae"))

(namer phoenician-names (random
  "Tyros" "Sidon" "Byblos" "Arvad" "Karthago" "Gades" "Utica" "Leptis"
  "Sabrata" "Kition" "Sarepta" "Simurru" "Tingis" "Mogador" "Amrit" "Karalis"))

(namer sumerian-names (random
  "Uruk" "Ur" "Eridu" "Kish" "Adab" "Nippur" "Shuruppak" "Bad-Tibara"
  "Kisiga" "Umma" "Lagash" "Sirara" "Puzrish-Dagan" "Zabalam" "Girsu" "Kutalla"))

(namer greek-names (random
  "Mykene" "Thebe" "Athen" "Argos" "Miletos" "Samos" "Ilion" "Ephesos"
  "Halikarnassos" "Megara" "Dodona" "Elis" "Aigion" "Pherai" "Orchomenos" "Delphi"))

(namer independent-names (random
  "Karkar" "Kisurra" "Kutu" "Kesh" "Gasur" "Shusharra" "Karana" "Qatara" 
  "Basidqi" "Hasanlu" "Tushpa" "Kahat" "Tuttul" "Hit" "Kurruhani" "Qabra" 
  "Talmusa" "Hindanu" "Saggaratum" "Razama" "Tabate" "Zalpa" "Badna" "Halpi" 
  "Urshu" "Hahhum" "Markash" "Kesh" "Washushana" "Hurama" "Timelkiya" "Yabliya"
  "Haleb" "Hama" "Mero‘" "Nashala" "Akhetaten" "Djeba" "Djew-Qa" "Hut-Sekhem"
  "Menat-Khufu" "Per-Hathor" "Swenet" "Tjamet" "Zawty" "Tjenu" "Itjtawy"
  "Per-Medjed" "Per-Sopdu" "Pi-Riamsese" "Henen-Nesut" "Sile" "Teudjoi" "Karahna"
  "Kummanni" "Mahama" "Tawiniya" "Washhaniya" "Kistan" "Kanesh" "Aigina" 
  "Ambrakia" "Amyklai" "Chalkis" "Chios" "Heraia" "Ithaka" "Kalydon" "Karystos" 
  "Knidos" "Knossos" "Korinthos" "Kos" "Kydonia" "Kythera" "Larisa""Leukas" "Lyktos" 
  "Melos" "Mytilene" "Naxos" "Orcos" "Patrai" "Phaistos" "Praisos" "Pylos" "Rhodos" 
  "Samothrake" "Smyrna" "Stratos" "Sybrita""Thera" "Tiryns"))

(add tribe namer "independent-names")
(add village namer "independent-names")
(add city namer "independent-names")

;;  Use short generic names if we run out of cities.

(include "ng-weird")
(set default-namer "short-generic-names")

(add colonizers image-name "ancient-colonizer")
(add corvees image-name "ancient-corvee")
(add sappers image-name "ancient-sapper")
(add donkeys image-name "ancient-donkey")
(add caravan image-name "ancient-caravan")

(add warriors image-name "ancient-warrior")
(add spearmen image-name "ancient-spearman")
(add swordmen image-name "ancient-swordman")
(add slingers image-name "ancient-slinger")
(add archers image-name "ancient-archer")

(add horsemen image-name "ancient-horseman")
(add cavalry image-name "ancient-cavalry")
(add elephants image-name "ancient-elephant")
(add war-carts image-name "ancient-war-cart")
(add chariots image-name "ancient-chariot")

(add battering-ram image-name "ancient-battering-ram")
(add siege-tower image-name "ancient-siege-tower")
(add ballista image-name "ancient-ballista")
(add onager image-name "ancient-onager")
(add siege-ramp image-name "ancient-siege-ramp")

(add reed-boat image-name "ancient-reed-boat")
(add longship image-name "ancient-longship")
(add barge image-name "ancient-barge")
(add bireme image-name "ancient-bireme")
(add trireme image-name "ancient-trireme")

(add phalanx image-name "ancient-phalanx")
(add legion image-name "ancient-legion")
(add city image-name "ancient-city")
