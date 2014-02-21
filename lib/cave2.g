(game-module "cave2"
  (title "Cave of Amazement")
  (blurb "Explore and conquer a cave full of treasures and monsters. Modified cave game by Lincoln Peters.")
  (instructions "The cave contains magical treasures and monsters.
Different treasures give you different advantages in combat.
You can capture the homes of monsters to make them do your bidding.
Be prepared to fight over control of the cave!")
  (variants
   (world-seen false)        ;; Don't know what this would be useful for
   (see-all false)          ;; Useful for testing, but not much else.
   (sequential false)
   (world-size)
   ("Include Rivers" include-rivers
     "Generate a cave with underground streams."
     (true
       (add passage maze-passage-occurrence 99)
       (add river maze-passage-occurrence 1)
         ;; Players really shouldn't try this; sometimes maze rooms end up 
         ;; being cut off from other parts of the cave.  I don't have any 
         ;; sort of useful thing for rivers anyway (yet).
     ))
   )
  )

;; Many of the images have been replaced with a more consistent set.
;; Still needs better insect images, though.

(unit-type human (image-name "adventurer") (char "@")
  (hp-max 10) (acp-per-turn 4) (cp 10)
  (help "Moves around, fights enemy monsters, captures places")
  (notes "In the beginning, this is your only unit.  He can capture orc holes, nests, and pentagrams, and he can search for treasures.  He can also fight.  Although in the begining he's not very good at fighting, he learns fast; slay enough orcs and you'll be ready to take on the big dragons!"))
(unit-type corpse (image-name "animal-skeleton") (char "x")
  (hp-max 6) (acp-per-turn 0)
  (help "A dead thing")
  (notes "A corpse is only useful because it might possess valuable objects. Otherwise, it's worthless."))
  ;; Note that without corpses, items that are being carried by a slain 
  ;; unit would vanish.  Instead, they can be picked up.

(unit-type orc (image-name "monsters-orc3") (char "o")
  (hp-max 10) (acp-per-turn 4) (cp 4)
  (help "Easy to produce, but no special features")
  (notes "Orcs have no special traits; they're as fast as humans, as strong as humans, etc.  However, they can be produced very quickly.  If a group of orcs corners an enemy, the enemy has little chance of escape (unless the enemy is a pit demon or dragon)."))
(unit-type elf (image-name "heroes-elf1") (char "e")
  (hp-max 10) (acp-per-turn 6) (cp 12)
  (help "Faster than orcs, can outrun anything except a dragon")
(notes "Elves and spiders are the fastest walking creatures.  Other than that, they're no stronger or fiercer than orcs, but speed does sometimes offer a strong advantage.  Just keep in mind that they take longer to produce than orcs."))
(unit-type dwarf (image-name "heroes-dwarf1") (char "d")
  (hp-max 10) (acp-per-turn 4) (cp 16)
  (help "Builds your infrastructure")
  (notes "As far as combat goes, dwarves are no more useful than orcs.  However, they are the only units that can build orc holes, nests, or pentagrams.  They can give you quite an edge if your opponents don't have any."))

;; The prefix "giant" has been removed from all insects...
(unit-type ant (name "ant") (image-name "ant") (char "a")
  (hp-max 15) (acp-per-turn 3) (cp 4)
  (help "Slowest of all monsters, but easy to produce, armored, and explosive")
  (notes "Ants are really slow-moving, but they can be produced rapidly and they have hard exoskeletons.  Because of the exoskeleton, an ant can survive 50% more damage than any humanoid.  Ants can also explode on command with enough destructive force to critically injure any living creature (except a dragon or beholder) within 2 cells.  Three exploding ants are enough to kill a red dragon, and seven could wipe out an orc hole or a nest."))
(unit-type beetle (name "beetle") (image-name "beetle") (char "b")
  (hp-max 20) (acp-per-turn 4) (cp 8)
  (help "Faster than ants, and even more heavily armored")
  (notes "Beetles are faster and even more heavily armored than ants.  They can move as fast as a human or orc, and can survive twice as much damage.  Of course, they take longer to produce than ants."))
(unit-type spider (name "spider") (image-name "spider") (char "s")
  (hp-max 25) (acp-per-turn 6) (cp 12)
  (help "One of the fastest, most heavily armored monsters")
  (notes "Spiders and elves are the fastest walking creatures.  Spiders also have the thickest exoskeleton of all the insects.  It would probably take at least three humanoids of any kind working together to bring down a spider (although a single combat-hardened human could take one on)."))

(unit-type green-dragon (name "green dragon") (image-name "monsters-green-dragon") (char "g")
  (hp-max 40) (acp-per-turn 4)
  (help "Slightly weaker than a pit demon, but brethes fire")
  (notes "Dragons cannot be produced; if you plan to bring them over to your side, you need a humanity stone so that you can hypnotize them.  Green dragons are not quite as fast or as powerful as red dragons; they're not even as strong as a pit demon.  Nevertheless, they are deadly when they brethe fire.  Dragons can also carry any human or humanoid on their backs, making them even more fierce in combat.  However, the speed of a green dragon would not warrant using them for any sort of cavalry."))
(unit-type red-dragon (name "red dragon") (char "r")
  (hp-max 50) (acp-per-turn 8)
  (help "A harbinger of doom, if motivated to kill")
  (notes "Dragons cannot be produced; if you plan to bring them over to your side, you need a humanity stone so that you can hypnotize them.  The red dragon is unquestionably the most powerful unit.  They move faster than any other unit, they (along with pit demons) are the most resilient creatures to injury, and they brethe fire.  Dragons can also carry any human or humanoid on their backs, making them an excellent way to get around.  Few would ever take on a dragon cavalry!"))

(unit-type imp (image-name "monsters-undead") (char "i")
  (hp-max 5) (acp-per-turn 4) (acp-min -3) (cp 4)
  (help "Rather weak, but can move through solid rock (slowly)")
  (notes "Imps are the weakest of all the monsters.  The second-weakest creatures can survive twice as much damage an an imp.  However, imps possess the ability to pass through solid rock.  They can easily launch a surprise attack on an isolated room, but make sure that you have enough to overcome whatever defenses they might have!"))
  ;; Is there a better imp picture?
(unit-type pit-demon (name "pit demon") (char "p")
  (hp-max 50) (acp-per-turn 4) (cp 40)
  (help "Very hard to kill, strikes fear in all but the dragons")
  (notes "A pit demon is as strong as a dragon, but it's slower and doesn't brethe fire.  Still, it's a powerful unit that could defeat just about any moderate opposition (although a pit demon can't take on a dozen orcs).  Just remember that they take a long time to produce, and won't be of any use if their pentagram is captured before they are completed."))

(unit-type beholder (char "B")
  (hp-max 50) (acp-per-turn 4) (cp 40)
  (help "Evil eye can kill if it stares at a victim for long enough")
  (notes "Beholders cannot be produced; if you want to have any, you must seek them out and hypnotize them.  Beholders are similar in strength and overall function to dragons.  They move as fast as anything except an elf, spider, or red dragon; and they can transfix an enemy from a distance by staring at it.  A guardian stone and a lightning stone in combination is your only hope, short of a mob attack, of defeating an enemy's beholder."))

;; The only purpose of some units is to increase an enemy's CXP...
(unit-type skeleton
  (hp-max 5) (acp-per-turn 4)
  (help "Runs around and attacks anything that it doesn't recognize")
  (notes "Living skeletons run around and may attack you (or your enemies).  They are easy to kill, but they are nonetheless worth worrying about."))

;; At one time, I had defined balrogs as a unit, but they turned out to be 
;; WAY too powerful.

(unit-type coins (name "Magic supply kit") (image-name "bank") (char "K")
  (help "Produces food and water for the bearer") (cp 180)
  (notes "A magic supply kit spontaneously produces food and water.  Oftentimes, it is enough that anybody who carries one could wander around for eternity and never need to resupply."))
(unit-type sapphire (name "Guardian stone") (char "G")
  (help "Reduces the risk of injury in battle") (cp 180)
  (notes "A guardian stone protects the bearer from most damage (60%) during a battle.  It could make anybody very hard to kill, but there is always the small risk that the stone could be shattered during battle."))
(unit-type emerald (name "Humanity stone") (char "H")
  (acp-per-turn 1) (cp 180)
  (help "Can clone a human or hypnotize your enemies")
  (notes "The humanity stone can do two things.  First, it is the only way to create new humans (that's sure to confuse your enemies, if not more).  Second, it can be used to hypnotize enemies into coming over to your side.  It's not perfect, though.  It only has a 40% success rate at hypnosis, so it may take several turns to succeed.  It's also useless at a distance; it can't save you if your enemy has a lightning stone."))
(unit-type ruby (name "Lightning stone") (char "L")
  (acp-per-turn 1) (cp 180)
  (help "Can fire a bolt of lightning at a distant target")
  (notes "A lightning stone can fire a bolt of lightning at a target up to 3 cells away.  It's one of the best ways to attack enemy strongholds because you can move the bearer close to the target, fire a bolt of lightning, then get out of sight."))
(unit-type diamond (name "Glowing stone") (char "g")
  (help "Allows the bearer to see things father away") (cp 180)
  (notes "A glowing stone allows the bearer to see up to 8 cells away without allowing whatever is 8 cells away to see you.  It is most useful for exploration and intelligence-gathering."))

(unit-type nest (char "N")
  (hp-max 50) (acp-per-turn 1) (cp 30)
  (help "Produces giant insects"))
(unit-type orc-hole (name "orc hole") (image-name "ruins") (char "O")
  (hp-max 50) (acp-per-turn 1) (cp 30)
  (help "Produces humanoid monsters"))
(unit-type pentagram (image-name "star-2") (char "P")
  (hp-max 50) (acp-per-turn 1) (cp 60)
  (help "Produces the most demonic creatures"))
(unit-type ruins (image-name "ancient-ruins")
  (hp-max 6) (acp-per-turn 0)
  (help "Any orc hole, nest, or pentagram that has been destroyed in battle"))

(define humanoids (orc elf dwarf))

(define insects (ant beetle spider))

(define dragons (green-dragon red-dragon))

(define reptiles (green-dragon red-dragon))

(define demons (imp pit-demon))

(define other (beholder))

(define monsters (append humanoids insects reptiles demons other))

(define animate (append human monsters))

(define items (coins sapphire emerald ruby diamond corpse))

(define places (nest orc-hole pentagram))

(define small-obstacles (skeleton))

;; All units are powerful enough (in their respective ways) that they can 
;; be counted equally point-wise.  Corpses and ruins, of course, don't 
;; count, and one place is worth five moving units, point-wise.
(add u* point-value 1)
(add (corpse ruins) point-value 0)
(add places point-value 5)
;; (should dragons and beholders count for more points?)

;; If an independent side is created, it shouldn't be given too much power 
;; or it could easily mash the regular players.
(add elf possible-sides (not "independent"))
(add beetle possible-sides (not "independent"))
(add spider possible-sides (not "independent"))
(add pit-demon possible-sides (not "independent"))
;; This scheme allows independent orcs, dwarves, ants, and imps.  Dragons 
;; and beholders are unaffected (they start independent and must be 
;; hypnotized, as usual).

(add animate wrecked-type corpse)
(add places wrecked-type ruins)
;; This has a problem: corpses seem to be able to capture places.
;; It also seems to delay the defeat of a side where the human has been 
;; killed.

(material-type food)
(material-type water)

(add food help "Everybody needs to eat or they starve to death")
(add water help "Everybody needs water or they shivel up and die")

(add food notes "Depending on the size of the game, you may find that some creatures consume more food than others.  It's quite simple: the more that anything moves, the more food it needs to sustain itself.")
(add water notes "Water is consumed at the same rate no matter what a creature is doing.  That may sound strange, but keep in mind that in an underground cave, the temperature is low enough that few things ever need to prespire.")

;; The game is more interesting when the items do things

(table protection     ;; Remember, this denotes change in hit/capture chance!
  (human places 0.01)
  (humanoids places 0.2)
  (insects places 0.4)
  (dragons places 0.1)
  (imp places 0.5)
  (pit-demon places 0.1)
  (beholder places 0.1)
  (sapphire u* 0.4)
)

(add ruby acp-to-fire 1)
(add ruby range 3)

;; Supplies

(table unit-initial-supply
  (u* m* 9999)
)
(table base-production
  (coins food 6)
  (coins water 3)
  (places food 20)
  (places water 10)
  (emerald food 1)      ;; Necessary for when it produces clones, but the
  (emerald water 1)     ;; production is low to prevent lots of clones.
)

(table base-consumption
  (u* food 1)
  (u* water 1)
  (items food 0)
  (items water 0)
  (small-obstacles m* 0)
)

(table consumption-per-move
  (u* food 1)
  (small-obstacles m* 0)
)

(table hp-per-starve
  ;; Needs to be greater than 1 or hp-recovery will cancel it out
  (u* food 2.00)
  (u* water 4.00)  ;; Dehydration is 3X (not 2X) more deadly than starvation.
  ;; Non-living things don't need food or water.
  (items m* 0.00)
)

(table out-length
  ;; Supplies can travel between nearby places.
  (places m* 2)
  ;; Anybody can pick up supplies from a corpse...
  (corpse m* 1)
  ;; ...or from ruins.
  (ruins m* 1)
)
 (table in-length
  ;; Supplies can travel between nearby places.
  (places m* 2)
  ;; No point in a corpse getting supplies, so don't allow it.
  (corpse m* -1)
)

;; The terrain graphics have been redefined; passages were flagstone, the 
;; others were their names (except river and tunnel, which previously 
;; didn't exist.
(terrain-type floor (char "X") (image-name "desert"))
(terrain-type passage (image-name "vacuum") (char "."))
(terrain-type rock (char "#") (image-name "mountains"))
(terrain-type river (char "=") (image-name "blue"))
(terrain-type tunnel (char "-") (image-name "black")
  (subtype connection) (subtype-x road-x))

(add floor notes "Floors refer to large open parts of a cave.  They are where you are likely to find magical objects and the homes of cave-dwelling monsters.")
(add passage notes "Passageways connect floors to each other.  However, they are narrow, usually only wide enough for one creature to pass at a time.")
(add rock notes "Rock refers to any part of the cave that is not hollow.  Only a few supernatural beings such as imps can pass through rock, although some can dig through it.")
(add tunnel notes "Tunnels can be dug out to allow easy movement between passageways and/or rooms.")

;; No tunnels are created by default; they must be dug out manually.

(add rock thickness 10)

;; Unit-unit relationships.

(add human capacity 16)
(add corpse capacity 16)
(add humanoids capacity 8)
(add dragons capacity 1)

(table unit-capacity-x
  (u* coins 1)
  (items coins 0)
  (places coins 0)
  (ruins coins 0)
  (small-obstacles coins 0)
)

(table occupant-max
  (u* coins 1)      ;; No reason to have more than one Magic supply kit
  (u* diamond 1)    ;; No reason to have more than one Glowing stone
)

(add places capacity 8)
(add ruins capacity 8)

(table unit-size-as-occupant
  (u* u* 999)
  (items u* 1)
  (corpse u* 999)
  (u* nest 1)
  (u* orc-hole 1)
  (u* pentagram 1)
  (items places 999)
  (corpse places 1)

  ;; Dragons can be ridden
  (u* dragons 999)
  (human dragons 1)
  (humanoids dragons 1)
  )

;; Unit-terrain relationship.

(add t* capacity (16 4 16 4 4))

(table unit-size-in-terrain
  (animate t* 4)
  (items t* 0)

  ;; Note that places can exist in passageways.  However, it's usually not 
  ;; a good idea, as it may slow down the movement of troops.
  (places floor 16)
  (places passage 4)
  )

;; Unit-material.

(table unit-storage-x
  (u* food 200)
  (u* water 50)
  (places food 2000)
  (places water 500)
  (items food 0)
  (items water 0)
  (emerald (food water) (200 50))      ;; Needed for cloning
  (corpse (food water) (200 50))       ;; Corpses may carry supplies
  (ruins (food water) (2000 500))      ;; Ruins may have supplies
)

;;; Vision.

;; (should make longer only around light sources or some such)

(add u* vision-range 5)
;; Humans are not cave dwellers by nature, don't see as well.
(add human vision-range 4)

(add items vision-range -1)

;; A glowing stone (diamond) can increase aanybody's vision.
(add diamond vision-range 8)

(add u* can-see-behind false)

(table eye-height
  (u* t* 5)
  )

;;; Movement.

(add places speed 0)

(table mp-to-enter-terrain
  (u* rock 99)
  (items t* 99)
  (imp rock 4)
  (animate tunnel 0)
  )

(table mp-to-traverse
  (animate tunnel 1)
)

(table mp-to-enter-own
  (u* items 0)
  )

;;; Construction.

(table can-create
  (nest insects 1)
  (orc-hole humanoids 1)
  (pentagram demons 1)
  (dwarf places 1)
  (emerald human 1)
  )

(table can-build
  (nest insects 1)
  (orc-hole humanoids 1)
  (pentagram demons 1)
  (dwarf places 1)
  (emerald human 1)
  )

(table acp-to-create
  (nest insects 1)
  (orc-hole humanoids 1)
  (pentagram demons 1)
  (dwarf places 1)
  (emerald human 1)
  )

(table acp-to-build
  (nest insects 1)
  (orc-hole humanoids 1)
  (pentagram demons 1)
  (dwarf places 1)
  (emerald human 1)
  )

;; Doctrines are critical for dwarves and Humanity stones.  Without 
;; doctrines, they would keep building until they ran out of space to 
;; build in!

(doctrine build-once
  (construction-run (u* 1))
)

(side-defaults
  (doctrines (emerald build-once) (dwarf build-once))
)

;;; Combat.

(table acp-to-attack
  (u* u* 2)
  (places u* 1)
  )

(table acp-to-defend
  (u* u* 1)
  )

(table hit-chance
  (u* u* 50)
  (places human 50)
  (human places 50)
  (u* items 5)     ;; Nobody wants to smash the power stones!
  (u* corpse 75)   ;; Corpses are very easy to destroy.
  )

(table damage
  (u* u* 1d6)
  (places u* 1d4)  ;; Places shouldn't be able to kill anything in 1 hit.
  (ruby u* 5)      ;; A magic bolt of lightning has a predictable effect.
  )

(table acp-to-capture
  (u* items 1)
  (places u* 0)
  (human places 1)
  (dragons places 1)
  (beholder places 1)
  ;; Humans, dragons, and beholders can nonviolently capture places.  All 
  ;; others can still capture, but risk damage to themselves and to the 
  ;; prize.

  (emerald u* 1)
  (emerald human 0)

  )

(table capture-chance
  (u* items 100)
  (u* places 30)
  (u* ruins 100)
  ;; Humanity stones can hypnotize enemies (but does nothing to humans).
  (emerald u* 40)
  (emerald items 100)
  ;; Dragons and beholders are easier to hypnotize.
  ;;    (this counterbalances that they're impossible to produce.)
  (emerald dragons 70)
  (emerald beholder 60)
  )

;; If an enemy is captured, it can be made to talk...
(add u* see-terrain-if-captured 10)
;; ...unless it's an inanimate object.
(add items see-terrain-if-captured 0)

(table see-others-if-captured
  (places places 25)
  (humanoids u* 30)
  (insects u* 10)
  (imp u* 15)
  (pit-demon u* 10)
  (items u* 0)
)

(table occupant-escape-chance
  (u* u* 100)
) ;; This prevents mobile units from vanishing if they're in a place that 
  ;; is captured, but it also causes anything that can't escape to be 
  ;; captured, whether or not it is supposedly possible.

(table independent-capture-chance
  (u* items 100)
  )

(add dragons acp-to-fire 4)
(add beholder acp-to-fire 4)

(add dragons range 6)
(add beholder range 4)

(add human cxp-max 999)

(table cxp-per-combat
  (u* u* 1)
  (u* items 0)
  )

(table damage-cxp-effect
  (human u* 1000)
  )

(table hit-cxp-effect
  (human u* 1000)
  )

;; Ants can explode themselves (it's true; did you know that?)
(add ant acp-to-detonate 2)
(add ant hp-per-detonation 50)
(table detonation-unit-range
  (ant u* 2)
 )
(table detonation-damage-at
  (ant u* 50)     ;; Enough to kill anything, if you can get this close
)
(table detonation-damage-adjacent
  (ant u* 25)    ;; Enough to kill anything except a dragon or beholder...
  (ant human 9)  ;; ...or a human.
)
;; (shouldn't terrain (e.g. solid rock) block the effect of detonation?)


;; Dwarves and humans can dig.
;;   (this didn't work with cell terrain, so I changed it to allow 
;;   tunneling)

(table acp-to-add-terrain
  (dwarf tunnel 1)
  (human tunnel 2)
  )


(table attrition
  (corpse t* 2.00)
  (ruins t* 2.00)
)

;;; Backdrop activities.

(add u* hp-recovery 1.00)
;; (add u* acp-to-repair 2)

(set action-notices '(
  ((capture u* items) (actor " finds " actee "!"))
  ((liberate u* items) (actor " finds " actee "!"))
  ((destroy u* human) (actor " kills " actee "!"))
  ((destroy u* items) (actor " smashes " actee "!"))
  ((destroy u* nest) (actor " erradicates " actee "!"))
  ((destroy u* orc-hole) (actor " plugs " actee "!"))
  ((destroy u* pentagram) (actor " erases " actee "!"))
  ((capture emerald u*) (actor " hypnotizes " actee "!"))
  ((liberate emerald u*) (actor " de-hypnotizes " actee "!"))
  ((unit-starved u*) (0 " runs out of supplies and starves to death!"))
  ))

;;; What if a player resigns or gives up?

(add u* lost-vanish-chance 0)         ;; Don't vanish by default
(add u* lost-revolt-chance 10000)     ;; All non-humans revolt
(add human lost-vanish-chance 10000)  ;; Human vanishes

;; Put some places near the player that are easy to capture.
;; This help machine players and makes sure that the game really is 
;; exciting!
(add places independent-near-start 1)

;;; Random setup.

(add floor maze-room-occurrence 100)

(add passage maze-passage-occurrence 100)

(add t* occurrence 0)

(add rock occurrence 1)

(set edge-terrain rock)

(set maze-passage-density 5000)

;;; One adventurer on a side.

;; When humans battle for control of the cave, nobody starts with anything 
;; except themselves.
(add human start-with 1)

(add human can-be-self true)
(set self-required true)


;; Give all units appropriate names.
(include "ng-weird")

(add human namer "generic-names")
(add humanoids namer "generic-names")
(add dragons namer "generic-names")
(add imp namer "generic-names")
(add pit-demon namer "generic-names")
(add beholder namer "generic-names")
(add insects namer "junky")
;; This name system needs improvement; different kinds of monsters should 
;; have different kinds of names.

(set country-radius-min 5)
(set country-separation-min 20)
(set country-separation-max 999)

(table independent-density
  (items floor 200)
  (corpse floor 0)
  (places floor 200)
  (red-dragon floor 20)
  (green-dragon floor 32)
  (beholder floor 24)
  (red-dragon passage 20)
  (green-dragon passage 32)
  (beholder passage 24)
  (small-obstacles passage 200)    ;; Is this a good density?
  )    ;; Needs work; there should be at least one of everything 
       ;; guaranteed in every game.

(table favored-terrain
  (u* floor 100)
  (places passage 0)
  (items passage 50)
  (u* rock 0)
  (small-obstacles passage 100)
  )

(set synthesis-methods
  '(make-maze-terrain make-countries make-independent-units))

(set sides-min 2)

(scorekeeper (do last-side-wins))

(set meridians false)

(game-module (notes (
"The cave is filled with magical stones and strange monsters.  Find the stones, capture the monsters' homes, and turn them to your purposes.  Slay all who oppose you.
"
"Different places (i.e. orc holes, nests, pentagrams) produce different kinds of creatures.  You can use them for your own armies to fight your enemies (combat is rarely one-on-one).  You and your monsters can also carry magical stones that help to fight and/or to stay alive, but remember, the enemy may also have magical stones!  Stones can also be destroyed in combat, so don't put your life in their hands.
"
"You might run across dragons and beholders.  They will not attack anyone (unless you turned on the iplayer), but they will defend themselves from attackers.  If the dragon knows its stuff, it can turn armies into mass graves.  If you want them to join your side, you need to hypnotize them with a Humanity Stone.
"
"In large games, you should pay attention to resources like food and water.  If you produce too many units too quickly, your facilities may run out of supplies and your armies may starve.  The Magic Supply Kits are helpful for such circumstances, but a single Kit cannot feed an army.
"
"Most of the time, defeating an AI mplayer is very easy.  It doesn't seem to understand how to use the magical stones, although it understands armies well enough."
  )))

(game-module (design-notes (
  "This game is an elaboration on the Cave of Wandering Death.
""Major changes are:
""1. Sides used to be 'You' and 'The Monsters'; now they're undefined (i.e. A, B, C...)
""2. Every side begins with a single human, nothing more.
""3. Places begin independent and can be captured by any side and used to produce monsters.
""4. All items do things that directly affect the game; more on that later.
""5. Dragons and beholders are independent and impossible to produce.
""6. Food and water are now critical supplies; if you run out, you starve.
""7. Many new image sets are used; I think that everything looks better now.

""Minor changes are:
""1. Elves get 6 ACP instead of 4, thereby justifying their CP (orcs are easier to produce, and until now were the same speed)
""2. Dwarves not only can alter terrain, they can build new orc holes, new nests, and new pentagrams.
""3. Imps can move through walls.  Before, they were virtually worthless.
""4. Gold coins are now a Magic Supply Kit; they produce food and water.
""5. Sapphires are now Guardian Stones; they make you harder to hit.
""6. Emeralds are now Humanity Stones; they hypnotize any monster (but not a human).
""7. Rubies are now Lightning Stones; they fire up to 3 cells away and deliver 5 HP of damage.
""8. Diamonds are now Glowing Stones; they see anything up to 8 cells away.

""The game now also includes living skeletons.  Perhaps it could still be elaborated."

  )))

;; The original author is unknown; the original game definition had no 
;; indication of who wrote it or when.  The new modifications were made by 
;; Lincoln Peters in the year 2002.  At the time of this writing, I can be 
;; reached by e-mail at peters2000@mindspring.com or at 
;; lincoln_peters@hotmail.com

;; Of course, if this game definition sits unchanged until ten years from 
;; now (as some actually have), I might very well no longer be at either 
;; of those addresses. Only time will tell...

