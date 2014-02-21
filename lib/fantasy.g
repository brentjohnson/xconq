(game-module "fantasy"
  (title "Fantasy and Magic")
  (blurb "Long-ago times, that perhaps never were...")
  (variants
   (see-all false)
   (world-seen false)
   (sequential false)
   (world-size)
   ("Alternate economy" model1
    "Use a different algorithm for moving materials among units."
    (true (set backdrop-model 1)))
   ("AltEcon uses doctrine" model1doctrine
    "When using alternate economy, don't drain units below doctrine levels."
    (true (set backdrop-ignore-doctrine 0))
    (false (set backdrop-ignore-doctrine 1)))
   )
  )

;;; Unit types.

;; Movers & fighters.

(unit-type wizard (char "W")
  (help "magician, wielder of awesome magic"))
(unit-type i (name "light infantry") (image-name "peltast") (char "i")
  (help "foot soldiers and mercenaries"))
(unit-type barbarian (image-name "viking") (char "b")
  (help "rugged, ferocious, lightly armoured warriors"))
(unit-type k (name "heavy infantry") (image-name "hoplite") (char "k")
  (help "foot soldiers in heavy armour"))
(unit-type paladin (char "P")
  (help "holy warrior, extremely tough and well equipped"))
(unit-type longbowmen (image-name "archer") (char "R")
  (help "archers, scouts, rangers"))
(unit-type priest (char "H")
  (help "the spiritual guides of the land"))
(unit-type serfs (image-name "serf")
  (help "mobs of peasant serfs and levies"))
(unit-type dragon (char "D")
  (help "lays waste to the lands"))
(unit-type undead
  (help "undead warriors, raised by priests or wizards"))
(unit-type cavalry (char "h")
  (help "knights that ride horses"))

;; Magic spells.

(unit-type A (name "Armageddon spell") (image-name "mininuke") (char "A")
  (help "spell of awesome destruction"))
(unit-type lightning (char "L")
  (help "spell good for killing tough things"))
(unit-type fireball (char "F")
  (help "spell great for causing lots of damage to soft targets"))
(unit-type M (name "mass charm") (image-name "charm-spell") (char "M")
  (help "spell that charms units over to the caster's side!"))
(unit-type wizard-eye (name "wizard eye") (image-name "eye") (char "E")
  (help "spell for spying on things"))

;; Transport.

(unit-type wagon (image-name "wagon-2") (char "G")
  (help "wagons, logistical unit"))
(unit-type flying-carpet (name "flying carpet") (char "C")
  (help "magical flying carpet"))
(unit-type cloudkeep (image-name "city30") (char "K")
  (help "wizard's floating stronghold"))
(unit-type sailing-ship (name "sailing ship") (image-name "frigate") (char "S")
  (help "for traveling the seas"))
(unit-type galley (image-name "trireme")
  (help "fast, light, not so seaworthy warship"))

;; Forts, bases and other special places.

(unit-type wall (image-name "walltown")
  (help "fortified walls"))
(unit-type castle
  (help "walled castle"))
(unit-type tower (char "T")
  (help "mage's fortified tower"))
(unit-type camp (char "/")
  (help "campsite, practically a mobile village"))
(unit-type temple (image-name "parthenon")
  (help "holy place of spiritual power"))

; Cities.

(unit-type village (image-name "village-2") (char "V")
  (help "small town"))
(unit-type town (image-name "village") (char "*")
  (help "smaller than a city"))
(unit-type city (name "city") (image-name "city15") (char "@")
  (help "large city, capital of a city-state"))

;; Magical items.

(unit-type r (name "ring of regeneration") (image-name "ring-2") (char "r")
  (help "magical ring that makes its wearer heal faster"))
(unit-type crystal-ball (name "crystal ball") (char "B")
  (help "magical device that scans the local area"))
(unit-type p (name "ring of protection") (image-name "ring-2") (char "p")
  (help "protective magical device"))
(unit-type f (name "everfull plate") (image-name "plate") (char "f")
  (help "magical wonder that creates food!"))
(unit-type a (name "amulet of power") (image-name "amulet") (char "a")
  (help "generates mana for its wearer"))

;; Material types.

(material-type food
  (help "foodstuffs: an army marches on its stomach!"))
(material-type mana 
  (help "magical power"))	; used to produce magical effects
(material-type metal
  (help "steel, metal, manufacturing capacity for armour and weapons"))

;; Terrain types.

(terrain-type sea (char "."))
(terrain-type swamp (char "="))
(terrain-type desert (char "~")(image-name "adv-desert"))
(terrain-type plains (char "+")(image-name "adv-plain"))
(terrain-type forest (char "%")(image-name "adv-forest"))
(terrain-type hills (char "(")(image-name "adv-hills"))
(terrain-type mountains (char "^")(image-name "adv-mountains"))
(terrain-type ice (char "_")(image-name "adv-ice"))
(terrain-type void (image-name "black") (char ":"))

(define W wizard)

(define b barbarian)

(define P paladin)
(define R longbowmen)
(define H priest)
(define s serfs)
(define D dragon)
(define u undead)
(define h cavalry)

(define L lightning)
(define F fireball)
(define E wizard-eye)

(define G wagon)
(define C flying-carpet)
(define S sailing-ship)
(define g galley)

(define w wall)
(define c castle)
(define T tower)
(define K cloudkeep)
(define _ camp)
(define t temple)
(define V village)
(define ~ town)
(define @ city)

(define B crystal-ball)

(define creatures (W i b k P R H s D u h))
(define spells (A lightning fireball M wizard-eye))
(define bases (wall castle tower temple cloudkeep camp village town city))
(define transport (wagon flying-carpet cloudkeep sailing-ship galley))
(define cities (village town city))
(define magic-items ( r crystal-ball p f a ))

(define forts (wall castle tower cloudkeep))
(define flyers (dragon flying-carpet cloudkeep))
(define ships (sailing-ship galley))
(define ltfoot (priest longbowmen barbarian i serfs))
(define hvyfoot ( k paladin wizard ))
(define hoof (cavalry wagon))
(define magickers (wizard dragon))
(define movers (append creatures transport))
(define foot ( W i b k P R H s u ))
(define mundanes (i b k P R H s h G S g))

(define water ( sea ))
(define land ( swamp plains forest desert hills mountains ))

(add water liquid true)

;;; Static relationships.

;; Unit-unit.

(add wizard capacity 10)
(add priest capacity 1)		; for repair?
(add dragon capacity 3)

(add transport capacity (2 1 8 5 4))

(add bases capacity (2 4 4 4 12 10 16 20 30))

(table unit-size-as-occupant
  (u* u* 99)
  ((i k serfs) galley 4)
  (longbowmen galley 2)
  (spells wizard 1)
  (fireball dragon 1)
  (spells (tower cloudkeep) 1)	; resident mages handle these
  (movers bases 1)
  (movers transport 1)
  ((dragon wagon cavalry sailing-ship galley ) u* ( 3 2 2 4 3 ))
  ((wizard paladin) dragon 1)	; dragonriders
  (magic-items u* 0)		; magic items are negligible in size
  (magic-items spells 99)	; ..but not to spells
  )

(table occupant-max
  (wizard spells 6)
  (dragon spells 1)
  (u* magic-items 6)
  )

;; Unit-terrain relationships.

;; Unit-material relationships.

(table unit-storage-x 
  (wizard m* ( 10 100 6 ))
  (i m* ( 16  0 10 ))
  (b m* ( 16  0  6 ))
  (k m* ( 16  0 12 ))
  (P m* ( 12  4  6 ))
  (R m* ( 16  0  6 ))
  (H m* ( 10  8  6 ))
  (s m* ( 10  0  6 ))
  (dragon m* ( 200 30 6 ))	; dragons can eat alot!
  (u m* (  0  0  4 ))
  (spells food 0)
  (spells metal 0)
  (spells mana 3)
  (L mana 2)
  (E mana 8)	; it has extra duration
  (C m* (  0  10 2 ))
  (G m* ( 150 0  6 ))
  (K m* ( 50 200 10 ))
  (h m* ( 50  0 20 ))
  (S m* ( 100 0 30 ))
  (g m* (  50 0 15 ))
  (w m* (  20 0  6 ))
  (c m* ( 200 0 30 ))
  (T m* ( 100 60 20 ))
  (_ m* ( 100 0 25 ))
  (t m* ( 100 8 20 ))
  (village m* ( 150 0 15 ))
  (town m* ( 300 0 30 ))
  (city m* ( 600 0 60 ))
  (r m* 10)
  (f food 10)
  (a mana 100)	; holds lots of power, might have it all when found!
  )

;;; Vision.

(add cities see-always 1)

;; Dragons are hard to miss...

(add dragon see-always 1)

(add flyers vision-range 3)
(add spells vision-range -1)
(add wizard-eye vision-range 4)
(add magic-items vision-range -1)
(add bases vision-range 2)
(add cloudkeep vision-range 5)
(add tower vision-range 4)
(add wall vision-range 2)	; can see a little ways from walls.
(add city vision-range 3)	; Cities have tall structures.
(add town vision-range 2)
(add castle vision-range 3)
(add longbowmen vision-range 2)	; Archers are good at knowing the surrounding land.
(add (galley sailing-ship) vision-range ( 2 3 ))	; ships can see far over flat water
(add crystal-ball vision-range 10)

; visibility of stuff
; non 0/1 visibility must be harsh on CPU! keep # of units with this down
;FIX visibility is U T -> N, U's % visibility in T
(table visibility add (cloudkeep t* 20))	; hard to spot in them clouds!
(table visibility add (C t* 5))	; hard to spot in the clouds
(table visibility add (R t* 50))	; Archers are good at hiding on the land.
(table visibility add (E t* 0))	; invisible spy-eye
(table visibility add (magic-items t* 20))	;  magic items are sometimes hard to spot.

; small forces hidden by rough terrain
;FIX conceal is now visibility, you should subtr these numbers from that table
;FIX ( 80 75 60 40 ) ( forest swamp mountains hills ) R conceal

;;; Actions.

;                         W i b k P R H s  D u h G C  K S g
(add movers acp-per-turn (1 2 3 2 2 3 2 1 10 1 6 5 12 4 5 7))

;                         A L F M E
(add spells acp-per-turn (2 9 5 3 6))

(add cities acp-per-turn 1)

;;; Movement.

(add bases speed 0)
(add cloudkeep speed 1.00)
(add magic-items speed 0)

(table mp-to-enter-terrain
  (u* t* 99)
  (bases land 1)
  (ships water 1)
  ;; Flyers can go almost anywhere.
  (flyers t* 1)
  ;; Spells can "go" almost anywhere.
  (spells t* 1)
  (wizard land (2 1 1 1 1 2))
  (i land (2 1 1 1 1 2))
  (b land (2 1 2 1 1 2))
  (k land (2 1 2 2 2 2))
  (P land (1 1 1 1 2 2))
  (R land (1 1 1 1 1 2))
  (H land (2 1 2 2 2 2))
  (s land (2 1 2 1 2 2))
  (u land (1 1 1 1 1 2))
  (G land (99 1 2 2 3 4))
  (h land (3 1 2 2 3 4))
  ;; Some type can handle ice!
  (( wizard barbarian paladin longbowmen undead ) ice 1)
  ;; Nothing can go into the void!
  (u* void 99)
  )

(table consumption-per-move
  ;; All units take 1 fuel to move.
  (movers food 1)
  ;; This is actually lost food production.
  (s food 4)
  ;; Magical things don't require food to move....
  (u food 0) 
  (cloudkeep food 0)
  (cloudkeep mana 1)	; floating castle requires mana to move
  (spells food 0)
  (spells mana 1)
  (C food 0)
  (S food 0)	; they sail!
  )

(table mp-to-enter-unit
  (flyers u* 20)
  ;; Once launched, spells can't be recovered.
  (spells u* 99)
  (ships bases 20)
  (ships bases 1)
  )

;; Let everything attack everywhere, no terrain immunity.

(table ferry-on-entry (u* u* over-all))

;;; Construction.

;; Mages must first figure out how to cast their spells!
#|
(add spells tech-max 200)
(add M tech-max 300)
(add A tech-max 1000)
(add magic-items tech-max 50)
(add spells tech-to-build 200)
(add M tech-to-build 300)
(add A tech-to-build 1000)
(add magic-items tech-to-build 50)

(table acp-to-develop
  (wizard spells 1)
  (wizard magic-items 1)
  )

(table tech-per-develop
  (wizard spells 1.00)
  (wizard magic-items 1.00)
  )
|#

(add spells cp 2)

(add (dragon undead flying-carpet cloudkeep tower) cp (25 2 10 30 10))

(add bases cp (6 25 15 20 20 6 30 35 40))

(add ( i b  R  H s G  h  S  g  W  k  P )
  cp ( 6 8 10 18 6 7 18 19 16 30  9 20))

(add camp cp 7)
(add temple cp 15)
(add magic-items cp (12 12 9 6 15))

(table can-create
  (wizard spells 1)
  (wizard (dragon undead flying-carpet cloudkeep tower) 1)
  (wizard magic-items 1)
  ((i k R) camp 1)
  ;; Paladins and priests can make shrines.
  ((paladin priest) temple 1)
  ;; Priest can raise the dead, despite dubious holiness of it...
  (priest undead 1)
  (priest magic-items 1)
  (priest a 0)
  (dragon fireball 1)
  (serfs bases 1)
  (serfs cloudkeep 0)
  (castle (i k paladin cavalry) 1)
  (tower wizard 1)
  (cloudkeep spells 1)
  (temple (paladin priest) 1)
  (cities mundanes 1)
  ;; Villages have some limitations on the training possible.
  (village (k paladin) 0)
  (city wizard 1)
  )

(table acp-to-create
  (wizard spells 1)
  (wizard (dragon undead flying-carpet cloudkeep tower) 1)
  (wizard magic-items 1)
  ((i k R) camp 1)
  ;; Paladins and priests can make shrines.
  ((paladin priest) temple 1)
  ;; Priest can raise the dead, despite dubious holiness of it...
  (priest undead 1)
  (priest magic-items 1)
  (priest a 0)
  (dragon fireball 1)
  (serfs bases 1)
  (serfs cloudkeep 0)
  (castle (i k paladin cavalry) 1)
  (tower wizard 1)
  (cloudkeep spells 1)
  (temple (paladin priest) 1)
  (cities mundanes 1)
  ;; Villages have some limitations on the training possible.
  (village (k paladin) 0)
  (city wizard 1)
  )

(table cp-on-creation
  (wizard magic-items (3 7 2 1 1))
  (priest magic-items (1 1 1 3 1))
  ;; Castles are well-equipped to train military units.
  (castle (i k paladin cavalry) (2 2 4 4)) 
  ;; Towns and cities build faster by having new units more complete.
  (town ( i b  R  H s G  h  S  g  W  k  P )
        ( 2 2  1  4 2 1  3  6  4  1  1  1 ))
  (city ( i b  R  H s G  h  S  g  W  k  P )
        ( 3 3  1  6 2 2  5  7  8  6  3  1 ))
  )

(table can-build
  (wizard spells 1)
  (wizard (dragon undead flying-carpet cloudkeep tower) 1)
  (wizard magic-items 1)
  ((i k R) camp 1)
  (dragon fireball 1)
  ((paladin priest) temple 1)
  (priest undead 1)
  (priest magic-items 1)
  (priest a 0)
  (serfs bases 1)
  (serfs cloudkeep 0)
  (castle (i k paladin cavalry) 1)
  (tower wizard 1)
  (cloudkeep spells 1)
  (temple (paladin priest) 1)
  (cities mundanes 1)
  (village (k paladin) 0)
  (city wizard 1)
  )

(table acp-to-build
  (wizard spells 1)
  (wizard (dragon undead flying-carpet cloudkeep tower) 1)
  (wizard magic-items 1)
  ((i k R) camp 1)
  (dragon fireball 1)
  ((paladin priest) temple 1)
  (priest undead 1)
  (priest magic-items 1)
  (priest a 0)
  (serfs bases 1)
  (serfs cloudkeep 0)
  (castle (i k paladin cavalry) 1)
  (tower wizard 1)
  (cloudkeep spells 1)
  (temple (paladin priest) 1)
  (cities mundanes 1)
  (village (k paladin) 0)
  (city wizard 1)
  )

(table cp-per-build
  ;; Temples are very good at educating the priesthood.
  (temple priest 2)
  )

;; Material needed to make units.

;  mana required for magical stuff
; typical mage should get 2 to 4 mana per turn, dep on site...

(table consumption-per-built
  ;               A  L   F  M  E
  (spells mana ( 50 14  12 20  6 ))
  ((dragon undead cloudkeep flying-carpet ) mana ( 200 10 400 80 ))
  ; most units don't really take food to make...
  ; but armoured ones take metal...
  (i metal 3)
  (k metal 15)
  (P metal 15)
  (G metal 5)
  (S metal 40)
  (g metal 25)
  (w metal 20)
  (c metal 80)
  (T metal 20)
  (_ metal 3)
  (cities metal (45 60 120))
  (magic-items mana ( 30 20 25 10 60 ))
  )

;; Newly-created units start out with nothing, but magic-related things
;; get some or all of their mana for free.

(table supply-on-completion
  (magickers mana (30 10)) ; magic casters get about 1/3 of their mana capacity
  (spells mana 9999)
  (magic-items mana 9999)
  )

;;; Repair.

(add cavalry hp-recovery 0.25)
(add serfs hp-recovery 0.33)
(add (paladin R) hp-recovery 0.50)
(add dragon hp-recovery 1.00)
(add cities hp-recovery 0.25)

#| need to fix this up still
;(table acp-to-repair add ( W 1))
;(table hp-per-repair add ( W ( 33 33 33 25 50 33 33 33 33 50 33 33 33 100 100 100 100 100 100 100 100 100 100 100 100 100 100 100 )))
(table acp-to-repair add (i bases 1))
(table hp-per-repair add (i bases 50))
(table acp-to-repair add (b galley 1))
(table hp-per-repair add (b galley 50))
(table acp-to-repair add (b bases 1))
(table hp-per-repair add (b bases 33))
(table acp-to-repair add (k castle 1))
(table hp-per-repair add (k castle 50))
(table acp-to-repair add (P bases 1))
(table hp-per-repair add (P bases 50))
(table acp-to-repair add (R _ 1))
(table hp-per-repair add (R _ 50))
(table acp-to-repair add (H foot 1))
(table hp-per-repair add (H foot 100))
(table acp-to-repair add (s bases 1))
(table hp-per-repair add (s bases 100))
(table acp-to-repair add (s G 1))
(table hp-per-repair add (s G 100))
(table acp-to-repair add (s S 1))
(table hp-per-repair add (s S 50))
(table acp-to-repair add (W cloudkeep 1))
(table hp-per-repair add (W cloudkeep 50))	;  Wizards slowly repair cloud castles
(table acp-to-repair add (G foot 1))
(table hp-per-repair add (G foot 50))

(table acp-to-repair add (bases u* 1))
(table hp-per-repair add (bases u* 100))
(table acp-to-repair add (forts u* 1))
(table hp-per-repair add (forts u* 100))
(table acp-to-repair add (cities u* 1))
(table hp-per-repair add (cities u* 100))
(table acp-to-repair add (forts forts 1))
(table hp-per-repair add (forts forts 25))	; forts have a few people who can fix them
(table acp-to-repair add (c ( i cloudkeep cavalry ) 1))
(table hp-per-repair add (c ( i cloudkeep cavalry ) 100))
(table acp-to-repair add (T wizard 1))
(table hp-per-repair add (T wizard 100))
(table acp-to-repair add (t foot 1))
(table hp-per-repair add (t foot 100))
|#

;;; Combat.

;                    W i b k P R H s D u h G  C K S  g 
(add movers hp-max ( 2 2 2 3 4 2 2 2 8 4 3 1 10 4 3 10))

;; Spells need more than 1 hp so they can survive an attack when attacking.

(add spells hp-max 2)

;                    w  c T t K _  V  ~  @
(add bases hp-max ( 10 20 8 4 4 8 15 30 45 ))

(add magic-items hp-max 1)

(add W speed-damage-effect ((1 50) (2 100)))
(add i speed-damage-effect ((1 50) (2 100)))
(add b speed-damage-effect ((1 50) (2 100)))
(add k speed-damage-effect ((1 50) (2 100)))
(add P speed-damage-effect ((2 50) (3 100)))
(add R speed-damage-effect ((1 50) (2 100)))
;; should add more?
(add D speed-damage-effect ((2 20)))

;; Split the matrix in half to make it fit on an 80 col screen.

(define u1 (  W   i   b   k   P   R   H   s   D   u   G   C   h ))
(define u2 (  A   L   F   K   S   g   w   c   T   _   t   V   ~   @ ))

; so, here we define the units by how good they are at hitting others
; ranges:  30% is typical attack percent.
; it ranges up and down from there...

(table hit-chance
  ;        W   i   b   k   P   R   H   s   D   u   G   C   h
  (W u1 ( 25  35  10  30  10  35  20  40  40  30  40  40  30 ))
  (i u1 ( 50  30  40  20  15  60  25  45  10  20  40  30  20 ))
  (b u1 ( 45  45  35  25  25  55  35  45  15  30  50  20  25 ))
  (k u1 ( 35  40  45  25  20  65  30  50  20  30  30  15  30 ))
  (P u1 ( 40  40  45  35  30  80  30  30  40  50  30  15  30 ))
  (R u1 ( 30  45  50  25  20  65  30  40  20  15  30  30  25 ))
  (H u1 ( 30  20  35  20  20  50  35  30  10  45  30  10  20 ))
  (s u1 ( 20  10  15   5   3  10  10  30   1  10  30   5  10 ))
  (D u1 ( 40  50  55  35  20  70  40  80  30  40  60  40  80 ))
  (u u1 ( 20  30  35  20  10  50  10  50  10  30  40  30  30 ))
  (A u1 ( 50  99  60  90  70  90  80  99  60  99  99  99  99 ))
  (L u1 ( 50  80  50  90  60  80  80  99  70  99  99  99  99 ))
  (F u1 ( 50  90  50  80  60  90  80  99  60  99  99  99  99 ))
  (G u1 (  0   0   0   0   0   0   0   0   0   0   0   0   0 ))
  (C u1 (  0   0   0   0   0   0   0   0   0   0   0   0   0 ))
  (K u1 ( 10   5   5   5   5   5   5   5  25   5  15  20   5 ))
  (h u1 ( 50  45  50  30  30  70  40  60  25  40  50  15  30 ))
  (S u1 (  0   0   0   0   0   0   0   0   0   0   0   0   0 ))
  (g u1 (  0   0   0   0   0   0   0   0   0   0   0   0   0 ))
  (w u1 ( 11  30  35  20  10  40  30  40  20  30   0   0  20 ))
  (c u1 ( 10  40  40  25  12  40  35  40  25  35   0   0  20 ))
  (T u1 ( 10  35  35  20  10  40  30  40  20  30   0   0  20 ))
  (_ u1 (  5  20  17  15  10  25  20  30  10  20   0   0  10 ))
  (t u1 (  8  25  30  20  18  35  30  40  15  35   0   0  15 ))
  (V u1 (  9  25  30  20  15  30  30  40  15  20   0   0  25 ))
  (~ u1 ( 10  30  35  25  20  40  35  45  20  25   0   0  30 ))
  (@ u1 ( 15  35  40  30  25  50  40  50  25  30   0   0  30 ))

  ;        A   L   F   K   S   g   w   c   T   _   t   V   ~   @
  (W u2 (  1   1   1  50  30  40  25  20  25  30  35  40  40  40 ))
  (i u2 (  1   1   1  10  10  10  10  10  10  25  20  20  20  20 ))
  (b u2 (  1   1   1  10  10  10   8   5   8  40  30  50  50  50 ))
  (k u2 (  1   1   1  10  10  10   9   9  10  20  25  30  30  30 ))
  (P u2 (  1   1   1  11   7   8  15  15  15  20  40  20  20  20 ))
  (R u2 (  1   1   1   8   5   5   8   8   8  30  30  30  30  30 ))
  (H u2 (  1   1   1   3   4   4   4   4   4  20  50   5   5   5 ))
  (s u2 (  1   1   1   1   2   2   1   2   2  20  15  20  20  20 ))
  (D u2 (  1   1   1  20  60  50  30  25  28  50  40  60  60  60 ))
  (u u2 (  1   1   1   2  10  10  11  11  10  30  20  30  30  30 ))
  ; there is some value in chances >100%, since may be adjusted
  ; downwards by other factors, but for now they're not allowed.
  ;(A u2 (  1   1   1  30 199 199 199 150 150 299  70 130 110 100 ))
  (A u2 (  1   1   1  30 100 100 100 100 100 100  70 100 100 100 ))
  (L u2 (  1   1   1  30  40  40  80  80  80  99  70  90  90  80 ))
  (F u2 (  1   1   1  30  99  99  60  50  69  99  75  90  90  90 ))
  (G u2 (  1   1   1   0   0   0   0   0   0   0   0   0   0   0 ))
  (C u2 (  1   1   1   0   0   0   0   0   0   0   0   0   0   0 ))
  (K u2 (  1   1   1  40   0   0   0   0   0   0   0   0   0   0 ))
  (h u2 (  1   1   1  10  10  10   6   6   9  40  35  30  30  30 ))
  (S u2 (  1   1   1   0  20  10   0   0   0   0   0   0   0   0 ))
  (g u2 (  1   1   1   0  50  30   0   0   0   0   0   0   0   0 ))
  (w u2 (  1   1   1   0   0   0   0   0   0   0   0   0   0   0 ))
  (c u2 (  1   1   1   0   0   0   0   0   0   0   0   0   0   0 ))
  (T u2 (  0   1   0   0   0   0   0   0   0   0   0   0   0   0 ))
  (_ u2 (  0   0   0   0   0   0   0   0   0   0   0   0   0   0 ))
  (t u2 (  0   0   0   0   0   0   0   0   0   0   0   0   0   0 ))
  (V u2 (  0   0   0   0   0   0   0   0   0   0   0   0   0   0 ))
  (~ u2 (  0   0   0   0   0   0   0   0   0   0   0   0   0   0 ))
  (@ u2 (  0   0   0   0   0   0   0   0   0   0   0   0   0   0 ))

  (E u* 0)
  (M u* 10)	; can do some damage as it attempts to capture

  ; and this allows ti to even attack...
  (movers spells 50)	;  spells can be killed by attacking them, but
  (movers M 10)
  ; they shouldn't have much time where they can be attacked.
  ; they are either in-flight, or stored on someone!
  (u* magic-items 100)
  (magic-items u* 0)
  )

(table defend-terrain-effect
  (foot ( forest swamp mountains hills ) 85)
  (hvyfoot ( forest swamp mountains hills ) 90)
  ;; Archers are good at using terrain to hide in.
  (R ( forest swamp mountains hills desert ) (25 40 40 60 90))
  (forts ( hills mountains ) 60)
  (tower ( hills mountains ) (80 50))
  (castle ( hills mountains ) (80 50))
  )

(table damage
  (u* u* 1)		; default
  (dragon u* 2)
  (u* magic-items 0)	;  otherwise, items are always destroyed by capture
  (dragon bases 3)
  (dragon cities 4)
  (barbarian bases 3)	; barbarians torch them cities!
  (R dragon 4)		; rangers with those special arrows!
  (g ships 2)
  ((paladin priest) undead 3)
  (cavalry ltfoot 1)
  (fireball ships 4)
  (fireball cities 4)
  (fireball u* 2)
  (fireball camp 4)
  (lightning u* 3)
  (M u* 1)		; it might do some damage as the victim struggles
  (A u* 21)		; Armageddon spells are nuclear!
  (magic-items u* 0)	; magic items can't hurt anything
  )

(table acp-to-defend 
  (spells u* 0)		; they're specifically targeted!
  (w u* 0)		; walls don't fight back! They're just walls!
  (magic-items u* 0)
  )

; these units may be persuaded to go traitor

(table capture-chance
  (W ( W i k s D u A L F G K T ) ( 20 20 20 40 20 40 30 30 30 20 50 40 ))
  ;                                   s  G h  S  g  w c  T  _  t  V  ~  @
  (i ( s G h S g w c T _ t V ~ @ ) ( 40 50 5 30 30 10 5  8 30 20 40 25 15 ))
  (b ( s G h S g w c T _ t V ~ @ ) ( 40 50 5 45 40  8 3  7 40 25 20 10  7 ))
  (R ( s G h S g w c T _ t V ~ @ ) ( 10 20 5 10 10  8 3  7 40 25 45 25 10 ))
  (k ( s G h S g w c T _ t V ~ @ ) ( 45 40 5 15 10 12 6  9 35 25 45 25 13 ))
  (P ( s G h S g w c T _ t V ~ @ ) ( 50 30 8 20 20 11 7 10 36 45 40 20 10 ))
  (h ( s G h S g w c T _ t V ~ @ ) ( 50 40 5  5  5  5 1  5 60 30 40 20 10 ))
  (H ( s G h           _ t V ~ @ ) ( 60 10 10              20 35 40 15 15 ))
  (u ( G S   S g w c T _ t V ~ @ ) ( 40 50   25 30 11 6  9 20 10 15 10  5 ))
  (H ( i barbarian undead ) ( 20 15 40 ))	; priests can convert people!
  (s ( serfs wagon cloudkeep camp V ~ @ ) ( 20 30 45 30 30 20 15 ))
  ; the Charm Spell...
  (M u* 100)	; the standard default
  ; the 200% of M vs _ is not accepted, though perhaps it should be.
  (M ( w c T _ t V ~ @ ) ( 100 50 100 100 #|200|# 25 60 50 40 ))
  ; works better against low populations
  (M ( wizard paladin priest dragon cloudkeep tower ) ( 50 10 40 50 25 80 ))
  ;; Magical items don't have loyalty.
  (u* magic-items 100)
  )

(table protection
  ;; Generic bases protect their occupants ok.
  (bases u* 70)
  ;; Forts protect their occupants very well.
  (forts u* 20)
  (camp movers 80)
  (village movers 80)
  (town movers 70)
  (city movers 60)
  (( i k paladin wizard ) forts 50)
  (u forts 60)
  (( i k paladin wizard ) bases 70)
  (H temple 50)	; priests and their temples...
  ;; wiz's protect their towers and keeps very well
  (wizard ( cloudkeep tower ) 20)
  ;; Ring of protection reduces hit-chance by 20%.
  (u* p 80)
  )

(table withdraw-chance-per-attack
  (u* movers 10)
  (u* longbowmen 33)	; those rangers like to run away!
  (u* ships 0)		; boats can't retreat!
  )

(table consumption-per-attack
  ;; Dragon feeds on enemies!
  (dragon food -5)
  ;; Mages use spells in combat!
  (wizard mana 1)
  ;; Cloudcastles throw stuff down to attack.
  (cloudkeep metal 1)
  )

;;; Detonation.

;; Nuke spell works by detonation rather than by conventional attack actions.

(add A acp-to-detonate 1)

(add A hp-per-detonation 2)

(table detonation-damage-at (A u* 60))

(table detonation-damage-adjacent (A u* 1))

(table detonation-terrain-damage-chance
  (A (plains forest ice) 100)
  )

(table terrain-damaged-type
  (plains desert 1)	; plains become desert always
  (forest plains 1)	; trees are all blown down
  (ice mountains 1)	; ice melts
  )

;;; Texts.

(set action-notices '(
  ((destroy A u*) (actor " vaporizes " actee "!"))
  ((destroy lightning u*) (actor " electrocutes " actee "!"))
  ((destroy fireball u*) (actor " incinerates " actee "!"))
  ((destroy u* wizard) (actor " slays " actee "!"))
  ((destroy u* paladin) (actor " slays " actee "!"))
  ((destroy u* priest) (actor " slays " actee "!"))
  ((destroy u* dragon) (actor " slays " actee "!"))
  ((destroy u* land) (actor " defeats " actee "!"))
  ((destroy u* ships) (actor " sinks " actee "!"))
  ((destroy u* flyers) (actor " knocks down " actee "!"))
  ((destroy u* bases) (actor " sacks " actee "!"))
  ))

(set event-notices '(
  ((unit-died-in-accident wizard) (0 " disappears in a puff of smoke!"))
  ((unit-died-in-accident D) (0 " returns to its lair to hibernate and disappear into legend"))
  ((unit-died-in-accident S) (0 " was lost in a storm"))
  ((unit-died-in-accident g) (0 " was lost in a storm"))
  ))

;;; Other Actions.

(add u* acp-to-disband 1)

(add u* hp-per-disband 100)

;; Magic items can't be destroyed!

(add magic-items hp-per-disband 0)

;;; Scoring.

(add bases point-value ( 1 20 8 8 30 2 8 10 50 ))
(add serfs point-value 5)	; serfs are valuable!

(scorekeeper (do last-side-wins))

(table acp-to-repair add (r u* 1))
(table hp-per-repair add (r u* 100))	; ring of regeneration magically fixes everything!
; ring must be able to hold supplies to perform repairs! Sheesh!

;;; Backdrop.

(table base-production
  (wizard mana 5)
  (dragon mana 4)
  (H mana 2)
  (cloudkeep mana 12)
  (tower mana 5)
  (t mana 2)
  (paladin mana 1)
  (r m* 1)	; ring produces everything, just in case...
  (f food 6)	; FOOD!
  (a mana 6)	; amulet of power generates mana
  (( priest serfs tower castle ) metal ( 2 4 1 2 ))
  (cities metal (4 8 16))
  ((W i b k P R H s c T _ t K) food (1 2 4 2 3 6 4 10 2 1 3 6 3))
  (cities food (8 12 16))
  )

(table productivity
  (u* t* 100)
  (foot sea 0)
  (hoof sea 0)
  ((paladin priest) sea 50)
  (magic-items t* 100)
  ;           sea swp des pln for hil mtn ice voi
  ;            .   =   -   +   %   ~   ^   _   :
  (wizard t* ( 60  60  60  40  60  80 100 120  0 ))	; 1 per 20%
  (dragon t* ( 75  50  50  50  75  75 100 125  0 ))	; 1 per 25%
  (T t* (  0  60  40  40  60  80 100 120  0 ))	; 1 per 20%
  ; considerable advantage for mana production on 'ice' and other
  ; difficult terrain.
  ; ice is usually highest mountain.
  ; note, building a tower on ice is a pain, as a wiz must do it!
  ; swamp/plains/forest/desert/hills/mountains = land
  (i land (  50 100  90 20  25  50 ))	; one arg must be a scalar...
  (b land (  50 100  90 20  25  50 ))
  (k land (  00 100  90 20  25  50 ))
  (P land (  50 100 100 50 100  75 ))
  (R land ( 100 100 100 50 100 100 ))
  (H land (  75 100 100 50  75  75 ))
  (s land (  50 100  90 70  75  50 ))
  (_ land (  50 100  90 70  75  50 ))
  (t land (  50 100  90 70  75  50 ))
  (V land (  50 100  90 25  75  50 ))
  (town land (  50 100  90 25  75  50 ))
  (city land (  50 100  90 25  75  50 ))
  (ships sea 100)
  (cloudkeep t* 100)
  )

(table base-consumption
  (movers food 1)	; all consume 1 food per turn, except...
  (( i k ) food 2)	; many infantry per unit, knights are pigs
  (dragon food 2)	; dragons are voracious!
  (undead food 0)	; just skeletons, not ghouls!
  (spells food 0)
  (C food 0)
  ((cavalry wagon) food 2)	; horses eat a lot
  (g food 1)		; crew needs to eat more when active
  (spells mana 1)	; spells burn mana in-flight
  (C mana 1)		; magic carpet burns mana in flight
  )

(table consumption-as-occupant
  (u* m* 1)
  (spells m* 0)	; spells in storage require no mana support
  (ships m* 0)	; ships not at sea need no food...
  (magic-items m* 0)	; they dont consume anyway!
  )

(table out-length
  (movers m* 0)		; most movables can't supply other things
  (bases m* 1)		; bases somewhat supply neighbors
  (cities m* 2)		; cities trade with surrounding lands
  (wizard mana 0)	; mages selfishly hoard magic to themselves
  (wagon food 2)	; wagons are supply vehicles
  (serfs m* 1)		; serfs will trade supplies to units near them
  (cloudkeep mana 3)	; flying citadel magically transmits its mana
  (magic-items m* 0)
  (a mana 2)
  )

(table in-length
  (movers m* 1)		; but they can be supplied by nearby things.
  (bases m* 1)		; bases can receive things from neighbors.
  (cities m* 2)		; cities trade more widely
  (spells mana -1)	; you have to charge the spells yourself
  (serfs m* 1)
  (magic-items m* 0)
  (a mana -1)		; amulet doesn't take mana, just gives it!
  )

(table hp-per-starve
  ;; In general, lack of food is a problem.
  (u* food 1.00)
  (undead food 0)
  (spells food 0)
  (magic-items food 0)
  ;; Archer, barbarian can live off the land.
  ((longbowmen barbarian) food 75)
  ;; Holy dudes are used to fasting.
  ((paladin priest) food 50)
  ;; ... so are peasants.
  (serfs food 33)
  ;; When the mana is gone, so is the spell.
  (spells mana 1.00)
  )

;;; Random events.

(add magickers revolt-chance 5)	; sure, tell wizards and dragons orders! Good luck!
(add barbarian revolt-chance 5)	; those crazy barbarians
(add serfs revolt-chance 50)	; serfs don't enjoy being oppressed

(table surrender-chance
  (forts u* 50)
  (cities u* ( 100 60 30 ))
  (cloudkeep u* 0)
  )

;FIX 1500 bases siege	; most things could be sieged easily
;FIX 350 forts siege	; rare for a siege to take a fort...
;FIX ( 1500 3000 3000 ) cities siege	; medieval cities fell easily
;FIX 0 cloudkeep siege

;; Attrition. Mostly ships at sea.

(table attrition 
  (ships sea ( 50  200 ))
  (foot ( mountains  swamp ) 100)
  (hvyfoot ( mountains swamp ) 200)
  (hoof ( mountains swamp ) 300)
;  (magickers t* 50)
  )

;FIX "gets caught in a storm" ships attrition-message
;FIX "had a dangerous Arcane accident" magickers attrition-message

(table accident-hit-chance
  ;; ships aren't too seaworthy...
  (ships sea (10 50))
  ;; heavy-foot units should have accidents in swamp and/or mountains.
  (foot (mountains swamp) 50)
  (hoof (mountains swamp) 100)
  ;; Magickers should have 'accidents'.
  (magickers t* 5)
  )

(table accident-damage
  (ships sea (1 2))
  (foot (mountains swamp) 1)
  (hoof (mountains swamp) 1)
  (magickers t* 1)
  )

;FIX "fell down and couldn't get up!" foot accident-message
;FIX "broke down and had to be destroyed." hoof accident-message

;;; Setup.

;                            sea swp des pln for hil mtn ice voi
;                             .   =   -   +   %   ~   ^   _   :
(add t* alt-percentile-min (   0  29  70  70  70  75  93  99   0 ))
(add t* alt-percentile-max (  69  71  93  93  93  98  99 100   0 ))

(add t* wet-percentile-min (   0  50   0  20  80   0   0   0   0 ))
(add t* wet-percentile-max ( 100 100  20  80 100  99 100 100   0 ))

(set edge-terrain void)

(set country-radius-min 4)
(set country-separation-min 17)
(set country-separation-max 90)

(add cities start-with ( 1 2 1 ))
(add (castle serfs temple wizard) start-with ( 1 4 1 1 ))

(table independent-density 
  (village land 60)
  (town land 30)
  (city land 15)
  ;; some rare freaks to find
  (cloudkeep t* 1)
  (serfs plains 50)
  (wizard land 2)
  (flying-carpet land 5)
  (tower land 10)
  (temple land 20)
  (dragon (plains mountains) 5)
  ;; Some random ones to be lying around easy to capture.
  (magic-items t* 5)
  (magic-items water 0)
  )

(table favored-terrain 
  (u* sea 0)
  (u* land 30)
  (forts land 100)
  ;; Flying citadels could be anywhere!
  (cloudkeep t* 100)
  (cloudkeep void 0)
  (magickers land 30)
  (magickers hills 60)
  (magickers mountains 100)
  (serfs land ( 20 100 80 10 40 20 ))
  (hoof mountains 0)	; just in case...
  (cities land 20)
  (cities plains 100)
  (magic-items water 0)	; they'd sink!
  ;; Nothing goes in the void.
  (u* void 0)
  )

(add u* already-seen 100)

;; Don't let rare magical stuff be visible.

(add flying-carpet already-seen 0)
(add cloudkeep already-seen 0)
(add magic-items already-seen 0)

(add u* initial-seen-radius 5)

(include "ng-weird")

(add (wizard dragon) namer "short-generic-names")
(add cities namer "short-generic-names")

(doctrine magicker-doctrine
  (construction-run (spells 1))
  )

(side-defaults
  (doctrines (magickers magicker-doctrine))
  )

;;; Documentation.

(game-module (instructions (
  )))

(game-module (notes (
  )))

(game-module (design-notes (
  )))
