(game-module "greek"
  (title "Ancient Greece")
  (version "1.0")
  (blurb "Classical Greece from 500 to 350 BC.")
  (variants
   (world-seen true)
   (see-all false)
   (sequential false)
   (world-size (60 30 2500))
   ("Last Side Wins" last-side-wins true
     (true (scorekeeper (do last-side-wins))))
   )
  )

(unit-type peltast
  (help "light fast infantry")(image-name "ancient-spearman"))
(unit-type hoplite
  (help "heavy destructive infantry")(image-name "ancient-swordman"))
(unit-type archer
  (help "archers and slingers")(image-name "ancient-archer"))
(unit-type cavalry
  (help "more like light cavalry")(image-name "ancient-cavalry"))
(unit-type trireme (char "T")
  (help "three-decked ship - mainstay of the navies") (image-name "ancient-trireme"))
(unit-type siege-engine (name "siege engine") (image-name "ancient-ballista") (char "S")
  (help "bashes cities (slowly)"))
(unit-type fortifications (image-name "walltown")
  (help "augments a polis' defense"))
(unit-type polis (image-name "ancient-city") (char "*")
  (help "typical city-state"))
(unit-type metropolis (image-name "parthenon") (char "@")
  (help "a large and powerful city"))

(define p peltast)
(define h hoplite)
(define a archer)
(define c cavalry)
(define T trireme)
(define S siege-engine)
(define F fortifications)
(define ~ polis)
(define @ metropolis)

(material-type food
  (help "everybody needs food to survive"))
(material-type talents
  (help "the unit of big money, funds ships and armies"))

(terrain-type sea (char "."))
(terrain-type plains (char "+"))
(terrain-type forest (char "%"))
(terrain-type desert (char "~"))
(terrain-type mountains (char "^"))
(terrain-type river (char "-")
  (subtype border))

(define cities (~ @))
(define places (F ~ @))
(define ship-u* (T))
(define land-u* (p h a c S))
(define movers (p h a c T S))

(define water (sea river))
(define land (plains forest desert mountains))

(add water liquid true)

;;; Static relationships.

(set self-required true)			;	Require self units.
(add metropolis can-be-self true)	;	Only the metropolis can be self unit.
(add metropolis self-resurrects true)	;	Let the government relocate if necessary.

(table vanishes-on
  (land-u* water true)
  (places water true)
  (ship-u* land true)
  )

;; Allow only one city per cell, but several units otherwise.

(add t* capacity 16)

(table unit-size-in-terrain
  (cities t* 9)
  )

;; Cities and ships have relatively limited capacity.

(add u* capacity 0)

(add cities capacity 8)

(add trireme capacity 4)

(table unit-size-as-occupant
  (u* u* 99)
  ((p h a c) trireme (2 2 1 4))
  (movers cities 1)
  )

;;; Unit-material capacities.

(table unit-storage-x
  (u* food (2 2 2 2 100 0 50 50 200))
  (u* talents (10 80 10 100 240 0 500 1000 4000))
  )

;;; Vision.

;; A month's time is sufficient for news about cities
;; to get around everywhere.

(add cities see-always true)

;;; Actions.

;; A turn is a whole month, so lots can happen.

(add u* acp-per-turn (16 12 16 32 24 4 0 1 1))

;;; Movement.

(add places speed 0)

(table mp-to-enter-terrain
  (land-u* water 99)
  (land-u* mountains 2)
  (ship-u* land 99)
  )

;;; Construction.

;;          p  h a  c T S F ~ @
(add u* cp (6 12 6 12 6 6 1 1 1))

(table acp-to-create
  ((~ @) movers 1)
  )

(table acp-to-build
  ((~ @) movers 1)
  )

(table consumption-per-built
  (movers talents (5 20 5 20 24 30))
  )

;; Production.

(table base-production
  ((p h a c) food 1)
  (cities food (5 20))
  (cities talents (2 20))
  )

(table productivity
  (movers (desert mountains) 0)
  )

(table base-consumption
  ((p h a c) food 1)
  (cities food (5 20))
  ((p h a c T) talents (1 8 1 10 24))
  )

(table hp-per-starve
  ((p h) food 10.00)
  ((a c) food 1.00)
  (cities food 5.00)
  ((p h a c T) talents (1.00 10.00 1.00 10.00 6.00))
  )

;;; Combat.

;;               p  h  a  c  T  S  F  ~  @
(add u* hp-max (20 20  4  4 12  2 10 20 40))

(table acp-to-attack
  (u* u* 8)
  (S  u* 4)
  )

(table hit-chance
  ;;      p  h  a  c  T  S  F  ~  @
  (p u* (40 20 50 40 10 40 10  0  0))
  (h u* (60 40 50 50 70 60 70 20 10))
  (a u* (50 20 30 30 60 50  0  0  0))
  (c u* (60 30 50 50 50 60 50  0  0))
  (T u* ( 0  0  0  0 50  0  0  0  0))
  (S u* ( 0  0  0  0  0  5 99 99 99))
  (F u* ( 0  0  0  0 20  0  0  0  0))
  (~ u* ( 0 40  0  0  0 10  0  0  0))
  (@ u* ( 0 80  0  0  0 20  0  0  0))
  )

(table damage
  (u* u* 1)
  (h u* 2)
  (S cities 1d3+3)
  )

(table capture-chance
  ;; Hoplites can capture many things.
  (h (T S F ~ @) (20 50 30 10 5))
  ;; Sea-fights usually end with sinkings, but sometimes captures.
  (T T 20)
  )

;;; Backdrop.

(table out-length
  ;; Mobile units will share with their nearby friends.
  (movers food 1)
  ;; Places have a basic distribution system in effect.
  (places food 3)
  ;; Money could be shipped long distances easily.
  (u* talents 10)
  )

(table in-length
  (u* food 3)
  (u* talents 10)
  )

(table attrition
  ((p h) t* 50) 
  ((a c) t* 10)
  )

(table accident-vanish-chance
  ;; Ships disappeared all the time back then.
  (T sea 4.00)
  )

;; The allegiances of cities weren't what they could be...

(add cities revolt-chance 10)

;; The mere presence of a hoplite army might cause the city
;; to change sides.

(table surrender-chance
  (cities h (10 5))
  (cities F (10 5))
  )

(table surrender-range
  (cities h 1)
  (cities F 1)
  )

;;; Text.

(set action-notices '(
  ((destroy u* (p h a c)) (actor " defeats " actee "!"))
  ((destroy u* T) (actor " sinks " actee "!"))
  ((destroy u* cities) (actor " levels " actee "!"))
  ))

(set event-notices '(
  ((unit-starved hoplite) (0 " is unpaid and goes home."))
  ((unit-starved trireme) (0 " is unpaid and goes home."))
  ((unit-vanished ship) (0 " is lost in a storm"))
  ))

(set event-narratives '(
  ((unit-starved hoplite) (0 " was not paid and went home"))
  ((unit-starved hoplite) (0 " was not paid and went home"))
  ((unit-vanished ship) (0 " was lost in a storm"))
  ))

;;; Random generation.

(set alt-blob-density 10000)
(set alt-blob-size 40)
(set alt-smoothing 1)
(set wet-blob-density 2000)
(set wet-blob-size 100)

(add t* alt-percentile-min (  0  70  70  70  90 0))
(add t* alt-percentile-max ( 70  90  90  90 100 0))
(add t* wet-percentile-min (  0  20  80   0   0 0))
(add t* wet-percentile-max (100  80 100  20 100 0))

(set edge-terrain sea) ; the river Oceanus

(add @ start-with 1)
(add h start-with 1)
(add T start-with 1)
(add ~ independent-near-start 3)
;; Try to get countries on the coast.
(add (sea plains) country-terrain-min (1 1))

(table independent-density
  (~ (plains forest desert) (1000 500 250)))

(table favored-terrain
  (u* t* 100)
  (u* sea 0)
  (ship-u* sea 100)
  )

(table unit-initial-supply
  (u* m* 9999)
  ;; Cities have food, though never as much as they would like.
  (cities food (30 120))
  ;; All cities have some money.
  (cities talents (600 2400))
  )

(set calendar '(usual month))

(world 2500 (year-length 12))  ; big world, can't circumnavigate.

;;; An assortment of city-states.  The famous ones are weighted more heavily.

(set side-library '(
  (10 (name "Athens") (adjective "Athenian")(unit-namers (metropolis "athens")))
  (10 (name "Sparta") (adjective "Spartan")(unit-namers (metropolis "sparta")))
  (5 (name "Corinth") (adjective "Corinthian")(unit-namers (metropolis "corinth")))
  (5 (name "Thebes") (adjective "Theban")(unit-namers (metropolis "thebes")))
  (3 (name "Argos") (adjective "Argive")(unit-namers (metropolis "argos")))
  (2 (name "Megara") (adjective "Megaran")(unit-namers (metropolis "megara")))
  (2 (name "Miletos") (adjective "Miletan")(unit-namers (metropolis "miletos")))
  (2 (name "Messene") (adjective "Messenian")(unit-namers (metropolis "messene")))
  (2 (name "Syracuse") (adjective "Syracusan")(unit-namers (metropolis "syracuse")))
  (2 (name "Ephesos") (adjective "Ephesian")(unit-namers (metropolis "ephesos")))
  (2 (name "Delos") (adjective "Delian")(unit-namers (metropolis "delos")))
  ((name "Lemnos") (adjective "Lemnian")(unit-namers (metropolis "lemnos")))
  ((name "Ambracia") (adjective "Ambraciot")(unit-namers (metropolis "ambracia")))
  ((name "Phokia") (adjective "Phokian")(unit-namers (metropolis "phokia")))
  ((name "Chios") (adjective "Chian")(unit-namers (metropolis "chios")))
  ((name "Gelos") (adjective "Geloan")(unit-namers (metropolis "gelos")))
  ((name "Caria") (adjective "Carian")(unit-namers (metropolis "caria")))
  ((name "Lokria") (adjective "Lokrian")(unit-namers (metropolis "lokria")))
  ((name "Melos") (adjective "Melian")(unit-namers (metropolis "melos")))
  ((name "Phlias") (adjective "Phliasian")(unit-namers (metropolis "phlias")))
  ((name "Samos") (adjective "Samian")(unit-namers (metropolis "samos")))
  ((name "Thuria") (adjective "Thurian")(unit-namers (metropolis "thuria")))
  ((name "Tegea") (adjective "Tegean")(unit-namers (metropolis "tegea")))
  ((name "Dolope") (adjective "Dolopian")(unit-namers (metropolis "dolope")))
  ((name "Olynthia") (adjective "Olynthian")(unit-namers (metropolis "olynthia")))
  ((name "Elis") (adjective "Elean")(unit-namers (metropolis "elis")))
  ((name "Lucania") (adjective "Lucanian")(unit-namers (metropolis "lucania")))
  ((name "Kythera") (adjective "Kytheran")(unit-namers (metropolis "kythera")))
  ((name "Skios") (adjective "Skionian")(unit-namers (metropolis "skios")))
  ))

;	This is a convoluted way of ensuring that each sides metropolis is named like the side.

(namer athens (random "Athens"))
(namer sparta (random "Sparta"))
(namer corinth (random "Corinth"))
(namer thebes (random "Thebes"))
(namer argos (random "Argos"))
(namer megara (random "Megara"))
(namer miletos (random "Miletos"))
(namer messene (random "Messene"))
(namer syracuse (random "Syracuse"))
(namer ephesos (random "Ephesos"))
(namer delos (random "Delos"))
(namer lemnos (random "Lemnos"))
(namer ambracia (random "Ambracia"))
(namer phokia (random "Phokia"))
(namer chios (random "Chios"))
(namer gelos (random "Gelos"))
(namer caria (random "Caria"))
(namer lokria (random "Lokria"))
(namer melos (random "Melos"))
(namer phlias (random "Phlias"))
(namer samos (random "Samos"))
(namer thuria (random "Thuria"))
(namer tegea (random "Tegea"))
(namer dolope (random "Dolope"))
(namer olynthia (random "Olynthia"))
(namer elis (random "Elis"))
(namer lucania (random "Lucania"))
(namer kythera (random "Kythera"))
(namer skios (random "Skios"))

(namer greek-names (random
  "Mykene" "Magnesia" "Skepsis" "Phokaia" "Pergamon" "Methymna" "Ilion" "Mende" 
  "Halikarnassos" "Hermione" "Dodona" "Same" "Aigion" "Pherai" "Orchomenos" "Delphi" "Abydos" 
  "Ambrakia" "Amyklai" "Chalkis" "Stymphalos" "Heraia" "Anaktorion" "Kalydon" "Karystos" 
  "Knidos" "Knossos" "Thespiai" "Hypata" "Kydonia" "Trikka" "Larisa""Leukas" "Lyktos" 
  "Thebai" "Mytilene" "Naxos" "Oreos" "Patrai" "Phaistos" "Praisos" "Pylos" "Opus" 
  "Sikyon" "Smyrna" "Stratos" "Sybrita""Eretria" "Tiryns" "Amphissa" "Phalanna"
  "Aigai" "Kierion" "Elateia" "Pagasai" "Larymna" "Eleon" "Phyllas" "Kanope" "Echinos"
  "Oropos" "Thestia" "Trichonion" "Alope" "Alyzia" "Methana" "Asine" "Porthmos"))

(namer greek-island-names (random
  "Naxos" "Kalymnos" "Ikaria" "Siphnos" "Kos" "Ithaka" "Thera" "Telos" "Lesbos" 
  "Thasos" "Andros" "Euboia" "Skyros" "Rhodos" "Leukas" "Kephallenia" "Zakynthos" 
  "Psyra" "Ikos" "Tenos" "Patmos" "Astypalaia" "Leros" "Anaphe" "Mykonos"  "Hydrea" 
  "Imbros" "Oinussai" "Ephyra" "Halonessos" "Prokonessos" "Arkionessos""Samothrake" "Skiathos" 
  "Peparethos" "Keos" "Paros" "Seriphos" "Helena" "Halone" "Amorgos" "Nisyros" "Kalymnos" 
  "Leros" "Korsiai" "Ios" "Aigina""Lebinthos" "Salamis"))

(namer greek-mountain-names (random
  "Kyllene" "Pentelikon" "Parnes" "Dirphys" "Helikon" "Parnassos" "Othrys" "Ossa" "Olympos" 
  "Pieros" "Pindos" "Pelion" "Sipylos" "Mykale" "Messagis" "Pangaion" "Barnus" 
  "Oche" "Kithairon" "Oita" "Panaitolion" "Tomaros" "Bertiskon" "Boion" "Tymphe"  "Lynkos" 
  "Lakmos" "Arakynthos" "Telethrion" "Taygetos" "Parnon" "Aigaleon""Lykaion" "Mainalos" 
  "Aroania" "Erymanthos" "Nomia" "Panachaikon" "Korykos" "Ida" "Pindasos" "Ganos" "Athos"))

(add polis namer "greek-names")
(add metropolis namer "greek-names")

(set feature-types '((islands 1 500)(mountains 3)))

(set feature-namers
  '((islands greek-island-names)(mountains greek-mountain-names)))

(game-module (notes (
  "This time strictly covers about 500 to 350 B.C. Land warfare"
  "was most significant, with some notable sea-fights. Cities were nearly"
  "untakeable, so the action centered around sieges and field battles."
  ""
  "The numbers border on the plausible, but again this one has not been"
  "played enough to find the imbalances even, let alone decide on good"
  "strategies."
  )))

(add peltast notes (
  "Useful in skirmishes."
  ))
(add hoplite notes (
  "Hoplite (heavy infantry).  For the capture of cities by assault."
  "This unit represents about 500 men."
  ))
(add archer notes (
  "Similar to peltasts, but equipped with bows instead of spears."
  ))
(add cavalry notes (
  "Fast, but ineffective against cities."
  ))
(add trireme notes (
  "This is a squadron of about 24 ships."
  ))
(add siege-engine notes (
  "During the Classical period, siege engines were uncommon."
  "This is reflected in the difficulty of building these, and"
  "the difficulty of transporting them to the desired location."
  ))

;;; Should add variable loyalties.

(game-module (design-notes (
  "Time scale is one month."
  )))
