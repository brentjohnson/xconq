(game-module "space"
  (title "Space Empires")
  (blurb "Colonize the local galactic neighbourhood.")
  (variants (world-seen true)(see-all false)(sequential false)(world-size (120 60 9999)))
  )

;;; Unit types.

(unit-type army (image-name "trooper")
  (help "military force, equipped with in-system ships"))
(unit-type engineers (image-name "builder")
  (help "construction team, equipped with in-system ships"))

;; Starships.

(unit-type transport (image-name "space-transport")
  (help "deep space transport"))
(unit-type fighter (image-name "space-fighter")
  (help ""))
(unit-type cruiser (image-name "space-cruiser")
  (help ""))

(define starships (transport fighter cruiser))

;; Other construction.

(unit-type starbase (image-name "space-station")
  (help "staging and refueling post in deep space"))
(unit-type shipyard (image-name "spaceyard") (char "S")
  (help "where starships get built"))

(define places (starbase shipyard))

;; Planets and planetoids.

(unit-type small-planet (name "small planet") (image-name "moon")
  (help "small airless planet"))
(unit-type earthlike-planet (name "planet") (image-name "planet")
  (help "medium-sized habitable planet"))
(unit-type gas-giant (name "gas giant")
  (help "large planet with no surface, can only orbit"))
(unit-type ringed-gas-giant (name "ringed gas giant")
  (help "large planet with rings but no surface, can only orbit"))
(unit-type asteroid-belt (name "asteroid belt")
  (help "a region of small planetoids, rich in metal"))
  
(define solid-planets (small-planet earthlike-planet asteroid-belt))

(define planets (small-planet earthlike-planet gas-giant ringed-gas-giant asteroid-belt))

;; Stars.
 
(unit-type yellow-star (name "yellow star")
  (description-format (name))
  (help "normal mainstream star"))
(unit-type red-giant (name "red giant")
  (description-format (name))
  (help "large reddish star, can have planets"))
(unit-type black-hole (name "black hole")
  (help "space-time anomaly, quite visible"))

(define stellar (yellow-star red-giant black-hole))

;;; Material types.

(material-type fuel
  (help "mixture of gases, powers everything"))
(material-type metal
  (help "used to build everything"))

;;; Terrain types.

(imf "off-black" ((1 1) 5000 5000 5000))
(imf "gray-black" ((1 1) 10000 10000 10000))

(terrain-type deep-space (name "deep space") (image-name "space")
  (help "interstellar space"))
(terrain-type nebula
  (help "area of gas, masks area behind"))
(terrain-type star-system (name "star system") (image-name "gray-black")
  (help "area around a star"))

;;; Static relationships.

;;; Unit-unit.

(table unit-size-as-occupant
  (u* u* 99)
  ((army engineers) transport 1)
  ((army engineers) planets 1)
  (starships places 1)
  (starships planets 1)
  (places planets 1)
  )

(add transport capacity 4)
(add places capacity 16)
(add planets capacity 32)
(add earthlike-planet capacity 64)

;;; Unit-terrain.

(table unit-size-in-terrain
  (engineers t* 0)
  ;; Any number of starships can share the same space.
  (starships t* 0)
  (planets t* 1)
  ;; Near-stellar regions are offlimits.
  (stellar t* 1)
  )

;;; Unit-material.

(table unit-storage-x
  ((army engineers) fuel 100)
  (starships fuel (200 18 400))
  (places fuel 500)
  (planets fuel (200 500 5000 5000 200))
  )

;;; Vision.

; (set terrain-seen true)

;; Stars are always obvious.

(add stellar see-always true)

(add u* can-see-behind false)

(add u* vision-range 5)
(add transport vision-range 3)
(add cruiser vision-range 7)
(add planets vision-range 10)

(add nebula thickness 100)

(table eye-height
  (u* t* 5)
  )

;;; Actions.

(add army acp-per-turn 1)
(add engineers acp-per-turn 1)
(add places acp-per-turn 1)
(add solid-planets acp-per-turn 1)

(add starships acp-per-turn (3 9 6))

;;; Movement.

(add places speed 0)
(add starbase speed 1)
(add planets speed 0)

(table mp-to-enter-terrain
  ;; Armies and engineers have in-system ships only; they can
  ;; survive if set adrift in deep space, but won't go anywhere.
  ((army engineers) (deep-space nebula) 99)
  )

;(table mp-to-leave-unit
;  ;; Starbases and shipyards don't have the engines to leave orbit?
;  (places planets 99)
;  )

;;; Construction.

(add (army engineers) cp (5 10))
(add starships cp (10 5 20))
(add places cp 20)

(table can-create
  (engineers places 1)
  (shipyard starships 1)
  (solid-planets (army engineers) 1)
  (asteroid-belt starships 1)
  )

(table can-build
  (engineers places 1)
  (shipyard starships 1)
  (solid-planets (army engineers) 1)
  (asteroid-belt starships 1)
  )

(table acp-to-create
  (engineers places 1)
  (shipyard starships 1)
  (solid-planets (army engineers) 1)
  (asteroid-belt starships 1)
  )

(table acp-to-build
  (engineers places 1)
  (shipyard starships 1)
  (solid-planets (army engineers) 1)
  (asteroid-belt starships 1)
  )

(table create-range
  (engineers places 1)
  )

(table build-range
  (engineers places 1)
  )

;;; Combat.

(add (army engineers) hp-max (9 3))
(add starships hp-max (4 1 8))
(add places hp-max 10)
(add planets hp-max 100)

(table hit-chance
  (u* u* 0)
  (army (army engineers) 50)
  (engineers (army engineers) 50)
  (army places 80)
  (army planets 100)
  (cruiser starships 50)
  )

(table damage
  (u* u* 0)
  (army planets 1)
  (army places 1)
  (cruiser starships 1)
  )

(table capture-chance
  (army places 50)
  (army planets 20)
  (army stellar 50)
  (army black-hole 0)
  (cruiser starships 20)
  )

(table independent-capture-chance
  (army places 100)
  (army planets 100)
  (army stellar 100)
  (army black-hole 0)
  (transport planets 100)
  (transport stellar 100)
  )

;; (should have protection by occupying armies)

;;; Backdrop.

(table base-consumption
  ((army engineers) fuel 1)
  (starships fuel 1)
  )

(table hp-per-starve
  ((army engineers) fuel 1.00)
  ;; Starships have a very short grace period.
  (starships fuel 0.50)
  )

;;; Random setup.

(add t* alt-percentile-min (  0   0  97))
(add t* alt-percentile-max ( 97  97 100))
(add t* wet-percentile-min (  0  94   0))
(add t* wet-percentile-max ( 94 100 100))

(set alt-blob-size 7)
(set alt-blob-height 1000)
(set alt-blob-density 100)
(set alt-smoothing 1)

(set country-radius-min 4)
(set country-separation-min 30)
(set country-separation-max 32)

(add shipyard start-with 1)
(add small-planet start-with 2)
(add earthlike-planet start-with 1)
(add gas-giant start-with 1)
(add asteroid-belt start-with 1)
(add yellow-star start-with 1)

(table favored-terrain
  (u* t* 0)
  (engineers star-system 100)
  (shipyard star-system 100)
  (planets star-system 100)
  (stellar star-system 100)
  ;; Black holes could actually have planets, but they would be
  ;; unusable, so ignore them.
  (black-hole star-system 0)
  (black-hole (deep-space nebula) 100)
  )

(table independent-density
  (planets star-system (1000 500 1000 200 500))
  (yellow-star star-system 1000)
  (red-giant star-system 200)
  (black-hole (deep-space nebula) 10)
  )

(table unit-initial-supply
  ;; All pre-existing units are fully fueled.
  (u* fuel 10000)
  )

(scorekeeper (do last-side-wins))

(set meridians false)

;;; Doctrine.

(doctrine default-doctrine
  (construction-run (u* 1))
  )

(doctrine place-doctrine
  (construction-run (u* 99)))

(doctrine engineers-doctrine
  (construction-run (u* 1)))

(side-defaults
  (default-doctrine default-doctrine)
  (doctrines (engineers engineers-doctrine) (places place-doctrine))
  )


;;; Naming.

(add planets namer "planet-names")
(add asteroid-belt namer "")

(add stellar namer "star-names")

(namer star-names (grammar root 30
  (root (or name 10 (letter " " constellation)))
  (name (or
    "Achernar" "Adhara" "Aldebaran" "Algol" "Altair" "Antares" "Arcturus"
    "Betelgeuse"
    "Canopus" "Castor" "Capella"
    "Deneb"
    "Hadar"
    "Mimosa" "Miraz"
    "Polaris" "Pollux" "Procyon"
    "Regulus" "Rigel"
    "Sirius" "Spica"
    "Vega"
    ))
  (letter (or "Alpha" "Beta" "Gamma" "Delta" "Epsilon" "Zeta" "Eta" "Theta"
              "Iota" "Kappa" "Lambda"))
  (constellation (or ; Note that these are in the genitive form
    "Andromedae" "Antliae" "Apodis" "Aquarii" "Aquilae" "Arae"
    "Arietis" "Aurigae"
    "Bootis"
    "Caeli" "Cameleopardalis" "Cancri" "Capricorni" "Centauri" "Cephei"
    "Ceti" "Corvi" "Crucis" "Cygni"
    "Delphini" "Doradus" "Draconis"
    "Eridani"
    "Fornacis"
    "Geminorum"
    "Herculis" "Hydrae"
    "Indi"
    "Lacertae" "Leonis" "Librae" "Lupi" "Lyncis" "Lyrae"
    "Monocerotis" "Muscae"
    "Ophiuchi" "Orionis"
    "Pavonis" "Pegasi" "Phoenicis" "Piscium" "Pyxidis"
    "Reticuli"
    "Sagittae" "Sagitarii" "Scorpii"
    "Tauri" "Tucana"
    "Ursa Major" "Ursa Minor"
    "Velorum" "Virginis" "Volantis" "Vulpecula"
    ))
  ))

(namer planet-names (grammar root 30
  (root (or 10 (generic-names)
	    90 (short-generic-names)
	))
  (foo "")
  ))

(include "ng-weird")

(game-module (design-notes (
  "Capture of stars means to claim ownership and install sensors orbiting"
  "them."
  )))
