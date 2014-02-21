(game-module "russian-rev"
  (title "Russian Revolution")
  (version "7.3+")
  (blurb "Base module for the Russian Revolution games by Massimo Campostrini.")
  (variants ; copied from standard.g
    (world-seen false)
    (see-all false)
    (sequential false)
    (world-size (60 30 360))
    ("Mostly Land" mostly-land
      (true
        (add sea alt-percentile-max 20)
        (add shallows alt-percentile-min 20)
        (add shallows alt-percentile-max 21)
        (add swamp alt-percentile-min 21)
        (add swamp alt-percentile-max 23)
        (add (desert plains forest) alt-percentile-min 21)
        ))
    ("Mostly Sea" mostly-sea
      (true
        (add sea alt-percentile-max 68)
        (add shallows alt-percentile-min 68)
        (add shallows alt-percentile-max 69)
        (add swamp alt-percentile-min 69)
        (add swamp alt-percentile-max 71)
        (add (desert plains forest) alt-percentile-min 70)
        ))
    ("All Land" all-land
      (true
        (add sea alt-percentile-max 1)
        (add shallows alt-percentile-min 1)
        (add shallows alt-percentile-max 2)
        (add swamp alt-percentile-min 2)
        (add swamp alt-percentile-max 4)
        (add (desert plains forest) alt-percentile-min 2)
        ))
    ("Large Countries" large
     (true (set country-radius-max 100))
     )
    ("Alternate economy" model1
     "Use a different algorithm for moving materials among units."
     (true (set backdrop-model 1)))
    ("AltEcon uses doctrine" model1doctrine
     "When using alternate economy, don't drain units below doctrine levels."
     (true (set backdrop-ignore-doctrine 0))
     (false (set backdrop-ignore-doctrine 1)))
    )
)

(unit-type militia (image-name "fremen")
  (help "cheap, weaker than infantry, good for patrol and exploration"))
(unit-type infantry (image-name "soldiers")
  (help "the backbone of the army, captures cities, makes forts"))
(unit-type cavalry
  (help "fast, explores and fights, good for exploration and reserve"))
(unit-type police
  (help "spies, fights terrorists, captures neutral troops"))
(unit-type terrorist
  (help "sneaks around, infiltrates, blows up things"))

(unit-type train (char "T")
  (help "moves troops on land, very vulnerable"))
(unit-type fleet (image-name "ca") (char "n")
  (help "fights and carries troops on sea"))

(unit-type fort (image-name "walltown") (char "/")
  (help "fort, port and canal"))
(unit-type town (image-name "town20")  (char "*")
  (help "smaller than a city"))
 (unit-type city (image-name "city19") (char "@")
  (help "main producer"))

(define m militia)
(define i infantry)
(define c cavalry)
(define p police)
(define t terrorist)
(define T train)
(define n fleet)
(define F fort)
(define ~ town)
(define @ city)

(material-type fuel
  (help "to move your trains and ships"))
(material-type food
  (help "to survive"))
(material-type ammo
  (help "to fight"))

(include "stdterr")

;; to use "stdterr" as it stands:
(add road name "railway")
(add road image-name "railroad")
(define railway road)

;; The balance between sea and land is a delicate one: Too much land
;; and fleets become useless; too much sea and railroads become useless.
(add cell-t* alt-percentile-min (  0  50  51  52  52  52  93  99))
(add cell-t* alt-percentile-max ( 50  51  53  93  93  93  99 100))
;; Increase the number of railroads (stdterr by itself will give us
;; only a bare minimum).
(table road-chance
  (city (town city) (80 100))
  (town (town city) ( 2   5))
  )
;; Probably road-to-edge-chance should be a probfraction (e.g. 200 =
;; two roads from each town to some edge), but for now it is just a
;; probability.
(add (town city) road-to-edge-chance 100)
(set edge-road-density 10000)
;; Nearly all towns should be connected by road to
;; somewhere else.
(add town spur-chance 90)
(add town spur-range 2)
(table road-into-chance add
  ;; We would like long rail lines which extend to most land areas.
  (plains (desert forest mountains) (80 70 60)))
;; Perhaps a better way to make railroads relevant is to use a larger
;; world (see for example feb-1917.g).

(define cities (~ @))
(define places (F ~ @))
(define troops (m i c))
(define ground (m i c p t))
(define movers (m i c p t T n))
(define water (sea shallows))
(define land (swamp plains forest desert mountains))
(define dry-land (desert plains forest mountains)) ; same as land-t*

;;; Static relationships.

(table vanishes-on
  (ground water true)
  (places (sea shallows swamp) true)
  (n land true)
  (n railway true)
  (u* ice true)
  (T t* true)
  (T railway false)
  )

;; Unit-unit capacities.

(table unit-size-as-occupant
  (u* u* 500)
;             m i c p t T  n
  (movers u* (1 2 2 1 1 6 15))
  )

(add (T n F ~ @) capacity (4 10 20 100 200))

(table occupant-max
  (u* u* 100)
  )

(table road-chance
  (town (town city) ( 1  5))
  (city (town city) (10 90))
  )

;;; Unit-terrain capacities.

;; Limit units from 4 to 16 in one cell.
;;  Places cover the entire cell, however.
  
(table unit-size-in-terrain
  (m t* 2)
  ((i c n T) t* 4)
  ((p t) t* 1)
  (places t* 16)
  )

(add t* capacity 16)
(add railway capacity 4)

(table terrain-capacity-x (T railway 2))

;;; Unit-material capacities.

(table unit-storage-x
;            m  i  c p t  T   n   F   ~   @
  (u* food ( 5  8 15 8 4 50 200 200 400 800))
  (u* fuel ( 0  0  0 0 0 50 200 200 400 800))
  (u* ammo ( 5 10 10 3 1 25 100 100 200 400))
)

;; A game's starting units will be full by default.
(table unit-initial-supply (u* m* 9999))

;;; Vision.

(add places already-seen 100)

(add cities see-always true)

(table visibility
  (u* t* 100)
  (t t* 10)
  (t (railway river) 100)
  (t (mountains forest) 1)
)
;                    m i c p t T n F ~  @
(add u* stack-order (3 4 5 2 1 6 7 8 9 10))

;;; Actions.

;                      m  i  c  p  t  T  n  F  ~  @
(add u* acp-per-turn ( 2  2  4  2  2  6  4  1  1  1))

(add movers acp-min  (-1 -1 -2 -1 -1 -3 -2))

(add F acp-damage-effect ((1 0) (2 1) (5 1)))
(add ~ acp-damage-effect ((3 0) (4 1) (15 1)))
(add @ acp-damage-effect ((5 0) (6 1) (25 1)))

;;; Movement.

(add places speed 0)
(add movers speed 100)

;(table mp-to-enter-unit
;   (u* u* 1)
;   (T n 6)
;)
; the above don't seem to work properly...
(table mp-to-enter-unit
   (u* u* 0)
)

(table mp-to-enter-terrain 
  (u* t* 99)
  (ground (plains desert) 1)
  (ground (forest mountains) 2)
  (n water (1 2))
  (c (forest mountains) (3 4))
  ((m p t) swamp 2)
  (movers railway 0)
;                m i c p t T n 
  (movers river (2 2 4 2 2 6 1))
;                          ^
; should be 99, but then trains can't cross rivers even on bridges
  )

(table mp-to-traverse
  (movers river 99)
  (n river 2)
  (movers railway 1)
  ((c n) railway (2 99))
  )

(table material-to-move ((n T) fuel 1))

(table consumption-per-move ((n T) fuel 1))

;;; Construction.

;           m  i  c  p  t  T  n  F ~ @
(add u* cp (8 12 20 20 16 16 34 12 1 1))

(table can-create
  (troops F (2 2 4))
  (cities movers 1)
  )

(table acp-to-create
  (troops F (2 2 4))
  (cities movers 1)
  )

(table cp-on-creation
  (troops F 1)
  (~ movers 1)
  (@ movers 2)
  )

(table can-build
  (troops F (2 2 4))
  (cities movers 1)
  )

(table acp-to-build
  (troops F (2 2 4))
  (cities movers 1)
  )

(table cp-per-build
  (troops F (1 2 1))
  (~ movers 1)
  (@ movers 2)
  )

(table occupant-can-construct
  (u* u* false)
  )

;(define constr (i c p t  T  n))
;(table tp-to-build
;     (~ constr (8 8 6 6 20 20))
;     (@ constr (4 4 3 3 10 10))
;)
;(table acp-to-toolup (cities constr 1))
;(table tp-per-toolup (cities constr 1))
;(table tp-max        (cities constr 100))
; the above make construction of i c p t T n impossible

;;; Repair.

;; Automatic repair work.
(add (  m    i    c    n    F    ~    @ ) hp-recovery 
     (0.25 0.50 0.25 0.50 0.25 0.50 1.00)
)

;;; Explicit repair actions accelerate the recovery of lost hp.

(table can-repair
  (places u* true)
  (n movers true)
  ((m i) places true)
  )

(table acp-to-repair
  (places u* 1)
  (n movers 1)
  ((m i) places 1)
  )

(table hp-per-repair
  (F u* 0.50)
  (cities u* 1.00)
  (n ground 0.50)
  (n n 0.50)
  (i places 1.00)
  (m places 0.50)
  )

;;; Production.

(table base-production
  (ground food 1)
  (c food 2)
  ; until troop to get food from terrain
  (places fuel (10 20 40))
  (places food (10 20 40))
  (places ammo ( 5 10 20))
  )

(table productivity
  (u* t* 0)
  (ground (plains forest) 100)
  (c forest 50)
  (F dry-land (100 50 30 30))
  (~ dry-land (100 50 30 30))
  (@ dry-land (100 50 30 30))
  )

; the above does not work :-)

; (table terrain-storage-x ((forest plains) food (3 12)))
; (table terrain-initial-supply ((forest plains) food (3 12)))
; (table terrain-base-production ((forest plains) food (1 3)))
; but then no way for troop to get food from terrain

(table base-consumption
  (movers food 1)
  ((c T) food (2 0))
  )

(table hp-per-starve
  (movers food 0.5)
  )

;;; Combat.

;                        m i c p t T n F  ~  @
(add u* hp-max          (2 3 2 2 1 1 8 5 10 25))

(add p speed-damage-effect ((1 50) (2 100)))
(add n speed-damage-effect ((1 50) (2 50) (3 100) (8 100)))

(table acp-to-attack
  (m u* 2)
  (i u* 2)
  (c u* 4)
  (p u* 2)
  (t u* 1)
  (n u* 1)
  )

(table hit-chance
  ;;      m  i  c  p  t  T  n  F  ~  @
  (m u* (70 50 40 50 50 50 10 30 25 15))
  (i u* (80 70 50 80 75 70 20 50 40 30))
  (c u* (80 60 80 80 80 60 00 30 20 10))
  (p u* (40 30 30 50 75 40 00 05 20 10))
  (t u* (40 30 10 10 75 40 20 40 60 50))
  (T u* 0)
  (n u* (60 50 30 60 30 70 60 60 70 80))
  (F u* (60 50 60 60 10 30 10 00 00 00))
  (~ u* (40 35 40 50 05 40 20 00 00 00))
  (@ u* (60 50 60 70 05 50 30 00 00 00))
  )

(table damage
  (u* u* 1)
  ;;     m i c p t T n F ~ @
  (t u* (2 3 2 1 1 1 2 5 5 9))
  (n u* (2 2 2 2 1 1 2 2 3 4))
  )

(table capture-chance
  ((m i c) T (30 50 40))
  (m places (10 20 15))
  (i places (20 40 30))
  (c places ( 5 20 15))
  )

(table independent-capture-chance
  (p movers 50) 
  (p (p t) 0)
  ((m i c) T (60 90 70))
  (m places (20 40 30))
  (i places (40 75 60))
  (c places (10 40 30))
  (i n 30)
  )

(table ferry-on-entry
  (u* u* over-own)
  (n movers over-border)
  )

(table ferry-on-departure
  (u* u* over-own)
  (n movers over-border)
  )

(table bridge
  (i places true)
  (p movers true)
  )

(table protection 
  (F movers 80)
  (~ movers 65)
  (@ movers 50)
  (i places (70 80 80))
  ((m c) places 90)
  )

(table consumption-per-attack (u* ammo 1))

(table hit-by (u* ammo 1))

;;; Revolt & surrender.

(table surrender-range (u* u* 1))

(table defend-terrain-effect
  ((m i) mountains (80 90))
  ((m i) forest (90 95))
  (F mountains 80)
  (n shallows 150)
  )

; this is a civil war:  everybody revolts and turns coat
(add u* acp-to-change-side 1)
; excluding politically-aware units
(add (p t) acp-to-change-side 0)

(add places revolt-chance 0.50)
(add movers revolt-chance 0.15)
(add (p t) revolt-chance 0.00)
(add m revolt-chance 0.30)

(table surrender-chance
  (m u* 1.00)
  (u* u* 0.50)
  ((i c @) u* 0.20)
  (F u* 0.10)
  (n u* 0.05)
  ((p t) u* 0.00)
  (t p 0.10)
  (u* (t F) 0)
  )

(add p possible-sides (not "independent"))
(add t possible-sides (not "independent"))

(table can-enter-independent (t (T ~ @) true))
; should be able to enter even hostile units!

;;; Spying.

(add p spy-chance 25.00)
(add p spy-range 2)

;;; Accidents.

(table attrition
  (n shallows 4.00)
  (troops swamp 5.00)
  (c (mountains forest desert) (2.00 1.00 1.00))
  )

;;; Scoring.

(add u* point-value 0)
(add (i c F ~ @) point-value (1 1 1 10 30))

;;; Initial setup.

(table favored-terrain
  (n water (100 20))
  (ground plains 100)
  (ground forest 20)
  (t (plains forest) (50 100))
  ;;            ~   +  %  ^
  (F dry-land (20 100 10 20))
  (@ dry-land (25 100 10 10))
  (~ dry-land (40 100 20 20))
  )

(table independent-density add (places t* (10 100 30)))

(add @ start-with 1)
(add ~ independent-near-start 3)

(set country-separation-min 12)
(set country-separation-max 60)
(set country-radius-min 4)

;; Naming.

(include "ng-european")
(include "town-names")
(set default-namer "random-town-names")
(add places namer "russian-place-names")

;; Sides.

(set sides-min 2)
(scorekeeper (do last-side-wins)) ; for now

(set side-library '(
  ; ------------ parties
  ((noun "Bolshevik") (color "red") (emblem-name "hammer-and-sickle"))
  ((noun "Red") (color "red") (emblem-name "soviet-star"))
  ((noun "Menshevik") (color "black,pink"))
  ((noun "Tzarist") (color "blue") (emblem-name "arms-russia"))
  ((noun "White") (color "white,black") (emblem-name "arms-russia"))
  ; ------------ peoples
  ((noun "Russian") (color "blue,red")
                    (emblem-name "flag-russia"))
  ((noun "Ukrainian") (color "yellow,blue")
                      (emblem-name "flag-ukraine"))
  ((noun "Georgian") (emblem-name "flag-georgia"))
  ((noun "Azer") (emblem-name "flag-azerbaijan"))
  ((noun "Armenian") (emblem-name "flag-armenia"))
  ((noun "Cossack") (color "white,blue"))
  ; ------------ foreign powers
  ((noun "Polish") (color "white,red") (emblem-name "flag-poland"))
  ((noun "Czechoslovak") (color "red,blue")
                         (emblem-name "flag-czech"))
  ((noun "German") (color "gray,black,white")
                   (emblem-name "german-cross"))
  ((noun "Turkish") (emblem-name "crescent"))
  ((noun "Japanese") (color "red,white,black")
                     (emblem-name "flag-japan"))
; ((noun "Chinese") (color "red,yellow") (emblem-name "flag-dragon")) 
))

; 12 Apr 2001: Massimo Campostrini writes:
;   I am not able to work on xconq any longer (not even to play xconq),
;   but I still lurk on the mailing list.
;   . . .
;   If anyone feels as taking over any other xconq game I left orphan,
;   please do so.

(game-module (design-notes (
  "Massimo Campostrini <Massimo.Campostrini@df.unipi.it>."
  ""
  "Tooling up does not work, so it is commented out."
  ""
  "Troops seem to be unable to collect food from terrain,"
  "so terrain-storage-x and friends are commented out"
  "and troops make their own food."
  ""
  "Police is supposed to be able to capture neutral troops;"
  "need to implement this in Xconq 7.0"
  )))

(game-module (notes (
  "This games tries to model Russia at the time of the Revolution."
  ""
  "About 60% of your production should be infantry"
  ""
  "Trains can move your troops around fast,"
  "but they should never get close to the front."
  ""
  "Terrorists should infiltrate enemy territory"
  "and blow up trains and other things."
  "Police should try to avoid this."
  ""
  "A fort can be built to block a railway."
  )))
