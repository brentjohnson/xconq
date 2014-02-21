(game-module "gazala"
  (title "Gazala Battles in 1942")
  (version "1.0")
  (blurb "North Africa in May 1942. The British defend Tobruk against Rommel.")
  (variants (world-seen true)(see-all false)(sequential false))
  )

(unit-type inf-reg (name "infantry regiment")
  (help "infantry regiment (Italian only)"))
(unit-type inf-bde (name "infantry brigade")
  (help "infantry brigade (British only)"))
(unit-type inf-div (name "infantry division")
  (help "infantry division (Italian only)"))
(unit-type garrison (image-name "unit")
  (help "town garrison (British only)"))
(unit-type inf-mot-bde (name "motorized brigade") (image-name "inf-bde")
  (help "motorized infantry brigade (British only)"))
(unit-type inf-mot-div (name "motorized division") (image-name "inf-div")
  (help "motorized infantry division (Axis only)"))
(unit-type recon (image-name "cav-trp")
  (help "Recon detachment from German division"))
(unit-type light-div (name "light division") (image-name "pzgren-div")
  (help "German 90th Light Division"))
(unit-type armor-bde (name "armored brigade")
  (help "armored brigade (British only)"))
(unit-type armor-div (name "armored division")
  (help "armored division (German only)"))

(unit-type british-minefield (name "minefield(b)") (image-name "minefield")
  )
(unit-type axis-minefield (name "minefield(a)") (image-name "minefield")
  )
(unit-type british-box (name "box") (image-name "fort")
  (help "a fortified area"))
(unit-type british-supply-source (name "supply source(b)") (image-name "supply-depot")
  (help "connection to British supply sources"))
(unit-type axis-supply-source (name "supply source(a)") (image-name "supply-depot")
  (help "connection to Italian and German supply sources"))
(unit-type town (image-name "village")
  (help "a cluster of houses that passes for a town in this area"))
(unit-type large-town (image-name "town20")
  (help "Tobruk"))

(material-type supply-b
  (help "British supply"))
(material-type supply-a
  (help "Axis supply"))

(terrain-type sea
  (help "the sea, impassable to all"))
(terrain-type clear (image-name "desert")
  (help "open sandy/rocky terrain"))
(terrain-type escarpment (image-name "cliffs")
  (subtype border)
  (help "cliffs low or high, passable with difficulty"))
(terrain-type road
  (subtype connection)
  (help "paved road, good for mobility"))
(terrain-type track (image-name "dirt-road")
  (subtype connection)
  (help "basically a poor dirt road, only slightly better than open desert"))

(define nonmot-inf-types (inf-reg inf-bde inf-div garrison))
(define mot-inf-types (inf-mot-bde inf-mot-div))
(define inf-types (append nonmot-inf-types mot-inf-types))

(define armor-types (recon light-div armor-bde armor-div))

(define mech-types (append mot-inf-types armor-types))

(define combat-types (append inf-types armor-types))

(define minefield-types (british-minefield axis-minefield british-box))

(define supply-source-types (british-supply-source axis-supply-source))

(define british-types (inf-bde garrison inf-mot-bde armor-bde))

(add british-types possible-sides "British")

(define axis-types (inf-reg inf-div inf-mot-div recon light-div armor-div))

(add axis-types possible-sides "Axis")

(define town-types (town large-town))

(add sea liquid true)

(set no-indepside-ingame true)

;;; Static relationships.

(table vanishes-on
  ;; No naval element, all units stick to land.
  (u* sea true)
  )

;; Unit-unit capacities.

(add minefield-types capacity 1)
(add town-types capacity 1)

(table unit-size-as-occupant
  ;; Disable occupancy by default.
  (u* u* 100)
  ;; Units on each side can go into their own minefields.
  (british-types british-minefield 1)
  (british-types british-box 1)
  (axis-types axis-minefield 1)
  (garrison town-types 1)
  )

(table occupant-max (u* u* 99))

;;; Unit-terrain capacities.

(add t* capacity 1)

(table unit-size-in-terrain
  (u* t* 1)
  ;; Regiments too small to affect stacking limits.
  (inf-reg t* 0)
  ;; The "towns" here are microscopic in size.
  (town-types t* 0)
  )

;;; Unit-material capacities.

(table unit-storage-x
  (british-types supply-b 7)
  (british-supply-source supply-b 999)
  (axis-types supply-a 7)
  (axis-supply-source supply-a 999)
  (large-town m* 70)
  )

;;; Vision.

(set terrain-seen true)

(add supply-source-types already-seen 100)

(add town-types already-seen 100)

;; Minefields don't add anything to vision coverage.

(add minefield-types vision-range -1)

;; Nor do towns.

(add town-types vision-range -1)

;;; Actions.

(add inf-types acp-per-turn (24 24 24 0 32 48))
(add armor-types acp-per-turn (48 48 32 48))

;;; Movement.

;; Don't be too picky about movement allowances.

(add u* free-mp 4)

(table mp-to-enter-terrain
  (u* t* 99)
  (u* clear 4)
  ;; Takes all day to wiggle through a dropoff.
  (inf-types escarpment (24 24 24 0 32 48))
  (armor-types escarpment (48 48 32 48))
  (nonmot-inf-types (road track) 0)
  (mech-types road 0)
  (mech-types track 0)
  )

(table mp-to-leave-terrain
  (nonmot-inf-types (road track) 0)
  (mech-types road 0)
  (mech-types track 0)
  )

(table mp-to-traverse
  (nonmot-inf-types (road track) 2)
  (mech-types road 1)
  (mech-types track 2)
  )

(table can-enter-independent
  (british-types (british-minefield british-box) true)
  (axis-types axis-minefield true)
  )

(table zoc-range
  (combat-types combat-types 1)
  )

(table mp-to-enter-zoc
  ;; (should match acp? but then lose ability to attack)
  (combat-types combat-types 4)
  )

(table mp-to-leave-zoc
  (combat-types combat-types 8)
  (armor-types combat-types 24)
  )

(table mp-to-traverse-zoc
  ;; Once in contact with enemy units, can only move away.
  ;; (this doesn't actually work!)
  (combat-types combat-types 100)
  )

;;; Combat.

(add u* hp-max 1)

(add inf-reg hp-max 2)
(add (inf-bde inf-mot-bde armor-bde) hp-max 6)
(add (inf-div inf-mot-div) hp-max (8 16))
(add garrison hp-max 10)
(add light-div hp-max 20)
(add armor-div hp-max 30)
(add town-types hp-max 60)

(table acp-to-attack
  (u* u* 16)
  )

(table hit-chance
  (u* u* 50)
  ;; Mobile units are more effective against static ones.
  (axis-types garrison 70)
  (u* minefield-types 50)
  ;; Minefields don't hit back, but you can't go through
  ;; them as long as they're there.
  (minefield-types u* 0)
  (u* town-types 100)
  ;; Towns don't hit back.
  (town-types u* 0)
  ;; Attacking a supply source is more of an interdiction;
  ;; it's unlikely, though not impossible to encounter any resistance.
  (supply-source-types u* 10)
  )

(table damage
  (u* u* 1)
  ;; Supply sources are not real objects, can't actually be damaged.
  (u* supply-source-types 0)
  )

(table capture-chance
  ;; If you can get to the supply source, capture is certain.
  (u* supply-source-types 100)
  ;; Similarly for towns (unless garrisoned).
  (u* town-types 100)
  )

(table protection
  ;; Garrison must be eliminated before the town can be captured.
  (garrison town-types 20)
  ;; (other types should protect also?)
  )

(table stack-protection
  ;; Must clear the area before a town can be captured.
  (combat-types town-types 0)
  )

;;; Backdrop.

(table base-production
  (british-supply-source supply-b 50)
  (axis-supply-source supply-a 50)
  )

(table base-consumption
  (british-types supply-b 1)
  (axis-types supply-a 1)
  )

(table out-length
  (british-supply-source supply-b 30)
  (axis-supply-source supply-a 30)
  ;; Tobruk can help forward supplies to the front.
  (large-town m* 30)
  )

(table in-length
  (british-types supply-b 30)
  (axis-types supply-a 30)
  (large-town m* 30)
  )

(table hp-per-starve
  (british-types supply-b 1.00)
  (axis-types supply-a 1.00)
  )

(table unit-initial-supply
  (u* m* 999)
  )

;;; Scoring.

(scorekeeper (do last-side-wins))

(add u* point-value 0)
(add british-types point-value 10)
(add light-div point-value 30)
(add armor-div point-value 30)
(add minefield-types point-value 0)
(add town point-value 0)
(add large-town point-value 400)

;;; Sides.

(side 1 (noun "British") (class "British") (emblem-name "flag-uk")
  (already-seen-independent (axis-minefield 50) (british-minefield 100) (british-box 100))
  )

(side 2 (noun "Axis") (class "Axis") (emblem-name "german-cross")
  (already-seen-independent (axis-minefield 100) (british-minefield 50) (british-box 80))
  )

(set sides-min 2)
(set sides-max 2)

;;; Dates.

(set calendar '(usual day))

(set initial-date "27 May 1942")

;;; Info.

(game-module (instructions (
  "The Germans/Italians attempt to capture Tobruk, the British defend."
  ""
  "While the British have elaborate fixed defenses, the Afrika Korps is "
  "both fast and powerful, and will just go around those defenses."
  ""
  "Smaller towns have some value as strongpoints, but only Tobruk really "
  "matters; focus on it.  There are not enough units to cover all the "
  "terrain here, so you must always be patrolling, and always be ready "
  "to respond to a penetration."
  )))

;;; Terrain.

(area 60 40)

(area (terrain
 (by-name
    (sea 0) (clear 1) (escarpment 2))
  "60a"
  "3b57a"
  "7b53a"
  "10ba3b46a"
  "11ba11b37a"
  "27b33a"
  "33b27a"
  "33b27a"
  "34b26a"
  "37b23a"
  "40b20a"
  "52b8a"
  "53b7a"
  "54b6a"
  "55b5a"
  "56b4a"
  "57b3a"
  "58b2a"
  "59ba"
  "60b"
  "a59b"
  "2a58b"
  "3a57b"
  "4a56b"
  "5a55b"
  "6a54b"
  "7a53b"
  "8a52b"
  "9a51b"
  "10a50b"
  "11a49b"
  "12a48b"
  "13a47b"
  "14a46b"
  "15a45b"
  "16a44b"
  "17a43b"
  "18a42b"
  "19a41b"
  "20a40b"
))

(area (aux-terrain escarpment
  "60a"
  "60a"
  "60a"
  "3agq3ami50a"
  "3aoL3af?y49a"
  "amib<3agTn4mi44a"
  "ab=2a3moMb?M2=?}8mi34a"
  "5ab3=<6ab8=?}4m29a"
  "26ab3=AH28a"
  "30acM<2aoq23a"
  "35ab<moq20a"
  "23ae2mi10ab=<emae3mi12a"
  "23acM=?}2mi10a=<a4=12a"
  "18a2mi3ae2mpYIGq28a"
  "18ab=AmiagM4=?}4mi23a"
  "21aIE2a<5ab4=?}9mi12a"
  "21ab=14ab9=?q11a"
  "60a"
  "39ami19a"
  "39ab?}mi4ae8mi2a"
  "41ab=?}3moM8=2a"
  "44ab4=<10a"
  "60a"
  "60a"
  "60a"
  "60a"
  "60a"
  "60a"
  "60a"
  "60a"
  "60a"
  "60a"
  "60a"
  "60a"
  "60a"
  "60a"
  "60a"
  "60a"
  "60a"
  "60a"
  ))

(area (aux-terrain road
  "60a"
  "60a"
  "ac4su53a"
  "7a>u51a"
  "9a@7aksu40a"
  "10a>6sr2a>2su36a"
  "24aFu34a"
  "24aja>5sy27a"
  "24af7af27a"
  "25a@7a>u25a"
  "26a@8aH24a"
  "27a@7aj>2su20a"
  "28a@6aj4a>3s{su13a"
  "29a@4akr8aj2a>u11a"
  "30a>u2af9aj4a>su8a"
  "32a>uaD8aj7a>sq5a"
  "34a>t8sr15a"
  "60a"
  "60a"
  "60a"
  "60a"
  "60a"
  "60a"
  "60a"
  "60a"
  "60a"
  "60a"
  "60a"
  "60a"
  "60a"
  "60a"
  "60a"
  "60a"
  "60a"
  "60a"
  "60a"
  "60a"
  "60a"
  "60a"
  "60a"
  ))

(area (aux-terrain track
  "60a"
  "60a"
  "2ai57a"
  "2aj57a"
  "2aj6ai2ae47a"
  "acr6af3a<6ai39a"
  "10a@ak2su4aj6ai32a"
  "11a>v3a>3s~6aj2ai29a"
  "13a@5akr>su3af2ajai27a"
  "14aH3akr4a>{2sRsxsr27a"
  "ac5su5akrF2sr6aj3a@aD5ai22a"
  "8a>4sraj9af4a>|4sqb22a"
  "15aj10aDac2sr28a"
  "15aj10aj23aksq7a"
  "ac3su9an10af23ab9a"
  "6a>3su4aj@10aD32a"
  "11a>suaja>uak6s~32a"
  "14a>|w2sNv6aj>7s:2su20a"
  "5ak9sra>sua@5aj8aj@2a>9su9a"
  "ac3sr14a>sVsu2an7akraD12a>6sqa"
  "22aj2a>sz>u5aj2af21a"
  "22af4aj2a>u2akr3a@20a"
  "23a@3aj4a>uj5a@19a"
  "24aDakr4aksOu5aFsu16a"
  "24afaj3aksr3a>u3af2a>2su12a"
  "25aFz2akr7a>u2aD5a>su9a"
  "8ac16st|swr6ae3a>sv8a>su6a"
  "26aj2a>su5aD5aF2su7a>su3a"
  "26af5a>2swstu4aj3a>u8a>qa"
  "27aD8a<2a>3sz5a>u9a"
  "27af15aj7a>u7a"
  "28aD14aj9a>u5a"
  "28af14aj11a>2sqa"
  "29a@13aj16a"
  "30a@12aj16a"
  "31a@11aj16a"
  "32aD10aj16a"
  "32af10aj16a"
  "33a<9ab16a"
  "60a"
  ))

(inf-bde 22 21 1 (nb 150))
(inf-bde 16 26 1 (nb 151))
(inf-bde 13 28 1 (n "1SA"))
(inf-bde 45 12 1 (n "29Ind"))
(inf-bde 31 29 1 (n "4SA"))
(inf-bde 19 24 1 (n "69"))
(inf-bde 29 32 1 (n "6SA"))
(inf-bde 35 29 1 (n "9Ind"))
(inf-bde 18 33 1 (n "Seacol"))
(inf-bde 27 28 1 (n "Stopcol"))
(garrison 26 13 1 (n "1FF") (in "Bir Hacheim"))
(garrison 32 32 1 (in "Tobruk"))
(inf-mot-bde 27 24 1 (nb 201))
(inf-mot-bde 33 12 1 (n "3Ind"))
(inf-mot-bde 39 11 1 (n "7"))
(armor-bde 30 22 1 (nb 2))
(armor-bde 31 17 1 (nb 22))
(armor-bde 38 14 1 (nb 4))
(british-minefield 32 28 0)
(british-minefield 31 28 0)
(british-minefield 30 28 0)
(british-minefield 29 29 0)
(british-minefield 28 30 0)
(british-minefield 27 31 0)
(british-minefield 27 32 0)
(british-minefield 27 33 0)
(british-minefield 33 28 0)
(british-minefield 34 28 0)
(british-minefield 35 28 0)
(british-minefield 36 28 0)
(british-minefield 37 28 0)
(british-minefield 37 29 0)
(british-minefield 26 14 0)
(british-minefield 28 18 0)
(british-minefield 28 17 0)
(british-minefield 28 15 0)
(british-minefield 28 16 0)
(british-minefield 27 15 0)
(british-minefield 27 14 0)
(british-minefield 28 14 0)
(british-minefield 27 13 0)
(british-minefield 27 12 0)
(british-minefield 24 15 0)
(british-minefield 24 16 0)
(british-minefield 23 17 0)
(british-minefield 22 18 0)
(british-minefield 22 19 0)
(british-minefield 22 20 0)
(british-minefield 21 21 0)
(british-minefield 20 21 0)
(british-minefield 19 22 0)
(british-minefield 18 22 0)
(british-minefield 18 23 0)
(british-minefield 17 23 0)
(british-minefield 16 24 0)
(british-minefield 15 25 0)
(british-minefield 14 26 0)
(british-minefield 13 26 0)
(british-minefield 12 27 0)
(british-minefield 11 28 0)
(british-minefield 10 30 0)
(british-minefield 10 31 0)
(british-minefield 9 34 0)
(british-minefield 9 36 0)
(british-box 28 12 0)
(british-box 26 12 0)
(british-box 25 13 0)
(british-box 25 14 0)
(british-box 27 23 0)
(british-box 13 27 0)
(british-box 11 29 0)
(british-box 10 32 0)
(british-box 9 33 0)
(british-box 9 35 0)
(british-supply-source 52 26 1)
(british-supply-source 54 24 1)
(british-supply-source 58 20 1)
(british-supply-source 58 7 1)
(british-supply-source 58 11 1)
(town 25 30 1 (n "Acroma"))
(town 12 28 1 (n "Alem Hemza"))
(town 44 23 1 (n "Belhamel"))
(town 36 13 1 (n "Bir Beuid"))
(town 26 13 1 (n "Bir Hacheim"))
(town 32 22 1 (n "Bir Lefa"))
(town 43 12 1 (n "Bir el Gubi"))
(town 41 16 1 (n "Bir el Hafad"))
(town 27 20 1 (n "Bir el Harmat"))
(town 36 22 1 (n "El Adem"))
(town 41 23 1 (n "El Duda"))
(town 22 29 1 (n "Eluet et Tamar"))
(town 12 35 1 (n "Gazala"))
(town 36 10 1 (n "Retma"))
(town 22 23 1 (n "Sidi Muftah"))
(town 44 21 1 (n "Sidi Rezegh"))
(large-town 32 32 1 (n "Tobruk"))

(inf-reg 4 30 2 (nb 7))
(inf-reg 11 24 2 (nb 9))
(inf-div 13 24 2 (n "Brescia"))
(inf-div 18 20 2 (n "Pavia"))
(inf-div 4 34 2 (n "Sabratha"))
(inf-div 7 29 2 (n "Trento"))
(inf-mot-div 5 36 2 (n "15RB"))
(inf-mot-div 18 18 2 (n "Trieste"))
(recon 28 10 2 (n "15R"))
(recon 29 9 2 (n "21R"))
(recon 30 8 2 (n "90R"))
(light-div 29 8 2 (nb 90))
(armor-div 27 10 2 (nb 15))
(armor-div 28 9 2 (nb 21))
(axis-minefield 5 33 0)
(axis-minefield 5 34 0)
(axis-minefield 5 35 0)
(axis-minefield 6 35 0)
(axis-minefield 6 36 0)
(axis-minefield 6 37 0)
(axis-supply-source 1 20 2)
(axis-supply-source 1 25 2)
(axis-supply-source 1 29 2)
(axis-supply-source 1 37 2)
(axis-supply-source 1 34 2)
(town 5 29 2 (n "Bir Temrad"))
(town 15 22 2 (n "Rotonda Meifal"))

;; Addons to specific units.

(unit "Bir Hacheim" (point-value 100))
(unit "Gazala" (point-value 100))
