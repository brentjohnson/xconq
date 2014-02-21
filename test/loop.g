(game-module "loop"
  (blurb "Thrash the mplayer")
  (title "Put the mplayer into an endless loop")
  (variants
    (see-all true)
    (world-size (40 20 360)))
)

(unit-type i (name "infantry") (char "i"))
(unit-type / (name "base") (char "/"))
(unit-type @ (name "city") (char "@"))

(add i image-name "soldiers")
(add / image-name "airbase")
(add @ image-name "city20")

(material-type fuel)
(material-type ammo)

(terrain-type plains (color "green") (image-name "plains") (char "+"))
(add plains alt-percentile-min   0)
(add plains alt-percentile-max 100)
(add plains wet-percentile-min   0)
(add plains wet-percentile-max 100)

(add (i / @) start-with (8 4 4))
(table favored-terrain 
  (u* t* 100)
  (/ t* 0)
)
(set country-separation-min 15)
(set country-separation-max 30)
(set country-radius-min 2)

(table productivity (u* t* 100))
(table unit-initial-supply 
  (/ m* 999) 
  (i m* (10 6))
)
(table unit-storage-x
  ((/ i) fuel (200 20))
  ((/ i) ammo (100 6))
)
(table base-production 
  (/ fuel 20)
  (/ ammo 10)
)
(table base-consumption (i fuel 1))
(table hp-per-starve (i fuel 1.00))
(table in-length 
  (u* m* 0)
  (/ m* -1)
)
(table out-length 
  (u* m* 0)
  (i m* -1)
)

(add i acp-per-turn 2)
(add i speed 100)
(table mp-to-enter-terrain 
  (u* t* 99)
  (i plains 1)
)
(table material-to-move (i fuel 1))

(table unit-size-as-occupant
  (u* u* 9999)
  (i / 1)
  (/ @ 1)
)
(add / capacity 4)
(add @ capacity 1)
(table occupant-max 
  (u* u* 0)
  (/ i 4)
  (@ / 1)
)
(table unit-size-in-terrain
  ((i / @) t* (4 16 16))
)
(add t* capacity 16)

(table damage 
  (u* u* 1)
  (u* @ 0)
)
(add (i / @) hp-max (2 5 20))
(table hit-chance (u* u* 50))
(table capture-chance  (i (/ @) 50))
(table protection (u* u* 50))
(table consumption-per-attack (u* ammo 1))
(table hit-by (u* ammo 1))
