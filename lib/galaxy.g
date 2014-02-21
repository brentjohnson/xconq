(game-module "galaxy"
  (title "Galaxy 1")
  (blurb "Original Galaxy game by Henry Ware & Victor Mascari.")
  ; (version "1.0")
  (picture-name "galaxy")
  (variants (see-all) ("Galaxy Seen" world-seen) (sequential false) (world-size))
)

; UNITS

(unit-type |photon torpedo| (image-name "kb-explosion-orange")
  (help "shoot these at your enemy"))
(unit-type storm-trooper (image-name "kb-emp-st")
  (help "for capturing planets, moons, & death stars"))
(unit-type |SF-1 fighter| (image-name "kb-emp-tie-fighter")
  (help  "moves fast, can hit most things"))
(unit-type transport (image-name "kb-emp-ship-assault")
  (help "for transporting the troopers"))
(unit-type |light cruiser| (image-name "kb-emp-ship-carrack")
  (help "quick lightly armoured cruiser"))
(unit-type battlecruiser (image-name "kb-emp-sd-imperial")
  (help "specially designed for fleet action"))
(unit-type dreadnought (image-name "kb-emp-sd-super")
  (help "heavily armoured with deadly phasers"))
(unit-type |death star| (image-name "death-star")
  (help "make your enemies afraid, very afraid..."))
(unit-type moon (image-name "moon")
  )
(unit-type planet (image-name "planet")
  (help "makes stuff (esp. cruisers, dreadnoughts & deathstars)"))

(define p |photon torpedo|)
(define s storm-trooper)
(define f |SF-1 fighter|)
(define t transport)
(define l |light cruiser|)
(define b battlecruiser)
(define d dreadnought)
(define @ |death star|)
(define x moon)
(define X planet)

; MATERIALS

(material-type photons
  (notes "fuels photon torpedos"))
(material-type matter
  (notes "part of the fuel for ships' movement"))
(material-type anti-matter
  (notes "part of the fuel for ships' movement & phasers"))
(material-type o2
  (notes "for the troopers to breathe"))

(define ph photons)
(define m matter)
(define a anti-matter)
(define o o2)

; TERRAIN

;(set grid-color "white")
;(set unseen-color "black")
;(set unseen-image-name "black")
(terrain-type vacuum (image-name "navy-blue") (char "."))
(terrain-type nebula (image-name "mists") (char "%"))
(terrain-type blackhole (image-name "black") (char "!"))

(set alt-blob-density 10000)
(set alt-blob-height 500)
(set alt-blob-size 50)
(set alt-smoothing 1)
(set wet-blob-density 2000)
(set wet-blob-size 100)

(add t* alt-percentile-min ( 0   0  98))
(add t* alt-percentile-max (98  98 100))
(add t* wet-percentile-min ( 0  90   0))
(add t* wet-percentile-max (90 100 100))

; should have low blob density

;; DEFINES

(define places (@ x X))
(define planets (x X))
(define movers (p s f t l b d @))
(define starships (t l b d))
(define hosts (t l b d @ x X))
(define guests (p s f))

(table vanishes-on
  (u* blackhole true)
  ;; no troops allowed out in space
  (s t* true)
)

;; STARTUP

(add places point-value 5000)
(add (s f t l b d @ p) point-value (256 256 255 24 10 200 230 256))

(add vacuum country-terrain-min 6)

(table favored-terrain
  (u* t* 0)
  (starships vacuum 100)
  (places blackhole 0)
  (places vacuum (70 50 85))
  (places nebula (30 50 15))
  )

(add (X x t l b d @ s f) start-with (2 3 2 2 2 1 1 4 5))

(table independent-density ((X x) vacuum (20 40)))

(set country-radius-min 3)
(set country-separation-min 20)
(set country-separation-max 100)

(add (X x) already-seen 100)

(add (X x) see-always true)

;; Units always start out full of everything.

(table unit-initial-supply (u* m* 20000))

;; THINGS OUT OF CONTROL

;10000 blackhole u* accident
;"has been sucked into a BLACK HOLE!!" u* accident-message

;; THINGS IN CONTROL

;; Production and Repair

(add movers cp (3  2  3  8  9 15 30 150))

(table can-create
  ((x X) movers 1)
  ((b d @) p 1)
  (@ f 1)
  )

(table acp-to-create
  ((x X) movers 1)
  ((b d @) p 1)
  (@ f 1)
  )

(table cp-on-creation
  ((x X) movers 1)
  (X movers (2 1 1 1 3 4 11 121))
  ((b d @) p (2 3 3))
  (@ f 1)
  )

(table can-build
  ((x X) movers 1)
  ((b d @) p 1)
  (@ f 1)
  )

(table acp-to-build
  ((x X) movers 1)
  ((b d @) p 1)
  (@ f 1)
  )

(table cp-per-build
  (u* u* 1)
  )

(table consumption-on-creation
  (p ph 5)
  )

(add (@ x X) hp-recovery 2.00)

(table auto-repair
  (u* u* 0)
  ((l b d) f 1.00) 
  ((@ x X) (f t l) 1.00)
  ((@ x X) (b d) 2.00)
)

(table auto-repair-range
  (u* u* -1)) ; repair occupants only

;1 movers [ @ x X ] repair
;1 f starships repair
;2 d d repair
;3 starships starships repair

;; Materials

(table base-production
  (starships (ph m a) 3)
  (d ph 5)
  (@ m* 30)
  (x m* 40)
  (X m* 50)
  )

;;; the following doesn't make any sense to sts - nebulas
;;; should be better for material production, not worse
;100 t* u* productivity
;50 nebula u* productivity 
;100 nebula [ x X @ ] productivity

(table unit-storage-x
  ;;      p s  f    t   l   b    d    @     x     X
  (u* ph (1 0  0    0  15  20   30   30    60    90)) 
  (u*  m (0 0 34 1500 300 550 1000 5000 10000 20000))
  (u* o2 (0 1  0 1500 300 550 1000 5000 10000 20000))
  (u*  a (0 0  5 1500 300 550 1000 5000 10000 20000))
  )

(table base-consumption
  ((f t l b d) m 1)
  ((f t l b d) a 1)
  (p ph 1)
  (s o2 1) ; storm-troopers must die in space
  )

(table consumption-per-move
  (f m 1)
  ((t l b d @) (m a) 1)
  (s o2 1)
  )

(table hp-per-starve
  (p ph 1.00)
  (s o2 1.00)
  )

(table consumption-per-attack
  (f a 1)
  ((t l b d @ x X) a 2)
  )
(table hit-by (u* a 1))

;0 r* u* out-length
;0 r* u* in-length
;-1 r* [ f s ] out-length 
;-1 r* [ x X ] in-length
;0 s t in-length ; 

; CAPACITY

(table unit-capacity-x
  ((l b d) f (1 2 3))
  (t s 5)
  (@ u* 10)
  (@ places 0)
  (@ d 1)
  ((b d) p (2 5))
  (x u* 10)
  (X u* 20)
  (planets planets 0)
  )

;1 u* volume
;100 u* hold-volume

; MOVEMENT

;                     p  s  f  t  l  b  d  @  x  X
(add u* acp-per-turn (7  1 17  7 12 10 12  4  1  1))

(add (x X) speed 0)

(table mp-to-enter-terrain
  (u* nebula 2)
)

;17 u* f enter-time
;100 u* p enter-time

(table material-to-move
  (f m 1)
  ((t l b d @) (m a) 1)
  )

;; SIGHT

; nebulas hide stuff
(table visibility
  (u* t* 100)
  (movers nebula 20))

;                     p s f t l b d @ x X
(add u* vision-range (1 1 2 2 3 3 4 7 2 4))

;1 p see-best   ;1
;0 p see-worst
;100 [ X x d @ b l ] see-best 
;60 [ X x d @ b l f t ]  see-worst

;COMBAT

;                        p  s  f  t  l  b  d  @  x   X
(add u* hp-max          (1  1  2  2  5  9 14 31 65 110))
;(add u* hp-at-max-speed (0 0 0 1 2 4  5  0  5  5))

(table hit-chance
;;       p  s   f   t   l   b   d   @   x   X
  (p u* (0 100 100 100  95  90  85  99 100 100))
  (s u* (0  50  10  30  30  30  20  10  50  50))
  (f u* (0  50  50  70  40  35  30   2   0   0))
  (t u* (0  20  20  50  20  20  20  20   0   0))
  (l u* (0  70  70  70  50  35  30  30   0   0))
  (b u* (0  90  60  85  65  50  40  40   0   0))
  (d u* (0 100  70  95  75  60  50  50   0   0))
  (@ u* (0  60  85  90  80  80  80  50  85  60))
  (x u* (0  10  10  30  20  10   5   5   0   0))
  (X u* (0  20  20  40  30  20  10  10   0   0))
  )

(table damage
  (u* u* 1)
  (p u* 2)
  (d u* 2)
  (f d 2)
  (f @ 999)
  (@ u* 50) ;if you see a death star, you should be terrified
  (@ @ 5)   ; (but not so much if you have one yourself!)
  (@ (x X) 999)
  )

;true p self-destruct
;false p can-counter

;50 guests hosts protect
;80 u* places protect
;0 places places protect 
;30 @ [ x X ] protect 
;80 p u* protect

(table capture-chance
  (s places (20 50 30))
  )

(add movers acp-to-disband 1)
(add movers hp-per-disband 99)

;GENERAL STUFF

(add u* acp-to-change-side 1)
(add s acp-to-change-side 0)

(add s possible-sides (not "independent"))

;;; Scoring.

(scorekeeper (do last-side-wins))

;;; Text.

(set action-notices '(
  ((destroy u* p) (actor " detonates on " actee "!"))
  ((destroy u* s) (actor " annihilates " actee "!"))
  ((destroy u* l) (actor " zorches " actee "!"))
  ((destroy u* b) (actor " ZORCHES " actee "!"))
  ((destroy u* d) (actor " ZORCHES " actee "!"))
  ))

(set event-notices '(
  ((unit-starved s) (0 " suffocates!"))
  ((unit-starved f) (0 " runs out of fuel and disintegrates!"))
  ((unit-starved p) (0 " misses!"))
  ))

(set event-narratives '(
  ((unit-starved s) (0 " suffocated"))
  ((unit-starved f) (0 " ran out of fuel and disintegrated"))
  ((unit-starved p) (0 " missed"))
  ))

(set action-movies '(
  ((attack p) (sound "explosion-long"))
  ((attack f) (sound "laser"))
  ((attack |SF-1 fighter|) (sound "laser"))
  ((attack l) (sound "laser"))
  ((overrun l) (sound "laser"))
  ((overrun |light cruiser|) (sound "laser"))
  ))

;; NAMES 

;;; Sort of a mishmash, but who cares...

(set side-library '(
  (10 (name "Federation") (adjective "Federation"))
  (10 (name "Klingon Empire") (adjective "Klingon"))
  ((adjective "Vulcan"))
  ((adjective "Jedi"))
  ((name "Romulus") (adjective "Romulan"))
  ((adjective "Rebel"))
  ((adjective "Corazan"))
  ((name "Earth") (adjective "Terran"))
  ((adjective "Cylon"))
  ((adjective "Orion"))
  ((adjective "Ferengi"))
  ((adjective "Zen"))
  ((adjective "Time Lord"))
  ((adjective "Kharg"))
  ((adjective "Cyborg"))
  ((adjective "Dalek"))
  ((adjective "Kelvin"))
  ((adjective "Goth"))
  ((adjective "Vogon"))
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
"Io" "Ganymede" "Europa" "Rigel" "Rigel VII"
"Rigel XII" "DeLorian" "Pyrrus" "New Warth"
"Dorsai" "Xax" "Cygnus X-1" ; its really a black hole but..
"Terminus"  "Felicity" "Sanction" "goes BOOM"
))

(set meridians false)

(game-module (notes (
  "This period has a number of strange features:"
  " Stormtroopers are terriblly loyal. On your command,"
  "  they will move into outer-space and suffocate."
  " Photon torpedos are generally the most effective means"
  "  of destroying enemy ships; but hold your fire until "
  "  you have enough for the kill."
  " SF-1 fighters have a \"Skywalker\" chance of destroying Death"
  "  Stars.  Naturally, its a suicide run.  "
  " Blackholes will eat any ships you put into them.  So"
  "  watch for them."
  ""
  "The machine players haven't figured any of this out. "
  " Thus, their play is less than wonderful."
  ""
  ""
;; this should probably be part of the module header,
;; so can be displayed in appropriate contexts.
  " by Victor Mascari"
  "Copyright 1991, Henry Ware & Victor Mascari"
  "Please distribute this, rewrite it etc. However,"
  "you may not sell it, and this message must remain"
  "intact.  Of course, if you do any of these things, we"
  "will never know."
  "Converted to version 7 by Stan Shebs."
  ""
  " Thanks to the writer of the 'Starwars'"
  "  period for icons and ideas, and to "
  "  Henry Ware for some icons and technical"
  "  advice."
  "This period was written by Victor Mascari at The Ohio State University."
  ""
  "Strategy depends upon size of the map you're playing on. The smaller the"
  "map, the smaller unit that should be built. Large maps require big fleets."
  ""
  "Fleets tend to be the preferred method of warfare. In addition, the person"
  "that attacks first usually wins, so gather as much intelligence about"
  "the movements of the enemy as possible, and keep main battle fleet away "
  "from his until you can strike with full force."
  ""
  "Nebula hide ships 80% of the time, but also slow them down."
  ""
  "5 photon torpedos are needed to destroy a moon, 8 for a planet."
  ""
  "Blackholes are hard to spot, so pay attention to where your moving or else"
  "you'll loose your ship."
  ""
  "The computer is fairly stupid in this period and is fairly easy to defeat. The"
  "skills learned to defeat the computer will, in general, not apply to human"
  "opponents."
)))

(add @ notes (
  "The Death Star is the most powerful unit in this period.  In close quarters"
  "other units can be flattened, however Death Stars can be captured by"
  "storm troopers and have some vulnerability to fighters. They are the"
  "only moving unit which repairs itself once a turn."
))
(add p notes (
  "The Photon Torpedo is the main means of attack.  It has a range of seven"
  "and does two points of damage to any other unit.  It's wise to save these"
  "until you have enough to kill a big ship in one turn: wounded ships often"
  "make it home."
))
(add s notes (
  "Storm Trooper suffocate in outer space, so don't put them there.  "
))
(add f notes (
  "Fighters are good for reconnaissance and for harrasing the enemy battle"
  "cruisers.  Send them, take a point of damage, and come home. As"
  "previously mentioned they have a slim chance of destroying a Death Star"
  "in one hit, however, its a suicide run."
))
(add t notes (
  "Transports are, naturally, vulnerable and sorta slow."
))
(add l notes (
  "Light Cruisers are the smallest of the fleet ships.  They are very vulnerable"
  "in fleet combat however, especially to dreadnoughts.  They are good at"
  "reconnaissance.  The main use of light cruisers"
  "is to provide bait to attract his ships or, occasionally, to eat up his"
  "photon torpedos."
))
(add b notes (
  "Battle Cruisers provide photon torpedos.  Generally, they provide the"
  "backbone of a battle fleet."
))
(add d notes (
  "Dreadnoughts are the largest of the fleet ships and are tough in close"
  "combat.  But, despite the name, it best to play these ships with a"
  "certain amount of caution; one dreadnought can not take on a fleet,"
  "and Death Stars demand a respectful amount of distance."
))

(add x notes (
  "Planets and moons are the main producers.  Generally, the more powerful"
  "the unit to be constructed, the better it is to produce it in a planet."
  "As for the defense of these units, there are several schools of thought."
  "One school says the best defense is a good offense.  Another holds"
  "that fighter patrols are cheap and effective against enemy landings."
  "Or that a loaded transport should be kept to retake any enemy conquests,"
  "and as defense against a deathstar.  Some people like to supply their"
  "planets with a large number of torpedos.  Finally, some people like to"
  "let the enemy bombard their planets, thus 'wasting' torpedos and "
  "allowing the defender to destroy the attacking fleet.   "
))

(add X notes (
  "Planets and moons are the main producers.  Generally, the more powerful"
  "the unit to be constructed, the better it is to produce it in a planet."
  "As for the defense of these units, there are several schools of thought."
  "One school says the best defense is a good offense.  Another holds"
  "that fighter patrols are cheap and effective against enemy landings."
  "Or that a loaded transport should be kept to retake any enemy conquests,"
  "and as defense against a deathstar.  Some people like to supply their"
  "planets with a large number of torpedos.  Finally, some people like to"
  "let the enemy bombard their planets, thus 'wasting' torpedos and "
  "allowing the defender to destroy the attacking fleet.   "
))


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


