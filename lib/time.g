(game-module "time"
  (title "Time: Combat Through The Ages")
  (blurb "Ancient Greece, but not for long. Written by Henry Ware, with help from Victor Mascari and Massimo Campostrini.")
  (version "1.0")
  (instructions (
    "Each city type represents an era."
    "One of your cities should always build the more modern city,"
    "while your other cities build units for expansion and conquest."
    "When the new city is completed, all your other cities should upgrade."
  ))
  (variants
    (world-seen false)
    (see-all false)
    (sequential false)
    (world-size (90 60 360))
    ("Mostly Land" mostly-land
      (true
        ;; Adjust so that sea is 20% instead of 50% of the world.
        (add sea alt-percentile-max 20)
        (add shallows alt-percentile-min 20)
        (add shallows alt-percentile-max 21)
        (add swamp alt-percentile-min 21)
        (add swamp alt-percentile-max 23)
        (add (desert plains forest) alt-percentile-min 21)
        ))
    ("All Land" all-land
      (true
;; Adjust sea and shallows out of existence, let swamp take all the low spots.
        (add sea alt-percentile-min 0)
        (add sea alt-percentile-max 0)
        (add shallows alt-percentile-min 0)
        (add shallows alt-percentile-max 0)
        (add swamp alt-percentile-min 0)
        (add swamp alt-percentile-max 2)
        (add swamp wet-percentile-min 0)
        (add swamp wet-percentile-max 100)
        (add (desert plains forest) alt-percentile-min 2)
        ;; Counterproductive to try to set up near water.
		(add sea country-terrain-min 0)
        ))
    (slow-progress false
      (true
	(add (M N T @ & U F) tech-max 30)
	(add (M N T @ & U F) tech-to-build 30) 
	))
    ("Alternate economy" model1
     "Use a different algorithm for moving materials among units."
     (true (set backdrop-model 1)))
    ("AltEcon uses doctrine" model1doctrine
     "When using alternate economy, don't drain units below doctrine levels."
     (true (set backdrop-ignore-doctrine 0))
     (false (set backdrop-ignore-doctrine 1)))
    )
)

(imf "blimp" ((16 16)
  (mono "0000/0000/0000/0000/0000/03fc/8ffe/dfff/ffff/ffff/8ffe/01e0/0000/0000/0000/0000")
  (mask "0000/0000/0000/0000/07fe/dfff/ffff/ffff/ffff/ffff/ffff/dfff/03e0/0000/0000/0000")))
(imf "ogre-2" ((16 16)
  (mono "0000/0000/0000/0000/0000/1010/0e24/1ff9/2fbe/3eee/7fff/aaaa/f7df/7bf7/3dfe/0000")
  (mask "0000/0000/0000/0000/0000/1f7e/3fff/7fff/7fff/ffff/ffff/ffff/ffff/ffff/ffff/7fff")))
(imf "mech-2" ((16 16)
  (mono "0000/0000/0180/07e0/0180/1ff8/1a58/27e4/366c/33cc/23c4/0180/0240/0240/0c30/0c30")
  (mask "0000/03c0/07e0/0ff0/1ff8/3ffc/3ffc/7ffe/7ffe/7ffe/7ffe/07e0/07e0/0ff0/1e78/1e78")))

(unit-type x (name "phalanx") (image-name "hoplite") (char "x")
  (help "men with spears")
  (notes "fights, captures cities, explores"))
(unit-type g (name "galley") (image-name "bireme") (char "g")
  (help "carries ground units")
  (notes "carries phalanxes, knights, longbowmen"))
(unit-type k (name "knight") (image-name "swordman") (char "k")
  (help "men with swords and armour")
  (notes "good to capture cities"))
(unit-type l (name "longbowman") (image-name "archer") (char "l")
  (help "men with bows")
  (notes "good to fight ground units"))
(unit-type s (name "boat") (image-name "twodecker") (char "s")
  (help "carries more stuff than a galley")
  (notes "carries ground units except armor and battledroids"))
(unit-type y (name "cannon") (image-name "cannon") (char "y")
  (help "kills infantry"))
(unit-type c (name "cavalry") (image-name "cavalry") (char "c")
  (help "fast, kills cannon"))
(unit-type i (name "infantry") (image-name "soldiers") (char "i")
  (help "men with guns"))
(unit-type d (name "dirigible") (image-name "blimp") (char "d")
  (help "good for reconassiance"))
(unit-type B (name "battleship") (image-name "bb") (char "B")
  (help "hard to kill, good at killing"))
(unit-type t (name "transport") (image-name "ap") (char "t")
  (help "easy to kill- but carries alot")
  (notes "carries ground units except battledroids"))
(unit-type C (name "carrier") (image-name "cv") (char "C")
  (help "takes airplanes where they need to go"))
(unit-type f (name "fighter") (image-name "fighter") (char "f")
  (help "kills bombers & does light bombing itself"))
(unit-type a (name "armor") (image-name "tank") (char "a")
  (help "men in tanks"))
(unit-type h (name "helicopter") (image-name "helicopter") (char "h")
  (help "carries infantry, good ground support"))
(unit-type b (name "bomber") (image-name "stealth-bomber-2") (char "b")
  (help "long range aircraft with nuclear bombs"))
(unit-type z (name "flying saucer") (image-name "saucer") (char "z")
  (help "men in metallic clothing")
  (notes "carries battledroids"))
(unit-type q (name "battledroid") (image-name "mech-2") (char "q")
  (help "self-motivated weapon")
  (notes (
  "The battledroid is a heavily armed robot whose sophisticated programming"
  "allows it to kill all armed opponents without harming civilians."
  "Battledroids refuse to attack phalanxes, due to a software bug.")))
(unit-type X (name "TraXor") (image-name "ogre-2") (char "X")
  (help "the ultimate war machine")
  (notes (
  "Not much is known about the TraXor."
  "We are reasonably certain it emanates rainbow colored beams"
  "and can reduce vast areas to ruin.")))
(unit-type W (name "fort") (image-name "walltown") (char "W")
  (help "useful as a canal & is defensible"))
(unit-type _ (name "base") (image-name "airbase") (char "/")
  (help "a port and airstrip"))
(unit-type A (name "Polis") (image-name "parthenon") (char "A")
  (help "can build phalanxes, galleys or it can ungrade"))
(unit-type M (name "Castle") (image-name "castle") (char "M")
  (help "a step above ancients- hard to capture"))
(unit-type N (name "City") (image-name "town20") (char "N")
  (help "a Napoleonic Era city- makes units with guns"))
(unit-type T (name "Metropolis") (image-name "city18") (char "*")
  (help "a WWI era city.  Battleships are killer."))
(unit-type @ (name "Modern City") (image-name "city19") (char "@")
  (help "a WWII city- an air power era"))
(unit-type & (name "Post-Modern City") (image-name "city20") (char "&")
  (help "a WWIII city- Nuke 'em!"))
(unit-type U (name "Habitat") (image-name "town22") (char "U")
  (help "The future has arrived"))
(unit-type F (name "Flying City") (image-name "city30") (char "F")
  (help "Paradise on(?) Earth & a flying party"))

(material-type food (help "for thought?"))
(material-type fuel (help "energy for a better tomorrow"))
(material-type ammo (help "the general's munitions"))
(material-type bombs (help "nuke 'em"))

(include "stdterr")

; more land, more forest
;                                sea sha swa des pla for mou ice
(add cell-t* alt-percentile-min (  0  48  50  50  51  51  90  99))
(add cell-t* alt-percentile-max ( 48  50  51  90  90  90  99 100))
(add cell-t* wet-percentile-min (  0   0  20   0  20  70   0   0))
(add cell-t* wet-percentile-max (100 100 100  20  70 100 100 100))

(table terrain-damaged-type
  (plains desert 1)
  (forest desert 1)
  (ice mountains 1)
  )

(set edge-terrain ice)

;DEFINITIONS
; F not in movement definitions.  Cleaner this way.
; Everything is in either movers or places.

(define cities (A M N T @ & U F))	; just devlopers.
(define bases (W _))	; other places
(define places (W _ A M N T @ & U F))	; things that host lots.
(define makers (A M N T @ & U F))
(define ground (x k l c y i a q))
(define aircraft (d f h b z X))
(define pitstops (_ N T @ & U F))	; with gas
(define ship (g s B t C))
(define movers (x g k l s c y i d B t C f a h b z q X))
(define water (sea shallows))
(define land (plains forest desert mountains))

;**************************************************************
(define firstu A)	; <-- CHANGING THIS LINE TO A DIFFERENT CITY
;********************** WILL CHANGE THE STARTING CITY IN THE WHOLE PROGRAM
;			 Is that user-friendly or what?
;
;                       Well, you Tmight* want to change the period name too.


; (add u* point-value 1) ; better thus way?
(add cities point-value (5 20 80 320 1280 5120 20480 32000))

;STARTUP STUFF
(add firstu start-with 4)

(table independent-density (firstu plains 180))

(table favored-terrain 
  (u* t* 0)
  (cities plains 100)
  )

(table vanishes-on 
  (u* t* false)
  (places (sea shallows ice) true)
  (F t* false)
  (firstu (swamp desert mountains) true)
  )

(set country-separation-min 30)
(set country-separation-max 100)
(set country-radius-min 5)
; (set country-radius-max 10)
(add cities initial-seen-radius 2)

(add places already-seen 1)
(add places see-always 1)

;PRODUCTION and REPAIR

(add A cp 1)
(add (M x g) cp (5 4 12))
(add (N k l s) cp (5 4 4 12))
(add (T s c y i) cp (5 7 6 6 6))
(add (@ d B t) cp (5 8 12 10))
(add (& C f a) cp (5 12 5 6))
(add (U h b) cp (5 6 8))
(add (F z q) cp (5 8 6))
(add X cp 6)
(add (W _) cp (8 10))

(table can-create
  (A (M x g) 1)
  (M (N g k l s) 1)
  (N (T s c y i) 1)
  (T (@ c i d B t) 1)
  (@ (& i B t C f a) 1)
  (& (U i t C f a h b) 1)
  (U (F z q) 1)
  (F (z q X) 4)
  ((x k l) W 2)
  ((c y i a) _ (3 1 2 4))
  )

(table can-build
  (A (M x g) 1)
  (M (N g k l s) 1)
  (N (T s c y i) 1)
  (T (@ c i d B t) 1)
  (@ (& i B t C f a) 1)
  (& (U i t C f a h b) 1)
  (U (F z q) 1)
  (F (z q X) 4)
  ((x k l) W 2)
  ((c y i a) _ (3 1 2 4))
  )

(table acp-to-create 
  (A (M x g) 1)
  (M (N g k l s) 1)
  (N (T s c y i) 1)
  (T (@ c i d B t) 1)
  (@ (& i B t C f a) 1)
  (& (U i t C f a h b) 1)
  (U (F z q) 1)
  (F (z q X) 4)
  ((x k l) W 2)
  ((c y i a) _ (3 1 2 4))
  )

(table acp-to-build 
  (A (M x g) 1)
  (M (N g k l s) 1)
  (N (T s c y i) 1)
  (T (@ c i d B t) 1)
  (@ (& i B t C f a) 1)
  (& (U i t C f a h b) 1)
  (U (F z q) 1)
  (F (z q X) 4)
  ((x k l) W 2)
  ((c y i a) _ (3 1 2 4))
  )

(table cp-per-build
  (A (M x g) 1)
  (M (N g k l s) 1)
  (N (T s c y i) 1)
  (T (@ c i d B t) 1)
  (@ (& i B t C f a) 1)
  (& (U i t C f a h b) 1)
  (U (F z q) 1)
  (F (z q X) 1)
  ((x k l) W 2)
  ((c y i a) _ (3 1 2 4))
  )

(table cp-on-creation
  (A (M x g) 1)
  (M (N k l s) 1)
  (M g 6)
  (N (T c y i) 1)
  (N s 4)
  (T (@ d B t) 1)
  (T i 3)
  (@ (& C f a) 1)
  (@ (i B t) (5 3 3))
  (& (U h b) 1)
  (& (t C f a) (3 4 2 3))
  (U (F z q) 1)
  (F X 1)
  (F (z q) (5 3))
  ((x k l) W (1 4 3))
  ((c y i a) _ (1 1 3 5))
  )

(table constructor-absorbed-by
  (A M true)
  (M N true)
  (N T true)
  (T @ true)
  (@ & true)
  (& U true)
  (U F true)
  ((x k l) W true)
  ((c y i a) _ true)
  )

(table hp-to-garrison
  (A M 4)
  (M N 5)
  (N T 6)
  (T @ 7)
  (@ & 8)
  (& U 9)
  (U F 10)
  ((x k l) W 1)
  ((c y i a) _ (2 2 2 3))
  )

(table occupant-can-construct
  (u* u* true)
  )

; development

(add u* tech-per-turn-max 1)

(add (M N T @ & U F) tech-max 15)
(add (M N T @ & U F) tech-to-build 15) 

(add l tech-max 1)
(add l tech-to-build 1)

(add X tech-max 2)
(add X tech-to-build 2)

(table tech-per-develop
  (A M 1.00)
  (M (N l) 1.00)
  (N T 1.00)
  (T @ 1.00)
  (@ & 1.00)
  (& U 1.00)
  (U F 1.00)
  (F X 1.00)
)
(table acp-to-develop 
  (A M 1)
  (M (N l) 1)
  (N T 1)
  (T @ 1)
  (@ & 1)
  (& U 1)
  (U F 1)
  (F X 1)
  )

(table auto-repair
  ; cities & bases repair older & contemporary units, not newer ones
  (A (x g) 1.00)
  (M (x g k l s) 1.00)
  (N (x g k l s c y i) 1.00)
  (T (x g k l s c y i d B t) 1.00)
  (@ (x g k l s c y i d B t C f a) 1.00)
  (& (x g k l s c y i d B t C f a h b) 1.00)
  (U (x g k l s c y i d B t C f a h b z q) 1.00)
  (F (x g k l s c y i d B t C f a h b z q X) 1.00)
  (W (x g k l s W) 0.33)
  (_ (x g k l s c y i d B t C f a h b _) 0.33)
  (cities cities 0.33)
  (i i 0.50)
  (a a 0.50)
  (q q 0.50)
  (X X 1.00)
  )

(add cities hp-recovery 0.33)
(add bases  hp-recovery 0.33)

;RESOURCES

(table supply-on-completion
  (u* m* 0)
;                   _  N   T   @   &  U F
  (pitstops fuel (100 50 100 200 300 50 0))
  (pitstops ammo ( 50 25  50  50  50 50 0))
  ((& b U) bombs (25 0 50))
  )

(table unit-storage-x
;                   _   N   T   @   &   U F
  (pitstops fuel (200 100 200 400 600 100 0))
  (pitstops ammo (100  50 100 100 100 100 0))
;               x g k l s c y  i  d   B   t   C  f  a  h  b z q X
  (movers fuel (0 0 0 0 0 0 0 20 75 400 400 400 18 40 12 36 0 0 0))
  (movers ammo (0 0 0 0 0 8 5  6  3  40  50  80  3 12  4  0 0 0 0))
  ((& b U) bombs (50 1 100))
  )

(table base-production
;                   _   N   T   @   &   U   F
  (pitstops fuel ( 20  30  60  90 120 100 999))
  (pitstops ammo ( 10   5  10  10  10 100 999))
  ((& U F C _) bombs (1 50 100 3 3))
  )

; ----------------------------------------
; (table productivity 
;   ((i a) t* 0)
;   ((i a) plains 100)
; )
; (table base-production
;   ((i a) forest 50)
;   ((i a) fuel 2)
; )
; ----------------------------------------

(table base-consumption ((a i d h f b) fuel (1 1 5 2 3 2)))

(table hp-per-starve ((a i d h f b) fuel 1.00))

(table in-length 
  (u* m* 0)
  (places m* -1)
  )

(table out-length 
  (u* m* 0)
  (ground m* -1)
  (ship m* 0)
  (F m* 0)
  )

;MOVEMENT

(add movers acp-per-turn 
;  x g k l s c y i d B t C f a h b  z q X
  (2 3 2 2 3 3 1 2 5 5 5 5 9 4 4 6 12 8 9))
(add movers acp-min 
;   x  g  k  l  s  c  y  i  d  B  t  C  f  a  h  b  z  q  X
  (-1 -1 -1 -1 -1 -2 -2 -1 -1 -1 -1 -1  0 -3  0  0  0 -2  0))
(add places acp-per-turn 1)
(add bases acp-per-turn 0)
(add F acp-per-turn 4)

(add places speed 0)
(add movers speed 100)
(add F speed 100)

(table mp-to-enter-terrain 
  (u* t* 99)
  (ground land 1)
  (ground river 1)
  (c river 2)
  ((y a) river (2 5))
  ((x i) swamp 2)
  (c (mountains forest) 3)
  (a (mountains forest) 2)
  (aircraft t* 1)
  (aircraft river 0)
  (ship water 1)
  ((B C) shallows 2)
  (F t* 1)
  (F river 0)
  (X sea 2)
  )

(table mp-to-traverse
  (ship river 2)
  (ground road 1)
  )

(table consumption-per-move 
  (aircraft fuel 1)
  ((z X) fuel 0)
  ((B t C) fuel 1)
  (F m* 0)
  )

(table material-to-move ((i a B t C d f h b) fuel 1))

(table accident-hit-chance 
  (u* t* 0)
  ((x i) swamp (1000 500))
  )

;FIXME "has been overcome by malaria" (x i) accident-message

;VOLUME STUFF

(table unit-size-as-occupant
  (u* u* 9999)
  (places places 200)
  (ground u* 1)
  ((i y c) u* 2)
  ((a h) u* 4)
  (q u* 8)
  (g u* 3)
  ((s t) u* 6)
  ((B C) u* 24)
  (d u* 10)
  (f u* 5)
  ((b z) u* 15)
  (X u* 100)
  )

(table occupant-max 
  (u* u* 0)
  (g (x k l) 3)
  (s (x k l c y i) 8)
  (t (x k l c y i a h) 8)
  (h i 1)
  (C (h f b z) 3)
  (z q 2)
  (bases u* 10)
  (makers u* 20)
  (U u* 10)
  ((W A M N) aircraft 0)
  ((_ T @ & U) X 0)
  (F X 1)
  (N d 1)
  (ship ship 0)
  (places places 0)
  (A M 1)
  (M N 1)
  (N T 1)
  (T @ 1)
  (@ & 1)
  (& U 1)
  (U F 1)
  )

(add g capacity 3)
(add s capacity 8)
(add t capacity 16)
(add h capacity 2)
(add C capacity 33)
(add z capacity 16)
(add _ capacity 100)
(add W capacity 16)
(add cities capacity 1000)

(table mp-to-enter-unit 
  (f u* 9)
  (b u* 6)
  (d u* 5)
  (z u* 12)
  (X u* 8)
  (h u* 4)
  )

(add (f b d z X h) free-mp (9 6 5 12 8 4))

;;; Unit-terrain capacities.

;; Limit units to 4 in one cell.
;;  Places cover the entire cell, however.
  
(table unit-size-in-terrain
  (movers t* 4)
  (X t* 12)
  (places t* 16)
  )

(add t* capacity 16)

; aircrafts can always overfly places
(table terrain-capacity-x 
  (u* t* 0)
  (aircraft cell-t* 2)
  (X t* 0)
  )


;SIGHT AREA

(add d vision-range 2)	; was 1
(add C vision-range 2)
(add & vision-range 4)
(add U vision-range 6)
(add X vision-range 8)
(add F vision-range 10)

;COMBAT
; Note that the battledroid does not recognize the phalanx as being armed
; and will refuse to attack it.  Other than that things are fairly staight
; forward.  I based these numbers on my recolections of history, where such
; recollections were available.  (ie. Italians in Ethiopia, Brits vs Zulus,
; etc. for i vs x)  For how TraXors fare against armor, I used the best
; available sources.

(table damage
  (u* u* 1)
  (aircraft ship 2)
  (aircraft places 2)
  (b u* 100)
  (b aircraft 1)
  (b X 100)
  (y i 2)
  (y (A M) 3)
  (B u* 2)
  ((z X F) u* 99)	; who knows how they do it...
  (z (q F X) (2 2 8))
  (q u* 3)
  (X (F X) 5)
  (F (F X) 5)
  (q places 0)
  )

;                   W _ A M N T @ & U  F
(add places hp-max (2 5 4 5 6 7 8 9 10 11))
;                   x  g  k  l  s  c  y  i  d  B  t  C  f  a  h  b  z  q  X
(add movers hp-max (1  1  1  1  2  2  2  2  1  8  2  4  1  3  1  1  1  4 24))


(table hit-chance
; ------------------------- movers vs. movers -------------------------
;             x  g  k  l  s  c  y  i  d  B  t  C  f  a  h  b  z  q  X
  (x movers (40 10 50 20 10 10 05 15  0  0  0  0  0  1  0  0  0  5  0))
  (g movers (10 40 10 05 20  0  0  0  0  0  0  0  0  0  0  0  0  0  0))
  (k movers (60 20 40 20 15 20 05 15  0  0  0  0  0  1  0  0  0  0  0))
  (l movers (80 40 80 40 30 30 20 20  4  0  0  0  0  1  0  0  0  0  0))
  (s movers (12 40 10 07 40  5 15  5  0  0  5  0  0  0  0  0  0  0  0))
  (c movers (60 40 60 60 35 40 80 20 15 10 20 10  5 20 10 10  3  0  0))
  (y movers (90 90 90 80 60 20 40 80  8 15 25 10 10 25  0  0  0  0  0))
  (i movers (80 70 80 70 35 80 20 40 25  5  5  5  5 20 10 10  3  0  3))
  (d movers (60 60 60 60 40 30 30 30 20 40 40 40  5 15  5  1  1  0  1))
  (B movers (90 90 90 90 90 60 60 40 30 40 80 80 20 30 25 20  5  3  5))
  (t movers (15 15 15 15 15  5  5  5 30  5 15  5 20  3 20 10  2  0  2))
  (C movers ( 0 15  0  0 15  0  0  0 99 10  0  0 40  0 30 30 10  0  1))
  (f movers (90 90 90 90 90 50 50 50 99 80 80 60 30 40 90 90 40  3 60))
  (a movers (99 80 99 99 80 70 70 70 40 20 20 20 30 40 30 40 12  5 20))
  (h movers (99 90 99 99 90 99 99 90 80 10 30 10 10 70 20 10 05  5 05))
  (b movers (99 99 99 99 99 99 99 99 99 99 99 99 20 99 99 10 40 10 80))
  (z movers (99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 49 60 99))
  (q movers ( 0 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 40 50 80))
  (X movers (99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 25 70 90))
; ------------------------- places vs. movers -------------------------
;             x  g  k  l  s  c  y  i  d  B  t  C  f  a  h  b  z  q  X
  (W movers (50 50 60 40 10 10 05 10  0  0  0  0  0  0  0  0  0  0  0))
  (_ movers (80 70 80 70 35 80 25 35 30  5  5  5 15 20 20 20  3  0  3))
  (A movers (50 50 40 40 10 10 05 10  0  0  0  0  0  0  0  0  0  0  0))
  (M movers (80 40 40 40 20 20 05 12  2  0  0  0  0  0  0  0  0  0  0))
  (N movers (80 60 60 50 30 30 30 30 30 10 10  5  5  5  0  0  0  0  0))
  (T movers (90 70 70 70 60 50 35 50 40 20 20 10 15 20  5  0  0  0  0))
  (@ movers (95 80 80 80 70 60 40 60 40 25 25 20 20 25 15 10  0  0  0))
  (& movers (95 90 90 90 90 70 45 70 50 30 30 25 25 40 25 20  5  0 10))
  (U movers (99 99 99 99 99 90 80 80 60 50 50 50 60 60 60 60 40  0 40))
  (F movers (99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 99 70 45 30))
; ------------------------- movers vs. places -------------------------
;              W   _   A   M   N   T   @   &   U   F
  (x places ( 30   5  50  30  10   4   1   1   0   0))
  (k places ( 30  15  70  50  20   5   3   2   1   0))
  (l places ( 40  10  50  50  20   6   5   4   2   0))
  (c places ( 40  20  50  50  40  30  20  10   5   0))
  (y places ( 60  40  90  90  70  60  50  20   5   0))
  (i places ( 40  30  70  60  50  50  40  30  10   0))
  (B places ( 99  99  99  99  90  90  90  60  10   0))
  (f places ( 99  60  99  99  90  90  60  50  15   0))
  (a places ( 50  90  99  95  95  90  80  80  30   0))
  (h places ( 40  20  20  20  20  20  20  20  20   0))
  (b places ( 99  99  99  99  99  99  99  99  40  01))
  (z places ( 99  99  99  95  90  80  60  50  30  30))
  (q places 100)
  (X places (100 100 100 100 100 100 100 100 100  70))
  (F places (100 100 100 100 100 100 100 100 100  30))
  )

(table capture-chance
;             W  _  A  M  N  T  @  &  U  F
  (x places (40  5 50 20 20 10  5  5  0  0))
  (k places (50 15 70 50 30 12 10 10  1  0))
  (l places (20 10 20 20 20 10 10 10  0  0))
  (c places (80 30 90 60 40 30 30 20 10  0))
  (i places (90 70 90 70 70 70 60 60 20  0))
  (a places (99 90 99 95 95 90 80 80 30  0))
  (h places (20 20 20 20 20 20 20 20 20  0))
  (q places (99 99 99 95 95 90 90 90 50 30))
  )

(table independent-capture-chance
;             W  _  A  M  N  T  @  &  U  F
  (x places (65 15 75 30 22 15  9  7  2  0))
  (k places (75 40 90 75 50 30 20 14  3  0))
  (l places (45 30 45 45 40 25 15 10  2  0))
  (c places (90 55 95 80 60 50 40 30 15  1))
  (i places (99 85 99 95 90 90 80 70 30  5))
  (a places (99 99 99 99 99 95 90 85 50 10))
  (h places (30 30 30 30 30 30 30 30 30  1))
  (q places (99 99 99 99 99 98 98 98 75 50))
  )

;****************************************************

(table protection add (places movers 50))
(table consumption-per-attack add ((c y i d B t C f a h) ammo 1))
(table hit-by add (u* (ammo bombs) 1))
(table consumption-per-attack add (b bombs 1))

;OTHER STUFF

(add u* acp-to-change-side 1)	; equipment is indifferent to its fate
(add (x k l c i) acp-to-change-side 0)	; but armies have some loyalty

; disband takes one turn
(add movers hp-per-disband 100)
;                           x g k l s c y i d B t C f a h b  z q X
(add movers acp-to-disband (2 3 2 2 3 3 1 2 5 5 5 5 9 4 4 6 12 8 9))

(add u* spy-chance 0)
(table spy-quality (u* u* 10))	; a higher spy-quality is too big an advantage

;;; Scoring.

(scorekeeper
  (title "")
  (do last-side-wins)
  )

;;; Text.

(set action-notices '(
  ((destroy u* ground) (actor " defeats " actee "!"))
  ((destroy u* aircraft) (actor " shoots down " actee "!"))
  ((destroy u* ship) (actor " sinks " actee "!"))
  ((destroy u* cities) (actor " flattens " actee "!"))
  ((destroy u* q) (actor " terminates " actee "!"))
  ((destroy u* X) (actor " liquidifies " actee "!"))
  ))

(set event-notices '(
  ((unit-starved ground) (0 "runs out of fuel and disbands"))
  ((unit-starved aircraft) (0 "runs out of fuel and crashes"))
  ((unit-starved u*) (0 "runs out of supplies and is sucked into a wormhole"))
  ))

(set event-narratives '(
  ((unit-starved ground) (0 "ran out of fuel and disbanded"))
  ((unit-starved aircraft) (0 "ran out of fuel and crashed"))
  ((unit-starved u*) (0 "ran out of supplies and was sucked into a wormhole"))
  ))

(set advantage-min 1)
(set advantage-default 1)
(set advantage-max 5)

; This is a totally arbitrary list.  I'd be happy to add your favorite
; linguistic group, tribe or dead civilization if you send me email
; at <al172@yfn.ysu.edu>.
(set side-library '(
  ((noun "Abyssinian") (name "Abyssinia"))
  ((noun "Aryan"))
  ((noun "Ashanti"))
  ((noun "Assyrian") (name "Assyria"))
  ((noun "Aztec"))
  ((noun "Carthaginian") (name "Carthago"))
  ((noun "Celt"))
  ((noun "Dravidian"))
  ((noun "Etruscan"))
  ((noun "Gaul"))
  ((noun "Hittite"))
  ((noun "Hun"))
  ((noun "Inca") (adjective "Incan"))
  ((noun "Iroquois"))
  ((noun "Khmer"))
  ((noun "Manchu"))
  ((noun "Maya") (adjective "Mayan"))
  ((noun "Minoan"))
  ((noun "Mongol"))
  ((noun "Mycenaean") (name "Mycenae"))
  ((noun "Parthian"))
  ((noun "Phoenician"))
  ((noun "Saxon"))
  ((noun "Semite"))
  ((noun "Spartan") (name "Sparta"))
  ((noun "Sumer"))
  ((noun "Viking"))
  ((noun "Visigoth"))
  ((noun "Zulu"))
))

; Henry Ware's cities.nmz                     Last modified 25 January 1994
; I wrote these because I didn't like the collection of small US towns in
; the default names.  They are an eclectic collection of place-names, weird
; allusions & random thoughts.  If you like, you can think of it as my
; Finnegan's Wake.  ;^) I'm sure even the misspellings have Freudian (&
; Jungian!)  connotations.  It must still be considered a work in progress.
; Its been run through a Unix 'sort -df', of course (Joyce was _so_ 19th
; century :^))

(namer unit-names (random
"Abelarde" "Abidjan" "Abruzzi" "Abuja" "Abzu" 
"Acropolis" "Adis Abbaba" "Adonis" "Aeneid" 
"Agamemnon" "Agrabah" "Aiko Aiko" "Albion" 
"Alexandria"  "Allacante" "Allegory" "Alliteration" 
"Alma Ata" "Altamoro" "Althea" "Amsterdam" 
"Anakara"  "Ancorage" "Angst" "Antietam" 
"Antigone" "Appomattox" "Aramis" "Archangel" 
"Arimathea" "Armageddon" "Artos" "Asmera" 
"Asterlitz" "Aswan" "Athenry" "Athens" 
"Atlantis" "Austin" "Avalon" "Avignon" 

"Babylon" "Baghdad" "Baku"  "Ballymote" 
"Baltimore" "Bangalore" "Bangkok" "Bangore" 
"Barbizon" "Bartok" "Bauhaus" "Beijing" 
"Belfast"  "Belfry" "Belograd" "Berlin" "Beruit" 
"Bethlehem" "Bexley"  "Biafra" "Bimini" 
"Blackfriar" "Bogota"  "Bombay"  "Bonn" 
"Bordeaux" "Borodino" "Boston"  "Boulder" 
"Bowery" "Brandenberg" "Brighton Beach" "Brooklyn" 
"Budapest"  "Buenos Aires"  "Buffalo" "Burgandy" 
"Bushido" "Bytor" "Byzantium" 

"Caer Asseyra"  "Cairo" "Calcutta" "Calgary" 
"Caliban" "Caligula" "Cambridge" "Camden" 
"Camelot" "Campo Formio" "Canterbury"  "Cape May" 
"Caracas"  "Carranza" "Casablanca" "Casanova" 
"Cashel" "Cassius" "Cayenne"  "Chartes" 
"Chicago"  "China Grove" "Chinatown" "Chronicles" 
"Cicero" "Cincinnati"  "Claudius" "Clausewitz" 
"Clemenceau" "Cleveland"  "Cognac"  "Coleridge" 
"Cologne" "Columbus"  "Commodus" "Concepcion" 
"Concord" "Conquistador" "Constantinople" 
"Copenhagen" "Cordoba" "Corinth" "Cork" 
"Cornucopia" "Corrina" "Corsair" "Crayola" 
"Cuzco" 

"Daedalus" "Dakar" "Dallas"  "Dar es Salaan" 
"Dark Hollow" "Dasharastria" "Datona"  "De Valera" 
"Decameron" "Delphi" "Demosthenes" "Denver" 
"Des Moins"  "Detroit" "Dharma" "Diamonds" 
"Dijon" "Djibouti"  "Dodge"  "Dol Amroth" 
"Dominican" "Dominion" "Dresden" "Dublin" 
"Duluth" 

"East Orange" "Eclipse"  "Edo" "El Norte" 
"El Paso" "Ellis Island" "Elysium" "Empire" 
"Endor" "Enquiry" "Entebe" "Enterprise" "Ermine" 

"Fenario" "Firenze" "Florence" "Fort Fenian" 
"Fort Sumter" "Fort William Henry" 

"Gadalajara" "Galahad" "Galapogos" "Ganelon" 
"Geneva" "Genoa" "Gettysburg" "Gibbon" 
"Gibraltar" "God's Crucible" "Gomorrah" "Gonzaga" 
"Gorky" "Gotham City" "Greenville" "Greenwich" 
"Gremlin" "Guadalcanal" "Guantanamo" "Guido's Razor" 

"Hackensack" 
"Hades" "Haiku" "Hajar-al-Aswad" "Halifax" 
"Hamburg" "Hammurabi" "Hanibal" "Hanoi" 
"Harbour Springs" "Harlem" "Hassan-i-Sabbah" 
"Hastings" "Hathaway" "Havana" "Havelock" 
"Heidelburg" "Hel" "Heloise" "Herraclitus" 
"Hiaku" "Hiawatha" "Hiroshima" "Hoboken" 
"Hong Kong" "Houston" "Huron" "Hyde Park" 
"Hyperborea" "Hyperion" 

"Icarus" "Ille de Batz" "Inverness" "Istanbul" 
"Izmir" 

"Jagannath" "Jakarta" "Java" "Jericho" 
"Johannasburg" "Juneau" "Jutland" 

"Kabuki" "Kabul" "Kalamazoo" "Kali" 
"Kaluha Kona" "Kankakee" "Kanpur" "Karma" 
"Katharsis" "Katmando" "Kenosha" "Kiel" "Kiev" 
"Kingston" "Kinshasa" "Kirkuk" "Konigsberg" 
"Kourian" "Kyoto" "Kyushu" 

"La Paz" "Labarynth" "Lagos" "Lakewood" 
"Lancaster" "Laurasia" "Left Field" "Leibnitz" 
"Leinster" "Lemberg" "Leningrad" "Lexington" 
"Lhasa" "Liffey" "Lille" "Lima" "Limbo" 
"Limerick" "Lisbon" "Lithopolis" "Little Big Horn" 
"Logos" "London" "Lorient" "Los Angeles" 
"Lusitania" "L'vov" "Lye" "Lyon" 

"Macao" "Machiavelli" "Machinaw" "Madrid" 
"Managua" "Manchester" "Mandarin" "Mandrake" 
"Manila"  "Mannheim" "Marengo" "Marne" 
"Marrachesh" "Marseilles" "Maximillian" "Medellin" 
"Merrimac" "Miami" "Milan" "Milwaukee" 
"Minerva Park" "Minglewood" "Minsk" "Mogadishu" 
"Monitor" "Monrovia" "Mont Saint Michel" 
"Monte Cristo" "Monte Video" "Montreal" "Moscow" 
"Munich"  "Murmansk" "Mythos" 

"Nagasaki" "Nagoya" "Nairobi" "Nantes" "Naples" 
"Napoleon" "Naraka" "New Aesop" "New Atlantis" 
"New Haven" "New York" "Newark" "Newcastle" 
"Nicea" "Nirvanna" "Norfolk"  "Norwich" "Nyack" 

"Obetz" "Occam's Razor" "Odessa" "Omar Khayyam" 
"Oracle" "Orth" "Oslo" 

"Palermo" "Palo Alto" "Pamplona" "Pangaea" 
"Paradox" "Paris" "Pataskala" "Pathos" 
"Pendragon" "Petovsky" "Phaedra" "Pheonix Park" 
"Picadilly" "Pindar Point" "Pisa" "Pittsburgh" 
"Plattsburgh" "Pomona" "Port au Prince" 
"Port Manteau" "Port Nemo" "Porthos" "Porto Alegre" 
"Prauge" "Pretoria" "Princeps" "Pueblo" 
"Purgatory" 

"Quebec" "Quito" 

"Rakshasa" "Rangoon" "Rennes" "Rio de Janeiro" 
"Riverrun" "Riyadh" "Roger's Losing" "Roland" 
"Rome" "Roscoff" 

"Sacramento" "Saginaw" "Saigon" "Saint Malo" 
"Saint Petersburg" "Saki" "Salamanca" "Salem" 
"Salt Lake" "Sampan" "San Antionio" "San Fransico" 
"San Juan" "Sanctuary" "Sandusky" "Sangria" 
"Sanity" "Sans Serif" "Santa Cruz" "Santa Fe" 
"Santiago" "Santo Domingo" "Sao Paulo" "Saragosa" 
"Saratoga" "Sarjevo" "Saskatoon" "Sault Ste. Marie" 
"Savannah" "Saxophone" "Seppuku" "Serendipity" 
"Sevastopol" "Shadowfax" "Shanghai" "Shangri-La" 
"Sherwood" "Shiloh" "Sidney" "Sienna" 
"Singapore" "Skagway" "Smyrna" "Sparta" 
"Stalingrad" "Steppenburgh" "Sterling" "Stockhom" 
"Suez" "Summit" "Sun Zoo" "Sunzi Bingfa" 
"Superior" "Syracuse" 

"Tabriz" "T'aipei" "Tannenberg" "Tashkent" 
"Tbilisi" "Tehran" "Tel Al-Aviv" "Tequila" 
"The Aesir" "The Brewery" "The Bronx" "The Hague" 
"The Sprawl" "The Vatican" "Thrace" "Thunder Bay" 
"Tiblisi" "Timbuktoo"  "Tippecanoe" "Tokyo" 
"Toledo" "Toronto" "Trafalgar" "Transvaal" 
"Trantor" "Trenton" "Trieste" "Trouble" "Troy" 
"Tupelo" "Tupper Lake" "Tuzla" 

"Ulan Batar" "Ulysses" "Uno" 

"Valencia" "Vancover" "Venice" "Verdi" "Verdun" 
"Versailles" "Vichy" "Vicksburg" "Vienna" 
"Vladivostok" 

"Walhalla" "Wapakoneta" "Ware Shoals"  "Warsaw" 
"Washington" "Waterloo" "Wellington" "Westport" 
"Wheeling" "Winnipeg" "Wonsan" "Woodstock" 
"Worms" 

"Xax" 

"Yazoo" "Yonkers" "Yorktown" "Ypres" 

"Zagreb" "Zenda" "Zhivago" "Zinfandel" "Zymurgy" 
))
(add cities namer "unit-names")

(game-module (notes (
  "The basic idea here is that each city represents an era.  Each era has its"
  "own set of units & is, in effect, a miniperiod. One of your original cities"
  "should build the more modern city & the others should build units for"
  "expansion & conquest.  After the new city is completed, your older cities"
  "should be upgraded & the develop city should continue developing.  Any"
  "neutral cities you capture will catch up to your current production level"
  "in a reasonable period of time."
  ""
  "When your country is larger, you should consider adding one or two"
  "additional development cities as a backup so you aren't excessively set back"
  "if you primary development center is captured or destroyed."
  ""
  "Generally more modern things have a significant advantage over older"
  "things, but even very old units can do sentry duty."
  ""
  "Any given unit represents a wide range of actual technologies.  For"
  "example, infantry is every thing from muzzle loading guns to weapons with"
  "50 times the range and exploding (heat seeking?) bullets."
  ""
  "Most of the subperiods have a trick in themselves.  For example, the"
  "Napoleonic era is set up with an i>c>y>i, 'paper-scissors-rock', dynamic."
  "That it is part of a larger period affects the balance: while cavalry might"
  "have an immediate advantage due to its speed, they become outdated more"
  "rapidly than the other two units."
  ""
  "With that said, I should also say that the play balance is not perfect.  It"
  "is still, to a certain extent, a novelty period; but I think it is"
  "reasonably successful on that level."
  ""
  "There should be more notes, but this period needs a book :^)"
  ""
  "Let me recast:  there are no notes because, in simulation of real life,"
  "the interplay of future events is unclear until you experience them..."
 )))

(game-module (design-notes
  "   T*** TIME: COMBAT THRU THE AGES T***"
  ""
  "Version 1.0.  This is part of the Ohio Xconq Suite, version 1.0."
  "This was written by Henry Ware.  Victor Mascari helped."
  "Massimo Campostrini helped with beta testing."
  "Thanks also to Jay Scott of the Future Period, to the author of the Greek"
  "Period and to whoever wrote the Napoleonic Period; this period owes alot"
  "to those works."
  ""
  "I *do* like this period, but its not as sound or as neat from a game-"
  "theory standpoint as Nukem, WW1 or the Standard period.  To say nothing of"
  "Galaxy, which is my personal favorite."
  ""
  "If you think it is complicated now, just wait!  I am adding nomad, Roman,"
  "nanotech, pirate, Ancient Egypt/Inca and cyber eras!  (Just kidding.  Really."
  "There is no room for more units anyway. :^)  The only thing I might do is"
  "reintroduce the robowarrior."
  ""
  "This period is way too complicated & way too slow.   (It may be Baroque,"
  "but I am not going to fix it)."
  ""
  "Gratuitous Sun Tzu quote:"
  "\"Which general has the ability?\""
  ""
  "Copywrite 1990, 1991, 1993, 1994 by Henry Ware."
  "My work on this period is distributed under the GNU general public license."
  "There are a few lines authored by Stan Shebs, and may be protected by"
  "his copywrite also."
  ""
  "Converted to xconq 7 with minor modifications" 
  "by Massimo Campostrini."
))
