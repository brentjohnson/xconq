(game-module "omniterr2"
  (blurb "A highly-detailed module for representing terrain")
  (variants
    (see-all true)
    ; (world-size (80 32 80))
    (world-size (800 320 800))
    ("No axial tilt" no-axial-tilt
     "Reset axial tilt to zero (to work around a day/night bug)"
     (true
       (world (axial-tilt 0))
    ))
    ("Hi-res turns" hi-res-turns
     "Set each turn equal to six minutes (usually equals one hour)"
     (true
       (world (day-length 144) (year-length 52560))
       (set season-names '((0 13140 "Winter") (13141 26280 "Spring") (26281 39420 "Summer") (39421 52560 "Autumn")))
    ))
    ("Low-res turns" low-res-turns
     "Set each turn equal to one day (thus eliminating day/night considerations)"
     (true
       (world (day-length 1) (year-length 364))
       (set season-names ((0 91 "Winter") (92 182 "Spring") (183 273 "Summer") (274 364 "Autumn")))
    ))
    ("No climate" no-climate
     "Don't generate climates (leave all dry land as undefined land)"
     (true
       (table terrain-density (t* t* 0.00))
       (table adjacent-terrain-effect (t* t* non-terrain))
       (table adjacent-terrain-effect-chance (t* t* 0.00))
    ))
  )
)

(set sides-min 1)
(set sides-wanted 1)
(set sides-max 1)
 
(world
  ; Set up the world to equal Earth at 50km/hex, with an axial tile of 23.5 degrees.

  (axial-tilt 23.5)
  
  ; Set up the game for 1 hour per turn, 24 hours per day, 365 days per year.

  (day-length 24)	; One turn  = one hour
  (year-length 8760)	; One turn  = one hour
)

; Set Earth-like seasons.

(set season-names '((0 2190 "Winter") (2191 4380 "Spring") (4381 6570 "Summer") (6571 8760 "Autumn")))

;;; CELL TERRAIN

;; Cell terrain reflects soil composition or other such factors.

(terrain-type void (image-name "vacuum")
  (help "An area with no mass"))

(terrain-type fresh-water (image-name "shallows")
  (help "Drinkable water, usually in a lake or river"))
  
(terrain-type salt-water (image-name "sea")
  (help "Undrinkable water, usually found in oceans"))
  
; (terrain-type glacier (image-name "ice")
;   (help "Frozen water and no land, usually found at the poles and at extreme altitude"))

; "Dummy" terrain types

(terrain-type land (image-name "mountains")
  (help "Dry land"))
  
; (define base-terrain (void land fresh-water salt-water glacier))
  
;;; TERAIN COATINGS

;; Level 1: Climate

(terrain-type af (image-name "civ-jungle") (name "Tropical rain forest")
  (help "High annual temperatures, abundant rainfall, no dry season"))
  
(terrain-type am (image-name "civ-jungle") (name "Tropical monsoon")
  (help "High annual temperatures, abundant rainfall, short dry season"))
  
(terrain-type aw (image-name "plains") (name "Tropical wet-and-dry")
  (help "High annual temperatures, adundant but unstable rainfall, medium dry season"))
  
(terrain-type bwh (image-name "adv-desert") (name "Hot arid desert")
  (help "High annual temperatures, very low precipitation"))
  
(terrain-type bwk (image-name "adv-semi-desert") (name "Cold arid desert")
  (help "Moderate or low annual temperatures, very low precipitation"))
  
(terrain-type bsh (image-name "desert") (name "Hot semi-arid desert")
  (help "High annual temperatures, low precipitation"))
  
(terrain-type bsk (image-name "adv-steppe") (name "Cold semi-arid desert")
  (help "Moderate or low annual temperatures, low precipitation"))
  
(terrain-type cfa (image-name "adv-plain") (name "Humid sub-tropical")
  (help "Moderate annual temperatures, moderate precipitation, high humidity"))
  
(terrain-type cfb (image-name "plains") (name "Marine 1")
  (help "Moderate annual temperatures with low variation, heavy precipitation, short summer"))

(terrain-type cfc (image-name "plains") (name "Marine 2")
  (help "Moderate annual temperatures with low variation, heavy precipitation, medium-length summer"))
  
(terrain-type csa (image-name "civ-plains") (name "Interior Mediterranean")
  (help "High summer temperatures with low precipitation, mild winter temperatures with high precipitation"))
  
(terrain-type csb (image-name "civ-plains") (name "Coastal Mediterranean")
  (help "Medium-high summer temperatures, mild winter temperatures, near-constant fog cover"))
  
; (should implement something to distinguish between the very-dry/less-hot Mediterranean climate of California and the less-dry/very-hot Mediterranean climate of the Mediterranean sea?)

(terrain-type cw (image-name "adv-plain") (name "Dry winter")
  (help "Moderate annual temperatures, moderate precipitation, dry, mild winters"))

(terrain-type dfa (image-name "adv-forest") (name "Humid continental, hot summers")
  (help "Low annual temperatures with hot, long summers, high precipitation"))
  
(terrain-type dfb (image-name "forest") (name "Humid continental, long cool summers")
  (help "Low annual temperatures with moderate summers, moderate precipitation"))
  
(terrain-type dfc (image-name "civ-forest") (name "Subpolar 1")
  (help "Low annual temperatures with cool, short summers and severe winters, moderate precipitation"))
  
(terrain-type dfd (image-name "civ-forest") (name "Subpolar 2")
  (help "Very low annual temperatures with cool, short summers, moderate precipitation"))
  
(terrain-type dw (image-name "civ-forest") (name "Subpolar, dry-winter")
  (help "Low annual temperatures with cool, short summers and severe winters, low precipitation"))
  
(terrain-type et (image-name "civ-tundra") (name "Polar tundra")
  (help "Very low annual temperatures, permafrost most of the year"))
  
(terrain-type ef (image-name "adv-ice") (name "Polar ice cap")
  (help "Temperatures always below freezing, very low precipitation"))
  
; Is h climate necessary here, since elevation is already handled?

; (terrain-type h (image-name "maroon") (name "Highland")
;   (help "Temperatures always below freezing due to extreme altitude, very low precipitation"))
  
(define climates (af am aw bwh bwk bsh bsk cfa cfb cfc csa csb dfa dfb dfc dfd dw et ef))
; (add climates subtype coating)

(define base-terrain (append void climates land fresh-water salt-water))

(define a (af am aw))
(define b (bwh bsh bwk bsk))
(define c (cfa cfb cfc csa csb cw))
(define d (dfa dfb dfc dfd dw))
(define e (et ef))

;; Level 2: Plant life

(terrain-type rainforest (image-name "forest")
  (help "A myriad assortment of plant species in a very dense arrangement")
  (notes "Rainforests depend on high temperatures and heavy rainfall, and so are only found in tropical wet and tropical monsoon climates."))
  
(terrain-type savanna-grass (image-name "plains") (name "savanna grass")
  (help "Grasslands of tropical wet-and-dry climates")
  (notes "A tropical wet-and-dry climate cannot support rainforests.  Such climates are typically populated by hardy grasses that are capable of surviving a dry season with up to about three months without rain."))
  
(terrain-type xerophytes (image-name "yellow")
  (help "Plant-life adapted to high evapotranspiration")
  (notes "In desert climates, evaporation tends to exceed precipitation, forcing anything living within it to survive with minimal water.  Xerophytes are plants adapted to such dry conditions.  The cactus is the classic example of a xerophyte."))
  
(terrain-type steppe-grassland (image-name "plains") (name "steppe grassland")
  (help "Short bunch grass, low brush, trees, and/or sagebrush")
  (notes "The steppes (semi-arid) climate is not as harsh as truly arid climate, but it still severely restricts the vegetation that can grow on it.  This vegetation is usually indigestible for humans, but it sustains herbivores that are hunted by humans or that are kept as livestock."))
  
(terrain-type evergreen-forest (image-name "forest") (name "evergreen forest")
  (help "Trees that keep their leaves during the winter")
  (notes "Evergreen forests are found in many climates.  The evergreen forests of the humid subtropical climates are mostly made up of pine trees, whereas those of the marine climates are dominated by fir trees.  The forests of humid continental climates are a mixture of pine, spruce and fir trees.  In subpolar regions, coniferous evergreen trees compose taiga forests that spread for hundreds of miles, creating some of the largest forests in the world."))
  
(terrain-type deciduous-forest (image-name "forest") (name "deciduous forest")
  (help "Trees that lose their leaves during the winter")
  (notes "Deciduous forests are found in humid subtropical and humid continental climates.  The most abundant of the deciduous trees are the oaks.  In subpolar regions, birch trees compose taiga forests similar to those of the coniferous evergreens, but they are less common."))
  
(terrain-type chaparral (image-name "green")
  (help "Scrubby, low-growing plants of the Mediterranean")
  (notes "Mediterranean climates have hot summers with dry seasons that can last as long as five months.  Chaparral vegetation, such as chamise, manzanita, and foothill pine, ranks among the few plants adapted to such an environment."))
  
(terrain-type tundra-vegetation (image-name "yellow")
  (help "Small plants adapted to extreme cold")
  (notes "Polar tundra is too cold even for the coniferous and birch forests of the subpolar regions, where temperatures barely rise above freezing during the short summer, liquid water is almost nonexistent, and cold winds sap the life out of anything that grows too tall.  Consequently, the sparse vegetation of the tundra consists of mosses, lichens, dwarf trees, and scattered woody vegetation.  Because of the icy winds, no tundra plants are taller than a few inches."))
  
; No plant life exists on the polar ice caps; it is the only climate that is truly desolate.

(define vegetation (rainforest savanna-grass xerophytes steppe-grassland evergreen-forest deciduous-forest chaparral tundra-vegetation))
(add vegetation subtype coating)

;; Level 3: Normal weather

(terrain-type rain (image-name "blue")
  (help "Normal rainfall"))
  
(terrain-type glaze (image-name "white")
  (help "Freezing rain"))
  
(terrain-type snow (image-name "white")
  (help "Loose ice crystals"))
  
(terrain-type hail (image-name "white")
  (help "Frozen raindrops"))
  
(define weather (rain glaze snow hail))
(add weather subtype coating)
  
;; Level 4: Ususual weather

(terrain-type fire (image-name "red")
  (help "Burning vegetation"))
  
(terrain-type tropical-storm (image-name "purple")
  (help "A mass of spinning water and high winds"))
  
(terrain-type thunderstorm (image-name "purple")
  (help "A large, potentially destructive storm"))
  
(terrain-type lava (image-name "red")
  (help "An extremely hot flow of molten rock"))
  
(terrain-type dust-storm (image-name "brown")
  (help "A whirling cloud of dust"))
  
(define special-weather (fire tropical-storm thunderstorm lava dust-storm))

; A single unit type is defined so that this module can be loaded on its own
; without crashing.

(unit-type human (image-name "person") (acp-per-turn 2) (acp-min -1) (start-with 1))

(define water (fresh-water salt-water))
(add water liquid true)

(set edge-terrain ef)

;;; TERRAIN PROPERTIES

; To keep things simple, allow a lot of creative liberty.

(add base-terrain elevation-min -10000)
(add base-terrain elevation-max 10000)

(add water elevation-max 0)

(table coating-depth-max
  ; While topographical coatings can appear just about anywhere, vegetation
  ; coatings are more restricted.
  
  (t* t* 0)				; By default
  
  ; (climates land 1)			; Climate is rather explicit.
  (vegetation climates 10)		; Allow 10 levels of vegetation per cell.
  (weather base-terrain 10)		; Allow 10 levels of weather per cell.
  (special-weather base-terrain 10)	; Likewise for unusual weather.
)

; To simplify things while debugging the day/night cycle, this can be commented
; out...

; Defaults...

(add t* temperature-min -500)
(add t* temperature-max 5000)

(add t* temperature-average 15)
(add t* temperature-variability 10)

; Wind should be controlled by temperature and elevations, but no such
; mechanism exists at this time.

(add t* wind-force-min 0)
(add t* wind-force-max 1000)

; These temperatures are given in degrees Celsius, averaged for an entire year.
; Seasonal variations must be implemented separately.
; I make no claims as to the accuracy of these numbers!

; Temperatures should be affected by seasons and by day/night cycles.  So far,
; I can only do the former, and I haven't even tried yet.

(add af temperature-average 27)
(add af temperature-variability 5)

(add am temperature-average 27)
(add am temperature-variability 5)

(add aw temperature-average 25)
(add aw temperature-variability 3)

(add bwh temperature-average 21)
(add bwh temperature-variability 22)

(add bwk temperature-average 3)
(add bwk temperature-variability 75)

(add bsh temperature-average 28)
(add bsh temperature-variability 20)	; WARNING: Made-up stat!

(add bsk temperature-average 10)
(add bsk temperature-variability 25)

(add cfa temperature-average 19)
(add cfa temperature-variability 16)

(add cfb temperature-average 8)
(add cfb temperature-variability 12)

(add csa temperature-average 15)
(add csa temperature-variability 17)

(add csb temperature-average 14)
(add csb temperature-variability 6)

(add cw temperature-average 20)		; WARNING: Made-up stat!
(add cw temperature-variability 15)	; WARNING: Made-up stat!

(add dfa temperature-average 9)
(add dfa temperature-variability 31)

(add dfb temperature-average 3)
(add dfb temperature-variability 38)

(add dfc temperature-average -3)
(add dfc temperature-variability 39)

(add dfd temperature-average -13)	; WARNING: Made-up stat!
(add dfd temperature-variability 39)

(add dw temperature-average -13)	; WARNING: Made-up stat!
(add dw temperature-variability 39)

(add et temperature-average -12)
(add et temperature-variability 32)

(add ef temperature-average -30)
(add ef temperature-variability 36)

; (add glacier temperature-average -30)
; (add glacier temperature-variability 36)

(set temperature-moderation-range 1)	; Kind of low, but high values use a *lot* of CPU power!

; temperature-year-cycle only works for pre-defined maps!

(add t* wind-force-min 1)
(add t* wind-force-average 1)
(add t* wind-force-max 4)
(add t* wind-force-variability 50.00)
(add t* wind-variability 50.00)

(set wind-mix-range 1)	; Kind of low, but high values use a *lot* of CPU power!

; Cannot set speed-wind-effect without knowing the nature of the affected unit!

; Just to see if clouds work at all (not that they're at all useful)...

#| And they don't!
(add t* clouds-min 0)
(add t* clouds-max 1)
|#

;;; SYNTHESIS

; This is grossly over-simplified!

(add t* wet-percentile-min 0)
(add t* wet-percentile-max 0)

(add t* alt-percentile-min 0)
(add t* alt-percentile-max 0)

(add salt-water wet-percentile-min 0)
(add salt-water wet-percentile-max 30)

(add fresh-water wet-percentile-min 30)
(add fresh-water wet-percentile-max 32)

(add land wet-percentile-min 32)
(add land wet-percentile-max 100)

; (add glacier wet-percentile-min 98)
; (add glacier wet-percentile-max 100)

(add base-terrain alt-percentile-min 0)
(add base-terrain alt-percentile-max 100)

; To create something that resembles climate, we'll try making semi-random
; changes to the map!

(table terrain-density
  (land climates 0.10)
  (void salt-water 100.00)	; A work-around for a faulty (?) application of
  				; make-fractal-percentile-terrain
)

; ***  DANGER!  DANGER!  ***
(table adjacent-terrain-effect-passes
  (t* t* 32767)
)

(table adjacent-terrain-effect
  (land af af)
  (land am am)
  (land aw aw)
  (land bwh bwh)
  (land bsh bsh)
  (land bwk bwk)
  (land bsk bsk)
  (land cfa cfa)
  (land cfb cfb)
  (land cfc cfc)
  (land csa csa)
  (land csb csb)
  (land cw cw)
  (land dfa dfa)
  (land dfb dfb)
  (land dfc dfc)
  (land dfd dfd)
  (land dw dw)
  (land et et)
  (land ef ef)
; (land h h)
  (water t* non-terrain)
  
  (land water cfa)	; This is a gross oversimplification!
  
  ; Try to prevent climate zones from being placed illogically.
  
  ; This is inefficient, but it should work.
  
  (aw b land)
  (am b land)
  (d b land)
  (e b land)
  
  (a d land)
  (a e land)
  (b aw land)
  (b am land)
  (b e land)
  (c e land)
  (d a land)
  (d b land)
  (e a land)
  (e b land)
  (e c land)
  
  ; Note that the following are pure speculation on my part!
  
  #|
  (af bwh bsh)
  (af bwk bsh)
  (af bsh aw)
  (af bsk aw)
  (af csa csb)
  (af dfa cfa)
  (af dfb cfc)
  (af dfc cfb)
  (af dfd csa)
  (af dw bsk)
  (af et dfa)
  (af ef dfd)
  
  (am bwh bsh)
  (am bwk bsk)
  (am bsh cfa)
  (am bsk cfa)
  (am dfa cfa)
  (am dfb cfc)
  (am dfc cfb)
  (am dfd csa)
  (am dfd bsk)
  (am et dfb)
  (am ef dw)
  
  (aw bwh bsh)
  (aw bwk bsh)
  (aw cfa csa)
  |#
  
)

(table adjacent-terrain-effect-chance
  (land climates 100.00)
  (climates climates 50.00)
  (land water 100.00)
)
