(game-module "wizard"
  (title "Wizard")
  (version "1.0")
  (blurb "When the gods were young, wizards dominated the world.")
  (variants
    (world-seen false)
    (see-all false)
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

(unit-type wagon (image-name "trident-settlers")
  (help "moves supplies"))
(unit-type ship (image-name "ancient-trireme")
  (help "transports units over water"))
(unit-type grog (image-name "heroes-dwarf1")
  (help "marches around and captures things"))
(unit-type knight (image-name "ancient-cavalry")
  (help "rides around and captures things better"))
(unit-type d (name "demon") (image-name "monsters-vampire")
  (help "kills and captures even better"))
(unit-type f (name "fire") (image-name "wiz-fire")
  (help "burns things up"))
(unit-type earth (image-name "wiz-earth")
  (help "very strong, hard to kill"))
(unit-type a (name "air") (image-name "wiz-air")
  (help "carries things through the sky"))
(unit-type c (name "crystal") (image-name "wiz-crystal")
  (help "sees very far"))
(unit-type manicon (image-name "wiz-manicon")
  (help "concentrated mana ball"))
(unit-type wizard (image-name "heroes-priest1") (char "W")
  (help "makes magic"))
(unit-type town (image-name "ancient-village1") (char "*")
  (help "produces food and grogs only"))
(unit-type city (image-name "ancient-blue-city") (char "@")
  (help "produces all things except mana"))
(unit-type guild (image-name "ancient-white-castle") (char "!")
  (help "produces mana and wizards"))
(unit-type outpost (image-name "ancient-tower") (char "$")
  (help "supplies mundanes"))

(material-type food
  (help "what people eat"))
(material-type drink
  (help "what people drink"))
(material-type mana
  (help "magic energy"))

(terrain-type sea (char "."))
(terrain-type shallows (char ","))
(terrain-type swamp (char "="))
(terrain-type desert (char "~"))
(terrain-type plains (char "+"))
(terrain-type forest (char "%"))
(terrain-type mountains (char "^"))
(terrain-type ice (char "_"))
(terrain-type vacuum (char ":"))

;FIXME I guess nuked is damaged-terrain now, T1 T2 -> N
;FIXME t* t* nuked	; most terrain won't actually change
;FIXME desert ( plains forest ) nuked
;FIXME mountains ice nuked

(define water ( sea shallows ))
(define land ( plains forest desert mountains ))
(define marsh swamp)

(define w wagon)
(define s ship)
(define g grog)
(define k knight)

(define e earth)

(define m manicon)

(define W wizard)
(define T town)
(define @ city)
(define ! guild)
(define $ outpost)

(define cities ( guild town city ))
(define places ( guild town city outpost ))
(define mundanes ( wagon ship grog knight ))
(define wizards ( wizard ))
(define ships ( ship ))
(define flyers ( a manicon ))
(define ground ( wagon grog knight d f earth c wizard ))
(define movers ( wagon ship grog knight d f earth a c manicon wizard ))
(define spells ( d f earth a c manicon ))
(define stupid ( wagon grog manicon town ))
(define smart ( knight ship d f earth a c wizard city guild ))
(define slow ( wagon grog c ))
(define swift ( knight d f earth a wizard ))

(add water liquid true)

;;; Static relationships.

(table vanishes-on
  ((wagon grog knight) water true)
  (ship land true)
  )

;; Unit-unit capacities.

(add movers capacity 1)
(add places capacity (5 10 20 5))
(add ship capacity 3)

(table unit-size-as-occupant
  (u* u* 99)
  ;; Wagons carry mundanes.
  ((grog knight) wagon 1)
  (movers ship 1)
  ;; Earth elemental can be used for transport.
  (movers earth 1)
  (earth earth 99)
  ;; Air elemental can carry a wizard.
  (wizards a 1)
  ;; Anybody can carry a crystal ball.
  (c u* 1)
  ;; Wizards may carry manicons.
  ;; (Note that wizards need not carry elementals, construction can
  ;; happen while stacked in same cell).
  (manicon wizards 1)
  ;; Places can hold lots of other unit types.
  (movers places 1)
  )

;; Unit-terrain capacities.

;; Limit units to 16 in one cell, for the sake of playability and
;; and drawability.  Places cover the entire cell, however.
  
(add t* capacity 16)

(table unit-size-in-terrain
  (u* t* 1)
  ;; Allow air elementals to pass over instead of entering.
  (a t* 0)
  (places t* 16)
  )

;; Unit-material capacities.

(table unit-storage-x
  (wizards mana 25)
  (guild mana 150)
  (spells mana 10)
  (m mana 25)
  (city food 150)
  (( town outpost ) food 40)
  (mundanes food 10)
  (wagon food 20)
  (mundanes drink 3)
  (wagon drink 5)
  (ships drink 4)
  (cities drink 10)
  (outpost drink 10)
  (guild food 10)
  )

;;; Vision.

;                      w s g k d f e a  c m W T @ ! $
(add u* vision-range ( 1 2 1 1 1 1 1 2 10 1 2 1 2 3 1))

;FIXME vision-at is N, coverage afforded by unit in its own hex
;(add u* vision-at-max-range ( 40 60 50 70 80 70 50 70 90 30 85 40 70 85 70 ))
;(add u* vision-at ( 1 2 1 1 1 1 1 2 10 1 2 1 2 3 ))

;FIXME visibility is U T -> N, U's % visibility in T
;(table visibility add (wizards t* 40))
;FIXME conceal is now visibility, you should subtr these numbers from that table
;FIXME 15 ( forest mountains ) u* conceal

(table spy-quality (u* u* 10))

;;; Actions.

;                          w s g k d f e  a c m W
(add movers acp-per-turn ( 1 6 1 2 3 4 1 10 3 8 3 ))

(add places acp-per-turn 1)

;;; Movement.

(add places speed 0)

(table mp-to-enter-terrain
  (slow land 1)
  (grog marsh 1)
  (e marsh 2)
  (swift ( forest mountains marsh ) 2)
  (swift ( plains desert ) 1)
  (ships land 99)
  (ships water 1)
  (ships shallows 3)
  (ships marsh 5)
  (outpost land 1)  ; ?
  (flyers t* 1)
  )

;(table mp-per-occupant add (ships earth 50))
;(table mp-per-occupant add (wizards ( d f earth ) 0))

;FIXME ferry-on-entry is U1 U2 -> FTYPE how much terrain U2 crosses to board U1
(table ferry-on-entry add (places ( grog knight d wizard ) over-all))	; infantry can capture cities even on water.

;;; Construction.

(add mundanes cp (7 20 4 15))
(add spells cp 4)
(add guild cp 12)
(add outpost cp 4)

(table can-create
  (wagon outpost 1)
  (wizards spells 1)
  (wizards guild 1)
  ((town city) mundanes 1)
  (guild wizards 1)
  )

(table can-build
  (wagon outpost 1)
  (wizards spells 1)
  (wizards guild 1)
  ((town city) mundanes 1)
  (guild wizards 1)
  )

(table acp-to-create
  (wagon outpost 1)
  (wizards spells 1)
  (wizards guild 1)
  ((town city) mundanes 1)
  (guild wizards 1)
  )

(table acp-to-build
  (wagon outpost 1)
  (wizards spells 1)
  (wizards guild 1)
  ((town city) mundanes 1)
  (guild wizards 1)
  )

(table cp-on-creation
  (city mundanes (3 8 1 6))
  )

(table consumption-per-built
  (spells mana 4)
  )

;; Set up an approximate 20% extra toolup time.

(table can-toolup-for
  (u* u* true)
  )

(table acp-to-toolup
  (u* u* 1)
  )

(table tp-to-build
  (wagon outpost 1)
  (wizards spells 1)
  (wizards guild 2)
  (town mundanes (1 4 1 3))
  (city mundanes (1 4 1 3))
  )

(table tp-max
  (wagon outpost 1)
  (wizards spells 1)
  (wizards guild 2)
  (town mundanes (1 4 1 3))
  (city mundanes (1 4 1 3))
  )

;; Everything requires significant develop time.

(add mundanes tech-max 50)
(add wizards tech-max 200)
(add spells tech-max 100)
(add guild tech-max 50)
(add outpost tech-max 50)

(add mundanes tech-to-build 50)
(add wizards tech-to-build 200)
(add spells tech-to-build 100)
(add guild tech-max 50)
(add outpost tech-to-build 50)

(table acp-to-develop
  (wagon outpost 1)
  (wizards spells 1)
  (wizards guild 1)
  ((town city) mundanes 1)
  (guild wizards 1)
  )

(table tech-per-develop
  (wagon outpost 1.00)
  (wizards spells 1.00)
  (wizards guild 1.00)
  ((town city) mundanes 1.00)
  (guild wizards 1.00)
  )

;; Repair.

(table can-repair
  (u* u* true)
  )

(table acp-to-repair
  (u* u* 1)
  )

(table hp-per-repair
  (u* u* 0.14)
  )

;; Wizards can heal themselves, but note that they don't have many hp.\
;; and thus shouldn't be engaging in combat personally.

(add wizards hp-recovery 0.20)

;;; Combat.

;                w s g k d f e a c m W T  @  ! $
(add u* hp-max ( 1 2 1 1 3 1 5 1 1 1 2 5 10 15 8 ))

;; Badly damaged movers move more slowly.

(add ship speed-damage-effect ((1 50) (2 100)))
(add d speed-damage-effect ((1 50) (2 100)))
(add earth speed-damage-effect ((2 50) (4 100)))
(add wizard speed-damage-effect ((1 50) (2 100)))

(table hit-chance
  ;        w  s  g  k  d  f  e  a  c  m  W  T  @  !  $
  (w u* ( 50 20 10 10  5  5  5  5 20 70 30 30 10  5 10 ))
  (s u* ( 40 50 30 20 15 20  5 20 30 70 30 35 25 10 25 ))
  (g u* ( 70 60 50 30 20 30 10 30 40 80 40 50 40 25 40 ))
  (k u* ( 80 70 70 50 35 50 20 50 50 85 50 65 50 35 50 ))
  (d u* ( 85 80 75 65 50 60 30 65 70 90 25 70 60 50 60 ))
  (f u* ( 90 90 85 70 60 70 40 70 85 80 35 75 70 50 70 ))
  (e u* ( 80 70 70 50 35 40 20 50 85 80 15 70 70 70 70 ))
  (a u* ( 50 80 50 40 35 40 20 50 45 75 20 40 35 30 35 ))
  (c u* ( 20 15 10  5  5  5  5  5 20 50 15 20 10  5 10 ))
  (m u* ( 90 90 80 70 65 70 60 75 80 70 65 90 80 60 80 ))
  (W u* ( 90 90 75 65 60 65 40 65 80 80 50 70 60 50 60 ))
  (T u* ( 50 20 10 10  5  5  5  5 20 70 30 30 10  5 10 ))
  (@ u* ( 70 60 50 30 20 30 10 30 40 80 40 50 40 25 40 ))
  (! u* ( 90 90 75 65 60 65 40 65 80 80 50 70 60 50 60 ))
  ($ u* ( 70 60 50 30 20 30 10 30 40 80 40 50 40 24 40 ))
  )

(table damage
  (u* u* 1)
  (d u* 2)
  (f u* 4)
  (f ( earth guild outpost ) 1)
  (e places 3)
  (m u* 8)
  (a ships 3)
  )

(table consumption-per-attack (m mana 20))

(add manicon acp-to-detonate 1)

(table capture-chance
  (grog places ( 50 30 10 30 ))
  (k places ( 70 50 15 50 ))
  (d places ( 70 50 25 50 ))
  (wizards spells 20)
  (wizard places ( 70 50 50 50 ))
  )

(table protection
  ;; Cities offer some protection to occupants.
  (town movers 80)
  (city movers 50)
  (guild movers 20)
  ;; Knights protect the cities housing them.
  (knight places 90)
  (wizard guild 90)	; can't make this too large or city can be
  (( d f earth ) guild 93)	; invulnerable.
  (grog places 95)	; same for infantry.
  (earth movers 50)
  )

;; Other Actions.

(add u* acp-to-change-side 0)	; but armies have some loyalty
(add ( c a manicon ) acp-to-change-side ( 80 65 40 ))
(add places acp-to-change-side 100)

;FIXME true u* neutral
;FIXME false spells neutral
;FIXME true ( c manicon ) neutral

; 10 u* max-quality

; 2 u* veteran

; ( 90 95 90 95 90 90 90 90 90 90 95 85 90 95 90 ) u* control

;FIXME hp-per-disband is U1 U2 -> HP lost in a disband action performed by U2
; you might add U1 acp-to-disband U2 as well, U1 U2 -> ACP
; (table hp-per-disband add (movers u* 100))

;;; Backdrop.

(table base-production
  (mundanes drink 2)
  (mundanes food 1)
  (wizards drink 1)
  (wizards food 1)
  (places food 4)
  (places drink 10)
  (guild mana 10)
  ((town city) food 10)
  )

(table productivity
  (places land 100)
  (town land ( 100 50 20 20 ))
  (city land ( 100 50 20 20 ))
  (ships ( shallows marsh ) 100)
  (mundanes land 100)
  (mundanes desert 20)
  )

(table base-consumption
  (mundanes food 1)
  (mundanes drink 1)
  (spells mana 1)
  (wizards mana 1)
  )

(table out-length
  (u* m* 1)
  (places m* 2)
  )

(table in-length
  (u* m* 1)
  (places m* 2)
  )

(table consumption-as-occupant
  (u* m* true)
  )

(table hp-per-starve
  (u* food 0.30)
  (wizards food 0.10)
  (u* drink 0.30)
  (wizards drink 0.10)
  (spells mana 0.30)
  (wizards mana 0.30)
  )

;                        w  s  g  k  d  f  e  a  c  m  W  T  @  !  $
(add u* revolt-chance ( 10 10 10  5 10  1  1  1  1  1  5 50 40 15 15 ))

;  ( 30 20 20 10 5  1  1  1  1  1  15 99 70 20 30 ) u* surrender
;  ( 30 20 20 10 1  1  1  1  1  1  20 99 99 15 20 ) u* siege

(table accident-vanish-chance
  (ships sea 300)
  (a t* 100)
  )

;; Text.

(set action-notices '(
  ((destroy u* movers) (actor " vanquishes " actee "!"))
  ((destroy u* ship) (actor " sinks " actee "!"))
  ((destroy u* cities) (actor " sacks " actee "!"))
  ))

(set event-notices '(
  ((unit-starved demon) (0 " returns to the Abyss"))
  ((unit-vanished ship) (0 " is lost in a storm"))
  ((unit-vanished a) (0 " disintegrates in a storm"))
  ))

(set event-narratives '(
  ((unit-starved demon) (0 " returned to the Abyss"))
  ((unit-vanished ship) (0 " was lost in a storm"))
  ((unit-vanished a) (0 " disintegrated in a storm"))
  ))

;;; Setup.

(add t* alt-percentile-min (   0  68  69  70  70  70  93  99  0 ))
(add t* alt-percentile-max (  68  69  71  93  93  93  99 100  0 ))
(add t* wet-percentile-min (   0   0  50   0  20  80   0   0  0 ))
(add t* wet-percentile-max ( 100 100 100  20  80 100 100 100  0 ))

(set edge-terrain ice)

(add city start-with 1)
(add guild start-with 1)
(add town start-with 5)
(add wizard start-with 1)

(table favored-terrain
  (u* t* 0)
  (town land 20)
  (town plains 40)
  (city plains 100)
  (guild land 20)
  (guild mountains 40)
  )

(set country-separation-min 10)
(set country-separation-max 30)

(table independent-density
  (town land 50)
  (town plains 150)
  (city land 20)
  (city plains 50)
  (guild land 20)
  (guild plains 50)
  )

;;; Scoring.

(add places point-value ( 1 5 10 1 ))

(scorekeeper (do last-side-wins))

;;; Naming

(include "ng-features")

(include "ng-weird")

(set default-namer "short-generic-names")

(add (town city guild outpost) namer "short-generic-names")

(set default-namer "short-generic-names")

(set feature-types '(continents islands seas lakes bays
			    (desert 10)(forest 10)(mountains 5)))

(set feature-namers
  '((islands generic-island-names) (lakes generic-lake-names) (bays generic-bay-names)
  	(seas generic-sea-names) (continents generic-continent-names)
  	(desert generic-desert-names) (forest generic-forest-names)
  	(mountains generic-mountain-names)))

;;; Documentation.

(game-module (instructions (
  )))

(game-module (notes (
  )))

(game-module (design-notes (
  )))
