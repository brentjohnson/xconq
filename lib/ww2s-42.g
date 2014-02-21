(game-module "ww2s-42"
  (title "WWII World 1942")
  (version "1.0")
  (blurb "The world in January 1942. Based on the standard game.")
  (base-module "standard")
  (variants (world-seen true) (see-all true) (sequential false))
  )

(game-module (instructions (
  "The lines are drawn: Axis on one side and Allies on the other, with only unconditional surrender to end the game."
  "The units and terrain types are exactly those from the standard game, so don't expect great historical accuracy!"
  "Instead focus on using your resources to win by whatever means that work."
  )))

;; Preset the sides, which restricts this to post-Dec 1941, when the
;; sides were clearly lined up.

(side 1 (name "Allies") (adjective "Allied") (emblem-name "allies"))

(side 2 (name "Axis") (adjective "Axis") (emblem-name "axis"))

(set sides-min 2)
(set sides-max 2)

;; The terrain of the world will always be known.

(set terrain-seen true)

;; Bases predate the war, no secrets there.

(add base already-seen 100)

;;; Initial setup.

(add u* start-with 0)
(add u* independent-near-start 0)

(set synthesis-methods '(name-units-randomly))

;;; Scoring.

(scorekeeper (do last-side-wins))

;;; Terrain.

(include "earth-1deg")

;; We want the rivers.

(include "t-e1-river")

;;; Units.

(define capital city)

;; This maps countries to Allies, Axis, or neutral.

(define uk 1)
(define fr 2)
(define us 1)
(define de 2)
(define it 2)
(define jp 2)
(define su 1)
(define zh 1)
(define es 0)
(define tr 0)

(include "u-e1-1938")

;; Changes for beginning of 1942.
;; (should be in a generic 1942 status file)

;; Iceland was occupied (sort of) by Britain.

(unit "Reykjavik" (s 1))

;; Douala is Free French.

(unit "Douala" (s 1))

;; Indonesia is Dutch.

(unit "Banjarmasin" (s 1))
(unit "Jakarta" (s 1))
(unit "Medan" (s 1))
(unit "Palembang" (s 1))
(unit "Surabaya" (s 1))
(unit "Ujung Pendang" (s 1))

;; German/Italian conquests and allies.

(unit "Athens" (s 2))
(unit "Belgrade" (s 2))
(unit "Brussels" (s 2))
(unit "Bucharest" (s 2))
(unit "Budapest" (s 2))
(unit "Copenhagen" (s 2))
(unit "Danzig" (s 2))
(unit "Kiev" (s 2))
(unit "Minsk" (s 2))
(unit "Odessa" (s 2))
(unit "Prague" (s 2))
(unit "Riga" (s 2))
(unit "Rotterdam" (s 2))
(unit "Sofia" (s 2))
(unit "Thessaloniki" (s 2))
(unit "Vienna" (s 2))
(unit "Vilnius" (s 2))
(unit "Warsaw" (s 2))
(unit "Zagreb" (s 2))

;; Japanese conquests in China and Indochina.

(unit "Amoy" (s 2))
(unit "Canton" (s 2))
(unit "Hankow" (s 2))
(unit "Hanoi" (s 2))
(unit "Kaifeng" (s 2))
(unit "Nanking" (s 2))
(unit "Nanchang" (s 2))
(unit "Ningpo" (s 2))
(unit "Paoting" (s 2))
(unit "Peiping" (s 2))
(unit "Phnom Penh" (s 2))
(unit "Saigon" (s 2))
(unit "Shanghai" (s 2))
(unit "Soochow" (s 2))
(unit "Swatow" (s 2))
(unit "Taiyuan" (s 2))
(unit "Tientsin" (s 2))
(unit "Tsinan" (s 2))
(unit "Tsingtao" (s 2))

(d (in "Honolulu") (s 1))
(d (in "Honolulu") (s 1))
(s (in "Honolulu") (s 1))
(s (in "Honolulu") (s 1))
(t (in "Honolulu") (s 1))
(cv (n "Lexington") (in "Honolulu") (s 1))
(cv (n "Saratoga") (in "Honolulu") (s 1))
(cv (n "Enterprise") (in "Honolulu") (s 1))
(bb (in "Honolulu") (s 1))
(fighter (in "Lexington") (s 1))
(fighter (in "Lexington") (s 1))
(fighter (in "Lexington") (s 1))
(fighter (in "Lexington") (s 1))
(bomber (in "Lexington") (s 1))
(fighter (in "Saratoga") (s 1))
(fighter (in "Saratoga") (s 1))
(fighter (in "Saratoga") (s 1))
(fighter (in "Saratoga") (s 1))
(bomber (in "Saratoga") (s 1))
(fighter (in "Enterprise") (s 1))
(fighter (in "Enterprise") (s 1))
(fighter (in "Enterprise") (s 1))
(fighter (in "Enterprise") (s 1))
(bomber (in "Enterprise") (s 1))

(unit-defaults (@ 225 40))

(infantry 19 63 1)
(infantry 21 61 1)
(infantry 26 58 1)
(infantry 26 57 1)
(infantry 23 55 1)
(infantry 24 54 1)
(infantry 27 49 1)
(infantry 18 54 1)
(infantry 20 50 1)
(infantry 20 52 1)
(infantry 22 53 1)
(infantry 18 65 1)
(infantry 16 67 1)
(infantry 23 59 1)
(infantry 22 58 1)
(infantry 30 55 1)
(infantry 30 52 1)
(infantry 38 41 1)
(infantry 86 2 1)

(base 93 54 1 (n "Midway"))
(fighter (in "Midway") (s 1))
(bomber (in "Midway") (s 1))

(infantry 26 59 2)
(infantry 27 57 2)
(infantry 26 56 2)
(infantry 25 54 2)
(infantry 29 56 2)
(infantry 30 51 2)
(infantry 26 50 2)
(infantry 20 64 2)
(infantry 21 63 2)
(infantry 22 61 2)
(infantry 24 49 2)
(infantry 37 44 2)
(infantry 37 43 2)
(infantry 38 43 2)
(infantry 39 58 2)
(infantry 39 61 2)
(infantry 41 61 2)
(infantry 43 61 2)
(infantry 45 62 2)
(infantry 47 62 2)
(infantry 46 63 2)
(infantry 46 66 2)
(infantry 45 68 2)
(infantry 33 50 2)
(infantry 32 64 2)
(infantry 26 74 2)
(infantry 30 71 2)
(infantry 33 68 2)
(infantry 20 69 2)
(infantry 21 71 2)
