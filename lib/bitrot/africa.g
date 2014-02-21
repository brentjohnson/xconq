(game-module "africa"
  (title "Explorers")
  (blurb "Base module for African exploration.")
  (picture-name "lion")
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
  (instructions "Beware of lions and crocodiles!")
)

;;; Unit types.

(unit-type explorer
  (help "the intrepid explorer, along with trusted assistants"))

; add bearers and warriors?

(unit-type horse
  )

(unit-type camel
  )

(unit-type antelope (image-name "eland")
  )

(unit-type zebra
  )

(unit-type gnu
  )

(unit-type giraffe
  )

(unit-type buffalo
  )

(unit-type hippo
  )

(unit-type rhino
  )

(unit-type elephant
  )

(unit-type crocodile
  (help "found around water, often deadly"))

(unit-type lion
  (help "king of beasts, dangerous to tangle with"))

(define domestic (horse camel))

(define herd (antelope zebra gnu giraffe elephant))

(define solitary (buffalo hippo rhino))

(define carnivorous (crocodile lion))

(define animals (append domestic herd solitary carnivorous))

(add herd possible-sides "independent")
(add solitary possible-sides "independent")
(add carnivorous possible-sides "independent")

(unit-type carcass
  (help "what's left when an animal is killed"))

(unit-type canoe (image-name "ap")
  )

(unit-type ship (image-name "ap")
  (help "transportation to and from civilization"))

(unit-type tribe (image-name "horde")
  (description-format (type))
  (help "a native African tribe"))

(unit-type village (image-name "hut")
  (description-format (type))
  (help "a native African settlement"))

(imf "hut" ((32 32) (file "civmisc.gif" 2 2)))

(unit-type town (image-name "town20")
  (description-format (name))
  (help "civilization's toehold in Africa"))

(unit-type city (image-name "city18")
  (description-format (name))
  (help "a European city with all the amenities"))

(define places (village town city))

;;; Material types.

(material-type water
  (help "animals and people must have plenty of water"))

(material-type food
  (help "what people eat"))

;;; Terrain types.

(terrain-type sea (char ".")
  )
(terrain-type lake (image-name "shallows") (char ",")
  )
(terrain-type swamp (char "=")
  (help "flat terrain with standing water and vegetation"))
(terrain-type desert (char "~")
  (help "dry terrain, no water or food"))
(terrain-type savannah (image-name "plains") (char "+")
  )
(terrain-type forest (char "%")
  )
(terrain-type mountains (char "^")
  )
(terrain-type wide-river (image-name "shallows") (char "-")
  (help "miles-wide river"))
(terrain-type salt-lake (image-name "shallows") (char ",")
  )
(terrain-type hills (char "^")
  )
(terrain-type forested-hills (image-name "forest") (char "%")
  )
(terrain-type Europe (image-name "plains")
  )
(terrain-type river (image-name "river-conn") (char "<")
  (subtype connection)
  (help "canoe-able river"))

(define water-t* (sea lake wide-river salt-lake))

(define land-t* (desert savannah forest mountains hills forested-hills Europe))

(define cell-t* (append water-t* land-t*))

(add water-t* liquid true)

(table drawable-terrain
  (t* t* true)
  (river (sea lake wide-river salt-lake) false)
  )

;; Everything is measured in feet.

(add t* elevation-min 0)
(add t* elevation-max 4000)
(add mountains elevation-max 4001)
(add mountains elevation-max 20000)
(add sea elevation-max 0)

;; Cells are 10 miles across.

(area (cell-width 52800))

;;; Static relationships.

(table vanishes-on
  (explorer water-t* true)
  (animals water-t* true)
  (ship land-t* true)
  (tribe water-t* true)
  (places water-t* true)
  )

;;; Unit-unit capacities.

(add places capacity 20)
(add ship capacity 2)

(table unit-size-as-occupant
  ;; Disable occupancy by default.
  (u* u* 100)
  (explorer places 1)
  (explorer ship 1)
  (canoe places 1)
  (ship places 1)
  )

;;; Unit-terrain capacities.

;; Limit units to 16 in one cell, for the sake of playability and
;; and drawability.  Only towns and cities cover the entire cell.
  
(add t* capacity 16)

(table unit-size-in-terrain
  (u* t* 1)
  (ship land-t* 99)
  ((town city) t* 16)
  )

;;; Unit-material capacities.

(table unit-storage-x
  (explorer water 14)
  (ship water 900)
  (tribe water 50)
  (places water (100 900 900))
  (explorer food 20)
  (animals food 100)
  )

;;; Vision.

(add explorer already-seen 100)
(add places already-seen 100)

(add places already-seen-independent 100)

(table see-chance-adjacent
  ;; Crocodiles are always hard to see.
  (u* crocodile 10)
  ;; Lions can be hard to see sometimes.
  (u* lion 50)
  )

;;; Actions.

(add explorer acp-per-turn 7)
(add animals acp-per-turn 7)
(add domestic acp-per-turn 11)
(add carcass acp-per-turn 1)
(add canoe acp-per-turn 14)
(add ship acp-per-turn 140)
(add places acp-per-turn 0)

;;; Movement.

(add carcass speed 0)
(add places speed 0)

(table mp-to-enter-terrain
  (explorer t* 1)
  (explorer water-t* 99)
  (explorer swamp 3)
  (explorer (forest mountains) 2)
  (animals water-t* 99)
  (canoe t* 99)
  (ship land-t* 999)
  )

(table mp-to-leave-terrain
  (explorer water-t* 99)
  (explorer swamp 4)
  (explorer (forest mountains) 2)
  (animals water-t* 99)
  (canoe t* 99)
  (ship land-t* 999)
  )

(table mp-to-traverse
  (canoe (lake wide-river) 1)
  )

(add explorer free-mp 7)

;; Ships are not the explorers' personal yachts; they can only pick up and
;; drop off at towns or cities.

(table ferry-on-entry
  ((town city) explorer over-all)
  (ship explorer over-nothing)
  )

(table ferry-on-departure
  ((town city) explorer over-all)
  (ship explorer over-nothing)
  )

(table zoc-range
  (u* u* 0)
  (explorer u* -1)
  (animals u* -1)
  )

(table can-enter-independent
  (explorer places true)
;  (ship (town city) true)
  )

;;; Combat.

(add u* hp-max 1)
(add explorer hp-max 20)
(add rhino hp-max 5)
(add elephant hp-max 10)

(table hit-chance
  (explorer animals 50)
  ;; A human can generally take care of wild animals, but the job
  ;; is not without risk!
  (animals explorer 10)
  )

(table damage
  (explorer animals 1)
  (animals explorer 1d15+1)
  )

(table withdraw-chance-per-attack
  ;; Most types of animals will panic and run at times.
  (explorer animals 10)
  ;; Herd animals survive by running away all the time.
  (explorer herd 90)
  ;; Native tribes may run also.
  (explorer tribe 10)
  )

(table retreat-chance
  ;; Most animals will run away if hurt.
  (explorer animals 30)
  )

;; Dead animals leave carcasses that can be used for food.

(add animals wrecked-type carcass)

(table capture-chance
  ;; Not too hard to "capture" a dead body.
  (explorer carcass 100)
  )

;;; Other Actions.

(add carcass acp-to-disband 1)

(add carcass hp-per-disband 1)

(table recycleable-material
  (carcass food 100)
  )

;;; Production.

(table base-production
  (explorer water 8)
  ;; Most terrain will have rabbits and birds and such, that don't require
  ;; taking on the big game.
  (explorer food 1)
  )

(table productivity
  (explorer t* 100)
  (explorer (sea desert salt-lake) 0)
  )

(table base-consumption
  (explorer water 7)
  (explorer food 3)
  )

(table hp-per-starve
  ;; Lack of water is very dangerous.
  (explorer water 4.50)
  ;; Lack of food is also bad, but slower to act (the truly
  ;; hungry can always find grubs and such).
  (explorer food 1.00)
  )

;;; Backdrop.

(table attrition
  ;; All kinds of diseases and minor accidents can happen at any
  ;; time.
  (explorer t* 0.10)
  ;; Some kinds of terrain are worse than others.
  (explorer swamp 1.50)
  (explorer forest 0.75)
  ;; Carcasses all disappear in a week.
  (carcass t* 1.00)
  )

;; An explorer takes about 6 months to recuperate from near-fatal
;; injuries.

(add explorer hp-recovery 1.00)

;; Set up weekly turns.

(world (year-length 52))

(set calendar '(usual week))

;;; Initial setup.

(set sides-wanted 4)

(add explorer start-with 1)
(add ship start-with 1)

(set self-required true)

(add explorer can-be-self true)

(set synthesis-methods
  '(make-random-terrain make-countries make-independent-units))

(set edge-terrain sea)

(set country-radius-min 1)
(set country-separation-min 1)
(set country-separation-max 5)
;; Try to get countries to be on the coast.
(add (sea Europe) country-terrain-min (2 2))

(table favored-terrain
  (u* t* 0)
  (explorer Europe 100)
  (ship sea 100)
  )

(table independent-density
  (herd savannah 100)
  (giraffe savannah 50)
  (camel desert 50)
  (hippo (forest swamp) (100 150))
  (elephant (forest swamp) (100 150))
  ;; Crocodiles are a notable hazard, but in swamps only.
  (crocodile swamp 1000)
  (lion (savannah swamp) (100 10))
  (tribe (desert savannah forest mountains) (10 50 50 20))
  (village (savannah forest mountains) (100 50 25))
  )

;; A game's starting units will be full by default.

(table unit-initial-supply (u* m* 9999))

;; No way (yet) to do variable advantage in this game.

(set advantage-max 1)

(set side-library '(
  ((name "Baker") (emblem-name "flag-uk"))
  ((name "Burton") (emblem-name "flag-uk"))
  ((name "Grant") (emblem-name "flag-uk"))
  ((name "Speke") (emblem-name "flag-uk"))
  ((name "Stanley") (emblem-name "flag-usa"))
  ))
