(game-module "ww2-div-pac"
  (title "WWII Division Level Sea")
  (version "1.0")
  (blurb "Base module for division level sea war. Emphasis on the Pacific.")
  (variants
   (world-seen true)
   (see-all true)
   (sequential false)
   ("Last Side Wins" keep-score true (true (scorekeeper (do last-side-wins))))
   )
  )

(unit-type a-inf-bn (image-name "inf-bn")
  (help "Allied infantry battalion"))
(unit-type a-inf-reg (image-name "inf-reg")
  (help "Allied infantry regiment"))
(unit-type a-inf-bde (image-name "inf-bde")
  (help "Allied infantry brigade"))
(unit-type a-inf-div (image-name "inf-div")
  (help "Allied infantry division"))
(unit-type a-inf-corps (image-name "inf-corps")
  (help "Allied infantry corps"))

(unit-type c-inf-army (image-name "inf-army")
  (help "Chinese infantry army"))

(define a-inf-types (a-inf-bn a-inf-reg a-inf-bde a-inf-div a-inf-corps c-inf-army))

(unit-type a-marine-bn (image-name "marine-bn")
  (help "Allied marine battalion"))
(unit-type a-marine-reg (image-name "marine-reg")
  (help "Allied marine regiment"))
(unit-type a-marine-bde (image-name "marine-bde")
  (help "Allied marine brigade"))
(unit-type a-marine-div (image-name "marine-div")
  (help "Allied marine division"))

(define a-marine-types (a-marine-bn a-marine-reg a-marine-bde a-marine-div))

(unit-type a-para-div (image-name "para-div")
  (help "Allied airborne division"))

(unit-type a-spec-reg (image-name "spec-reg")
  (help "Allied special forces regiment"))

(unit-type a-armor-bde (image-name "armor-bde")
  (help "Allied armor brigade"))

(unit-type a-engr-reg (image-name "engr-reg")
  (help "Allied engineer regiment"))

(define a-other-types (a-para-div a-spec-reg a-armor-bde a-engr-reg))

(define a-ground-types (append a-inf-types a-marine-types a-other-types))

(add a-ground-types possible-sides "allied")

(add c-inf-army possible-sides "chinese")

(unit-type j-inf-bn (image-name "inf-bn")
  (help "Japanese infantry battalion"))
(unit-type j-inf-reg (image-name "inf-reg")
  (help "Japanese infantry regiment"))
(unit-type j-inf-bde (image-name "inf-bde")
  (help "Japanese infantry brigade"))
(unit-type j-inf-div (image-name "inf-div")
  (help "Japanese infantry division"))

(define j-inf-types (j-inf-bn j-inf-reg j-inf-bde j-inf-div))

(unit-type j-marine-bn (image-name "marine-bn")
  (help "Japanese marine battalion"))

(unit-type j-para-bde (image-name "para-bde")
  (help "Japanese airborne brigade"))

(unit-type j-armor-bde (image-name "armor-bde")
  (help "Japanese armor brigade"))

(unit-type j-engr-reg (image-name "engr-reg")
  (help "Japanese engineer regiment"))

(define j-other-types (j-marine-bn j-para-bde j-armor-bde j-engr-reg))

(define j-ground-types (append j-inf-types j-other-types))

(add j-ground-types possible-sides "japanese")

(unit-type hq (image-name "flag"))

(define ground-types (append a-ground-types j-ground-types hq))

(unit-type apd (image-name "ap")
  (help ""))
(unit-type aa (image-name "ap")
  (help "amphibious assault ship"))
(unit-type de (image-name "dd")  ; (should have a distinct DE picture?)
  (help "destroyer escort"))
(unit-type destroyer (short-name "DD") (image-name "dd")
  (help "small group of destroyers"))
(unit-type submarine (short-name "SS") (image-name "sub")
  (help "submarines are deadly to surface ships"))
(unit-type |light cruiser| (short-name "CL") (image-name "cl")
  (help "bigger than a destroyer, smaller than a heavy cruiser"))
(unit-type cruiser (short-name "CA") (image-name "ca")
  (help "heavy cruiser"))
(unit-type |escort carrier| (short-name "CVE") (image-name "cv")
  (help "half-size aircraft carrier"))
(unit-type cvl (short-name "CVL") (image-name "cv")
  (help "light carrier"))
(unit-type cvs (short-name "CVS") (image-name "cv")
  (help "seaplane carrier"))
(unit-type carrier (short-name "CV") (image-name "cv")
  (help "aircraft carrier"))
(unit-type battlecruiser (short-name "BC") (image-name "bb")
  (help "a lightly-armored battleship"))
(unit-type battleship (short-name "BB") (image-name "bb")
  (help "the most powerful ship"))

(define dd destroyer)
(define ss submarine)
(define cl |light cruiser|)
(define ca cruiser)
(define cve |escort carrier|)
(define cv carrier)
(define bc battlecruiser)
(define bb battleship)

(define transport-types (apd aa))

(define small-ship-types (apd aa de dd ss))

(define large-ship-types (cl ca cve))

(define capital-ship-types (cvl cvs cv bc bb))

(define carrier-types (cve cvl cvs cv))

(define ship-types (append small-ship-types large-ship-types capital-ship-types))

(unit-type airfield (image-name "airbase"))
(unit-type airbase)
(unit-type port (image-name "anchor"))
(unit-type town (image-name "town20"))
(unit-type city (image-name "city20"))

(define place-types (airfield airbase port town city))

(material-type org
  (help "important to do things, acquired from HQs"))
(material-type air
  (help "attached aircraft used for support"))

(terrain-type sea
  (help "open water"))
(terrain-type atoll
  (help "group of small coral islands, surrounded by lagoon"))
(terrain-type clear (image-name "plains"))
(terrain-type jungle (image-name "forest"))
(terrain-type hills)
(terrain-type mountains)
(terrain-type road
  (subtype connection))
(terrain-type river
  (subtype border))
(terrain-type reef (image-name "gray")
  (subtype border)
  (help "a band of coral reefs, usually between land and ocean"))

(define water (sea atoll))
(define land (clear jungle mountains))

(define cell-t* (sea atoll clear jungle hills mountains))

;; Atolls don't have obvious shorelines, so call them liquid.

(add water liquid true)

;;; Static relationships.

(table vanishes-on
  (ground-types sea true)
  (ship-types land true)
  (place-types sea true)
  ((town city) atoll true)
  )

(add sea capacity 100)

(add land capacity 4)

(add transport-types capacity 12)

(add (port town city) capacity (8 8 32))

(table unit-size-as-occupant
  (u* transport-types 99)
  (u* place-types 99)
  ;; (should account for varying size of ground units)
  (ground-types transport-types 1)
  (ground-types (town city) 1)
  (ship-types (port town city) 1)
  )

(table unit-storage-x
  (u* org 1)
  ;; Aircraft complements for various types of carriers.
  (carrier-types air (24 36 12 72))
  (place-types air (72 144 0 288 576))
  )

;;; Vision.

(set terrain-seen true)

;; Other sides will only know about a handful of military units.
(add u* already-seen 5)
;; The biggest ships are harder to hide.
(add capital-ship-types already-seen 10)
;; But there's no hiding of permanent installations.
(add place-types already-seen 100)

;; Range should depend on availability of scout aircraft...

(add carrier-types vision-range 4)

(add place-types vision-range 4)

;;; Action parameters.

(add ground-types acp-per-turn 1)
(add ship-types acp-per-turn 24)
(add place-types acp-per-turn 24)

(add carrier-types free-acp 6)
(add place-types free-acp 6)

;;; Movement.

(add place-types speed 0)

(table mp-to-enter-terrain
  (place-types t* 99)
  (ground-types sea 99)
  (ship-types land 99)
  (ship-types atoll 12)
  )

(table mp-to-leave-terrain
  (ship-types atoll 12)
  )

;;; Combat.

;; For ground units, 1 battalion is approximately 1 hp.

(add a-inf-types hp-max (1 3 6 12 12 12))
(add a-marine-types hp-max (1 3 6 12))
(add a-other-types hp-max (12 3 6 3))
(add j-inf-types hp-max (1 3 6 12))
(add j-other-types hp-max (1 6 6 3))

#|
(add a-inf-types parts-max (1 3 6 12 12 12))
(add a-marine-types parts-max (1 3 6 12))
(add a-other-types parts-max (12 3 6 3))
(add j-inf-types parts-max (1 3 6 12))
(add j-other-types parts-max (1 6 6 3))
|#

;apd aa de dd ss cl ca cve cvl cvs cv bc bb

(add ship-types hp-max    (6 6 6 6 6 4 6 4 4 4 5 5 6))
#|
(add ship-types parts-max (6 6 6 6 6 2 2 2 1 1 1 1 1))
|#

(add place-types hp-max (10 20 20 40 80))

(table acp-to-attack
  (ship-types u* 6)
  (carrier-types u* 0)
  (place-types u* 0)
  )

(table acp-to-defend
  (u* ship-types 6)
  )

(add carrier-types acp-to-fire 12)
(add place-types acp-to-fire 12)

(table acp-to-be-fired-on
  ;; Defense consumes a good bit of time.
  (carrier-types carrier-types 6)
  (carrier-types place-types 6)
  (place-types carrier-types 6)
  (place-types place-types 6)
  )

(add carrier-types range 8)
(add place-types range 8)

(table hit-chance
  ;; Ground combat.
  (ground-types ground-types 50)
  (ground-types place-types 50)
  ;; Naval combat.
  (ship-types ship-types 50)
  ;; Air combat.
  (carrier-types ground-types 50)
  (carrier-types ship-types 50)		; 	This is redundant with ship-types ship-types 50, but
  (carrier-types place-types 50)		; 	we spell it out for clarity.
  (place-types ground-types 50)
  (place-types ship-types 50)
  (place-types place-types 50)
  )

(table damage
  ;; Ground combat.
  (ground-types ground-types 1)
  (ground-types place-types 1)
  ;; Naval combat.
  (ship-types ship-types 1)
  ;; Air combat.
  (carrier-types ground-types 1)
  (carrier-types ship-types 1)		; 	This is redundant with ship-types ship-types 1, but
  (carrier-types place-types 1)		; 	we spell it out for clarity.
  (place-types ground-types 1)
  (place-types ship-types 1)
  (place-types place-types 1)
  )

(table consumption-per-fire
  ;; Expect to lose one plane in an attack.
  (carrier-types air 1)
  (place-types air 1)
  )

(table hit-by
  ;; Planes can attack just about anything.
  (u* air 1)
  )

(table material-to-fire
  ;; Carrier needs at least a few planes.
  (carrier-types air 6)
  (place-types air 6)
  )

(table capture-chance
  (ground-types airfield 100)
  (ground-types (airbase port) 90)
  (ground-types (town city) 50)
  )

;;; Text.

(set action-notices '(
  ((disband ground-types self done) (actor " goes home"))
  ((disband ship-types self done) (actor " is scuttled"))
  ((disband u* u* done) (actee " disbands"))
  ((destroy u* ground-types) (actor " defeats " actee "!"))
  ((destroy u* ship-types) (actor " sinks " actee "!"))
  ((destroy u* place-types) (actor " flattens " actee "!"))
  ))

;;; Defaults for testing.

;;; Some defns for the fractal percentile generator.

(set alt-blob-density 10000)
(set alt-blob-size 20)
(set alt-smoothing 6)
(set wet-blob-density 2000)
(set wet-blob-size 100)

(add cell-t* alt-percentile-min (  0  90  90  90  97  98))
(add cell-t* alt-percentile-max ( 90  91  97  97  98 100))
(add cell-t* wet-percentile-min (  0   0   0  50   0   0))
(add cell-t* wet-percentile-max (100 100  50 100 100 100))

;;; River generation.

(add (clear jungle hills mountains) river-chance (10.00 25.00 25.00 25.00))

(set river-sink-terrain sea)

;(table terrain-interaction (river (sea shallows) non-terrain))

(set edge-terrain sea)

(add u* start-with 1)  ; one of everything

(set country-radius-min 4)
(set country-separation-min 20)
(set country-separation-max 60)

(add (sea clear) country-terrain-min (10 5))

(table favored-terrain
  (u* t* 0)
  (ground-types land 100)
  (ship-types sea 100)
  (place-types land 100)
  )

(table unit-initial-supply
  (u* m* 9999)
  (place-types air 100)
  )

;;; Sides.

(side 1 (name "Allies") (adjective "Allied") (emblem-name "flag-usa")
   (class "allied") (names-locked true)
   (unit-namers
    (cl "us-cl-names")
    (ca "us-ca-names")
    (cve "us-cve-names")
    (cv "us-cv-names")
    (bb "us-bb-names"))
   )

(side 2 (name "Japan") (adjective "Japanese") (emblem-name "flag-japan")
   (class "japanese") (names-locked true)
   (unit-namers
    (ca "japanese-ca-names")
    (cve "japanese-cve-names")
    (cv "japanese-cv-names")
    (bb "japanese-bb-names"))
   )

(side 3 (noun "Chinese") (adjective "Chinese") (emblem-name "flag-china")
   (class "chinese") (names-locked true))

(include "ng-ships")

(set calendar '(usual day))

(world 800 (year-length 365))

(game-module (design-notes (
  "The map-scale is 30 miles/hex, game time is 1 day/turn."
  "One unit of aircraft material is one plane."
  ""
  "To Do:"
  ""
  "Add fuel as material, scenarios could predefine certain places to have lots."
  )))
