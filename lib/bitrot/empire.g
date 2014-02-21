(game-module "empire"
  (title "Empire")
  (version "0.5")
  (blurb "Emulation of Empire, the classic big economic-military game.")
  (variants
   (world-seen false)
   (see-all false)
   (sequential false)
   ("Only One Winner" one-winner true
     (true
	  (scorekeeper (do last-side-wins))
	  ))
   ("More Starters" more-starters true
     (true
      (add harbor start-with 1)
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

(unit-type infantry (image-name "soldiers"))
(unit-type motor-infantry (image-name "truck"))
(unit-type mech-infantry (image-name "halftrack"))
(unit-type marines (image-name "elite"))
(unit-type security (image-name "police"))
(unit-type cavalry (image-name "cavalry"))
(unit-type light-armor (image-name "maultier"))
(unit-type armor (image-name "pz-4"))
(unit-type heavy-armor (image-name "tiger-1"))
(unit-type artillery (image-name "arty"))
(unit-type light-artillery (image-name "field-gun"))
(unit-type heavy-artillery (image-name "arty"))
(unit-type mech-artillery (image-name "arty"))
(unit-type aa (image-name "flak"))
(unit-type supply (image-name "cart"))
(unit-type engineers (image-name "worker"))
(unit-type mech-engineers (image-name "tractor"))
(unit-type mobile-radar (image-name "radar"))

(define infantry-types (infantry motor-infantry mech-infantry marines security))

(define armor-types (light-armor armor heavy-armor))

(define artillery-types (artillery light-artillery heavy-artillery mech-artillery aa))

(define engineer-types (engineers mech-engineers))

(define land-types
  (append infantry-types cavalry armor-types artillery-types
   supply engineers mech-engineers mobile-radar
   ))

(add infantry-types acp-per-turn (2 3 3 2 2))
(add armor-types acp-per-turn (3 3 3))
(add artillery-types acp-per-turn (2 2 2 3 2))
(add engineer-types acp-per-turn (2 3))
(add mobile-radar acp-per-turn 2)

(unit-type fishing-boat (image-name "ap"))

(unit-type cargo-ship (image-name "ap"))
(unit-type tanker (image-name "ap"))
(unit-type ore-ship (image-name "ap"))
(unit-type slave-ship (image-name "ap"))
(unit-type troop-transport (image-name "ap"))
(unit-type landing-craft (image-name "ap"))

(unit-type torpedo-boat (image-name "dd"))
(unit-type destroyer (image-name "dd"))
(unit-type frigate (image-name "dd"))
(unit-type light-cruiser (image-name "cl"))
(unit-type heavy-cruiser (image-name "ca"))
(unit-type battleship (image-name "bb"))

(unit-type light-carrier (image-name "cv"))
(unit-type escort-carrier (image-name "cv"))
(unit-type carrier (image-name "cv"))
(unit-type missile-frigate (image-name "dd"))
(unit-type missile-cruiser (image-name "ca"))

(unit-type submarine (image-name "sub"))
(unit-type asw-cruiser (image-name "ca"))
(unit-type minesweeper (image-name "ap"))

(define production-ship-types (fishing-boat))

(define material-transport-ship-types (cargo-ship tanker ore-ship slave-ship))

(define transport-ship-types (troop-transport landing-craft))

(define surface-combat-ship-types
  (torpedo-boat destroyer frigate light-cruiser heavy-cruiser battleship))

(define carrier-ship-types
  (light-carrier escort-carrier carrier))

(define air-combat-ship-types
  (light-carrier escort-carrier carrier missile-frigate missile-cruiser))

(define ship-types
  (append production-ship-types material-transport-ship-types
   transport-ship-types surface-combat-ship-types air-combat-ship-types
   submarine asw-cruiser minesweeper
   ))

(add ship-types acp-per-turn 4)
; are these correct for empire?
;(add material-transport-ship-types acp-per-turn 3)
;(add transport-ship-types acp-per-turn (4 2))
;(add surface-combat-ship-types acp-per-turn (4 4 4 3 4 3))
;(add air-combat-ship-types acp-per-turn 4)
;(add submarine acp-per-turn 3)
;(add asw-cruiser acp-per-turn 4)
;(add minesweeper acp-per-turn 3)

(unit-type fighter-1 (image-name "fighter"))
(unit-type fighter-2 (image-name "fighter"))
(unit-type escort (image-name "fighter"))
(unit-type naval-fighter (image-name "fighter"))
(unit-type light-bomber (image-name "4e"))
(unit-type medium-bomber (image-name "4e"))
(unit-type heavy-bomber (image-name "4e"))
(unit-type airlifter (image-name "airlifter"))
(unit-type attack-helicopter (image-name "helicopter"))
(unit-type transport-helicopter (image-name "cargo-chopper"))
(unit-type recon-plane (image-name "sr71"))
(unit-type asw-plane (image-name "4e"))
(unit-type missile (image-name "missile"))
(unit-type icbm (image-name "icbm"))
(unit-type sam (image-name "sam"))
(unit-type abm (image-name "missile-2"))

(define fighter-types (fighter-1 fighter-2 escort naval-fighter))

(define bomber-types (light-bomber medium-bomber heavy-bomber))

(define helicopter-types (attack-helicopter transport-helicopter))

(define missile-types (missile icbm sam abm))

(define plane-types
  (append fighter-types
   bomber-types
   airlifter
   helicopter-types
   recon-plane asw-plane
   missile-types
   ))

;; These are twice the plane's "range" in Empire.

(add plane-types acp-per-turn 16)

;(add helicopter-types acp-per-turn (12 10))
;(add transport acp-per-turn 14)
;(add recon-plane acp-per-turn 18)
;(add asw-plane acp-per-turn 16)

(add bomber-types acp-per-turn (24 48 60))

;; But missiles are one-way, so acp is comparable.

(add missile-types acp-per-turn (8 40 8 20))

(unit-type small-nuke (image-name "bomb"))
(unit-type medium-nuke (image-name "bomb"))
(unit-type large-nuke (image-name "bomb"))

(define nuke-types (small-nuke medium-nuke large-nuke))

(add nuke-types acp-per-turn 1)

;; The remaining types are actually "sector types" in Empire, but
;; they work better as cell-filling unit types in Xconq.
;; Bridges can be highways over water, not needed as units.

(unit-type radar (image-name "radar") (char ")")
  (help "fixed radar station - has a long-range view"))
(unit-type mines (image-name "minefield")
  (help "useful for blocking land and sea passages"))

(define inert-types (radar mines))

(add inert-types acp-per-turn 0)

(unit-type agribusiness (image-name "barn") (char "a"))
(unit-type oil-field (image-name "oil-derrick") (char "o"))
(unit-type oil-platform (image-name "oil-derrick"))
(unit-type mine (image-name "iron-mine") (char "m"))
(unit-type gold-mine (image-name "gold-mine") (char "g"))
(unit-type uranium-mine (image-name "uranium-mine") (char "u"))

(unit-type technical-center (image-name "facility") (char "t"))
(unit-type fortress  (image-name "fort") (char "f"))
(unit-type research-lab (image-name "facility") (char "r"))
(unit-type nuclear-plant (image-name "facility") (char "n"))
(unit-type library/school (image-name "facility") (char "l"))
(unit-type park (image-name "facility") (char "p"))
(unit-type enlistment (image-name "facility") (char "e"))
(unit-type headquarters (image-name "corps-hq") (char "!"))

(unit-type harbor (image-name "anchor") (char "h"))
(unit-type airfield (image-name "airbase") (char "*"))
(unit-type refinery (image-name "refinery") (char "%"))
(unit-type lcm-factory (image-name "facility") (char "j"))
(unit-type hcm-factory (image-name "facility") (char "k"))
(unit-type defense-plant (image-name "facility") (char "d"))
(unit-type shell-industry (image-name "shell") (char "i"))
(unit-type warehouse (image-name "facility") (char "w"))

(unit-type bank (image-name "bank") (char "b"))

(define facility-types
  (agribusiness oil-field oil-platform mine gold-mine uranium-mine
   bank
   technical-center fortress research-lab
   nuclear-plant
   library/school park enlistment
   headquarters harbor airfield
   refinery lcm-factory hcm-factory defense-plant shell-industry warehouse))

;;; Most facilities only produce or store materials, have no use for acp.

(add facility-types acp-per-turn 0)

;;; These build units, however, and need acp to do so.

(add (nuclear-plant headquarters harbor airfield) acp-per-turn 1)

(add (refinery) acp-per-turn 1)

;;; Fortresses can engage in combat.

(add fortress acp-per-turn 1)

(unit-type capital (image-name "city20") (char "c")
  (help "center of the country"))

(add capital acp-per-turn 3)

(material-type food (char "f")
  (help "food"))
(material-type sh (char "s")
  (help "shells - ammunition for guns"))
(material-type gun (char "g")
  (help "guns"))
(material-type pet (char "p")
  (help "petroleum - refined oil, used as fuel"))
(material-type iron (char "i")
  (help "iron ore"))
(material-type dust (char "d")
  (help "gold dust - as mined from the ground"))
(material-type gold (char "b")
  (help "gold - refined gold, in bars"))
(material-type oil (char "o")
  (help "crude oil"))
(material-type lcm (char "l")
  (help "light construction materials"))
(material-type hcm (char "h")
  (help "heavy construction materials"))
(material-type rad (char "r")
  (help "rads - radioactive materials"))
(material-type education
  (help "a class of graduates"))
(material-type happiness
  (help "happy strollers"))
(material-type civ
  (help "civilians"))
(material-type mil
  (help "military"))
(material-type uw
  (help "uncompensated workers (slaves, really)"))

(define raw (iron dust oil rad))
(define manufactures (food sh gun pet lcm hcm gold))
(define peoples (civ mil uw))

(add peoples people 1)

(terrain-type sea (char "."))
(terrain-type settled (image-name "clear") (char "-"))
(terrain-type wilderness (image-name "forest") (char "-"))
(terrain-type mountains (char "^"))
(terrain-type wasteland (image-name "orange") (char "/")
  (help "uninhabitable due to radioactivity"))
(terrain-type highway (image-name "road") (char "+")
  (subtype connection))

(define land-t* (settled wilderness mountains wasteland highway))

(add sea liquid true)

;;; Static relationships.

;; Unit vs unit.

(table unit-capacity-x
  ;; Engineers can be inside any facility.
  (facility-types engineer-types 1)
  )

(add transport-ship-types capacity (6 3))
(add carrier-ship-types capacity (4 4 8))
(add bomber-types capacity (1 3 9))
(add missile-types capacity (1 9 0 0))
(add headquarters capacity 8)
(add harbor capacity 8)
(add airfield capacity 8)
(add nuclear-plant capacity 8)
(add fortress capacity 8)
(add capital capacity 16)

(table unit-size-as-occupant
  (u* u* 100)
  (land-types transport-ship-types 1)
  (land-types headquarters 1)
  (land-types fortress 1)
  (ship-types harbor 1)
  (naval-fighter carrier-ship-types 1)
  (helicopter-types carrier-ship-types 1)
  ((recon-plane asw-plane) carrier-ship-types 1)
  (plane-types airfield 1)
  (small-nuke bomber-types 1)
  (medium-nuke bomber-types 3)
  (large-nuke bomber-types 9)
  (small-nuke missile-types 1)
  (medium-nuke missile-types 3)
  (large-nuke missile-types 9)
  (nuke-types nuclear-plant 1)
  ;; The capital has a bit of room for anything.
  (u* capital 1)
  (capital capital 100)
  )

;; Unit vs terrain.

(table vanishes-on
  (land-types sea true)
  (ship-types land-t* true)
  ;; (allow radar and mines to be placed in the sea)
  (facility-types sea true)
  (capital sea true)
  ;; Oil platforms can only be placed in sea cells.
  (oil-platform sea false)
  (oil-platform land-t* true)
  ;; Units normally die in wasteland.
  (u* wasteland true)
  ;; Planes can fly over wasteland safely.
  (plane-types wasteland false)
)

(add t* capacity 16)

(table unit-size-in-terrain
  (u* t* 1)
  ;; Only fill 13/16 of the cell, so mobile units can pass through, but
  ;; don't let multiple facilities be in a cell.
  (inert-types t* 13)
  (facility-types t* 13)
  (capital t* 13)
  )

(table terrain-storage-x
  ;; Room for raw materials
  (t* oil 100)
  (t* iron 100)
  (t* dust 10)
  (t* rad 10)
  ;; Room for peoples
  (settled peoples 999)
  )

(table unit-storage-x
  (land-types food 10)
  (ship-types food 100)
  ;; Special-function ships.
  (fishing-boat food 200)
  (tanker oil 100)
  (ore-ship iron 100)
  (cargo-ship (sh gun lcm hcm) 100)
  ;; This is not strictly part of Empire, but is needed to
  ;; ensure planes' and missile' limited range.
  (plane-types pet 16)
  (bomber-types pet (24 48 60))
  (missile-types pet (8 40 8 20))
  (facility-types food 30)
  (facility-types civ 10)
  ;; Special-function facilities.
  (agribusiness food 150)
  ;; What a harbor wants to have on hand to build ships.
  (harbor (lcm hcm) 200)
  ;; What an airfield wants to have on hand to build aircraft.
  (airfield (lcm hcm) 200)
  ;; What a nuclear plant needs to have on hand to build bombs.
  (nuclear-plant (oil lcm hcm rad) 999)
  ;; Fuel capacities.
  (land-types pet 10)
  (ship-types pet 100)
  (facility-types pet 10)
  ;; Havens for ships and planes are able to refuel their occupants.
  ((harbor airfield) pet 100)
  ;; Warehouses have lots of room, but only for manufactured goods.
  (warehouse manufactures 999)
  ;; Combat supplies.
  (artillery-types sh 100)
  (fortress sh 100)
  (artillery-types gun 9)
  (fortress gun 9)
  (land-types mil 1)
  ;; Everything centers on the capital.
  (capital m* 999)
  )

;;; (really need generic capacities for cargo ships)

;;; Vision.

(add mobile-radar vision-range 6)
(add radar vision-range 8)

(table see-chance
  ;; do radars in empire not see ground units?
  (mobile-radar land-types 0)
  (radar land-types 0)
  )

;;; Actions.

(table material-to-act
  ;; Ground units need soldiers to run them.
  (land-types mil 1)
  ;; Facilities all need civilians to run them.
  (facility-types civ 1)
  )

;;; Movement.

;(add missile-types speed 30.00)

(add inert-types speed 0)
(add facility-types speed 0)
(add capital speed 0)

(table mp-to-enter-terrain
  ;; Don't let land units drown themselves.
  (land-types sea 99)
  ;; Or ships beach themselves.
  (ship-types land-t* 99)
  )

(table consumption-per-move
  (plane-types pet 1)
  ;; Should doublecheck the following against real empire.
  (u* pet 1)
  )

;;; Construction.

;; Old version
(add u* cp 6)

;; New version
(add u* cp 20)
(add infantry-types cp (5 5 6 6 6))
(add armor-types cp (10 12 14))
(add artillery-types cp (10 9 12 12 8))
(add engineer-types cp (8 10))
(add fighter-types cp 14)
(add helicopter-types cp 10)
(add bomber-types cp (14 16 18))
(add missile-types cp (8 12 8 10))
(add facility-types cp 14)
(add transport-ship-types cp 14)
(add surface-combat-ship-types cp (14 16 16 20 25 30))
(add air-combat-ship-types cp (20 22 25 20 25))
(add warehouse cp 6)
(add capital cp 32)
(add fortress cp 10)

(table acp-to-create
  (engineer-types facility-types 1)
  (headquarters land-types 1)
  (harbor ship-types 1)
  (airfield plane-types 1)
  (nuclear-plant nuke-types 1)
  (capital engineers 1)
  )

(table acp-to-build
  (engineer-types facility-types 1)
  (headquarters land-types 1)
  (harbor ship-types 1)
  (airfield plane-types 1)
  (nuclear-plant nuke-types 1)
  (capital engineers 1)
  )

;; need pet/lcm/hcm to build things also.

(table consumption-per-build
  (ship-types lcm 50)
  (ship-types hcm 50)
  (plane-types lcm 50)
  (plane-types hcm 50)
  (nuke-types oil (50 75 100))
  (nuke-types lcm (50 75 100))
  (nuke-types hcm (50 75 100))
  (nuke-types rad (50 75 100))
  )

(table material-to-build
  ;; Need people to operate the equipment.
  (u* civ 1)
  ;; But engineer units *are* the people, so no extra requirement here.
  (engineer-types civ 0)
  )

;;; Research.

;; (some unit types should be higher tech than others)

;;; Production (and consumption).

(table acp-to-produce
  (refinery pet 1)
;   (lcm-factory lcm 1)
;   (hcm-factory hcm 1)
;   (shell-industry sh 1)
;   (nuclear-plant rad 1)
  )

(table material-per-production
  (refinery pet 100)
;   (lcm-factory lcm 100)
;   (hcm-factory hcm 100)
;   (shell-industry sh 100)
;   (nuclear-plant rad 100)
  )

(table material-to-produce
;  (u* civ 1)
  )

(table material-to-act
  (land-types pet 1)
  (plane-types pet 1)
  (ship-types pet 1)
  (facility-types pet 1)
  (fortress mil 2)
  )

;;; Combat.

;; Hit points are like "effectiveness" or "efficiency".
(add u* hp-max 100)

(table hit-chance
  (infantry-types facility-types 100)
  (infantry-types fortress 50)
  (infantry-types capital 100)
  )

(table damage
  (infantry-types facility-types 3d15)
  (infantry-types capital 2d10)
  )

(table capture-chance
  (u* u* 0)
  ;; Most facilities can't prevent their own capture.
  (infantry-types facility-types 100)
  ;; Fortresses are tough.
  (infantry-types fortress 10)
  (infantry-types capital 50)
  (armor-types fortress 20)
  ;; Capitals have people
  (armor-types capital 70)
  ;; Nuclear weapons are supposedly not captureable.
  (land-types nuke-types 0)
  )

(table independent-capture-chance
  (u* u* 100)
  (infantry-types facility-types 100)
  (infantry-types fortress 80)
  (infantry-types capital 80)
  (armor-types facility-types 100)
  (armor-types fortress 90)
  (armor-types capital 90)
  )

; sea shallows swamp desert plains forest mountains ice vacuum wasteland road
;@deffn Table @code{attack-terrain-effect} u1 t -> n%
(table attack-terrain-effect
;  (land-types shallows 30)
;  (land-types swamp 70)
;  (land-types forest 80)
;  (land-types mountains 60)
;  (land-types ice 90)
  )
  
;@deffn Table @code{defend-terrain-effect} u2 t -> n%
(table defend-terrain-effect
;  (land-types shallows 50)
;  (land-types swamp 50)
;  (land-types forest 130)
;  (land-types mountains 150)
;  (land-types ice 100)
;  (land-types road 80)
  )

(table protection ; actually ablation?
  ;; places offer some protection to occupants
  (fortress land-types 80)
  (capital land-types 40)
  (facility-types land-types 20)
  )

(add fortress acp-to-fire 1)
(add artillery-types acp-to-fire 1)

(add fortress range 3)
(add artillery-types range 3)

;	This table prevented non-artillery types from attacking anything since
;	they cannot carry shells.
;	(table consumption-per-attack
;	  (u* sh 1)
;	)

(table consumption-per-fire
	(artillery-types sh 1)
	(fortress sh 1)
)

(table hit-by
  (u* sh 1)
  )

(table material-to-attack
  (u* mil 1)
)

(table material-to-fire
  (artillery-types mil 1)
  (artillery-types gun 1)
  (fortress mil 1)
  (fortress gun 1)
  )

;; Nuclear detonation results in the destruction of units and the
;; creation of wasteland.

(add nuke-types acp-to-detonate 1)

(add nuke-types hp-per-detonation 100)

(table detonation-unit-range
  (small-nuke u* 1)
  (medium-nuke u* 2)
  (large-nuke u* 3)
  )

(table detonation-damage-at
  (small-nuke u* 200)
  (medium-nuke u* 200)
  (large-nuke u* 200)
  )

(table detonation-damage-adjacent
  ;; Ensure that all units within range are nailed.
  (small-nuke u* 1000)
  (medium-nuke u* 1000)
  (large-nuke u* 1000)
  )

(table detonation-terrain-range
  (small-nuke t* 1)
  (medium-nuke t* 2)
  (large-nuke t* 3)
  )

(table detonation-terrain-damage-chance
  (nuke-types t* 100)
  (nuke-types sea 0)
  )

(table terrain-damaged-type
  (t* wasteland 1)
  )

;;; Terrain alteration.

(table acp-to-add-terrain
  (engineer-types settled 1)
  (engineer-types highway 1)
  )

(table acp-to-remove-terrain
  (engineer-types wilderness 1)
  (engineer-types highway 1)
  )

;;; Backdrop.

(table base-production
  ((agribusiness fishing-boat) food (100 50))
  ((oil-field oil-platform) oil 1)  ;  should be extraction really
  (refinery pet 1)
  (enlistment mil 1)
;  (refinery pet 100)
;  (lcm-factory lcm 100)
;  (hcm-factory hcm 100)
  ;; The capital always has a few small farms and factories.
  (capital (food lcm hcm) (20 10 10))
  (capital (civ mil) 1)
  )

(table base-consumption
  ;; Everything eats food.
  (u* food 1)
  (agribusiness food 0) ; a hack
  (nuke-types food 0) ; does this happen in Empire?
  ;; Capital is self-supporting, but has no excess.
  (capital food 20)
  ;; Most hardware eats petroleum products.
  (ship-types pet 1)
  (plane-types pet 1)
  ;; Inert types don't consume anything at all.
  (inert-types m* 0)
  )

(table hp-per-starve
  ;; Going without food is potentially deadly.
  (u* food 1.00)
  ;; Technology types really need their petroleum products.
  (ship-types pet 10.00)
  (plane-types pet 100.00)
  )

(table out-length
  ;; This is to simplify basic game play.
  (u* m* 1)
  ;; Farming has a default supply infrastructure.
  (agribusiness food 5)
  ;; Fishing fleets must rely on land systems to redistribute.
  (fishing-boat food 1)
  ;; The oil business' support machinery can get the oil sent around.
  (oil-field oil 5)
  (oil-platform oil 5)
  ;; Civilians will tend to go where they're needed (looking for jobs).
  (facility-types civ 2)
  ;; Factories want to unload their manufactures as fast as possible.
  (refinery pet 5)
  (lcm-factory lcm 5)
  (hcm-factory hcm 5)
  ;; Warehouses are redistribution points.
  (warehouse m* 5)
  ;; Capital will be generous with nearby needy units.
  (capital m* 2)
  )

(table in-length
  ;; This is to simplify basic game play.
  (u* m* 1)
  ;; Every type of unit that needs food can get it from an adjacent cell.
  (u* food 1)
  ;; Similarly for petroleum.
  (u* pet 2)
  ;; Civilians will tend to go where they're needed (looking for jobs).
  (facility-types civ 2)
  ;; Facilities want to pull in whatever they need for their work.
  (harbor (lcm hcm) 5)
  (airfield (lcm hcm) 5)
  ;; Warehouses are redistribution points.
  (warehouse m* 5)
  ;; The capital always gets special service if it needs something.
  (capital m* 5)
  )

;;; Random setup.

;;; Everybody starts with just the one capital.

(add capital start-with 1)

(add (sea settled wilderness mountains) alt-percentile-min (  0  70 70  95))
(add (sea settled wilderness mountains) alt-percentile-max ( 70  75 95 100))
(add t* wet-percentile-min 0)
(add t* wet-percentile-max 100)
(add settled wet-percentile-min 40)
(add settled wet-percentile-max 60)

(table favored-terrain
  (u* t* 0)
  (capital settled 100)
  (facility-types settled 100)
  )

;; A game's starting units will be full by default.

(table unit-initial-supply
  (u* m* 9999)
  ;; ...but with minimal soldiers.
  (u* mil 1)
  )

(table terrain-initial-supply
  (t* oil 100)
  (t* iron 100)
  (t* dust 1)  ; should be rare but high concentration randomly
  (t* rad 1)
  )

(include "town-names")

(add capital namer "random-town-names")

(include "ng-features")

(set feature-types '(continents islands seas lakes bays (mountains 5) peaks))

(set feature-namers
  '((islands generic-island-names) (lakes generic-lake-names) (bays generic-bay-names)
  	(seas generic-sea-names) (continents generic-continent-names)
  	(mountains generic-mountain-names)))

;; Allow from 2 to many sides to play.

(set sides-min 2)
; The program complains if this is higher than the compiled-in limit,
; although in this case it should probably silently accept the GDL
; value and then complain only if too many actual sides get added.
;(set sides-max 30)
(set sides-max 15)

(game-module (instructions (
  "Build up your country and materials."
  "Create engineers in your capital, then send them out to clear the"
  "land and build farms, factories, harbors, roads, and so forth."
  ""
  "If you can't live in peace with your neighbors,"
  "then be prepared to fight with them."
  )))

(game-module (design-notes (
  "Scale is unimportant, this is an abstract game."
  ""
  "This is not entirely like True Empire, but it has some of the feel;"
  "and the complexity!"
  ""
  "This game is supposed to be an emulation.  If you don't like the
   way True Empire works, don't mess with this game; either work with
   `modern' if you're looking for more realism and scaling, or else
   set up a new game design."
  "Some of the differences are concessions to Xconq's limits."
  "You cannot just designate `sectors', you have to have units for
   them, and then you have to have a unit (engineers) to build."
  ""
  "Note: the mplayer does not quite understand how to play this game."
  "It builds lots of engineers which in turn build lots of headquarters."
  "No useful units are built. Hence the low version number." 
   
  )))
