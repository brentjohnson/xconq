(game-module "galaxy2"
  (title "Galaxy 2")
  (version "1.0")
  (blurb "Improved Galaxy game by Massimo Campostrini.")
  (instructions "Explore, conquer, meet a fleet with a bigger fleet!")
  (variants
    (see-all false)
    ("Galaxy Seen" world-seen false)
    (world-size (60 30 360))
    (sequential false)
    ("Alternate economy" model1
     "Use a different algorithm for moving materials among units."
     (true (set backdrop-model 1)))
    ("AltEcon uses doctrine" model1doctrine
     "When using alternate economy, don't drain units below doctrine levels."
     (true (set backdrop-ignore-doctrine 0))
     (false (set backdrop-ignore-doctrine 1)))
  )
)

; UNITS

(unit-type stormtrooper (image-name "kb-emp-st") (char "s")
  (help "for capturing planets, moons, & starbases"))
(unit-type fighter (name "SF-1 fighter") (image-name "kb-st-ship-small") (char "f")
  (help  "moves fast, can hit most things"))
(unit-type transport (image-name "kb-st-ship-nose") (char "t")
  (help "for transporting the troopers"))
(unit-type light-cruiser (name "light cruiser") (image-name "kb-st-ship-enterprise") (char "l")
  (help "quick lightly armoured cruiser"))
(unit-type battlecruiser (image-name "kb-st-ship-reliant") (char "b")
  (help "specially designed for fleet action"))
(unit-type dreadnought (image-name "kb-st-ship-excelsior") (char "d")
  (help "heavily armoured with deadly phasers"))
(unit-type starbase (image-name "kb-st-station-research") (char "@")
  (help "your home away from home"))
(unit-type moon (image-name "moon") (char "x")
  (help "makes stuff (esp. troopers and fighters)"))
(unit-type planet (image-name "planet") (char "X")
  (help "makes stuff (esp. cruisers, dreadnoughts & starbases)"))

(define s stormtrooper)
(define f fighter)
(define t transport)
(define l light-cruiser)
(define b battlecruiser)
(define d dreadnought)
(define @ starbase)
(define x moon)
(define X planet)

; MATERIALS

(material-type photon-torpedo (name "photon torpedo") (image-name "gray")
  (notes "ammunition to fire in long-range combat"))
(material-type matter
  (notes "part of the fuel for ships' movement"))
(material-type anti-matter
  (notes "part of the fuel for ships' movement, also used up in short-range combat"))

(define ph photon-torpedo)
(define m matter)
(define a anti-matter)

; TERRAIN

(terrain-type vacuum (char ".")) ;  (color "navy blue")
(terrain-type nebula (char "%")) ;  (color "grey")

(set alt-blob-density 10000)
(set alt-blob-height 500)
(set alt-blob-size 200)
(set alt-smoothing 4)
(set wet-blob-density 2000)
(set wet-blob-size 100)

; should have low blob density

(add t* alt-percentile-min   0)
(add t* alt-percentile-max 100)
(add t* wet-percentile-min (  0  90))
(add t* wet-percentile-max ( 90 100))


;; DEFINES

(define places (x X))
(define planets (x X))
(define movers (s f t l b d @))
(define starships (t l b d @))
(define fleetships (l b d))

;; TEXTS

(set action-notices '(
  ((disband u* movers done) (actor " dismantles " actee))
  ((disband u* s done) (actee " disbands"))
  ((destroy u* u*) (actor " destroys " actee "!"))
  ((destroy s s) (actor " defeats " actee "!"))
  ((destroy fleetships (s x X)) (actor " annihilates " actee "!"))
  ((destroy fleetships (f t l b d @)) (actor " zorches " actee "!"))
  ((destroy u* f) (actor " shoots down " actee "!"))
  ))

(set event-notices '(
  ((unit-starved u*) (0 " runs out of supplies"))
  ))

(set event-narratives '(
  ((unit-starved u*) (0 " ran out of supplies"))
  ))

;; STARTUP

(add places point-value 1000)

(add vacuum country-terrain-min 6)

(table favored-terrain
  (s t* 0)
  (places t* 100)
)
(add (X x t l b d @ s f) start-with (2 3 2 3 2 1 1 4 5))

(table independent-density 
  (X t* 40)
  (x t* 60)
)

(set country-radius-min 3)
(set country-separation-min 20)
(set country-separation-max 100)

(add places already-seen true)
(add places see-always true)

;; Units always start out full of everything.

(table unit-initial-supply (u* m* 20000))

;; Production

;               s f t l  b  d  @
(add movers cp (2 3 8 9 15 30 40))

(table can-create
  ((x X) movers 1)
)

(table acp-to-create
  ((x X) movers 1)
)

(table cp-on-creation
  (x movers 1)
;            s f t l b  d  @
  (X movers (1 1 2 3 4 11 13))
)

(table can-build
  ((x X) movers 1)
)

(table acp-to-build
  ((x X) movers 1)
)

(table cp-per-build
  (u* u* 1)
)

;; Automatic repair work.
;                             t    l    b    d    @
(add starships hp-recovery (0.00 0.25 0.50 0.75 2.00))

(table auto-repair
   (u* u* 0)
   ((@ x X) movers 1.00)
   ((@ x X) b 1.50)
   ((@ x X) d 2.00)
)

;; Materials

(table base-production
  (l ph 2)
  (b ph 3)
  (d ph 5)
;        ph   m  a
  (@ m* (10  20 10))
  (x m* (10 100 20))
  (X m* (20 200 40))
)

(table unit-storage-x
  ;;      s  f   t   l   b   d    @     x     X
  (u* ph (0  0   0  10  20  30  100   100   200)) 
  (u*  m (0 24 300 100 200 300 2000 10000 20000))
  (u*  a (0  5 300 100 200 300 1000  1000  2000))
)

(table base-consumption
  ((f t l b d) m 1)
  ((f t l b d) a 1)
  (@ (m a) 2)
)

(table consumption-as-occupant
  (u* m* 0)
)

(table consumption-per-move
  (f m 1)
  (starships (m a) 1)
  (@ (m a) 2)
)

(table hp-per-starve
  ((f t l b d) m 1.00)
  ((f t l b d) a 1.00)
  (@ (m a) 1.00)
)

(add fleetships acp-to-fire  1)
(add fleetships range 5)

(table consumption-per-attack
   (u* m* 0)
   ((f l b d) a (1 2 2 3))
)

;	These two tables are redundant with consumption-per-attack and
;	consumption-per-fire. The idea with material-to-attack is that
;	it should reflect non-consumable stuff needed (e.g. weapons).

;	(table material-to-attack
;	   (u* m* 0)
;	   ((f l b d) a (1 2 2 3))
;	)

;	(table material-to-fire
;	       (u* m* 0)
;	       (fleetships ph 10)
;	)

(table consumption-per-fire
       (u* m* 0)
       (fleetships ph 10)
)

; CAPACITY

; transports have separate cradles for each occupant type
(table unit-capacity-x
  (u* u* 0)
  ((l b d) f (1 2 3))
  (@ (s f t l b d) (5 5 2 2 1 1))
  (t s 5)
  (x u* 10)
  (x (b d @) (5 3 2))
  (X u* 20)
  (X (s b d @) (40 10 5 3))
  (planets planets 0)
)

;;; Unit-terrain capacities.
;;  Limit units per cell.
;;  Allow only one place per cell.
  
(table unit-size-in-terrain
  (s t* 1)
  (f t* 1)
  ((t l) t* 3)
  (b t* 6)
  (d t* 10)
  (@ t* 20)
  (planets t* 51)
)

(add t* capacity 100)

; MOVEMENT

;                     s  f t l  b  d @ x X
(add u* acp-per-turn (1 12 5 9 16 21 3 1 1))
(add planets speed 0)

;; ; defined later...       s f t l b  d  @  x  X
;; (add u* hp-max          (1 2 2 5 9 14 22 25 40))

(add f speed-damage-effect ((1 50) (2 100)))
(add t speed-damage-effect ((1 33) (2 100)))
; these can stand some damage without slowing down:
(add l speed-damage-effect ((1 20) (4 100) (5 100)))
(add b speed-damage-effect ((1 20) (7 100) (9 100)))
(add d speed-damage-effect ((1 20) (11 100) (14 100)))
(add @ speed-damage-effect ((1 20) (17 100) (22 100)))


(table mp-to-enter-terrain
  (movers vacuum 1)
  (movers nebula 3)
  ((b d) vacuum (2 3))
  ((b d) nebula (6 9))
  (s t* 99) ; stormtroopers are always occupants
)

(table mp-to-enter-unit
  (u* u* 1)
  (l u* 9)
  (b u* 16)
  (d u* 21)
  (@ u* 3)
  (f u* 12)
)

(add u* free-mp 3)
(add l free-mp 9)
(add b free-mp 16)
(add d free-mp 21)
(add @ free-mp 3)
(add f free-mp 12)

(table material-to-move
  (f m 1)
  (starships (m a) 1)
  (@ (m a) 2)
)

;; SIGHT

; nebulas hide stuff

(table visibility
   (u* t* 100)
   (u* nebula 20)
)

;                     s f t l b d @ x X
(add u* vision-range (1 2 2 3 3 3 5 5 5))

; COMBAT

;                        s f t l b  d  @  x  X
(add u* hp-max          (1 2 2 5 9 14 22 25 40))

(table hit-chance
;;        s   f   t   l   b   d   @   x   X
  (s u* (050 010 030 030 030 020 030 050 050))
  (f u* (050 050 070 040 035 030 040 000 000))
  (t u* (020 020 050 020 020 020 030 000 000))
  (l u* (070 070 070 050 035 030 040 000 000))
  (b u* (090 060 085 065 050 040 050 000 000))
  (d u* (100 070 095 075 060 050 060 000 000))
  (@ u* (040 060 080 065 050 040 070 000 000))
  (x u* (010 010 030 020 010 005 010 000 000))
  (X u* (020 020 040 030 020 010 020 000 000))
)

(table fire-hit-chance
  (fleetships u* 0)
  (fleetships f 65)
  (fleetships t 95)
  (fleetships fleetships 75)
  (fleetships @ 85)
)

(table damage
  (u* u* 1)
  ((l b) u* 2)
  ((l b) f 1)
  (d u* 3)
)

(table fire-damage
  (u* u* 3)
)

(table protection
  (s places 80)
  (s @ 80)
  ((l b d @) f (80 65 50 50))
  (@ fleetships 65)
  (X u* 60)
  (x u* 75)
  (places s 50)
  (places @ 100)
)

(table occupant-combat
  (u* u* 0)
  (s u* 100)
)

(table occupant-vision
  (u* u* 0)
)

(table capture-chance
  (s places (50 30))
  (s @ 20)
)

(table independent-capture-chance
  (s places (75 50))
  (s @ 50)
)

(table occupant-escape-chance
  ((f t l b d) @ (50 20 40 30 20))
)

(add movers acp-to-disband 1)
(add movers hp-per-disband 99)

;GENERAL STUFF

(add u* acp-to-change-side 1)
(add s acp-to-change-side 0)

; (add movers possible-sides (not "independent"))

(scorekeeper (do last-side-wins))

;; NAMES 

;;; Sort of a mishmash, but who cares...

(set side-library '(
; Star Trek names
  (10 (name "Federation") (long-name "United Federation of Planets") (short-name "UFP") (adjective "Federation"))
  (10 (name "Klingon Empire") (adjective "Klingon"))
  ((name "Cardassian Union") (adjective "Cardassian"))
  (2 (name "Ferengi Alliance") (adjective "Ferengi"))
  (2 (name "Romulan Star Empire") (adjective "Romulan"))
;  ((name "Betazed") (adjective "Betazoid")) ; a member of the UFP
  ((name "Earth") (adjective "Terran")) ; a member of the UFP, but we
					; don't cater only to Star Trek
;  ((name "Vulcan") (adjective "Vulcan")) ; a member of the UFP
  ((adjective "Orion"))
; Star Wars names
  (10 (name "Galactic Empire") (adjective "Imperial"))
  (2 (adjective "Jedi"))
  (10 (name "Rebel Alliance") (adjective "Rebel"))
; Others
  ((adjective "Corazan"))
  ((adjective "Cyborg"))
  ((adjective "Cylon"))
  ((adjective "Dalek"))
  ((adjective "Goth"))
  ((adjective "Kelvin"))
  ((adjective "Kharg"))
  ((adjective "Time Lord"))
  (5 (adjective "Vogon"))
  ((adjective "Zen"))
))

(add (x X) namer "random-planets")

(namer random-planets (random
"Argon" "Tertulian" "Atlantis" "Isis" "Cuthka"
"Prene" "Ziul" "Aja" "Hukan" "Burran" 
"Zenon" "Vorscica" "Khakan" "Trantor" "Solcan"
"Cjiny" "Caligula" "Ipaar" "Ryurr" "Mentha"
"Vrudn" "Martel" "Loki" "Mendelan"
"Saure" "Juvak" "Kaos" "Irrenbuk" "Calazan"
"Ether" "Astral" "Keeguran" "Dospas" "Centari"
"Alpha" "Guerilli" "Galeo" "Charon" "Appolo"
"Ulam Batar" "Urishina" "Zaariana" "Shadowfax"
"Cloudy Coffee"  "with nice beaches" "of the Apes"
"Kyuin" "Reydak" "Baal" "Geryon" "Dispater"
"Azurn" "Mythos" "Nantes" "Kourion" "Turton"
"Moori" "Pelanda" "Sirith" "Celan" "Corolla"
"Muur" "Sol Invictus" "Saladan" "Samnth" "Awe"
"Kvack" "Portuli" "Turin" "Anthran" "Xerxes"
"Zeus" "Keno" "Terre" "Vito" "Vishnu" 
"Visigoth" "Romulus" "Kresge" "Kor" "Kolkhoz"
"Algol" "Quixote" "Quirinal" "Chisholm" 
"Archangel" "Ariananus" "Buran" "Kea" "Tara"
"Gustavus" "Antioch" "Piraeus" "Vaudois" "Vault"
"Rastaan" "Daggoroth" "Beina" "Breton" 
"Gallivent" "Galaak" "Galifre" "Lise" "Abidjan"
"Acanthus" "Achilles" "Actaeon" "Adelia" "Adonis"
"Axla" "Alnu" "Alcestis" "Alkmena" "Alexei"
"Garcia" "New Peoria" "Penelope" "Taurus"
"Bevois" "Odin" "Dwyer" "Aegean" "Vega"
"Altar" "Betelgeuse" "Aldebaran" "Procyon"
"Spica" "Lapella" "Deneb" "Venus" "Mars" 
"Io" "Ganymede" "Europa" "Callisto" "Rigel" "Rigel VII"
"Rigel XII" "DeLorian" "Pyrrus" "New Warth"
"Dorsai" "Xax" "Cygnus X-1" ; its really a black hole but..
"Terminus"  "Felicity" "Sanction" "goes BOOM"
))

(set meridians false)

(game-module (notes (
  "Written by Victor Mascari. "
  "Copyright 1991, Henry Ware & Victor Mascari."
  ""
  "Please distribute this, rewrite it etc.  However,"
  "you may not sell it, and this message must remain"
  "intact.  Of course, if you do any of these things, we"
  "will never know."
  ""
  "Converted to version 7 by Stan Shebs. "
  "Much altered by Massimo Campostrini. "
  "Bugfixed by Keir E. Novik."
  ""
  ""
  "Thanks to the writer of the 'Starwars'"
  "period for icons and ideas, and to "
  "Henry Ware for some icons and technical advice. "
  "This period was written by Victor Mascari at The Ohio State University."
  ""
  ""
  "Photon torpedos are generally the most effective means"
  "of destroying enemy ships; but hold your fire until"
  "you have enough for the kill."
  ""
  "Strategy depends upon the size of the map you're playing on.  The smaller the"
  "map, the smaller the units that should be built.  Large maps require big"
  "fleets of big ships."
  ""
  "Fleets tend to be the preferred method of warfare.  In addition, the person"
  "that attacks first usually wins, so gather as much intelligence about"
  "the enemy movements as possible, and keep your main battle fleet away "
  "from his until you can strike with full force."
  ""
  "Fighting near your planets give a good advantage, since you can resupply"
  "your ships with photon torpedos.  Starbases also carry extra supplies."
  ""
  "Nebula hide ships 80% of the time, but also slow them down."
  ""
  "The computer is fairly stupid in this period and is fairly easy to defeat. "
  "The skills learned to defeat the computer will, in general, not apply"
  "to human opponents."
)))

(add s notes (
  "Stormtroopers suffocate in outer space, so don't put them there.  "
))
(add f notes (
  "Fighters are good for exploring, directing fire, destroying troop"
  "transports, and harassing cruisers (avoid dreadnoughts). "
  "Send them to take a point of damage and come home."
))
(add t notes (
  "Transports are, naturally, vulnerable and sorta slow."
))
(add l notes (
  "Light Cruisers are the smallest of the fleet ships.  They are very"
  "vulnerable in fleet combat, however, especially to dreadnoughts. "
  "They are good for reconnaissance.  They can be used as batteries"
  "to provide photon torpedos, but they don't make many."
))
(add b notes (
  "Battle Cruisers provide photon torpedos.  Generally, they provide the"
  "backbone of a battle fleet."
))
(add d notes (
  "Dreadnoughts are the largest of the fleet ships and are tough in close "
  "combat.  But, despite the name, it best to play these ships with a "
  "certain amount of caution; one dreadnought can not take on a fleet."
))
(add @ notes (
  "Starbases provide supplies for a fleet, and have a limited"
  "fighting capability.  They are quite slow, and can be captured"
  "by stormtroopers."
))

(add x notes (
  "Planets and Moons are the main producers.  Generally, the more powerful"
  "the unit to be constructed, the better it is to produce it in a planet. "
  "As for the defense of these units, there are several schools of thought. "
  "One school says the best defense is a good offense.  Another holds"
  "that fighter patrols are cheap and effective against enemy landings. "
  "Or that a loaded transport should be kept to retake any enemy conquests."
))

(add X notes (
  "Planets and Moons are the main producers.  Generally, the more powerful"
  "the unit to be constructed, the better it is to produce it in a planet. "
  "As for the defense of these units, there are several schools of thought. "
  "One school says the best defense is a good offense.  Another holds"
  "that fighter patrols are cheap and effective against enemy landings. "
  "Or that a loaded transport should be kept to retake any enemy conquests."
))

(game-module (design-notes (
  "Modifications by Massimo Campostrini:"
  ""
  "Photon torpedoes demoted to material; they were units, and it was"
  "very annoying to have them wake up all the time."
  ""
  "Replaced Death Star with Starbase; the Death Star was really too powerful."
  ""
  "Got rid of black holes.  They were just annoying,"
  "and the AI never learned to avoid them."
  ""
  "Reduced speed and capacities of all movers."
  ""
  "Got rid of oxygen.  Stormtroopers now must be occupants at all times."
)))

(imf "kb-st-ship-enterprise" ((64 64) (file "kobayashi.gif" std 0 0)))
(imf "kb-st-ship-reliant" ((64 64) (file "kobayashi.gif" std 0 1)))
(imf "kb-st-ship-gagarin" ((64 64) (file "kobayashi.gif" std 0 2)))
(imf "kb-st-ship-excelsior" ((64 64) (file "kobayashi.gif" std 0 3)))
(imf "kb-st-ship-yamato" ((64 64) (file "kobayashi.gif" std 0 4)))
(imf "kb-st-ship-galaxy" ((64 64) (file "kobayashi.gif" std 0 5)))
(imf "kb-st-ship-phoenix" ((64 64) (file "kobayashi.gif" std 0 6)))
(imf "kb-st-ship-enterprise3" ((64 64) (file "kobayashi.gif" std 0 7)))
(imf "kb-st-ship-defiant" ((64 64) (file "kobayashi.gif" std 0 8)))
(imf "kb-st-ship-open" ((64 64) (file "kobayashi.gif" std 0 9)))

(imf "kb-st-ship-sovereign" ((64 64) (file "kobayashi.gif" std 1 0)))
(imf "kb-st-ship-trip" ((64 64) (file "kobayashi.gif" std 1 1)))
(imf "kb-st-ship-open2" ((64 64) (file "kobayashi.gif" std 1 2)))
(imf "kb-st-ship-voyager" ((64 64) (file "kobayashi.gif" std 1 3)))
(imf "kb-st-ship-fat" ((64 64) (file "kobayashi.gif" std 1 4)))
(imf "kb-st-station-research" ((64 64) (file "kobayashi.gif" std 1 5)))
(imf "kb-st-ship-transport" ((64 64) (file "kobayashi.gif" std 1 6)))
(imf "kb-explosion-blue" ((64 64) (file "kobayashi.gif" std 1 7)))
(imf "kb-explosion-orange" ((64 64) (file "kobayashi.gif" std 1 8)))
(imf "kb-emp-tie-tri" ((64 64) (file "kobayashi.gif" std 1 9)))

(imf "kb-kling-ship-d7" ((64 64) (file "kobayashi.gif" std 2 0)))
(imf "kb-kling-ship-bop" ((64 64) (file "kobayashi.gif" std 2 1)))
(imf "kb-kling-ship-ca" ((64 64) (file "kobayashi.gif" std 2 2)))
(imf "kb-rom-ship-ca" ((64 64) (file "kobayashi.gif" std 2 3)))
(imf "kb-st-station-ds9" ((64 64) (file "kobayashi.gif" std 2 4)))
(imf "kb-sw-ship-ca" ((64 64) (file "kobayashi.gif" std 2 5)))
(imf "kb-sw-fighter-z95" ((64 64) (file "kobayashi.gif" std 2 6)))
(imf "kb-sw-ship-corcorv" ((64 64) (file "kobayashi.gif" std 2 7)))
(imf "kb-sw-gun-ion" ((64 64) (file "kobayashi.gif" std 2 8)))
(imf "kb-emp-sd-sovereign" ((64 64) (file "kobayashi.gif" std 2 9)))

(imf "kb-kling-ship-ktinga" ((64 64) (file "kobayashi.gif" std 3 0)))
(imf "kb-kling-cloak-bop" ((64 64) (file "kobayashi.gif" std 3 1)))
(imf "kb-kling-ship-ca2" ((64 64) (file "kobayashi.gif" std 3 2)))
(imf "kb-rom-cloak-ca" ((64 64) (file "kobayashi.gif" std 3 3)))
(imf "kb-rom-ship-bc" ((64 64) (file "kobayashi.gif" std 3 4)))
(imf "kb-sw-ship-mc80" ((64 64) (file "kobayashi.gif" std 3 5)))
(imf "kb-sw-ship-dreadnought" ((64 64) (file "kobayashi.gif" std 3 6)))
(imf "kb-sw-ship-dd" ((64 64) (file "kobayashi.gif" std 3 7)))
(imf "kb-sw-ship-dd2" ((64 64) (file "kobayashi.gif" std 3 8)))
(imf "kb-emp-sd-super" ((64 64) (file "kobayashi.gif" std 3 9)))

(imf "kb-kling-ship-brel" ((64 64) (file "kobayashi.gif" std 4 0)))
(imf "kb-kling-cloak-brel" ((64 64) (file "kobayashi.gif" std 4 1)))
(imf "kb-kling-ship-dd" ((64 64) (file "kobayashi.gif" std 4 2)))
(imf "kb-st-ship-green" ((64 64) (file "kobayashi.gif" std 4 3)))
(imf "kb-st-station-small" ((64 64) (file "kobayashi.gif" std 4 4)))
(imf "kb-emp-st" ((64 64) (file "kobayashi.gif" std 4 5)))
(imf "kb-sw-ship-mc90" ((64 64) (file "kobayashi.gif" std 4 6)))
(imf "kb-emp-atat" ((64 64) (file "kobayashi.gif" std 4 7)))
(imf "kb-sw-ship-escort" ((64 64) (file "kobayashi.gif" std 4 8)))
(imf "kb-sw-gun-blaster" ((64 64) (file "kobayashi.gif" std 4 9)))

(imf "kb-rom-ship-warbird" ((64 64) (file "kobayashi.gif" std 5 0)))
(imf "kb-rom-ship-cl" ((64 64) (file "kobayashi.gif" std 5 1)))
(imf "kb-rom-ship-ca2" ((64 64) (file "kobayashi.gif" std 5 2)))
(imf "kb-st-cloak-green" ((64 64) (file "kobayashi.gif" std 5 3)))
(imf "kb-bubble-open" ((64 64) (file "kobayashi.gif" std 5 4)))
(imf "kb-emp-tie-bomber" ((64 64) (file "kobayashi.gif" std 5 5)))
(imf "kb-emp-atst" ((64 64) (file "kobayashi.gif" std 5 6)))
(imf "kb-emp-ship-assault" ((64 64) (file "kobayashi.gif" std 5 7)))
(imf "kb-sw-fighter-bwing" ((64 64) (file "kobayashi.gif" std 5 8)))
(imf "kb-sw-gun-double" ((64 64) (file "kobayashi.gif" std 5 9)))

(imf "kb-st-ship-bb" ((64 64) (file "kobayashi.gif" std 6 0)))
(imf "kb-rom-cloak-cl" ((64 64) (file "kobayashi.gif" std 6 1)))
(imf "kb-rom-cloak-ca2" ((64 64) (file "kobayashi.gif" std 6 2)))
(imf "kb-rom-ship-dd" ((64 64) (file "kobayashi.gif" std 6 3)))
(imf "kb-bubble-lit" ((64 64) (file "kobayashi.gif" std 6 4)))
(imf "kb-emp-tie-fighter" ((64 64) (file "kobayashi.gif" std 6 5)))
(imf "kb-emp-shuttle" ((64 64) (file "kobayashi.gif" std 6 6)))
(imf "kb-emp-tie-int" ((64 64) (file "kobayashi.gif" std 6 7)))
(imf "kb-sw-fighter-snow" ((64 64) (file "kobayashi.gif" std 6 8)))
(imf "kb-sw-fighter-awing" ((64 64) (file "kobayashi.gif" std 6 9)))

(imf "kb-st-ship-green2" ((64 64) (file "kobayashi.gif" std 7 0)))
(imf "kb-st-ship-nose" ((64 64) (file "kobayashi.gif" std 7 1)))
(imf "kb-st-ship-nose2" ((64 64) (file "kobayashi.gif" std 7 2)))
(imf "kb-st-ship-black" ((64 64) (file "kobayashi.gif" std 7 3)))
(imf "kb-city-gray" ((64 64) (file "kobayashi.gif" std 7 4)))
(imf "kb-emp-ship-carrack" ((64 64) (file "kobayashi.gif" std 7 5)))
(imf "kb-emp-tie-adv" ((64 64) (file "kobayashi.gif" std 7 6)))
(imf "kb-emp-ship-carrier" ((64 64) (file "kobayashi.gif" std 7 7)))
(imf "kb-sw-ship-bulk" ((64 64) (file "kobayashi.gif" std 7 8)))
(imf "kb-sw-ship-falcon" ((64 64) (file "kobayashi.gif" std 7 9)))

(imf "kb-st-cloak-green2" ((64 64) (file "kobayashi.gif" std 8 0)))
(imf "kb-st-cloak-brel" ((64 64) (file "kobayashi.gif" std 8 1)))
(imf "kb-rom-ship-cl2" ((64 64) (file "kobayashi.gif" std 8 2)))
(imf "kb-st-ship-small" ((64 64) (file "kobayashi.gif" std 8 3)))
(imf "kb-sw-station-drydock" ((64 64) (file "kobayashi.gif" std 8 4)))
(imf "kb-emp-ship-interedictor" ((64 64) (file "kobayashi.gif" std 8 5)))
(imf "kb-emp-ship-lancer" ((64 64) (file "kobayashi.gif" std 8 6)))
(imf "kb-emp-sd-imperial" ((64 64) (file "kobayashi.gif" std 8 7)))
(imf "kb-sw-ship-freighter" ((64 64) (file "kobayashi.gif" std 8 8)))
(imf "kb-sw-ship-ugly" ((64 64) (file "kobayashi.gif" std 8 9)))

(imf "kb-kling-ship-bb" ((64 64) (file "kobayashi.gif" std 9 0)))
(imf "kb-st-station-blue" ((64 64) (file "kobayashi.gif" std 9 1)))
(imf "kb-rom-cloak-cl2" ((64 64) (file "kobayashi.gif" std 9 2)))
(imf "kb-kling-ship-bb2" ((64 64) (file "kobayashi.gif" std 9 3)))
(imf "kb-st-feature-nebula" ((64 64) (file "kobayashi.gif" std 9 4)))
(imf "kb-sw-fighter-naboo" ((64 64) (file "kobayashi.gif" std 9 5)))
(imf "kb-emp-sd-victory" ((64 64) (file "kobayashi.gif" std 9 6)))
(imf "kb-sw-ship-droid" ((64 64) (file "kobayashi.gif" std 9 7)))
(imf "kb-sw-ship-generic" ((64 64) (file "kobayashi.gif" std 9 8)))
(imf "kb-sw-ship-mc83" ((64 64) (file "kobayashi.gif" std 9 9)))

