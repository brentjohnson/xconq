(game-module "mormon"
  (title "Mormon")
  ; (version "1.0")
  (blurb "The heroic age of the heroic Mormon pioneers.")
  (variants (world-seen true)(see-all false)(sequential false))
  )

(unit-type mormon (image-name "man")
  (possible-sides "mormon")
  (help "prolific settlers"))
(unit-type avenging-angel (name "avenging angel") (image-name "trooper") (char "A")
  (possible-sides "mormon")
  (help "Mormon police/army/vigilantes"))
(unit-type prophet (image-name "man") (char "P")
  (possible-sides "mormon")
  (help "prolific leader"))
(unit-type army (image-name "cavalry")
  (possible-sides "us")
  (help "ordered to keep Mormons under control"))
(unit-type indian (name "Indian") (image-name "archer") (char "u")
  (possible-sides "indian")
  (help "want to get rid of Mormons and cavalry"))
(unit-type fort (image-name "stockade") (char "/")
  (help "where the army hangs out"))
(unit-type settlement (image-name "log-cabin") (char "*")
  (help "where Mormons live"))
(unit-type temple (image-name "cathedral") (char "@")
  (help "where all good Mormons aspire to live"))

(material-type water (help "very scarce in Utah"))

(terrain-type lake (image-name "sky-blue") (char "."))
(terrain-type river (image-name "cyan") (char ","))
(terrain-type valley (image-name "plains") (char "+"))
(terrain-type forest (char "%"))
(terrain-type desert (char "~"))
(terrain-type mountains (char "^"))
(terrain-type salt-flat (image-name "white") (char "_"))
(terrain-type not-Utah (image-name "black") (char ":"))

(define places (fort settlement temple))

(add (lake river) liquid true)

;;; Static relationships.

(table vanishes-on
  (u* (lake not-Utah) true)
  )

(add (settlement temple) capacity 8)

(table unit-size-as-occupant
  (u* u* 100) ; disables occupancy usually
  ((mormon avenging-angel prophet) (fort settlement temple) 1)
  )

(table unit-capacity-x
  (fort army 6)
  ((settlement temple) army 1)
  )

;;; Unit-terrain capacity.

(add t* capacity 4)

(table unit-size-in-terrain
  (u* t* 1)
  ((settlement temple) t* 4)
  )

;;; Vision.

(add places already-seen 100)

;;; Unit-material capacities.

(table unit-storage-x
  (u* water 2)
  (army water 4)
  (places water 100)
  )

;;; Actions.

;;;; scale should be 15 km?

(add (mormon avenging-angel army indian prophet) acp-per-turn (2 4 4 4 6))

(add places acp-per-turn 1)

;;; Movement.

(add places speed 0)

(table mp-to-enter-terrain
  ;; Nobody will go into deadly terrain by accident.
  (u* (lake not-Utah) 99)
  )

;;; Construction.

(add (army indian) cp 4)

(add places cp (5 10 20))

(table can-create
  (prophet mormon 6)
  (prophet temple 6)
  (mormon mormon 2)
  (mormon settlement 2)
  (army fort 4)
  (fort army 1)
  (indian indian 4)
  (settlement mormon 1)
  (temple avenging-angel 1)
  )

(table can-build
  (prophet mormon 6)
  (prophet temple 6)
  (mormon mormon 2)
  (mormon settlement 2)
  (army fort 4)
  (fort army 1)
  (indian indian 4)
  (settlement mormon 1)
  (temple avenging-angel 1)
  )

(table acp-to-create
  (prophet mormon 6)
  (prophet temple 6)
  (mormon mormon 2)
  (mormon settlement 2)
  (army fort 4)
  (fort army 1)
  (indian indian 4)
  (settlement mormon 1)
  (temple avenging-angel 1)
  )

(table acp-to-build
  (prophet mormon 6)
  (prophet temple 6)
  (mormon mormon 2)
  (mormon settlement 2)
  (army fort 4)
  (fort army 1)
  (indian indian 4)
  (settlement mormon 1)
  (temple avenging-angel 1)
  )

(table supply-on-creation
  ;; Minimal water supply is free.
  (u* water 1)
  )

;;; Production (water).

(table base-production
  (u* water 1)
  (places water 3)
  )

(table productivity
  (u* t* 0)
  (u* (valley river) 100)  ; note that the lake is salt, so useless
  (u* forest 200)
  (army mountains 50)
  ;; Indians can get water anywhere but in salt flats.
  (indian t* 100)
  (indian salt-flat 0)
  (u* valley 100)
  )

(table base-consumption
  (u* water 1)
  )

(table hp-per-starve
  ;; Might be able to do without water, but not for long.
  (u* water 0.50)
  )

;;; Combat.

(add u* hp-max (4 4 8 4 2 16 16 16))

(table hit-chance
  (u* u* 50)
  (places u* 0)
  (mormon army 5)
  ;; armies and indians are deadly.
  ((army indian) mormon 95)
  ;; Prophet is wily.
  ((army indian) prophet 50)
  (army avenging-angel 30)
  ;; avenging angels are best way to hit back.
  (avenging-angel army 50)
  )

(table damage
  (u* u* 1)
  (army u* 1d2)
  ;; indians can be very destructive sometimes.
  (indian u* 1d4)
  )

(table withdraw-chance-per-attack
  ;; OK, so they weren't that heroic...
  (u* u* 10)
  )

(table hp-min
  ;; Temple is made of stone, can only be destroyed by the army or indians
  (u* temple 4)
  ((army indian) temple 0)
  )

(table capture-chance
  (army places (10 10 5))
  (avenging-angel places 5)
  ;; Give the civilians a small chance of success.
  (mormon places 1)
  )

(table stack-protection
  ;; Can't capture forts if the army is around.
  (army fort 0)
  )

(add u* hp-recovery 1.00)
(add army hp-recovery 0.50)

;;; Initial setup.

(add u* initial-seen-radius 4)

(table unit-initial-supply
  (u* water 9999)
  )

;;; Scoring.

(scorekeeper (do last-side-wins))

;;; Text.

(set action-notices '(
  ((destroy u* mormon) (actor " kills " actee "!"))
  ((destroy u* avenging-angel) (actor " kills " actee "!"))
  ((destroy u* prophet) (actor " kills " actee "!"))
  ((destroy u* indian) (actor " kills " actee "!"))
  ))

(set event-notices '(
  ((unit-starved (mormon avenging-angel a)) (0 "dies of thirst"))
  ))

(set event-narratives '(
  ((unit-starved (mormon avenging-angel a)) (0 "died of thirst"))
  ))

(world 50)  ; hexagon would be better

(area 50 42)

(area (terrain
  "50h"
  "6ecfcfc2af33h3e"
  "2f5ecfcf2a2f32h2ef"
  "8ecfcfafd33h2e"
  "4e2a3ecfc2fdf32hef"
  "e2g2e2a2ecf2c2fd33he"
  "fe2ge3aeacf2cfdf32he"
  "2e3ge4aecf2cfd33h"
  "fe4ge3aecf3cdf32h"
  "he3gfge3aecf4c10dc3e18h"
  "he3gfge2ae2acf3c2d7f2dc2e18h"
  "2he5g2e3aecf2cd10fce2b17h"
  "2h2e4g3ea2ecf3c3d6fc2ebe17h"
  "3he3g3ef2efcf2cf3dc5ece2b2e16h"
  "3h2e3g3efefecfc2d3c7eb3e16h"
  "4h2e2g3ef2efacfcfdf9eb3e15h"
  "4h4eg3efefeacfcfd2f7eb3e15h"
  "5h7ef3eacfc2fd4f4ebe3f14h"
  "5h4ef7ecfc2efce4f2ebefdf14h"
  "6h4ef4efecfefefece4febefd2f13h"
  "6h2ef2ef4efcef2efe2c3efebef2df13h"
  "7h2efef6efefef5ec2ebefd3f12h"
  "7h2ef2ef2e2cef2efef8ebe2f2e12h"
  "8h2efef2e3c2ef2e2f5ecebe2f2eb11h"
  "8h2ef4e3cecfce2f7ebe2f2eb11h"
  "9h5ea6ece2f7ecbefe2be10h"
  "9h3efea5efc3ef7eb3eb2e10h"
  "10h2efea4ecfecfef8eb2eb3e9h"
  "10hefef2ef5ec11eb2ebefe9h"
  "11hefefef4e2f6ef5ebeb2efe8h"
  "11hef2ef4ec2ef5e2f5e2b2efe8h"
  "12hefe2f4ef2e2f4ef5eb6e7h"
  "12h2e2f6efef2ef2e2f4eb6e7h"
  "13h2f5ec2f5ef2ef3e2b3e2f2e6h"
  "13h5ec2e2f5ef2ef2e2b4e3fe6h"
  "14h6ec2f11eb6e2f2e5h"
  "14h3ef3ef11e2b10e5h"
  "15h2efe2cf11ebe5b6e4h"
  "15h2efec11e3b6e6b4h"
  "16h3ec11eb15e3h"
  "16h10ec4eb2e2f11e3h"
  "50h"
))

(set sides-min 3)
(set sides-max 3)

(side 1 (name "US") (emblem-name "flag-usa")
  (class "us"))

(side 2 (noun "Mormon") (emblem-name "none")
  (class "mormon"))

(side 3 (noun "Ute") (adjective "Ute") (emblem-name "none")
  (class "indian"))

(army 14 31 1 (in "Ft Douglas"))
(army 15 31 1)
(army 16 31 1)
(army 16 32 1)
(army 17 32 1)
(army 18 32 1)
(army 28 28 1 (in "Ft Duchesne"))
(fort 14 31 1 (n "Ft Douglas"))
(fort 28 28 1 (n "Ft Duchesne"))

(temple 14 30 2 (n "Salt Lake City"))
(prophet 14 30 2 (n "Brigham Young") (in "Salt Lake City"))
(avenging-angel 14 30 2 (in "Salt Lake City"))
(mormon 14 30 2 (in "Salt Lake City"))

(settlement 15 27 2 (n "Orem"))
(settlement 17 25 2 (n "Provo"))
(settlement 14 29 2 (n "Sandy"))
(settlement 12 32 2 (n "Ogden"))

(indian  9 39 3)
(indian  9 37 3)
(indian 14 37 3)
(indian 14 34 3)
(indian 22 28 3)
(indian 18 26 3)
(indian 17 23 3)
(indian 16 18 3)
(indian 21 16 3)

(game-module (instructions (
  "The Mormons should try to reproduce themselves "
  "and spread out as much as possible "
  "before the cavalry catches up with them."
  "Use the Avenging Angels to ambush the cavs."
  ""
  "The US cavalry just has to kill as many Mormons "
  "as possible, as fast as possible."
  ""
  "The Indians want both other parties gone, the "
  "sooner the better.  They should also work to "
  "increase their numbers before tangling with the "
  "other sides."
  )))

(game-module (notes (
  "Can the US army kill all the Mormons before they overpopulate Utah?"
  ""
  "(This is all a joke of course.)"
  )))

(game-module (design-notes (
  )))
