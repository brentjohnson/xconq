(game-module "valhalla"
  (title "Valhalla")
  (version ".1")

#| BACKSTORY

Once upon a time, men with guns and tanks and APCs and jet fighters and battleships tried to kill each other.

|#

;; From .1: 

;; PLANNED IMPROVEMENTS:
;;

  (blurb "Ruleset for platoon-ish level fighting.")
  (variants
     (sequential true)
     (world-seen true)
     (see-all false)
     (world-size (100 100 360))
        ("30 Infantry" thirty-inf
	    "25 Infantry, 5 Engineers." false
	    (true
		(add infantry start-with 25)
;;		(add para start-with 5)
		(add engineer start-with 5)

		(add u* start-with 0)                
            )
        )

;; 30 INFANTRY

        ("30 Armor" thirty-armor
	    "20 Tanks, 10 APCs." false
	    (true
		(add tank start-with 20)
		(add apc start-with 10)                
            )
        )

;; 30 ARMOR

        ("10 Artillery" ten-arty
	    "10 Artillery." false
	    (true
		(add arty start-with 10)
            )
        )

;; 10 ARTILLERY


        ("20 Air" twenty-air
	    "10 fighters, 9 Choppers, 1 AWACS." false
	    (true
		(add heli start-with 9)
		(add fighter start-with 10)
		(add awacs start-with 1)
		(add hanger start-with 5)
            )
        )

;; 20 AIR

        ("5 SAM" five-sam
	    "5 SAMs." false
	    (true
		(add sam start-with 5)
            )
        )

;; 5 SAM

        ("4 Bombers" four-bomber
	    "4 Bombers." false
	    (true
		(add bomber start-with 4)
		(add para start-with 15)
		(add hanger start-with 6)
            )
        )

;; 4 BOMBERS


        ("Victory Points" vic-points
	    "Scattered throughout the map are victory points.  Whoever controls the most after 20 turns will win." false
	    (true
		(table independent-density
			(burb outskirts 50.00)
			(urban city 50.00)
			(inactive-vp land-t* .50)
		)
;;	    (set last-turn 30)
;;	    (set extra-turn-chance 90)
            )
        )

;; VICTORY POINTS



        ("Cities" cities
	    "Experimental urban combat, the AI hates buildings--even when its own troops occupy them." false
	    (true

(define city-ter (sea shallows swamp outskirts city land desert steppe forest mountains ice))

(add city-ter alt-percentile-min 	(0  5  0  15  15  15  15  15  15  95  99))
(add city-ter alt-percentile-max 	(5  10  15  95  95  95  95  95  95  99  100))
(add city-ter wet-percentile-min 	(0  0  0  0  10	 15  40	 60  80  0  0))
(add city-ter wet-percentile-max 	(70  80  100  10  15  40  60  80  100  100  100))

            )
        )



;; CITY COMBAT

        ("Navies" navies
	    "Huge fleets and a wet world.  Overrides all other unit choices" false
	    (true

(define nav-ter (sea shallows swamp outskirts city land desert steppe forest mountains ice))

(add nav-ter alt-percentile-min 	(0  60  68  70  70  70  70  70  70  98  99))
(add nav-ter alt-percentile-max 	(60  68  70  98  98  98  98  98  98  99  100))
(add nav-ter wet-percentile-min 	(0  	0    0   0  10	15  40	 60  87  0  0))
(add nav-ter wet-percentile-max 	(100  100  100  10  15  40  60  87  100  100  100))

(add battleship start-with 2)
(add carrier start-with 2)
(add escarrier start-with 3)
(add aegis start-with 6)
;; (add sub start-with 2)
(add supply start-with 1)
(add transport start-with 6)
(add bomber start-with 3)
(add fighter start-with 14)
(add heli start-with 6)
(add awacs start-with 1)
(add hanger start-with 1)

(add infantry start-with 10)
(add para start-with 5)
(add engineer start-with 3)

(add tank start-with 6)
(add apc start-with 3)
(add arty start-with 3)
(add sam start-with 2)

            )
        )



;; NAVIES

        ("Resupply Often" navies
	    "Units carry less ammo." false
	    (true

(table unit-storage-x
  (tank anti-tank 7)
  (tank anti-per 3)

  (arty anti-per 10)
  (arty anti-tank 3)

  (sam surface-air-missile 3)

  (apc anti-tank 2)
  (apc anti-per 7)

  (truck anti-tank 100)
  (truck anti-per 100)
  (truck surface-air-missile 100)

  (fighter aams 2)
  (fighter mavs 2)
  (fighter clusters 2)
  (fighter anti-ship 2)

  (bomber mk82 16)
 
  (heli mavs 4)
  (heli aams 2)

  (hanger mavs 50)
  (hanger clusters 50)
  (hanger aams 50)
  (hanger mk82 50)

  (carrier mavs 50)
  (carrier clusters 50)
  (carrier aams 50)

  (escarrier mavs 30)
  (escarrier clusters 30)
  (escarrier aams 30)

  (aegis mavs 15)
  (aegis aams 10)

  (sub torps 4)

  (supply cruise 100)
  (supply big-guns 100)
  (supply surface-air-missile 100)
  (supply anti-sub 100)
  (supply torps 100)

  (battleship big-guns 10)

  (aegis cruise 4)
  (aegis surface-air-missile 6)
  (aegis anti-sub 4)
)

(add supply start-with 2)
(add truck start-with 2)

            )
        )



;; NAVIES



  )
)



;;; Types.


(set grid-color "light-gray")
(set unit-gbox-border-color "dark-gray")
;; (set unit-gbox-fill-color "green")
	          	(set ai-may-resign true)
	          	(set ai-advanced-unit-separation 0)
	          	(set ai-badtask-max-retries 4)
 	          	(set ai-badtask-remove-chance 50)
	          	(set ai-badtask-reserve-chance 50)
;;; LAND UNITS


(unit-type infantry (name "Infantry") (image-name "ko-corps-2")  (char "i") 
  (vision-range 3) (capacity 0) (see-always false) (acp-per-turn 6) 
  (hp-max 4) (hp-recovery .25)
  (point-value 1)
  (ground true)
  (help "An infantry unit, the foundation of any good army."))

(unit-type engineer (name "Engineers") (image-name "ko-marines")  (char "e") 
  (vision-range 3) (capacity 0) (see-always false) (acp-per-turn 6) 
  (hp-max 4) (hp-recovery .25)
  (point-value 1)
  (ground true)
  (help "An engineer unit, with good but short-ranged attack and the ability to build defenses."))

(unit-type para (name "Paratrooper") (image-name "trident-paratroopers")  (char "p") 
  (vision-range 3) (capacity 0) (see-always false) (acp-per-turn 6) 
  (hp-max 4) (hp-recovery .25) (acp-to-fire 0) (range 0)
  (point-value 1)
  (ground true)
  (help "A paratrooper unit, able to be carried by a bomber."))

(unit-type tank (name "Tank") (image-name "trident-armor")  (char "t") 
  (vision-range 3) (capacity 0) (see-always false) (acp-per-turn 6) 
  (hp-max 5) (hp-recovery .25) (acp-to-fire 2) (range 5)
  (point-value 3)
  (ground true)
  (help "A general-purpose, heavy tank."))

(unit-type apc (name "APC") (image-name "trident-mech-infantry")  (char "a") 
  (vision-range 3) (capacity 0) (see-always false) (acp-per-turn 6) 
  (hp-max 5) (hp-recovery .25) (acp-to-fire 2) (range 4)
  (point-value 2)
  (ground true)
  (ai-peace-garrison 1)  (ai-war-garrison 0)
  (help "An armored personnel carrier, able to withstand some damage and carry an infantry or engineer."))

(unit-type arty (name "Artillery") (image-name "trident-howitzer")  (char "r") 
  (vision-range 3) (capacity 0) (see-always false) (acp-per-turn 6) 
  (hp-max 5) (hp-recovery .25) (acp-to-fire 2) (range 10) (range-min 3)
  (point-value 3)
  (ground true)
  (help "An artillery unit, capable of damaging other units at a distance."))

(unit-type sam (name "SAM") (image-name "ko-sam")  (char "s") 
  (vision-range 3) (capacity 0) (see-always false) (acp-per-turn 6) 
  (hp-max 5) (hp-recovery .25) (acp-to-fire 2) (range 5)
  (point-value 3)
  (ground true)
  (help "A surface-to-air missile unit, able to shoot down enemy aircraft."))

(unit-type truck (name "Supply Truck") (image-name "trident-freight")  (char "o") 
  (vision-range 3) (capacity 0) (see-always false) (acp-per-turn 6) 
  (hp-max 4) (hp-recovery .25)
  (point-value 1)
  (ground true)
  (help "Filled with ammo."))


(unit-type filler (name "Filler Unit") (image-name "ba-sat-dish")  (char "x") 
  (vision-range 60) (capacity 0) (see-always false) (acp-per-turn 6) 
  (hp-max 20) (cp 10) (hp-recovery 5.00) (acp-to-fire 0) (range 0)
  (point-value 3)
  (ground true)
  (help "Used to keep definitions from being empty."))

;; AIR UNITS

(unit-type bomber (name "Bomber") (image-name "trident-bomber")  (char "b") 
  (vision-range 1) (capacity 2) (see-always false) (acp-per-turn 8) 
  (hp-max 9) (hp-recovery .25)
  (point-value 5)
  (air true)
  (help "Heavy bombers are capable of destroying large numbers of land units, if the bomber isn't shot down."))

(unit-type fighter (name "Fighter") (image-name "ba-jet-frog")  (char "f") 
  (vision-range 4) (capacity 0) (see-always false) (acp-per-turn 8) 
  (hp-max 5) (hp-recovery .25) (acp-to-fire 2) (range 4) (range-min 2)
  (point-value 5)
  (air true)
  (help "A multi-role fighter that can fire missiles at 2-4 hexes and drop clusterbombs on adjacent hexes."))

(unit-type heli (name "Attack Helicopter") (image-name "trident-helicopter")  (char "h") 
  (vision-range 4) (capacity 0) (see-always false) (acp-per-turn 6) 
  (hp-max 5) (hp-recovery .25) (acp-to-fire 2) (range 4)
  (point-value 4)
  (air true)
  (help "An attack helicopter."))

(unit-type awacs (name "AWACS") (image-name "ko-awacs")  (char "E") 
  (vision-range 15) (capacity 0) (see-always false) (acp-per-turn 6) 
  (hp-max 5) (hp-recovery .25) (acp-to-fire 0) (range 0)
  (point-value 5)
  (air true)
  (help "Airborne early warning, sees all air units within 15 hexes."))

;; SEA UNITS

(unit-type carrier (name "Fleet Carrier") (image-name "ko-carrier2")  (char "V") 
  (vision-range 8) (capacity 4) (acp-per-turn 6)  
  (hp-max 14) (hp-recovery 0.00) (acp-to-fire 0) (range 0)
  (point-value 8)
  (help "Large, heavy carrier, capable of absorbing punishment and holding four Fighters."))

(unit-type escarrier (name "Escort carrier") (image-name "ko-carrier1")  (char "v")
  (vision-range 6) (capacity 2) (acp-per-turn 6) 
  (hp-max 8) (hp-recovery 0.00) (acp-to-fire 0) (range 0)
  (point-value 4)
  (help "Light carrier that can hold two fighters or helicopters."))

(unit-type battleship (name "Battleship") (image-name "trident-battleship")  (char "T") 
  (vision-range 8) (acp-per-turn 6)
  (hp-max 16) (hp-recovery 0.00) (acp-to-fire 2) (range 10)
  (point-value 6)
  (help "A massive surface ship with heavy guns."))

(unit-type aegis (name "Cruiser") (image-name "trident-aegis-cruiser")  (char "i") 
  (vision-range 6)  (acp-per-turn 6) (capacity 1)
  (hp-max 8) (hp-recovery 0.00) (acp-to-fire 2) (range 5)
  (point-value 4)
  (help "Jack of all trades.  Capable of anti-submarine warfare and equipped with SAMs and Cruise Missiles.  Can also hold a single helicopter."))

(unit-type transport (name "Transport") (image-name "trident-transport")  (char "i") 
  (vision-range 3)  (acp-per-turn 6) (capacity 4)
  (hp-max 6) (hp-recovery 0.00)
  (point-value 4)
  (help "Four infantry or a tank and an infantry."))

(unit-type supply (name "Supply Ship") (image-name "trident-destroyer")  (char "i") 
  (vision-range 2)  (acp-per-turn 6)
  (hp-max 6) (hp-recovery 0.00)
  (point-value 4)
  (help "A lightly armored supply craft."))

(unit-type sub (name "Nuclear Sub") (image-name "trident-submarine")  (char "i") 
  (vision-range 4)  (acp-per-turn 6)
  (hp-max 4) (hp-recovery 0.00)
  (point-value 5)
  (help "Deadly anti-ship attack."))


;; STRUCTURES

(unit-type hanger (name "Hardened Aircraft Hanger") (image-name "ko-airbase")  (char "H") 
  (vision-range 0) (capacity 4) (see-always true) (acp-independent true) 
  (hp-max 14) (hp-recovery 0.00) (acp-to-fire 0) (range 0)
  (point-value 3)
;;  (ai-peace-garrison 5)  (ai-war-garrison 0)
  (help "Reinforced concrete hangers used to protect aircraft from attack."))

(unit-type burb (name "Suburbs") (image-name "nt-town-gray-medium")  (char "S")
  (vision-range 0) (capacity 2) (see-always true) (acp-independent true) 
  (hp-max 10) (hp-recovery 0.00) (acp-to-fire 0) (range 0)
  (point-value 0)
  (help "Residential areas good for hiding troops and limiting the amount of units that can pass through."))

(unit-type urban (name "Urban Center") (image-name "nt-city-gray-large")  (char "U") 
  (vision-range 0) (capacity 4) (see-always true) (acp-independent true) 
  (hp-max 20) (hp-recovery 0.00) (acp-to-fire 0) (range 0)
  (point-value 0)
  (help "A collection of skyscrapers and factories capable of hiding an entire company."))

(unit-type bunker (name "Bunker") (image-name "ba-bunker-concrete")  (char "b") 
  (vision-range 0) (capacity 1) (see-always false) (acp-per-turn 3)
  (hp-max 10) (hp-recovery 0.00) (acp-to-fire 1) (range 5)
  (point-value 3)
  (help "A reinforced bunker with machine guns and cannon."))

(unit-type post (name "Listening Post") (image-name "ba-sat-dish")  (char "H") 
  (vision-range 9) (capacity 1) (see-always false) (acp-independent true) 
  (hp-max 8) (hp-recovery 0.00) (acp-to-fire 0) (range 0)
  (point-value 5)
  (help "Sees all aircraft, armor and ships within 9 hexes."))


(unit-type sea-mines (name "Mines (Sea)") (image-name "ba-mines-sea")  (char "*") 
  (vision-range 0) (capacity 1) (see-always false) (acp-independent true) 
  (hp-max 1) (hp-recovery 0.00) (acp-to-fire 0) (range 0)
  (point-value 3)
  (help "Hidden mines that can be removed by a supply ship."))

(unit-type fixed-gun (name "16in Heavy Emplacement") (image-name "ba-big-gun")  (char "b") 
  (vision-range 0) (capacity 0) (see-always false) (acp-per-turn 3)
  (hp-max 6) (hp-recovery 0.00) (acp-to-fire 1) (range 10)
  (point-value 3)
  (help "A reinforced bunker with machine guns and cannon."))

(unit-type fixed-sam (name "Heavy Sam Emplacement") (image-name "ba-big-sam")  (char "b") 
  (vision-range 0) (capacity 0) (see-always false) (acp-per-turn 3)
  (hp-max 6) (hp-recovery 0.00) (acp-to-fire 1) (range 6)
  (point-value 3)
  (help "A reinforced bunker with machine guns and cannon."))


;; OTHER

(unit-type inactive-vp (name "Uncontrolled Victory Point") (image-name "ba-vp-gray")
  (acp-independent true) (hp-max 1)
  (point-value 10) (see-always true)
  (help "Capture or destroy these to win in a Conquest game."))

(unit-type active-vp (name "Controlled Victory Point") (image-name "ba-vp-blue")
  (acp-independent true) (hp-max 1) (vision-range 100)
  (point-value 25) (see-always true)
  (help "Capture or destroy these to win in a Conquest game."))


;; UNIT DEFINITIONS

(define land-unit-types (infantry engineer para tank apc arty sam truck hanger burb urban bunker fixed-gun fixed-sam))

(define infantry-types (infantry engineer para truck))

(define armor-types (tank apc arty sam))

(define air-unit-types (heli fighter awacs bomber))

(define ship-types (carrier escarrier aegis battleship transport supply))

(define structure-types (hanger burb urban bunker post fixed-gun fixed-sam))

;; ANTI-INFANTRY ATTACK

(define direct-anti-infantry-0-types (sam arty apc tank truck))

(define direct-anti-infantry-1-types (filler))

(define direct-anti-infantry-2-types (engineer))

(define direct-anti-infantry-3-types (infantry para fighter bomber))

(define indirect-anti-infantry-0-types (infantry engineer para sam arty truck fixed-gun))

(define indirect-anti-infantry-1-types (aegis))

(define indirect-anti-infantry-2-types (tank heli bunker apc))

(define indirect-anti-infantry-3-types (arty battleship))

;; ANTI-ARMOR ATTACK

(define direct-anti-armor-0-types (sam arty tank apc truck))

(define direct-anti-armor-1-types (infantry))

(define direct-anti-armor-2-types (para bomber))

(define direct-anti-armor-3-types (engineer))

(define indirect-anti-armor-0-types (infantry engineer para sam truck fixed-gun))

(define indirect-anti-armor-1-types (apc aegis))

(define indirect-anti-armor-2-types (arty heli battleship bunker))

(define indirect-anti-armor-3-types (tank fighter))

;; ANTI-AIR ATTACK

(define direct-anti-air-0-types (sam arty tank apc truck))

(define direct-anti-air-1-types (infantry para engineer battleship carrier escarrier))

(define direct-anti-air-2-types (filler))

(define direct-anti-air-3-types (filler))

(define indirect-anti-air-0-types (infantry engineer para apc tank arty truck bunker))

(define indirect-anti-air-1-types (heli))

(define indirect-anti-air-2-types (filler))

(define indirect-anti-air-3-types (sam fighter aegis fixed-sam))

;; ANTI-SHIP ATTACK

(define direct-anti-ship-0-types (sam arty tank apc truck infantry para engineer battleship carrier escarrier))

(define direct-anti-ship-1-types (filler))

(define direct-anti-ship-2-types (filler))

(define direct-anti-ship-3-types (sub))

(define indirect-anti-ship-0-types (infantry engineer para apc tank arty truck))

(define indirect-anti-ship-1-types (heli))

(define indirect-anti-ship-2-types (aegis bunker))

(define indirect-anti-ship-3-types (battleship fighter fixed-gun))

;; ANTI-SUB ATTACK

(define direct-anti-sub-0-types (sam arty tank apc truck infantry para engineer battleship carrier escarrier))

(define direct-anti-sub-1-types (filler))

(define direct-anti-sub-2-types (sub))

(define direct-anti-sub-3-types (aegis))

(define indirect-anti-sub-0-types (infantry engineer para apc tank arty truck))

(define indirect-anti-sub-1-types (filler))

(define indirect-anti-sub-2-types (filler))

(define indirect-anti-sub-3-types (filler))



(terrain-type sea (char ".")
  (help "deep water"))
(terrain-type shallows (char ",")
  (help "shallow coastal water and lakes"))
(terrain-type swamp (char "="))
(terrain-type desert (char "~")
  (help "dry open terrain"))
(terrain-type land (image-name "plains") (char "}")
  (help "open flat or rolling country"))
(terrain-type forest (char "%"))
(terrain-type mountains (char "^"))
(terrain-type ice (char "_"))
(terrain-type city (image-name "urban") (char ":"))
(terrain-type outskirts (image-name "urban") (char ";"))
(terrain-type steppe (image-name "adv-steppe") (char "]"))
(terrain-type neutral (image-name "gray") (char "-"))
(terrain-type river (char "<")
  (subtype border) (subtype-x river-x))
(terrain-type beach (char "|")
  (subtype border))
(terrain-type road (char ">")
  (subtype connection) (subtype-x road-x))
(terrain-type peace1-political-border (image-name "yellow")
	(subtype border))
(terrain-type peace2-political-border (image-name "yellow")
	(subtype border))
(terrain-type peace3-political-border (image-name "yellow")
	(subtype border))
(terrain-type war-political-border (image-name "green")
	(subtype border))



;; (terrain-type snow (image-name "ice")
;;  (subtype coating))

(add (sea shallows) liquid true)

(define sea-t* (sea shallows))

(define land-t* (desert land steppe forest mountains city outskirts))

(define political-t* (war-political-border peace1-political-border peace2-political-border peace3-political-border))

;;; Static relationships.

(table vanishes-on
;;  (ground-types sea-t* true)
;;  (ground-types ice true)
;;  (ship-types land-t* true)
;;  (ship-types ice true)
)

;;; Unit-terrain capacities.

(table unit-size-in-terrain
  (land-unit-types t* 35)
  (ship-types t* 35)

  (air-unit-types t* 30)
  (sub t* 30)

  (filler t* 1)
)

(add t* capacity 100)



(add apc capacity 1)

;;; Unit-unit capacities.

(table unit-size-as-occupant
  ;; Disable occupancy by default
  (u* u* 99)
  (infantry apc 1)
  (para apc 1)
  (engineer apc 1)

  (infantry transport 1)
  (para transport 1)
  (engineer transport 1)
  (truck transport 1)
  (armor-types transport 3)

  (air-unit-types hanger 1)
  (air-unit-types carrier 1)
  (air-unit-types escarrier 1)

  (bomber (carrier escarrier) 99)

  (heli aegis 1)

  (infantry-types (burb urban bunker) 1)

  (para bomber 1)

  (fixed-gun bunker 1)

  (fixed-sam bunker 1)

  )

;;; Material ZOINK

(material-type anti-per (name "HE")
  (help "High Explosive rounds used for infantry targets."))

(material-type anti-tank (name "AT")
  (help "Anti-tank rounds or missiles."))

(material-type surface-air-missile (name "SAMs"))

(material-type anti-sub (name "Depth-Charges"))

(material-type anti-ship (name "Harpoons"))

(material-type mavs (name "Mavericks"))

(material-type clusters (name "Cluster-Bombs"))

(material-type aams (name "Sidewinders"))

(material-type mk82 (name "Bombs"))

(material-type cruise (name "Cruise-Missiles"))

(material-type big-guns (name "16in"))

(material-type torps (name "Torpedoes"))


(table unit-storage-x
  (tank anti-tank 20)
  (tank anti-per 10)

  (arty anti-per 50)
  (arty anti-tank 20)

  (sam surface-air-missile 10)

  (apc anti-tank 8)
  (apc anti-per 25)

  (truck anti-tank 100)
  (truck anti-per 100)
  (truck surface-air-missile 100)

  (fighter aams 4)
  (fighter mavs 4)
  (fighter clusters 2)
  (fighter anti-ship 2)

  (bomber mk82 30)
 
  (heli mavs 8)
  (heli aams 2)

  (hanger mavs 50)
  (hanger clusters 50)
  (hanger aams 50)
  (hanger mk82 50)

  (carrier mavs 50)
  (carrier clusters 50)
  (carrier aams 50)

  (escarrier mavs 30)
  (escarrier clusters 30)
  (escarrier aams 30)

  (aegis mavs 15)
  (aegis aams 10)

  (sub torps 10)

  (supply cruise 100)
  (supply big-guns 100)
  (supply surface-air-missile 100)
  (supply anti-sub 100)
  (supply torps 100)

  (battleship big-guns 50)

  (aegis cruise 10)
  (aegis surface-air-missile 20)
  (aegis anti-sub 10)

)


;;; Unit-material capacities.


;;; Vision.

(set terrain-seen false)

(table see-chance

;; You never see everything you're supposed to

  (u* u* 80)

  (air-unit-types infantry-types 10)

  (awacs u* 1)
  (awacs air-unit-types 100)


;; NAVAL
  (ship-types u* 1)
  (ship-types ship-types 80)
  (ship-types air-unit-types 50)
  (transport bunker 100)

;; SAM
  (sam u* 1)
  (sam air-unit-types 75)

  (u* sub 1)

  (post ship-types 95)
  (post air-unit-types 95)
  (post armor-types 95)
  (post sub 10)

  (filler u* 100)

  (active-vp u* 0)
  (active-vp active-vp 100)
  (active-vp inactive-vp 100)
)


(table see-chance-adjacent
   (infantry-types u* 100)

   (u* sub 1)
   (ship-types sub 10)
   (aegis sub 25)
   (air-unit-types sub 20)

  )

(table see-chance-at
   (infantry-types u* 100)
   (ship-types sub 10)
   (aegis sub 25)
   (air-unit-types sub 20)
  )

(table visibility
   (land-unit-types forest 20)
   (land-unit-types mountains 20)
   (land-unit-types swamp 20)
)



;;; Movement.

(table mp-to-enter-terrain
  (land-unit-types sea-t* 99)
  (land-unit-types beach -99)
  (armor-types swamp 6)
  (armor-types desert 1)
  (armor-types road 1)
  (armor-types (land steppe) 1)
  (armor-types forest 3)
  (armor-types outskirts 3)
  (armor-types city 3)
  (armor-types mountains 6)
  (armor-types river 3)
  (infantry-types swamp 3)
  (infantry-types desert 2)
  (infantry-types road 2)
  (infantry-types (land steppe) 2)
  (infantry-types outskirts 2)
  (infantry-types city 2)
  (infantry-types forest 2)
  (infantry-types mountains 3)
  (infantry-types river 2)

  (ship-types land-t* 99)
  (ship-types swamp 99)
  (ship-types sea 1)
  (ship-types shallows 2)

  (sub sea 1)
  (sub shallows 2)

  (u* neutral 99)

  (air-unit-types t* 1)
  (air-unit-types neutral 8)
  (ship-types neutral 6)

  (structure-types t* 99)
  (u* ice 99)
	(u* war-political-border 0)
	(u* peace1-political-border 99)
	(u* peace2-political-border 99)
	(u* peace3-political-border 99)

)


(table mp-to-leave-terrain
  (land-unit-types sea-t* 99)
  (structure-types t* 99)
)

;;; Transports will get you to the edge of the cell, but the
;;; passengers have to cross any borders on their own.

(table ferry-on-entry
	(u* u* over-own)
)

(table ferry-on-departure
	(u* u* over-own)
)

(table mp-to-enter-unit
   (u* apc 2)
   (u* bomber 2)
   (u* structure-types 1)

   (u* carrier 1)
   (u* escarrier 1)

   (u* transport 1)
)

(table occupant-escape-chance
	(u* u* 0)
	(infantry-types structure-types 50)
)


(table supply-on-creation
  (u* m* 9999)
)

;;; Repair.

(table auto-repair
;;  (place-types ground-types 1.00)
  )

(table auto-repair-range
  ;; Replacements can easily get to nearby units.
;;  (u* ground-types 1)
  )

;;; Production.

(table base-production
  (u* m* 0)
)

(table terrain-production
  (t* m* 0)
)


(table base-consumption
)

;; ZOC

(table zoc-range
	(air-unit-types u* -1)
	(ship-types u* -1)
	(active-vp u* 0)
	(inactive-vp u* 0)
	(land-unit-types u* 1)
	(arty u* 0)
	(sam u* 0)
	(sub u* -1)

	(structure-types u* -1)

	(filler u* -1)
)

(table mp-to-traverse-zoc
	(land-unit-types land-unit-types -1)
	(air-unit-types u* 0)
	(sub u* 0)
)

(table mp-to-enter-zoc
	(land-unit-types land-unit-types 0)
	(air-unit-types u* 0)
	(sub u* 0)
)

(table mp-to-leave-zoc
	(land-unit-types land-unit-types 0)
	(air-unit-types u* 0)
	(sub u* 0)
)
	
;;; Combat.

(table acp-to-attack
  (u* u* 0)
  (direct-anti-infantry-1-types infantry-types 2)
  (direct-anti-infantry-2-types infantry-types 2)
  (direct-anti-infantry-3-types infantry-types 2)

  (direct-anti-infantry-1-types structure-types 2)
  (direct-anti-infantry-2-types structure-types 2)
  (direct-anti-infantry-3-types structure-types 2)

  (direct-anti-armor-1-types armor-types 2)
  (direct-anti-armor-2-types armor-types 2)
  (direct-anti-armor-3-types armor-types 2)

  (direct-anti-air-1-types air-unit-types 2)
  (direct-anti-air-2-types air-unit-types 2)
  (direct-anti-air-3-types air-unit-types 2)

  (direct-anti-air-1-types bomber 2)
  (direct-anti-air-2-types bomber 2)
  (direct-anti-air-3-types bomber 2)

  (direct-anti-ship-1-types ship-types 2)
  (direct-anti-ship-2-types ship-types 2)
  (direct-anti-ship-3-types ship-types 2)

  (direct-anti-sub-1-types sub 2)
  (direct-anti-sub-2-types sub 2)
  (direct-anti-sub-3-types sub 2)

  (bomber land-unit-types 1)

  (infantry-types inactive-vp 6)
  (infantry-types active-vp 6)
)


(table hit-chance
  (u* u* 0)

  (direct-anti-infantry-1-types infantry-types 20)
  (direct-anti-infantry-2-types infantry-types 40)
  (direct-anti-infantry-3-types infantry-types 60)

  (direct-anti-infantry-1-types structure-types 20)
  (direct-anti-infantry-2-types structure-types 40)
  (direct-anti-infantry-3-types structure-types 60)

  (direct-anti-armor-1-types armor-types 20)
  (direct-anti-armor-2-types armor-types 40)
  (direct-anti-armor-3-types armor-types 60)

  (direct-anti-air-1-types air-unit-types 20)
  (direct-anti-air-2-types air-unit-types 40)
  (direct-anti-air-3-types air-unit-types 60)

  (direct-anti-ship-1-types ship-types 20)
  (direct-anti-ship-2-types ship-types 40)
  (direct-anti-ship-3-types ship-types 60)

  (infantry-types inactive-vp 100)
  (infantry-types active-vp 100)
)

(table damage

  (direct-anti-infantry-1-types infantry-types 1d4+1)
  (direct-anti-infantry-2-types infantry-types 1d4+1)
  (direct-anti-infantry-3-types infantry-types 1d4+1)

  (direct-anti-infantry-1-types structure-types 1d4+1)
  (direct-anti-infantry-2-types structure-types 1d4+1)
  (direct-anti-infantry-3-types structure-types 1d4+1)

  (direct-anti-armor-1-types armor-types 1d5+3)
  (direct-anti-armor-2-types armor-types 1d5+3)
  (direct-anti-armor-3-types armor-types 1d5+3)

  (direct-anti-air-1-types air-unit-types 1d5+2)
  (direct-anti-air-2-types air-unit-types 1d5+2)
  (direct-anti-air-3-types air-unit-types 1d5+2)

  (direct-anti-ship-1-types ship-types 1d5+2)
  (direct-anti-ship-2-types ship-types 1d5+2)
  (direct-anti-ship-3-types ship-types 1d5+2)

  (direct-anti-sub-1-types sub 1d5+2)
  (direct-anti-sub-2-types sub 1d5+2)
  (direct-anti-sub-3-types sub 1d5+2)

  (infantry-types inactive-vp 100)
  (infantry-types active-vp 100)

  (truck u* 0)
)


(table fire-hit-chance
  (u* u* 0)

  (indirect-anti-infantry-1-types infantry-types 20)
  (indirect-anti-infantry-2-types infantry-types 40)
  (indirect-anti-infantry-3-types infantry-types 60)

  (indirect-anti-infantry-1-types structure-types 20)
  (indirect-anti-infantry-2-types structure-types 40)
  (indirect-anti-infantry-3-types structure-types 60)

  (indirect-anti-armor-1-types armor-types 20)
  (indirect-anti-armor-2-types armor-types 40)
  (indirect-anti-armor-3-types armor-types 60)

  (indirect-anti-air-1-types air-unit-types 20)
  (indirect-anti-air-2-types air-unit-types 40)
  (indirect-anti-air-3-types air-unit-types 60)

  (indirect-anti-ship-1-types ship-types 20)
  (indirect-anti-ship-2-types ship-types 40)
  (indirect-anti-ship-3-types ship-types 60)
)

(table fire-damage

  (indirect-anti-infantry-1-types infantry-types 1d4)
  (indirect-anti-infantry-2-types infantry-types 1d4)
  (indirect-anti-infantry-3-types infantry-types 1d4)

  (indirect-anti-infantry-1-types structure-types 1d4+1)
  (indirect-anti-infantry-2-types structure-types 1d4+1)
  (indirect-anti-infantry-3-types structure-types 1d4+1)

  (indirect-anti-armor-1-types armor-types 1d5+1)
  (indirect-anti-armor-2-types armor-types 1d5+2)
  (indirect-anti-armor-3-types armor-types 1d10+3)

  (indirect-anti-air-1-types air-unit-types 1d10+3)
  (indirect-anti-air-2-types air-unit-types 1d10+3)
  (indirect-anti-air-3-types air-unit-types 1d10+3)

  (indirect-anti-ship-1-types ship-types 1d5+2)
  (indirect-anti-ship-2-types ship-types 1d5+2)
  (indirect-anti-ship-3-types ship-types 1d5+2)
)

(table zoo-range
   (fixed-gun ship-types 10)
   (fixed-sam air-unit-types 6)
   (bunker ship-types 2)
   (bunker armor-types 2)

)

(table acp-for-overwatch
   (u* u* 0)
)
(table defend-terrain-effect
  (infantry-types forest 75)
  (infantry-types mountains 66)
  (bunker mountains 50)
  (infantry-types swamp 133)
)

(table fire-defend-terrain-effect
  (infantry-types forest 75)
  (infantry-types mountains 66)
  (bunker mountains 50)
  (infantry-types swamp 133)
)

(table capture-chance
   	(infantry-types active-vp 100)
        (truck u* 0)
)

(table hp-min
	(active-vp u* 1)
	(u* active-vp 1)
)

(table counterattack
   	(infantry-types air-unit-types 0)
   	(air-unit-types infantry-types 0)

   	(structure-types u* 0)
   	(u* structure-types 0)

   	(sub u* 0)
   	(u* sub 0)
)


(table occupant-combat
	(u* u* 0)
	(infantry-types structure-types 100)

	(fixed-gun bunker 100)
	(fixed-sam bunker 100)
)

(table protection
   	(infantry-types transport 100)
	(transport infantry-types 0)

   	(armor-types transport 100)
	(transport armor-types 0)

   	(air-unit-types hanger 100)
	(hanger air-unit-types 0)

   	(air-unit-types carrier 100)
	(carrier air-unit-types 0)

   	(air-unit-types escarrier 100)
	(escarrier air-unit-types 0)

   	(infantry-types apc 100)
	(apc infantry-types 0)

   	(infantry-types bomber 100)
	(bomber infantry-types 0)

   	(infantry-types structure-types 60)
	(structure-types infantry-types 40)

   	(infantry-types bunker 100)
	(bunker infantry-types 0)

   	((fixed-gun fixed-sam) bunker 100)
	(bunker (fixed-gun fixed-sam) 0)

)


(table consumption-per-attack
	(fighter clusters 1)
	(bomber mk82 2)
	(ship-types anti-sub 2)

	(sub torps 1)

)


(table consumption-per-fire
	(armor-types anti-per 1)
	(armor-types anti-tank 1)

	(sam surface-air-missile 1)

	(fighter mavs 1)
	(fighter anti-ship 1)
	(fighter aams 1)

	(heli mavs 1)
	(heli aams 1)

	(battleship big-guns 1)

	(aegis surface-air-missile 1)
	(aegis cruise 1)

)

(table hit-by

	(armor-types anti-tank 1)
	(armor-types mavs 1)
	(armor-types mk82 2)
	(armor-types big-guns 1)
	(armor-types cruise 1)

	(infantry-types anti-per 1)
	(infantry-types clusters 1)
	(infantry-types cruise 1)
	(infantry-types mk82 2)

	(air-unit-types aams 1)
	(air-unit-types surface-air-missile 1)

	(structure-types anti-per 1)
	(structure-types clusters 1)
	(structure-types mk82 2)
	(structure-types big-guns 1)
	(structure-types cruise 1)

	(ship-types anti-ship 1)
	(ship-types big-guns 1)
	(ship-types cruise 1)

	(sub anti-sub 2)

	(ship-types torps 1)
	(sub torps 1)

)

(table wrecked-type-if-killed
	(inactive-vp u* active-vp)
)


(table consumption-per-move
)

;;; Backdrop.

(table out-length
  (u* m* 1)
  (truck m* 3)
  (supply m* 3)
  (air-unit-types m* 0)
  (hanger m* 0)
  (carrier m* 0)
  (escarrier m* 0)
  (aegis m* 0)

)

(table in-length
  (u* m* 1)
  (air-unit-types m* 0)
  (hanger m* -1)
  (carrier m* -1)
  (escarrier m* -1)
  (aegis m* -1)

)

;; INDEPENDENT RELATED

(table can-enter-independent
	(u* structure-types true)
)

(table independent-density
	(burb outskirts 50.00)
	(urban city 50.00)
)

;; SIDE LOSS RESULTS

;; Some units are lost, some are captured
(add u* lost-vanish-chance 10000)

;;; Sides.

;;; SUPPLY

 (table unit-initial-supply
  (u* m* 9999)
  )

(scorekeeper (do last-side-wins))

(doctrine rrr
    (construction-run (u* 1))
    (resupply-percent 0)
    (rearm-percent 0)
    (repair-percent 0)
)

(side-defaults
    (default-doctrine rrr)
;;    (doctrines
;;        (city-u* city-unit-doctrine)
;;        (base base-doctrine)
;;        (shipyard shipyard-doctrine)
;;	)
)


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;Random set-up

(set country-radius-max 25)
(set country-separation-min 40)
(set edge-terrain ice)

(add u* start-with 0)
(add infantry start-with 10)
;;(add para start-with 2)
(add engineer start-with 2)
(add tank start-with 5)
(add apc start-with 4)
(add arty start-with 2)
(add sam start-with 1)
(add fighter start-with 2)
(add heli start-with 2)
(add truck start-with 1)
(add hanger start-with 1)
(add bunker start-with 3)
(add fixed-gun start-with 1)
(add fixed-sam start-with 1)
(add post start-with 1)


(define cell-t* (sea shallows swamp land desert steppe forest mountains ice))


(add cell-t* alt-percentile-min 	(0  	5 	0  	15	20	15	15  	95  	99))
(add cell-t* alt-percentile-max 	(5  	10  	15  	95	50	95	95  	99 	100))
(add cell-t* wet-percentile-min 	(0   	0  	0   	0	40	40	80   	0   	0))
(add cell-t* wet-percentile-max 	(70 	80 	100	40	70	80	100	100 	100))

;;;	Some defns for the fractal percentile generator.

(set alt-blob-density 10000)
(set alt-blob-height 500)
(set alt-blob-size 100)
(set wet-blob-density 2000)
(set wet-blob-size 100)
(set alt-smoothing 5)
(set wet-smoothing 5)


(table favored-terrain
  (u* t* 0)	  
  (land-unit-types (land desert forest steppe) 50)
  (ship-types sea 50)
  (sub sea 50)

  (bunker desert 100)
  ((fixed-gun fixed-sam) t* 0)
)



(game-module (design-notes
  "Units are modern, strategic level."
  ))

(imf "nt-city-gray-large"  ((32 32) (file "neo-trident.gif" std 0 0)))
(imf "nt-town-gray-medium"  ((32 32) (file "neo-trident.gif" std 0 1)))
(imf "nt-city-white-small"  ((32 32) (file "neo-trident.gif" std 0 2)))

(imf "ko-city-damage"  ((32 32) (file "korea.gif" std 3 5)))
(imf "ko-city-wreck"  ((32 32) (file "korea.gif" std 3 6)))
(imf "ko-city-level"  ((32 32) (file "korea.gif" std 3 7)))
(imf "ko-marines"  ((32 32) (file "korea.gif" std 3 8)))


(imf "ko-reinforcedcorps" ((32 32) (file "korea.gif" std 0 0)))
(imf "ko-corps" ((32 32) (file "korea.gif" std 0 1)))
(imf "ko-damagedcorps" ((32 32) (file "korea.gif" std 0 2)))
(imf "ko-wreckedcorps" ((32 32) (file "korea.gif" std 0 3)))

(imf "ko-reinforcedarmy"  ((32 32) (file "korea.gif" std 1 0)))
(imf "ko-army"  ((32 32) (file "korea.gif" std 1 1)))
(imf "ko-damagedarmy"  ((32 32) (file "korea.gif" std 1 2)))
(imf "ko-wreckedarmy"  ((32 32) (file "korea.gif" std 1 3)))
(imf "ko-nuke1"  ((32 32) (file "korea.gif" std 1 5)))
(imf "ko-nuke2"  ((32 32) (file "korea.gif" std 1 6)))
(imf "ko-nuke3"  ((32 32) (file "korea.gif" std 1 7)))
(imf "ko-nuke4"  ((32 32) (file "korea.gif" std 1 8)))

(imf "ko-reinforcedarmor"  ((32 32) (file "korea.gif" std 2 0)))
(imf "ko-armor"  ((32 32) (file "korea.gif" std 2 1)))
(imf "ko-damagedarmor"  ((32 32) (file "korea.gif" std 2 2)))
(imf "ko-wreckedarmor"  ((32 32) (file "korea.gif" std 2 3)))
(imf "ko-redmissile"  ((32 32) (file "korea.gif" std 2 5)))
(imf "ko-bluemissile"  ((32 32) (file "korea.gif" std 2 6)))
(imf "ko-greenmissile"  ((32 32) (file "korea.gif" std 2 7)))
(imf "ko-blackmissile"  ((32 32) (file "korea.gif" std 2 8)))
(imf "ko-corps-2"  ((32 32) (file "korea.gif" std 2 9)))

(imf "ko-reinforcedwing"  ((32 32) (file "korea.gif" std 3 0)))
(imf "ko-wing"  ((32 32) (file "korea.gif" std 3 1)))
(imf "ko-damagedwing"  ((32 32) (file "korea.gif" std 3 2)))
(imf "ko-wreckedwing"  ((32 32) (file "korea.gif" std 3 3)))

(imf "ko-reinforcedsurface" ((32 32) (file "korea.gif" std 4 0)))
(imf "ko-surface" ((32 32) (file "korea.gif" std 4 1)))
(imf "ko-damagedsurface" ((32 32) (file "korea.gif" std 4 2)))
(imf "ko-wreckedsurface" ((32 32) (file "korea.gif" std 4 3)))

(imf "ko-reinforcedcarrier" ((32 32) (file "korea.gif" std 5 0)))
(imf "ko-carrier" ((32 32) (file "korea.gif" std 5 1)))
(imf "ko-damagedcarrier" ((32 32) (file "korea.gif" std 5 2)))
(imf "ko-wreckedcarrier" ((32 32) (file "korea.gif" std 5 3)))

(imf "ko-reinforcedadn" ((32 32) (file "korea.gif" std 6 0)))
(imf "ko-adn" ((32 32) (file "korea.gif" std 6 1)))
(imf "ko-damagedadn" ((32 32) (file "korea.gif" std 6 2)))
(imf "ko-wreckedadn" ((32 32) (file "korea.gif" std 6 3)))

(imf "ko-carrier-wing"  ((32 32) (file "korea.gif" std 8 0)))
(imf "ko-coastsub"  ((32 32) (file "korea.gif" std 8 1)))
(imf "ko-carrier2"  ((32 32) (file "korea.gif" std 8 2)))
(imf "ko-carrier1"  ((32 32) (file "korea.gif" std 8 3)))
(imf "ko-advanced-fighter"  ((32 32) (file "korea.gif" std 8 4)))
(imf "ko-advanced-tank"  ((32 32) (file "korea.gif" std 8 5)))
(imf "ko-sat"  ((32 32) (file "korea.gif" std 8 6)))
(imf "ko-sam"  ((32 32) (file "korea.gif" std 8 7)))
(imf "ko-milair"  ((32 32) (file "korea.gif" std 8 8)))
(imf "ko-civair"  ((32 32) (file "korea.gif" std 8 9)))

(imf "ko-coastal"  ((32 32) (file "korea.gif" std 9 0)))
(imf "ko-squadron"  ((32 32) (file "korea.gif" std 9 1)))
(imf "ko-brigade"  ((32 32) (file "korea.gif" std 9 2)))
(imf "ko-engineer"  ((32 32) (file "korea.gif" std 9 3)))
(imf "ko-specops"  ((32 32) (file "korea.gif" std 9 4)))
(imf "ko-specheli"  ((32 32) (file "korea.gif" std 9 5)))
(imf "ko-sub"  ((32 32) (file "korea.gif" std 9 6)))
(imf "ko-csub"  ((32 32) (file "korea.gif" std 9 7)))
(imf "ko-miltran"  ((32 32) (file "korea.gif" std 9 8)))
(imf "ko-civtran"  ((32 32) (file "korea.gif" std 9 9)))

;;(imf "flag-north-korea"  ((16 16) (file "flags16x16.gif" std 4 8)))

(imf "ko-airbase"  ((32 32) (file "korea.gif" std 1 9)))

(imf "ko-zsu"  ((32 32) (file "korea.gif" std 3 9)))

(imf "ko-awacs"  ((32 32) (file "korea.gif" std 4 9)))

(imf "ba-vp-blue" ((32 32) (file "battles1.gif" std 9 2)))
(imf "ba-vp-gray" ((32 32) (file "battles1.gif" std 9 3)))

(imf "ba-mines-sea" ((32 32) (file "battles1.gif" std 9 6)))
(imf "ba-bunker-concrete" ((32 32) (file "battles1.gif" std 9 7)))
(imf "ba-big-gun" ((32 32) (file "battles1.gif" std 9 8)))
(imf "ba-big-sam" ((32 32) (file "battles1.gif" std 9 9)))

(imf "ba-jet-sing" ((32 32) (file "battles1.gif" std 8 7)))
(imf "ba-jet-frog" ((32 32) (file "battles1.gif" std 8 8)))
(imf "ba-sat-dish" ((32 32) (file "battles1.gif" std 8 9)))

