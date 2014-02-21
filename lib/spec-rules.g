(game-module "spec-rules"
  (title "Specula")
  (version "1.0")
  (blurb "base module")
  )



;;; Maybe princes?  More ministers?
;; occupant heart
;; Golems. knights, adventurers
;; acp-to-defend reduction against ghouls?
;; Dispel
;; Extinction/Banish/Seal spell
;; Ruins produce undead?

;;; Vision.

(set terrain-seen true)
(set ai-may-resign false)
(set indepside-has-ai true)
(set self-required true)


;;; Types.
;;; Self-units

(unit-type king (name "King") (image-name "spec-king")
  (acp-per-turn 6) (hp-max 80) (vision-range 4) (type-in-game-max 1) (possible-sides (or "loyal" (or "rebel")))
  (can-be-self true) (self-resurrects false)
  (help "King of the Aristocracy, as he goes, so goes the Aristocracy"))

(unit-type usurper (name "Usurper") (image-name "spec-usurper")
  (acp-per-turn 6) (hp-max 80) (vision-range 4) (type-in-game-max 1) (possible-sides (or "loyal" (or "rebel")))
  (can-be-self true)
  (help "Lord of the Aristocracy Rebels"))

(unit-type minister (name "Minister") (image-name "spec-minister")
  (acp-per-turn 6) (hp-max 80) (vision-range 4) (type-in-game-max 1) (possible-sides "confed")
  (can-be-self true)
  (help "High councilor of the Confederacy"))

(unit-type shogun (name "Shogun") (image-name "spec-shogun")
  (acp-per-turn 6) (hp-max 80) (vision-range 4) (type-in-game-max 1) (possible-sides "shogunate")
  (can-be-self true)
  (help "The Shogunate incarnate"))

(unit-type loremaster (name "Loremaster") (image-name "spec-loremaster")
  (acp-per-turn 6) (hp-max 80) (vision-range 4) (acp-to-fire 1) (range 2)  (type-in-game-max 1)
  (possible-sides "oni")  (can-be-self true)
  (help "Lord of the Oni"))

(unit-type emperor (name "Emperor") (image-name "spec-emperor")
  (acp-per-turn 6) (hp-max 80) (vision-range 4) (type-in-game-max 1)
  (possible-sides "empire")  (can-be-self true)
  (help "Emperor of the Iriken"))

(unit-type shah (name "Shah") (image-name "spec-shah")
  (acp-per-turn 6) (hp-max 80) (vision-range 4) (type-in-game-max 1)
  (possible-sides "bedouin")  (can-be-self true)
  (help "Shah of the Bedouin"))

(unit-type pharoah (name "Pharoah") (image-name "spec-pharoah")
  (acp-per-turn 6) (hp-max 80) (vision-range 4) (type-in-game-max 1)
  (possible-sides "dwarf")  (can-be-self true)
  (help "Pharoah of the City-States"))

(unit-type greyone (name "Grey One") (image-name "spec-greyone")
  (acp-per-turn 3) (hp-max 80) (vision-range 4) (type-in-game-max 1)
  (possible-sides "undead")  (can-be-self true)
  (help "Mysterious lord of the Undead"))

(unit-type talon (name "Talon") (image-name "spec-talon")
  (acp-per-turn 6) (hp-max 80) (vision-range 4) (type-in-game-max 1)
  (possible-sides "lizard")  (can-be-self true)
  (help "Ruler of the Lizardmen"))

(unit-type spawn (name "Spawn") (image-name "spec-spawn")
  (acp-per-turn 6) (hp-max 80) (vision-range 4) (acp-to-fire 1) (range 2)  (type-in-game-max 1)
  (possible-sides "kraken")  (can-be-self true)
  (help "Ruler of the Kraken"))

(unit-type wocke (name "Jabberwocke") (image-name "spec-wocke")
  (acp-per-turn 6) (hp-max 80) (vision-range 4) (type-in-game-max 1)
  (possible-sides "wild")
  (help "King of the forest"))

(define self-types (king usurper minister shogun loremaster emperor shah pharoah greyone talon spawn wocke))



;; Basic Units

(unit-type sword (name "Swordsmen") (image-name "spec-sword")
  (acp-per-turn 6) (hp-max 20) (vision-range 1) (cp 6)
  (help "Basic melee unit"))

(unit-type halberd (name "Halberdier") (image-name "spec-halberd")
  (acp-per-turn 6) (hp-max 30) (vision-range 1) (cp 6)
  (help "Improved melee unit"))

(unit-type bow (name "Bowmen") (image-name "spec-bow")
  (acp-per-turn 6) (hp-max 15) (acp-to-fire 1) (cp 6) (range 1) (vision-range 1)
  (help "Basic ranged unit"))

(unit-type cavalry (name "Cavalry") (image-name "spec-cavalry")
  (acp-per-turn 8) (hp-max 40) (vision-range 3) (cp 6)
  (help "Basic mounted unit"))

(unit-type engineer (name "Engineers") (image-name "spec-engineer")
  (acp-per-turn 4) (hp-max 15)  (vision-range 1) (cp 9)
  (help "Capable of building and repairing fortresses"))

(unit-type catapult (name "Catapult") (image-name "spec-catapult")
  (acp-per-turn 4) (hp-max 25) (acp-to-fire 1) (cp 9)  (range 1) (vision-range 1)
  (help "Basic siege engine"))

(unit-type mage (name "Mage") (image-name "spec-mage")
  (acp-per-turn 6) (hp-max 10) (vision-range 2) (acp-to-fire 1) (range 1) (cp 9)
  (help "Basic magic unit"))

(define basic-types (sword halberd bow cavalry engineer catapult mage))



;; Nation-specific units

(unit-type scout (name "Scout") (image-name "spec-scout")
  (acp-per-turn 6) (hp-max 15)  (vision-range 3) (cp 12)
  (help "Light infantry with great vision, only produced in forts"))

(unit-type gladiator (name "Imperial Gladiator") (image-name "spec-gladiator")
  (acp-per-turn 6) (hp-max 35)  (vision-range 1) (cp 6)
  (possible-sides "empire")
  (help "Imperial heavy infantry"))

(unit-type longbow (name "Longbowman") (image-name "spec-longbow")
  (acp-per-turn 6) (hp-max 15) (acp-to-fire 1) (range 1) (vision-range 1) (cp 6)
  (possible-sides "shogunate")
  (help "Shogunate improved range unit"))

(unit-type crusader (name "Crusaders") (image-name "spec-crusader")
  (acp-per-turn 6) (hp-max 35)  (vision-range 1) (cp 6)
  (possible-sides "loyal")
  (help "Loyalist heavy infantry"))

(unit-type assassin (name "Assassins") (image-name "spec-assassin")
  (acp-per-turn 6) (hp-max 10)  (vision-range 3) (cp 6)
  (possible-sides "rebel")
  (help "Rebel spy and killer"))

(unit-type cannon (name "Steam Cannon") (image-name "spec-cannon")
  (acp-per-turn 4) (hp-max 50) (acp-to-fire 1)  (range 1) (vision-range 1) (cp 9)
  (possible-sides "dwarf")
  (help "Dwarven improved siege engine"))

(unit-type dervish (name "Dervish") (image-name "spec-dervish")
  (acp-per-turn 6) (hp-max 15) (vision-range 2) (acp-to-fire 1) (range 1)  (cp 6)
  (possible-sides "bedouin")
  (help "Weak bedouin mage able to be produced in smaller cities"))

(unit-type weaver (name "Loreweaver") (image-name "spec-weaver")
  (acp-per-turn 6) (hp-max 15) (vision-range 2) (acp-to-fire 1) (range 1)  (cp 6)
  (possible-sides "oni")
  (help "Oni mage that adds to RP")) 

(unit-type croc (name "Crocadilion") (image-name "spec-croc")
  (acp-per-turn 4) (hp-max 15) (vision-range 3) (cp 6)
  (possible-sides "lizard")
  (help "Lizardmen tamer"))

(define specific-types (scout gladiator longbow crusader assassin cannon dervish weaver croc))

;; INDEPENDANTS

(unit-type spear (name "Orcspawn Spearmen") (image-name "spec-spear")
  (acp-per-turn 6) (hp-max 10) (vision-range 1) (cp 6)
  (help "Orcspawn spearmen"))

(unit-type shaman (name "Orcspawn Shaman") (image-name "spec-shaman")
  (acp-per-turn 6) (hp-max 10) (vision-range 1) (acp-to-fire 1) (range 1)  (cp 9)
  (help "Heals"))

(unit-type thug (name "Ogre Thugs") (image-name "spec-thug")
  (acp-per-turn 6) (hp-max 40) (vision-range 1) (cp 9)
  (help "Big"))

(unit-type champ (name "Ogre Champion") (image-name "spec-champ")
  (acp-per-turn 6) (hp-max 45) (vision-range 1) (cp 12)
  (help "Bigger"))

(unit-type cutter (name "Cutter") (image-name "spec-cutter")
  (acp-per-turn 8) (hp-max 60) (vision-range 5) (cp 3) (acp-to-fire 1) (range 1) (naval true) 
  (help "Fast ship, carries two units"))

(unit-type whaler (name "Whaler") (image-name "spec-whaler")
  (acp-per-turn 8) (hp-max 80) (vision-range 4) (cp 3) (acp-to-fire 1) (range 1) (naval true) 
  (help "Powerful attack, against big creatures"))

(unit-type thuvi (name "Thuvi Irikani") (image-name "spec-thuvi")
  (acp-per-turn 6) (hp-max 35)  (vision-range 1) (cp 6)
  (help "Descendants of Imperial rebels"))

(unit-type tribesman (name "Orc Warrior") (image-name "spec-tribesman")
  (acp-per-turn 6) (hp-max 25)  (vision-range 1) (cp 6)
  (help "Descendants of Imperial rebels"))

(define independant-types (spear shaman thug champ cutter whaler thuvi tribesman))

;; RUINS

(unit-type cityruin (name "Fallen City") (image-name "spec-citadelruin")
  (acp-independent true) (hp-max 1) (vision-range 1)
  (help "Still produces some material."))

(unit-type villageruin (name "Razed Village") (image-name "spec-keepruin")
  (acp-independent true) (hp-max 1) (vision-range 1)
  (help "Still produces some material."))

(unit-type keepruin (name "Fallen Keep") (image-name "spec-keepruin")
  (acp-independent true) (hp-max 1) (vision-range 1)
  (help "The remains of a castle, still provides some protection."))

(unit-type citadelruin (name "Fallen Citadel") (image-name "spec-citadelruin")
  (acp-independent true) (hp-max 1) (vision-range 1)
  (help "The remains of a citadel, still provides some protection."))

(unit-type ruins (name "Ruins") (image-name "spec-keepruin")
  (acp-independent true) (hp-max 1) (vision-range 1)
  (help "Scary."))

(define ruin-types (cityruin villageruin keepruin citadelruin ruins))


;; FORTRESSES
(unit-type tower (name "Watchtower") (image-name "spec-tower")
  (acp-per-turn 0) (hp-max 25)  (vision-range 4) (cp 3)
  (ai-war-garrison 1) (ai-peace-garrison 1)
  (help "Protects 1 unit, provides excellent vision."))

(unit-type fort (name "Wooden Fort") (image-name "spec-fort")
  (acp-per-turn 3) (hp-max 50)  (vision-range 2) (cp 3)
  (ai-war-garrison 2) (ai-peace-garrison 2)
  (help "Cheap to produce."))

(unit-type keep (name "Keep") (image-name "spec-keep")
  (acp-per-turn 3) (hp-max 100) (wrecked-type keepruin) (vision-range 3) (cp 3)
  (ai-war-garrison 3)  (ai-peace-garrison 2)
  (help "Protects 3 units, provides good vision."))

(unit-type citadel (name "Citadel") (image-name "spec-citadel")
  (acp-per-turn 3) (hp-max 200) (wrecked-type citadelruin) (vision-range 4) (cp 3)
  (ai-war-garrison 4)  (ai-peace-garrison 4)
  (help "Protects 8 units, provides excellent vision."))

(define fortress-types (tower fort keep citadel))




;; Death Creatures


(unit-type zombie (name "Zombies") (image-name "spec-zombie")
  (acp-per-turn 6) (hp-max 25) (vision-range 1) (cp 6)
  (possible-sides "undead")
  (help "Smelly"))

(unit-type spectre (name "Spectres") (image-name "spec-spectre")
  (acp-per-turn 6) (hp-max 45) (vision-range 1) (acp-to-fire 1) (range 1) (cp 9)
  (possible-sides "undead")
  (help "Not too powerful in direct combat, but can create ghosts"))

(unit-type ghost (name "Ghosts") (image-name "spec-ghost")
  (acp-per-turn 6) (hp-max 20) (vision-range 1) (cp 3)
  (possible-sides "undead")
  (help "Short-lived, difficult to destroy"))

(define death-types (zombie spectre ghost))


;; WILDERNESS

(unit-type tyrannosaur (name "Tyrannosaur") (image-name "spec-tyrannosaur")
  (acp-per-turn 6) (hp-max 90) (vision-range 1) (cp 9) 
  (help "Extremely dangerous creature, look out!"))

(unit-type apatosaur (name "Apatosaur") (image-name "spec-apatosaur")
  (acp-per-turn 6) (hp-max 150) (vision-range 1) (cp 9) 
  (help "Very large, dangerous against fortresses and cities.  Many, many hit points."))

(unit-type pterodactyl (name "Pterodactyl") (image-name "spec-pterodactyl")
  (acp-per-turn 6) (hp-max 40) (vision-range 3) (cp 6) 
  (help "Flying dinosaur.  Timid, runs away a lot, dangerous against small units."))

(unit-type plesiosaur (name "Plesiosaur") (image-name "spec-plesiosaur")
  (acp-per-turn 6) (hp-max 75) (vision-range 1) (cp 6) (naval true) 
  (help "Swimming dinosaur.  Dangerous, hard to hit."))

(unit-type deadwild (name "Pelts") (image-name "spec-pelts")
  (acp-independent true) (hp-max 1) (vision-range 0)
  (help "Valuable furs, meat and feathers."))

(unit-type wolves (name "Wolves") (image-name "spec-wolves")
  (acp-per-turn 6) (hp-max 20) (wrecked-type deadwild) (vision-range 4) (cp 6)
  (help "Great vision, high movement in forests."))

(unit-type bears (name "Bears") (image-name "spec-bears")
  (acp-per-turn 6) (hp-max 30) (wrecked-type deadwild) (vision-range 3) (cp 9)
  (help "Tough against small units."))

(unit-type ferak (name "Ferak") (image-name "spec-ferak")
  (acp-per-turn 6) (hp-max 10) (wrecked-type deadwild) (vision-range 4) (cp 6) (air true) 
  (help "Eyebird."))

(unit-type royalferak (name "Royal Ferak") (image-name "spec-royalferak")
  (acp-per-turn 6) (hp-max 25) (wrecked-type deadwild) (vision-range 4) (cp 9) (air true) 
  (help "Big, armored, eyebird."))

(define wild-types (tyrannosaur apatosaur pterodactyl plesiosaur wolves bears ferak royalferak))


;; GLASS-DARK

(unit-type axecorpse (name "Dead Axehead") (image-name "spec-axecorpse")
  (acp-independent true) (hp-max 1) (vision-range 0)
  (help "Dead, huge, dangerous fish."))

(unit-type axehead (name "Axehead") (image-name "spec-axehead")
  (acp-per-turn 6) (hp-max 100) (wrecked-type axecorpse) (vision-range 1) (cp 9) (naval true)
  
  (help "Huge, dangerous fish."))

(unit-type boscorpse (name "Dead Bospallian") (image-name "spec-boscorpse")
  (acp-independent true) (hp-max 1) (vision-range 0)
  (help "Dead, huge, dangerous fish."))

(unit-type bospallian (name "Bospallian") (image-name "spec-bospallian")
  (acp-per-turn 6) (hp-max 200) (wrecked-type boscorpse) (vision-range 1) (cp 3) (naval true)
  
  (help "Huge, not so dangerous fish."))

(unit-type mold (name "Devil's Mold") (image-name "spec-mold")
  (acp-per-turn 3) (hp-max 9) (vision-range 1) (cp 6) (naval true)
  
  (help "Killer sludge that's slow and almost impossible to kill."))

(unit-type dajawyrm (name "Dajaspec Sea Monster") (image-name "spec-dajawyrm")
  (acp-per-turn 6) (hp-max 120) (vision-range 1) (cp 9) (naval true)
  
  (help "???"))

(unit-type dajaturtle (name "Veld") (image-name "spec-dajaturtle")
  (acp-per-turn 6) (hp-max 80) (vision-range 2) (cp 9) (naval true)
  
  (help "One mean turtle."))

(define glass-types (axehead bospallian mold dajawyrm dajaturtle))

;; COMFORT-WHITE

(unit-type gibambi (name "Gibambi") (image-name "spec-gibambi")
  (acp-per-turn 6) (hp-max 15) (vision-range 1) (cp 6)
  
  (help "Tough little jerks."))

(unit-type squid (name "Giant Squid") (image-name "spec-squid")
  (acp-per-turn 9) (hp-max 75) (vision-range 1) (cp 9)
  
  (help "A lot of little attacks"))

(unit-type thucorpse (name "Festering Remains of Thurastes") (image-name "spec-thucorpse")
  (acp-independent true) (hp-max 1) (vision-range 0)
  (help "That's impossible!!!"))

(unit-type thurastes (name "Thurastes") (image-name "spec-thurastes")
  (acp-per-turn 3) (hp-max 500) (wrecked-type thucorpse) (vision-range 12) 
  (help "Really, really hard to kill.  And mean."))

(unit-type tentacle (name "Tentacle of Thurastes") (image-name "spec-tentacle")
  (acp-per-turn 6) (hp-max 40) (vision-range 1) (cp 15)
  
  (help "One big attack per turn."))

(unit-type churambi (name "Churambi") (image-name "spec-churambi")
  (acp-per-turn 6) (hp-max 20) (vision-range 1) (cp 6)  
  (help "Tougher, littler jerks."))

(unit-type ulitar (name "Ulitar") (image-name "spec-ulitar")
  (acp-per-turn 6) (hp-max 70) (vision-range 1) (cp 6)  
  (help "Weird umber hulk looking thing, sometimes with sludge."))

(unit-type sludge (name "Strange Mold") (image-name "spec-sludge")
  (acp-per-turn 3) (hp-max 1) (vision-range 2) (cp 9)  
  (help "Occasionally carried by an Ulitar, can capture creatures."))

(define comfort-types (gibambi squid thurastes tentacle churambi ulitar sludge))



;; Kraken



(unit-type slaver (name "Slavemaster") (image-name "spec-slaver")
  (acp-per-turn 6) (hp-max 60) (vision-range 2) (cp 6) 
  (help "Kraken Slavemaster."))

(unit-type sorcerer (name "Sorcerer") (image-name "spec-sorcerer")
  (acp-per-turn 6) (hp-max 40) (vision-range 2) (acp-to-fire 1) (range 1) (cp 9) 
  (help "Kraken Sorcerer"))

(define kraken-types (slaver sorcerer))

;; DRAGONKIND

(unit-type gwyrm (name "Great Wyrm") (image-name "spec-gwyrm")
  (acp-per-turn 8) (hp-max 250) (vision-range 2)  (air true)
  (help "The most powerful dragon."))

(unit-type wyrm (name "Wyrm") (image-name "spec-wyrm")
  (acp-per-turn 8) (hp-max 100) (vision-range 2) (air true)
  (help "Several hundred years old."))

(unit-type hatchling (name "Young Dragon") (image-name "spec-hatchling")
  (acp-per-turn 6) (hp-max 75) (vision-range 2) (cp 6)  (air true)
  (help "Still dangerous."))

(define dragon-types (gwyrm wyrm hatchling))









;; CITIES

(unit-type city1 (name "Small City") (image-name "ancient-small-city")
  (acp-per-turn 3) (hp-max 100) (wrecked-type cityruin) (vision-range 2)
  (ai-war-garrison 2) (ai-peace-garrison 1)
  (help "Small city."))

(unit-type city2 (name "City") (image-name "ancient-city")
  (acp-per-turn 3) (hp-max 100) (wrecked-type city1) (vision-range 3) 
  (ai-war-garrison 3) (ai-peace-garrison 2)
  (help "City."))

(unit-type city3 (name "Large City") (image-name "ancient-blue-city")
  (acp-per-turn 3) (hp-max 100) (wrecked-type city2) (vision-range 3)
  (ai-war-garrison 4) (ai-peace-garrison 3)
  (help "Large city."))

(unit-type city4 (name "Huge City") (image-name "ancient-yellow-castle")
  (acp-per-turn 3) (hp-max 100) (wrecked-type city3) (vision-range 3)
  (ai-war-garrison 5) (ai-peace-garrison 4)
  (help "Huge city."))

(unit-type wcity1 (name "Walled Small City") (image-name "ancient-small-city")
  (acp-per-turn 3) (hp-max 150) (wrecked-type city1) (vision-range 2)
  (ai-war-garrison 2) (ai-peace-garrison 1)
  (help "Small city, with a wall."))

(unit-type wcity2 (name "Walled City") (image-name "ancient-city")
  (acp-per-turn 3) (hp-max 150) (wrecked-type city2) (vision-range 3)
  (ai-war-garrison 3) (ai-peace-garrison 2)
  (help "City, with a wall."))

(unit-type wcity3 (name "Walled Large City") (image-name "ancient-blue-city")
  (acp-per-turn 3) (hp-max 150) (wrecked-type city3) (vision-range 3)
  (ai-war-garrison 4) (ai-peace-garrison 3)
  (help "Large city, with a wall."))

(unit-type wcity4 (name "Walled Huge City") (image-name "ancient-black-castle")
  (acp-per-turn 3) (hp-max 150) (wrecked-type city4) (vision-range 3)
  (ai-war-garrison 5) (ai-peace-garrison 4)
  (help "Huge city, with a wall."))

(unit-type town (name "Township") (image-name "ancient-village2")
  (acp-per-turn 3) (hp-max 100) (wrecked-type villageruin) (vision-range 2)
  (ai-war-garrison 1)
  (help "A town."))

(unit-type svillage (name "Seaside Village") (image-name "ancient-village1")
  (acp-per-turn 3) (hp-max 100) (wrecked-type villageruin) (vision-range 3)
  (ai-war-garrison 1)
  (help "A town."))

(unit-type ohamlet (name "Orcspawn Hamlet") (image-name "spec-hamlet")
  (acp-per-turn 3) (hp-max 100) (wrecked-type villageruin) (vision-range 2)
  (ai-war-garrison 1)
  (help "A little hamlet that produces wimps."))

(unit-type ovillage (name "Ogre Village") (image-name "spec-ogvillage")
  (acp-per-turn 3) (hp-max 100) (wrecked-type villageruin) (vision-range 2)
  (ai-war-garrison 1)
  (help "A big village that produces thugs."))

(unit-type deadcity (name "Dead City") (image-name "ancient-ruins")
  (acp-per-turn 3) (hp-max 250) (vision-range 5) (wrecked-type cityruin) 
  (ai-peace-garrison 8) (ai-war-garrison 8)
  (help "It's haunted.  Cannot be captured, can only be destroyed by some Life spell and maybe a nature spell."))

(define place-types (wcity4 city4 wcity3 city3 wcity2 city2 wcity1 city1 town svillage ohamlet ovillage deadcity))


;; POINTS OF INTEREST

(unit-type lair (name "Dragon's Lair") (image-name "spec-lair")
  (acp-per-turn 3) (hp-max 100) (vision-range 120)
  (possible-sides "dragon")  (can-be-self true)
  (help "Produces young dragons very slowly."))

(unit-type pit (name "Kraken Pit") (image-name "spec-heart")
  (acp-per-turn 3) (hp-max 100) (vision-range 10)
  (help "Produces Kraken."))

(unit-type dino1 (name "Dinosaur Breeding Ground") (image-name "spec-heart")
  (acp-per-turn 3) (hp-max 1) (vision-range 3)
  (possible-sides "wild")  (can-be-self true)
  (help "Produces dinosaurs.  Cannot be captured."))

(unit-type dino2 (name "Saurian Breeding Ground") (image-name "spec-heart")
  (acp-per-turn 3) (hp-max 1) (vision-range 3)
  (help "Produces dinosaurs.  Cannot be captured."))


(unit-type fheart (name "Heart of the Forest") (image-name "spec-heart")
  (acp-per-turn 3) (hp-max 1) (vision-range 3)
  (help "Produces wolves and bears.  Cannot be captured."))

(unit-type jheart (name "Heart of the Jungle") (image-name "spec-heart")
  (acp-per-turn 3) (hp-max 1) (vision-range 3)
  (help "Produces Ferak and Royal Ferak.  Cannot be captured."))

(unit-type sheart (name "Heart of the Sulspec") (image-name "spec-heart")
  (acp-per-turn 3) (hp-max 1) (vision-range 3)
  (possible-sides "white")  (can-be-self true)
  (help "Produces Giant Squid and Gibambi."))

(unit-type gheart (name "Heart of the Glaspec") (image-name "spec-heart")
  (acp-per-turn 3) (hp-max 1) (vision-range 3)
  (possible-sides "glass")  (can-be-self true)
  (help "Produces Bospallian and Axehead."))

(unit-type hheart (name "Heart of the Hoespec") (image-name "spec-heart")
  (acp-per-turn 3) (hp-max 1) (vision-range 3)
  (help "Produces Churambi and Ulitars."))

(unit-type dheart (name "Heart of the Dajaspec") (image-name "spec-heart")
  (acp-per-turn 3) (hp-max 1) (vision-range 3)
  (help "Produces Daja Turtles and Daja Wyrms."))

(define wilderness-types (lair pit dino1 dino2 fheart jheart sheart gheart hheart dheart))



(unit-type needle (name "The Needle") (image-name "spec-needle")
  (acp-independent true) (hp-max 1) (vision-range 3) (type-in-game-max 1)
  (help "Opal is known as the pincushion of the gods."))

;; NODES

(unit-type enode (name "Earth Node") (image-name "spec-node")
  (acp-per-turn 3) (hp-max 1) (vision-range 2)
  (help "Produces Earth Mana."))

(unit-type snode (name "Sorcery Node") (image-name "spec-node")
  (acp-per-turn 3) (hp-max 1) (vision-range 2)
  (help "Produces Sorcery Mana."))

(unit-type cnode (name "Chaos Node") (image-name "spec-node")
  (acp-per-turn 3) (hp-max 1) (vision-range 2)
  (help "Produces Chaos Mana."))

(unit-type lnode (name "Life Node") (image-name "spec-node")
  (acp-per-turn 3) (hp-max 1) (vision-range 2)
  (help "Produces Life Mana."))

(unit-type dnode (name "Death Node") (image-name "spec-node")
  (acp-per-turn 3) (hp-max 1) (vision-range 2)
  (help "Produces Death Mana."))

(unit-type temple (name "Ancient Temple") (image-name "spec-temple")
  (acp-per-turn 0) (hp-max 1) (vision-range 2)
  (help "Provides research"))

(define node-types (enode snode cnode lnode dnode temple))



(unit-type fish (name "Fish") (image-name "spec-heart")
  (acp-per-turn 0) (hp-max 1) (vision-range 0)
  (help "Produces gold and anvils."))

(unit-type goldmine (name "Gold Mine") (image-name "spec-heart")
  (acp-per-turn 0) (hp-max 1) (vision-range 0)
  (help "Produces gold."))

(unit-type ironmine (name "Iron Mine") (image-name "spec-heart")
  (acp-per-turn 0) (hp-max 1) (vision-range 0)
  (help "Produces iron."))

(unit-type quarry (name "Quarry") (image-name "spec-heart")
  (acp-per-turn 0) (hp-max 1) (vision-range 0)
  (help "Produces stone."))

(unit-type lumber (name "Groves") (image-name "spec-heart")
  (acp-per-turn 0) (hp-max 1) (vision-range 0)
  (help "Produces wood."))

(define resource-types (fish goldmine ironmine quarry lumber))




;; SPELLS

(unit-type powerword (name "Power Word, Kill") (image-name "spec-dspell")
  	(acp-per-turn 10) (hp-max 1) (acp-to-fire 1) (range 5)  (cp 3)
	(help "Produces traits and such"))

(unit-type deathwalk (name "Deathwalker") (image-name "spec-dspell")
  (acp-per-turn 3) (hp-max 1) (help "Produces traits and such"))

(unit-type wastemaker (name "Wasteland") (image-name "spec-dspell")
	(acp-per-turn 21) (hp-max 1) (acp-to-detonate 1) (hp-per-detonation 1) (cp 3) 
	(help "Produces traits and such"))

(unit-type riddle (name "Labrynthine Riddle") (image-name "spec-dspell")
  (acp-independent true) (hp-max 1) (help "Produces traits and such"))

(unit-type breath (name "Gray Breath") (image-name "spec-dspell")
  (acp-per-turn 3) (hp-max 1) (help "Produces traits and such"))

(unit-type seance (name "Seance") (image-name "spec-dspell")
  (acp-independent true) (hp-max 1) (help "Produces traits and such"))

(unit-type poltergeist (name "Poltergeist") (image-name "spec-dspell")
  	(acp-per-turn 3) (hp-max 1) 
	(help "Used to take control of resource centers and ruins"))

(unit-type bonearrow (name "Bone Arrows") (image-name "spec-dspell")
	(acp-per-turn 3) (hp-max 1) (acp-to-fire 1) (range 3) (cp 3) 
	(help "Moderate Damage"))

(unit-type deathtap (name "Deathtap") (image-name "spec-dspell")
  	(acp-independent true) (hp-max 1) (help "Provides death mana"))





(unit-type ghoul (name "Ghouls") (image-name "spec-ghoul")
  (acp-per-turn 6) (hp-max 35) (vision-range 1) (cp 9)
  (help "Mean"))

(unit-type skeleton (name "Skeletons") (image-name "spec-skeleton")
  (acp-per-turn 6) (hp-max 15) (vision-range 1) (cp 6)
  (help "Brittle"))

(unit-type vampyre (name "Vampyres") (image-name "spec-vampyre")
  (acp-per-turn 8) (hp-max 50) (vision-range 2) (cp 9)
  (help "Regenerates quickly, most powerful undead unit"))

(unit-type seeker (name "Seekers") (image-name "spec-seeker") (cp 6)
  (acp-per-turn 8) (hp-max 5) (help "Animated, severed hands."))

(unit-type iblis (name "Iblis") (image-name "spec-iblis") (cp 24)
  (acp-per-turn 4) (hp-max 150) 
;; (has-opinions true) (opinions -3 -3 -3 -3 -3 -3 -3 -3 -3 -3 -3 -3 -3 -3 -3)
(help "Learn from Ad Avis"))

(unit-type lich (name "Lich") (image-name "spec-lich") (cp 12)
  (acp-per-turn 6) (hp-max 50) (acp-to-fire 1) (range 1)
  (help "Undead magician"))



(unit-type naturevoice (name "Nature's Voice") (image-name "spec-nspell")
  (acp-independent true) (hp-max 1) (vision-range 100) (cp 6)
	(help "Knowledge of the entire world"))

(unit-type tsunami (name "Tsunami") (image-name "spec-nspell")
	(acp-per-turn 51) (hp-max 1) (acp-to-detonate 1) (hp-per-detonation 1) (cp 6) 
	(help "Monster wave"))

(unit-type mountainchange (name "Mountain") (image-name "spec-nspell")
	(acp-per-turn 48) (hp-max 1) (acp-to-detonate 1) (hp-per-detonation 1) (cp 6) 
	(help "Change terrain into mountain"))

(unit-type beguile (name "Beguile") (image-name "spec-nspell")
	(acp-per-turn 3) (hp-max 1) 
	(help "Charm many things"))

(unit-type earthquake (name "Earthquake") (image-name "spec-nspell")
	(acp-per-turn 21) (hp-max 1) (acp-to-detonate 1) (hp-per-detonation 1) (cp 6) 
	(help "Damages units and cities in a wide area"))

(unit-type deepchange (name "Deep") (image-name "spec-nspell")
	(acp-per-turn 48) (hp-max 1) (acp-to-detonate 1) (hp-per-detonation 1) (cp 6) 
	(help "Produces traits and such"))

(unit-type shallowchange (name "Shallow") (image-name "spec-nspell")
	(acp-per-turn 48) (hp-max 1) (acp-to-detonate 1) (hp-per-detonation 1) (cp 6) 
	(help "Changes deep water to shallow."))

(unit-type charmfish (name "Oceanic Charm") (image-name "spec-nspell")
  	(acp-per-turn 4) (hp-max 1) (acp-to-fire 1) (range 10) (cp 3) 
	(help "Take control of sea creatures"))

(unit-type hillchange (name "Hill") (image-name "spec-nspell")
	(acp-per-turn 48) (hp-max 1) (acp-to-detonate 1) (hp-per-detonation 1) (cp 3) 
	(help "Produces traits and such"))

(unit-type plainchange (name "Plains") (image-name "spec-nspell")
	(acp-per-turn 48) (hp-max 1) (acp-to-detonate 1) (hp-per-detonation 1) (cp 3)  
	(help "Produces traits and such"))

(unit-type swampchange (name "Swamp") (image-name "spec-nspell")
	(acp-per-turn 48) (hp-max 1) (acp-to-detonate 1) (hp-per-detonation 1) (cp 3) 
	(help "Produces traits and such"))

(unit-type charmbeast (name "Charm Beast") (image-name "spec-nspell")
	(acp-per-turn 4) (hp-max 1) (acp-to-fire 1) (range 10) (cp 3) 
	(help "Take control of beasts and birds"))



;; Nature Creatures
(unit-type earth1 (name "Minor Earth Elemental") (image-name "spec-earth")
	(acp-per-turn 6) (hp-max 20) (vision-range 1) (cp 6)
	(help "Small man of mud"))

(unit-type earth2 (name "Earth Elemental") (image-name "spec-earth")
  	(acp-per-turn 6) (hp-max 40) (vision-range 1) (cp 9)
  	(help "Big man of dirt"))

(unit-type earth3 (name "Major Earth Elemental") (image-name "spec-earth")
  	(acp-per-turn 4) (hp-max 80) (vision-range 1) (cp 12)
  	(help "Huge man of stone"))

(unit-type crebain (name "Crebain") (image-name "spec-crebain")
  	(acp-per-turn 8) (hp-max 3)  (vision-range 3) (cp 3)
	(help "Spies"))

(unit-type leviathan (name "Leviathan") (image-name "spec-leviathan")
	(acp-per-turn 6) (hp-max 300) (cp 18)
	(help "Enormous creature"))

(define nature-types (earth1 earth2 earth3 crebain leviathan))



(unit-type canticle (name "Chaos Canticle") (image-name "spec-cspell")
	(acp-per-turn 1) (hp-max 1)  (acp-to-fire 1) (range 80) (cp 12)
	(help "One shot per turn"))

(unit-type volcano (name "Volcano") (image-name "spec-cspell")
	(acp-per-turn 1) (hp-max 1) (acp-to-detonate 1) (hp-per-detonation 0) (cp 12)
	(help "Explode each turn to cause damage and wasteland"))

(unit-type meteor (name "Meteor Storm") (image-name "spec-cspell")
	(acp-per-turn 51) (hp-max 1) (acp-to-detonate 1) (hp-per-detonation 1) (cp 3) 
	(help "Produces traits and such"))

(unit-type disintegrate (name "Disintegrate") (image-name "spec-cspell")
	(acp-per-turn 1) (hp-max 1) (acp-to-fire 1) (range 30) (cp 3) 
	(help "Destroys any non-leader, damages any place"))

(unit-type flamewall (name "Wall of Flames") (image-name "spec-cspell")
	(acp-per-turn 10) (hp-max 50) (cp 6)
	(help "Produces traits and such"))

(unit-type fireball (name "Fireball") (image-name "spec-cspell")
	(acp-per-turn 21) (hp-max 1) (acp-to-detonate 1) (hp-per-detonation 1) (cp 3)
	(help "Produces traits and such"))

(unit-type flamestrike (name "Flame Strike") (image-name "spec-cspell")
	(acp-per-turn 1) (hp-max 1) (acp-to-fire 1) (range 10) (cp 3) 
	(help "Produces traits and such"))

(unit-type flamearrow (name "Flame Arrow") (image-name "spec-cspell")
	(acp-per-turn 5) (hp-max 1) (acp-to-fire 1) (range 3) (cp 3) 
	(help "Produces traits and such"))


;; Chaos Creatures
(unit-type fire1 (name "Minor Fire Elemental") (image-name "spec-fire")
	(acp-per-turn 6) (hp-max 15) (vision-range 1) (cp 6)
	(help "Flame"))

(unit-type fire2 (name "Fire Elemental") (image-name "spec-fire")
	(acp-per-turn 6) (hp-max 30) (vision-range 1) (cp 9)
	(help "Sparks"))

(unit-type fire3 (name "Major Fire Elemental") (image-name "spec-fire")
  	(acp-per-turn 4) (hp-max 60) (vision-range 1) (cp 12)
  	(help "Magma"))

(unit-type hellhound (name "Gévaudan") (image-name "spec-hellhound")
  	(acp-per-turn 3) (hp-max 30) (cp 6)
	(help "Produces traits and such"))

(unit-type chaoscreature (name "The Ordros") (image-name "spec-chaoscreature")
  	(acp-per-turn 3) (hp-max 100) (cp 24)
	(help "Produces traits and such"))

(unit-type demon (name "Demon") (image-name "spec-demon")
  	(acp-per-turn 3) (hp-max 100) (cp 18)
	(help "Produces traits and such"))

(define chaos-types (fire1 fire2 fire3 hellhound chaoscreature demon))




(unit-type restore (name "Restoration") (image-name "spec-lspell")
  	(acp-per-turn 48) (hp-max 1) (acp-to-detonate 1) (hp-per-detonation 1) (cp 3)
	(help "Produces traits and such"))

(unit-type enlighten (name "Enlighten") (image-name "spec-lspell")
	(acp-per-turn 1) (hp-max 1) (acp-to-fire 1) (range 20) (cp 3) 
	(help "Take control of any one creature"))

(unit-type doorway (name "Doorway") (image-name "spec-doorway")
	(acp-per-turn 3) (hp-max 50) (cp 6)
	(help "A tower of light that produces spirits"))

(unit-type spiritwall (name "Spirit Wall") (image-name "spec-lspell")
	(acp-independent true) (hp-max 50) (cp 6)
	(help "Placed in empty terrain to block movement"))

(unit-type sunray (name "Sunray") (image-name "spec-lspell")
	(acp-per-turn 1) (hp-max 1) (acp-to-fire 1) (range 15) (cp 3) 
	(help "Very damaging, especially to evil units"))

(unit-type improveheal (name "Improved Heal") (image-name "spec-lspell")
	(acp-per-turn 3) (hp-max 1) (acp-to-fire 1) (range 15) (cp 3) 
	(help "Produces traits and such"))

(unit-type sanctify (name "Sanctify") (image-name "spec-lspell")
	(acp-independent true) (hp-max 1) (cp 6)
	(help "Used on fortresses to improve their strength"))

(unit-type bless (name "Bless") (image-name "spec-lspell")
  	(acp-independent true) (hp-max 1) (cp 6)
	(help "Used on cities to protect cities"))

(unit-type justiceeye (name "Eye of Justice") (image-name "spec-lspell")
  	(acp-independent true) (hp-max 1) (vision-range 10) (cp 6)
	(help "Allows a watchtower to see farther"))

(unit-type bolt (name "Bolt") (image-name "spec-lspell")
  	(acp-per-turn 1) (hp-max 1) (acp-to-fire 1) (range 5) (cp 3)  
	(help "Slightly damaging, moreso to evil creatures"))

(unit-type heal (name "Heal") (image-name "spec-lspell")
  	(acp-per-turn 1) (hp-max 1) (acp-to-fire 1) (range 10) (cp 3)  
	(help "Restore some hitpoints to a single unit"))


;; Life Creatures
(unit-type spirit1 (name "Lesser Spirits") (image-name "spec-spirit")
  	(acp-per-turn 6) (hp-max 15) (vision-range 1) (cp 6)
  	(help "Seraphim"))

(unit-type spirit2 (name "Spirits") (image-name "spec-spirit")
  	(acp-per-turn 6) (hp-max 30) (vision-range 1) (cp 9)
  	(help "Cherubrim"))

(unit-type spirit3 (name "Greater Spirit") (image-name "spec-spirit")
  	(acp-per-turn 6) (hp-max 60) (vision-range 1) (cp 12)
  	(help "Mike")) 

(unit-type guardian (name "Guardian") (image-name "spec-spirit") (cp 6)
  	(acp-per-turn 3) (hp-max 1) (help "Protects forces around it"))

(define life-types (spirit1 spirit2 spirit3 guardian))



(unit-type castle (name "Phantom Castle") (image-name "spec-castle")
	(acp-per-turn 3) (hp-max 100) (cp 6)
	(help "Produces phantom creatures and protects troops"))

(unit-type greatcharm (name "Greater Charm") (image-name "spec-sspell")
  	(acp-per-turn 3) (hp-max 1) (acp-to-fire 1) (range 25) (cp 3)
	(help "Produces traits and such"))

(unit-type truedivine (name "True Divination") (image-name "spec-sspell")
  	(acp-independent true) (hp-max 1) (vision-range 60) (cp 6)
	(help "Eldritch knowledge of Opal"))

(unit-type guardward (name "Guards and Wards") (image-name "spec-sspell")
  	(acp-independent true) (hp-max 50) (cp 6)
	(help "Protects fortresses"))

(unit-type prismwall (name "Prismatic Wall") (image-name "spec-sspell")
  	(acp-per-turn 10) (hp-max 40) (cp 6)
	(help "Guards cities and fortresses.  Sometimes incredibly damaging, sometimes
	beguiling."))

(unit-type greatdivine (name "Greater Divination") (image-name "spec-sspell")
  	(acp-independent true) (hp-max 1) (vision-range 30) 
	(help "Mystical knowledge of Opal"))

(unit-type lightning (name "Lightning Bolt") (image-name "spec-sspell")
	(acp-per-turn 1) (hp-max 1) (acp-to-fire 1) (range 4) (cp 3)  
	(help "Damaging"))

(unit-type charm (name "Charm") (image-name "spec-sspell")
	(acp-per-turn 1) (hp-max 1) (acp-to-fire 1) (range 3) (cp 3) 
	(help "Produces traits and such"))

(unit-type divine (name "Divination") (image-name "spec-sspell")
	(acp-independent true) (hp-max 1) (vision-range 15) 
	(help "Simple knowledge of Opal"))


;; Sorcery Creatures
(unit-type air1 (name "Minor Air Elemental") (image-name "spec-air")
	(acp-per-turn 8) (hp-max 10) (vision-range 1) (cp 6)
	(help "Small gust"))

(unit-type air2 (name "Air Elemental") (image-name "spec-air")
	(acp-per-turn 8) (hp-max 20) (vision-range 1) (cp 9)
	(help "Big whirlwind"))

(unit-type air3 (name "Major Air Elemental") (image-name "spec-air")
	(acp-per-turn 8) (hp-max 40) (vision-range 1) (cp 12)
	(help "Great Tornado"))

(unit-type phantombeast (name "Phantom Beast") (image-name "spec-phantomwarrior")
	(acp-per-turn 4) (hp-max 100) (cp 12)
	(help "A powerful, though ephemeral, creature"))

(unit-type phantomwarrior (name "Phantom Warriors") (image-name "spec-phantombeast")
	(acp-per-turn 6) (hp-max 10) (cp 6)
	(help "Easily killed, but powerful attackers"))


(define sorcery-types (air1 air2 air3 phantombeast phantomwarrior))



(define spell-types (powerword iblis lich deathwalk wastemaker riddle breath seance poltergeist bonearrow 
	seeker deathtap naturevoice leviathan tsunami mountainchange beguile earthquake deepchange shallowchange 
	charmfish hillchange plainchange swampchange charmbeast crebain canticle demon chaoscreature 
	volcano meteor disintegrate flamewall hellhound fireball flamestrike flamearrow restore enlighten 
	doorway spiritwall sunray guardian improveheal sanctify bless justiceeye bolt heal castle greatcharm 
	truedivine guardward phantombeast prismwall greatdivine lightning charm phantomwarrior divine 
	air1 air2 air3 earth1 spirit1 spirit2 spirit3 fire1 fire2 fire3 skeleton ghoul vampyre)
)

(define shot-types (powerword bonearrow charmfish charmbeast disintegrate
	flamestrike flamearrow enlighten sunray improveheal bolt greatcharm lightning charm)
)

(define summon-types (iblis lich poltergeist seeker leviathan crebain demon chaoscreature hellhound guardian 
	phantombeast phantomwarrior air1 air2 air3 earth1 spirit1 spirit2 spirit3 fire1 fire2 fire3 skeleton 
	ghoul vampyre poltergeist))

(define explode-types (wastemaker tsunami earthquake meteor fireball restore mountainchange 
	deepchange shallowchange hillchange plainchange swampchange)
)

(define enchantment-types (riddle deathtap flamewall sanctify bless justiceeye guardward prismwall)
)

(define create-types (castle deathwalk naturevoice canticle volcano doorway spiritwall divine truedivine greatdivine))

(define land-types (king usurper minister shogun loremaster emperor shah pharoah greyone wocke sword halberd bow
   cavalry engineer catapult mage scout gladiator longbow crusader assassin cannon dervish weaver earth1
   earth2 earth3 fire1 fire2 fire3 skeleton zombie ghoul spectre vampyre spear shaman thug champ tyrannosaur
   apatosaur wolves bears tower keep citadel lich canticle chaoscreature doorway spiritwall castle phantombeast
   phantomwarrior seeker hellhound volcano thuvi))

(define amphibious-types (talon spawn croc dajaturtle churambi gibambi ulitar slaver sorcerer))

(define deep-types (axehead bospallian dajawyrm thurastes leviathan))

(define water-types (squid tentacle mold sludge plesiosaur cutter whaler tsunami))

(define air-types (ferak royalferak pterodactyl vampyre spirit1 spirit2 spirit3 air1 air2 air3 gwyrm wyrm
   hatchling ghost crebain demon guardian poltergeist))

(define all-types (append land-types amphibious-types deep-types water-types air-types))

(define mage-types (sorcerer mage dervish weaver shaman spectre vampyre lich gwyrm))



(add place-types advanced true)
(add place-types use-own-cell true)
(add (axecorpse boscorpse thucorpse deadwild) advanced true)
(add (axecorpse boscorpse thucorpse deadwild) use-own-cell true)
(add ruin-types advanced true)
(add ruin-types use-own-cell true)
(add fortress-types advanced true)
(add fortress-types use-own-cell true)
(add wilderness-types advanced true)
(add wilderness-types use-own-cell true)
(add node-types advanced true)
(add node-types use-own-cell true)
(add resource-types advanced true)
(add resource-types use-own-cell true)

(add resource-types see-always true)
(add node-types see-always true)
(add place-types see-always true)



;;; Materials
(material-type gold (name "Gold") (treasury true)
  (help "Gold"))
(material-type wood (name "Wood") (treasury true)
  (help "Wood"))
(material-type stone (name "Stone") (treasury true)
  (help "Stone"))
(material-type mana (name "Mana") (treasury true)
  (help "Magical power"))
(material-type study (name "Study") (treasury true)
  (help "For research"))
(material-type anvil (name "Industry") (treasury true)
  (help "Production"))
(material-type corpse (name "Corpses")
  (help "Used for evil acts"))
(material-type timer (name "X")
  (help "To keep things alive, like ghosts, which aren't really alive, which is weird."))



;;; Advances
(advance-type s-powerword (name "Power Word, Kill (500)") (rp 500))
(advance-type s-iblis (name "Summon Iblis (500)") (rp 500))
(advance-type s-lich (name "Summon Lich (500)") (rp 500))
(advance-type s-deathwalk (name "Deathwalk (400)") (rp 400))
(advance-type s-wastemaker (name "Wasteland (400)") (rp 400))
(advance-type s-vampyre (name "Summon Vampyre (400)") (rp 400))
(advance-type s-riddle (name "Labrynthine Riddle (300)") (rp 300))
(advance-type s-breath (name "Gray Breath (300)") (rp 300))
(advance-type s-ghoul (name "Summon Ghouls (300)") (rp 300))
(advance-type s-seance (name "Seance (200)") (rp 200))
(advance-type s-poltergeist (name "Poltergeist (200)") (rp 200))
(advance-type s-bonearrow (name "Bone Arrows (200)") (rp 200))
(advance-type s-seeker (name "Summon Seekers (200)") (rp 100))
(advance-type s-deathtap (name "Deathtap (100)") (rp 100))
(advance-type s-skeleton (name "Summon Skeletons (100)") (rp 100))
(advance-type s-naturevoice (name "Nature's Voice (500)") (rp 500))
(advance-type s-leviathan (name "Summon Leviathan (500)") (rp 500))
(advance-type s-tsunami (name "Tsunami (500)") (rp 500))
(advance-type s-mountainchange (name "Mountains (400)") (rp 400))
(advance-type s-beguile (name "Beguile (400)") (rp 400))
(advance-type s-earthquake (name "Earthquake (400)") (rp 400))
(advance-type s-deepchange (name "Deepen (300)") (rp 300))
(advance-type s-shallowchange (name "Shallow (300)") (rp 300))
(advance-type s-charmfish (name "Oceanic Charm (300)") (rp 300))
(advance-type s-hillchange (name "Hills (200)") (rp 200))
(advance-type s-plainchange (name "Plains (200)") (rp 200))
(advance-type s-swampchange (name "Swamp (200)") (rp 200))
(advance-type s-charmbeast (name "Charm Beast (100)") (rp 100))
(advance-type s-crebain (name "Summon Crebain (100)") (rp 100))
(advance-type s-earth1 (name "Summon Minor Earth Elemental (100)") (rp 100))
(advance-type s-canticle (name "Chaos Canticle (500)") (rp 500))
(advance-type s-demon (name "Summon Demon (500)") (rp 500))
(advance-type s-chaoscreature (name "The Ordros (500)") (rp 500))
(advance-type s-volcano (name "Raise Volcano (400)") (rp 400))
(advance-type s-meteor (name "Meteor Shower (400)") (rp 400))
(advance-type s-fire3 (name "Summon Major Fire Elemental (400)") (rp 400))
(advance-type s-disintegrate (name "Disintegrate (300)") (rp 300))
(advance-type s-flamewall (name "Wall of Flames (300)") (rp 300))
(advance-type s-fire2 (name "Summon Fire Elemental (300)") (rp 300))
(advance-type s-hellhound (name "Summon Hell Hounds (200)") (rp 200))
(advance-type s-fireball (name "Fireball (200)") (rp 200))
(advance-type s-flamestrike (name "Flame Strike (100)") (rp 100))
(advance-type s-flamearrow (name "Flame Arrows (100)") (rp 100))
(advance-type s-fire1 (name "Summon Minor Fire Elemental (100)") (rp 100))
(advance-type s-restore (name "Restoration (500)") (rp 500))
(advance-type s-enlighten (name "Enlighten (500)") (rp 500))
(advance-type s-doorway (name "Doorway (500)") (rp 500))
(advance-type s-spiritwall (name "Spirit Wall (400)") (rp 400))
(advance-type s-spirit3 (name "Summon Major Spirit (400)") (rp 400))
(advance-type s-guardian (name "Summon Guardian (400)") (rp 400))
(advance-type s-sunray (name "Sunray (300)") (rp 300))
(advance-type s-improveheal (name "Improved Heal (300)") (rp 300))
(advance-type s-sanctify (name "Sanctify (300)") (rp 300))
(advance-type s-bless (name "Bless (200)") (rp 200))
(advance-type s-justiceeye (name "Eye of Justice (200)") (rp 200))
(advance-type s-spirit2 (name "Summon Spirits (200)") (rp 200))
(advance-type s-bolt (name "Bolt (100)") (rp 100))
(advance-type s-heal (name "Heal (100)") (rp 100))
(advance-type s-spirit1 (name "Summon Minor Spirits (100)") (rp 100))
(advance-type s-castle (name "Phantom Castle (500)") (rp 500))
(advance-type s-greatcharm (name "Greater Charm (500)") (rp 500))
(advance-type s-truedivine (name "True Divination (500)") (rp 500))
(advance-type s-guardward (name "Guards and Wards (400)") (rp 400))
(advance-type s-phantombeast (name "Phantom Beast (400)") (rp 400))
(advance-type s-air3 (name "Summon Major Air Elemental (400)") (rp 400))
(advance-type s-prismwall (name "Prismatic Walls (300)") (rp 300))
(advance-type s-greatdivine (name "Greater Divination (300)") (rp 300))
(advance-type s-air2 (name "Summon Air Elemental (300)") (rp 300))
(advance-type s-lightning (name "Lightning Bolt (200)") (rp 200))
(advance-type s-charm (name "Charm (200)") (rp 200))
(advance-type s-phantomwarrior (name "Phantom Warriors (100)") (rp 100))
(advance-type s-divine (name "Divination (100)") (rp 100))
(advance-type s-air1 (name "Summon Minor Air Elemental (100)") (rp 100))

(advance-type sbook1-r (name "First Circle Sorcery (1k)") (rp 1000))
(advance-type sbook2-r (name "Second Circle Sorcery (1k)") (rp 1000))
(advance-type sbook3-r (name "Third Circle Sorcery (1k)") (rp 1000))
(advance-type sbook4-r (name "Fourth Circle Sorcery (1k)") (rp 1000))
(advance-type sbook5-r (name "Fifth Circle Sorcery (1k)") (rp 1000))
(advance-type cbook1-r (name "Fadua Chaos (1k)") (rp 1000))
(advance-type cbook2-r (name "Rem'lith Chaos (1k)") (rp 1000))
(advance-type cbook3-r (name "Utu Chaos (1k)") (rp 1000))
(advance-type cbook4-r (name "Zeem Chaos (1k)") (rp 1000))
(advance-type cbook5-r (name "Pure Chaos (1k)") (rp 1000))
(advance-type nbook1-r (name "First Secret of Nature (1k)") (rp 1000))
(advance-type nbook2-r (name "Second Secret of Nature (1k)") (rp 1000))
(advance-type nbook3-r (name "Third Secret of Nature (1k)") (rp 1000))
(advance-type nbook4-r (name "Fourth Secret of Nature (1k)") (rp 1000))
(advance-type nbook5-r (name "Fifth Secret of Nature (1k)") (rp 1000))
(advance-type lbook1-r (name "The Law of Life (1k)") (rp 1000))
(advance-type lbook2-r (name "The Way of Life (1k)") (rp 1000))
(advance-type lbook3-r (name "The Knowledge of Life (1k)") (rp 1000))
(advance-type lbook4-r (name "The Language of Life (1k)") (rp 1000))
(advance-type lbook5-r (name "The Meaning of Life (1k)") (rp 1000))
(advance-type dbook1-r (name "The Law of Death (1k)") (rp 1000))
(advance-type dbook2-r (name "The Way of Death (1k)") (rp 1000))
(advance-type dbook3-r (name "The Knowledge of Death (1k)") (rp 1000))
(advance-type dbook4-r (name "The Language of Death (1k)") (rp 1000))
(advance-type dbook5-r (name "The Meaning of Death (1k)") (rp 1000))


(table advance-needed-to-build
  (u* a* false)
  ;; units
	(powerword s-powerword true)
	(iblis s-iblis true)
	(lich s-lich true)
	(deathwalk s-deathwalk true)
	(wastemaker s-wastemaker true)
	(riddle s-riddle true)
	(breath s-breath true)
	(seance s-seance true)
	(poltergeist s-poltergeist true)
	(bonearrow s-bonearrow true)
	(seeker s-seeker true)
	(deathtap s-deathtap true)
	(naturevoice s-naturevoice true)
	(leviathan s-leviathan true)
	(tsunami s-tsunami true)
	(mountainchange s-mountainchange true)
	(beguile s-beguile true)
	(earthquake s-earthquake true)
	(deepchange s-deepchange true)
	(shallowchange s-shallowchange true)
	(charmfish s-charmfish true)
	(hillchange s-hillchange true)
	(plainchange s-plainchange true)
	(swampchange s-swampchange true)
	(charmbeast s-charmbeast true)
	(crebain s-crebain true)
	(canticle s-canticle true)
	(demon s-demon true)
	(chaoscreature s-chaoscreature true)
	(volcano s-volcano true)
	(meteor s-meteor true)
	(disintegrate s-disintegrate true)
	(flamewall s-flamewall true)
	(hellhound s-hellhound true)
	(fireball s-fireball true)
	(flamestrike s-flamestrike true)
	(flamearrow s-flamearrow true)
	(restore s-restore true)
	(enlighten s-enlighten true)
	(doorway s-doorway true)
	(spiritwall s-spiritwall true)
	(sunray s-sunray true)
	(guardian s-guardian true)
	(improveheal s-improveheal true)
	(sanctify s-sanctify true)
	(bless s-bless true)
	(justiceeye s-justiceeye true)
	(bolt s-bolt true)
	(heal s-heal true)
	(castle s-castle true)
	(greatcharm s-greatcharm true)
	(truedivine s-truedivine true)
	(guardward s-guardward true)
	(phantombeast s-phantombeast true)
	(prismwall s-prismwall true)
	(greatdivine s-greatdivine true)
	(lightning s-lightning true)
	(charm s-charm true)
	(phantomwarrior s-phantomwarrior true)
	(divine s-divine true)
	(air1 s-air1 true)
	(air2 s-air2 true)
	(air3 s-air3 true)
	(earth1 s-earth1 true)
	(spirit1 s-spirit1 true)
	(spirit2 s-spirit2 true)
	(spirit3 s-spirit3 true)
	(fire1 s-fire1 true)
	(fire2 s-fire2 true)
	(fire3 s-fire3 true)
	(skeleton s-skeleton true)
	(ghoul s-ghoul true)
	(vampyre s-vampyre true)

)


(set side-can-research true)

(table advance-consumption-per-rp
  (a* m* 0)
  (a* study 1)
  )

(table advance-needed-to-research
  (a* a* false)

;; SORCERY

  (sbook2-r (sbook1-r) true)
  (s-phantomwarrior (sbook1-r) true)
  (s-divine (sbook1-r) true)
  (s-air1 (sbook1-r) true)

  (sbook3-r (sbook2-r) true)
  (s-charm (sbook2-r) true)
  (s-lightning (sbook2-r) true)

  (sbook4-r (sbook3-r) true)
  (s-air2 (sbook3-r) true)
  (s-greatdivine (sbook3-r) true)
  (s-prismwall (sbook3-r) true)

  (sbook5-r (sbook4-r) true)
  (s-air3 (sbook4-r) true)
  (s-phantombeast (sbook4-r) true)
  (s-guardward (sbook4-r) true)

  (s-truedivine (sbook5-r) true)
  (s-greatcharm (sbook5-r) true)
  (s-castle (sbook5-r) true)

;; NATURE

  (nbook2-r (nbook1-r) true)
  (s-earth1 (nbook1-r) true)
  (s-crebain (nbook1-r) true)
  (s-charmbeast (nbook1-r) true)

  (nbook3-r (nbook2-r) true)
  (s-swampchange (nbook2-r) true)
  (s-plainchange (nbook2-r) true)
  (s-hillchange (nbook2-r) true)

  (nbook4-r (nbook3-r) true)
  (s-charmfish (nbook3-r) true)
  (s-shallowchange (nbook3-r) true)
  (s-deepchange (nbook3-r) true)

  (nbook5-r (nbook4-r) true)
  (s-earthquake (nbook4-r) true)
  (s-beguile (nbook4-r) true)
  (s-mountainchange (nbook4-r) true)

  (s-tsunami (nbook5-r) true)
  (s-leviathan (nbook5-r) true)
  (s-naturevoice (nbook5-r) true)

;; CHAOS

  (cbook2-r (cbook1-r) true)
  (s-fire1 (cbook1-r) true)
  (s-flamearrow (cbook1-r) true)
  (s-flamestrike (cbook1-r) true)

  (cbook3-r (cbook2-r) true)
  (s-fireball (cbook2-r) true)
  (s-hellhound (cbook2-r) true)

  (cbook4-r (cbook3-r) true)
  (s-fire2 (cbook3-r) true)
  (s-flamewall (cbook3-r) true)
  (s-disintegrate (cbook3-r) true)

  (cbook5-r (cbook4-r) true)
  (s-fire3 (cbook4-r) true)
  (s-meteor (cbook4-r) true)
  (s-volcano (cbook4-r) true)

  (s-chaoscreature (cbook5-r) true)
  (s-demon (cbook5-r) true)
  (s-canticle (cbook5-r) true)

;; LIFE

  (lbook2-r (lbook1-r) true)
  (s-spirit1 (lbook1-r) true)
  (s-heal (lbook1-r) true)
  (s-bolt (lbook1-r) true)

  (lbook3-r (lbook2-r) true)
  (s-spirit2 (lbook2-r) true)
  (s-justiceeye (lbook2-r) true)
  (s-bless (lbook2-r) true)

  (lbook4-r (lbook3-r) true)
  (s-sanctify (lbook3-r) true)
  (s-improveheal (lbook3-r) true)
  (s-sunray (lbook3-r) true)

  (lbook5-r (lbook4-r) true)
  (s-guardian (lbook4-r) true)
  (s-spirit3 (lbook4-r) true)
  (s-spiritwall (lbook4-r) true)

  (s-doorway (lbook5-r) true)
  (s-enlighten (lbook5-r) true)
  (s-restore (lbook5-r) true)

;; DEATH

  (dbook2-r (dbook1-r) true)
  (s-skeleton (dbook1-r) true)
  (s-deathtap (dbook1-r) true)
  (s-seeker (dbook1-r) true)

  (dbook3-r (dbook2-r) true)
  (s-bonearrow (dbook2-r) true)
  (s-poltergeist (dbook2-r) true)
  (s-seance (dbook2-r) true)

  (dbook4-r (dbook3-r) true)
  (s-ghoul (dbook3-r) true)
  (s-breath (dbook3-r) true)
  (s-riddle (dbook3-r) true)

  (dbook5-r (dbook4-r) true)
  (s-vampyre (dbook4-r) true)
  (s-wastemaker (dbook4-r) true)
  (s-deathwalk (dbook4-r) true)

  (s-lich (dbook5-r) true)
  (s-iblis (dbook5-r) true)
  (s-powerword (dbook5-r) true)

)


(terrain-type sea (char ".")
  (help "deep water"))
(terrain-type shallows (char ",")
  (help "shallow coastal water and lakes"))
(terrain-type swamp (char "="))
(terrain-type desert (image-name "adv-desert") (char "~")
  (help "dry open terrain"))
(terrain-type land (image-name "adv-plain") (char "+")
  (help "open flat or rolling country"))
(terrain-type forest (image-name "adv-forest") (char "%"))
(terrain-type mountains (image-name "adv-mountains") (char "^"))
(terrain-type ice  (image-name "adv-ice") (char "_"))
(terrain-type neutral (image-name "gray") (char "-"))
(terrain-type hills (image-name "adv-hills"))
(terrain-type semi-desert (image-name "adv-semi-desert"))
(terrain-type steppe (image-name "adv-steppe"))
(terrain-type deepforest (image-name "adv-forest"))
(terrain-type wasteland (image-name "gray"))
(terrain-type river (char "<")
  (subtype border) (subtype-x river-x))
(terrain-type beach (char "|")
  (subtype border))
(terrain-type cliffs (image-name "cliffs")
  (subtype border))
(terrain-type road (char ">")
  (subtype connection) (subtype-x road-x))
(terrain-type mud (image-name "brown")
  (subtype coating))
(terrain-type snow (image-name "ice")
  (subtype coating))
(terrain-type jungle (image-name "adv-forest"))

(add (sea shallows) liquid true)

(define sea-t* (sea shallows))

(define land-t* (jungle deepforest wasteland desert semi-desert hills steppe land forest mountains))

;;; Unit-unit capacities.


(table unit-size-as-occupant
  ;; Disable occupancy by default.
	(u* u* 99)
	(sludge (ulitar) 1)
	(basic-types (cutter) 1)
	(specific-types (cutter) 1)
	(basic-types fortress-types 1)
	(specific-types fortress-types 1)
	(basic-types place-types 1)
	(specific-types place-types 1)
	(death-types place-types 1)
	((shaman spear champ thug) place-types 1)
	((shaman spear champ thug) (cutter) 1)
	(enchantment-types place-types 1)
	(enchantment-types fortress-types 1)
	(deathtap ruin-types 1)
	(riddle resource-types 1)
)

(add ruin-types capacity 1)

(add (ulitar) capacity 1)

(add (cutter) capacity 2)

(add (keep) capacity 4)

(add (tower) capacity 1)

(add (castle) capacity 5)

(add (citadel) capacity 6)

(add place-types capacity 8)

(table occupant-max
	(tower all-types 1)
	(keep all-types 4)
	(citadel all-types 6)
	(castle all-types 7)
	(place-types all-types 8)
	(cutter catapult 1)
	(cutter cannon 1)
	
	(ruin-types u* 0)
	(resource-types u* 0)
	(fortress-types enchantment-types 0)
	(place-types enchantment-types 0)
	(tower justiceeye 1)
	(fortress-types prismwall 1)
	(place-types prismwall 1)
	(fortress-types guardward 1)
	(place-types bless 1)
	(fortress-types sanctify 1)
	(ruin-types deathtap 1)
	(place-types flamewall 1)
	(fortress-types flamewall 1)
	(fortress-types riddle 1)
	
)

;;; Unit-terrain capacities.

(table unit-size-in-terrain
  (u* t* 25)
  (place-types t* 100)
  (fortress-types t* 100)
  (bospallian t* 75)
  (axehead t* 75)
  (thurastes t* 75)
  (boscorpse t* 100)
  (axecorpse t* 100)
  (thucorpse t* 100)
  (gwyrm t* 75)
  (wyrm t* 75)
  (hatchling t* 37)
  (dajawyrm t* 75)
  (dajaturtle t* 75)
  (cutter t* 50)
  (whaler t* 50)

	(land-types sea-t* 125)
	(water-types land-t* 125)
	(deep-types land-t* 125)

)

(add t* capacity 100)

;;; Unit-material capacities.

(table base-production
	(wcity4 gold 10)
	(city4 gold 10)
	(wcity4 anvil 13)
	(city4 anvil 13)
	(wcity4 study 10)
	(city4 study 10)
	(wcity3 gold 6)
	(city3 gold 6)
	(wcity3 anvil 8)
	(city3 anvil 8)
	(wcity3 study 6)
	(city3 study 6)
	(wcity2 gold 4)
	(city2 gold 4)
	(wcity2 anvil 5)
	(city2 anvil 5)
	(wcity2 study 4)
	(city2 study 4)
	(wcity1 gold 2)
	(city1 gold 2)
	(wcity1 anvil 3)
	(city1 anvil 3)
	(wcity1 study 2)
	(city1 study 2)
	(town gold 1)
	(town anvil 2)
	(svillage gold 2)
	(svillage anvil 3)
	(svillage wood 3)
	(ohamlet gold 1)
	(ohamlet anvil 3)
	(ovillage gold 1)
	(ovillage anvil 5)
        (deadcity anvil 8)
        (deadcity gold 8)
	(citadel anvil 3)
	(keep anvil 3)
	(tower anvil 2)
	(thurastes anvil 10)
	(lair anvil 8) 
	(lair gold 8)
	(dino1 anvil 13)
	(dino1 gold 5)
	(dino2 anvil 13)
	(dino2 gold 5)
	(fheart anvil 9)
	(fheart gold 4)
	(jheart anvil 9)
	(jheart gold 4)
	(sheart anvil 13)
	(sheart gold 5)
	(gheart anvil 13)
	(gheart gold 5)
	(hheart anvil 13)
	(hheart gold 5)
	(dheart anvil 13)
	(dheart gold 5)
	(pit anvil 13)
	(pit gold 13)
	(enode mana 8)
  	(enode study 10)
	(snode mana 8)
  	(snode study 10)
	(lnode mana 8)
  	(lnode study 10)
	(dnode mana 8)
  	(dnode study 10)
	(cnode mana 8)
  	(cnode study 10)
	(temple study 20)
	(boscorpse gold 10)
	(axecorpse gold 15)
	(deadwild anvil 1)
	(deadwild gold 1)
	(thucorpse gold 100)
	(fish gold 5)
	(fish anvil 5)
	(lumber wood 10)
	(goldmine gold 10)
	(ironmine anvil 10)
	(quarry stone 10)
	(hatchling timer 1)
	(wyrm timer 1)
	(bless anvil 3)
	(bless mana 3)
	(mage-types study 5)
)


(table unit-storage-x
  ;; This requires the units to stay close to supply centers.
  	(place-types anvil 200)
	(place-types gold 200)
	(place-types study 200)
  	(ghost timer 3)
	(boscorpse timer 3)
	(axecorpse timer 3)
	(thucorpse timer 3)
	(deadwild timer 3)
	(hatchling timer 20)
	(wyrm timer 20)
	(boscorpse gold 200)
	(axecorpse gold 75)
	(thucorpse gold 700)
	(deadwild gold 4)
	(deadwild anvil 4)
	(fortress-types anvil 200)
	(place-types anvil 200)
	(wilderness-types anvil 200)
	(thurastes anvil 200)
	(fortress-types gold 200)
	(wilderness-types gold 200)
	(node-types study 200)
	(node-types mana 200)
	(shot-types timer 1)
	(explode-types timer 1)
	(bless (anvil mana) 5)
	(lumber wood 3)
	(goldmine gold 3)
	(ironmine anvil 3)
	(quarry stone 3)
	(mage-types study 5)
)



;;; Movement.

(add needle speed 0)
(add place-types speed 0)
(add fortress-types speed 0)
(add wilderness-types speed 0)
(add node-types speed 0)
(add thurastes speed 0)
(add sludge speed 0)
(add shot-types speed 0)
(add enchantment-types speed 0)


(table mp-to-enter-terrain
  (u* t* 2)
  (land-types sea-t* 99)
  (deep-types land-t* 99)
  (water-types land-t* 99)
  (amphibious-types (mountains desert semi-desert) 99)
  (land-types cliffs 99)
  (deep-types cliffs 99)
  (water-types cliffs 99)
  (amphibious-types cliffs 99)
  (land-types river 3)
  (amphibious-types river 1)
  (amphibious-types beach 1)
  (land-types (hills forest semi-desert swamp jungle) 3)
  (land-types (desert wasteland deepforest) 4)
  (land-types mountains 6)
  ((tribesman gladiator) jungle 2)
  (dervish desert 1)
  (cavalry steppe 1)
  (air-types t* 1)
  (deep-types shallows 99)
  (deep-types sea 1)
  (water-types sea-t* 1)
  ((wocke bears wolves ferak royalferak) t* 99)
  ((bears wolves) (deepforest forest) 1)
  (wocke (deepforest) 1)
  ((ferak royalferak) jungle 1)
  (death-types wasteland 1)
  (greyone wasteland 1)
	(tsunami land-t* 99)
	(tsunami sea 1)
	(tsunami shallows 10)
)

(table mp-to-leave-terrain
  (land-types sea-t* 99)
  )


;;; Construction.

(define build1 (wcity4 city4 wcity3 city3))
(define build2 (wcity2 city2))
(define build3 (wcity1 city1))

(define a-types (gladiator longbow crusader assassin cannon weaver))
(define b-types (sword halberd bow cavalry engineer catapult longbow crusader assassin cannon dervish croc))
(define c-types (sword halberd bow cavalry engineer longbow dervish croc))




(table can-create
	(build1 basic-types 3)
	(build1 (flamewall bless) 3)
	(build1 a-types 3)
	(build2 b-types 3)
	(build2 (flamewall bless) 3)
	(build3 c-types 3)
	(build3 (flamewall bless) 3)

  (town (sword bow) 3)
  (svillage (cutter whaler) 3)
  (ohamlet (spear shaman) 3)
  (ovillage (thug champ) 3)
  (spectre ghost 3)
  (engineer (citadel keep tower fort) 3)
  (citadel (riddle flamewall sanctify guardward prismwall halberd cavalry sword bow longbow crusader catapult cannon 
	engineer) 3)
  (keep (riddle flamewall sanctify guardward prismwall cavalry sword bow longbow) 3)
  (fort (riddle flamewall sanctify guardward prismwall scout justiceeye) 3)
  (lair dragon-types 3)
  (pit kraken-types 3)
  (dino1 (plesiosaur tyrannosaur) 3)
  (dino2 (apatosaur pterodactyl) 3)
  (fheart (wolves bears) 3)
  (jheart (ferak royalferak) 3)
  (sheart (squid gibambi) 3)
  (gheart (axehead bospallian) 3)
  (hheart (ulitar sludge churambi) 3)
  (dheart (dajawyrm dajaturtle) 3)
  (enode (earth1 earth2 earth3 crebain) 3)
  (snode (air1 air2 air3 phantombeast phantomwarrior) 3)  
  (cnode (fire1 fire2 fire3 hellhound chaoscreature demon) 3)
  (lnode (spirit1 spirit2 spirit3 guardian) 3)
  ((dnode deadcity) (spectre zombie ghoul skeleton vampyre seeker iblis lich) 3)
  (thurastes (tentacle) 3)

	(place-types enchantment-types 3)

	(self-types shot-types 3)
	(self-types explode-types 3)
	(self-types (deathtap riddle) 6)
	(self-types summon-types 3)
	(self-types create-types 3)

	(mage-types shot-types 3)
	(mage-types explode-types 3)
	(mage-types summon-types 3)

	(castle (phantomwarrior phantombeast) 3)

)

(table acp-to-create
	(build1 basic-types 3)
	(build1 (flamewall bless) 3)
	(build1 a-types 3)
	(build2 b-types 3)
	(build2 (flamewall bless) 3)
	(build3 c-types 3)
	(build3 (flamewall bless) 3)

  (town (sword bow) 3)
  (svillage (cutter whaler) 3)
  (ohamlet (spear shaman) 3)
  (ovillage (thug champ) 3)
  (spectre ghost 3)
  (engineer (citadel keep tower fort) 3)
  (citadel (riddle flamewall sanctify guardward prismwall halberd cavalry sword bow longbow crusader catapult cannon 
	engineer) 3)
  (keep (riddle flamewall sanctify guardward prismwall cavalry sword bow longbow) 3)
  (fort (riddle flamewall sanctify guardward prismwall scout justiceeye) 3)
  (lair dragon-types 3)
  (pit kraken-types 3)
  (dino1 (plesiosaur tyrannosaur) 3)
  (dino2 (apatosaur pterodactyl) 3)
  (fheart (wolves bears) 3)
  (jheart (ferak royalferak) 3)
  (sheart (squid gibambi) 3)
  (gheart (axehead bospallian) 3)
  (hheart (ulitar sludge churambi) 3)
  (dheart (dajawyrm dajaturtle) 3)
  (enode (earth1 earth2 earth3 crebain) 3)
  (snode (air1 air2 air3 phantombeast phantomwarrior) 3)  
  (cnode (fire1 fire2 fire3 hellhound chaoscreature demon) 3)
  (lnode (spirit1 spirit2 spirit3 guardian) 3)
  ((dnode deadcity) (spectre zombie ghoul skeleton vampyre seeker iblis lich) 3)
  (thurastes (tentacle) 3)

	(place-types enchantment-types 3)

	(self-types shot-types 3)
	(self-types explode-types 3)
	(self-types (deathtap riddle) 6)
	(self-types summon-types 3)
	(self-types create-types 3)

	(mage-types shot-types 3)
	(mage-types explode-types 3)
	(mage-types summon-types 3)

	(castle (phantomwarrior phantombeast) 3)

)

(table cp-on-creation
	(build1 basic-types 3)
	(build1 (flamewall bless) 3)
	(build1 a-types 3)
	(build2 b-types 3)
	(build2 (flamewall bless) 3)
	(build3 c-types 3)
	(build3 (flamewall bless) 3)

  (town (sword bow) 1)
  (svillage (cutter whaler) 3)
  (ohamlet (spear shaman) 3)
  (ovillage (thug champ) 3)
  ((deadcity ruins) death-types 3)
  (spectre ghost 3)
  (engineer (citadel keep tower fort) 3)
  (citadel (riddle flamewall sanctify guardward prismwall halberd cavalry sword bow longbow crusader catapult cannon 
	engineer) 3)
  (keep (riddle flamewall sanctify guardward prismwall cavalry sword bow longbow) 3)
  (fort (riddle flamewall sanctify guardward prismwall scout justiceeye) 3)
  (lair dragon-types 3)
  (pit kraken-types 3)
  (dino1 (plesiosaur tyrannosaur) 3)
  (dino2 (apatosaur pterodactyl) 3)
  (fheart (wolves bears) 3)
  (jheart (ferak royalferak) 3)
  (sheart (squid gibambi) 3)
  (gheart (axehead bospallian) 3)
  (hheart (ulitar sludge churambi) 3)
  (dheart (dajawyrm dajaturtle) 3)
  (enode (earth1 earth2 earth3 crebain) 1)
  (snode (air1 air2 air3 phantombeast phantomwarrior) 1)  
  (cnode (fire1 fire2 fire3 hellhound chaoscreature demon) 1)
  (lnode (spirit1 spirit2 spirit3 guardian) 1)
  (deadcity (spectre zombie ghoul skeleton vampyre seeker iblis lich) 3)
  (dnode (spectre zombie ghoul skeleton vampyre seeker iblis lich) 1)
  (thurastes (tentacle) 3)

	(place-types enchantment-types 3)

	(self-types shot-types 3)
	(self-types explode-types 3)
	(self-types (deathtap riddle) 6)
	(self-types summon-types 3)
	(self-types create-types 3)

	(mage-types shot-types 3)
	(mage-types explode-types 3)
	(mage-types summon-types 3)

	(castle (phantomwarrior phantombeast) 3)
)

(table can-build
	(build1 basic-types 3)
	(build1 (flamewall bless) 3)
	(build1 a-types 3)
	(build2 b-types 3)
	(build2 (flamewall bless) 3)
	(build3 c-types 3)
	(build3 (flamewall bless) 3)

  (town (sword bow) 3)
  (svillage (cutter whaler) 3)
  (ohamlet (spear shaman) 3)
  (ovillage (thug champ) 3)
  ((deadcity ruins) death-types 3)
  (spectre ghost 3)
  (engineer (citadel keep tower fort) 3)
  (citadel (riddle flamewall sanctify guardward prismwall halberd cavalry sword bow longbow crusader catapult cannon 
	engineer) 3)
  (keep (riddle flamewall sanctify guardward prismwall cavalry sword bow longbow) 3)
  (fort (riddle flamewall sanctify guardward prismwall scout justiceeye) 3)
  (lair dragon-types 3)
  (pit kraken-types 3)
  (dino1 (plesiosaur tyrannosaur) 3)
  (dino2 (apatosaur pterodactyl) 3)
  (fheart (wolves bears) 3)
  (jheart (ferak royalferak) 3)
  (sheart (squid gibambi) 3)
  (gheart (axehead bospallian) 3)
  (hheart (ulitar sludge churambi) 3)
  (dheart (dajawyrm dajaturtle) 3)
  (enode (earth1 earth2 earth3 crebain) 3)
  (snode (air1 air2 air3 phantombeast phantomwarrior) 3)  
  (cnode (fire1 fire2 fire3 hellhound chaoscreature demon) 3)
  (lnode (spirit1 spirit2 spirit3 guardian) 3)
  ((dnode deadcity) (spectre zombie ghoul skeleton vampyre seeker iblis lich) 3)
  (thurastes (tentacle) 3)

	(place-types enchantment-types 3)

	(self-types shot-types 3)
	(self-types explode-types 3)
	(self-types (deathtap riddle) 6)
	(self-types summon-types 3)
	(self-types create-types 3)

	(mage-types shot-types 3)
	(mage-types explode-types 3)
	(mage-types summon-types 3)

	(castle (phantomwarrior phantombeast) 3)
)

(table acp-to-build
	(build1 basic-types 3)
	(build1 (flamewall bless) 3)
	(build1 a-types 3)
	(build2 b-types 3)
	(build2 (flamewall bless) 3)
	(build3 c-types 3)
	(build3 (flamewall bless) 3)

  (town (sword bow) 3)
  (svillage (cutter whaler) 3)
  (ohamlet (spear shaman) 3)
  (ovillage (thug champ) 3)
  ((deadcity ruins) death-types 3)
  (spectre ghost 3)
  (engineer (citadel keep tower fort) 3)
  (citadel (riddle flamewall sanctify guardward prismwall halberd cavalry sword bow longbow crusader catapult cannon 
	engineer) 3)
  (keep (riddle flamewall sanctify guardward prismwall cavalry sword bow longbow) 3)
  (fort (riddle flamewall sanctify guardward prismwall scout justiceeye) 3)
  (lair dragon-types 3)
  (pit kraken-types 3)
  (dino1 (plesiosaur tyrannosaur) 3)
  (dino2 (apatosaur pterodactyl) 3)
  (fheart (wolves bears) 3)
  (jheart (ferak royalferak) 3)
  (sheart (squid gibambi) 3)
  (gheart (axehead bospallian) 3)
  (hheart (ulitar sludge churambi) 3)
  (dheart (dajawyrm dajaturtle) 3)
  (enode (earth1 earth2 earth3 crebain) 3)
  (snode (air1 air2 air3 phantombeast phantomwarrior) 3)  
  (cnode (fire1 fire2 fire3 hellhound chaoscreature demon) 3)
  (lnode (spirit1 spirit2 spirit3 guardian) 3)
  ((dnode deadcity) (spectre zombie ghoul skeleton vampyre seeker iblis lich) 3)
  (thurastes (tentacle) 3)

	(place-types enchantment-types 3)

	(self-types shot-types 3)
	(self-types explode-types 3)
	(self-types (deathtap riddle) 6)
	(self-types summon-types 3)
	(self-types create-types 3)

	(mage-types shot-types 3)
	(mage-types explode-types 3)
	(mage-types summon-types 3)

	(castle (phantomwarrior phantombeast) 3)
)

(table cp-per-build
	(build1 basic-types 6)
	(build1 (flamewall bless) 3)
	(build1 a-types 6)
	(build2 b-types 3)
	(build2 (flamewall bless) 3)
	(build3 c-types 3)
	(build3 (flamewall bless) 3)

  (town (sword bow) 1)
  (svillage (cutter whaler) 3)
  (ohamlet (spear shaman) 3)
  (ovillage (thug champ) 3)
  ((deadcity ruins) death-types 3)
  (spectre ghost 3)
  (engineer (citadel keep tower fort) 3)
  (citadel (riddle flamewall sanctify guardward prismwall halberd cavalry sword bow longbow crusader catapult cannon 
	engineer) 3)
  (keep (riddle flamewall sanctify guardward prismwall cavalry sword bow longbow) 3)
  (fort (riddle flamewall sanctify guardward prismwall scout justiceeye) 3)
  (lair dragon-types 3)
  (pit kraken-types 3)
  (dino1 (plesiosaur tyrannosaur) 3)
  (dino2 (apatosaur pterodactyl) 3)
  (fheart (wolves bears) 3)
  (jheart (ferak royalferak) 3)
  (sheart (squid gibambi) 3)
  (gheart (axehead bospallian) 3)
  (hheart (ulitar sludge churambi) 3)
  (dheart (dajawyrm dajaturtle) 3)
  (enode (earth1 earth2 earth3 crebain) 1)
  (snode (air1 air2 air3 phantombeast phantomwarrior) 1)  
  (cnode (fire1 fire2 fire3 hellhound chaoscreature demon) 1)
  (lnode (spirit1 spirit2 spirit3 guardian) 1)
  (dnode (spectre zombie ghoul skeleton vampyre seeker iblis lich) 1)
  (deadcity (spectre zombie ghoul skeleton vampyre seeker iblis lich) 3)
  (thurastes (tentacle) 3)

	(place-types enchantment-types 3)

	(self-types shot-types 3)
	(self-types explode-types 3)
	(self-types (deathtap riddle) 6)
	(self-types summon-types 3)
	(self-types create-types 3)

	(mage-types shot-types 3)
	(mage-types explode-types 3)
	(mage-types summon-types 3)

	(castle (phantomwarrior phantombeast) 3)
)



(table create-range
	(svillage (cutter whaler) 1)

	(self-types spell-types 2)
	(self-types summon-types 0)

	(mage-types spell-types 1)

	(lair gwyrm 1)

)


;;; **************************DO THIS NEXT************************

(table gives-to-treasury
	(node-types m* true)
	(wilderness-types gold true)
	(place-types gold true)
	(boscorpse gold true)
	(axecorpse gold true)
	(thucorpse gold true)
	(deadwild gold true)
	(deadwild anvil true)
	(node-types m* true)
	(wilderness-types anvil true)
	(place-types anvil true)
	(fortress-types anvil true)
	(thurastes anvil true)
	(resource-types m* true)
)

(table takes-from-treasury 
	(node-types gold true)
	(wilderness-types gold true)
	(place-types gold true)
	(fortress-types gold true)
	(node-types anvil true)
	(wilderness-types anvil true)
	(place-types anvil true)
	(fortress-types anvil true)
	(engineer gold true)
	(engineer anvil true)
	(spectre gold true)
	(thurastes anvil true)
	(self-types m* true)
)


(table consumption-on-creation
  (sword anvil 20)
  (bow anvil 10)
  (bow wood 10)
  (halberd anvil 30)
  (cavalry anvil 25)
  (cavalry gold 15)
  (engineer anvil 25)
  (engineer gold 10)
  (catapult anvil 25)
  (catapult wood 25)
  (catapult gold 25)
  (mage gold 50)

  (scout anvil 5)
  (scout wood 5)
  (scout gold 5)
  (longbow anvil 15)
  (longbow wood 15)
  (gladiator gold 30)
  (crusader anvil 20)
  (crusader gold 20)
  (assassin gold 50)
  (cannon anvil 75)
  (cannon gold 50)
  (dervish gold 30)
  (dervish anvil 0)
  (weaver gold 30)
  (croc gold 30)

  (hatchling anvil 250)
  (wyrm gold 500)
  (gwyrm gold 1000)

  (bospallian anvil 200)
  (axehead gold 400)
  (tentacle anvil 200)
  (dajawyrm gold 300)
  (dajaturtle anvil 300)

  (plesiosaur anvil 150)
  (tyrannosaur gold 150)
  (apatosaur anvil 175)
  (pterodactyl gold 50)

  (spear anvil 15)
  (shaman anvil 25)
  (thug anvil 30)
  (champ anvil 40)
  (cutter anvil 20)
	(cutter wood 30)
	(whaler anvil 20)
	(whaler gold 20)
	(whaler wood 45)
  (wolves anvil 120)
  (bears gold 100)
  (ferak anvil 100)
  (royalferak gold 100)

  (squid anvil 150)
  (gibambi gold 80)
  (ulitar anvil 200)
  (sludge gold 30)
  (churambi gold 85)

  (fort wood 75)
  (tower stone 50)
  (keep stone 100)
  (citadel stone 200)

  (zombie anvil 30)
  (spectre gold 100)
  (ghost gold 20)

  (slaver gold 200)
  (sorcerer anvil 200)

	(powerword mana 50)
	(iblis mana 100)
	(lich mana 75)
	(deathwalk mana 40)
	(wastemaker mana 50)
	(riddle mana 30)
	(breath mana 40)
	(seance mana 25)
	(poltergeist mana 20)
	(bonearrow mana 10)
	(seeker mana 15)
	(deathtap mana 1)
	(naturevoice mana 25)
	(leviathan mana 100)
	(tsunami mana 40)
	(mountainchange mana 20)
	(beguile mana 30)
	(earthquake mana 30)
	(deepchange mana 20)
	(shallowchange mana 20)
	(charmfish mana 30)
	(hillchange mana 20)
	(plainchange mana 20)
	(swampchange mana 20)
	(charmbeast mana 15)
	(crebain mana 15)
	(canticle mana 100)
	(demon mana 75)
	(chaoscreature mana 100)
	(volcano mana 60)
	(meteor mana 50)
	(disintegrate mana 35)
	(flamewall mana 20)
	(hellhound mana 20)
	(fireball mana 20)
	(flamestrike mana 10)
	(flamearrow mana 10)
	(restore mana 35)
	(enlighten mana 50)
	(doorway mana 50)
	(spiritwall mana 25)
	(sunray mana 25)
	(guardian mana 40)
	(improveheal mana 25)
	(sanctify mana 20)
	(bless mana 20)
	(justiceeye mana 5)
	(bolt mana 5)
	(heal mana 10)
	(castle mana 75)
	(greatcharm mana 50)
	(truedivine mana 30)
	(guardward mana 20)
	(phantombeast mana 50)
	(prismwall mana 20)
	(greatdivine mana 25)
	(lightning mana 15)
	(charm mana 20)
	(phantomwarrior mana 10)
	(divine mana 15) 
	(skeleton mana 20)
	(ghoul mana 35)
 	(vampyre mana 65)

	(earth1 mana 20)
	(air1 mana 20)  
	(fire1 mana 20)
	(spirit1 mana 20)
	(earth2 mana 35)
	(air2 mana 35)  
	(fire2 mana 35)
	(spirit2 mana 35)
	(earth3 mana 65)
	(air3 mana 65)  
	(fire3 mana 65)
	(spirit3 mana 65)

)

(table base-consumption
  (sword anvil 1)
  (bow anvil 1)
  (halberd anvil 2)
  (cavalry anvil 1)
  (cavalry gold 1)
  (engineer anvil 1)
  (engineer gold 1)
  (catapult anvil 2)
  (catapult gold 1)
  (mage gold 2)

  (scout anvil 1)
  (longbow anvil 1)
  (gladiator gold 2)
  (crusader anvil 1)
  (crusader gold 1)
  (assassin gold 2)
  (cannon anvil 2)
  (cannon gold 2)
  (dervish gold 1)
  (weaver gold 2)
  (croc gold 2)

  (gwyrm gold 10)
  (wyrm gold 4)
  (hatchling gold 2)

  (bospallian anvil 2)
  (axehead gold 3)
  (tentacle anvil 3)
  (dajawyrm gold 3)
  (dajaturtle anvil 3)

  (plesiosaur anvil 3)
  (tyrannosaur gold 3)
  (apatosaur anvil 3)
  (pterodactyl gold 1)

  (earth1 anvil 1)
  (air1 anvil 1)  
  (fire1 anvil 1)
  (spirit1 anvil 1)

  (spear anvil 1)
  (shaman anvil 2)
  (thug anvil 1)
  (champ anvil 2)
  (cutter anvil 1)
  (whaler gold 2)

  (wolves anvil 1)
  (bears gold 2)
  (ferak anvil 2)
  (royalferak gold 2)

  (squid anvil 3)
  (gibambi gold 2)
  (ulitar anvil 3)
  (sludge gold 1)
  (churambi gold 2)

  (skeleton anvil 1)
  (zombie anvil 1)
  (ghoul anvil 2)
  (spectre gold 3)
  (vampyre gold 3)
  (ghost gold 1)

  (slaver gold 3)
  (sorcerer anvil 3)

  	(ghost timer 1)
 	(boscorpse timer 1)
	(axecorpse timer 1)
	(thucorpse timer 1)
	(deadwild timer 1)
	(shot-types timer 1)
	(explode-types timer 1)
	(spell-types mana 2)
	(enchantment-types mana 1)
)


(table supply-on-creation
  (u* m* 0)
  (ghost timer 3)
  (boscorpse timer 3)
  (thucorpse timer 3)
  (axecorpse timer 3)
  (deadwild timer 3)
  (boscorpse gold 100)
  (thucorpse gold 500)
  (axecorpse gold 125)
  (deadwild gold 4)
  (deadwild anvil 4)
	(shot-types timer 20)
	(explode-types timer 20)
	(bless m* 5)
)



;;; Repair.

(add u* hp-recovery 100)
(add self-types hp-recovery 1000)
(add vampyre hp-recovery 32000)
(add dragon-types hp-recovery 500)

(table can-repair
	((heal improveheal) u* true)
    (svillage (whaler cutter) true)
    (place-types (catapult cannon) true)
    (fortress-types (catapult cannon) true)
    (fortress-types fortress-types true)
)

(table acp-to-repair
	((heal improveheal) u* 1)
    (svillage (whaler cutter) 1)
    (place-types (catapult cannon) 1)
    (fortress-types (catapult cannon) 1)
    (fortress-types fortress-types 1)
)

(table hp-per-repair
    (svillage (whaler cutter) 100)
    (place-types (catapult cannon) 100)
    (fortress-types (catapult cannon) 100)
    (fortress-types fortress-types 1000)
)

(table material-to-repair
;;	(heal timer 20)
;;	(improveheal timer 20)
)


(table auto-repair
  (place-types basic-types 1.00)
  (place-types specific-types 1.00)
  )

(table auto-repair-range
  (place-types specific-types 1)
  )





(table hp-per-starve
  	(ghost timer 100.00)
	(boscorpse timer 100.00)
	(axecorpse timer 100.00)
	(thucorpse timer 100.00)
	(deadwild timer 100.00)
	(shot-types timer 100.00)
	(explode-types timer 100.00)
)

;;; Combat.

(define weak-types (spear sword halberd zombie cavalry gibambi bospallian apatosaur crebain wolves skeleton hellhound earth1 air1 spirit1 fire1 scout ferak croc engineer guardian tribesman)
)

(define strong-types (crusader gladiator slaver thug pterodactyl plesiosaur bears royalferak churambi earth2 air2 spirit2 fire2 ghoul hatchling phantomwarrior squid ulitar ghost thuvi)
)

(define powerful-types (champ dajaturtle axehead demon vampyre wyrm sludge phantombeast earth3 air3 spirit3 fire3 tentacle)
)

(define incredible-types (tyrannosaur chaoscreature iblis gwyrm dajawyrm mold thurastes)
)

(define corpse-types (thucorpse boscorpse deadwild axecorpse)
)

(define ranged-types (bow longbow catapult cannon cutter whaler)
)

(define nosave-types (powerword sunray)
)

(define magicimmune-types (iblis demon thuvi)
)

(define notcapture-types  (catapult cannon assassin tyrannosaur apatosaur pterodactyl plesiosaur wolves bears
   	ferak royalferak axehead bospallian mold dajawyrm dajaturtle squid thurastes ulitar hatchling))

(table acp-to-attack
  (u* u* 1)
  (squid u* 1)
  (bospallian u* 2)
  (axehead u* 2)
  (all-types air-types 99)
  (air-types air-types 1)
  (u* wilderness-types 99)
  (notcapture-types node-types 99)
  (notcapture-types ruin-types 99)

	(place-types u* 99)
	(node-types u* 99)
	(wilderness-types u* 99)
	(needle u* 99)


;; Some enchantments cause damage, but none of them do so on their own
	(enchantment-types u* 99)

)


(table fire-hit-chance
	(u* u* 0)
;; Saving Throws
	(shot-types wild-types 90)
	(shot-types basic-types 85)
	(shot-types specific-types 75)
	(shot-types water-types 80)
	(shot-types amphibious-types 75)
	(shot-types deep-types 70)
	(shot-types kraken-types 60)
	(shot-types summon-types 50)
	(nosave-types all-types 100)
	(shot-types dragon-types 35)
	(shot-types magicimmune-types 0)
	(nosave-types magicimmune-types 20)

	((bow cutter) all-types 50)
	((bow cutter) (bow whaler cutter) 75)
	((bow cutter) longbow 25)
	((bow cutter) air-types 75)
	((bow cutter) basic-types 75)
	((bow cutter) specific-types 65)

	((longbow whaler) all-types 60)
	((longbow whaler) longbow 75)
	((longbow whaler) bow 90)
	((longbow whaler) air-types 85)
	((longbow whaler) basic-types 85)
	((longbow whaler) specific-types 75)

	(mage-types all-types 50)
	(mage-types mage-types 75)
	(mage-types (dervish weaver) 85)
	(mage-types air-types 75)
	(mage-types basic-types 85)
	(mage-types specific-types 85)
	(mage-types spell-types 75)

	((shaman dervish weaver) all-types 50)
	((shaman dervish weaver) mage-types 60)
	((shaman dervish weaver) (dervish weaver) 75)
	((shaman dervish weaver) air-types 70)
	((shaman dervish weaver) basic-types 80)
	((shaman dervish weaver) specific-types 80)
	((shaman dervish weaver) spell-types 70)

	(self-types all-types 90)

	(all-types self-types 25)

	(catapult all-types 33)
	(cannon all-types 40)

	(ranged-types place-types 100)
	(mage-types place-types 100)
	(ranged-types fortress-types 100)
	(mage-types fortress-types 100)


	(loremaster u* 80)
	(spawn u* 80)
	(u* wilderness-types 0)
	(u* node-types 0)


)


(table fire-damage
	(bow u* 2d6)
	(longbow u* 2d8)
	(mage-types u* 3d10)
	((dervish weaver shaman) u* 3d8)
	(catapult u* 5d8)
	(cannon u* 7d8+8)
	(cutter u* 3d10)
	(whaler u* 5d12)

	((cutter whaler) place-types 1d4)

	(shot-types self-types 1d10)
	(mage-types spell-types 2d6)

	(loremaster u* 2d10)
	(spawn u* 2d10)
	(u* mold 1d2)
	(powerword u* 200)
	(powerword self-types 2d10)
	(bonearrow u* 1d6)
	(earthquake place-types 100)
	(disintegrate all-types 100)
	(disintegrate place-types 100)
	(disintegrate fortress-types 100)
	(disintegrate self-types 0)
	(flamestrike u* 3d10+10)
	(flamearrow u* 2d8)
	(sunray u* 3d10)
	(sunray death-types 50)
	(bolt u* 1d10)
	(bolt death-types 4d10)
	(lightning u* 5d10)
)


(table retreat-chance
  (u* spear 50)
  (u* ferak 50)
  (u* royalferak 25)
  )

(table acp-for-retreat
  (spear u* 1)
  (ferak u* 1)
  (royalferak u* 1)
  )

(table control-range
  (u* t* 0)
  )

(table zoc-range
;;  (infantry-types infantry-types 1)
;;  (place-types infantry-types 1)
  )

;; Can only enter/leave ZOC, not move around in.

(table mp-to-traverse-zoc
;;  (infantry-types infantry-types -1)
;;  (infantry-types place-types -1)
  )

(table mp-to-enter-zoc
;;  (infantry-types infantry-types 0)
;;  (infantry-types place-types 0)
  )

(set combat-model 0)


(table hit-chance
	(u* u* 0)
	(weak-types all-types 50)
	(weak-types weak-types 75)
	(weak-types strong-types 60)
	(weak-types powerful-types 40)
	(weak-types incredible-types 30)
	(weak-types ranged-types 75)

	(strong-types all-types 60)
	(strong-types weak-types 85)
	(strong-types strong-types 75)
	(strong-types powerful-types 60)
	(strong-types incredible-types 40)
	(strong-types ranged-types 85)

	(powerful-types all-types 70)
	(powerful-types weak-types 95)
	(powerful-types strong-types 85)
	(powerful-types powerful-types 75)
	(powerful-types incredible-types 50)
	(powerful-types ranged-types 95)

	(incredible-types all-types 80)
	(incredible-types weak-types 95)
	(incredible-types strong-types 95)
	(incredible-types powerful-types 80)
	(incredible-types incredible-types 75)
	(incredible-types ranged-types 95)

	(air-types air-types 75)
	(self-types all-types 75)

	(flamewall all-types 75)
	(prismwall all-types 50)

	(assassin basic-types 100)
	(assassin specific-types 100)
	(assassin king 65)
	(croc (plesiosaur tyrannosaur apatosaur pterodactyl hatchling wyrm) 100)

	(u* wilderness-types 0)
	(notcapture-types node-types 0)
	(notcapture-types ruin-types 0)
	(all-types snode 100)
	(all-types resource-types 100)

	(tsunami place-types 100)

)



(table damage
	(u* u* 0)
	((scout spear croc engineer) u* 1d6+2)
	(sword u* 1d10+2) 
	(halberd u* 1d12+4)
	((zombie tribesman) u* 1d10+3)
	(cavalry u* 2d7)
	((gibambi hellhound) u* 2d5)
	(bospallian u* 5d10)
	(apatosaur u* 5d10)
	((guardian crebain) u* 1d4)
	(wolves u* 1d8+1)
	(skeleton u* 1d8+2)
	(earth1 u* 2d8)
	(air1 u* 1d14)
	(spirit1 u* 2d7)
	(fire1 u* 2d9)
	(ferak  u* 1d6)

	(scout wild-types 2d10+10)

	(crusader u* 1d14+6)
	((gladiator thuvi) u* 2d8+2)
	((thug slaver) u* 2d10+5)
	(pterodactyl u* 2d14)
	((plesiosaur bears) u* 3d8)
	(royalferak u* 2d13)
	(churambi u* 1d11+4)
	(earth2 u* 3d8)
	(air2 u* 2d14)
	(spirit2 u* 3d7)
	(fire2 u* 3d9)
	(ghoul u* 2d7+5)
	(hatchling u* 4d10)
	(phantomwarrior u* 3d9)
	(squid u* 1d8)
	(ulitar u* 3d4+5)
	(ghost u* 1d15)

	(champ u* 2d10+8)
	(dajaturtle u* 3d10)
	(axehead u* 5d12+30) 
	(demon u* 6d4)
	(vampyre u* 1d14)
	(wyrm u* 6d12)
	(sludge u* 1)
	(phantombeast u* 5d9)
	(earth3 u* 4d8)
	(air3 u* 3d14)
	(spirit3 u* 4d7)
	(fire3 u* 4d9)
	(tentacle u* 4d8+6)

	(tyrannosaur u* 6d13)
	(chaoscreature u* 4d10)
	(iblis u* 4d10)
	(gwyrm u* 7d14+20)
	(dajawyrm u* 3d12)
	(mold u* 5d8+10)
	(thurastes u* 6d13+50)

	(wild-types scout 1d5)
	((plesiosaur tyrannosaur apatosaur pterodactyl hatchling wyrm) croc 1d5)
	(assassin basic-types 100)
	(assassin specific-types 100)

	(flamewall all-types 3d10)
	(prismwall all-types 2d13)

	(all-types self-types 1d10)
	(self-types all-types 2d10)
	(all-types mold 1d2)
			
	(assassin basic-types 2d10+30)
	(assassin specific-types 2d10+30)

;; Engineers make good sappers
	(engineer fortress-types 3d10)

	(tsunami place-types 100)
)


;;; Transports will get you to the edge of the cell, but the
;;; passengers have to cross any borders on their own.




(table ferry-on-entry
	(u* u* 0)
	(cutter basic-types 3)
	(cutter specific-types 3)
	(cutter self-types 3)
	(cutter (shaman spear champ thug) 3)
	(cutter death-types 3)
	(ulitar sludge 3)
)

(table ferry-on-departure
	(u* u* 0)
	(cutter basic-types 3)
	(cutter specific-types 3)
	(cutter self-types 3)
	(cutter (shaman spear champ thug) 3)
	(cutter death-types 3)
	(ulitar sludge 3)
)


(table hp-min
	(u* ruin-types 1)
	(u* (dino1 dino2 fheart jheart sheart gheart hheart dheart) 1)
	(u* node-types 1)
	(u* resource-types 1)
)



(table capture-chance
	(basic-types (cutter whaler) 100)
	(specific-types (cutter whaler) 100)
	(croc (plesiosaur tyrannosaur apatosaur pterodactyl) 50)
	(croc hatchling 40)
	(croc wyrm 20)
	(slaver basic-types 75)
	(spawn basic-types 100)
	(vampyre basic-types 75)
	(sludge basic-types 75)
	((hatchling wyrm gwyrm scout whaler cutter bow longbow) corpse-types 100)

	(all-types ruin-types 100)
	(all-types place-types 90)
	(all-types node-types 100)
	(notcapture-types u* 0)
	((cutter whaler) place-types 0)
	(wild-types place-types 0)
	(u* deadcity 0)
	(u* snode 100)
	(u* resource-types 100)
	(u* self-types 0)
	(assassin king 100)
	(crusader usurper 100)
	(charm basic-types 100)	
	(charmfish (bospallian dajawyrm axehead squid) 33)
	(charmbeast (wolves bears ferak royalferak) 45)
	(enlighten all-types 100)
	(enlighten self-types 0)
	(prismwall all-types 10)
	(prismwall self-types 0)
	(greatcharm basic-types 100)
	(u* fortress-types 0)

)

(table protection
  ;; Ground units protect cities.
  (all-types all-types 50)
;;  ((balloon) place-types 80)
  (all-types place-types 0)
  (all-types node-types 0)
  (all-types fortress-types 100)
	(place-types enchantment-types 100)
	(fortress-types enchantment-types 100)


  )

(table transport-affects-defense
;;	(tower units 125)		;	Cities increase occs def value by 50%
)

(table defend-terrain-effect
  (all-types hills 75)
  (all-types forest 60)
  (all-types swamp 50)
  (all-types mountains 35)
)




(table occupant-combat
	(u* u* 0)
	(u* fortress-types 0)
	(ranged-types fortress-types 1)
	(cutter (longbow bow sword thug champ) 1)
	(all-types place-types 1)

)



(table detonation-unit-range
	(fireball u* 1)
	(meteor u* 2)
	(wastemaker u* 1)
)

(table detonation-damage-at
	(fireball u* 6d6)
	(meteor u* 7d9)
	(u* self-types 1d10)
	(wastemaker u* 1d8)
	(earthquake place-types 50)
	(earthquake fortress-types 200)
)

(table detonation-damage-adjacent
	(fireball u* 6d6)
	(meteor u* 7d9)
	(u* self-types 1d10)
	(wastemaker u* 1d6)
	(earthquake place-types 25)
	(earthquake fortress-types 100)
)

(table detonation-terrain-range
	(wastemaker (land steppe forest hills desert semi-desert) 1)
)


(table detonation-terrain-damage-chance
	(wastemaker (land steppe forest hills desert semi-desert) 50)
)

(table terrain-damaged-type
	((land steppe forest hills desert semi-desert) wasteland 50)
)

(table acp-to-remove-terrain 

	(restore wasteland 15)
	(mountainchange (desert semi-desert hills steppe land forest) 15)
	(deepchange shallows 15)
	(shallowchange (sea swamp) 15)
	(hillchange  (desert semi-desert steppe land forest) 15)
	(plainchange (hills desert semi-desert steppe land forest swamp) 15)
	(swampchange (semi-desert steppe land shallows) 15)
)


(table acp-to-add-terrain

	(restore land 15)
	(mountainchange mountains 15)
	(deepchange sea 15)
	(shallowchange shallows 15)
	(hillchange hills 15)
	(plainchange land 15)
	(swampchange swamp 15)

)

(table unit-consumption-per-size
)

(table unit-consumption-to-grow
	(u* gold 9999)
)

(table acp-to-change-type
	(hatchling wyrm 1)
	(wyrm gwyrm 1)
	(crusader cavalry 1)
)

(table material-to-change-type
	(hatchling timer 20)
	(wyrm timer 20)
)

(table out-length
  ;; Net consumers of supply should never give any up automatically.
	(bless m* 1)
	(mage-types study 2)
)

(table in-length
  ;; Supply to ground units can go several hexes away.
	(place-types m* 1)
	(place-types study 2)
)



;;; Temperature characteristics of terrain.

(add t* temperature-min -20)
(add (sea shallows) temperature-min 4)
(add desert temperature-min 0)
(add semi-desert temperature-min 0)

(add t* temperature-max 30)
(add desert temperature-max 40)
(add semi-desert temperature-max 35)
(add mountains temperature-max 20)
(add ice temperature-max 0)

(add t* temperature-average 20)
(add mountains temperature-average 10)
(add ice temperature-average -10)

(add land temperature-variability 5)

(table coating-depth-max
  ((mud snow) t* 1)
  ((mud snow) (sea shallows neutral) 0)
  ;; Mountains are rocky, don't really have a "mud season".
  (mud mountains 0)
  )

;; If a side loses, its combat units vanish, but not its cities.

(add all-types lost-vanish-chance 0)

;;; The world.

(world 360 (year-length 12) (axial-tilt 22))


(set calendar '(usual month))

(set season-names
  ;; 0 is January, 3 is April, 6 is July, 9 is October
  ((0 2 "winter") (3 5 "spring") (6 8 "summer") (9 11 "autumn")))

(set initial-date "Jan 973")

;;; River generation.

(add (land forest mountains) river-chance (10.00 25.00 25.00))

(set river-sink-terrain shallows)

;;; Road generation.

(table road-into-chance
  (t* land 100)
  (land (desert forest mountains) (50 40 20))
  )

(set edge-terrain ice)


(table favored-terrain
  (u* t* 0)
  )

(add land country-people-chance 100)

(table unit-initial-supply
	(u* m* 0)
  	(ghost timer 3)
	(boscorpse timer 3)
	(axecorpse timer 3)
	(thucorpse timer 3)
	(deadwild timer 3)
	(boscorpse gold 100)
	(axecorpse gold 100)
	(thucorpse gold 500)
	(deadwild gold 4)
	(deadwild anvil 4)
	(shot-types timer 20)
	(explode-types timer 20)
)


(scorekeeper (do last-side-wins))


(game-module (design-notes
  "It is ambitious"
  ))
