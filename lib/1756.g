(game-module "1756"
  (title "Europe 1756")
  (version "0.9")
  (blurb "Central Europe at the beginning of summer 1756.  Frederick must strike or die.")
  (base-module "fred")
  (variants
  	(world-seen true)
  	(see-all false)
  	(sequential false)
  )
  (instructions (
   "Each side should attempt to capture as many of the other's fortresses "
   "as possible.  "
   ))
)

(include "t-cent-eur")

;;; Modify the defaults in fred.

(add u* start-with 0)

;;; All scenarios include these sides, but only three were active in 1756.

(side 1 (name "Prussia") (adjective "Prussian") (emblem-name "arms-prussia"))
(side 2 (name "Hannover") (adjective "Hannoverian") (emblem-name "arms-hannover")
  (active false))
(side 3 (name "Austria") (adjective "Austrian") (emblem-name "arms-austria"))
(side 4 (name "Saxony") (adjective "Saxon") (emblem-name "arms-saxony"))
(side 5 (name "France") (adjective "French") (emblem-name "arms-france")
  (active false))
(side 6 (name "Russia") (adjective "Russian") (emblem-name "arms-russia")
  (active false))
(side 7 (name "Sweden") (adjective "Swedish") (emblem-name "arms-sweden")
  (active false))
(side 8 (name "Empire") (adjective "Empire")
  (active false))

;;; Austria and Saxony are allies.

(side 3 (trusts (3 1) (4 1)))
(side 4 (trusts (3 1) (4 1)))

(set sides-max 8)

(town 13 28 0 (n "Bremen"))
(town 18 19 0 (n "Cassel"))
(town 29 16 0 (n "Chemnitz"))
(town 35 35 0 (n "Danzig"))
(town 29 6 0 (n "Donauworth"))
(town 31 12 0 (n "Eger"))
(town 8 30 0 (n "Emden"))
(town 12 32 0 (n "Hamburg"))
(town 27 18 0 (n "Leipzig"))
(town 36 5 0 (n "Passau"))
(town 36 25 0 (n "Posen"))
(town 32 6 0 (n "Regensburg"))
(town 38 29 0 (n "Thorn"))
(town 27 4 0 (n "Ulm"))
(town 24 11 0 (n "Wurzburg"))

(fortress 25 26 0 (n "Berlin"))
(fortress 15 14 0 (n "Coblenz"))
(fortress 29 34 0 (n "Colberg"))
(fortress 12 17 0 (n "Cologne"))
(fortress 11 19 0 (n "Dusseldorf"))
(fortress 20 13 0 (n "Frankfort"))
(fortress 17 25 0 (n "Hannover"))
(fortress 38 38 0 (n "Konigsberg"))
(fortress 30 25 0 (n "Kustrin"))
(fortress 18 12 0 (n "Mainz"))
(fortress 15 8 0 (n "Metz"))
(fortress 14 24 0 (n "Minden"))
(fortress 11 23 0 (n "Munster"))
(fortress 28 9 0 (n "Nuremberg"))
(fortress 9 18 0 (n "Roermond"))
(fortress 12 31 0 (n "Stade"))
(fortress 26 30 0 (n "Stettin"))
(fortress 20 34 0 (n "Stralsund"))
(fortress 20 5 0 (n "Strasbourg"))
(fortress 7 20 0 (n "Venlo"))
(fortress 9 22 0 (n "Wesel"))

(king 22 23 1 (n "Frederick") (in pa1))
(prince 22 23 1 (n "Ferdinand") (in "Magdeburg"))
(general 22 23 1 (n "Keith") (in "Magdeburg"))
(army 22 23 1 (hp 48) (sym pa1) (in "Magdeburg"))

(marshal 40 18 1 (n "Schwerin") (in pa2))
(army 40 18 1 (hp 25) (sym pa2) (in "Breslau"))

(marshal 36 18 1 (n "Brunswick") (in pa3))
(general 36 18 1 (n "Maurice"))
(army 36 18 1 (hp 15) (sym pa3))
(depot 36 18 1)

(army 42 17 1 (hp 1) (in "Brieg"))
(army 43 15 1 (hp 1) (in "Neisse"))
(army 40 16 1 (hp 1) (in "Glatz"))
(army 39 17 1 (hp 1) (in "Schweidinitz"))
(army 36 21 1 (hp 1) (in "Glogau"))

(fortress 40 18 1 (n "Breslau"))
(fortress 42 17 1 (n "Brieg"))
(fortress 40 16 1 (n "Glatz"))
(fortress 36 21 1 (n "Glogau"))
(fortress 22 23 1 (n "Magdeburg"))
(fortress 43 15 1 (n "Neisse"))
(fortress 39 17 1 (n "Schweidinitz"))

(marshal 46 9 3 (n "Browne") (in aa1))
(general 46 9 3 (n "Lascy"))
(army 46 9 3 (hp 50) (sym aa1) (in "Olmutz"))

(marshal 39 12 3 (n "Picolomini") (in aa2))
(general 39 12 3 (n "Nadasy"))
(army 39 12 3 (hp 25) (sym aa2))

(marshal 35 12 3 (n "Serbeloni") (in aa3))
(army 35 12 3 (hp 10) (sym aa3) (in "Prague"))

(fortress 44 7 3 (n "Brunn"))
(fortress 40 13 3 (n "Koniggratz"))
(fortress 46 9 3 (n "Olmutz"))
(fortress 35 12 3 (n "Prague"))
(fortress 46 3 3 (n "Vienna"))

(king 31 17 4 (n "Rutkowski") (in sa1))
(army 31 17 4 (hp 20) (sym sa1) (in "Dresden"))

(army 28 19 4 (hp 1) (in "Torgau"))

(fortress 31 17 4 (n "Dresden"))
(fortress 28 19 4 (n "Torgau"))

(scorekeeper
;  (title "VP")	; The AI does not understand this scorekeeper.
;  (initial 0)
;  (do (set (sum point-value)))
   (do last-side-wins)
  )

(set initial-date "24 Jun 1756")

(set initial-year-part 13)

;;; Game only lasts until the end of the year (armies would
;;; invariably go into winter quarters from December thru March).

;(set last-turn 13)
