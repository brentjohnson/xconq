(game-module "flattop"
  (title "Flattop Battles")
  (version "1.0")
  (blurb "WW II in the Pacific aboard a carrier.")
  (variants 
    (see-all false)
    (world-seen true)
    (sequential false)
  )
)

(unit-type f (name "fighter") (image-name "1e")
  (help "protects ships from aircraft"))
(unit-type d (name "dive bomber") (image-name "1e")
  (help "attacks ships and shore installations by bombing at close range"))
(unit-type t (name "torpedo bomber") (image-name "torpedo-bomber")
  (help "carries deadly torpedos"))
(unit-type pby (name "PBY") (char "p")
  (help "long-range scout plane"))
(unit-type dd (name "destroyer") (char "D")
  (help "small ship that protects against subs"))
(unit-type sub (name "submarine") (char "S")
  (help ""))
(unit-type cve (name "escort carrier") (char "E") (image-name "cv")
  (help "small aircraft carrier"))
(unit-type cl (name "light cruiser") (char "L")
  (help "bigger than a destroyer, smaller than a heavy cruiser"))
(unit-type ca (name "heavy cruiser") (char "R")
  (help ""))
(unit-type cv (name "carrier") (char "C")
  (help "mobile airfield for aircraft"))
(unit-type bb (name "battleship") (char "B")
  (help ""))
(unit-type B (name "base") (image-name "airbase")
  (help ""))

(material-type fuel (char "o") (help "how to get around"))
(material-type ammo (char "a") (help "small stuff"))
(material-type shell (char "A") (help "8 to 16 inch"))
(material-type bomb (char "b") (help "iron bombs"))
(material-type torp (char "!") (help "death to ships"))

(define o fuel)
(define a ammo)
(define A shell)
(define b bomb)
(define ! torp)

(terrain-type sea (char "."))
(terrain-type atoll (char ","))
(terrain-type island (image-name "forest") (char "+"))

(define combat-air (f d t))
(define air (f d t pby))
(define heavy (cl ca bb))
(define carriers (cve cv))
(define ships (dd sub cve cl ca cv bb))

(add (sea atoll) liquid true)

;;; Static relationships.

(table unit-size-as-occupant
  (u* u* 99)
  (air carriers 1)
  (air B 4)
  (ships B 1)
  )

(add air capacity 0)
(add carriers capacity (4 8))
(add B capacity 32)

(add t* capacity 4)

(table unit-size-in-terrain
  ;; Any number of aircraft can share the same airspace.
  (air t* 0)
  ;; Up to 4 ships (irrespective of size) in a cell.
  (ships t* 1)
  ;; Base takes up entire cell.
  (base t* 4)
  )

(table unit-storage-x
  (ships (fuel ammo) 500)
  (air fuel 24)
  (pby fuel 48)
  (air ammo 2)
  (pby ammo 0)
  (d b 2)
  (t torp 2)
  ((dd sub) torp 12)
  (cve (b torp) 25)
  (cv (b torp) 50)
  (heavy shell 50)
  (B m* 1000)
  )

(table base-consumption
  (air fuel 12)
  (pby fuel  6)
  (ships fuel 1)
  )

(table hp-per-starve
  ;; Aircraft splash immediately if they run out of fuel.
  (air fuel 1.00)
  ;; Ships have a short grace period.
  (ships fuel 0.10)
  )

(table consumption-as-occupant
  ;; Aircraft on the ground or in a carrier just sit there.
  (air fuel 0)
  )

;;; Vision.

(table see-chance-at
  (u* sub 50)
  )

(table see-chance-adjacent
  (u* sub 10)
  (dd sub 20)
  )

;;; Actions.

(add air acp-per-turn 12)
(add pby acp-per-turn 6)
(add ships acp-per-turn 3)

(add B acp-per-turn 0)

;;; Movement.

(add B speed 0)

(add air free-mp 12)
(add pby free-mp 6)

(table mp-to-enter-terrain
  (ships atoll 2)
  (ships island 10)
  )

(table mp-to-enter-unit
  ;; Aircraft cannot sortie again until next turn.
  (air ships 12)
  (pby ships 6)
  )

(table zoc-range
  ;; PBYs can never block anyone.
  (u* pby -1)
  (pby u* -1)
  ;; Nor can subs.
  (u* sub -1)
  (sub u* -1)
  )

;; Everybody needs fuel.

(table material-to-move
  (u* fuel 1)
  )

(table consumption-per-move
  (u* fuel 1)
  )

;;; Combat.

(add ships hp-max (5 5 5 5 10 10 20))
(add B hp-max 100)

(table acp-to-attack
  (air u* 3)
  )

(table hit-chance
  (u* u* 50)
  (f air 80)
  (f f 60)
  ;; PBY's SOP generally keeps them out of reach of all except fighters.
  (u* pby 10)
  (f pby 100)
  ;; ... because they have no combat capability.
  (pby u* 0)
  (f carriers 20)
  ;; Subs have almost no anti-air capability.
  (sub air 10)
  (dd sub 70)
  ;; Carriers' own combat capability is minimal.
  (carriers u* 10)
  ;; Only combat aircraft and heavy ships can hit the base.
  (u* B 0)
  ;; ...and they always hit.
  (combat-air B 100)
  (heavy B 100)
  ;; ...except that torpedo bombers aren't equipped for it.
  (t B 0)
  )

(table damage
  (u* u* 1)
  (heavy u* 1d2+1)
  ((t dd sub) ships 1d4)
  ((t dd sub) (cv bb) 1d5+5)
  (bb B 1d10+4)
  (pby u* 0)
  (t B 0)
  )

(table consumption-per-attack
  (air ammo 1)
  (carriers ammo 1)
  (d bomb 1)
  ((t dd sub) torp 1)
  (heavy shell 1)
  )

(table hit-by
  (ships (bomb torp) 1)
  (air ammo 1)
  (u* shell 1)
  )

;50 air carriers protect

(set action-notices '(
  ((destroy u* air) (actor " shoots down " actee "!"))
  ((destroy u* ships) (actor " sinks " actee "!"))
  ((destroy u* B) (actor " demolishes " actee "!"))
  ))

;; 0 air control

;;; Random setup.

(add t* alt-percentile-min (  0  96  98))
(add t* alt-percentile-max ( 96  98 100))
(add t* wet-percentile-min 0)
(add t* wet-percentile-max 100)

(set edge-terrain sea)

(set country-radius-min 4)
(set country-separation-min 30)
(set country-separation-max 32)

(add u* start-with (12 12 10 6 6 2 4 2 2 2 2 1))

(table favored-terrain
  (u* t* 0)
  (ships sea 100)
  (B (atoll island) 100)
  )

(set advantage-min 1)
(set advantage-default 2)
(set advantage-max 8)

;;; Everybody starts out full.

(table unit-initial-supply (u* m* 9999))

(set terrain-seen true)
(add B already-seen 100)

;; Note that there are no default namers for unit types, so any type
;; not mentioned in the side's own namers will simply not be named.

(set side-library '(
  ((noun "American") (emblem-name "flag-usa")
   (unit-namers
    (cl  "us-cl-names")
    (ca  "us-ca-names")
    (cve "us-cve-names")
    (cv  "us-cv-names")
    (bb  "us-bb-names"))
   )
  ((noun "Japanese") (emblem-name "flag-japan") ; naval flag doesn't scale well
   (unit-namers
    (ca  "japanese-ca-names")
    (cve "japanese-cve-names")
    (cv  "japanese-cv-names")
    (bb  "japanese-bb-names"))
   )
  ))

(include "ng-ships")

;; Also worthwhile to have generator for names like "LC-1", "CA6", etc?

(scorekeeper (do last-side-wins))

(game-module (notes (
  "This is a somewhat expanded version of the navy in the standard game. "
  "Each side commands a large fleet, and the sole objective is to wipe out "
  "the enemy's fleet.  There are different kinds of planes and ships, but "
  "nothing else."

  "Speeds of ships are uniform, so that formations work better (don't want "
  "the carriers outstripping their escorts). "
  )))
