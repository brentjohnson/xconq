(game-module "ancient-days"
  (title "Ancient Days")
  (version "0.1")
  (blurb "Human pre-history and early history. Collect materials, build villages and make advances.")
  (picture-name "altamira")
  (instructions "Use the \"collect food\" command to pick berries and \"collect gold\" to collect gold. Beware of lions and elephants.")
  (variants
    (world-seen false)
    (see-all false)
    (sequential false)
    (world-size (90 60))
    ("Alternate economy" model1
     "Use a different algorithm for moving materials among units."
     (true (set backdrop-model 1)))
    ("AltEcon uses doctrine" model1doctrine
     "When using alternate economy, don't drain units below doctrine levels."
     (true (set backdrop-ignore-doctrine 0))
     (false (set backdrop-ignore-doctrine 1)))
  )
)

(set no-indepside-ingame true)

; Disable until working properly.
; (set real-time-per-turn 3)

;;; Unit types.

(unit-type villager
  (help "the workers that collect food, wood, and metals"))

(define civilian-types (villager))

(add civilian-types acp-per-turn (2))

(unit-type clubman
  (help "oogah oogah"))
(unit-type axeman
  (help ""))
(unit-type swordsman (image-name "swordman")
  (help ""))
(unit-type hoplite
  (help ""))
(unit-type phalanx (image-name "pikeman")
  (help ""))
(unit-type legion
  (help "iron age army, very powerful"))

(define infantry-types (clubman axeman swordsman hoplite phalanx legion))

(add infantry-types acp-per-turn (2 2 2 1 1 1))

(unit-type horseman (image-name "cavalry")
  (help ""))
(unit-type cavalry
  (help ""))
(unit-type war-elephant (image-name "elephant")
  (help ""))
(unit-type chariot (image-name "wagon")
  (help ""))

(define mounted-types (horseman cavalry war-elephant chariot))

(add mounted-types acp-per-turn (4 4 1 4))

(unit-type archer
  (help ""))
(unit-type composite-archer (image-name "archer")
  (help ""))
(unit-type horse-archer (image-name "archer")
  (help ""))
(unit-type elephant-archer (image-name "archer")
  (help ""))
(unit-type chariot-archer
  (help ""))

(define archer-types (archer composite-archer horse-archer elephant-archer chariot-archer))

(add archer-types acp-per-turn (2 2 4 1 4))

(unit-type shaman
  (help ""))
(unit-type priest
  (help ""))
(unit-type messiah (image-name "person")
  (help ""))

(define priest-types (shaman priest messiah))

(add priest-types acp-per-turn (2 1 4))

(unit-type catapult
  (help ""))
(unit-type ballista (image-name "catapult")
  (help ""))

(define engine-types (catapult ballista))

(add engine-types acp-per-turn 1)

(unit-type fishing-boat (image-name "barge")
  (help ""))
(unit-type transport (image-name "galley")
  (help ""))
(unit-type galley
  (help ""))
(unit-type trireme
  (help ""))
(unit-type catapult-trireme (image-name "trireme")
  (help ""))

(define ship-types (fishing-boat transport galley trireme catapult-trireme))

(add ship-types acp-per-turn (2 2 2 4 4))

(unit-type wall (image-name "walltown")
  (help ""))
(unit-type watch-tower (name "watch tower") (image-name "watchtower")
  (help ""))
(unit-type ballista-tower (name "ballista tower") (image-name "keep")
  (help ""))

(define fort-types (wall watch-tower ballista-tower))

(add fort-types acp-per-turn (0 1 1))

(unit-type village
  (help "where the village people live"))
(unit-type granary
  (help "food storage"))
(unit-type treasury (image-name "camp")
  (help "wood and metal storage"))
(unit-type market (image-name "camp")
  (help "economic center"))
(unit-type farm (image-name "barn")
  (help "produces food"))
(unit-type barracks (image-name "stockade")
  (help "trains infantry units"))
(unit-type stable (image-name "stockade")
  (help "trains horse and elephant units"))
(unit-type workshop (image-name "camp")
  (help "builds engines and researches technologies"))
(unit-type shipyard (image-name "camp")
  (help "builds ships"))
(unit-type temple (image-name "parthenon")
  (help ""))
(unit-type palace (image-name "castle")
  (help ""))
(unit-type forum (image-name "parthenon")
  (help ""))

(define place-types (village granary treasury market farm barracks stable workshop shipyard temple palace forum))

(unit-type berry-bushes
  (help "a source of food"))
(unit-type antelope (image-name "eland")
  (help "safe prey for hunters, but hard to catch"))
(unit-type elephant
  (help "excellent source of food, but dangerous to hunt"))
(unit-type crocodile
  (help ""))
(unit-type lion
  (help ""))
(unit-type fish
  (help ""))
(unit-type carcass
  (help "body of a dead animal"))
(unit-type skeleton (image-name "animal-skeleton")
  (help "fate of a carcass that has been stripped of food"))

(define animals (antelope elephant crocodile lion))

(add animals acp-per-turn (3 1 1 3))

(define food-sources (berry-bushes fish carcass))

(unit-type stone-deposits
  (help "a source of stone for construction"))
(unit-type copper-deposits
  (help "a source of copper for bronze working"))
(unit-type iron-deposits
  (help "a source of iron for iron working"))
(unit-type gold-deposits
  (help "a source of gold"))

(define mineral-deposits (stone-deposits copper-deposits iron-deposits gold-deposits))

;;; Material types.

(material-type food
  (help "basic requirement for life"))
(material-type wood
  (help "most common construction material"))
(material-type stone
  (help "stronger construction material"))
(material-type copper
  (help "used for Bronze Age tools and weaponry"))
(material-type iron
  (help "used for Iron Age tools and weaponry"))
(material-type gold
  (help "valuable in all ages, used to purchase more advanced units"))

(add m* treasury true)

;;; Terrain types.

(terrain-type deep-sea (image-name "sea")
  (help ""))
(terrain-type sea (image-name "shallows")
  (help ""))
(terrain-type shallows (image-name "atoll")
  (help ""))
(terrain-type swamp
  (help ""))
(terrain-type desert
  (help ""))
(terrain-type forest
  (help ""))
(terrain-type grassland (image-name "plains")
  (help ""))
(terrain-type felled
  (help ""))
(terrain-type cleared
  (help ""))
(terrain-type hills
  (help ""))
(terrain-type mountains
  (help "high impassable mountains"))

(define water (deep-sea sea))

(define land (swamp desert forest grassland felled cleared hills mountains))

;;; Advance types.

(advance-type alchemy
  (help ""))
(advance-type archery
  (help "enables archers"))
(advance-type architecture
  (help "needed for more advanced buildings"))
(advance-type bronze-working (name "bronze working")
  (help "the fundamental advance of the Bronze Age"))
(advance-type chain-mail (name "chain mail")
  (help "Iron Age armor"))
(advance-type coinage
  (help ""))
(advance-type composite-bow (name "composite bow")
  (help ""))
(advance-type copper-mining (name "copper mining")
  (help ""))
(advance-type domestication
  (help "domestication of animals, improves food production of farms"))
(advance-type elephant-riding (name "elephant riding")
  (help "technique of taming and riding elephants"))
(advance-type engineering
  (help ""))
(advance-type gold-mining (name "gold mining")
  (help ""))
(advance-type horseback-riding (name "horseback riding")
  (help "technique of riding horses"))
(advance-type iron-mining (name "iron mining")
  (help ""))
(advance-type iron-working (name "iron working")
  (help "the fundamental advance of the Iron Age"))
(advance-type irrigation
  (help "improves food production, allows farms in deserts"))
(advance-type leather-armor (name "leather armor")
  (help "most primitive type of armor"))
(advance-type masonry
  (help ""))
(advance-type mathematics
  (help ""))
(advance-type monotheism
  (help ""))
(advance-type mysticism
  (help ""))
(advance-type navigation
  (help "needed to cross open sea"))
(advance-type plow
  (help "improves food production of farms"))
(advance-type polytheism
  (help ""))
(advance-type pottery
  (help "improves food storage?"))
(advance-type scale-armor
  (help "Bronze Age armor"))
(advance-type stone-mining
  (help ""))
(advance-type toolworking
  (help "the fundamental advance of the Tool Age"))
(advance-type wheel
  (help "needed to make wheeled units"))
(advance-type woodworking
  (help "needed to make complex wooden construction"))
(advance-type writing
  (help ""))

(table advance-needed-to-build
  (u* a* false)
  (archer-types archery true)
  ((palace forum) architecture true)
  ((swordsman hoplite phalanx) bronze-working true)
  ((composite-archer horse-archer elephant-archer chariot-archer) composite-bow true)
  ((war-elephant elephant-archer) elephant-riding true)
  ((ballista ballista-tower) engineering true)
  (mounted-types horseback-riding true)
  (stable horseback-riding true)
  ((horse-archer elephant-archer chariot-archer) horseback-riding true)
  (legion (chain-mail iron-working) true)
  ((ballista ballista-tower) iron-working true)
  (fort-types masonry true) ; others, or not?
  (catapult mathematics true)
  (messiah monotheism true)
  (temple mysticism true)
  (priest polytheism true)
  (axeman toolworking true) ; others?
  ;; Once you have tools, the opportunity arises to trade them for other
  ;; things of value.
  ((farm market stable wall watch-tower) toolworking true)
  ((chariot chariot-archer) wheel true)
  (ship-types woodworking true)
  ((workshop shipyard) woodworking true)
  ((trireme ballista ballista-tower) writing true)
  )

(table advance-needed-to-research
  (a* a* false)
  (architecture masonry true)
  (coinage gold-mining true)
  (elephant-riding horseback-riding true)
  (engineering mathematics true)
  (monotheism mysticism true)
  (mysticism polytheism true)
  (wheel bronze-working true) ; dubious, should be indirect?
  (wheel horseback-riding true)
  )

;;; Definitions.

(define wheeled-types (chariot chariot-archer catapult ballista))

(define land-types (append civilian-types infantry-types mounted-types archer-types priest-types fort-types place-types mineral-deposits))

;;; Static relationships.

(table vanishes-on
  (land-types water true)
  (ship-types land true)
  ;; Not very accurate, but needed to make the game work.
  (u* (forest mountains) true)
  )

(add t* capacity 4)

(table unit-size-in-terrain
  ;; Allow limited stacking of mobile units.
  (u* t* 1)
  ;; but not places.
  (fort-types t* 4)
  (place-types t* 4)
  ;; Allow mobile unit to pass through, but not stacking of, material sources.
  (food-sources t* 3)
  (mineral-deposits t* 3)
  )

;;; Unit-material capacities.

(table unit-storage-x
  (villager m* 10)
  (food-sources food 100)
  (stone-deposits stone 100)
  (copper-deposits copper 100)
  (iron-deposits iron 100)
  (gold-deposits gold 100)
  (farm food 20)
  )

(table terrain-storage-x
  (forest wood 20)
  (felled wood 80)
  )

(table gives-to-treasury
  (village m* true)
  (granary food true)
  (treasury m* true)
  (treasury food false)
  )

;;; Vision.

(add u* vision-range 2)

(add horseman vision-range 4)

;; Note that we allow our people to see "through" impassable terrain like
;; forests and mountains.  This is because these are not literal barriers;
;; individuals belonging to the groups represented by the units may pass through
;; and report on what they see, even though the whole unit won't go.

;;; Actions.

(add place-types acp-per-turn 1)
(add farm acp-per-turn 0)

;;; Movement.

(add place-types speed 0)
(add fish speed 0)
(add mineral-deposits speed 0)

(table mp-to-enter-terrain
  (land-types water 99)
  (land-types (forest felled) 99)
  (villager hills 2)
  (land-types mountains 99)
  ;; Antelopes stick to open ground.
  (antelope t* 99)
  (antelope (grassland desert) 1)
  )

(table mp-to-enter-own
  (civilian-types food-sources 0)
  (infantry-types food-sources 0)
  )

;;; Construction.

(add civilian-types cp 10)
(add infantry-types cp 10)

(add place-types cp 10)

(table can-create
  (villager place-types 1)
  (village villager 1)
  (barracks infantry-types 1)
  (barracks archer-types 1)
  (stable mounted-types 1)
  (shipyard ship-types 1)
  (temple (priest messiah) 1)
  )

(table can-build
  (villager place-types 1)
  (village villager 1)
  (barracks infantry-types 1)
  (barracks archer-types 1)
  (stable mounted-types 1)
  (shipyard ship-types 1)
  (temple (priest messiah) 1)
  )

(table acp-to-create
  (villager place-types 1)
  (village villager 1)
  (barracks infantry-types 1)
  (barracks archer-types 1)
  (stable mounted-types 1)
  (shipyard ship-types 1)
  (temple (priest messiah) 1)
  )

(table acp-to-build
  (villager place-types 1)
  (village villager 1)
  (barracks infantry-types 1)
  (barracks archer-types 1)
  (stable mounted-types 1)
  (shipyard ship-types 1)
  (temple (priest messiah) 1)
  )

(table create-range
  (villager place-types 1)
  (village villager 1)
  (barracks infantry-types 1)
  (barracks archer-types 1)
  (stable mounted-types 1)
  (shipyard ship-types 1)
  (temple (priest messiah) 1)
  )

(table build-range
  (villager place-types 1)
  (village villager 1)
  (barracks infantry-types 1)
  (stable mounted-types 1)
  (shipyard ship-types 1)
  (temple (priest messiah) 1)
  )

(table consumption-on-creation
  (villager food 40)
  (infantry-types food 50)
  (place-types wood 100)
  )

;;; Production.

(table acp-to-load
  (villager food 1)
  )

(table acp-to-unload
  (farm food 1)
  )

(table base-production
  (village food 1)
  (farm food 1)
  )

(table productivity
  (u* t* 0)
  (village grassland 100)
  (village desert 50)
  (farm grassland 100)
  )

(table acp-to-extract
  (villager m* 1)
  )

(table hp-per-starve
  ;; Depleted sources of materials disappear.
  (berry-bushes food 20.00)
  (fish food 20.00)
  (carcass food 20.00)
  (stone-deposits stone 20.00)
  (copper-deposits copper 20.00)
  (iron-deposits iron 20.00)
  (gold-deposits gold 20.00)
  )

;;; Combat.

(add u* hp-max 20)
(add animals hp-max (3 20 5 10))
(add (carcass skeleton) hp-max 10)

(table acp-to-attack
  (u* berry-bushes 0)
  (u* (carcass skeleton) 0)
  (u* mineral-deposits 0)
  (antelope u* 0)
  )

(table withdraw-chance-per-attack
  (u* antelope 50)
  )

(table hit-chance
  (villager villager 20)
  (villager animals 50)
  (infantry-types civilian-types 80)
  (infantry-types infantry-types 50)
  (infantry-types animals 80)
  (infantry-types place-types 100)
  (animals villager (50 70 40 70))
  (antelope villager 0)
  )

(table damage
  (villager villager 1)
  (villager animals 1d4+1)
  (infantry-types civilian-types 2d5+1)
  (infantry-types infantry-types 1d4+1)
  (infantry-types animals 2d5+4)
  (infantry-types place-types 1)
  (animals villager (0 2d5+1 1d5 2d4))
  (antelope villager 0)
  )

(table counterattack
  (antelope u* 0)
  )

;(table capture-chance
;  )

(add (antelope elephant crocodile lion) wrecked-type carcass)

(add carcass wrecked-type skeleton)

;;; Backdrop.

(table attrition
  ;; Carcasses eventually become skeletons, skeletons eventually disappear.
  ((carcass skeleton) t* 0.50)
  )

(table terrain-exhaustion-type
  (forest wood felled)
  (felled wood cleared)
  )

(table change-on-exhaustion-chance
  (forest wood 100)
  (felled wood 100)
  )

;;; Scoring.

(scorekeeper (do last-side-wins))

;;; Random generation.

(set alt-blob-size 60)
(set alt-blob-density 3000)
(set alt-smoothing 4)
(set wet-smoothing 6)

(add t* alt-percentile-min (   0  15  20  25  25  50  25   0   0  90  97))
(add t* alt-percentile-max (  15  20  25  30  90  80  90   0   0  97 100))
(add t* wet-percentile-min (   0   0   0  50   0  50  20   0   0   0   0))
(add t* wet-percentile-max ( 100 100 100 100  20 100 100   0   0 100 100))

(table adjacent-terrain-effect
  ;; Don't let forest run down to water's edge.
  (forest water grassland)
  )

(add villager start-with 3)
(add village start-with 1)

(set country-radius-min 2)
(set country-separation-min 30)
(set country-separation-max 60)
(add (grassland forest) country-terrain-min (15 1))

(table independent-density
  (berry-bushes grassland 500)
  (crocodile (swamp shallows) (1000 300))
  (antelope grassland 500)
  (elephant grassland 100)
  (lion grassland 100)
  (fish sea 1000)
  (mineral-deposits grassland (100 100 100 100))
  (mineral-deposits desert (200 200 200 200))
  (mineral-deposits hills (1000 1000 1000 1000))
  )

(table favored-terrain
  (u* t* 0)
  (villager grassland 100)
  (village grassland 100)
  (berry-bushes grassland 100)
  (crocodile (swamp shallows) 100)
  (antelope grassland 100)
  (elephant grassland 100)
  (lion grassland 100)
  (fish sea 100)
  (mineral-deposits hills 100)
  )

(add food initial-treasury 200)
(add wood initial-treasury 200)
(add stone initial-treasury 100)

(table unit-initial-supply
  (food-sources food 100)
  (stone-deposits stone 100)
  (copper-deposits copper 100)
  (iron-deposits iron 100)
  (gold-deposits gold 100)
  )

(table terrain-initial-supply
  (forest wood 20)
  (felled wood 80)
  )

;; Default doctrine.

(doctrine default-doctrine
  (construction-run (u* 1))
  )

(doctrine place-doctrine
  (construction-run (u* 99))
  )

(side-defaults
  (default-doctrine default-doctrine)
  (doctrines (place-types place-doctrine))
  )

(set sides-wanted 4)

(set side-library '(
  ((noun "Akkadian"))
  ((noun "Assyrian"))
  ((noun "Babylonian"))
  ((noun "Elamite"))
  ((noun "Greek"))
  ((noun "Hittite"))
  ((noun "Mitanni"))
  ((noun "Egyptian"))
  ((noun "Persian"))
  ((noun "Phoenician"))
  ((noun "Roman"))
  ((noun "Sumerian"))
  ))

(set side-color-library '(
  "blue" "red" "brown" "gold" "gray" "dark-green" "white"
  ))

(imf "cleared" ((8 8 tile)
  (mono "00/00/0c/1c/1c/1c/3e/00")))
(imf "cleared" ((16 16 tile)
  (color (pixel-size 4)
   (palette
    (0 7969 46995 5169)
    (1 37079 29024 14900)
    (2 22016 11421 1316)
    (3 39666 20577 2371)
    (4 16384 8499 979))
   "0000000000000000/0000000000000000/0000120000000000/0001320000000000"
   "0001320000000000/0001220000000000/0044444000000000/0000000000000000"
   "0000000000012000/0000000000132000/0000000000132000/0000000000122000"
   "0000000004444400/0000000000000000/0000000000000000/0000000000000000")))
(imf "felled" ((8 8 tile)
  (mono "00/70/ed/9d/1c/1c/3e/00")))
(imf "felled" ((16 16 tile)
  (color (pixel-size 4)
   (palette
    (0 7969 46995 5169)
    (1 37079 29024 14900)
    (2 22016 11421 1316)
    (3 39666 20577 2371)
    (4 16384 8499 979))
   "0000000200440000/4000000120004044/0400120012040400/3331323331200000"
   "4441324444120114/0001220000112320/0044444011331200/0400001133220120"
   "0040113322012010/0411332200132000/1133220040132001/3322004400122033"
   "2244440004444403/0040004400000002/0004000044000000/0000400000004400")))

(imf "fish" ((16 16)
  (color (pixel-size 4)
   (palette
    (0 white)
    (1 26214 26214 39321)
    (2 13107 26214 39321)
    (3 13107 13107 26214)
    (4 39321 52428 65535)
    (5 17476 17476 17476)
    (6 52428 65535 65535))
   "0000000000000000/0000000000000000/0000000000000000/0000000000000000"
   "0000000000000000/0000000000000000/0000003000000000/0000033000000050"
   "0003322330000500/0032222222303300/0111111111111100/0044444444401100"
   "0006666660000500/0000010000000050/0000000000000000/0000000000000000")
  (mono "0000/0000/0000/0000/0000/0000/0200/0602/1f84/3fec/7ffc/3fec/1f84/0402/0000/0000")
  (mask "0000/0000/0000/0000/0000/0000/0200/0602/1f84/3fec/7ffc/3fec/1f84/0402/0000/0000")))
(imf "berry-bushes" ((16 16)
  (color (pixel-size 4)
   (palette
    (0 white)
    (1 0 30583 0)
    (2 0 17476 0)
    (3 0 43690 0)
    (4 0 4369 0)
    (5 13107 13107 0)
    (6 52428 0 0)
    (7 39321 0 0)
    (8 red))
   "0000000000000000/0000000333000000/0000101386300000/0003031167310000"
   "0003386111230000/0003367328633000/0001142446711000/0001322232422000"
   "0001186142861000/0000167221672000/0000224142440000/0000041142100000"
   "0000000550000000/0000000550000000/0000000000000000/0000000000000000")
  (mono "0000/01c0/0be0/17f0/1ff0/1ff8/1ff8/1ff8/1ff8/0ff8/0ff0/07e0/0180/0180/0000/0000")
  (mask "0000/01c0/0be0/17f0/1ff0/1ff8/1ff8/1ff8/1ff8/0ff8/0ff0/07e0/0180/0180/0000/0000")))
(imf "copper-deposits" ((16 16)
  (color (pixel-size 4)
   (palette
    (0 white)
    (1 21845 21845 21845)
    (2 48059 48059 48059)
    (3 61166 61166 61166)
    (4 8738 8738 8738)
    (5 17476 17476 17476)
    (6 34952 34952 34952)
    (7 52428 13107 0))
   "0000000000000000/0000000000000000/0000000300000000/0000033226000000"
   "0000332776610000/0000326677110000/0003266111163000/0033267221122200"
   "0032677267326600/0026672677267100/0026712673277100/0326112112277610"
   "0221177152756440/0151755577477440/0544444444444440/0000000000000000")
  (mono "0000/0000/0000/0000/0030/00f0/03e0/0660/0600/0c8c/0c8c/1d9a/1fb6/7ffe/7ffe/0000")
  (mask "0000/0000/0100/07c0/0ff0/0ff0/1ff8/3ffc/3ffc/3ffc/3ffc/7ffe/7ffe/7ffe/7ffe/0000")))
(imf "gold-deposits" ((16 16)
  (color (pixel-size 4)
   (palette
    (0 white)
    (1 21845 21845 21845)
    (2 48059 48059 48059)
    (3 61166 61166 61166)
    (4 8738 8738 8738)
    (5 17476 17476 17476)
    (6 34952 34952 34952)
    (7 65535 52428 0))
   "0000000000000000/0000000000000000/0000000300000000/0000033226000000"
   "0000332776610000/0000326677110000/0003266111163000/0033267221122200"
   "0032677267326600/0026672677267100/0026712673277100/0326112112277610"
   "0221177152756440/0151755577477440/0544444444444440/0000000000000000")
  (mono "0000/0000/0000/0000/0030/00f0/03e0/0660/0600/0c8c/0c8c/1d9a/1fb6/7ffe/7ffe/0000")
  (mask "0000/0000/0100/07c0/0ff0/0ff0/1ff8/3ffc/3ffc/3ffc/3ffc/7ffe/7ffe/7ffe/7ffe/0000")))
(imf "iron-deposits" ((16 16)
  (color (pixel-size 4)
   (palette
    (0 white)
    (1 21845 21845 21845)
    (2 48059 48059 48059)
    (3 61166 61166 61166)
    (4 8738 8738 8738)
    (5 17476 17476 17476)
    (6 34952 34952 34952)
    (15 black))
   "0000000000000000/0000000000000000/0000000300000000/0000033226000000"
   "0000332ff6610000/000032f661110000/000326611ff63000/003326f22ff22200"
   "00326ff26f326600/00266ff6f62f1100/00261f26f32ff100/0326112f122f5610"
   "022ff11152ff6440/01515ff544444440/0544444444444440/0000000000000000")
  (mono "0000/0000/0000/0000/0030/00f0/03e0/0660/0600/0c8c/0c8c/1d9a/1fb6/7ffe/7ffe/0000")
  (mask "0000/0000/0100/07c0/0ff0/0ff0/1ff8/3ffc/3ffc/3ffc/3ffc/7ffe/7ffe/7ffe/7ffe/0000")))
(imf "stone-deposits" ((16 16)
  (color (pixel-size 4)
   (palette
    (0 white)
    (1 21845 21845 21845)
    (2 48059 48059 48059)
    (3 61166 61166 61166)
    (4 8738 8738 8738)
    (5 17476 17476 17476)
    (6 34952 34952 34952))
   "0000000000000000/0000000000000000/0000000300000000/0000033226000000"
   "0000332266610000/0000326661110000/0003266111163000/0033261221122200"
   "0032611266326600/0026612616261100/0026112613261100/0326112112215610"
   "0221111152556440/0151555544444440/0544444444444440/0000000000000000")
  (mono "0000/0000/0000/0000/0030/00f0/03e0/0660/0600/0c8c/0c8c/1d9a/1fb6/7ffe/7ffe/0000")
  (mask "0000/0000/0100/07c0/0ff0/0ff0/1ff8/3ffc/3ffc/3ffc/3ffc/7ffe/7ffe/7ffe/7ffe/0000")))
