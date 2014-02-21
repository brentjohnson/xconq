(game-module "postmodern"
  (title "Post-Modern")
  (blurb "A time of surviving city-states struggling for control of the apocalypse's ruins ...")
  (variants
   (world-seen false)
   (see-all false)
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

; possibly radar should get no moves - you have to embark it explicitly ?
; but then you can't disembark it.

(unit-type i (name "infantry") (image-name "soldiers")
  (help "marches around and captures things"))
(unit-type a (name "armor") (image-name "tank")
  (help "faster than infantry, limited to open terrain"))
(unit-type S (name "Special Forces") (image-name "elite")
  (help "special infiltration units"))
(unit-type r (name "radar") (image-name "radar")
  (help "small device that sees far"))
(unit-type A (name "airlifter") (image-name "airlifter")
  (help "big cargo plane, for moving supplies and troops"))
(unit-type f (name "fighter") (image-name "jets")
  (help "interceptor to get those nasty aircraft"))
(unit-type b (name "bomber") (image-name "bomber")
  (help "long range aircraft, carries infantry and bombs"))
(unit-type d (name "destroyer") (image-name "small-ship")
  (help "fast, cheap, and sinks subs"))
(unit-type s (name "submarine") (image-name "sub")
  (help "sneaks around and sinks ships"))
(unit-type m (name "sea mines") (image-name "seamine")
  (help "sea mine, floats around, sinks shps"))
(unit-type t (name "transport ship") (image-name "cargo-ship")
  (help "carries units and supplies across the water"))
(unit-type j (name "jeep") (image-name "jeep")
  (help "trucks and jeeps for moving infantry and supplies"))
(unit-type X (name "mech infantry") (image-name "mech")
  (help "infantry in powered armor and AFVs"))
(unit-type C (name "carrier") (image-name "carrier")
  (help "carries aircraft around"))
(unit-type z (name "spy plane") (image-name "spysat")
  (help "fast, hard to see spy craft"))
(unit-type B (name "battleship") (image-name "battleship")
  (help "the most powerful ship"))
(unit-type G (name "missile") (image-name "missile")
  (help "powerful explosive, very accurate"))
(unit-type N (name "atomic") (image-name "bomb")
  (help "nuclear fuel unit, or nuclear bomb"))
(unit-type e (name "engineers") (image-name "tractor")
  (help "produces things, fights sieges"))
(unit-type O (name "bolo") (image-name "ogre")
  (help "huge CyberTank - a veritable land battleship"))
(unit-type L (name "land fortifications") (image-name "walltown")
  (help "holds enemies at bay!"))
(unit-type & (name "bridge") (image-name "bridge")
  (help "serves as something units can walk over"))
(unit-type _ (name "base") (image-name "airbase")
  (help "airstrip plus port"))
(unit-type V (name "village") (image-name "village")
  (help "small town"))
(unit-type T (name "town") (image-name "town20")
  (help "smaller than a city"))
(unit-type @ (name "city") (image-name "city20")
  (help "capital of a side"))

(material-type fuel
  (help "basic motive power"))
(material-type ammo
  (help "generic hitting capability"))
(material-type people
  (help "population, used to stock cities, operate units"))

(terrain-type sea (char "."))
(terrain-type shallows (char ","))
(terrain-type swamp (char "="))
(terrain-type desert (char "~"))
(terrain-type plains (char "+"))
(terrain-type forest (char "%"))
(terrain-type mountains (char "^"))
(terrain-type ice (char "_"))

(define bases ( L & _ V T @ ))
(define cities ( V T @ ))
(define makers ( V T @ ))
(define ground ( i a e S j X O ))
(define aircraft ( A f b G z ))
(define ships ( d s m t C B ))
(define capital ( O C B ))
(define movers ( i a S r A f b d s m t j X C z B G N e O ))
(define water ( sea shallows ))
(define land ( plains forest desert mountains ))

(add water liquid true)

;;; Static relationships.

(table vanishes-on
  (ground water true)
  (e t* false)
  (ships land true)
  (bases water true)
  )

;; Unit-unit.

(table unit-capacity-x
  ;; All units can have radar.
  (u* r 1)
  ;; Fadar gives away their position - they use passive sensing.
  (( S z ) r 0)
  ;; Fighters carry missiles.
  (f G 4)
  ;; Destroyers carry sea mines and missiles.
  (d ( m G ) ( 8 2 ))
  ;; Mech infantry - missiles and a nuke.
  (X ( G N ) ( 20 1 ))
  ;; a missile can be a nuke carrier
  (G N 1)
  (A ( i a S j X  m O e )
     ( 2 1 2 3 1 10 1 1 ))
  ;; armor - a single troop
  (a (i S) 1)
  (b ( i S m j N r G )
     ( 1 1 6 1 2 1 4 ))
  (s ( S N G )
     ( 1 2 4 ))
  (t ( i a S j X m  O G e )
     ( 6 3 8 8 3 12 1 6 2 ))
  (j ( i S X G e )
     ( 3 3 2 3 1 ))
  (C ( f  b m z G  N )
     ( 10 3 6 2 10 2 ))
  (B ( i S m G N ) 8)
  (G N 3)
  (O ( G N ) ( 6 3 ))
  (bases u* 40)
  (bases bases 0)
  )

(add u* capacity 0)
(add (i a S A b s t j C B O) capacity
     (1 1 1 4 2 2 6 3 8 1 1))
;                     L  &  _  V  T  @
(add bases capacity ( 3  2  8 12 24 48 ))

(table unit-size-as-occupant
  (u* u* 99)
  ;; everything carries nukes and missiles
  ((N G) u* (1 1))
  (ground bases 1)
  ;             d s m t C B "ships"
  (ships bases (4 4 1 5 6 6))
  ;                A f b G z "aircraft"
  (aircraft bases (2 1 2 1 1))
  ;; Armor can carry stuff.
  ((i S N G) a (1 1 1 1))
  ;; Airlifter can carry all kinds of things.
  ((i a S j X m O e) A
   (1 1 1 1 1 1 1 1))
  )

(table occupant-max
  (u* N 1)
  (u* G 2)
  ; Small groups of troops can ride on armor!
  (a (i S) 1)
  (S ( N G ) ( 1 2 ))
  (A ( i a S j X  m O e )
     ( 2 1 2 2 1 10 1 1 )) ; heavy airlifter indeed
  (b ( i S m j N r G )
     ( 1 1 6 1 2 1 4 ))
  (s ( S N G ) ( 1 2 4 ))
  (t ( i a S j X  m O G e )
     ( 6 3 8 8 3 12 1 6 2 ))
  (j ( i S X G e )
     ( 3 3 2 3 1 ))
  (C ( f b m z G N )
     ( 10 3 6 2 10 2 ))
  (B ( i S m G N ) 8)
  (G N 3)
  (O ( G N ) ( 6 3 ))
  (bases u* 40)
  (bases bases 0)
  )

;; Unit-terrain.

;; We want to be able to stack lots but not infinite units, and have bases
;; only be 1/cell.

(add t* capacity 16)

(table unit-size-in-terrain
  (u* t* 1)
  (bases t* 12)
  )

;; Unit-material.

(table unit-storage-x
;                   L  &   _   V   T    @ "bases"
  (bases fuel   ( 100 50 100 300 500  500 ))
  (bases ammo   (  70 10  50  80 100  200 ))
  (bases people (   2 1   20 200 400  999 ))
;                  i  a  S r  A  f  b  d  s m  t  j  X   C  z   B  G  N  e   O "movers"
  (movers fuel   ( 6 10 20 2 40 18 36 99 99 5 99 50 30 400 48 200 20 10 30 180 ))
  (movers ammo   ( 6 10 20 2 40 18 36 99 99 2 99 50 30 400 48 200 20 10 30  80 ))
  (movers people ( 3  1  1 0  2  0  5  4  1 0 20 10  1   8  0   9  0  0 30   0 ))
  )

;;; Vision.

(table visibility
  (( S m s z ) t* 0) ; can't see these unless you step on them
  (( G N ) t* 10) ; missiles and nukes tough to see
  ; small forces hidden by rough terrain
  ( (i X j) forest 40)
  ( (i X j) swamp 30)
  ( (i X j) mountains 50)
  )

(add cities see-always 1)

(add aircraft vision-range 2)
(add (C B) vision-range 2)
(add (r s z T @) vision-range 6)
(add m vision-range 0)

;;; Actions.

;                          i a S r A f b d s m t j X C  z B  G N e O "movers"
(add movers acp-per-turn ( 1 2 2 1 5 9 6 3 3 1 3 3 2 4 12 4 10 2 1 3 ))

(add cities acp-per-turn 1)

;;; Movement.

;(add ( r m ) speed 0)

(add cities speed 0)

(table mp-to-enter-terrain
  (u* t* 99)
  (ships water 1)	; sea is pretty straightforward...
  (( s C B ) shallows (2 3 3))	; big ships don' like shallows
  (ground water 99)
  (( X O ) shallows 3)
  (aircraft t* 1)	; Aircraft ignore most terrain
  (bases t* 1)	; bases can go anywhere
  ; movement on terrain...
  ;; Ships can't normally go on land...
  (ships land 99)
  ;; ...but swamps are shallow enough for small ones.
  (( d t m ) swamp 1)
  ; ground units and land terrain:
  ;Ice... is special.. It's often water covered with ice.
  (ships ice 99)
  (s ice 1)	; subs can go under ice
  ;; Sea mines in the sea.
  (m water 1)
  ; ground units ant how they handle terrain:
  ;               i  a e S  j X O  "ground"
  (ground swamp  (1 99 1 1 3 2 3))	; most ground units don't like swamp
  (ground desert 1)
  (ground plains 1)
  ;; Vehicles must navigate/crush way through forest
  (ground forest (1 2 1 1 2 1 1))
  (ground mountains (1 99 1 2 99 2 3))
  (ground ice (99 99 1 2 3 2 2))
  ;Other special moves:
  ;-1 t* r moves ; radar can only operate as a passenger
  ;; Radar techs can take radar anywhere but water.
  (r t* 1)
  (r water 99)
  ;; Engineers can go anywhere, they have their own boats.
  (e t* 1)
  (cities land 1)	; looks strange, but needed to define allowable places
  )

(table consumption-per-move
  ;; Most units take 1 fuel to move.
  (movers fuel 1)
  ;; The big units take more fuel.
  (capital fuel 2)
  )

(table mp-to-enter-unit
  ;; aircraft can't sortie again until next turn
  (aircraft u* 20)
  (bases ships 1)
  )

(table mp-to-leave-unit (bases ships 1))

;;; Construction.

;;; Everybody starts out in a low-tech state, must do develop to get anything
;;; going.  Only towns and cities can do the develop.

;                           i   a   S   r   A   f  b   d  s  m  t  j  X  C   z  B   G   N   e  O
(add movers tech-max      (50 100 160 400 100 120 100 60 88 50 58 50 80 12 100  5 360 375 192 60))
(add movers tech-to-build (50 100 160 400 100 120 100 60 88 50 58 50 80 12 100  5 360 375 192 60))

(table acp-to-develop
  ((T @) movers 1)
  )

(table tech-per-develop
  ((T @) movers 1.00)
  )

;; Many units share technologies.

(table tech-crossover
  (( f b z ) A ( 100 100 100 ))
  (G ( b r ) ( 100 100 ))
  (O ( a X B N ) ( 100 50 100 80 ))
  (S ( i X ) ( 100 80 ))
  (r z 60)
  (A ( f b z G ) ( 60 100 50 50 ))
  (f ( A b z G ) ( 50 90 90 100 ))
  (b ( A f z G ) ( 80 90 60 30  ))
  (z ( A f b G ) ( 20 90 70 90  ))
  (d ( s m t C B ) ( 40 40 60 50 50 ) )
  (m ( s d t C B ) ( 40 40 10 20 20 ))
  (t ( s d m C B ) ( 20 50 20 60 60 ))
  (C ( f s d t m B N ) ( 50 10 40 60 10 80 40 ))
  (B ( O s d t C m N ) ( 60 10 80 50 60 10 40 ))
  (j ( i a ) ( 50 80 ))
  (X ( i a S ) ( 100 80 90 ))
  (N ( b m G O C B ) ( 60 80 60 20 40 40 ))
  (e ( i X ) ( 80 10 ))
  )

; big cities produce a tad faster, especially high tech stuff

;                i  a  S  r  A  f  b  d  s  m  t  j  X  C  z  B  G  N  e  O "movers"
(add movers cp ( 5  9 12  9 13 12 15 13 14  4 11  7 16 45 27 60  7 30 14 45 ))

(add bases cp ( 4 1 10 15 30 45 ))

(table can-create
  (i L 1)
  (ground _ 1)
  (O _ 0)
  (e bases 1)
  (makers movers 1)
  (V O 0)
  )

(table acp-to-create
  (i L 1)
  (ground _ 1)
  (O _ 0)
  (e bases 1)
  (makers movers 1)
  (V O 0)
  )

(table cp-on-creation
  ;           i a e S j X O "ground"
  (ground _ ( 7 5 8 1 7 8 0))
  ;           i  a  S  r  A  f  b  d  s  m  t  j  X  C  z  B  G  N  e  O "movers"
  (T movers ( 2  3  3  4  4  4  6  3  3  2  2  3  5 16 10 21  2  1  3  1 ))
  (@ movers ( 2  3  4  4  4  5  6  4  4  2  3  3  7 19 16 25  3  3  5  8 )) 
  )

(table can-build
  (i L 1)
  (ground _ 1)
  (e bases 1)
  (makers movers 1)
  (V O 0)
  )

(table acp-to-build
  (i L 1)
  (ground _ 1)
  (e bases 1)
  (makers movers 1)
  (V O 0)
  )

(table cp-per-build
  ;          L & _ V T @ "bases"
  (e bases ( 2 1 3 1 1 1 ))
  ((T @) N 2)
  )

(table consumption-per-built
  (movers people (  4  2  1  0  0  0  0  1  1  0  1  1  2  3  0  3  3  0  3  0 ))
  (L ( ammo fuel people ) ( 3 3 1 ))	; fortification needs stockpiles!
;; skip for now, population grows too slowly.
;  (bases people ( 0 0 0 200 400 800 ))	; bases need to be populated
  )

(table can-toolup-for
  (u* u* true)
  )

(table acp-to-toolup
  (u* u* 1)
  )

(table tp-to-build
  (u* u* 0)
  ;; on the average, add about 20% build time for toolup
  ;           i  a  S  r  A  f  b  d  s  m  t  j  X  C  z  B  G  N  e  O "movers"
  (V movers ( 1  2  2  2  2  2  3  2  2  1  2  1  3  9  5 12  1  6  3  9 ))
  (T movers ( 1  2  2  2  2  2  3  2  2  1  2  1  3  9  5 12  1  6  3  9 ))
  (@ movers ( 1  2  2  2  2  2  3  2  2  1  2  1  3  9  5 12  1  6  3  9 ))
  )

(table tp-max
  (u* u* 0)
  ;           i  a  S  r  A  f  b  d  s  m  t  j  X  C  z  B  G  N  e  O "movers"
  (V movers ( 1  2  2  2  2  2  3  2  2  1  2  1  3  9  5 12  1  6  3  9 ))
  (T movers ( 1  2  2  2  2  2  3  2  2  1  2  1  3  9  5 12  1  6  3  9 ))
  (@ movers ( 1  2  2  2  2  2  3  2  2  1  2  1  3  9  5 12  1  6  3  9 ))
  )

(table supply-on-completion
  (bases m* 100)
  )

;;; Repair.

(table can-repair
  (( _ V T @ ) u* true)
  (( L & e ) u* true)
  (capital capital true)
  (i bases true)
  (e u* true)
  (i ( L & _ V ) true)
  )

(table acp-to-repair
  (( _ V T @ ) u* 1)
  (( L & e ) u* 1)
  (capital capital 1)
  (i bases 1)
  (e u* 1)
  (i ( L & _ V ) 1)
  )

(table hp-per-repair
  (( _ V T @ ) u* 1.00)
  (( L & e ) u* 0.50)
  ;; Capital ships and tanks can repair themselves.
  (capital capital 0.12)
  (i bases 0.20)
  ;; Engineers are very good with repairing.
  (e u* 1.00)
  (i ( L & _ V ) ( 1.00 0.50 1.00 0.25 ))
  )

;;; Combat.

;                i a S r A f b d s m t j X C z B G N e  O L &  _  V  T  @
(add u* hp-max ( 1 2 2 1 2 1 3 3 2 1 3 1 3 4 1 8 1 1 1 12 3 2 10 12 20 40 ))

;FIXME in addition you can set hp-to-repair and other things to cripple a unit
;(add u* hp-at-min-speed ( 0 1 0 0 1 0 1 1 1 0 1 0 1 1 0 3 0 0 0  4  1  2  6  8 10 20 ))
; crippled is < test, not <=

; split the matrix in half to reduce the number of entries per line
(define u1 ( i  a  S  r  A  f  b  d  s  m  t  j  X ))
(define u2 ( C  z  B  G  N  e  O  L  &  _  V  T  @ ))

(table hit-chance
  ;         i   a   S   r   A   f   b  d  s  m  t  j  X "u1"
  (i u1 (  50  40  35  90  30  20  25 20 20 30 30 80 30 ))
  (a u1 (  60  50  50  85  35  25  25 25 25 25 35 85 45 ))
  (S u1 (  65  60  50  90  10   7   9 10 12 15 20 80 35 ))
  (r u* 0)	; radar don't hit anything
  (A u* 0)	; Transport plane isn't armed!
  (f u1 (  25  20  15  80  80  60  70 45 50 35 45 75 35 ))
  (b u1 (  20  30  15  80  30  10   9 55 50 35 60 70 40 ))
  (d u1 (   7   9   6  80  20  15  17 50 60 80 75 40 25 ))
  (s u1 (   9  10   7  80  30  10  10 50 50 60 75 20  6 ))
  (m u* 0)
  (m ships ( 40 50 80 70 75 75 ))
  (t u1 (   4   3   3  80  10  10  10 40 30 50 45 20  2 ))
  (j u1 (   1   0   5  80  00  00   0  0  0  0  0 50  0 ))
  (X u1 (  65  70  50  80  30  35  32 40 40 50 50 90 45 ))
  (C u1 (  30  20  15  80  60  50  55 40 40 30 55 30 15 ))
  ; z
  (B u1 (  50  50  45  80  55  40  45 65 60 40 70 80 40 ))
  (G u* 100)	; missiles hit everything, modifiers ineffective
  (N u* 100)	; nukes hit everything, modifiers ineffective
  (e u1 (   5   4   3  70  10   9  10 10 12 15  9 40  3 ))
  (O u1 (  80  70  65  90  60  40  35 60 80 40 60 90 60 ))
  (L u1 (  40  50  25  90  60  30  30 40 40 30 40 90 35 ))
  (& u* 0)	; bridges aren't armed!
  (_ u1 (  10  10  15  80  60  20  20 20 20 20 20 50  7 ))
  (V u1 (  25  15  30  80  70  45  35 25 20 20 30 80 12 ))
  (T u1 (  30  20  35  80  85  50  40 40 20 20 45 80 20 ))
  (@ u1 (  50  40  55  80  95  70  60 50 20 20 55 80 30 ))
  ; second half of matrix
  ;        C   z   B   G   N   e   O   L  &  _   V   T   @ "u2"
  (X u2 ( 65  70  50  80  30  35  32  40 40  50 50  90  45 ))
  (O u2 ( 80  20  65  90  60  60  45  80 85 80  70  65  60 ))
  (i u2 ( 20   1   9  40  40  50  10  60 40 80  70  60  40 ))
  (a u2 ( 20   1  20  50  50  60  30  40 50 90  80  70  50 ))
  (S u2 (  1   3   4   4  30  30  20  40 70 55  40  30  20 ))
  (A u2 (  5   3   0   0   0   0   3   1 10  5  10  25  30 ))
  (f u2 ( 50  50  40  80  80  65  30  48 60 70  80  80  80 ))
  (b u2 ( 70  10  60  50  50  20  40  70 60 90  90  90  94 ))
  (d u2 ( 40   1  20   0   0   5  10  15 50 99  90  90  80 ))
  (s u2 ( 40   1  50   0   0   0  10   0  6  0   0   0   0 ))
  ; m
  (t u2 ( 30   1   9   0   0  20   8   4 30  0   0   0   0 ))
  (j u2 (  1   0   5  80  00  00   0   0  0  0   0  50   0 ))
  ; X
  (C u2 ( 20  10  20   8   8  30   7   0  0  0   0   0   0 ))
  (B u2 ( 50  10  90   0   0  50  40  70 89 99  99  99  99 ))
  ; G
  ; N
  (e u2 (  0   1   3  20  21  30   8  80 70 80  70  60  40 ))
  (L u2 ( 20   5   9  40  40  50  60   0  0  0   0   0   0 ))
  (_ u2 ( 20  20  20   0   0  10  30   0  0  0   0   0   0 ))
  (V u2 ( 20  20  20   0   0  25  30   0  0  0   0   0   0 ))
  (T u2 ( 20  30  20   0   0  30  35   0  0  0   0   0   0 ))
  (@ u2 ( 20  40  50   0   0  50  40   0  0  0   0   0   0 ))
  ;; Missiles and nukes never survive their attack.
  (u* ( G N ) 100)
  )

(table defend-terrain-effect 
  (i (forest swamp mountains) 90)
  )

(table damage
  (u* u* 1)
  (a cities 2)
  (a ( a X O ) 2)
  (S bases 2)
  (b ships 2)
  (b s 1)
  (b ( _ V T ) 2)
  (b @ 3)
  (b ground 2)
  (d s 2)
  (( s m ) ships 3)
  (B u* 2)
  (O u* 3)
  (O bases 4)
  (B cities ( 3 3 4 ))
  (G u* 4)
  (G cities ( 5 5 6 ))
  (N u* 60)
  (X u* 2)
  )

(table acp-to-defend (( G N m ) u* 0))

(table capture-chance
  ;; Elite's special missions.
  (S ( A b s t j C B N ) ( 20 15 15 26 22 4 4 18 ))
  ;; Board and capture!
  (i ( b t j C B ) ( 00 10 15 3 3 ))
  ;           L  &  _  V  T  @ "bases"
  (S bases ( 10 70 60 30 20 10 ))
  (i bases (  5 30 70 55 50 30 ))
  (a bases (  6 40 90 75 70 50 ))
  (X bases (  8 90 90 75 70 60 ))
  )

(add ( G N m ) acp-to-detonate 1)

(table protection
  (cities movers 50)	; cities offer some protection to occupants
  (a cities 90)	; armor protect the cities housing them.
  ; can't make this too large or city can be
  ; invulnerable.
  (i cities 95)	; same for infantry.
  (a _ 75)
  (i _ 85)
  (L ( i a ) (75 90))
  )
; ???
(table protection add (bases ground 99))
(table protection add (L ( e i A X O S ) 98))


(table withdraw-chance-per-attack
  (u* ground 10)
  ;; Ogres don't retreat!
  (u* O 0)
  (u* ships 5)
  (u* aircraft 25)
  )

(table consumption-per-attack (u* ammo 1))

(table hit-by (u* ammo 1))

(set action-notices '(
  ((destroy u* aircraft) (actor " shoots down " actee "!"))
  ((destroy u* ships) (actor " sinks " actee "!"))
  ((destroy u* cities) (actor " devastates " actee "!"))
  ))

;;; Other actions.

(add u* acp-to-change-side 1)	; equipment is indifferent to its fate
(add ( i a e X O S ) acp-to-change-side 0)	; but armies have some loyalty
; Should N, e change sides? (gives away develop)

(add i possible-sides (not "independent"))
(add S possible-sides (not "independent"))

(add movers acp-to-disband 1)
(add ( L & _ V ) acp-to-disband 1)

(add movers hp-per-disband 99)
(add ( L & _ V ) hp-per-disband 99)

;100 u* efficiency ; so you can reclaim materials used to make disbanded units
; (should calc from to-build etc)

;;; Backdrop economy.

(table base-production
  (( i S m ) fuel ( 2 2 1 ))
  (N fuel 20)	; nuclear plant powers things
  (e ( fuel ammo ) ( 2 1 ))
  ;                L  &  _  V  T  @ "bases"
  (bases fuel   (  4  1 10 15 20 50 ))
  (bases ammo   (  0  1  5  8 10 20 ))
  (bases people (  0  0  0  2  4  8 ))
  )

(table productivity
  (i ( sea desert mountains ice ) 0)
  (i shallows 50)
  (a plains 100)	; in this case "plains" = "gas stations"
  ;         plains forest desert mountains
  (L land ( 100 90 70 50 ))	; one arg must be a scalar...
  (_ land ( 100 90 70 50 ))
  (V land ( 100 90 70 50 ))
  (T land ( 100 90 70 50 ))
  (@ land ( 100 90 70 50 ))
  ;; Cities in water have improved transport!
  (bases water 100)
  (m water 100)
  ;; Nuke power is independent of terrain!
  (N t* 100)
  (cities t* 100)
  )

(table base-consumption 
  (movers fuel 1)	; all consume 1 fuel, except...
  (L ammo 2)
  ;; Aircraft need lotsa fuel to stay aloft.
  (aircraft fuel 2)
  ;; Some types have negligible needs.
  ((r m) fuel 0)
  )

(table consumption-as-occupant
  (( r X S C B O ) m* 0)
  ;; (should include other types also?)
  )

; inlength is how far away a unit can receive fuel from
; out-length is how far away a unit can deliver fuel.  Should be kept
; small, as it can waste much CPU time

(table out-length
  (u* m* 0)
  (bases m* 4)
  (bases people 2)	;  they will look for nearby places to expand
  ;; Bases form good supply lines, so do transport units and big ships.
  (( A t j C B L & _ V T @ ) fuel ( 1 1 1 1 1 2 1 2 2 3 4 ))
  (( A t j C B L & _ V T @ ) ammo ( 1 1 1 1 1 2 1 2 2 3 4 ))
  )

(table in-length
  ;; All units can be resupplied by adjacent units.
  (u* fuel 1)
  (u* ammo 1)
  (u* people -1)	; People don't really want to migrate in war
  (bases people 4)	; except to cities, of course.
  (e m* 2)	;  They follow the engineers for jobs!
  (aircraft fuel 0)	; no mid-air refueling yet.  can't make it selective
  (bases fuel (2 2 3 4 4 4))
  (bases ammo (2 2 3 4 4 4))
  )

;;; Random events.

(table surrender-chance
  (V u* 6)
  (T u* 3)
  (@ u* 1)
  )

;( 60 50 30 ) ( V T @ ) siege
;FIXME 190 bases siege	; They have too much supply needs to withstand siege for long

(table attrition
  (s ( sea shallows ice ) ( 1 5 8 ))
  (t ( sea shallows ice ) ( 1 5 8 ))
  (( C B ) shallows 10)
  (O shallows 10)
  )

;FIXME "disabled at sea" ( s t ) attrition-message
;FIXME "runs aground" ( C B ) attrition-message
;FIXME "Gets stuck in mud" O attrition-message

(table accident-vanish-chance
  ;; flyers running into mountains
  (( A f b ) mountains ( 9 4 5 ))
  (m water 2)
  )

;FIXME spy-chance is now on a per unit basis...
; and spy-range specifies how far a unit can spy
(add u* spy-chance false)	; we don't spontaneously see all the enemies
;FIXME spy-quality is U1 U2 -> % that U1 returns info about U2
(table spy-quality (u* u* false))	; and we don't see any of them if we do!

;FIXME I guess nuked is damaged-terrain now, T1 T2 -> N
;FIXME t* t* nuked	; most terrain won't actually change
;FIXME desert ( plains forest ) nuked
;FIXME mountains ice nuked
;FIXME swamp shallows nuked

;;; Texts.

(set event-notices '(
  ((unit-died-in-accident A) (0 " crashes in the mountains!"))
  ((unit-died-in-accident f) (0 " crashes in the mountains!"))
  ((unit-died-in-accident b) (0 " crashes in the mountains!"))
  ((unit-died-in-accident m) (0 " drifts away"))
  ))

;;; Scoring.

(scorekeeper (do last-side-wins))

(add ( & _ V T @ ) point-value ( 100 200 800 2000 5000 ))	;  territory values for victory pts.

;;; Random setup.

(add t* alt-percentile-min (   0  68  69  70  70  70  93  97 ))
(add t* alt-percentile-max (  68  69  71  93  93  93  97 100 ))
(add t* wet-percentile-min (   0   0  50   0  20  80   0   0 ))
(add t* wet-percentile-max ( 100 100 100  20  80 100 100 100 ))

(set edge-terrain ice)

(set country-radius-min 8)
(set country-separation-min 15)
(set country-separation-max 30)

;; Set the number of starting units.

(add (T @) start-with (2 1))
(add cities independent-near-start (1 1 1))

(table independent-density (cities land (50 100 60)))

(table favored-terrain
  (u* t* 0)
  (cities land 20)
  (cities plains 40)
  ;; Engineers can go anywhere!
  (e t* 10)
  (@ plains 100)
  )

(add u* initial-seen-radius 7)

(include "nat-names")

(add cities namer "random-town-names")

(include "town-names")

(include "ng-features")

(set feature-types '(continents islands seas lakes bays
			    (desert 10)(forest 10)(mountains 5) peaks))

(set feature-namers
  '((islands generic-island-names) (lakes generic-lake-names) (bays generic-bay-names)
  	(seas generic-sea-names) (continents generic-continent-names)
  	(desert generic-desert-names) (forest generic-forest-names)
  	(mountains generic-mountain-names)))

; should add naming for C and B

;;; Documentation.

(game-module (instructions (
  "This is similar to the standard game, but includes more unit types"
  "and technological development is important.  You must choose the"
  "types you want and invest develop time to get to a level where"
  "you can actually build them."
  )))

; Hand this to a Lisp system, use for calculations...
; (defun res (res prod)
;	(setq res (- res (floor (T prod .2))))
;	(ceiling (T (_ res (floor (T prod 1.2))) 100)))

; (defun restime (prod res) (floor (_ (T (floor (T prod 1.2)) (+ res 100)) 100)))
