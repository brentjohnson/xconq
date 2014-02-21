(game-module "1805"
  (title "Austrian campaign in 1805")
  (blurb "Napoleon's bid to break up the Third Coalition.")
  (version "1.0")
  (base-module "napoleon")
  (variants (world-seen true)(see-all true)(sequential false))
  (instructions (
  "Capture the capitals of the other sides."
  ))
  )

;; No unknown terrain in Europe at this time.

; (set terrain-seen true)

(include "eur-50km")

(add u* assign-number false)

(add places acp-per-turn 0)

;; Suppress random unit generation.

(add u* start-with 0)
(table independent-density 0)

(set sides-min 6)
(set sides-max 6)

(side 1 fr (name "France") (adjective "French") (emblem-name "flag-france"))
(side 2 uk (name "England") (adjective "English") (emblem-name "flag-uk")
  (active false))
(side 3 es (name "Spain") (adjective "Spanish") (emblem-name "flag-spain")
  (active false))
(side 4 at (name "Austria") (adjective "Austrian") (emblem-name "arms-austria"))
(side 5 pr (name "Prussia") (adjective "Prussian") (emblem-name "arms-prussia")
  (active false))
(side 6 ru (name "Russia") (adjective "Russian") (emblem-name "arms-russia"))

;;; The coalition members don't really trust each other, but close enough.

(side 4 (trusts (4 1) (6 1)))
(side 6 (trusts (4 1) (6 1)))

(set initial-date "Jul 1805")

(set initial-year-part 6)

; 0(set last-turn 5)

(add u* point-value 0)
(add capital point-value 1)

;;; Owner of Vienna only should be the winner.

(scorekeeper 
;  (title "VP")
;  (initial 0)
;  (do (set (sum point-value)))
   (do last-side-wins)
  )

(capital 70 31 0 (n "Rome"))
(capital 56 69 0 (n "Stockholm"))

(inf 56 55 fr)
(inf 49 55 fr)
(inf 49 55 fr)
(inf 61 50 fr)
(inf 57 42 fr)
(inf 57 49 fr)
(inf 53 51 fr (n "IV Corps"))
(inf 38 48 fr (hp 4))
(inf 67 34 fr (hp 4))
(inf 53 50 fr (n "III Corps"))
(inf 63 38 fr (hp 3))
(inf 55 49 fr (hp 8) (n "V Corps"))
(inf 56 49 fr (hp 8) (n "VI Corps"))
(inf 53 53 fr (hp 3) (n "I Corps"))
(inf 56 51 fr (hp 3) (n "II Corps"))
(guards 55 48 fr (hp 2) (n "Imperial"))
(cav 61 50 fr)
(cav 54 51 fr)
(cav 55 49 fr)
(cav 55 49 fr)
(ldr 53 53 fr (n "Bernadotte") (in "I Corps"))
(ldr 53 50 fr (n "Davout") (in "III Corps"))
(ldr 55 49 fr (n "Lannes") (in "V Corps"))
(ldr 56 51 fr (n "Marmont") (in "II Corps"))
(ldr 63 38 fr (n "Massena"))
(ldr 55 48 fr (n "Napoleon") (in "Imperial"))
(ldr 56 49 fr (n "Ney") (in "VI Corps"))
(ldr 53 51 fr (n "Soult") (in "IV Corps"))
(ldr 49 55 fr)
(ldr 61 49 fr)
(ldr 61 50 fr)
(city 49 55 fr (n "Amsterdam"))
(city 57 45 fr (n "Basel"))
(city 38 48 fr (n "Brest"))
(city 50 52 fr (n "Brussels"))
(city 46 52 fr (n "Calais"))
(city 41 50 fr (n "Cherbourg"))
(city 56 51 fr (n "Coblenz"))
(city 53 53 fr (n "Cologne"))
(city 67 34 fr (n "Florence"))
(city 57 42 fr (n "Geneva"))
(city 57 54 fr (n "Hanover"))
(city 45 49 fr (n "Le Havre"))
(city 48 51 fr (n "Lille"))
(city 54 50 fr (n "Metz"))
(city 63 38 fr (n "Milan"))
(city 43 45 fr (n "Nantes"))
(city 52 49 fr (n "Rheims"))
(city 56 48 fr (n "Strasbourg"))
(city 57 49 fr (n "Stuttgart"))
(city 50 36 fr (n "Toulouse"))
(city 61 50 fr (n "Wurzburg"))
(capital 49 48 fr (n "Paris"))

(city 43 53 uk (n "Dover"))
(city 41 58 uk (n "Norwich"))
(city 38 53 uk (n "Plymouth"))
(city 41 53 uk (n "Portsmouth"))
(capital 42 54 uk (n "London"))

(capital 42 31 es (n "Madrid"))

(inf 65 43 at (hp 3))
(inf 72 44 at)
(inf 68 48 at (hp 1))
(inf 66 47 at (hp 1))
(inf 66 38 at)
(inf 66 38 at (hp 4))
(inf 60 48 at)
(cav 59 47 at)
(cav 60 47 at)
(cav 65 43 at)
(cav 60 48 at)
(ldr 66 38 at (n "Charles"))
(ldr 60 48 at (n "Ferdinand"))
(ldr 66 38 at (n "Hiller"))
(ldr 60 48 at (n "Mack"))
(ldr 65 43 at)
(ldr 72 44 at)
(city 65 43 at (n "Innsbruck"))
(city 68 48 at (n "Prague"))
(city 66 47 at (n "Ratisbon"))
(city 59 48 at (n "Ulm"))
(city 67 38 at (n "Venice"))
(capital 78 42 at (n "Budapest"))
(capital 63 47 at (n "Munich"))
(capital 72 44 at (n "Vienna"))

(inf 62 54 pr (hp 1))
(capital 62 54 pr (n "Berlin"))

(inf 74 51 ru (hp 7))
(inf 73 57 ru (hp 3))
(inf 74 54 ru)
(guards 74 55 ru (hp 2))
(cav 75 51 ru)
(cav 74 57 ru)
(cav 75 54 ru)
(ldr 74 51 ru (n "Bagration"))
(ldr 73 57 ru (n "Benningsen"))
(ldr 74 54 ru (n "Buxhowden"))
(ldr 74 54 ru (n "Constantine"))
(ldr 74 51 ru (n "Kutuzov"))
(city 74 54 ru (n "Brest-Litovsk"))
(city 73 57 ru (n "Grodno"))
(city 70 59 ru (n "Kovno"))
(city 74 51 ru (n "Lublin"))
(city 78 47 ru (n "Lvov"))
(city 68 60 ru (n "Memel"))
(capital 86 62 ru (n "Moscow"))


(area (aux-terrain road
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "66acu92a"
  "68a@91a"
  "63ai5a>su88a"
  "63aj8aD87a"
  "63aj8aj87a"
  "61aeaf2ai5ab87a"
  "62a>sN{r93a"
  "57acu6aj94a"
  "59a@5aj94a"
  "60a@4aj94a"
  "61a<3ab94a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  ))

(area (aux-terrain river
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "68ami90a"
  "68ab?}i88a"
  "70ab?y87a"
  "54ami10agq4ady86a"
  "54ab?}i8acT5ab86a"
  "56ab?}i7ahq91a"
  "48a5mi4ab?y6acXi90a"
  "48ab4=?}i4ady6ab?y89a"
  "54ab?y4ad}i6ad}i87a"
  "56ahq4ab?}i5abCq86a"
  "56agL6ab?}i4agL86a"
  "56agLae4miab?y3agL86a"
  "56agLagM3=?}iab4a<86a"
  "56agL2a<4ab?y92a"
  "56acXmi7ad}2mi88a"
  "57ab2=8ab2=?y87a"
  "72ad}2mi83a"
  "73ab2=?y82a"
  "77ady81a"
  "78ahq80a"
  "78acT80a"
  "79ady79a"
  "62ae4mi12ahq9aemi66a"
  "63a5=12acT9agM=66a"
  "81ad}8moL67a"
  "82ab9=<67a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  "160a"
  ))
