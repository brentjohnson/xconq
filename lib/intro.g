(game-module "intro"
  (title "Introductory Game")
  (version "1.0")
  (blurb "An introduction to Xconq. Your goal is to explore and take over the world.")
  ;; no variants, this is just a basic game.
)

;; Base this on the standard game, so that new players don't have to
;; learn a new set of rules just to play "the real game".

(include "stdunit")

;; Hexagon map is simpler to play on than cylinder.
 
(world 120)

;; Fixed area, easier to play on.

(area 40 20)

;; Wire to exactly two players, no options.

(side 1 (noun "Humans") (adjective "human") (emblem-name "flag-un"))

(side 2 (noun "Robots") (adjective "robot") (emblem-name "flag-robot"))

(set sides-min 2)
(set sides-max 2)

;; This is just a simple "take over the world", easier to explain.

(scorekeeper (do last-side-wins))

;; Only cities count.

(add u* point-value 0)
(add u* point-value 1)

(set synthesis-methods nil)

(area 40 20)

(area 40 20 (terrain
  "40a"
  "3a3e3a4e8g2e3a5e9a"
  "4a2e4a9e3ge3ae2a2eg8a"
  "5ae5a10e3ge5a3e7a"
  "ae3a2e5a9e5ge3a4e6a"
  "aege2a2e4a3eg6e4g4a2e8a"
  "a3e2ae2g14ege15a"
  "a3e3a4g3e2a7e13ae3a"
  "a3e3ae3g2e4a6e13aeg2a"
  "ag3e3ae3g2e4a6e12ae2ga"
  "2a4e3a3g3e3a10e8ae2ga"
  "3a3e3ae3g4e3aegea4e5ae3a2ea"
  "4a4e2a2eg4e4a2e2a3e5a2e3aea"
  "5a3e6a3e9aege3a3e5a"
  "6a4e4a4e8ae2ge2a2ege4a"
  "10ae2aeg4e8a3e3a3e4a"
  "11aeae2ge3a2e2a3e7a2e4a"
  "14aege4a7e6a3e3a"
  "22a3eg2e6a4e2a"
  "40a"
  ))

(town 18 9 0 (n "Bonsig"))
(town 5 15 0 (n "Druent"))
(town 3 11 0 (n "Ernou"))
(town 21 7 0 (n "Hall Sep"))
(town 14 15 0 (n "Hanawhald"))
(town 15 8 0 (n "Lynug"))
(town 13 13 0 (n "Mebnels"))
(town 16 15 0 (n "Nugbag"))
(town 25 9 0 (n "Opplen"))
(town 18 4 0 (n "Uk Fensotguf"))
(town 13 8 0 (n "Vetpi"))

(town 12 10 1 (n "Jinod"))

(town 20 12 2 (n "Ko Renme"))

(game-module (instructions (
  "This game is an introduction to Xconq."
  "You start with one town and try to expand throughout the world."
  "Eventually you will encounter the evil Robots, and must do battle with them."
  "To get things going, use your first infantry to explore around."
  "You should try to capture independent towns and set them to build new units, either infantry or armor."
  "Armor moves faster, but also takes longer to build."
  "When you find the bad guys, try to capture their towns."
  "There may be a lot of these, so search carefully."
  "They may also have armies secretly in reserve - watch out for surprise invasions!"
)))

