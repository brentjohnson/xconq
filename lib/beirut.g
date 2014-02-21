(game-module "beirut"
  (title "Beirut 1982")
  ; (version "1.0")
  (blurb "The heroic fighters of Beirut.")
  (variants
   (world-seen true)
   (see-all false)
   (sequential false)
   )
  )

; death squads shouldn't retreat
; no capturing of leader if they are win/lose condition.

(unit-type militia (image-name "trident-riflemen")
  (help "Hide and fight from building to building."))
(unit-type |death squad| (image-name "trident-fanatics")
  (help "Used for assassinating leaders and warlords."))
(unit-type warlord (image-name "trident-diplomat") (char "L")
  (help "The stuff that leaders are made of. A necessary evil in this game."))
(unit-type |car bomb| (image-name "auto-green") (char "C")
  (help "Destroys buildings and all else in the vicinity."))
(unit-type tank (image-name "trident-armor") (char "T")
  (help "Serious weaponry. Destroys buildings and other things."))
(unit-type wreckage (image-name "city20-fire") (char "W")
  (possible-sides "independent")
  (description-format ("wreckage at " position))
  (help "A disaster, but still OK for hiding out."))
(unit-type building (image-name "city20") (char "B")
  (description-format (side " building at " position))
  (help "Good for hiding out."))

(terrain-type sea (char "."))
(terrain-type beach (image-name "adv-desert") (char ","))
(terrain-type street (image-name "flagstone") (char "+")
  (help "Avenues and boulevards."))
(terrain-type block (image-name "lime-green") (char "^")
  (help "A city block, a mix of houses and narrow streets."))
(terrain-type fields (image-name "plains") (char "=")
  (help "Open flat ground."))
(terrain-type trees (image-name "adv-forest") (char "%"))

(add sea liquid true)

(set no-indepside-ingame true)

(define m militia)
(define d |death squad|)
(define L warlord)
(define C |car bomb|)
(define T tank)
(define W wreckage)
(define B building)

(define movers ( m d L C T ))
(define walkers ( m d L ))
(define vehicles ( C T ))
(define water ( sea ))
(define land ( beach street block fields trees ))

;;; Static relationships.

(set self-required true)		;	Require self units.
(add L can-be-self true)		;	Only warlords can be leaders.
(add L self-resurrects true)	;	Let the people pick a new leader if possible.

;;; Unit-unit capacities.

(add (W B) capacity (1 2))

(table unit-size-as-occupant
  (u* u* 99)
  (walkers (W B) 1)
  )

;;; Unit-terrain capacities.

(add t* capacity 4)

(table unit-size-in-terrain
  (u* t* 1)
  ((W B) t* 4)
  )

;;; Vision.

(add u* vision-range 4)
;; Tanks have optics to see further.
(add T vision-range 8)

(table visibility
  ;; Death squads are sneaky.
  (d t* 0)
  ;; Leaders are well-known, so can't hide in crowds on the street,
  ;; but can take cover in some kinds of terrain.
  (L (block trees) 0)
  )

(add u* can-see-behind false)

(table eye-height
  ;; Everybody can at least stand up to look around.
  (u* t* 6)
  ;; One can stand on top of a tank to see further.
  (T t* 12)
  ;; Buildings are multi-story, offer good viewing.
  (B t* 50)
  )

(add block thickness 30)
(add trees thickness 20)

(add (W B) see-always true)

(set terrain-seen true)

;;; Actions.

;                          m d L C T
(add movers acp-per-turn ( 2 4 4 6 6 ))

;;; Movement.

(add (W B) speed 0)

(table mp-to-enter-terrain
  (u* t* 99)
  ;; People can go anywhere.
  (walkers land 1)
  ;; Vehicles need flatness.
  (vehicles (beach street) 1)
  ;; Tanks can go cross-country.
  (T (fields block) (1 2))
  )

(table can-enter-independent
  ;; Anybody on foot can duck into buildings or wreckage.
  (walkers (W B) true)
  ;; ...but militia will always want to take and keep it.
  (m B false)
  )

;;; Combat.

;                m d L C T W  B
(add u* hp-max ( 9 1 1 1 12 1 10))

(table acp-to-attack
  ;; need this so clicking on the target detonates instead of
  ;; attempting an overrun
  (C u* 0)
  )

(table hit-chance
  ;        m  d  L  C  T  W  B
  (m u* ( 50 50 20 50 30  0 50 ))
  (d u* (  5 50 70 20 10  0  0 ))
  (L u* (  0 20 50 10 10  0  0 ))
  (C u* 0)
  (T u* ( 90 90 30 90 90  0 90 ))
  (W u* 0)
  (B u* ( 10 10 10 10  0  0  0 ))
  )

(table damage
  (u* u* 1)
  (u* W 0)
  (T m 2d2)
  (W u* 0)
  )

(add T acp-to-fire 1)

(add T range 8)

(add T fire-angle-max 10)

(table weapon-height
  (u* t* 0)
  (T t* 6)
  )

(table body-height
  (u* t* 6)
  ;; (make foot units "taller" in blocks?)
  )

(table capture-chance
  (m (C T B) (80 50 90))
  )

(table withdraw-chance-per-attack
  (u* m 20)
  (u* d 80)
  (u* L 95)
  (u* C 50)
  (u* T 25)
  ;; Car bombs are always surprises.
  (C u* 0)
  )

(table protection
  ((W B) (m d) 50)
  (m B 10)
  )

;; A destroyed building becomes wreckage; not worth owning, but still
;; an obstacle to movement.

(add B wrecked-type W)

;; Car bombs do their work by detonation.

(add C acp-to-detonate 1)

(add C hp-per-detonation 1)

(table detonation-unit-range
  (C u* 2)
  )

(table detonation-damage-at
  (C u* 10)
  (C W 0)
  )

(table detonation-damage-adjacent
  (C u* 8)
  (C T 1)
  (C W 0)
  )

(table detonate-on-hit
  ;       m  d  L   C   T   W   B
  (C u* (50 50 50 100 100 100 100))
  )

(table detonate-on-capture
  ;; Car bombs often have booby traps in them.
  (C u* 30)
  )

(table detonation-accident-chance
  ;; Car bombs are dangerous to handle.
  (C t* 4)
  )

;; Militia bounce back if they take a moment to recuperate.

(add m hp-recovery 0.30)

;;; Random events.

;; Buildings may always revert to their original sides.

(add B revolt-chance 50)

;; Buildings may surrender to nearby units, including
;; other buildings.

(table surrender-chance
  ((m L T B) B (10.00 40.00 20.00 5.00))
  )

(table surrender-range
  (u* u* 1)
  )

(table occupant-escape-chance
	(u* u* 100)
)

(table acp-for-retreat
	((m d L C T)  u* (1 1 1 1 1))
)

;;; Scoring.

;; The AI won't resign with this scorekeeper (scorekeeper code in
;; determine_subgoals in mplayer.c), so stick with last-side-wins for
;; now.
;;(scorekeeper
;;  (title "Points")
;;  (initial 0)
;;  (do (set (sum point-value)))
;;  )
(scorekeeper (do last-side-wins))

;; The faction's leader is most valuable.

(add L point-value 25)

;; Control of intact buildings is the staple of scoring though.

(add B point-value 1)

;; Units on a losing side disappear, except for buildings.

(add B lost-vanish-chance 0)
(add B lost-revolt-chance 100.00)

;;; Texts.

(set action-notices '(
  ((destroy d leader) (actor " assassinates " actee "!"))
  ((destroy u* leader) (actor " kills " actee "!"))
  ((destroy u* building) (actor " demolishes " actee "!"))
  ))

;;; Setup.

;                   m d L C T W B
(add u* start-with (6 2 5 5 4 0 5))

(table independent-density (B block 8000))

(table favored-terrain
  (u* t* 0)
  (movers street 100)
  (B block 100)
  )

(set country-radius-min 5)
(set country-separation-min 8)
(set country-separation-max 12)

(add street country-terrain-min 15)
(add block country-terrain-min 1)

(add (W B) assign-number false)

;; Don't let this go on forever.

(set last-turn 100)

(set side-library '(
((noun "Maronite")(emblem-name "flag-lebanon")(class "locals")(unit-namers (warlord "maronites")))
((name "the Amal")(adjective "Amal")(emblem-name "flag-amal")(class "locals")(unit-namers (warlord "amal")))
((noun "Hezbollah")(emblem-name "flag-hezbollah")(class "locals")(unit-namers (warlord "hezbollah")))
((name "the Druze")(adjective "Druze")(emblem-name "flag-druze")(class "locals")(unit-namers (warlord "druze")))
((noun "Syrian")(emblem-name "flag-syria")(class "locals")(unit-namers (warlord "syria")))
((noun "Israeli")(emblem-name "flag-israel")(class "locals")(unit-namers (warlord "israel")))
((name "the PLO")(adjective "Palestinian")(emblem-name "flag-plo")(class "locals")(unit-namers (warlord "plo")))))

(namer maronites (in-order "Bashir Gemayel" "Amin Gemayel" "Sulayman Franjieh" "Michel Aoun" "Elie Hobeika"))
(namer amal (in-order "Nabih Berri" "Husayn al-Husayni" "Dawud Dawud" "Mustafa Nasir" "Saad Haddad"))
(namer hezbollah (in-order "Sheikh Fadlallah" "Abbas Musawi" "Imad Mughniyah" "Hassan Nasrallah" "Sobhi Tufaili"))
(namer druze (in-order "Walid Jumblatt" "Nadim al Hakim" "Bahjat Ghayth" "Talal Arslan" "Tawfiq Arslan"))
(namer syria (in-order "Hafez al-Assad" "Rifaat al-Assad" "Ghazi Kanaan" "Abdel Khaddam" "Mustafa Tlass"))
(namer israel (in-order "Menachem Begin" "Ariel Sharon" "Moshe Arens" "Yitzhak Shamir" "Raful Eitan"))
(namer plo (in-order "Yasser Arafat" "Abu Jihad" "Abu Iyad" "Saad Sayel" "Abu Hassan Salameh"))

(add movers possible-sides "locals")

; Give old indepside a new adjective.

(side 0 (adjective "empty"))

(set sides-min 7)
(set sides-max 7)

(world 100000)

;; Distances are in feet.

(area 72 42 (cell-width 150))

(area (terrain
  (by-name
    (sea 0) (beach 1) (street 2) (block 3) (fields 4)
    (trees 5))
  "72a"
  "72a"
  "72a"
  "15ab3d6b4a2bc2dc2dbd4bc2dc2dc4bc17a"
  "9abc5bc4d2c2dc2d4c2dc2dc2dc2dc2dc2dc2dc2dc16a"
  "8a2b3c3dc2d3cd6c3d6c3d16c15a"
  "7ab2dc2dc2d4cdc3dc2dc3dedc2dcd3cdc2dc2dc2dc2dcdc14a"
  "7ab2dc2d2cdc2dc2dc2dc2dc3dedc2dcdc3dc2dc2dc2dcdfc2dc13a"
  "7ab3c2dcd2c2d14ce10c2d11cd2c12a"
  "7ab2dc2dc2dc2dc2dcdc3dcf2c2df2dc2dc2dc2dc2dc2dc2dcd2cde11a"
  "7ab3d10cdc2dc2dc2dc2de2dc2dc2dc2dc2dc3dcdc2dcdfe10a"
  "8ab3c3dc3dcd5c2d7cf7cd8cdc2d4cf2e9a"
  "8ab3dc2dc3dc2dc2dc2dc2dc2dcfdcdc3dc2dcdc3d2c4dcdcd2e8a"
  "9a4dcd5c2dc2d4c2dc2dcfdc2dc2dc2dc2dc2dc2dc2dcfd4e7a"
  "9ab2cd4c2d7c2d8cd2c2dcd6cd11cefefc6a"
  "10abd2c2dc2dc2dcd2c2dc2dc2dc2dedcdcdc5dc5dc2dcfdcde2ce5a"
  "10abdcd3c2dc2dc2dc2dc2dc2dc2de2d2cdc2dc2dc2df2dc2dc2dcefcfde4a"
  "11abcdcd14c2d6cedcd11cd3cd4c2ecf3e3a"
  "11abd2cdc2dc2dcdc3dcd4c3dcdec2dc2dc2dc2dcdc2d2c2dcdecd4e2a"
  "12abdc2dcdc2dc2dc2dc5dc2dcdfc2dc2dc3dcdc2dc2dc2dc2dcf2ef2ea"
  "13ab16c4d6cf8c2d14c3efefe"
  "13adcdc2dc2dc2dcdc2dc2d2cdc3df2dc2dc2dc2dc2dc2dcdcdfdcdf2ef2e"
  "14adcdcdc2dcd2c2dc2dcdc2dc3dc2dc3dcdc2dcd2c2dc2dc2dc7e"
  "15abcd4c2dcd16ce3cdc2d3cdc2d8cd2fef2e"
  "16adcdc2dcdc3dc2dcdc3dc2dc2de2dc2dc2dc2dc2dcdc3dcdf5e"
  "17ab2c2dc2dc3dcdc2dc2dc2dc3dedc2d2cfc2dcd2c2dc2dcd3ef2e"
  "18ab13c2d4c2d10cd7cdcdcd4c5e"
  "19a3dc2dc5dc2dcdc3dc2dcdec2dc5dc2dc2dc2dc2dcfd2e"
  "20a2dc3d2c3dcd2c2dc2dc2dcdfc2dc2dc2dcd2c2dc2dc2d3cfe"
  "22ad2b2cd2cd3c3d8cf10cdcdcd7cfece"
  "25a2b3d2c2d2cdcdc3dcd2c2dc2dc2dcdc2d2c2dc2dcd2ec"
  "27a2b2dc2dcd2c2dc3dcdcedc2dc2dcdc3dc2dc2dc2def"
  "30a3cdc2d8cd2cf4c2dc2d2cd10ce"
  "31abd2c2dc2dc5dc2df2dc3dc3d2c2dc2dc2dce"
  "32abdc3dcdc5dc2dc2dc2d3c3dc2dc2dc3dc"
  "34a7c2d2cd4cf5c2d14c"
  "35a2dc2dc2dcd2cd2cdfc2dc2dc5dc2dc3dc"
  "36adc2dc2dcdcdcdc2de2dc2dc5dc2dc3dc"
  "37ab8c2d4ce14cd4c"
  "37ab2dc2dc2dc2dc2dcedc2dc2dc2dc2dc3dc"
  "38a2dc2dc2dc2dc2dc2dc2dc2dc2dc2dc3dc"
  "38a34c"
))

;;; Documentation.

(game-module (instructions (
  "Take control of the city and eliminate your rival factions."
  "A side loses when it has no leaders (warlords) left."
  "Try not to destroy too much of the city in the process."
  )))

(game-module (design-notes (
  "Actually, this includes only a subset of the actual participants.
  They all fight each other here; there should actually be some alliances."
  )))

(game-module (notes (
  "Relive the heroic struggles of the heroic factions fighting for"
  "the just and righteous cause of control of Beirut."
  )))
