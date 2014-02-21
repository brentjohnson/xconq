(game-module "opal-rules"
  (title "Opal Base Ruleset")
  (version "2.1")
  (blurb "Originally an attempt at a Master of Magic clone.  Control one of eight major sides in an "
  	 "attempt to, you guessed it, rule the world.")
(variants
	(see-all false)
	(world-seen false)
	(sequential true)	
	(world-size (60 60 360))
)

  )

;; PROPOSED RANDOM EVENTS
;; Dragonflight
;; Myrkirian Raid

;; Dispel
;; Growing Bospallians, Axeheads, Sea Serpents, Velds
;; Thousand Years in a Day (999 stone, 999 gold, 999 iron, build any 1 unit)
;; The Tarrasque and the Nameless Beast

;; Zombie cavalry, Phantom Myrmidon
;; Banshees, witches, Ogre Overlord (Ogre upgrade), Mad Doctor (Orcspawn shaman upgrade)

;; Elemental Mines cost 20 Mana per turn.
;; Elemental Mine (Ekkeko) ('A wishing well that the foolhardy have decided to climb down, intent on prying gemstones from the great crown of Ekkeko.  Provides 15 gold/turn.)
;; Elemental Mine (Elysium) ('Cut into the fabric of myth, through which the reckless send their woodcutters to the World Tree.  Provides 15 wood/turn.)
;; Elemental Mine (Mulciber) ('An extradimensional mousehole, to pick the pocket of Mulciber.  Provides 15 iron/turn')
;; Elemental Mine (Utu) ('A bridge to Utu's Hourglass, to collect grains before they've fallen into Entropy.  Provides 15 stone/turn')


;;; Vision.

(set ai-may-resign false)
(set indepside-has-ai true)
(set indepside-has-treasury false)
(set self-required true)
(set sides-wanted 4)



;;; Types.

;; Basic Units

(unit-type scout (name "Scout") (image-name "ang-fighter-sword-dagger2")
  (acp-per-turn 6) (hp-max 15)  (vision-range 3) (cp 6)
  (cxp-max 1000)
  (point-value 2)
  (help "Light infantry with great vision.  Weak attack, medium defense, medium resistance."))

(unit-type sword (name "Swordsmen") (image-name "ang-fighter-chromatic-shield")
  (acp-per-turn 6) (hp-max 20) (vision-range 1) (cp 6)
  (cxp-max 1000)
  (point-value 2)
  (help "Basic melee unit.  Weak attack, moderate defense, low resistance."))

(unit-type halberd (name "Halberdier") (image-name "ang-fighter-skinny-axe")
  (acp-per-turn 6) (hp-max 30) (vision-range 1) (cp 6)
  (cxp-max 1000)
  (point-value 3)
  (help "Improved melee unit.  Weak attack, moderate defense, low resistance."))

(unit-type crusader (name "Crusaders") (image-name "ang-fighter-cross-shield")
  (acp-per-turn 6) (hp-max 35)  (vision-range 1) (cp 6)
  (cxp-max 1000)
  (point-value 4)
  (help "Members of any of a number of Cedrican orders, some centuries old, founded to to prepare men for crusades into the great jungles of the Iriken Empire.  Only crusaders can capture the usurper and reunite the Aristocracy.  Strong attack, high defense, high resistance."))

(unit-type assassin (name "Assassins") (image-name "ang-thief-black-cloak")
  (acp-per-turn 6) (hp-max 30)  (vision-range 3) (cp 6)
  (point-value 4)
  (help "Professional killers who have spent their lives devoted to learning the ancient art of assassination.  Only assassins can capture the king, an act that would force him to recognize the usurper and surrender his forces to the new order.  Strong attack, no defense, high resistance."))

(unit-type bow (name "Bowmen") (image-name "ang-archer-blonde-shortbow")
  (acp-per-turn 6) (hp-max 25) (acp-to-fire 1) (cp 6) (range 2) (vision-range 1)
  (cxp-max 1000)
  (point-value 2)
  (help "Basic ranged unit.  Weak attack, low defense, weak ranged attack, low resistance."))

(unit-type longbow (name "Longbowman") (image-name "ang-archer-longbow-green")
  (acp-per-turn 6) (hp-max 25) (acp-to-fire 1) (range 3) (vision-range 1) (cp 6)
  (cxp-max 1000)
  (point-value 3)
  (help "Shogunate archers with a more powerful ranged attack.  Weak attack, low defense, low resistance strong ranged attack."))

(unit-type cavalry (name "Cavalry") (image-name "ancient-rider")
  (acp-per-turn 8) (hp-max 40) (vision-range 3) (cp 6)
  (cxp-max 1000)
  (point-value 3)
  (help "Basic mounted unit.  Weak attack, moderate defense, low resistance."))

(unit-type catapult (name "Catapult") (image-name "ancient-onager")
  (acp-per-turn 6) (hp-max 25) (acp-to-fire 1) (cp 9)  (range 3) (vision-range 1)
  (point-value 4)
  (help "Basic siege engine.  Weak attack, no defense, no resistance, strong ranged attack."))

(unit-type mage (name "Mage") (image-name "ang-mage-black-caped")
  (acp-per-turn 6) (hp-max 25) (vision-range 2) (acp-to-fire 1) (range 3) (cp 9)
  (cxp-max 1000)
  (point-value 5)
  (help "Basic magic unit.  Strong attack, no defense, high resistance, strong ranged attack."))

(unit-type dervish (name "Dervish") (image-name "ang-mage-red-cloak")
  (acp-per-turn 6) (hp-max 25) (vision-range 2) (acp-to-fire 2) (range 3)  (cp 6)
  (cxp-max 1000)
  (point-value 5)
  (help "A weak bedouin mage able to be produced in smaller cities.  Strong attack, no defense, high resistance, strong ranged attack."))

(unit-type hero (name "Hero") (image-name ("ang-fighter-gold-sword" "ang-paladin-right-sword" "ang-paladin-left-sword" "ang-paladin-left-scimitar"))
  (acp-per-turn 6) (hp-max 60) (vision-range 4)  (cxp-max 2000) 
  (can-be-self true) (self-resurrects true)
  (point-value 7)
  (help "A veteran of many battles, the hero is a strong warrior who is able to lead a nation in the event of the death of its leader.  Heroes only gain xp when they fight other heroes, archmages and leaders.  If they survive a dozen such battles, they can turn into Legendary Heroes.  Powerful attack, high defense, high resistance."))

(unit-type knight (name "Knights") (image-name "ancient-cavalry")
  (acp-per-turn 8) (hp-max 80) (vision-range 3)
  (cxp-max 1000) (can-be-self true) (self-resurrects true)
  (point-value 8)
  (help "Elite mounted unit.  Strong attack, high defense, high resistance."))

(unit-type ranger (name "Ranger") (image-name ("ang-ranger-quiver-blonde" "ang-ranger-no-quiver" "ang-ranger-quiver-sword" "ang-ranger-quiver-sword2"))
  (acp-per-turn 6) (hp-max 60) (vision-range 3) (acp-to-fire 1) (range 3)
  (cxp-max 2000) (can-be-self true) (self-resurrects true)
  (point-value 7)
  (help "Elite ranged unit.  Powerful attack, high defense, high resistance, powerful ranged attack."))

(unit-type epic (name "Epic Hero") (image-name "ang-fighter-red-leggings")
  (acp-per-turn 6) (hp-max 100) (vision-range 5)  (cxp-max 2000) 
  (can-be-self true) (self-resurrects true)
  (point-value 15)
  (help "One of the greatest heroes of the age, an Achilles or Hector, who can slay great swathes of opponents.  Powerful attack, very high defense, full resistance."))

(unit-type legend (name "Legendary Hero") (image-name "ang-fighter-gold-gold")
  (acp-per-turn 6) (hp-max 150) (vision-range 6)  (cxp-max 3000) 
  (can-be-self true) (self-resurrects true)
  (point-value 20)
  (help "Extremely powerful, capable of leading a nation or defeating a Great Wyrm.   Incredible attack, very high defense, full resistance."))

(unit-type wizard (name "Wizard") (image-name ("ang-druid-no-staff" "ang-druid-brown-staff" "ang-druid-yellow-staff" "ang-enchanter-brown-staff" "ang-enchanter-orange-staff" "ang-enchanter-purple-staff" "ang-enchanter-yellow-staff"))
  (acp-per-turn 6) (hp-max 60) (vision-range 4) (acp-to-fire 1) (range 4) 
  (can-be-self true) (self-resurrects true)  (cxp-max 2000) 
  (point-value 8)
  (help "A magician of great strength and deep knowledge.  An wizard, like a hero, can hold the reins of power in the event that a nation's leader dies.  The wizard can channel the energies of a nation in the same way that a leader can, allowing for the casting of much more powerful spells.  Powerful attack, high defense, full resistance."))

(unit-type archmage (name "Archmage") (image-name "ang-druid-gray-staff")
  (acp-per-turn 6) (hp-max 100) (vision-range 5) (acp-to-fire 1) (range 5) 
  (can-be-self true) (self-resurrects true)  (cxp-max 2000) 
  (point-value 16)
  (help "The most powerful of magicians.  Powerful attack, very high defense, full resistance."))


(add (sword halberd) auto-upgrade-to hero)
(add (cavalry crusader) auto-upgrade-to knight)
(add (scout bow longbow) auto-upgrade-to ranger)
(add (dervish mage) auto-upgrade-to wizard)
(add wizard auto-upgrade-to archmage)
(add (hero ranger knight) auto-upgrade-to epic)
(add epic auto-upgrade-to legend)

(define basic-types (scout sword halberd bow cavalry catapult mage))

(define specific-types (longbow crusader assassin dervish))


;; LIZARDMEN

(unit-type lsword (name "Warrior") (image-name "ang-saurial-barbarian-fist")
  (acp-per-turn 6) (hp-max 30) (vision-range 1) (cp 6)
  (cxp-max 1000)
  (point-value 2)
  (help "Basic melee unit.  Weak attack, moderate defense, low resistance."))

(unit-type lhalberd (name "Brokenhorn") (image-name "ang-saurial-black-trident")
  (acp-per-turn 6) (hp-max 40) (vision-range 1) (cp 6)
  (cxp-max 1000)
  (point-value 3)
  (help "Improved melee unit.  Weak attack, moderate defense, low resistance."))

(unit-type lbow (name "Javelineer") (image-name "ang-saurial-gray-club")
  (acp-per-turn 6) (hp-max 35) (acp-to-fire 1) (cp 6) (range 2) (vision-range 1)
  (cxp-max 1000)
  (point-value 2)
  (help "Basic ranged unit.  Weak attack, low defense, weak ranged attack, low resistance."))

(unit-type lcavalry (name "Geh-Geh") (image-name "ang-monster-eye-head")
  (acp-per-turn 8) (hp-max 50) (vision-range 3) (cp 6)
  (cxp-max 1000)
  (point-value 3)
  (help "Basic mounted unit.  Weak attack, moderate defense, low resistance."))

(unit-type lmage (name "Priest") (image-name "ang-saurial-cross-cloak")
  (acp-per-turn 6) (hp-max 35) (vision-range 2) (acp-to-fire 1) (range 3) (cp 9)
  (cxp-max 1000)
  (point-value 5)
  (help "Basic magic unit.  Strong attack, no defense, high resistance, strong ranged attack."))

(unit-type croc (name "Tamer") (image-name "ang-saurial-gray-naked")
  (acp-per-turn 6) (hp-max 25) (vision-range 3) (cp 6)
  (cxp-max 1000) (point-value 3)
  (help "A lizardman born wild and still half-feral, who speaks the tongue of the reptile.  He can sometimes convince the great lizards of the swamp to help his people.  He can even convince dragons, if they are young enough.  Weak attack, moderate defense, medium resistance."))

(unit-type llegend (name "Elder") (image-name "ang-saurial-scimitar-red")
  (acp-per-turn 6) (hp-max 160) (vision-range 6)  (cxp-max 3000) 
  (can-be-self true) (self-resurrects true)
  (point-value 20)
  (help "Extremely powerful, capable of leading a nation or defeating a Great Wyrm.   Incredible attack, very high defense, full resistance."))

(unit-type lepic (name "Master") (image-name "ang-saurial-single-scimitar")
  (acp-per-turn 6) (hp-max 110) (vision-range 5)  (cxp-max 2000) 
  (can-be-self true) (self-resurrects true) (auto-upgrade-to legend)
  (point-value 15)
  (help "One of the greatest heroes of the age, an Achilles or Hector, who can slay great swathes of opponents.  Powerful attack, very high defense, full resistance."))

(unit-type lhero (name "Hero") (image-name "ang-saurial-gray-shield")
  (acp-per-turn 6) (hp-max 70) (vision-range 4)  (cxp-max 2000) 
  (can-be-self true) (self-resurrects true) (auto-upgrade-to epic)
  (point-value 7)
  (help "A veteran of many battles, the hero is a strong warrior who is able to lead a nation in the event of the death of its leader.  Heroes only gain xp when they fight other heroes, archmages and leaders.  If they survive a dozen such battles, they can turn into Legendary Heroes.  Powerful attack, high defense, high resistance."))

(unit-type lranger (name "Mystic") (image-name "ang-saurial-glowhand-staff")
  (acp-per-turn 6) (hp-max 70) (vision-range 3) (acp-to-fire 1) (range 3)
  (cxp-max 2000) (can-be-self true) (self-resurrects true)
  (point-value 7)
  (help "Elite ranged unit.  Powerful attack, high defense, high resistance, powerful ranged attack."))

(unit-type larchmage (name "Speaker") (image-name "ang-saurial-cross-cloak")
  (acp-per-turn 6) (hp-max 110) (vision-range 5) (acp-to-fire 1) (range 5) 
  (can-be-self true) (self-resurrects true)  (cxp-max 2000) 
  (point-value 16)
  (help "The most powerful of magicians.  Powerful attack, very high defense, full resistance."))

(unit-type lwizard (name "High Priest") (image-name "ang-saurial-ankh-staff")
  (acp-per-turn 6) (hp-max 70) (vision-range 4) (acp-to-fire 1) (range 4) 
  (can-be-self true) (self-resurrects true)  (cxp-max 2000) 
  (point-value 8)
  (help "A magician of great strength and deep knowledge.  An wizard, like a hero, can hold the reins of power in the event that a nation's leader dies.  The wizard can channel the energies of a nation in the same way that a leader can, allowing for the casting of much more powerful spells.  Powerful attack, high defense, full resistance."))

(add (lsword lhalberd) auto-upgrade-to lhero)
(add (lbow croc) auto-upgrade-to lranger)
(add lmage auto-upgrade-to lwizard)
(add lwizard auto-upgrade-to larchmage)
(add (lhero lranger) auto-upgrade-to lepic)
(add lepic auto-upgrade-to llegend)

(define liz-types (lsword lhalberd lbow lcavalry lmage lwizard lranger lhero lepic llegend))

;; DWARF

(unit-type dsword (name "Swordsman") (image-name "ang-dwarf-sword-skirt")
  (acp-per-turn 6) (hp-max 30) (vision-range 1) (cp 6)
  (cxp-max 1000)
  (point-value 2)
  (help "Basic melee unit.  Weak attack, moderate defense, low resistance."))

(unit-type dhalberd (name "Axeman") (image-name "ang-fighter-halfling-axe")
  (acp-per-turn 6) (hp-max 40) (vision-range 1) (cp 6)
  (cxp-max 1000)
  (point-value 3)
  (help "Improved melee unit.  Weak attack, moderate defense, low resistance."))

(unit-type dbow (name "Crossbowman") (image-name "ba-archer-dwarf")
  (acp-per-turn 6) (hp-max 35) (acp-to-fire 1) (cp 6) (range 2) (vision-range 1)
  (cxp-max 1000)
  (point-value 2)
  (help "Basic ranged unit.  Weak attack, low defense, weak ranged attack, low resistance."))

(unit-type cannon (name "Steam Cannon") (image-name "ba-cannon")
  (acp-per-turn 6) (hp-max 50) (acp-to-fire 1)  (range 3) (vision-range 1) (cp 9)
  (point-value 5)
  (help "An enormous, steampunk cannon that the Dwarves use to annihilate fortresses, giants and great swathes of enemies.  Weak attack, no defense, no resistance, very high ranged attack."))

(unit-type dlegend (name "Legendary Hero") (image-name "ang-paladin-dwarf-scimitar")
  (acp-per-turn 6) (hp-max 160) (vision-range 6)  (cxp-max 3000) 
  (can-be-self true) (self-resurrects true)
  (point-value 20)
  (help "Extremely powerful, capable of leading a nation or defeating a Great Wyrm.   Incredible attack, very high defense, full resistance."))

(unit-type depic (name "Epic Hero") (image-name "ang-dwarf-gold-shield")
  (acp-per-turn 6) (hp-max 110) (vision-range 5)  (cxp-max 2000) 
  (can-be-self true) (self-resurrects true) (auto-upgrade-to legend)
  (point-value 15)
  (help "One of the greatest heroes of the age, an Achilles or Hector, who can slay great swathes of opponents.  Powerful attack, very high defense, full resistance."))

(unit-type dhero (name "Hero") (image-name "ang-dwarf-scimitar-beard")
  (acp-per-turn 6) (hp-max 70) (vision-range 4)  (cxp-max 2000) 
  (can-be-self true) (self-resurrects true) (auto-upgrade-to epic)
  (point-value 7)
  (help "A veteran of many battles, the hero is a strong warrior who is able to lead a nation in the event of the death of its leader.  Heroes only gain xp when they fight other heroes, archmages and leaders.  If they survive a dozen such battles, they can turn into Legendary Heroes.  Powerful attack, high defense, high resistance."))

(unit-type dranger (name "Ranger") (image-name "ang-ranger-dwarf-sword")
  (acp-per-turn 6) (hp-max 70) (vision-range 3) (acp-to-fire 1) (range 3)
  (cxp-max 2000) (can-be-self true) (self-resurrects true)
  (point-value 7)
  (help "Elite ranged unit.  Powerful attack, high defense, high resistance, powerful ranged attack."))

(add (dsword dhalberd) auto-upgrade-to dhero)
(add (dbow) auto-upgrade-to dranger)
(add (dhero dranger) auto-upgrade-to depic)
(add depic auto-upgrade-to dlegend)

(define dwarf-types (dsword dhalberd dbow dranger dhero depic dlegend cannon))

;; ONI

(unit-type osword (name "Legionary") (image-name "ang-drow-stripe-shield")
  (acp-per-turn 6) (hp-max 25) (vision-range 1) (cp 6)
  (cxp-max 1000)
  (point-value 2)
  (help "Basic melee unit.  Weak attack, moderate defense, low resistance."))

(unit-type obow (name "Bowmen") (image-name "ba-archer-drow")
  (acp-per-turn 6) (hp-max 25) (acp-to-fire 1) (cp 6) (range 2) (vision-range 1)
  (cxp-max 1000)
  (point-value 2)
  (help "Basic ranged unit.  Weak attack, low defense, weak ranged attack, low resistance."))

(unit-type ocavalry (name "Chariot") (image-name "ancient-war-cart")
  (acp-per-turn 8) (hp-max 40) (vision-range 3) (cp 6)
  (cxp-max 1000)
  (point-value 3)
  (help "Basic mounted unit.  Weak attack, moderate defense, low resistance."))

(unit-type ocatapult (name "Ballista") (image-name "ancient-ballista")
  (acp-per-turn 6) (hp-max 25) (acp-to-fire 1) (cp 9)  (range 3) (vision-range 1)
  (point-value 4)
  (help "Basic siege engine.  Weak attack, no defense, no resistance, special ranged attack."))

(unit-type weaver (name "Loreweaver") (image-name "ang-drow-black-glowhand")
  (acp-per-turn 6) (hp-max 25) (vision-range 2) (acp-to-fire 2) (range 3)  (cp 6)
  (cxp-max 1000)
  (point-value 5)
  (help "An Oni mage that adds to the research of new spells.  Strong attack, no defense, high resistance, strong ranged attack.")) 

(unit-type olegend (name "Legendary Hero") (image-name "ang-drow-kirk")
  (acp-per-turn 6) (hp-max 150) (vision-range 6)  (cxp-max 3000) 
  (can-be-self true) (self-resurrects true)
  (point-value 20)
  (help "Extremely powerful, capable of leading a nation or defeating a Great Wyrm.   Incredible attack, very high defense, full resistance."))

(unit-type oepic (name "Epic Hero") (image-name "ang-drow-purple")
  (acp-per-turn 6) (hp-max 100) (vision-range 5)  (cxp-max 2000) 
  (can-be-self true) (self-resurrects true)
  (point-value 15)
  (help "One of the greatest heroes of the age, an Achilles or Hector, who can slay great swathes of opponents.  Powerful attack, very high defense, full resistance."))

(unit-type ohero (name "Hero") (image-name "ang-paladin-drow-sword")
  (acp-per-turn 6) (hp-max 60) (vision-range 4)  (cxp-max 2000) 
  (can-be-self true) (self-resurrects true)
  (point-value 7)
  (help "A veteran of many battles, the hero is a strong warrior who is able to lead a nation in the event of the death of its leader.  Heroes only gain xp when they fight other heroes, archmages and leaders.  If they survive a dozen such battles, they can turn into Legendary Heroes.  Powerful attack, high defense, high resistance."))

(unit-type oranger (name "Ranger") (image-name "ang-drow-gold")
  (acp-per-turn 6) (hp-max 60) (vision-range 3) (acp-to-fire 1) (range 3)
  (cxp-max 2000) (can-be-self true) (self-resurrects true)
  (point-value 7)
  (help "Elite ranged unit.  Powerful attack, high defense, high resistance, powerful ranged attack."))

(unit-type oknight (name "Knights") (image-name "ancient-war-cart")
  (acp-per-turn 8) (hp-max 80) (vision-range 3)
  (cxp-max 1000) (can-be-self true) (self-resurrects true)
  (point-value 8)
  (help "Elite mounted unit.  Strong attack, high defense, high resistance."))

(unit-type oarchmage (name "Archmage") (image-name "ang-drow-beard")
  (acp-per-turn 6) (hp-max 100) (vision-range 5) (acp-to-fire 1) (range 5) 
  (can-be-self true) (self-resurrects true)  (cxp-max 2000) 
  (point-value 16)
  (help "The most powerful of magicians.  Powerful attack, very high defense, full resistance."))

(unit-type owizard (name "Wizard") (image-name "ang-drow-black-glowhand")
  (acp-per-turn 6) (hp-max 70) (vision-range 4) (acp-to-fire 1) (range 4) 
  (can-be-self true) (self-resurrects true)  (cxp-max 2000) 
  (point-value 8)
  (help "A magician of great strength and deep knowledge.  An wizard, like a hero, can hold the reins of power in the event that a nation's leader dies.  The wizard can channel the energies of a nation in the same way that a leader can, allowing for the casting of much more powerful spells.  Powerful attack, high defense, full resistance."))

(add (ocavalry osword) auto-upgrade-to ohero)
(add obow auto-upgrade-to oranger)
(add weaver auto-upgrade-to owizard)
(add owizard auto-upgrade-to oarchmage)
(add (ohero oranger oknight) auto-upgrade-to oepic)
(add oepic auto-upgrade-to olegend)


(define oni-types (osword obow ocavalry oranger ohero oepic olegend weaver oknight owizard oarchmage))

;; ONI

(unit-type isword (name "Swordsmen") (image-name "ang-orc-cross-plate")
  (acp-per-turn 6) (hp-max 25) (vision-range 1) (cp 6)
  (cxp-max 1000)
  (point-value 2)
  (help "Basic melee unit.  Weak attack, moderate defense, low resistance."))

(unit-type ihalberd (name "Halberdier") (image-name "ang-orc-halberd-shield")
  (acp-per-turn 6) (hp-max 40) (vision-range 1) (cp 6)
  (cxp-max 1000)
  (point-value 3)
  (help "Improved melee unit.  Weak attack, moderate defense, low resistance."))

(unit-type gladiator (name "Imperial Gladiator") (image-name "ang-orc-defiant-sword")
  (acp-per-turn 6) (hp-max 35)  (vision-range 1) (cp 6)
  (cxp-max 1000)
  (point-value 4)
  (help "Orc slaves that have demonstrated great prowess in the arena.  Not only are they great fighters but they can impress wild Orcs for service to the Empire.  Strong attack, medium defense, medium resistance."))

(unit-type ibow (name "Bowmen") (image-name "ba-orc-archer")
  (acp-per-turn 6) (hp-max 25) (acp-to-fire 1) (cp 6) (range 2) (vision-range 1)
  (cxp-max 1000)
  (point-value 2)
  (help "Basic ranged unit.  Weak attack, low defense, weak ranged attack, low resistance."))

(unit-type image (name "Mage") (image-name "ba-orc-mage-a")
  (acp-per-turn 6) (hp-max 35) (vision-range 2) (acp-to-fire 1) (range 3) (cp 9)
  (cxp-max 1000)
  (point-value 5)
  (help "Basic magic unit.  Strong attack, no defense, high resistance, strong ranged attack."))

(unit-type icatapult (name "Ballista") (image-name "ancient-ballista")
  (acp-per-turn 6) (hp-max 25) (acp-to-fire 1) (cp 9)  (range 3) (vision-range 1)
  (point-value 4)
  (help "Basic siege engine.  Weak attack, no defense, no resistance, special ranged attack."))

(unit-type ilegend (name "Legendary Hero") (image-name "ang-king-red-orc")
  (acp-per-turn 6) (hp-max 150) (vision-range 6)  (cxp-max 3000) 
  (can-be-self true) (self-resurrects true)
  (point-value 20)
  (help "Extremely powerful, capable of leading a nation or defeating a Great Wyrm.   Incredible attack, very high defense, full resistance."))

(unit-type iepic (name "Epic Hero") (image-name "ang-goblin-red-scimitar")
  (acp-per-turn 6) (hp-max 100) (vision-range 5)  (cxp-max 2000) 
  (can-be-self true) (self-resurrects true)
  (point-value 15)
  (help "One of the greatest heroes of the age, an Achilles or Hector, who can slay great swathes of opponents.  Powerful attack, very high defense, full resistance."))

(unit-type ihero (name "Hero") (image-name "ang-orc-red-angry")
  (acp-per-turn 6) (hp-max 60) (vision-range 4)  (cxp-max 2000) 
  (can-be-self true) (self-resurrects true)
  (point-value 7)
  (help "A veteran of many battles, the hero is a strong warrior who is able to lead a nation in the event of the death of its leader.  Heroes only gain xp when they fight other heroes, archmages and leaders.  If they survive a dozen such battles, they can turn into Legendary Heroes.  Powerful attack, high defense, high resistance."))

(unit-type iranger (name "Ranger") (image-name "ang-orc-red-scimitar")
  (acp-per-turn 6) (hp-max 60) (vision-range 3) (acp-to-fire 1) (range 3)
  (cxp-max 2000) (can-be-self true) (self-resurrects true)
  (point-value 7)
  (help "Elite ranged unit.  Powerful attack, high defense, high resistance, powerful ranged attack."))

(unit-type iarchmage (name "Archmage") (image-name "ba-orc-mage-c")
  (acp-per-turn 6) (hp-max 100) (vision-range 5) (acp-to-fire 1) (range 5) 
  (can-be-self true) (self-resurrects true)  (cxp-max 2000) 
  (point-value 16)
  (help "The most powerful of magicians.  Powerful attack, very high defense, full resistance."))

(unit-type iwizard (name "Wizard") (image-name "ba-orc-mage-b")
  (acp-per-turn 6) (hp-max 70) (vision-range 4) (acp-to-fire 1) (range 4) 
  (can-be-self true) (self-resurrects true)  (cxp-max 2000) 
  (point-value 8)
  (help "A magician of great strength and deep knowledge.  An wizard, like a hero, can hold the reins of power in the event that a nation's leader dies.  The wizard can channel the energies of a nation in the same way that a leader can, allowing for the casting of much more powerful spells.  Powerful attack, high defense, full resistance."))

(add (gladiator ihalberd isword) auto-upgrade-to ihero)
(add ibow auto-upgrade-to iranger)
(add image auto-upgrade-to iwizard)
(add iwizard auto-upgrade-to iarchmage)
(add (ihero iranger) auto-upgrade-to iepic)
(add iepic auto-upgrade-to ilegend)


(define iriken-types (isword ihalberd gladiator image ibow iranger ihero iepic ilegend iwizard iarchmage))


;; INDEPENDANTS

(unit-type merc (name "Mercenary") (image-name "ang-pirate-red-cap")
  (acp-per-turn 6) (hp-max 25) (vision-range 1) (cp 3)
  (point-value 2)
  (help "Purchased melee unit.  Weak attack, moderate defense, low resistance."))

(unit-type spear (name "Orcspawn") (image-name "ang-goblin-sword2")
  (acp-per-turn 6) (hp-max 10) (vision-range 1) (cp 6)
  (point-value 1) (cxp-max 2000)
  (help "The Orcspawn have been all but wiped out by the Oni.  They are weak fighters and often flee from enemies.  Weak attack, low defense, low resistance."))

(unit-type shaman (name "Orcspawn Shaman") (image-name "ang-goblin-green-glowhand")
  (acp-per-turn 6) (hp-max 15) (vision-range 1) (acp-to-fire 1) (range 3)  (cp 9)
  (point-value 2)
  (help "The Orcspawn Shaman is a healer and a weak spellcaster.  Can heal other units.  Strong attack, no defense, weak ranged attack, high resistance, nature immunity."))

(unit-type thug (name "Ogre Thugs") (image-name "ang-fighter-dagger-apehead")
  (acp-per-turn 6) (hp-max 40) (vision-range 1) (cp 9)
  (point-value 3) (revolt-chance 500)
  (help "The Ogres, too, were wiped out by the Oni.  Few remain and many make their way as mercenaries.  Ogres have been known to change sides suddenly.  Strong attack, moderate defense."))

(unit-type champ (name "Ogre Champions") (image-name ("ang-fighter-ape-head" "ang-fighter-ogre-sword"))
  (acp-per-turn 6) (hp-max 45) (vision-range 1) (cp 12)
  (point-value 4) (revolt-chance 500)
  (help "A particularly large or experienced, but no more loyal, group of Ogre mercenaries.  Powerful attack, moderate defense, medium resistance."))

(unit-type cutter (name "Cutter") (image-name "ancient-barge")
  (acp-per-turn 8) (hp-max 60) (vision-range 5) (cp 3) (naval true) 
  (point-value 4)
  (help "A fast ship that can carry two units.  Strong attack, high defense, high resistance."))

(unit-type whaler (name "Whaler") (image-name "ancient-bireme")
  (acp-per-turn 8) (hp-max 80) (vision-range 4) (cp 3) (naval true) 
  (point-value 4)
  (help "A slow ship with a powerful attack that is useful against large creatures.  strong attack, very high defense, very high resistance."))

(unit-type thuvi (name "Thuvi Irikani") (image-name "ang-paladin-oni-sword")
  (acp-per-turn 6) (hp-max 35)  (vision-range 1) (cp 6)
  (point-value 4)
  (cxp-max 1000) (auto-upgrade-to hero)
  (help "Descendants of Imperial rebels and allies to the Dwarven City-States.  Strong attack, high defense, high resistance."))

(unit-type tribesman (name "Orc Warrior") (image-name "ang-orc-angry-staff")
  (acp-per-turn 6) (hp-max 25)  (vision-range 1) (cp 6)
  (point-value 2)
  (help "Wild orcs, hidden in the jungle.  Weak attack, low defense, low resistance."))

(define independant-types (spear shaman thug champ cutter whaler thuvi tribesman))

;; GOLEMS

(unit-type woodgolem (name "Wood Golem") (image-name "spec-golem-wood")
  (acp-per-turn 4) (hp-max 100)  (vision-range 1) (cp 9)
  (point-value 4)
  (help "A man built of wood using eldritch methods.  Wood golems are flimsy but easy to produce.  Weak attack, moderate defense, medium resistance.  Can only be repaired by an arcane laboratory."))

(unit-type stonegolem (name "Stone Golem") (image-name "spec-golem-stone")
  (acp-per-turn 4) (hp-max 100)  (vision-range 1) (cp 12)
  (point-value 5)
  (help "A large granite man, created through ancient arts.  Stone golems are slow but strong, especially against cities and fortresses.  Strong attack, very high defense, very high resistance.  Can only be repaired by an arcane laboratory"))

(unit-type irongolem (name "Iron Golem") (image-name "spec-golem-iron")
  (acp-per-turn 4) (hp-max 100)  (vision-range 1) (cp 15)
  (point-value 7)
  (help "The most powerful of the lesser golems.  Powerful attack, high defense, high resistance.  Can only be repaired by an arcane laboratory."))

(unit-type clockgolem (name "Clockwork Golem") (image-name "spec-golem-brass")
  (acp-per-turn 6) (hp-max 100)  (vision-range 1) (cp 18)
  (point-value 8)
  (help "Running on intricate copper clockwork and sheathed in brass, the clockwork golem is an imposing goliath.  Incredible attack, moderate defense, full resistance.  Can only be repaired by an arcane laboratory."))

(define golem-types (woodgolem stonegolem irongolem clockgolem))

;; RUINS

(unit-type cityruin (name "Fallen City") (image-name "spec-citadelruin")
  (acp-independent true) (hp-max 1) (vision-range 1)
  (help "A destroyed city that still produces some material."))

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
(unit-type tower (name "Watchtower") (image-name "ancient-tower")
  (acp-per-turn 0) (hp-max 25)  (vision-range 4) (cp 3)
  (ai-war-garrison 1) (ai-peace-garrison 0)
  (help "Protects 1 unit, provides excellent vision."))

(unit-type fort (name "Wooden Fort") (image-name "spec-fort")
  (acp-per-turn 3) (hp-max 50)  (vision-range 2) (cp 3)
  (ai-war-garrison 2) (ai-peace-garrison 0)
  (point-value 2)
  (help "Cheap to produce."))

(unit-type keep (name "Keep") (image-name "ba-castle")
  (acp-per-turn 3) (hp-max 100) (wrecked-type keepruin) (vision-range 3) (cp 3)
  (ai-war-garrison 2)  (ai-peace-garrison 0)
  (point-value 3)
  (help "Protects 3 units, provides good vision."))

(unit-type citadel (name "Citadel") (image-name "ancient-black-city")
  (acp-per-turn 3) (hp-max 200) (wrecked-type citadelruin) (vision-range 4) (cp 3)
  (ai-war-garrison 3)  (ai-peace-garrison 1)
  (point-value 7)
  (help "Protects 8 units, provides excellent vision."))

(define fortress-types (tower fort keep citadel))




;; Death Creatures


(unit-type zombie (name "Zombies") (image-name ("ang-man-purple-diseased" "ang-man-green-diseased"))
  (acp-per-turn 6) (hp-max 20) (vision-range 1) (cp 6)
  (point-value 2)
  (help "Zombies don't heal and if they kill a regular unit, it turns into a zombie.  Weak attack, low defense, no resistance, death immunity."))

(unit-type spectre (name "Spectres") (image-name "ang-wraith-black-flame")
  (acp-per-turn 6) (hp-max 45) (vision-range 3) (acp-to-fire 2) (range 1) (cp 9)
  (can-be-self true) (self-resurrects true) (point-value 5)
  (help "The lieutenants of the Grey One who can create ghosts and corrupt the living to produce the dead that walk.  Strong attack, low defense, low resistance, strong ranged attack, death immunity."))

(unit-type ghost (name "Ghosts") (image-name "ang-ghost-black-gray")
  (acp-per-turn 6) (hp-max 20) (vision-range 1) (cp 3)
  (point-value 5)  (help "Short-lived but difficult to destroy.  Strong attack, no defense, no resistance, death immunity."))

(unit-type deathknight (name "Death Knight") (image-name "ang-fighter-skull-shield")
  (acp-per-turn 6) (hp-max 50)  (vision-range 1)
  (point-value 5)
  (help "A corrupted crusader.  Powerful attack, high defense, high resistance, death immunity."))

(define death-types (zombie spectre ghost))


;; WILDERNESS

(unit-type tyrannosaur (name "Regiosaur") (image-name "ang-dino-claw-hand")
  (acp-per-turn 6) (hp-max 90) (vision-range 1) (cp 3) 
  (point-value 15)
  (help "An extremely dangerous creature, look out!  Incredible attack, moderate defense, medium resistance."))

(unit-type apatosaur (name "Elosaur") (image-name "ang-dino-red-frilled")
  (acp-per-turn 6) (hp-max 150) (vision-range 1) (cp 3) 
  (point-value 12)
  (help "Very large, dangerous against fortresses and cities.  Many, many hit points.  Weak attack, moderate defense, medium resistance."))

(unit-type pterodactyl (name "Pterodactyl") (image-name "monsters-pterodactyl")
  (acp-per-turn 6) (hp-max 40) (vision-range 3) (cp 3) 
  (point-value 5)
  (help "A primitive flying dinosaur.  Timid, runs away a lot, dangerous against small units.  Strong attack, moderate defense, medium resistance."))

(unit-type plesiosaur (name "Sesosaurian") (image-name "ang-dino-green-tusk")
  (acp-per-turn 6) (hp-max 75) (vision-range 1) (cp 3) (naval true) 
  (point-value 5)
  (help "Swimming dinosaur.  Dangerous, hard to hit.  Strong attack, moderate defense, medium resistance."))

(unit-type deadwild (name "Pelts") (image-name "ang-coins-small-gold")
  (acp-independent true) (hp-max 1) (vision-range 0)
  (point-value 10)
  (help "Valuable furs, meat and/or feathers."))

(unit-type wolves (name "Wolves") (image-name "ang-dog-gray")
  (acp-per-turn 6) (hp-max 20) (wrecked-type deadwild) (vision-range 4) (cp 3)
  (point-value 2)
  (help "Great vision, high movement in forests.  Weak attack, low defense, low resistance."))

(unit-type bears (name "Bears") (image-name "spec-wildlife-bear")
  (acp-per-turn 6) (hp-max 30) (wrecked-type deadwild) (vision-range 3) (cp 3)
  (point-value 3)
  (help "Tough against small units.  Strong attack, low defense, low resistance."))

(unit-type ferak (name "Ferak") (image-name "heroes-eagles")
  (acp-per-turn 6) (hp-max 10) (wrecked-type deadwild) (vision-range 4) (cp 3) (air true) 
  (point-value 1)
  (help "The Ferak, or Eyebird, is a small, beautiful, carrion-eating bird that blinds its prey and then waits for a larger predator to finish it off.  Weak attack, low defense, low resistance."))

(unit-type royalferak (name "Royal Ferak") (image-name "heroes-eagles")
  (acp-per-turn 6) (hp-max 25) (wrecked-type deadwild) (vision-range 4) (cp 3) (air true) 
  (point-value 2)
  (help "The Royal Ferak is much larger and often does not need a larger predator to prepare its meals.  Strong attack, moderate defense, medium resistance."))

(define wild-types (tyrannosaur apatosaur pterodactyl plesiosaur wolves bears ferak royalferak))


;; GLASS-DARK

(unit-type axecorpse (name "Dead Axehead") (image-name "ang-coins-small-gold")
  (acp-independent true) (hp-max 1) (vision-range 0)
  (point-value 10)
  (help "A dead axehead, which provides a hundred and twenty-five gold."))

(unit-type axehead (name "Axehead") (image-name "spec-fish-bighead")
  (acp-per-turn 6) (hp-max 100) (wrecked-type axecorpse) (vision-range 1) (cp 9) (naval true)
  (point-value 8)  
  (help "The axehead ranges in length from sixty to three hundred feet long, and though its organs and armored head are valuable, it is an aggressive and dangerous creature.  Powerful attack, high defense, high resistance."))

(unit-type boscorpse (name "Dead Bospallian") (image-name "ang-coins-small-gold")
  (acp-independent true) (hp-max 1) (vision-range 0)
  (point-value 10)
  (help "A dead bospallian, that provides a hundred gold."))

(unit-type bospallian (name "Bospallian") (image-name "spec-fish-spot")
  (acp-per-turn 6) (hp-max 200) (wrecked-type boscorpse) (vision-range 1) (cp 3) (naval true)  
  (point-value 8)
  (help "The Bospallian is an enormous creature, hunted for its fat and its meat.  Weak attack, moderate defense, high resistance."))

(unit-type mold (name "Devil's Mold") (image-name "ang-ooze-black")
  (acp-per-turn 3) (hp-max 9) (vision-range 1) (cp 6) (naval true)  
  (point-value 5)
  (help "Killer sludge that's slow and almost impossible to kill, except with fire.  Incredible attack, very high defense, very high resistance."))

(unit-type dajawyrm (name "Dajaspec Sea Monster") (image-name "ang-serpent-blue-sea")
  (acp-per-turn 6) (hp-max 120) (vision-range 1) (cp 9) (naval true)  
  (point-value 8)
  (help "One of the many creatures that makes the oceans of Opal nearly impossible to traverse.  Incredible attack, moderate defense, medium resistance."))

(unit-type dajaturtle (name "Veld") (image-name "ang-turtle-gold-brown")
  (acp-per-turn 6) (hp-max 80) (vision-range 2) (cp 9) (naval true)  
  (point-value 8)
  (help "The Veld lives at the bottom of the ocean, coming to the surface for air every hundred years or so.  It will often grow weary of the water and come to land, where it seems to consider the life of mammals to be a personal insult.  Powerful attack, very high defense, very high resistance."))

(define glass-types (axehead bospallian mold dajawyrm dajaturtle))

;; COMFORT-WHITE

(unit-type squid (name "Giant Damur") (image-name "ang-jellyfish-purple")
  (acp-per-turn 9) (hp-max 75) (vision-range 1) (cp 9)  
  (point-value 4)
  (help "Related to the man of war, it is a deadly and aggressive thing with many attacks.  Strong attack, low defense, low resistance."))

(unit-type thucorpse (name "Festering Remains of Thurastes") (image-name "ang-coins-small-gold")
  (acp-independent true) (hp-max 1) (vision-range 0)
  (point-value 10)
  (help "To kill Thurastes would require a great navy, or terrible magic, or help from dragonkind, but if it was done, old Thurastes' corpse would provide 500 gold worth of materials."))

(unit-type thurastes (name "Thurastes") (image-name "monsters-yabba")
  (acp-per-turn 3) (hp-max 500) (wrecked-type thucorpse) (vision-range 12) 
  (can-be-self true) (self-resurrects true)
  (point-value 20)
  (help "Thurastes rules the White Sea from a deep crevice.  He is of terrible bulk and malevolent intelligence and has existed for thousands of years.  He inflicts himself on Opal with great, miles-long tentacles--to randomly destroy and bring back food for his always-hungry maw.  Incredible attack, moderate defense, very high resistance."))

(unit-type tentacle (name "Tentacle of Thurastes") (image-name "spec-tent2")
  (acp-per-turn 6) (hp-max 40) (vision-range 1) (cp 15)  
  (point-value 5)
  (help "The tip of one of Thurastes' massive tentacles.  It can make one powerful attack every turn.  Powerful attack, moderate defense, medium resistance."))

(unit-type ulitar (name "Ulitar") (image-name "ang-alien-blue-bighead")
  (acp-per-turn 6) (hp-max 70) (vision-range 1) (cp 6)  
  (point-value 4)
  (help "A giant from the ocean, with little intelligence, that is sometimes infected with a strange, hypnotic mold.  Strong attack, powerful defense, high resistance."))

(unit-type sludge (name "Strange Mold") (image-name "ang-ooze-green")
  (acp-per-turn 3) (hp-max 1) (vision-range 2) (cp 9)  
  (point-value 3)
  (help "The strange hypnotic mold that ensnares minds.  Powerful attack, no defense, high resistance."))

(define comfort-types (squid thurastes tentacle ulitar sludge))



;; Kraken



(unit-type slaver (name "Slaver") (image-name "monsters-octopus")
  (acp-per-turn 6) (hp-max 60) (vision-range 2) (cp 6)  (can-be-self true) (self-resurrects true) 
  (point-value 6)
  (help "A Kraken that has focused on collecting and mastering lesser species.  Strong attack, moderate defense, medium resistance."))

(unit-type sorcerer (name "Sorcerer") (image-name "monsters-octopus")
  (acp-per-turn 6) (hp-max 40) (vision-range 2) (acp-to-fire 1) (range 3) (cp 9) 
  (can-be-self true) (self-resurrects true)
  (point-value 6)
  (help "A Kraken that has focused on learning magic as its way of mastering lesser species.  Strong attack, moderate defense, medium resistance."))

(define kraken-types (slaver sorcerer))

;; DRAGONKIND

(unit-type gwyrm (name "Great Wyrm") (image-name ("ang-dragon-wyrm-red" "ang-dragon-wyrm-blue" "ang-dragon-wyrm-white" "ang-dragon-wyrm-gray" "ang-dragon-wyrm-green" "ang-dragon-wyrm-bronze" "ang-dragon-wyrm-black"))
  (acp-per-turn 8) (hp-max 250) (vision-range 2) (air true)
;; (acp-to-fire 1) (range 4) (range-min 2)
  (can-be-self true) (self-resurrects true)
  (point-value 15)
  (help "The most powerful dragon.  Incredible attack, very high defense, very high resistance."))

(unit-type wyrm (name "Wyrm") (image-name ("ang-dragon-mature-red" "ang-dragon-mature-bronze" "ang-dragon-mature-gray" "ang-dragon-mature-green" "ang-dragon-mature-white" "ang-dragon-mature-gold" "ang-dragon-mature-blue"))
  (acp-per-turn 8) (hp-max 100) (vision-range 2) (air true) (can-be-self true) (self-resurrects true)
  (auto-upgrade-to gwyrm)
;; (acp-to-fire 1) (range 4) (range-min 2)
  (point-value 9)
  (help "A mature dragon, very dangerous.  Powerful attack, high defense, high resistance."))

(unit-type hatchling (name "Young Dragon") (image-name ("ang-dragon-young-red" "ang-dragon-young-blue" "ang-dragon-young-gold" "ang-dragon-young-white" "ang-dragon-young-green" "ang-dragon-young-gray" "ang-dragon-young-bronze"))
  (acp-per-turn 6) (hp-max 75) (vision-range 2) (air true) (can-be-self true) (self-resurrects true)
  (auto-upgrade-to wyrm)
;; (acp-to-fire 1) (range 4) (range-min 2)
  (point-value 5)
  (help "Still dangerous.  Powerful attack, moderate defense, moderate resistance."))

(unit-type dragonegg (name "Dragon Egg") (image-name "ang-egg-easter")
  (acp-per-turn 0) (hp-max 10) (vision-range 0) (cp 6)
  (auto-upgrade-to hatchling)
  (point-value 10)
  (help "Very valuable, will grow into a baby dragon."))


(define dragon-types (gwyrm wyrm hatchling))


;;; NOTABLES

(unit-type king (name "King") (image-name "ang-king-purple-glowhand")
  (acp-per-turn 6) (hp-max 80) (vision-range 4) (type-in-game-max 1) (possible-sides (or "loyal" (or "rebel")))
  (can-be-self true) (self-resurrects true)
  (point-value 10)
  (help "The king of Talicedric, 'the Aristocracy of Men'.  A spellcaster and powerful fighter.  Powerful attack, high defense, high resistance."))

(unit-type usurper (name "Usurper") (image-name "ang-king-glowhand-staff")
  (acp-per-turn 6) (hp-max 80) (vision-range 4) (type-in-game-max 1) (possible-sides (or "loyal" (or "rebel")))
  (can-be-self true) (self-resurrects true)
  (point-value 10)
  (help "The rebellious baron who, with the support of the Irikani, has managed to split the Aristocracy in half.  Also a powerful spellcaster and fighter.  Powerful attack, high defense, high resistance."))

(unit-type minister (name "Minister") (image-name "ang-mage-hippie-glowhand")
  (acp-per-turn 6) (hp-max 80) (vision-range 4) (type-in-game-max 1) (possible-sides "confed")
  (can-be-self true) (self-resurrects true)
  (point-value 10)
  (help "The high counciller of Talijanna and one of the few people able to keep its tenuous confederacy united.  Were he to die, it is likely the Confederacy of Men would slip into chaos.  Powerful attack, high defense, high resistance."))

(unit-type shogun (name "Shogun") (image-name "ang-lord-blue-cape")
  (acp-per-turn 6) (hp-max 80) (vision-range 4) (type-in-game-max 1) (possible-sides "shogunate")
  (can-be-self true) (self-resurrects true)
  (point-value 10)
  (help "The ruler of the Shogunate, resident of the Imperial Palace and most ardent supporter of the Oni.  Powerful attack, high defense, high resistance."))

(unit-type loremaster (name "Loremaster") (image-name "ang-drow-purple-glowhand")
  (acp-per-turn 6) (hp-max 80) (vision-range 4) (acp-to-fire 1) (range 2)  (type-in-game-max 1)
  (possible-sides "oni")  (can-be-self true) (self-resurrects true)
  (point-value 10)
  (help "The priest-king of the Oni Fold.  A magician with a powerful ranged attack.  Powerful attack, high defense, high resistance."))

(unit-type emperor (name "Emperor") (image-name "ang-king-red-orc")
  (acp-per-turn 6) (hp-max 80) (vision-range 4) (type-in-game-max 1)
  (possible-sides "empire")  (can-be-self true) (self-resurrects true)
  (point-value 10)
  (help "The emperor of the Iriken.  Powerful attack, high defense, high resistance."))

(unit-type shah (name "Shah") (image-name "ang-mage-white-glowhand2")
  (acp-per-turn 6) (hp-max 80) (vision-range 4) (type-in-game-max 1)
  (possible-sides "bedouin")  (can-be-self true) (self-resurrects true)
  (point-value 10)
  (help "The uniter of the Bedouin and lord of the desert.  Powerful attack, high defense, high resistance."))

(unit-type pharoah (name "Pharoah") (image-name "ang-dwarf-ankh-staff")
  (acp-per-turn 6) (hp-max 80) (vision-range 4) (type-in-game-max 1)
  (possible-sides "dwarf")  (can-be-self true) (self-resurrects true)
  (point-value 10)
  (help "The pharoah of the City-States and sworn enemy of both Oni and Iriken.  Powerful attack, high defense, high resistance."))

(unit-type greyone (name "Grey One") (image-name "ang-king-half-skeleton")
  (acp-per-turn 6) (hp-max 80) (vision-range 4) (type-in-game-max 1)
  (possible-sides "undead")  (can-be-self true) (self-resurrects true)
  (point-value 10)
  (help "The mysterious lord of the Undead, at whose hands living men are turned into the undead.  Powerful attack, high defense, high resistance."))

(unit-type talon (name "Talon") (image-name "ang-saurial-crown-staff")
  (acp-per-turn 6) (hp-max 80) (vision-range 4) (type-in-game-max 1)
  (possible-sides "lizard")  (can-be-self true) (self-resurrects true)
  (point-value 10)
  (help "The ruler of the Lizardmen.  Powerful attack, high defense, high resistance."))

(unit-type spawn (name "Spawn") (image-name "monsters-octopus")
  (acp-per-turn 6) (hp-max 80) (vision-range 4) (acp-to-fire 1) (range 2)  (type-in-game-max 1)
  (possible-sides "kraken")  (can-be-self true) (self-resurrects true)
  (point-value 10)
  (help "The bastard of Thurastes and the first Kraken.  Powerful attack, high defense, high resistance."))

(unit-type wocke (name "Jabberwocke") (image-name "ang-man-fang-stone")
  (acp-per-turn 6) (hp-max 80) (vision-range 4) (type-in-game-max 1)
  (possible-sides "wild") (can-be-self true) (self-resurrects true)
  (point-value 10)
  (help "The king of the old forest.  Powerful attack, high defense, high resistance."))



;; CITIES

(unit-type lcity (name "Lizardman City") (image-name "ba-swamp")
  (acp-per-turn 3) (hp-max 50) (wrecked-type cityruin) (vision-range 2)
  (ai-war-garrison 2) (ai-peace-garrison 0) (wrecked-type cityruin)
  (point-value 8)
  (help "Produces lizardman units."))

(unit-type dcity (name "Dwarf City") (image-name "ancient-ziggurat")
  (acp-per-turn 3) (hp-max 50) (wrecked-type cityruin) (vision-range 2)
  (ai-war-garrison 2) (ai-peace-garrison 0) (wrecked-type cityruin)
  (point-value 8)
  (help "Produces dwarf units."))

(unit-type icity (name "Orc City") (image-name "ancient-burrow")
  (acp-per-turn 3) (hp-max 50) (wrecked-type cityruin) (vision-range 2)
  (ai-war-garrison 2) (ai-peace-garrison 0) (wrecked-type cityruin)
  (point-value 8)
  (help "Produces Orc units."))

(unit-type ocity (name "Drow City") (image-name "ancient-black-castle")
  (acp-per-turn 3) (hp-max 50) (vision-range 3) (wrecked-type cityruin)
  (ai-war-garrison 2) (ai-peace-garrison 0)
  (point-value 10)
  (help "Produces drow units."))

(unit-type city1 (name "Small City") (image-name "ancient-small-city")
  (acp-per-turn 3) (hp-max 50) (wrecked-type cityruin) (vision-range 2)
  (ai-war-garrison 1) (ai-peace-garrison 0) (wrecked-type cityruin)
  (point-value 8)
  (help "Small city."))

(unit-type city2 (name "City") (image-name "ancient-city")
  (acp-per-turn 3) (hp-max 50) (wrecked-type city1) (vision-range 3) 
  (ai-war-garrison 1) (ai-peace-garrison 0) (wrecked-type cityruin)
  (point-value 12)
  (help "City."))

(unit-type city3 (name "Large City") (image-name "ancient-blue-city")
  (acp-per-turn 3) (hp-max 50) (wrecked-type city2) (vision-range 3)
  (ai-war-garrison 1) (ai-peace-garrison 0) (wrecked-type cityruin)
  (point-value 15)
  (help "Large city."))

(unit-type city4 (name "Huge City") (image-name "ancient-yellow-castle")
  (acp-per-turn 3) (hp-max 50) (wrecked-type city3) (vision-range 3)
  (ai-war-garrison 2) (ai-peace-garrison 0) (wrecked-type cityruin)
  (point-value 18)
  (help "Huge city."))

(unit-type wcity1 (name "Walled Small City") (image-name "ancient-small-city")
  (acp-per-turn 3) (hp-max 150) (wrecked-type city1) (vision-range 2)
  (ai-war-garrison 1) (ai-peace-garrison 0) (wrecked-type cityruin)
  (point-value 8)
  (help "Small city, with a wall."))

(unit-type wcity2 (name "Walled City") (image-name "ancient-city")
  (acp-per-turn 3) (hp-max 150) (wrecked-type city2) (vision-range 3)
  (ai-war-garrison 1) (ai-peace-garrison 0) (wrecked-type cityruin)
  (point-value 12)
  (help "City, with a wall."))

(unit-type wcity3 (name "Walled Large City") (image-name "ancient-blue-city")
  (acp-per-turn 3) (hp-max 150) (wrecked-type city3) (vision-range 3) (wrecked-type cityruin)
  (ai-war-garrison 1) (ai-peace-garrison 0)
  (point-value 15)
  (help "Large city, with a wall."))

(unit-type wcity4 (name "Walled Huge City") (image-name "ancient-black-castle")
  (acp-per-turn 3) (hp-max 150) (vision-range 3) (wrecked-type cityruin)
  (ai-war-garrison 3) (ai-peace-garrison 0)
  (point-value 18)
  (help "Huge city, with a wall."))

(unit-type town (name "Township") (image-name "ancient-village2")
  (acp-per-turn 3) (hp-max 50) (wrecked-type villageruin) (vision-range 2)
  (ai-war-garrison 0)
  (point-value 5)
  (help "A town."))

(unit-type svillage (name "Seaside Village") (image-name "ancient-village1")
  (acp-per-turn 3) (hp-max 50) (wrecked-type villageruin) (vision-range 3)
  (ai-war-garrison 0)
  (point-value 4)
  (help "A town."))

(unit-type ohamlet (name "Orcspawn Hamlet") (image-name "spec-hamlet")
  (acp-per-turn 3) (hp-max 50) (wrecked-type villageruin) (vision-range 2)
  (ai-war-garrison 0)
  (point-value 4)
  (help "A little hamlet that produces wimps."))

(unit-type ovillage (name "Ogre Village") (image-name "spec-ogvillage")
  (acp-per-turn 3) (hp-max 50) (wrecked-type villageruin) (vision-range 2)
  (ai-war-garrison 0) (revolt-chance 500)
  (point-value 4)
  (help "A big village that produces thugs."))

(unit-type deadcity (name "Dead City") (image-name "ancient-ruins")
  (acp-per-turn 3) (hp-max 100) (vision-range 6) (wrecked-type cityruin) 
  (ai-peace-garrison 0) (ai-war-garrison 0)
  (point-value 10)
  (help "It's haunted.  Can only be captured by undead units."))

(unit-type lab (name "Arcane Laboratory") (image-name "ancient-white-castle")
  (acp-per-turn 3) (hp-max 100) (vision-range 6)
  (ai-peace-garrison 0) (ai-war-garrison 1)
  (point-value 10)
  (help "For building golems."))

(define place-types (wcity4 city4 wcity3 city3 wcity2 city2 wcity1 city1 town svillage ohamlet ovillage deadcity lab
   lcity dcity ocity icity))

;; UNDERSEA CITIES

(unit-type gibcity (name "Gibambi City") (image-name "spec-city-bubble1")
  (acp-per-turn 3) (hp-max 100) (vision-range 6)
  (ai-peace-garrison 0) (ai-war-garrison 1)
  (point-value 10)
  (help "A Gibambi city."))

(unit-type churcity (name "Churambi City") (image-name "spec-city-bubble2")
  (acp-per-turn 3) (hp-max 100) (vision-range 6)
  (ai-peace-garrison 0) (ai-war-garrison 1)
  (point-value 10)
  (help "A Churambi city."))


;; NODES

(unit-type enode (name "Earth Node") (image-name "ang-hole-water-green")
  (acp-per-turn 3) (hp-max 1) (vision-range 2)
  (point-value 10)
  (help "Produces Earth Mana."))

(unit-type snode (name "Sorcery Node") (image-name "ang-hole-water-blue")
  (acp-per-turn 3) (hp-max 1) (vision-range 2)
  (point-value 10)
  (help "Produces Sorcery Mana."))

(unit-type cnode (name "Chaos Node") (image-name "ang-hole-fire")
  (acp-per-turn 3) (hp-max 1) (vision-range 2)
  (point-value 10)
  (help "Produces Chaos Mana."))

(unit-type lnode (name "Life Node") (image-name "ang-hole-spike-lblue")
  (acp-per-turn 3) (hp-max 1) (vision-range 2)
  (point-value 10)
  (help "Produces Life Mana."))

(unit-type dnode (name "Death Node") (image-name "ang-hole-large-black")
  (acp-per-turn 3) (hp-max 1) (vision-range 2)
  (point-value 10)
  (help "Produces Death Mana."))

(unit-type temple (name "Ancient Temple") (image-name "ancient-green-city")
  (acp-per-turn 0) (hp-max 1) (vision-range 2)
  (point-value 12)
  (help "Provides research"))

(define node-types (enode snode cnode lnode dnode temple))



(unit-type fish (name "Fish") (image-name "spec-heart")
  (acp-per-turn 0) (hp-max 1) (vision-range 0)
  (point-value 6)
  (help "Produces gold and anvils."))

(unit-type goldmine (name "Gold Mine") (image-name "spec-mine-green")
  (acp-per-turn 0) (hp-max 1) (vision-range 0)
  (point-value 6)
  (help "Produces gold."))

(unit-type ironmine (name "Iron Mine") (image-name "spec-mine-gray")
  (acp-per-turn 0) (hp-max 1) (vision-range 0)
  (point-value 6)
  (help "Produces iron."))

(unit-type quarry (name "Quarry") (image-name "spec-mine-brown")
  (acp-per-turn 0) (hp-max 1) (vision-range 0)
  (point-value 6)
  (help "Produces stone."))

(unit-type lumber (name "Groves") (image-name "spec-tree-brown")
  (acp-per-turn 0) (hp-max 1) (vision-range 0)
  (point-value 6)
  (help "Produces wood."))

(define resource-types (fish goldmine ironmine quarry lumber))


;; POINTS OF INTEREST

(unit-type lair (name "Dragon's Lair") (image-name "spec-lair")
  (acp-per-turn 3) (hp-max 50) (wrecked-type goldmine) (vision-range 5)
  (possible-sides "dragon")
  (ai-peace-garrison 0) (ai-war-garrison 1)
  (point-value 10)
  (help "Produces dragon eggs."))

(unit-type pit (name "Kraken Pit") (image-name "spec-heart")
  (acp-per-turn 3) (hp-max 50) (wrecked-type fish) (vision-range 10)
  (point-value 10)
  (help "Produces Kraken."))

(unit-type dino1 (name "Dinosaur Breeding Ground") (image-name "spec-heart")
  (acp-per-turn 3) (hp-max 50) (vision-range 3)
  (possible-sides "wild") (can-be-self true) (self-resurrects true)
  (point-value 10)
  (help "Produces dinosaurs.  Cannot be captured."))

(unit-type dino2 (name "Saurian Breeding Ground") (image-name "spec-heart")
  (acp-per-turn 3) (hp-max 50) (vision-range 3)
  (can-be-self true) (self-resurrects true)
  (point-value 10)
  (help "Produces dinosaurs.  Cannot be captured."))


(unit-type fheart (name "Heart of the Forest") (image-name "spec-heart")
  (acp-per-turn 3) (hp-max 50) (vision-range 3)
  (can-be-self true) (self-resurrects true)
  (point-value 10)
  (help "Produces wolves and bears.  Cannot be captured."))

(unit-type jheart (name "Heart of the Jungle") (image-name "spec-heart")
  (acp-per-turn 3) (hp-max 50) (vision-range 3)
  (can-be-self true) (self-resurrects true)
  (point-value 10)
  (help "Produces Ferak and Royal Ferak.  Cannot be captured."))

(unit-type sheart (name "Heart of the Sulspec") (image-name "spec-heart")
  (acp-per-turn 3) (hp-max 50) (vision-range 3)
  (possible-sides "white")  (can-be-self true) (self-resurrects true)
  (point-value 10)
  (help "Produces Giant Squid and Gibambi.  Cannot be captured."))

(unit-type gheart (name "Heart of the Glaspec") (image-name "spec-heart")
  (acp-per-turn 3) (hp-max 50) (vision-range 3)
  (possible-sides "glass") (can-be-self true) (self-resurrects true)
  (point-value 10)
  (help "Produces Bospallian and Axehead.  Cannot be captured."))

(unit-type hheart (name "Heart of the Hoespec") (image-name "spec-heart")
  (acp-per-turn 3) (hp-max 50) (vision-range 3)
  (can-be-self true) (self-resurrects true)
  (point-value 10)
  (help "Produces Churambi and Ulitars.  Cannot be captured."))

(unit-type dheart (name "Heart of the Dajaspec") (image-name "spec-heart")
  (acp-per-turn 3) (hp-max 50) (vision-range 3)
  (can-be-self true) (self-resurrects true)
  (point-value 10)
  (help "Produces Daja Turtles and Daja Wyrms.  Cannot be captured."))

(define wilderness-types (lair pit dino1 dino2 fheart jheart sheart gheart hheart dheart))

(unit-type needle (name "The Needle") (image-name "spec-needle")
  (acp-per-turn 0) (hp-max 1) (vision-range 3) (type-in-game-max 1)
  (help "Opal is known as the pincushion of the gods."))

;; NEW UNITS

(unit-type oslt (name "Orcspawn Lieutenant") (image-name "ang-goblin-shield")
  (acp-per-turn 6) (hp-max 20) (vision-range 1) (cp 6)
  (point-value 2) (cxp-max 2000)
  (help "An Orcspawn leader, they tend to be short-lived. Strong attack, low defense, moderate resistance."))

(unit-type oschief (name "Orcspawn Chief") (image-name "ang-goblin-cape")
  (acp-per-turn 6) (hp-max 35) (vision-range 2) (cp 6)
  (point-value 4) (cxp-max 2000)
  (help "An Orcspawn Chieftain, very rare.  Strong attack, moderate defense, high resistance."))

(unit-type oswarlord (name "Orcspawn Warlord") (image-name "ang-goblin-staff")
  (acp-per-turn 6) (hp-max 60) (vision-range 3) (cp 6)
  (point-value 5) (cxp-max 2000)
  (help "An Orcspawn Warlord.  There have been only half a dozen in all history.  Powerful attack, high defense, high resistance."))

(add spear auto-upgrade-to oslt)
(add oslt auto-upgrade-to oschief)
(add oschief auto-upgrade-to oswarlord)

(unit-type gibambi (name "Gibambi Raider") (image-name "ang-fishhead-sword")
  (acp-per-turn 6) (hp-max 15) (vision-range 1) (cp 6)
  (point-value 1) (cxp-max 2000)
  (help "Weak creatures that live in the ocean and raid the land.  Weak attack, low defense, no resistance."))

(unit-type gibsword (name "Gibambi Warrior") (image-name "ang-fishman-plate-armor")
  (acp-per-turn 6) (hp-max 15) (vision-range 1) (cp 6)
  (point-value 1) (cxp-max 2000)
  (help "Slightly stronger Gibambi."))

(unit-type gibmage (name "Gibambi Mage") (image-name "ang-fishhead-glowhand")
  (acp-per-turn 6) (hp-max 15) (vision-range 1) (cp 6)
  (point-value 1) (cxp-max 2000)
  (help "A Gibambi magician."))

(unit-type gibpriest (name "Gibambi Priest") (image-name "ang-fishhead-ankh")
  (acp-per-turn 6) (hp-max 15) (vision-range 1) (cp 6)
  (point-value 1) (cxp-max 2000)
  (help "A Gibambi holy man."))

(unit-type gibhero (name "Gibambi Hero") (image-name "ang-fishman-skull-shield")
  (acp-per-turn 6) (hp-max 15) (vision-range 1) (cp 6)
  (point-value 1) (cxp-max 2000) (auto-upgrade-to epic)
  (help "A Gibambi hero."))

(add (gibambi gibsword gibmage gibpriest) auto-upgrade-to gibhero)

(unit-type churambi (name "Churambi Swordsmen") (image-name "ang-codhead-sword2")
  (acp-per-turn 6) (hp-max 20) (vision-range 1) (cp 6)  
  (point-value 2)
  (help "Cousins of the Gibambi and slightly tougher.  Strong attack, moderate defense, medium resistance."))

(unit-type churhal (name "Churambi Skirmisher") (image-name "ang-codhead-sword")
  (acp-per-turn 6) (hp-max 20) (vision-range 1) (cp 6)  
  (point-value 2)
  (help "A stronger Churambi."))

(unit-type churcleric (name "Churambi Cleric") (image-name "ang-codhead-staff")
  (acp-per-turn 6) (hp-max 20) (vision-range 1) (cp 6)  
  (point-value 2)
  (help "A Churambi holy man."))

(unit-type churhero (name "Churambi Hero") (image-name "ang-codhead-staff2")
  (acp-per-turn 6) (hp-max 20) (vision-range 1) (cp 6)  
  (point-value 2)
  (help "A Churambi Hero."))

(unit-type vandrak (name "Vandrak") (image-name "ang-monster-who-knows")
  (acp-per-turn 6) (hp-max 15) (vision-range 1) (cp 6)
  (point-value 1)  
  (help "A giant lizard."))


;; SPELLS

(unit-type powerword (name "Power Word, Kill") (image-name "ang-fireball-gray-red")
  	(acp-per-turn 10) (hp-max 1) (acp-to-fire 1) (range 5)  (cp 3)
  (point-value 10)
	(help "Fifth-level Death Magic.  Ten devastating attacks.  Annihilates whole armies."))

(unit-type deathwalk (name "Deathwalker") (image-name "ang-fireball-multi-black")
  (acp-per-turn 3) (hp-max 1) (help "Transport through Hades."))

(unit-type wastemaker (name "Phantom World") (image-name "ang-fireball-gray-silver")
	(acp-per-turn 20) (hp-max 1) (acp-to-detonate 1) (hp-per-detonation 1) (cp 3) 
	(help "A devastating bit of eldritch sorcery, which will imbue a swath of land and its inhabitants with the essence of phantom."))

(unit-type riddle (name "Labrynthine Riddle") (image-name "ang-head-purple-skeleton")
  (acp-independent true) (hp-max 1) 
  (point-value 8) (facility true)

(help "A fortress or city can only be attacked if the riddle is solved, and can only be solved by a hero an archmage or a leader."))

(unit-type breath (name "Gray Breath") (image-name "ang-wraith-gray-flame")
  (acp-per-turn 10) (hp-max 1) (help "Short-lived.  Attack doesn't damage units but saps their ACP."))

(unit-type seance (name "Seance") (image-name "ang-fireball-multi-silver")
  (acp-independent true) (hp-max 1) (vision-range 5) 
(help "Can be created anywhere within 50 hexes and gives a one turn snapshot of the area."))

(unit-type poltergeist (name "Poltergeist") (image-name "ang-dragon-purple-smoke")
  	(acp-per-turn 10) (hp-max 1) 
  (point-value 4)
	(help "Very weak but difficult to see, Useful to take control of resource centers and ruins.  Weak attack, moderate defense, medium resistance, death immunity."))

(unit-type bonearrow (name "Bone Arrows") (image-name "ang-arrow-gray-dur")
	(acp-per-turn 3) (hp-max 1) (acp-to-fire 1) (range 3) (cp 3) 
  (point-value 2)
	(help "Moderate Damage"))

(unit-type deathtap (name "Deathtap") (image-name "ang-fireball-single-silver")
  (point-value 2) (facility true)
  	(acp-independent true) (hp-max 1) (help "Provides death mana"))





(unit-type ghoul (name "Ghouls") (image-name "ang-wight-brown-green")
  (acp-per-turn 6) (hp-max 35) (vision-range 1) (cp 9)
  (point-value 4)
  (help "Mean, their attacks sap ACP.  Strong attack, low defense, low resistance, death immunity."))

(unit-type skeleton (name "Skeletons") (image-name ("ang-skeleton-right-scimitar" "ang-skeleton-left-scimitar2" "ang-skeleton-right-dagger" "ang-skeleton-left-scimitar" "ang-skeleton-shield-sword"))
  (acp-per-turn 6) (hp-max 15) (vision-range 1) (cp 6)
  (point-value 2)
  (help "Brittle.  Weak attack, moderate defense, medium resistance, death immunity."))

(unit-type vampyre (name "Vampyres") (image-name "ang-vampire-blue-gold")
  (acp-per-turn 8) (hp-max 45) (vision-range 2) (cp 9)
  (point-value 6)
  (help "Relatively low HP but regenerates fully each turn.  Transforms units into ghouls, except heroes, which are turned to vampyres.  Sometimes charms the unit it's attacking.  Most powerful undead melee unit.  Land unit but moves like an air unit.  Powerful attack, no defense, high resistance, death immunity."))

(unit-type seeker (name "Seekers") (image-name "ang-hand-purple-skeleton") (cp 6)
  (point-value 1)
  (acp-per-turn 8) (hp-max 5) (help "Animated, severed hands.  Strong attack, low defense, low resistance, death immunity."))

(unit-type iblis (name "Iblis") (image-name "ang-tornado-purple-lightning") (cp 24)
  (acp-per-turn 4) (hp-max 150) (revolt-chance 1000)
;; (has-opinions true) (opinions -3 -3 -3 -3 -3 -3 -3 -3 -3 -3 -3 -3 -3 -3 -3)
  (point-value 9)
(help "A great and powerful djinn.  Learn from Ad Avis.  Incredible attack, high defense, full resistance."))

(unit-type lich (name "Lich") (image-name "ang-lich-glowhand-cape") (cp 12)
  (acp-per-turn 6) (hp-max 50) (acp-to-fire 1) (range 3)
  (point-value 7)
  (help "Undead magician.  Powerful attack, moderate defense, very high resistance, powerful ranged attack."))

(define death-summon-types (ghoul skeleton vampyre seeker lich iblis))


(unit-type naturevoice (name "Nature's Voice") (image-name "ang-fireball-yellow-green")
  (acp-independent true) (hp-max 1) (vision-range 100) (cp 6)
  (point-value 10)
	(help "Knowledge of the entire world"))

(unit-type tsunami (name "Tsunami") (image-name "ang-thunderball-blue-blue")
	(acp-per-turn 51) (hp-max 1) (acp-to-detonate 1) (hp-per-detonation 1) (cp 6) 
	(help "Monster wave useful for levelling coastal cities."))

(unit-type mountainchange (name "Mountain") (image-name "ang-thunderball-green-only")
	(acp-per-turn 48) (hp-max 1) (acp-to-detonate 1) (hp-per-detonation 1) (cp 6) 
	(help "Change terrain into mountain"))

(unit-type beguile (name "Beguile") (image-name "ang-symbol-green")
	(acp-per-turn 25) (hp-max 1) 
	(help "Charm many things"))

(unit-type earthquake (name "Earthquake") (image-name "ang-lightning-orange-ddr")
	(acp-per-turn 21) (hp-max 1) (acp-to-detonate 1) (hp-per-detonation 1) (cp 6) 
	(help "Damages units and cities in a wide area"))

(unit-type deepchange (name "Deep") (image-name "ang-thunderball-green-only")
	(acp-per-turn 48) (hp-max 1) (acp-to-detonate 1) (hp-per-detonation 1) (cp 6) 
	(help "Transform a swamp or shallow sea into deep sea."))

(unit-type shallowchange (name "Shallow") (image-name "ang-thunderball-green-only")
	(acp-per-turn 48) (hp-max 1) (acp-to-detonate 1) (hp-per-detonation 1) (cp 6) 
	(help "Changes deep water to shallow."))

(unit-type charmfish (name "Oceanic Charm") (image-name "ang-fireball-single-purple")
  	(acp-per-turn 25) (hp-max 1) (cp 3) 
	(help "Take control of sea creatures"))

(unit-type hillchange (name "Hill") (image-name "ang-thunderball-green-only")
	(acp-per-turn 48) (hp-max 1) (acp-to-detonate 1) (hp-per-detonation 1) (cp 3) 
	(help "Change most terrain to hills."))

(unit-type plainchange (name "Plains") (image-name "ang-thunderball-green-only")
	(acp-per-turn 48) (hp-max 1) (acp-to-detonate 1) (hp-per-detonation 1) (cp 3)  
	(help "Change most terrain to plains."))

(unit-type swampchange (name "Swamp") (image-name "ang-thunderball-green-only")
	(acp-per-turn 48) (hp-max 1) (acp-to-detonate 1) (hp-per-detonation 1) (cp 3) 
	(help "Change most terrain to swamp."))

(unit-type charmbeast (name "Charm Beast") (image-name "ang-powerball-big-green")
	(acp-per-turn 25) (range 10) (cp 3) 
	(help "Take control of beasts and birds"))



;; Nature Creatures
(unit-type earth1 (name "Small Earth Elemental") (image-name "ang-planer-earth")
	(acp-per-turn 6) (hp-max 20) (vision-range 1) (cp 6)
  (point-value 3)
	(help "A small and weak elemental essence.  Weak attack, low defense, no resistance, nature immunity."))

(unit-type earth2 (name "Large Earth Elemental") (image-name "ang-ent-brown")
  	(acp-per-turn 6) (hp-max 40) (vision-range 1) (cp 9)
  (point-value 4)
  	(help "Big man of dirt.  Strong attack, moderate defense, low resistance, nature immunity."))

(unit-type earth3 (name " Huge Earth Elemental") (image-name "ang-elemental-humanoid-basalt")
  	(acp-per-turn 4) (hp-max 80) (vision-range 1) (cp 12)
  (point-value 5)
  	(help "Huge man of stone.  Powerful attack, high defense, medium resistance, nature immunity."))

(unit-type crebain (name "Crebain") (image-name "monsters-bats")
  	(acp-per-turn 8) (hp-max 3)  (vision-range 3) (cp 3)
  (point-value 1)
	(help "Spies.  Weak attack, low defense, low resistance."))

(unit-type leviathan (name "Leviathan") (image-name "spec-fish-levi")
	(acp-per-turn 6) (hp-max 300) (cp 18)
  (point-value 8)
	(help "Enormous, legendary sea creature.  Incredible attack, moderate defense, medium resistance, nature immunity."))

(define nature-types (earth1 earth2 earth3 crebain leviathan))



(unit-type canticle (name "Chaos Canticle") (image-name "ang-symbol-red")
	(acp-per-turn 1) (hp-max 1)  (acp-to-fire 1) (range 80) (cp 12)
  (point-value 10) (ai-tactical-range 80)
	(help "One shot per turn with an eighty hex range."))

(unit-type volcano (name "Volcano") (image-name "spec-volcano-flow")
	(acp-per-turn 1) (hp-max 1) (acp-to-detonate 1) (hp-per-detonation 0) (cp 12)
  (point-value 10)
	(help "Explode each turn to cause damage and wasteland"))

(unit-type meteor (name "Meteor Storm") (image-name "ang-fireball-single-red")
	(acp-per-turn 51) (hp-max 1) (acp-to-detonate 1) (hp-per-detonation 1) (cp 3) 
  (point-value 10)
	(help "Causes enormous damage to all units nearby when it explodes."))

(unit-type disintegrate (name "Disintegrate") (image-name "ang-lightning-green-ddr")
	(acp-per-turn 1) (hp-max 1) (acp-to-fire 1) (range 30) (cp 3) 
  (point-value 8)
	(help "Destroys any non-leader, damages any place"))

(unit-type flamewall (name "Wall of Flames") (image-name "ang-fireball-multi-orange")
	(acp-per-turn 10) (hp-max 40) (cp 3)
  (point-value 8)
	(help "Currently just protective, especially against water creatures (which cause only 1hp of damage).  Eventually will also damage attackers."))

(unit-type fireball (name "Fireball") (image-name "ang-fireball-single-orange")
	(acp-per-turn 21) (hp-max 1) (acp-to-detonate 1) (hp-per-detonation 1) (cp 3)
  (point-value 6) (facility true)
	(help "Causes some damage to all units nearby when it explodes."))

(unit-type flamestrike (name "Flame Strike") (image-name "ang-fireball-single-yellow")
	(acp-per-turn 1) (hp-max 1) (acp-to-fire 1) (range 10) (cp 3) 
  (point-value 4)
	(help "One attack for 3d10+10."))

(unit-type flamearrow (name "Flame Arrow") (image-name "ang-arrow-red-dur")
	(acp-per-turn 5) (hp-max 1) (acp-to-fire 1) (range 3) (cp 3) 
  (point-value 2)
	(help "Five attacks for 1d10."))


;; Chaos Creatures
(unit-type fire1 (name "Minor Fire Elemental") (image-name "ang-planer-fire")
	(acp-per-turn 6) (hp-max 15) (vision-range 1) (cp 6)
  (point-value 3)
	(help "Flame.  Weak attack, low defense, no resistance, chaos immunity."))

(unit-type fire2 (name "Fire Elemental") (image-name "ang-tornado-gold-flame")
	(acp-per-turn 6) (hp-max 30) (vision-range 1) (cp 9)
  (point-value 4)
	(help "Sparks.  Strong attack, moderate defense, low resistance, chaos immunity."))

(unit-type fire3 (name "Greater Fire Elemental") (image-name "ang-elemental-humanoid-flame")
  	(acp-per-turn 4) (hp-max 60) (vision-range 1) (cp 12)
  (point-value 5)
  	(help "Magma.  Powerful attack, high defense, medium resistance, chaos immunity."))

(unit-type hellhound (name "Gévaudan") (image-name "ang-hellhound-spiked-tail")
  	(acp-per-turn 6) (hp-max 30) (cp 6)
  (point-value 4)
	(help "A large, mean, evil dog.  Weak attack, moderate defense, medium resistance, chaos immunity."))

(unit-type chaoscreature (name "Ordros") (image-name "ang-eye-floating-red")
  	(acp-per-turn 6) (hp-max 100) (cp 24)
  (point-value 8)
	(help "The Ordros is inexplicable and very dangerous.  Incredible attack, low defense, low resistance, chaos immunity."))

(unit-type demon (name "Demon") (image-name ("ang-demon-horn-sword" "ang-demon-buff" "ang-demon-sword" "ang-demon-shield" "ang-demon-glowhand"))
  	(acp-per-turn 6) (hp-max 100) (cp 18) (revolt-chance 500)
  (point-value 8)
	(help "Demons are powerful, but fickle.  Powerful attack, very high defense, full resistance."))

(define chaos-types (fire1 fire2 fire3 hellhound chaoscreature demon))




(unit-type restore (name "Restoration") (image-name "ang-powerball-burst-yellow")
  	(acp-per-turn 48) (hp-max 1) (acp-to-detonate 1) (hp-per-detonation 1) (cp 3)
	(help "Changes wasteland to plains."))

(unit-type enlighten (name "Enlighten") (image-name "ang-symbol-yellow")
	(acp-per-turn 25) (hp-max 1) (cp 3) 
	(help "Take control of any one creature"))

(unit-type doorway (name "Doorway") (image-name "spec-doorway")
	(acp-per-turn 3) (hp-max 50) (cp 6)
  (point-value 8)
	(help "A tower of light that produces spirits"))

(unit-type spiritwall (name "Spirit Wall") (image-name "ang-fireball-silver-bronze")
	(acp-independent true) (hp-max 40) (cp 3)
  (point-value 8) (facility true)
	(help "Placed in empty terrain to block movement or in a city or fortress.  Death units cause only 1hp damage against it."))

(unit-type sunray (name "Sunray") (image-name "ang-fireball-multi-yellow")
	(acp-per-turn 1) (hp-max 1) (acp-to-fire 1) (range 15) (cp 3) 
  (point-value 6)
	(help "Very damaging, especially to evil units"))

(unit-type improveheal (name "Improved Heal") (image-name "ang-lightning-gray-up")
	(acp-per-turn 3) (hp-max 1) (cp 3) (point-value 6)
	(help "Can heal three times for 20HP each."))

(unit-type sanctify (name "Sanctify") (image-name "ang-fireball-multi-bronze")
	(acp-independent true) (hp-max 50) (cp 6)
  (point-value 4) (facility true)
	(help "Used on fortresses to improve their strength and improve their ACP."))

(unit-type bless (name "Bless") (image-name "ang-fireball-single-yellow")
  	(acp-independent true) (hp-max 1) (cp 6)
  (point-value 4) (facility true)
	(help "Provides extra materials and doubles a city's ACP."))

(unit-type justiceeye (name "Eye of Justice") (image-name "ang-fireball-single-yellow")
  	(acp-independent true) (hp-max 1) (vision-range 10) (cp 6)
  (point-value 2)
	(help "Allows a watchtower to see farther"))

(unit-type bolt (name "Bolt") (image-name "ang-fireball-single-yellow")
  	(acp-per-turn 1) (hp-max 1) (acp-to-fire 1) (range 5) (cp 3)  
  (point-value 2)
	(help "Slightly damaging, moreso to evil creatures"))

(unit-type heal (name "Heal") (image-name "ang-lightning-gray-up")
  	(acp-per-turn 1) (hp-max 1) (cp 3) (point-value 2)
	(help "Restore 15HP to a single unit"))

;; ANGELS TO MARMOTS, WHEN YOU GET THE CHANCE
;; Life Creatures
(unit-type spirit1 (name "Sprites") (image-name "ang-planer-mystic")
  	(acp-per-turn 6) (hp-max 15) (vision-range 1) (cp 6)
  (point-value 3)
  	(help "Pixies.  Weak attack, no defense, low resistance, life immunity."))

(unit-type spirit2 (name "Spirits") (image-name "ang-angel-flame-wing")
  	(acp-per-turn 6) (hp-max 30) (vision-range 1) (cp 9)
  (point-value 4)
  	(help "Cherubrim.  Strong attack, low defense, medium resistance, life immunity."))

(unit-type spirit3 (name "Phoenix") (image-name "ang-phoenix-yellow-gold")
  	(acp-per-turn 6) (hp-max 60) (vision-range 1) (cp 12)
  (point-value 5)
  	(help "A legendary creature.  Powerful attack, moderate defense, high resistance, life immunity.")) 

(unit-type guardian (name "Guardian") (image-name "ang-scorpian-yellow-giant") (cp 6)
  	(acp-per-turn 6) (hp-max 10) 
  (point-value 2)
(help "Supposed to protects forces around it, but not yet.  Weak attack, very high defense, very high resistance, life immunity."))

(define life-types (spirit1 spirit2 spirit3 guardian))



(unit-type castle (name "Phantom Castle") (image-name "spec-phantom-castle")
	(acp-per-turn 3) (hp-max 100) (cp 3)
  (point-value 10)
	(help "Produces phantom creatures and protects troops"))

(unit-type greatcharm (name "Greater Charm") (image-name "ang-thunderball-blue-purple")
  	(acp-per-turn 25) (cp 3)
	(help "Far more effective at enrapturing opposing units and can be used three times."))

(unit-type truedivine (name "True Divination") (image-name "ang-powerball-burst-lblue")
  	(acp-independent true) (hp-max 1) (vision-range 60) (cp 6)
  (point-value 10)
	(help "Eldritch knowledge of Opal"))

(unit-type guardward (name "Guards and Wards") (image-name "ang-powerball-big-purple")
  	(acp-independent true) (hp-max 3) (cp 6)
  (point-value 8) (facility true)
	(help "Protects fortresses, very difficult to defeat.  All units have only a 20% chance to hit and it must be hit three times to be destroyed."))

(unit-type prismwall (name "Prismatic Wall") (image-name "ang-fireball-all-color")
  	(acp-per-turn 10) (hp-max 40) (cp 3)  (point-value 8) (facility true)
	(help "Guards cities and fortresses.  Magic attacks are far less effective against prismatic walls than other targets.  Should be sometimes incredibly damaging to attackers and sometimes beguiling, but it doesn't work right now."))

(unit-type greatdivine (name "Greater Divination") (image-name "ang-powerball-burst-blue")
  	(acp-independent true) (hp-max 1) (vision-range 30) 
  (point-value 6)
	(help "Mystical knowledge of Opal"))

(unit-type lightning (name "Lightning Bolt") (image-name "ang-lightning-gold-ddl")
	(acp-per-turn 1) (hp-max 1) (acp-to-fire 1) (range 4) (cp 3)  
  (point-value 6)
	(help "Damaging"))

(unit-type charm (name "Charm") (image-name "ang-thunderball-blue-yellow")
	(acp-per-turn 25) (hp-max 1) (cp 3) 
	(help "Take control of one unit unless it resists."))

(unit-type divine (name "Divination") (image-name "ang-powerball-big-blue")
	(acp-independent true) (hp-max 1) (vision-range 15) 
  (point-value 2)
	(help "Simple knowledge of Opal"))



;; Sorcery Creatures
(unit-type air1 (name "Minor Air Elemental") (image-name "ang-planer-fog")
	(acp-per-turn 8) (hp-max 10) (vision-range 1) (cp 6)
  (point-value 3)
	(help "Small gust.  Weak attack, no defense, low resistance, sorcery immunity."))

(unit-type air2 (name "Air Elemental") (image-name "ang-tornado-black-regular")
	(acp-per-turn 8) (hp-max 20) (vision-range 1) (cp 9)
  (point-value 4)
	(help "Big whirlwind.  Strong attack, low defense, medium resistance, sorcery immunity."))

(unit-type air3 (name "Greater Air Elemental") (image-name "ang-tornado-turq-flame")
	(acp-per-turn 8) (hp-max 40) (vision-range 1) (cp 12)
  (point-value 5)
	(help "Great Tornado.  Powerful attack, moderate defense, high resistance, sorcery immunity."))

(unit-type nameless (name "Nameless Beast") (image-name "spec-phantom-nameless")
  (acp-per-turn 6) (hp-max 200) (vision-range 2) 
  (point-value 8)
  (help "An enormous, lumbering, partially invisible thing that leaves ruin in its wake.  Incredible attack, no defense, very high resistance, sorcery immunity."))

(unit-type phantomshoggoth (name "Phantom Shoggoth") (image-name "spec-phantom-blob")
  (acp-per-turn 6) (hp-max 150) (vision-range 2) 
  (point-value 8)
  (help "A terrible, amorphous creature of great and fell power.  Incredible attack, no defense, very high resistance, sorcery immunity."))

(unit-type phantompavilion (name "Phantom Pavilion") (image-name "spec-phantom-pavilion")
  (acp-per-turn 3) (hp-max 50) (vision-range 2) (ai-war-garrison 1) (ai-peace-garrison 0)
  (point-value 8)
  (help "A gateway through which phantom creatures may be summoned."))

(unit-type phantomship (name "Phantom Ship") (image-name "spec-phantom-ship")
	(acp-per-turn 6) (hp-max 100) (cp 12) (point-value 6)
	(help "A ghost ship.  Hard to spot and dangerous.  Powerful attack, no defense, high resistance, sorcery immunity."))

(unit-type phantombeast (name "Phantom Beast") (image-name "spec-phantom-beast")
	(acp-per-turn 4) (hp-max 100) (cp 12)
  (point-value 5)
	(help "A powerful, though ephemeral, creature.  Powerful attack, no defense, medium resistance, sorcery immunity."))

(unit-type phantomwarrior (name "Phantom Warriors") (image-name "spec-phantom-warrior")
	(acp-per-turn 6) (hp-max 10) (cp 6)
  (point-value 3)
	(help "Easily killed, but powerful attackers.  Powerful attack, no defense, medium resistance, sorcery immunity."))


(define sorcery-types (air1 air2 air3 phantombeast phantomwarrior))

;;;RANDOM EVENT SEEDS

(unit-type banditseed (name "Hive of scum and villainy") (image-name "shield")
	(acp-per-turn 0) (hp-max 1) (wrecked-type sword)
	(help "Used to generate random events.  Produces an independant swordsman."))

(unit-type pirateseed (name "Notorious pirate waters") (image-name "ang-arrow-red-up")
	(acp-per-turn 0) (hp-max 1) (wrecked-type cutter)
	(help "Used to generate random events.  Produces an independant cutter."))

(unit-type heroseed (name "Rumors of a great fighter") (image-name "ang-coat-sword-unicorn")
	(acp-per-turn 0) (hp-max 1) (wrecked-type hero)
	(help "Used to generate random events.  Produces an independant hero."))

(unit-type rangerseed (name "Rumors of a ranger") (image-name "ang-coat-sword-unicorn")
	(acp-per-turn 0) (hp-max 1) (wrecked-type ranger)
	(help "Used to generate random events.  Produces an independant ranger."))

(unit-type knightseed (name "Rumors of a knight") (image-name "ang-coat-sword-unicorn")
	(acp-per-turn 0) (hp-max 1) (wrecked-type knight)
	(help "Used to generate random events.  Produces an independant knight."))

(unit-type archseed (name "Rumors of a powerful wizard") (image-name "ang-ball-crystal-bronze")
	(acp-per-turn 0) (hp-max 1) (wrecked-type wizard)
	(help "Used to generate random events.  Produces an independant wizard."))

(unit-type orcseed (name "Orc hunting grounds") (image-name "ang-arrow-gray-right")
	(acp-per-turn 0) (hp-max 1) (wrecked-type tribesman)
	(help "Used to generate random events.  Produces an independant tribesman."))

(unit-type fortseed (name "Rumors of a bandit hideout") (image-name "hut")
	(acp-per-turn 0) (hp-max 1) (wrecked-type fort)
	(help "Used to generate random events.  Produces an independant wooden fortress."))

(unit-type dragonseed (name "Rumors of a dragon") (image-name "ang-dragon-pink-smoke")
	(acp-per-turn 0) (hp-max 1) (wrecked-type wyrm)
	(help "Used to generate random events.  Produces an independant Wyrm."))

(unit-type faultline (name "Random Earthquake") (image-name "ang-scroll-door-broke")
	(acp-per-turn 0) (hp-max 1) (hp-per-detonation 0) 
	(help "1% chance per turn of an earthquake here."))


;; STORYLINE STUFF

(unit-type riplegend (name "Eldritch Artifact") (image-name "ang-treasure-varied")
	(acp-per-turn 0) (hp-max 1) (revolt-chance 1000)
        (help "This is some weapon or piece of armor that, if weilded, can make a hero into a 
	legendary hero.  Each hero gets only one chance to take any artifact (By attacking it)."))

(unit-type lantern (name "Magic Lantern") (image-name "ang-tool-lamp-gold")
	(acp-per-turn 12) (hp-max 1)
        (help "Purchase mercenaries and cast all spells."))

;; (add legend wrecked-type riplegend)

(add u* self-resurrects true)


(define seed-types (banditseed heroseed fortseed dragonseed pirateseed orcseed archseed rangerseed knightseed))

(define event-types (faultline))

(define levelone-spells (air1 earth1 spirit1 fire1 skeleton deathtap charmbeast crebain flamestrike flamearrow 
   bolt heal divine phantomwarrior))
(define leveltwo-spells (seance poltergeist bonearrow seeker hillchange plainchange swampchange hellhound
   fireball bless justiceeye spirit2 lightning charm))
(define levelthree-spells (riddle breath ghoul deepchange shallowchange charmfish disintegrate flamewall fire2
   sunray improveheal sanctify prismwall greatdivine air2))
(define levelfour-spells (deathwalk wastemaker vampyre mountainchange beguile earthquake volcano meteor fire3
   spiritwall spirit3 guardian guardward phantombeast air3))
(define levelfive-spells (powerword iblis lich naturevoice leviathan tsunami canticle demon chaoscreature restore
   enlighten doorway castle greatcharm truedivine))

(define spell-types (append levelone-spells leveltwo-spells levelthree-spells levelfour-spells levelfive-spells))

(define wall-types (prismwall spiritwall flamewall))

(define shot-types (powerword bonearrow disintegrate flamestrike flamearrow sunray improveheal 
bolt lightning))

(define charm-types (enlighten greatcharm charm charmbeast charmfish beguile))


(define summon-types (iblis lich poltergeist seeker leviathan crebain demon chaoscreature hellhound guardian 
	phantombeast phantomwarrior air1 air2 air3 earth1 spirit1 spirit2 spirit3 fire1 fire2 fire3 skeleton 
	ghoul vampyre poltergeist))


(define explode-types (wastemaker tsunami earthquake meteor fireball restore mountainchange 
	deepchange shallowchange hillchange plainchange swampchange)
)


(define enchantment-types (riddle deathtap flamewall sanctify bless justiceeye guardward prismwall)
)


(define create-types (castle deathwalk naturevoice canticle volcano doorway spiritwall divine truedivine greatdivine))


(define self-types (king usurper minister shogun loremaster emperor shah pharoah greyone talon spawn wocke))


(define land-types (king usurper minister shogun loremaster emperor shah pharoah greyone wocke sword halberd bow
   cavalry catapult mage scout gladiator longbow crusader assassin cannon dervish weaver earth1
   earth2 earth3 fire1 fire2 fire3 skeleton zombie ghoul spectre spear shaman thug champ tyrannosaur
   apatosaur wolves bears fort tower keep citadel lich canticle chaoscreature doorway spiritwall castle phantombeast
   phantomwarrior seeker hellhound volcano thuvi tribesman iblis demon deadwild
   cityruin villageruin keepruin citadelruin ruins flamewall enode snode cnode lnode dnode temple goldmine
   ironmine quarry lumber lair pit dino1 dino2 fheart jheart prismwall hero legend epic
   wcity4 city4 wcity3 city3 wcity2 city2 wcity1 city1 town svillage ohamlet ovillage deadcity
   deathknight wizard archmage ranger knight woodgolem stonegolem irongolem clockgolem phantompavilion vampyre
   oslt oschief oswarlord nameless phantomshoggoth
   dsword dhalberd dbow dranger dhero depic dlegend
   osword ocavalry obow oranger oknight ohero oepic olegend
   merc
   isword ihalberd gladiator image ibow iranger ihero iepic ilegend iwizard iarchmage

   ))



(define amphibious-types (talon spawn dajaturtle churambi gibambi ulitar slaver sorcerer
    croc lsword lhalberd lcavalry lmage llegend lepic lhero lranger larchmage lwizard))

(define deep-types (axehead bospallian dajawyrm thurastes leviathan axecorpse boscorpse thucorpse))

(define water-types (squid tentacle mold sludge plesiosaur cutter whaler tsunami fish sheart gheart hheart dheart phantomship))

(define air-types (ferak royalferak pterodactyl spirit1 spirit2 spirit3 air1 air2 air3 gwyrm wyrm
   hatchling ghost crebain demon guardian poltergeist iblis))

(define all-types (append land-types amphibious-types deep-types water-types air-types))

(define mage-types (sorcerer mage dervish weaver shaman spectre lich iblis vampyre lmage image))

(define heroic-types (legend archmage wizard hero epic ranger knight
   llegend larchmage lwizard lhero lepic lranger
   dranger dhero depic dlegend
   oranger ohero oknight oepic olegend owizard oarchmage
   iranger ihero iepic ilegend iwizard iarchmage

))

(define high-mage-types (archmage wizard larchmage lwizard oarchmage owizard iarchmage iwizard))

(define phantom-types (phantomwarrior phantombeast castle phantompavilion phantomship nameless phantomshoggoth))

;; TRAITS

(define sorcery-immune-types (phantomwarrior phantombeast phantomship prismwall phantomshoggoth nameless))

(define life-immune-types (spirit1 spirit2 spirit3 guardian spiritwall))

(define chaos-immune-types (canticle flamewall fire1 fire2 fire3 hellhound chaoscreature))

(define death-immune-types (ghoul zombie ghost skeleton spectre seeker poltergeist greyone deathknight vampyre deadcity lich demon))

(define nature-immune-types (leviathan tsunami shaman earth1 earth2 earth3 lmage lwizard larchmage))

(define hunter-trait (scout ranger bow longbow lranger lsword croc dranger oranger iranger gladiator))

(define climber-trait (legend epic hero wizard archmage ranger llegend lepic lhero lwizard larchmage lranger
   dlegend depic dhero dranger
   olegend oepic ohero oknight owizard oarchmage oranger 
   iranger ihero iepic ilegend iwizard iarchmage

))

(define tireless-trait (scout ranger dranger oranger iranger woodgolem stonegolem irongolem clockgolem))

(define swamp-born-trait (lcavalry lbow lsword lhalberd lepic llegend lwizard lranger larchmage lmage croc))

(define jungle-born-trait (tribesman gladiator isword ihalberd image ibow iranger ihero iepic ilegend iwizard iarchmage
))

(define desert-born-trait (dervish))

(define mountain-born-trait (dsword dhalberd dbow dranger dhero depic dlegend cannon earth1 earth2 earth3))

(define air-attack-trait (hero knight vampyre king usurper minister shogun loremaster emperor shah pharoah
   assassin legend epic llegend dlegend lhero dhero lepic depic earth3 fire3 thurastes tentacle nameless
   apatosaur scout oepic olegend ohero oknight iranger ihero iepic ilegend


))

(define mounted-trait (cavalry knight lcavalry ocavalry oknight hellhound

))


(define small-types (churambi gibambi spear shaman ferak wolves pharoah crebain dragonegg mold oslt oschief oswarlord dbow dsword dhalberd dhero dranger depic dlegend))

(define medium-types (sword halberd zombie skeleton hellhound air1 spirit1 fire1 scout croc 
   guardian tribesman crusader royalferak phantomwarrior ghost thuvi vampyre hero wizard
   king usurper minister shogun loremaster emperor shah pharoah greyone talon legend bow longbow
   mage dervish weaver ghoul deathknight epic archmage ranger
   llegend lepic lhero larchmage lwizard lmage lranger lsword lhalberd lbow merc
   olegend oepic ohero oarchmage owizard oranger osword obow
   isword ihalberd gladiator image ibow iranger ihero iepic ilegend iwizard iarchmage

))

(define large-types (cavalry thug pterodactyl bears earth2 air2 spirit2 fire2 ulitar champ demon 
   hatchling sludge knight woodgolem stonegolem irongolem clockgolem lcavalry ocavalry oknight))

(define huge-types (slaver sorcerer plesiosaur squid wyrm phantombeast earth3 air3 spirit3 fire3
   tentacle spawn))

(define gargantuan-types (bospallian apatosaur cutter dajaturtle axehead tyrannosaur chaoscreature
   iblis gwyrm dajawyrm thurastes leviathan phantomship))


(define weak-types (spear sword halberd zombie cavalry gibambi bospallian apatosaur crebain wolves skeleton hellhound earth1 air1 spirit1 fire1 scout ferak croc guardian tribesman dragonegg merc
crebain bow longbow catapult cannon poltergeist woodgolem 
   lsword lbow lcavalry dbow dsword dhalberd osword obow ocavalry ocatapult
   isword ihalberd ibow
))

(define strong-types (crusader gladiator slaver thug pterodactyl plesiosaur bears royalferak churambi earth2 air2 spirit2 fire2 ghoul squid ulitar ghost thuvi cutter mage dervish weaver assassin shaman
spectre seeker sorcerer whaler knight stonegolem oslt oschief
   lhalberd lmage oknight
    image))

(define powerful-types (champ dajaturtle axehead demon vampyre hatchling wyrm sludge phantombeast  earth3 air3 spirit3 fire3 tentacle hero wizard king usurper minister shogun loremaster emperor shah pharoah greyone talon spawn wocke deathknight wizard lich phantomwarrior epic spiritwall flamewall prismwall ranger irongolem phantomship oswarlord
   lhero lranger lepic lwizard dhero dranger depic oepic oranger ohero owizard
    iepic ihero iranger iwizard))

(define incredible-types (tyrannosaur chaoscreature iblis gwyrm dajawyrm mold thurastes legend leviathan
tsunami canticle archmage clockgolem phantomshoggoth nameless
   llegend larchmage dlegend oarchmage olegend
   ilegend iarchmage))


(define no-defense-types (phantomwarrior phantombeast air1 spirit1 catapult cannon ghost mage dervish weaver assassin shaman sludge tsunami canticle vampyre phantomship phantomshoggoth nameless 
   lbow lmage ocatapult
   ibow))

(define low-defense-types (spear zombie gibambi crebain wolves bears earth1 fire1 air2 spirit2 ferak tribesman bow longbow ghoul squid spectre seeker chaoscreature oslt
   lsword lhalberd lcavalry obow))

(define mid-defense-types (sword halberd cavalry churambi bospallian apatosaur skeleton hellhound scout royalferak croc poltergeist gladiator slaver thug pterodactyl plesiosaur earth2 fire2 air3 spirit3 sorcerer champ hatchling tentacle lich spiritwall flamewall prismwall tyrannosaur dajawyrm thurastes leviathan woodgolem clockgolem oschief
   lhero lwizard lranger dbow merc ocavalry osword
   isword ihalberd image
))

(define high-defense-types (crusader ulitar thuvi cutter axehead wyrm earth3 fire3 hero wizard king usurper minister shogun loremaster emperor shah pharoah greyone talon spawn wocke deathknight iblis ranger knight irongolem oswarlord
   larchmage llegend dsword dhalberd oranger oknight ohero owizard
   iranger ihero iwizard))

(define uber-defense-types (dragonegg guardian whaler dajaturtle demon epic gwyrm mold legend archmage stonegolem
   dlegend dhero dranger depic oepic olegend oarchmage
   iepic iarchmage ilegend))


;; RESISTANCE

(define no-resist-types (catapult cannon ghost zombie gibambi earth1 fire1))

(define low-resist-types (sword halberd cavalry churambi spear crebain wolves bears air1 spirit1 earth2 fire2 ferak tribesman bow longbow ghoul squid spectre seeker chaoscreature 
   lsword lhalberd lcavalry lbow merc
   isword ihalberd ibow))

(define mid-resist-types (phantomwarrior phantombeast bospallian apatosaur skeleton hellhound scout royalferak croc poltergeist gladiator slaver thug pterodactyl plesiosaur air2 spirit2 earth3 fire3 sorcerer champ hatchling tentacle spiritwall flamewall prismwall tyrannosaur dajawyrm leviathan woodgolem oslt oschief
   dsword dhalberd dbow ocatapult))

(define high-resist-types (crusader ulitar thuvi cutter axehead wyrm air3 spirit3 hero king usurper minister shogun loremaster emperor shah pharoah greyone talon spawn wocke deathknight ranger knight irongolem vampyre phantomship sludge tsunami canticle mage dervish weaver assassin shaman oswarlord
   lmage lhero lranger osword ocavalry obow
   image iranger ihero
))

(define uber-resist-types (dragonegg guardian whaler dajaturtle gwyrm mold stonegolem thurastes lich phantomshoggoth nameless dhero dranger))

(define full-resist-types (iblis demon thuvi wizard legend archmage epic clockgolem
   lepic llegend lwizard larchmage dlegend depic ohero oranger oknight oepic owizard oarchmage
   iepic ilegend iwizard iarchmage
))

;; DAMAGE - REGULAR

(define zero-damage-types (city1))

(define one-damage-types (scout spear croc))

(define two-damage-types (wolves skeleton squid gibambi hellhound))

(define three-damage-types (sword oslt zombie tribesman cavalry air1 dsword merc osword
   isword))

(define four-damage-types (halberd oschief earth1 spirit1 churambi ulitar
   lsword lcavalry dhalberd ocavalry ihalberd))

(define five-damage-types (gladiator thuvi crusader fire1 ghoul
   lhalberd))

(define six-damage-types (thug slaver pterodactyl oswarlord plesiosaur bears royalferak earth2 air2 spirit2 deathknight ranger knight dranger oranger oknight))

(define seven-damage-types (hero earth3 hatchling champ fire2 dajaturtle air3 spirit3 demon woodgolem
   lranger dhero ohero))

(define eight-damage-types (epic legend fire3 iblis chaoscreature bospallian apatosaur tentacle stonegolem irongolem clockgolem
   lhero depic dlegend oepic olegend
   ihero))

(define nine-damage-types (tyrannosaur axehead wyrm
   lepic llegend
   iepic llegend))

(define ten-damage-types (iblis

))

;; DAMAGE - NO DEFENSE

(define one-nodef-damage-types (sludge))

(define two-nodef-damage-types (guardian crebain seeker ferak))

(define three-nodef-damage-types (fireball))

(define four-nodef-damage-types (ghost))

(define five-nodef-damage-types (king usurper minister shogun loremaster emperor shah pharoah greyone talon spawn wocke))

(define six-nodef-damage-types (vampyre))

(define seven-nodef-damage-types (phantomwarrior))

(define eight-nodef-damage-types (dajawyrm))

(define nine-nodef-damage-types (phantombeast phantomship))

(define ten-nodef-damage-types (phantomshoggoth))

(define eleven-nodef-damage-types (mold nameless))

(define twelve-nodef-damage-types (gwyrm))

(define thirteen-nodef-damage-types (thurastes))

(define humanoid-types (gibambi sword halberd cavalry churambi spear tribesman bow longbow scout croc gladiator champ oslt oschief crusader thuvi hero king usurper minister shogun loremaster emperor shah pharoah ranger knight mage dervish weaver assassin shaman oswarlord thuvi wizard legend archmage epic 
   lsword lhalberd lbow lmage llegend lepic lhero lranger larchmage
   dsword dhalberd dbow dranger dhero depic dlegend merc
   osword obow oranger ohero oepic olegend owizard oarchmage
   isword ibow ihalberd iranger ihero iepic ilegend iwizard iarchmage
))

(define corpse-types (thucorpse boscorpse deadwild axecorpse
))

(define ranged-types (bow longbow catapult cannon cutter whaler ranger 
   lbow lranger
   dbow dranger cannon
   obow oranger ocatapult
   ibow iranger icatapult
))

(define nosave-types (powerword sunray)
)


(define notcapture-types  (catapult cannon assassin tyrannosaur apatosaur pterodactyl plesiosaur wolves   bears ferak royalferak axehead bospallian mold dajawyrm dajaturtle squid thurastes ulitar hatchling
  crebain seeker phantomshoggoth nameless))

(define old-firing-types (bow longbow catapult cannon cutter whaler powerword bonearrow earthquake
   sorcerer mage dervish weaver shaman spectre lich disintegrate flamestrike flamearrow sunray bolt    
   lightning archmage wizard 
   lbow lmage lwizard larchmage
   dbow cannon
   obow ocatapult
   ibow icatapult
))

(define firing-types (append old-firing-types shot-types))



(add place-types advanced true)
(add lantern advanced true)
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
(add (phantompavilion castle) advanced true)
(add (phantompavilion castle) use-own-cell true)

(add resource-types already-seen true)
(add node-types already-seen true)
(add place-types already-seen true)



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
(material-type anvil (name "Iron") (treasury true)
  (help "Production"))
(material-type corpse (name "Corpses")
  (help "Used for evil acts"))
(material-type timer (name "X")
  (help "To keep things alive, like ghosts, which aren't really alive, which is weird."))
(material-type flames (name "Fire")
  (help "Represents a dragon's flames or a cutter or whaler's pitch."))
(material-type opp (name "Chance")
  (help "Represents a hero's one chance to weild an eldritch artifact."))
(material-type charge (name "Charges")
  (help "The amount of shots a high-level mage can fire."))




;;; Advances

;;Limits for units


(advance-type gladiator-l (name "Limiter: Gladiator") (rp 30000))
(advance-type longbow-l (name "Limiter: Longbow") (rp 30000))
(advance-type crusader-l (name "Limiter: Crusader") (rp 30000))
(advance-type assassin-l (name "Limiter: Assassin") (rp 30000))
(advance-type cannon-l (name "Limiter: Cannon") (rp 30000))
(advance-type dervish-l (name "Limiter: Dervish") (rp 30000))
(advance-type weaver-l (name "Limiter: Weaver") (rp 30000))
(advance-type croc-l (name "Limiter: Croc") (rp 30000))
(advance-type hero-l (name "Limiter: Hero") (rp 30000))
(advance-type zombie-l (name "Limiter: Zombie") (rp 30000))
(advance-type spectre-l (name "Limiter: Spectre") (rp 30000))
(advance-type limit-break (name "Big Limit") (rp 30000))


;; Spells

(advance-type s-powerword (name "Power Word, Kill (750)") (rp 750))
(advance-type s-iblis (name "Summon Iblis (750)") (rp 750))
(advance-type s-lich (name "Summon Lich (750)") (rp 750))
(advance-type s-deathwalk (name "Deathwalk (600)") (rp 600))
(advance-type s-wastemaker (name "Wasteland (600)") (rp 600))
(advance-type s-vampyre (name "Summon Vampyre (600)") (rp 600))
(advance-type s-riddle (name "Labrynthine Riddle (450)") (rp 450))
(advance-type s-breath (name "Gray Breath (450)") (rp 450))
(advance-type s-ghoul (name "Summon Ghouls (450)") (rp 450))
(advance-type s-seance (name "Seance (300)") (rp 300))
(advance-type s-poltergeist (name "Poltergeist (300)") (rp 300))
(advance-type s-bonearrow (name "Bone Arrows (300)") (rp 300))
(advance-type s-seeker (name "Summon Seekers (150)") (rp 150))
(advance-type s-deathtap (name "Deathtap (150)") (rp 150))
(advance-type s-skeleton (name "Summon Skeletons (150)") (rp 150))
(advance-type s-naturevoice (name "Nature's Voice (750)") (rp 750))
(advance-type s-leviathan (name "Summon Leviathan (750)") (rp 750))
(advance-type s-tsunami (name "Tsunami (750)") (rp 750))
(advance-type s-earth3 (name "Summon Major Earth Elemental (600)") (rp 600))
(advance-type s-mountainchange (name "Mountains (600)") (rp 600))
(advance-type s-beguile (name "Beguile (600)") (rp 600))
(advance-type s-earthquake (name "Earthquake (600)") (rp 600))
(advance-type s-deepchange (name "Deepen (450)") (rp 450))
(advance-type s-shallowchange (name "Shallow (450)") (rp 450))
(advance-type s-charmfish (name "Oceanic Charm (450)") (rp 450))
(advance-type s-earth2 (name "Summon Earth Elemental (450)") (rp 450))
(advance-type s-hillchange (name "Hills (300)") (rp 300))
(advance-type s-plainchange (name "Plains (300)") (rp 300))
(advance-type s-swampchange (name "Swamp (300)") (rp 300))
(advance-type s-charmbeast (name "Charm Beast (150)") (rp 150))
(advance-type s-crebain (name "Summon Crebain (150)") (rp 150))
(advance-type s-earth1 (name "Summon Minor Earth Elemental (150)") (rp 150))
(advance-type s-canticle (name "Chaos Canticle (750)") (rp 750))
(advance-type s-demon (name "Summon Demon (750)") (rp 750))
(advance-type s-chaoscreature (name "The Ordros (750)") (rp 750))
(advance-type s-volcano (name "Raise Volcano (600)") (rp 600))
(advance-type s-meteor (name "Meteor Shower (600)") (rp 600))
(advance-type s-fire3 (name "Summon Greater Fire Elemental (600)") (rp 600))
(advance-type s-disintegrate (name "Disintegrate (450)") (rp 450))
(advance-type s-flamewall (name "Wall of Flames (450)") (rp 450))
(advance-type s-fire2 (name "Summon Fire Elemental (450)") (rp 450))
(advance-type s-hellhound (name "Summon Hell Hounds (300)") (rp 300))
(advance-type s-fireball (name "Fireball (300)") (rp 300))
(advance-type s-flamestrike (name "Flame Strike (150)") (rp 150))
(advance-type s-flamearrow (name "Flame Arrows (150)") (rp 150))
(advance-type s-fire1 (name "Summon Minor Fire Elemental (150)") (rp 150))
(advance-type s-restore (name "Restoration (750)") (rp 750))
(advance-type s-enlighten (name "Enlighten (750)") (rp 750))
(advance-type s-doorway (name "Doorway (750)") (rp 750))
(advance-type s-spiritwall (name "Spirit Wall (600)") (rp 600))
(advance-type s-spirit3 (name "Summon Greater Spirit (600)") (rp 600))
(advance-type s-guardian (name "Summon Guardian (600)") (rp 600))
(advance-type s-sunray (name "Sunray (450)") (rp 450))
(advance-type s-improveheal (name "Improved Heal (450)") (rp 450))
(advance-type s-sanctify (name "Sanctify (450)") (rp 450))
(advance-type s-bless (name "Bless (300)") (rp 300))
(advance-type s-justiceeye (name "Eye of Justice (300)") (rp 300))
(advance-type s-spirit2 (name "Summon Spirits (300)") (rp 300))
(advance-type s-bolt (name "Bolt (150)") (rp 150))
(advance-type s-heal (name "Heal (150)") (rp 150))
(advance-type s-spirit1 (name "Summon Minor Spirits (150)") (rp 150))
(advance-type s-castle (name "Phantom Castle (750)") (rp 750))
(advance-type s-greatcharm (name "Greater Charm (750)") (rp 750))
(advance-type s-truedivine (name "True Divination (750)") (rp 750))
(advance-type s-guardward (name "Guards and Wards (600)") (rp 600))
(advance-type s-phantombeast (name "Phantom Beast (600)") (rp 600))
(advance-type s-air3 (name "Summon Greater Air Elemental (600)") (rp 600))
(advance-type s-prismwall (name "Prismatic Walls (450)") (rp 450))
(advance-type s-greatdivine (name "Greater Divination (450)") (rp 450))
(advance-type s-air2 (name "Summon Air Elemental (450)") (rp 450))
(advance-type s-lightning (name "Lightning Bolt (300)") (rp 300))
(advance-type s-charm (name "Charm (300)") (rp 300))
(advance-type s-phantomwarrior (name "Phantom Warriors (150)") (rp 150))
(advance-type s-divine (name "Divination (150)") (rp 150))
(advance-type s-air1 (name "Summon Minor Air Elemental (150)") (rp 150))

(advance-type sbook1-r (name "First Circle Sorcery (500)") (rp 500))
(advance-type sbook2-r (name "Second Circle Sorcery (500)") (rp 500))
(advance-type sbook3-r (name "Third Circle Sorcery (500)") (rp 500))
(advance-type sbook4-r (name "Fourth Circle Sorcery (500)") (rp 500))
(advance-type sbook5-r (name "Fifth Circle Sorcery (500)") (rp 500))
(advance-type cbook1-r (name "Fadua Chaos (500)") (rp 500))
(advance-type cbook2-r (name "Rem'lith Chaos (500)") (rp 500))
(advance-type cbook3-r (name "Utu Chaos (500)") (rp 500))
(advance-type cbook4-r (name "Zeem Chaos (500)") (rp 500))
(advance-type cbook5-r (name "Pure Chaos (500)") (rp 500))
(advance-type nbook1-r (name "First Secret of Nature (500)") (rp 500))
(advance-type nbook2-r (name "Second Secret of Nature (500)") (rp 500))
(advance-type nbook3-r (name "Third Secret of Nature (500)") (rp 500))
(advance-type nbook4-r (name "Fourth Secret of Nature (500)") (rp 500))
(advance-type nbook5-r (name "Fifth Secret of Nature (500)") (rp 500))
(advance-type lbook1-r (name "The Law of Life (500)") (rp 500))
(advance-type lbook2-r (name "The Way of Life (500)") (rp 500))
(advance-type lbook3-r (name "The Knowledge of Life (500)") (rp 500))
(advance-type lbook4-r (name "The Language of Life (500)") (rp 500))
(advance-type lbook5-r (name "The Meaning of Life (500)") (rp 500))
(advance-type dbook1-r (name "The Law of Death (500)") (rp 500))
(advance-type dbook2-r (name "The Way of Death (500)") (rp 500))
(advance-type dbook3-r (name "The Knowledge of Death (500)") (rp 500))
(advance-type dbook4-r (name "The Language of Death (500)") (rp 500))
(advance-type dbook5-r (name "The Meaning of Death (500)") (rp 500))

(table advance-precludes-advance
	(lbook1-r dbook1-r true)
	(dbook1-r lbook1-r true)
	(nbook1-r cbook1-r true)
	(lbook1-r cbook1-r true)
	(cbook1-r nbook1-r true)
	(cbook1-r lbook1-r true)

	(a* gladiator-l true)
	(a* longbow-l true)
	(a* crusader-l true)
	(a* assassin-l true)
	(a* cannon-l true)
	(a* dervish-l true)
	(a* weaver-l true)
	(a* croc-l true)
	(a* limit-break true)
	(a* hero-l true)
	(a* zombie-l true)
	(a* spectre-l true)

	(zombie-l lbook1-r true)
	(zombie-l nbook1-r true)
	(zombie-l sbook1-r true)
	(zombie-l cbook1-r true)

;; THESE ARE BROKEN, OR UNUSABLE (AS OPPOSED TO UNUSED) BY THE AI
	(a* s-guardian true)
	(a* s-heal true)
	(a* s-restore true)
	(a* s-fireball true)
	(a* s-meteor true)
	(a* s-breath true)
	(a* s-deathwalk true)
;;	(a* s-wastemaker true)
	(a* s-mountainchange true)
	(a* s-deepchange true)
	(a* s-shallowchange true)
	(a* s-plainchange true)
	(a* s-swampchange true)
	(a* s-earthquake true)

)


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
	(earth2 s-earth2 true)
	(earth3 s-earth3 true)
	(spirit1 s-spirit1 true)
	(spirit2 s-spirit2 true)
	(spirit3 s-spirit3 true)
	(fire1 s-fire1 true)
	(fire2 s-fire2 true)
	(fire3 s-fire3 true)
	(skeleton s-skeleton true)
	(ghoul s-ghoul true)
	(vampyre s-vampyre true)

;;	(gladiator gladiator-l true)
	(longbow longbow-l true)
	(crusader crusader-l true)
	(assassin assassin-l true)
;;	(cannon cannon-l true)
	(dervish dervish-l true)
;;	(weaver weaver-l true)
;;	(croc croc-l true)
	(hero hero-l true)
	(zombie zombie-l true)
	(spectre spectre-l true)


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
  (s-earth2 (nbook2-r) true)
  (s-swampchange (nbook2-r) true)
  (s-plainchange (nbook2-r) true)
  (s-hillchange (nbook2-r) true)

  (nbook4-r (nbook3-r) true)
  (s-charmfish (nbook3-r) true)
  (s-shallowchange (nbook3-r) true)
  (s-deepchange (nbook3-r) true)

  (nbook5-r (nbook4-r) true)
  (s-earth3 (nbook4-r) true)
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

;; LIMITS - Not Meant to be researched, dammit.

  (limit-break (s-iblis) true)
  (gladiator-l (limit-break) true)
  (longbow-l (limit-break) true)
  (crusader-l (limit-break) true)
  (assassin-l (limit-break) true)
  (cannon-l (limit-break) true)
  (dervish-l (limit-break) true)
  (weaver-l (limit-break) true)
  (croc-l (limit-break) true)
  (hero-l (limit-break) true)
  (zombie-l (limit-break) true)
  (spectre-l (limit-break) true)

)


(terrain-type sea (char ".")
  (help "deep water"))
(terrain-type shallows (char ",")
  (help "shallow coastal water and lakes"))
(terrain-type swamp (char "=") (image-name "kiwiterr-swamp"))
(terrain-type desert (image-name "adv-desert") (char "~")
  (help "dry open terrain"))
(terrain-type land (image-name "adv-plain") (char "+")
  (help "open flat or rolling country"))
(terrain-type forest (image-name "kiwiterr-forest")(char "%"))
(terrain-type mountains (image-name "kiwiterr-mountains") (char "^"))
(terrain-type ice  (image-name "adv-ice") (char "_"))
(terrain-type neutral (image-name "gray") (char "-"))
(terrain-type hills (image-name "kiwiterr-hills"))
(terrain-type semi-desert (image-name "adv-semi-desert"))
(terrain-type steppe (image-name "adv-steppe"))
(terrain-type deepforest (image-name "kiwiterr-forest"))
(terrain-type wasteland (image-name "kiwiterr-rubble"))
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
(terrain-type jungle (image-name "kiwiterr-jungle"))

(terrain-type p-land (name "Phantom Plains") (image-name "blue") (char "+")
  (help "Wispy plains."))
(terrain-type p-forest (name "Phantom Forest") (image-name "blue") (char "%"))
(terrain-type p-mountains (name "Phantom Mountains") (image-name "blue") (char "^"))
(terrain-type p-hills (name "Phantom Hills") (image-name "blue"))
(terrain-type p-river (name "Phantom River") (image-name "blue")
  (subtype border) (subtype-x river-x))
(terrain-type p-cliffs (name "Phantom Cliffs") (image-name "blue")
  (subtype border))



(add (sea shallows) liquid true)

(define sea-t* (sea shallows))

(define land-t* (jungle deepforest wasteland desert semi-desert hills steppe land forest mountains river))

(define phantom-t* (p-land p-forest p-mountains p-hills p-river p-cliffs))

;;; Unit-unit capacities.


(table unit-size-as-occupant
  ;; Disable occupancy by default.
	(u* u* 99)
	(sludge (ulitar) 1)
	(humanoid-types (cutter) 2)
	(humanoid-types fortress-types 1)

	(all-types place-types 1)
	(place-types place-types 99)

	(huge-types place-types 99)
	(gargantuan-types place-types 99)

	((skeleton zombie deathknight ghoul) cutter 1)
	(enchantment-types place-types 1)
	(enchantment-types fortress-types 1)
	(deathtap ruin-types 1)
	(riddle resource-types 1)

	(dragon-types lair 1)

	(dragonegg hero 1)
	(dragonegg legend 1)
	(dragonegg epic 1)
	(dragonegg wizard 1)
	(dragonegg archmage 1)
	(dragonegg croc 1)

	(wall-types place-types 99)
	(wall-types fortress-types 99)

)

(add heroic-types capacity 1)

(add croc capacity 1)

(add ruin-types capacity 1)

(add (ulitar) capacity 1)

(add (cutter) capacity 4)

(add (keep) capacity 4)

(add (tower) capacity 1)

(add (castle) capacity 5)

(add (citadel) capacity 6)

(add place-types capacity 3)
(add phantompavilion capacity 3)

(add lair capacity 3)

(add phantomship capacity 2)

(table occupant-max
	(tower all-types 1)
	(keep all-types 4)
	(citadel all-types 6)
	(castle all-types 7)
	(place-types all-types 3)
	(phantompavilion phantomwarrior 3)
	(cutter catapult 1)
	(cutter cannon 1)
	(phantomship all-types 0)
	(phantomship phantomwarrior 2)
	
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
	(lair dragonegg 0)
	
)

;;; Unit-terrain capacities.

(table unit-size-in-terrain
  (u* t* 25)
  (place-types t* 100)
  (fortress-types t* 100)
  ((phantompavilion castle) t* 100)
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
	(node-types t* 25)
	(resource-types t* 25)
	(wilderness-types t* 25)

	(seed-types t* 0)
	(event-types t* 0)
	(lair t* 125)
	(lair mountains 70)
	(dragonegg mountains 1)
	(riplegend t* 0)
	(wastemaker t* 0)
	(wall-types t* 70)
)

(add t* capacity 100)

;;; Unit-material capacities.


(table base-production
	(wcity4 gold 5)
	(city4 gold 5)
	(wcity4 anvil 13)
	(city4 anvil 13)
	(wcity4 mana 3)
	(city4 mana 3)
	(wcity4 study 10)
	(city4 study 10)

	(wcity3 gold 3)
	(city3 gold 3)
	(wcity3 anvil 8)
	(city3 anvil 8)
	(wcity3 study 6)
	(city3 study 6)
	(wcity3 mana 2)
	(city3 mana 2)

	(wcity2 gold 2)
	(city2 gold 2)
	(wcity2 anvil 5)
	(city2 anvil 5)
	(wcity2 study 4)
	(city2 study 4)
	(wcity2 mana 1)
	(city2 mana 1)

	(lcity gold 2)
	(lcity wood 5)
	(lcity study 4)
	(lcity mana 1)

	(dcity gold 8)
	(dcity stone 8)
	(dcity anvil 10)

	(ocity anvil 8)
	(ocity gold 8)
	(ocity study 15)
	(ocity mana 8)

	(icity anvil 8)
	(icity gold 8)
	(icity study 10)
	(icity mana 5)

	(wcity1 gold 1)
	(city1 gold 1)
	(wcity1 anvil 3)
	(city1 anvil 3)
	(wcity1 study 2)
	(city1 study 2)
	(wcity1 mana 1)
	(city1 mana 1)

	(town gold 1)
	(town anvil 2)
	(town study 1)
	(town mana 1)

	(svillage gold 1)
	(svillage anvil 3)
	(svillage wood 3)
	(svillage study 1)
	(svillage mana 1)

	(ohamlet gold 1)
	(ohamlet anvil 3)
	(ohamlet study 1)
	(ohamlet mana 1)

	(ovillage gold 1)
	(ovillage anvil 5)
	(ovillage study 1)
	(ovillage mana 1)

        (deadcity anvil 8)
        (deadcity gold 8)
        (deadcity study 20)
        (deadcity mana 10)

	(citadel anvil 3)
	(citadel study 2)
	(citadel mana 2)

	(keep anvil 3)
	(keep study 1)
	(keep mana 1)

	(tower anvil 2)

	(phantompavilion gold 2)
	(phantompavilion study 5)
	(phantompavilion mana 5)

	(thurastes anvil 10)
	(lair anvil  8) 
	(lair gold 8)
	(lair study 8) 
	(lair mana 8)

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

	(enode mana 3)
  	(enode study 10)
	(snode mana 3)
  	(snode study 10)
	(lnode mana 3)
  	(lnode study 10)
	(dnode mana 3)
  	(dnode study 10)
	(cnode mana 3)
  	(cnode study 10)
	(temple study 20)
	(temple mana 10)

	(boscorpse gold 5)
	(axecorpse gold 8)
	(deadwild anvil 1)
	(deadwild gold 1)
	(thucorpse gold 100)
	(fish gold 5)
	(fish anvil 5)
	(lumber wood 10)
	(goldmine gold 5)
	(ironmine anvil 10)
	(quarry stone 10)

	(dragonegg timer 1)
	(gwyrm timer 1)
	(hatchling timer 1)
	(wyrm timer 1)

	(bless anvil 3)
	(bless mana 3)
	(mage-types study 5)

	(hatchling flames 1)
	(wyrm flames 1)
	(gwyrm flames 2)
	(cutter flames 1)
	(whaler flames 1)

	(mage-types mana 10)
	(mage mana 10)
	(dervish mana 10)
	(weaver mana 10)
	(high-mage-types charge 3)
	(owizard charge 4)
	(oarchmage charge 4)

	(hero opp 1)

)


(table unit-storage-x
  	(place-types anvil 200)
	(place-types gold 200)
	(place-types study 200)
	(place-types mana 200)

	(lantern gold 500)
	(lantern mana 500)
	(lantern study 2000)
	(lantern timer 8)

  	(ghost timer 3)
  	(breath timer 3)

	(boscorpse timer 3)
	(axecorpse timer 3)
	(thucorpse timer 3)
	(deadwild timer 3)

	(hatchling timer 50)
	(wyrm timer 50)
	(gwyrm timer 50)
	(dragonegg timer 50)

	(phantompavilion gold 200)
	(phantompavilion study 200)
	(phantompavilion mana 200)

	(castle gold 200)
	(castle study 200)
	(castle mana 200)

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
	(shot-types timer 2)
	(explode-types timer 2)
	(bless (anvil mana) 5)
	(lumber wood 60)
	(goldmine gold 60)
	(ironmine anvil 60)
	(quarry stone 60)
	(mage-types study 5)
	(hatchling flames 3)
	(wyrm flames 3)
	(gwyrm flames 8)

	(mage-types mana 30)

	(mage mana 25)
	(dervish mana 15)
	(weaver mana 20)

	(high-mage-types mana 300)
	(high-mage-types charge 5)

	(owizard charge 7)
	(oarchmage charge 7)

	(charm mana 1)
	(greatcharm mana 3)
	(enlighten mana 1)
	(charmfish mana 4)
	(charmbeast mana 4)

	(beguile mana 3)
	(tsunami timer 2)

	(wastemaker timer 2)
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
(add seed-types speed 0)
(add event-types speed 0)
(add dragonegg speed 0)
(add riplegend speed 0)
(add wastemaker speed 0)
(add (phantompavilion castle) speed 0)
(add lantern speed 0)


(table mp-to-enter-terrain
  (u* t* 2)
  (land-types sea-t* 99)
  (deep-types land-t* 99)
  (water-types land-t* 99)
  (land-types cliffs 99)
  (deep-types cliffs 99)
  (water-types cliffs 99)
  (climber-trait cliffs 1)
  (land-types river 3)
  (heroic-types river 1)
  (amphibious-types river 1)
  (amphibious-types beach 1)
  (land-types (hills forest semi-desert swamp jungle) 3)
  (land-types (desert wasteland deepforest) 4)
  (land-types mountains 6)
  (heroic-types (hills forest semi-desert swamp jungle) 2)
  (heroic-types (desert wasteland deepforest mountains) 3)
  (tireless-trait (hills forest semi-desert swamp jungle desert wasteland deepforest mountains) 2)
  (jungle-born-trait jungle 2)
  (desert-born-trait desert 1)
  (mountain-born-trait mountains 2)
  (swamp-born-trait swamp 1)
  (mounted-trait steppe 1)
  (air-types t* 1)
  (vampyre t* 1)
  (air-types river 0)
  (deep-types shallows 99)
  (deep-types sea 1)
  (water-types sea-t* 1)
  ((wocke bears wolves ferak royalferak) t* 99)
  ((bears wolves ferak royalferak) (deepforest forest jungle) 1)
  (wocke (deepforest) 1)
  (death-types wasteland 1)
  (skeleton wasteland 1)
  (vampyre wasteland 1)
  (ghoul wasteland 1)
  (seeker wasteland 1)
  (iblis wasteland 1)
  (lich wasteland 1)
  (greyone wasteland 1)
  (deathknight wasteland 1)
  (charm-types t* 1)
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

(define a-types (longbow crusader assassin))
(define b-types (sword halberd bow cavalry catapult longbow crusader assassin dervish))
(define c-types (sword halberd bow cavalry longbow dervish))
(define l-types (lsword lhalberd lbow lcavalry lmage croc))
(define d-types (dsword dhalberd dbow cannon))
(define o-types (osword ocavalry obow weaver))
(define i-types (isword ihalberd ibow gladiator image))



(table can-create
	(build1 basic-types 3)
	(build1 (flamewall bless) 3)
	(build1 a-types 3)
	(build1 summon-types 3)
	(build2 b-types 3)
	(build2 (flamewall bless) 3)
	(build3 c-types 3)
	(build3 (flamewall bless) 3)
	(lcity l-types 3)
	(dcity d-types 3)
	(ocity o-types 3)
	(icity i-types 3)

  (town (sword bow) 3)
  (svillage (cutter whaler) 3)
  (ohamlet (spear shaman) 3)
  (ovillage (thug champ) 3)
  (spectre ghost 3)
  (heroic-types (citadel keep tower fort) 3)
  (citadel (riddle flamewall sanctify guardward prismwall halberd cavalry sword bow longbow crusader catapult cannon ) 3)
  (keep (riddle flamewall sanctify guardward prismwall cavalry sword bow longbow) 3)
  (fort (riddle flamewall sanctify guardward prismwall scout justiceeye) 3)

  (lair dragonegg 3)

  (lab golem-types 3)

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
  (dnode (vampyre seeker lich skeleton iblis) 3)
  (deadcity (spectre zombie ghoul vampyre seeker lich skeleton iblis) 3)
  (thurastes (tentacle) 3)

	(place-types enchantment-types 3)

	(self-types shot-types 3)
	(self-types explode-types 3)
	(self-types (deathtap riddle) 6)
	(self-types create-types 3)
	(self-types charm-types 3)

	(high-mage-types shot-types 3)
	(high-mage-types explode-types 3)
	(high-mage-types (deathtap riddle) 3)
	(high-mage-types create-types 3)
	(high-mage-types charm-types 3)

	(wyrm shot-types 3)
	(wyrm explode-types 3)
	(wyrm charm-types 3)

	(gwyrm shot-types 3)
	(gwyrm explode-types 3)
	(gwyrm (deathtap riddle) 6)
	(gwyrm create-types 3)
	(gwyrm charm-types 3)

	((phantompavilion castle) (phantomship phantomwarrior phantombeast) 3)

	(lantern spell-types 3)
)

(table acp-to-create
	(build1 basic-types 3)
	(build1 (flamewall bless) 3)
	(build1 a-types 3)
	(build1 summon-types 3)
	(build2 b-types 3)
	(build2 (flamewall bless) 3)
	(build3 c-types 3)
	(build3 (flamewall bless) 3)
	(lcity l-types 3)
	(dcity d-types 3)
	(ocity o-types 3)
	(icity i-types 3)

  (town (sword bow) 3)
  (svillage (cutter whaler) 3)
  (ohamlet (spear shaman) 3)
  (ovillage (thug champ) 3)
  (spectre ghost 3)
  (heroic-types (citadel keep tower fort) 3)
  (citadel (riddle flamewall sanctify guardward prismwall halberd cavalry sword bow longbow crusader catapult cannon ) 3)
  (keep (riddle flamewall sanctify guardward prismwall cavalry sword bow longbow) 3)
  (fort (riddle flamewall sanctify guardward prismwall scout justiceeye) 3)

  (lair dragonegg 3)

  (lab golem-types 3)

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
  (dnode (vampyre seeker lich skeleton iblis) 3)
  (deadcity (spectre zombie ghoul vampyre seeker lich skeleton iblis) 3)
  (thurastes (tentacle) 3)

	(place-types enchantment-types 3)

	(self-types shot-types 3)
	(self-types explode-types 3)
	(self-types (deathtap riddle) 6)
	(self-types create-types 3)
	(self-types charm-types 3)

	(high-mage-types shot-types 3)
	(high-mage-types explode-types 3)
	(high-mage-types (deathtap riddle) 3)
	(high-mage-types create-types 3)
	(high-mage-types charm-types 3)

	(wyrm shot-types 3)
	(wyrm explode-types 3)
	(wyrm charm-types 3)

	(gwyrm shot-types 3)
	(gwyrm explode-types 3)
	(gwyrm (deathtap riddle) 6)
	(gwyrm create-types 3)
	(gwyrm charm-types 3)

	((phantompavilion castle) (phantomship phantomwarrior phantombeast) 3)

	(lantern spell-types 3)
)

(table cp-on-creation
	(build1 basic-types 3)
	(build1 (flamewall bless) 3)
	(build1 a-types 3)
	(build1 summon-types 3)
	(build2 b-types 3)
	(build2 (flamewall bless) 3)
	(build3 c-types 3)
	(build3 (flamewall bless) 3)
	(lcity l-types 3)
	(dcity d-types 3)
	(ocity o-types 3)
	(icity i-types 3)

  (town (sword bow) 1)
  (svillage (cutter whaler) 3)
  (ohamlet (spear shaman) 3)
  (ovillage (thug champ) 3)
  ((deadcity ruins) death-types 3)
  (spectre ghost 3)
  (heroic-types (citadel keep tower fort) 3)
  (citadel (riddle flamewall sanctify guardward prismwall halberd cavalry sword bow longbow crusader catapult cannon) 3)
  (keep (riddle flamewall sanctify guardward prismwall cavalry sword bow longbow) 3)
  (fort (riddle flamewall sanctify guardward prismwall scout justiceeye) 3)

  (lair dragonegg 3)

  (lab golem-types 3)


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
  (dnode (vampyre seeker lich skeleton iblis) 3)
  (deadcity (spectre zombie ghoul vampyre seeker lich skeleton iblis) 3)
  (thurastes (tentacle) 3)

	(place-types enchantment-types 3)

	(self-types shot-types 3)
	(self-types explode-types 3)
	(self-types (deathtap riddle) 6)
	(self-types create-types 3)
	(self-types charm-types 3)

	(high-mage-types shot-types 3)
	(high-mage-types explode-types 3)
	(high-mage-types (deathtap riddle) 6)
	(high-mage-types create-types 3)
	(high-mage-types charm-types 3)

	(wyrm shot-types 3)
	(wyrm explode-types 3)
	(wyrm charm-types 3)

	(gwyrm shot-types 3)
	(gwyrm explode-types 3)
	(gwyrm (deathtap riddle) 6)
	(gwyrm create-types 3)
	(gwyrm charm-types 3)

	((phantompavilion castle) (phantomship phantomwarrior phantombeast) 3)

	(lantern spell-types 3)

)


(table can-build
	(build1 basic-types 3)
	(build1 (flamewall bless) 3)
	(build1 a-types 3)
	(build1 summon-types 3)
	(build2 b-types 3)
	(build2 (flamewall bless) 3)
	(build3 c-types 3)
	(build3 (flamewall bless) 3)
	(lcity l-types 3)
	(dcity d-types 3)
	(ocity o-types 3)
	(icity i-types 3)

  (town (sword bow) 3)
  (svillage (cutter whaler) 3)
  (ohamlet (spear shaman) 3)
  (ovillage (thug champ) 3)
  ((deadcity ruins) death-types 3)
  (spectre ghost 3)
  (heroic-types (citadel keep tower fort) 3)
  (citadel (riddle flamewall sanctify guardward prismwall halberd cavalry sword bow longbow crusader catapult cannon ) 3)
  (keep (riddle flamewall sanctify guardward prismwall cavalry sword bow longbow) 3)
  (fort (riddle flamewall sanctify guardward prismwall scout justiceeye) 3)

  (lair dragonegg 3)

  (lab golem-types 3)

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
  (dnode (vampyre seeker lich skeleton iblis) 3)
  (deadcity (spectre zombie ghoul vampyre seeker lich skeleton iblis) 3)
  (thurastes (tentacle) 3)

	(place-types enchantment-types 3)

	(self-types shot-types 3)
	(self-types explode-types 3)
	(self-types (deathtap riddle) 3)
	(self-types create-types 3)
	(self-types charm-types 3)

	(high-mage-types shot-types 3)
	(high-mage-types explode-types 3)
	(high-mage-types (deathtap riddle) 3)
	(high-mage-types create-types 3)
	(high-mage-types charm-types 3)

	(wyrm shot-types 3)
	(wyrm explode-types 3)
	(wyrm summon-types 3)
	(wyrm charm-types 3)

	(gwyrm shot-types 3)
	(gwyrm explode-types 3)
	(gwyrm (deathtap riddle) 3)
	(gwyrm create-types 3)
	(gwyrm charm-types 3)


	((phantompavilion castle) (phantomship phantomwarrior phantombeast) 3)

	(lantern spell-types 3)
)

(table acp-to-build
	(build1 basic-types 3)
	(build1 (flamewall bless) 3)
	(build1 a-types 3)
	(build1 summon-types 3)
	(build2 b-types 3)
	(build2 (flamewall bless) 3)
	(build3 c-types 3)
	(build3 (flamewall bless) 3)
	(lcity l-types 3)
	(dcity d-types 3)
	(ocity o-types 3)
	(icity i-types 3)

  (town (sword bow) 3)
  (svillage (cutter whaler) 3)
  (ohamlet (spear shaman) 3)
  (ovillage (thug champ) 3)
  ((deadcity ruins) death-types 3)
  (spectre ghost 3)
  (heroic-types (citadel keep tower fort) 3)
  (citadel (riddle flamewall sanctify guardward prismwall halberd cavalry sword bow longbow crusader catapult cannon ) 3)
  (keep (riddle flamewall sanctify guardward prismwall cavalry sword bow longbow) 3)
  (fort (riddle flamewall sanctify guardward prismwall scout justiceeye) 3)

  (lair dragonegg 3)

  (lab golem-types 3)

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
  (dnode (vampyre seeker lich skeleton iblis) 3)
  (deadcity (spectre zombie ghoul vampyre seeker lich skeleton iblis) 3)
  (thurastes (tentacle) 3)

	(place-types enchantment-types 3)

	(self-types shot-types 3)
	(self-types explode-types 3)
	(self-types (deathtap riddle) 3)
	(self-types create-types 3)
	(self-types charm-types 3)

	(high-mage-types shot-types 3)
	(high-mage-types explode-types 3)
	(high-mage-types (deathtap riddle) 3)
	(high-mage-types create-types 3)
	(high-mage-types charm-types 3)

	(wyrm shot-types 3)
	(wyrm explode-types 3)
	(wyrm summon-types 3)
	(wyrm charm-types 3)

	(gwyrm shot-types 3)
	(gwyrm explode-types 3)
	(gwyrm (deathtap riddle) 3)
	(gwyrm create-types 3)
	(gwyrm charm-types 3)


	((phantompavilion castle) (phantomship phantomwarrior phantombeast) 3)

	(lantern spell-types 3)
)

(table cp-per-build
	(build1 basic-types 6)
	(build1 (flamewall bless) 3)
	(build1 a-types 6)
	(build1 summon-types 3)
	(build2 b-types 3)
	(build2 (flamewall bless) 3)
	(build3 c-types 3)
	(build3 (flamewall bless) 3)
	(lcity l-types 3)
	(dcity d-types 3)
	(ocity o-types 3)
	(icity i-types 3)

  (town (sword bow) 1)
  (svillage (cutter whaler) 3)
  (ohamlet (spear shaman) 3)
  (ovillage (thug champ) 3)
  ((deadcity ruins) death-types 3)
  (spectre ghost 3)
  (heroic-types (citadel keep tower fort) 3)
  (citadel (riddle flamewall sanctify guardward prismwall halberd cavalry sword bow longbow crusader catapult cannon) 3)
  (keep (riddle flamewall sanctify guardward prismwall cavalry sword bow longbow) 3)
  (fort (riddle flamewall sanctify guardward prismwall scout justiceeye) 3)

  (lair dragonegg 3)

  (lab golem-types 3)

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
  (dnode (vampyre seeker lich skeleton iblis) 3)
  (deadcity (spectre zombie ghoul vampyre seeker lich skeleton iblis) 3)
  (thurastes (tentacle) 3)

	(place-types enchantment-types 3)

	(self-types shot-types 3)
	(self-types explode-types 3)
	(self-types (deathtap riddle) 3)
	(self-types create-types 3)
	(self-types charm-types 3)

	(high-mage-types shot-types 3)
	(high-mage-types explode-types 3)
	(high-mage-types (deathtap riddle) 6)
	(high-mage-types create-types 3)
	(high-mage-types charm-types 3)

	(wyrm shot-types 3)
	(wyrm explode-types 3)
	(wyrm charm-types 3)

	(gwyrm shot-types 3)
	(gwyrm explode-types 3)
	(gwyrm (deathtap riddle) 3)
	(gwyrm create-types 3)
	(gwyrm charm-types 3)


	((phantompavilion castle) (phantomship phantomwarrior phantombeast) 3)


	(lantern spell-types 3)

)



(table create-range
	(svillage (cutter whaler) 1)
	((phantompavilion castle) (phantomship phantombeast) 1)

	(self-types spell-types 2)

	(high-mage-types spell-types 2)

	(wyrm spell-types 2)
	(gwyrm spell-types 2)

	(self-types seance 50)
	(mage-types seance 50)
	(wizard seance 50)
	(archmage seance 50)
	(wyrm seance 50)
	(gwyrm seance 50)

	(place-types wall-types 1)

	(u* fortress-types 1)


	(lantern spell-types 1)
)

(table build-range
	(svillage (cutter whaler) 1)
	((phantompavilion castle) (phantomship phantombeast) 1)

	(self-types spell-types 2)

	(high-mage-types spell-types 2)

	(wyrm spell-types 2)
	(gwyrm spell-types 2)

	(self-types seance 50)
	(mage-types seance 50)
	(high-mage-types seance 50)
	(wyrm seance 50)
	(gwyrm seance 50)

	(place-types wall-types 1)
	(u* fortress-types 1)

	(lantern spell-types 1)
)


(table gives-to-treasury
	(node-types m* true)
	(lantern m* true)
	(wilderness-types gold true)
	(place-types m* true)
	(boscorpse gold true)
	(axecorpse gold true)
	(thucorpse gold true)
	(deadwild gold true)
	(deadwild anvil true)
	(node-types m* true)
	(wilderness-types anvil true)
	(fortress-types anvil true)
	(thurastes anvil true)
	(resource-types m* true)
	(phantompavilion m* true)

	(u* study true)
)

(table takes-from-treasury 

	(u* m* true)
	(lantern m* true)

)


(table consumption-on-creation
  (sword anvil 20)
  (bow wood 20)
  (halberd anvil 30)
  (cavalry anvil 25)
  (cavalry gold 15)
  (catapult wood 25)
  (catapult gold 25)
  (mage gold 50)
  (merc gold 25)

  (lsword wood 20)
  (lbow wood 20)
  (lhalberd wood 30)
  (lcavalry wood 25)
  (lcavalry gold 15)
  (lmage gold 50)
  (croc gold 30)

  (osword anvil 20)
  (obow wood 20)
  (ocavalry anvil 25)
  (ocavalry gold 15)
  (ocatapult wood 25)
  (ocatapult gold 25)
  (weaver gold 30)

  (dsword anvil 25)
  (dbow anvil 10)
  (dhalberd anvil 40)
  (cannon anvil 50)
  (cannon gold 25)
  (cannon stone 25)

  (isword anvil 20)
  (ibow wood 20)
  (ihalberd anvil 30)
  (gladiator gold 30)
  (image gold 50)


  (scout anvil 5)
  (scout gold 10)
  (longbow wood 30)
  (crusader anvil 20)
  (crusader gold 20)
  (assassin gold 50)
  (dervish gold 30)
  (dervish anvil 0)

  (dragonegg gold 150)

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
	(phantomship mana 50)
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

	(lair timer 40)

  (zombie anvil 20)
  (spectre gold 100)
  (ghost gold 20)

  (slaver gold 200)
  (sorcerer anvil 200)

	(golem-types mana 20)

	(woodgolem wood 75)
	(stonegolem stone 75)
	(irongolem anvil 75)
	(clockgolem gold 75)

	(powerword mana 50)
	(iblis mana 200)
	(lich mana 150)
	(deathwalk mana 40)
	(wastemaker mana 100)
	(riddle mana 30)
	(breath mana 40)
	(seance mana 25)
	(poltergeist mana 40)
	(bonearrow mana 20)
	(seeker mana 30)
	(deathtap mana 1)
	(naturevoice mana 25)
	(leviathan mana 200)
	(tsunami mana 80)
	(mountainchange mana 20)
	(beguile mana 60)
	(earthquake mana 60)
	(deepchange mana 20)
	(shallowchange mana 20)
	(charmfish mana 60)
	(hillchange mana 20)
	(plainchange mana 20)
	(swampchange mana 20)
	(charmbeast mana 30)
	(crebain mana 30)
	(canticle mana 100)
	(demon mana 150)
	(chaoscreature mana 200)
	(volcano mana 120)
	(meteor mana 100)
	(disintegrate mana 70)
	(flamewall mana 20)
	(hellhound mana 40)
	(fireball mana 40)
	(flamestrike mana 20)
	(flamearrow mana 20)
	(restore mana 70)
	(enlighten mana 100)
	(doorway mana 100)
	(spiritwall mana 25)
	(sunray mana 50)
	(guardian mana 80)
	(improveheal mana 50)
	(sanctify mana 40)
	(bless mana 40)
	(justiceeye mana 10)
	(bolt mana 10)
	(heal mana 20)
	(castle mana 150)
	(greatcharm mana 100)
	(truedivine mana 60)
	(guardward mana 40)
	(phantombeast mana 100)
	(prismwall mana 20)
	(greatdivine mana 50)
	(lightning mana 30)
	(charm mana 40)
	(phantomwarrior mana 20)
	(divine mana 30) 
	(skeleton mana 40)
	(ghoul mana 70)
 	(vampyre mana 130)

	(earth1 mana 40)
	(air1 mana 40)  
	(fire1 mana 40)
	(spirit1 mana 40)
	(earth2 mana 70)
	(air2 mana 70)  
	(fire2 mana 70)
	(spirit2 mana 70)
	(earth3 mana 130)
	(air3 mana 130)  
	(fire3 mana 130)
	(spirit3 mana 130)


)

(table base-consumption
  (sword anvil 1)
  (bow anvil 1)
  (halberd anvil 2)
  (cavalry anvil 1)
  (cavalry gold 1)
  (catapult anvil 2)
  (catapult gold 1)
  (mage gold 2)
  (merc gold 1)

  (lsword wood 1)
  (lbow wood 1)
  (lhalberd wood 2)
  (lcavalry wood 1)
  (lcavalry gold 1)
  (lmage gold 2)

  (osword anvil 1)
  (obow anvil 1)
  (ocavalry anvil 1)
  (ocavalry gold 1)
  (ocatapult wood 1)
  (ocatapult gold 1)

  (dsword anvil 1)
  (dbow anvil 1)
  (dhalberd anvil 2)

  (isword anvil 1)
  (ibow wood 1)
  (ihalberd anvil 1)
  (image gold 2)

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
  	(breath timer 1)
 	(boscorpse timer 1)
	(axecorpse timer 1)
	(thucorpse timer 1)
	(deadwild timer 1)
	(shot-types timer 1)
	(explode-types timer 1)
	(spell-types mana 2)
	(enchantment-types mana 1)

	(wastemaker timer 1)
	(lantern timer 1)
)


(table supply-on-creation
  (u* m* 0)
  (ghost timer 3)
  (breath timer 3)
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
	(charm-types mana 3)
	(bless m* 5)
	(hero opp 1)

	(lantern timer 8)
	(lantern study 2000)
)

;; (table hp-to-garrison

;;   (hatchling wyrm 999)
;;   (wyrm gwyrm 999)
;;)


;; RANDOM EVENTS



(table see-chance
   (phantom-types u* 25)
   (u* phantom-types 25)
   (phantom-types phantom-types 100)

   (u* kraken-types 0)
   (u* spawn 0)
   (u* deep-types 0)
   (naturevoice u* 100)
   (u* assassin 0)
   (u* poltergeist 0)
   (u* seed-types 0)
   (u* event-types 0)
   (whaler bospallian 35)
   (whaler axehead 25)
   (cutter bospallian 25)
   (cutter axehead 15)

   (u* riplegend 0)
   (mage-types riplegend 20)
   (wizard riplegend 100)
   (archmage riplegend 100)


   (u* lantern 0)
)

(table see-chance-adjacent
   (u* seed-types 0)
   (u* event-types 0)
   (u* riplegend 0)
   (u* poltergeist 20)
   (heroic-types riplegend 100)
   (u* lantern 0)
)

(table see-chance-at
   (u* seed-types 0)
   (u* event-types 0)
   (u* riplegend 0)
   (heroic-types riplegend 100)
   (u* lantern 0)
)

(table accident-hit-chance
	(seed-types t* 100)
	(fortseed t* 300)
	(dragonseed t* 100)
	(merc t* 50)
)

(table accident-damage
   (seed-types t* 1)
	(merc t* 100)
)

(table detonation-accident-chance
	(faultline t* 100)
)


(table independent-capture-chance
   (basic-types basic-types 10)
   (basic-types specific-types 10)
   (specific-types basic-types 10)
   (specific-types specific-types 10)
   (self-types basic-types 25)
   (self-types specific-types 25)
   (heroic-types basic-types 20)
   (heroic-types specific-types 20)
   (archmage basic-types 50)
   (archmage specific-types 50)
   (epic basic-types 50)
   (epic specific-types 50)
   (legend basic-types 75)
   (legend specific-types 75)
   (legend hatchling 25)
   (legend wyrm 10)

   (liz-types liz-types 20)
   (dwarf-types dwarf-types 20)
   (oni-types oni-types 20)
   (iriken-types iriken-types 20)

   (basic-types heroic-types 100)
   (specific-types heroic-types 100)
   (u* epic 0)
   (hero heroic-types 100)
   (wizard heroic-types 100)
   (legend heroic-types 100)
   (epic heroic-types 100)
   (archmage heroic-types 100)
   (self-types heroic-types 100)

   (basic-types citadel 100)
   (specific-types citadel 100)
   (self-types citadel 100)
   (heroic-types citadel 100)

   (basic-types tower 100)
   (specific-types tower 100)
   (self-types tower 100)
   (heroic-types tower 100)

)

(table control-range
  (u* t* 0)
  (seed-types t* -1)
  (event-types t* -1)
  (wastemaker t* -1)

  )

(table zoc-range
  (seed-types u* -1)
  (event-types u* -1)
  (riplegend u* -1)
  (wastemaker u* -1)
  (u* wastemaker -1)

   (lantern u* -1)

;;  (infantry-types infantry-types 1)
;;  (place-types infantry-types 1)
  )


;;; Repair.

(define ten-hp-types (assassin spear shaman ferak mold sludge poltergeist seeker crebain guardian air1 phantomwarrior))

(define twenty-hp-types (scout longbow dervish weaver croc sword bow zombie ghost wolves gibambi churambi skeleton earth1 fire1 spirit1 air2 oslt mage obow))

(define thirty-hp-types (halberd catapult tribesman bears royalferak fire2 hellhound spirit2 lsword dsword lbow
   dbow merc osword ocatapult
   ibow isword ihalberd image))

(define forty-hp-types (gladiator crusader cavalry thug thuvi pterodactyl plesiosaur tentacle sorcerer ghoul earth2 air3
   oschief lhalberd dhalberd ocavalry))

(define fifty-hp-types (hero wizard cannon champ cutter spectre deathknight slaver vampyre lich fire3 spirit3 ranger
   oswarlord lcavalry ohero oranger oknight owizard))

(define eighty-hp-types (wocke spawn talon greyone pharoah shah emperor loremaster shogun minister usurper king whaler dajaturtle squid ulitar hatchling earth3 knight lhero lranger lwizard dhero dranger
   ihero iranger iwizard))

(define hundred-hp-types (tyrannosaur axehead wyrm chaoscreature demon phantombeast epic archmage phantomship lepic
   depic oarchmage oepic
   iepic iarchmage))

(define onetwenty-hp-types (dajawyrm))

(define onefifty-hp-types (apatosaur legend iblis phantomshoggoth llegend dlegend olegend ilegend))

(define twohun-hp-types (bospallian nameless))

(define threehun-hp-types (thurastes gwyrm leviathan))


(add u* hp-recovery 100)
(add twenty-hp-types hp-recovery 200)
(add thirty-hp-types hp-recovery 300)
(add forty-hp-types hp-recovery 400)
(add fifty-hp-types hp-recovery 500)
(add eighty-hp-types hp-recovery 800)
(add hundred-hp-types hp-recovery 1000)
(add onetwenty-hp-types hp-recovery 1250)
(add onefifty-hp-types hp-recovery 1500)
(add twohun-hp-types hp-recovery 2000)
(add threehun-hp-types hp-recovery 3000)
(add wall-types hp-recovery 800)

(add zombie hp-recovery 0)
(add skeleton hp-recovery 0)
(add wilderness-types hp-recovery 0)
(add self-types hp-recovery 1000)
(add vampyre hp-recovery 32000)
(add riplegend hp-recovery 0)
(add golem-types hp-recovery 0)

(table can-repair
	((heal improveheal) u* true)
	(shaman all-types true)
	(ranger all-types true)
    (svillage (whaler cutter) true)
    (place-types (catapult cannon) true)
    (fortress-types (catapult cannon) true)
    (fortress-types fortress-types true)
	(lab golem-types true)
)

(table acp-to-repair
	((heal improveheal) u* 1)
	(shaman all-types 1)
	(ranger all-types 1)
    (svillage (whaler cutter) 1)
    (place-types (catapult cannon) 1)
    (fortress-types (catapult cannon) 1)
    (fortress-types fortress-types 1)
	(lab golem-types 1)
)

(table hp-per-repair
	(heal u* 1500)
	(improveheal u* 2000)
	(shaman all-types 450)
	(shaman all-types 300)

	(lab golem-types 500)

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
  (place-types all-types 300)
  (lab golem-types 500)
  )

(table auto-repair-range
  (place-types all-types 1)
  (lab golem-types 1)
  )





(table hp-per-starve
  	(ghost timer 100.00)
  	(breath timer 100.00)
	(boscorpse timer 100.00)
	(axecorpse timer 100.00)
	(thucorpse timer 100.00)
	(deadwild timer 100.00)
	(shot-types timer 100.00)
	(explode-types timer 100.00)
	(lantern timer 100.00)

	(charm-types mana 100.00)
	(seance mana 100.00)
;;	(wastemaker timer 100.00)
)

;;; Combat.



(set combat-model 0)


(table acp-to-attack
  (u* u* 1)
  (squid u* 1)
  (bospallian u* 4)
  (axehead u* 4)
  (wild-types u* 2)
  (wild-types croc 4)

;; Sapping takes time

  (all-types air-types 0)
  (deep-types land-types 0)
  (land-types deep-types 0)
  (land-types water-types 0)

  (air-types air-types 1)
  (air-types all-types 1)

;; If a ship pulls next to the great maw of Thurastes, the swordsmen on board don't have to wait for it to attack

  (land-types thurastes 1)

;; And if a ship is sitting along the coast, land units who are willing to try hard enough can storm it

  (land-types (cutter whaler) 3)

	(u* riplegend 0)

;; And heroes do all sorts of things normal folk don't

  (heroic-types riddle 3)
  (self-types riddle 3)


  (heroic-types riplegend 1)
  (heroic-types (tentacle cutter whaler) 2)

;; Legends would normally have a fire attack to represent their bow (Which, of course, could only be bent by the legend, right before killing two-score of unwanted suitors) but the AI doesn't like that, so...

  (air-attack-trait air-types 2)

;; Crocs shouldn't attack nesting grounds, it goes against their morals, plus it screws up the AI.
  (croc dino1 0)
  (croc dino2 0)

	(place-types u* 0)
	(node-types u* 0)
	(needle u* 0)

	(wall-types u* 0)

;; Some enchantments cause damage, but none of them do so on their own
	(enchantment-types u* 0)

  (notcapture-types node-types 0)
  (notcapture-types ruin-types 0)

  (u* seed-types 0)
  (u* event-types 0)


;; Charm spells

	(charm-types u* 0)
	((charmbeast beguile) wild-types 1)
	((charm greatcharm enlighten beguile) basic-types 1)
	((charm greatcharm enlighten beguile) specific-types 1)
	((charmfish beguile) water-types 1)
	((charmfish beguile) amphibious-types 1)
	((charmfish beguile) deep-types 1)
	((charmfish beguile) kraken-types 1)
	(enlighten all-types 1)
	(enlighten heroic-types 0)
	(enlighten self-types 0)

	(explode-types all-types 1)

	(firing-types u* 0)

)

(table acp-to-defend
   (croc (plesiosaur tyrannosaur apatosaur pterodactyl hatchling wyrm) 0)
   (glass-types (cutter whaler) 0)

;; Ghouls sap energy

   (ghoul basic-types 3)
   (ghoul specific-types 3)
   (ghoul hero 2)

   ((royalferak ferak) u* 3)

   (breath u* 2)

  (ranged-types u* 0)
  (mage-types u* 0)
  (high-mage-types u* 0)

	(wall-types u* 1)
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
	(shot-types full-resist-types 0)
	(nosave-types full-resist-types 20)

;;
	((lbow obow ibow bow cutter) all-types 50)
	((lbow obow ibow bow cutter) (bow whaler cutter) 75)
	((lbow obow ibow bow cutter) longbow 25)
	((lbow obow ibow bow cutter) air-types 75)
	((lbow obow ibow bow cutter) basic-types 75)
	((lbow obow ibow bow cutter) specific-types 65)

	(longbow all-types 60)
	(longbow longbow 75)
	(longbow bow 90)
	(longbow air-types 85)
	(longbow basic-types 85)
	(longbow specific-types 75)

	((ranger lranger dranger oranger iranger) all-types 75)
	((ranger lranger dranger oranger iranger) air-types 90)
	((ranger lranger dranger oranger iranger) basic-types 90)
	((ranger lranger dranger oranger iranger) specific-types 90)

	((whaler cannon catapult) small-types 5)
	((whaler cannon catapult) medium-types 20)
	((whaler cannon catapult) large-types 40)
	((whaler cannon catapult) huge-types 65)
	((whaler cannon catapult) gargantuan-types 85)

	(mage-types all-types 50)
	(mage-types mage-types 75)
	(mage-types (dervish weaver) 85)
	(mage-types air-types 75)
	(mage-types basic-types 85)
	(mage-types specific-types 85)
	(mage-types spell-types 75)

	(high-mage-types all-types 75)
	(high-mage-types mage-types 85)
	(high-mage-types (dervish weaver) 95)
	(high-mage-types air-types 85)
	(high-mage-types basic-types 95)
	(high-mage-types specific-types 95)
	(high-mage-types spell-types 85)

	(high-mage-types riddle 50)

	((shaman dervish weaver) all-types 50)
	((shaman dervish weaver) mage-types 60)
	((shaman dervish weaver) (dervish weaver) 75)
	((shaman dervish weaver) air-types 70)
	((shaman dervish weaver) basic-types 80)
	((shaman dervish weaver) specific-types 80)
	((shaman dervish weaver) spell-types 70)

	(self-types all-types 90)

	(loremaster u* 80)
	(spawn u* 80)
;;	(hatchling u* 75)
;;	(wyrm u* 85)
;;	(gwyrm u* 95)

	(u* self-types 25)
;;	(dragon-types hero 50)
;;	(dragon-types wizard 50)

	(ranged-types place-types 100)
	(mage-types place-types 100)
	(ranged-types fortress-types 100)
	(mage-types fortress-types 100)

	(canticle u* 100)

	(u* guardward 20)

	(u* node-types 0)

	(u* wilderness-types 100)


)


(table fire-damage
	(u* u* 1d10)

	((ibow obow lbow dbow bow) u* 2d6)
	((ibow obow lbow dbow bow) no-defense-types 2d6)
	((ibow obow lbow dbow bow) low-defense-types 2d4)
	((ibow obow lbow dbow bow) mid-defense-types 2d3)
	((ibow obow lbow dbow bow) high-defense-types 1d3)
	((ibow obow lbow dbow bow) uber-defense-types 1d2)

	(longbow u* 2d8)
	(longbow no-defense-types 2d8)
	(longbow low-defense-types 2d6)
	(longbow mid-defense-types 2d5)
	(longbow high-defense-types 1d5)
	(longbow uber-defense-types 1d4)

	((lranger dranger ranger oranger iranger) u* 2d10+5)
	((lranger dranger ranger oranger iranger) no-defense-types 2d10+5)
	((lranger dranger ranger oranger iranger) low-defense-types 2d9+4)
	((lranger dranger ranger oranger iranger) mid-defense-types 2d8+3)
	((lranger dranger ranger oranger iranger) high-defense-types 1d8+2)
	((lranger dranger ranger oranger iranger) uber-defense-types 1d7+1)

	(mage-types u* 3d10)
	(mage-types no-resist-types 3d10)
	(mage-types low-resist-types 3d8)
	(mage-types mid-resist-types 3d6)
	(mage-types high-resist-types 2d6)
	(mage-types uber-resist-types 2d4)
	(mage-types full-resist-types 1d4)

	((dervish weaver shaman) u* 3d8)
	((dervish weaver shaman) no-resist-types 3d8)
	((dervish weaver shaman) low-resist-types 3d6)
	((dervish weaver shaman) mid-resist-types 3d4)
	((dervish weaver shaman) high-resist-types 2d4)
	((dervish weaver shaman) uber-resist-types 2d3)
	((dervish weaver shaman) uber-resist-types 1d3)

	((catapult ocatapult) u* 5d8)
	((catapult ocatapult) no-defense-types 5d8)
	((catapult ocatapult) low-defense-types 5d6)
	((catapult ocatapult) mid-defense-types 5d4)
	((catapult ocatapult) high-defense-types 4d4)
	((catapult ocatapult) uber-defense-types 4d3)

	(cannon u* 7d8+8)
	(cannon no-defense-types 7d8+8)
	(cannon low-defense-types 7d6+6)
	(cannon mid-defense-types 7d4+4)
	(cannon high-defense-types 6d4+2)
	(cannon uber-defense-types 6d3)

	((cutter whaler) place-types 1d4)

	(shot-types self-types 1d10)
	(mage-types spell-types 2d6)

	(high-mage-types u* 2d10+10)
	(high-mage-types no-resist-types 2d10+10)
	(high-mage-types low-resist-types 2d8+8)
	(high-mage-types mid-resist-types 2d6+6)
	(high-mage-types high-resist-types 1d6+4)
	(high-mage-types uber-resist-types 1d4+4)
	(high-mage-types full-resist-types 1d2+4)

	((oarchmage iarchmage larchmage archmage) u* 4d10+10)
	((oarchmage iarchmage larchmage archmage) no-resist-types 4d10+10)
	((oarchmage iarchmage larchmage archmage) low-resist-types 4d8+8)
	((oarchmage iarchmage larchmage archmage) mid-resist-types 4d6+6)
	((oarchmage iarchmage larchmage archmage) high-resist-types 3d6+4)
	((oarchmage iarchmage larchmage archmage) uber-resist-types 3d5+2)
	((oarchmage iarchmage larchmage archmage) full-resist-types 2d5+2)

	(loremaster u* 2d10)
	(spawn u* 2d10)
	(u* mold 1d2)
	(cutter u* 3d10)
	(whaler u* 5d12+3)

;;	((hatchling wyrm gwyrm) u* 5d10+20)
;;	((hatchling wyrm gwyrm) no-defense-types 5d10+20)
;;	((hatchling wyrm gwyrm) low-defense-types 5d8+15)
;;	((hatchling wyrm gwyrm) mid-defense-types 5d6+10)
;;	((hatchling wyrm gwyrm) high-defense-types 4d6+5)
;;	((hatchling wyrm gwyrm) uber-defense-types 4d5)


	(powerword u* 200)
	(powerword no-resist-types 200)
	(powerword low-resist-types 150)
	(powerword mid-resist-types 100)
	(powerword high-resist-types 50)
	(powerword uber-resist-types 20)
	(powerword full-resist-types 0)
	(powerword self-types 1d13)
	(powerword death-immune-types 0)

	(bonearrow u* 1d8)
	(bonearrow no-resist-types 1d8)
	(bonearrow low-resist-types 1d8)
	(bonearrow mid-resist-types 1d6)
	(bonearrow high-resist-types 1d4)
	(bonearrow uber-resist-types 1d2)
	(bonearrow full-resist-types 0)
	(bonearrow death-immune-types 0)

	(earthquake place-types 100)

	(disintegrate all-types 100)
	(disintegrate no-resist-types 100)
	(disintegrate low-resist-types 75)
	(disintegrate mid-resist-types 50)
	(disintegrate high-resist-types 25)
	(disintegrate uber-resist-types 10)
	(disintegrate full-resist-types 0)
	(disintegrate place-types 25)
	(disintegrate fortress-types 50)
	(disintegrate chaos-immune-types 0)

	(flamestrike u* 3d10+10)
	(flamestrike no-resist-types 3d10+10)
	(flamestrike low-resist-types 3d8+8)
	(flamestrike mid-resist-types 3d5+5)
	(flamestrike high-resist-types 3d3+3)
	(flamestrike uber-resist-types 1d3+1)
	(flamestrike full-resist-types 0)
	(flamestrike chaos-immune-types 0)

	(flamearrow u* 1d10)
	(flamearrow no-resist-types 1d10)
	(flamearrow low-resist-types 1d8)
	(flamearrow mid-resist-types 1d5)
	(flamearrow high-resist-types 1d3)
	(flamearrow uber-resist-types 1)
	(flamearrow full-resist-types 0)
	(flamearrow chaos-immune-types 0)

	(sunray u* 3d10)
	(sunray no-resist-types 3d10)
	(sunray low-resist-types 3d8)
	(sunray mid-resist-types 3d5)
	(sunray high-resist-types 3d3)
	(sunray uber-resist-types 1d3)
	(sunray full-resist-types 0)
	(sunray death-immune-types 50)
	(sunray life-immune-types 0)

	(bolt u* 1d10)
	(bolt no-resist-types 1d10)
	(bolt low-resist-types 1d8)
	(bolt mid-resist-types 1d5)
	(bolt high-resist-types 1d3)
	(bolt uber-resist-types 1)
	(bolt full-resist-types 0)
	(bolt death-immune-types 4d10)
	(bolt life-immune-types 0)

	(lightning u* 5d10)
	(lightning no-resist-types 5d10)
	(lightning low-resist-types 5d8)
	(lightning mid-resist-types 5d5)
	(lightning high-resist-types 5d3)
	(lightning uber-resist-types 1d5)
	(lightning full-resist-types 0)
	(lightning chaos-immune-types 0)

	(canticle u* 2d13)
	(canticle no-resist-types 2d13)
	(canticle low-resist-types 2d10)
	(canticle mid-resist-types 2d7)
	(canticle high-resist-types 2d4)
	(canticle uber-resist-types 1d4)
	(canticle full-resist-types 0)
	(canticle chaos-immune-types 0)

;;	(u* wilderness-types 1d2)

;;	(u* self-types 1d6)
;;	(u* epic 1d10)
;;	(u* legend 1d9)
;;	(dragon-types self-types 2d10)
;;	(dragon-types hero 2d10)
;;	(dragon-types wizard 2d10)
;;	(dragon-types archmage 2d10)
;;	(dragon-types legend 2d10)
;;	(dragon-types epic 2d10)

;; Walls

	(summon-types prismwall 1d4)
	(dragon-types prismwall 1d4)
	(shot-types prismwall 1d4)
	(mage-types prismwall 1d4)
	(heroic-types prismwall 1d4)

	(deep-types flamewall 1)
	(amphibious-types flamewall 1)
	(water-types flamewall 1)

	(death-types spiritwall 1)
	(death-summon-types spiritwall 1)

	(u* guardward 1)

	(disintegrate self-types 0)
)





(table hit-chance
	(u* u* 0)
	(weak-types all-types 50)
	(weak-types weak-types 75)
	(weak-types strong-types 60)
	(weak-types powerful-types 40)
	(weak-types incredible-types 30)
	(weak-types ranged-types 75)
	(weak-types place-types 75)
	(weak-types fortress-types 75)

	(strong-types all-types 60)
	(strong-types weak-types 85)
	(strong-types strong-types 75)
	(strong-types powerful-types 60)
	(strong-types incredible-types 40)
	(strong-types ranged-types 85)
	(strong-types place-types 85)
	(strong-types fortress-types 85)

	(powerful-types all-types 70)
	(powerful-types weak-types 95)
	(powerful-types strong-types 85)
	(powerful-types powerful-types 75)
	(powerful-types incredible-types 50)
	(powerful-types ranged-types 95)
	(powerful-types place-types 95)
	(powerful-types fortress-types 95)

	(incredible-types all-types 80)
	(incredible-types weak-types 95)
	(incredible-types strong-types 95)
	(incredible-types powerful-types 80)
	(incredible-types incredible-types 75)
	(incredible-types ranged-types 95)
	(incredible-types place-types 95)
	(incredible-types fortress-types 95)

	(all-types wilderness-types 100)
	(all-types node-types 100)
	(all-types resource-types 100)

	(air-types air-types 75)
	(air-types node-types 25)

	(flamewall all-types 75)
	(prismwall all-types 50)

	(assassin humanoid-types 100)
	(assassin king 65)
	(croc (plesiosaur tyrannosaur apatosaur pterodactyl hatchling wyrm) 100)

	(notcapture-types node-types 0)
	(notcapture-types ruin-types 0)

	(tsunami place-types 100)

	(breath u* 50)

;; Dragons
;;	(hatchling heroic-types 30)
;;	(wyrm heroic-types 40)
;;	(gwyrm heroic-types 50)

	(heroic-types dragonegg 100)
	(croc dragonegg 100)

	(heroic-types riddle 50)
	(self-types riddle 25)

	(all-types guardward 20)

  (croc dino1 0)
  (croc dino2 0)

	(heroic-types riplegend 100)

	(charm-types wild-types 90)
	(charm-types basic-types 85)
	(charm-types specific-types 75)
	(charm-types water-types 80)
	(charm-types amphibious-types 75)
	(charm-types deep-types 70)
	(charm-types kraken-types 60)
	(charm-types summon-types 50)
	(charm-types dragon-types 35)
	(charm-types full-resist-types 0)

	(explode-types all-types 100)

	(firing-types u* 0)

)


(table damage
	(u* u* 0)

	(one-nodef-damage-types u* 1)
	(two-nodef-damage-types u* 1d4)
	(three-nodef-damage-types u* 1d8)
	(four-nodef-damage-types u* 1d15)
	(five-nodef-damage-types u* 2d10)
	(six-nodef-damage-types u* 2d14)
	(seven-nodef-damage-types u* 3d9)
	(eight-nodef-damage-types u* 3d12)
	(nine-nodef-damage-types u* 4d9)
	(ten-nodef-damage-types u* 5d9)
	(eleven-nodef-damage-types u* 6d12)
	(twelve-nodef-damage-types u* 7d14+20)
	(thirteen-nodef-damage-types u* 6d13+50)

	(one-damage-types u* 1d6+2)
	(one-damage-types no-defense-types 1d6+2)
	(one-damage-types low-defense-types 1d5+2)
	(one-damage-types mid-defense-types 1d4+1)
	(one-damage-types high-defense-types 1d3+1)
	(one-damage-types uber-defense-types 1d2)

	(two-damage-types u* 1d8+2)
	(two-damage-types no-defense-types 1d8+2)
	(two-damage-types low-defense-types 1d7+2)
	(two-damage-types mid-defense-types 1d6+1)
	(two-damage-types high-defense-types 1d5+1)
	(two-damage-types uber-defense-types 1d4)

	(three-damage-types u* 1d10+2) 
	(three-damage-types no-defense-types 1d10+2) 
	(three-damage-types low-defense-types 1d8+2) 
	(three-damage-types mid-defense-types 1d6+1) 
	(three-damage-types high-defense-types 1d5+1) 
	(three-damage-types uber-defense-types 1d4) 

	(four-damage-types u* 1d12+4)
	(four-damage-types no-defense-types 1d12+4)
	(four-damage-types low-defense-types 1d10+3)
	(four-damage-types mid-defense-types 1d8+3)
	(four-damage-types high-defense-types 1d6+2)
	(four-damage-types uber-defense-types 1d5+1)

	(five-damage-types u* 2d8+2)
	(five-damage-types no-defense-types 2d8+2)
	(five-damage-types low-defense-types 2d6+2)
	(five-damage-types mid-defense-types 2d5+1)
	(five-damage-types high-defense-types 1d5+1)
	(five-damage-types uber-defense-types 1d4)

	(six-damage-types u* 3d7+4)
	(six-damage-types no-defense-types 3d7+4)
	(six-damage-types low-defense-types 3d6+4)
	(six-damage-types mid-defense-types 3d5+3)
	(six-damage-types high-defense-types 2d5+2)
	(six-damage-types uber-defense-types 2d4+1)

	(seven-damage-types u* 4d8+3)
	(seven-damage-types no-defense-types 4d8+3)
	(seven-damage-types low-defense-types 4d7+3)
	(seven-damage-types mid-defense-types 4d6+2)
	(seven-damage-types high-defense-types 3d6+2)
	(seven-damage-types uber-defense-types 3d5+1)

	(eight-damage-types u* 5d10)
	(eight-damage-types no-defense-types 5d10)
	(eight-damage-types low-defense-types 5d8)
	(eight-damage-types mid-defense-types 5d6)
	(eight-damage-types high-defense-types 4d6)
	(eight-damage-types uber-defense-types 4d4)

	(nine-damage-types u* 6d13)
	(nine-damage-types no-defense-types 6d13)
	(nine-damage-types low-defense-types 6d11)
	(nine-damage-types mid-defense-types 6d9)
	(nine-damage-types high-defense-types 5d9)
	(nine-damage-types uber-defense-types 5d7)


;;; SPECIAL RULES 

	(phantomwarrior phantom-types 2d7)
	(phantombeast phantom-types 3d7)
	(phantomship phantom-types 3d6)
	(phantomshoggoth phantom-types 4d6)
	(nameless phantom-types 5d7)

	(vampyre crusader 1d8)
	(ghost crusader 1d9)
	(ghoul crusader 1d5)
	(zombie crusader 1d4)
	(skeleton crusader 1d4)
	(deathknight crusader 1d4+3)
	(demon crusader 3d3)

	(hunter-trait wild-types 2d10+10)

	(wild-types hunter-trait 1d5)
	((plesiosaur tyrannosaur apatosaur pterodactyl hatchling wyrm) croc 1d5)
	(assassin humanoid-types 100)

	(flamewall all-types 3d10)
	(prismwall all-types 2d13)

;;	(incredible-types self-types 1d12+5)
;;	(powerful-types self-types 1d10+3)
;;	(strong-types self-types 1d9)
;;	(weak-types self-types 1d6)

	(greyone crusader 1d10)

	(all-types mold 1d2)

;; Walls

	(summon-types prismwall 1d4)
	(dragon-types prismwall 1d4)
	(shot-types prismwall 1d4)
	(mage-types prismwall 1d4)
	(heroic-types prismwall 1d4)

	(deep-types flamewall 1)
	(amphibious-types flamewall 1)
	(water-types flamewall 1)

	(death-types spiritwall 1)
	(death-summon-types spiritwall 1)

	(all-types guardward 1)


	(tsunami place-types 100)
	(u* wilderness-types 1d3)

	(breath u* 1)

	(croc dino1 0)
	(croc dino2 0)


	(heroic-types riplegend 100)

	(explode-types all-types 1)

	(firing-types u* 0)

)

(table capture-chance
	(humanoid-types (cutter whaler) 50)
	(heroic-types (cutter whaler) 80)

	(slaver humanoid-types 35)
	(spawn humanoid-types 50)
	(vampyre humanoid-types 25)
	(vampyre ghoul 100)
	(sludge humanoid-types 75)
	((hatchling wyrm gwyrm scout whaler cutter bow longbow slaver) corpse-types 100)

	(zombie zombie 100)
	(greyone death-types 100)
	(greyone vampyre 100)
	(greyone (ghoul skeleton vampyre seeker iblis lich deathknight demon) 100)

	(spectre death-types 100)
	(spectre vampyre 100)
	(spectre (ghoul skeleton vampyre seeker deathknight) 100)

	(deathknight skeleton 100)

	(vampyre (vampyre deathknight) 100)

	(lich deathknight 100)

	(u* ruin-types 100)
	(u* place-types 75)

;; Big guys are clumsy
	(huge-types place-types 50)
	(gargantuan-types place-types 33)

	(u* node-types 75)
	((cutter whaler) place-types 0)
	(wild-types place-types 0)
	(u* deadcity 1)
	(u* resource-types 75)
	(u* self-types 0)

	(phantomwarrior phantompavilion 75)	
	(assassin king 100)
	(crusader usurper 100)
	((charm beguile) humanoid-types 100)	
	((charmfish beguile) (bospallian dajawyrm axehead squid) 33)
	((charmbeast beguile) (wolves bears ferak royalferak) 45)
	(enlighten all-types 100)
	(enlighten heroic-types 25)
	(enlighten self-types 0)
	(prismwall all-types 10)
	(prismwall self-types 0)
	(greatcharm humanoid-types 100)
	(emperor tribesman 100)
	(gladiator tribesman 100)
	(u* fortress-types 0)
	(shot-types u* 0)
	(explode-types u* 0)
	(enchantment-types u* 0)
	(notcapture-types u* 0)

	(death-immune-types place-types 1)

	(death-immune-types deadcity 100)

	(bospallian fish 100)
	(axehead fish 100)

	(heroic-types dragonegg 100)
	(croc dragonegg 100)
	(croc (plesiosaur tyrannosaur apatosaur pterodactyl) 40)
	(croc hatchling 50)
	(croc wyrm 20)

	(breath u* 0)
)


(table retreat-chance

	(incredible-types spear 75)
	(powerful-types spear 50)
	(strong-types spear 30)

	(incredible-types royalferak 100)
	(powerful-types royalferak 50)

	(incredible-types scout 80)
	(powerful-types scout 70)
	(strong-types scout 60)
	(weak-types scout 50)

	(u* u* 1)
)

(table acp-for-retreat
;;  (u* u* 1)
;;  (spear u* 2)
;;  (ferak u* 1)
;;  (royalferak u* 2)
;;  (scout u* 1)
  )


;;;;;;; EXPERIENCE


(define a-xp-types (sword bow spear catapult scout cannon earth1 fire1 air1 skeleton spear wolves seeker
   tribesman dbow obow ferak spirit1 crebain guardian poltergeist
   ibow
))

(define b-xp-types (halberd cavalry gladiator crusader longbow zombie shaman bears hellhound thuvi oslt
   lsword dsword osword merc croc churambi gibambi
   isword ihalberd
))

(define c-xp-types (mage assassin lmage dervish weaver earth2 fire2 spectre ghoul thug phantomwarrior oschief
   dhalberd lhalberd lcavalry squid tentacle mold sludge plesiosaur cutter whaler phantomship royalferak
   pterodactyl spirit2 ghost 
   image
))   

(define d-xp-types (champ apatosaur phantombeast deathknight woodgolem stonegolem irongolem clockgolem 
   oswarlord ulitar slaver sorcerer bospallian 
))
   
(define e-xp-types (earth3 fire3 air3 spirit3 lich vampyre dajawyrm dajaturtle hatchling
))

(define f-xp-types (king usurper minister shogun loremaster emperor shah pharoah greyone talon spawn wocke hero
   dhero ohero lhero dranger oranger lranger lwizard owizard chaoscreature demon knight oknight
   phantomshoggoth talon spawn leviathan wyrm
   ihero iranger iwizard
))

(define g-xp-types (epic depic oepic lepic archmage larchmage oarchmage nameless
   iepic
))
 
(define h-xp-types (tyrannosaur iblis legend dlegend olegend llegend thurastes gwyrm
   ilegend
))

(define hero-types (sword halberd bow cavalry scout gladiator longbow crusader weaver mage dervish thuvi spear oslt oschief
   lsword lhalberd lbow lmage croc dsword dhalberd dbow
   osword ocavalry obow isword ihalberd image))

#|(table cxp-per-combat

  (hero-types a-xp-types 1)
  (hero-types b-xp-types 2)
  (hero-types c-xp-types 3)
  (hero-types d-xp-types 4)
  (hero-types e-xp-types 5)
  (hero-types f-xp-types 6)
  (hero-types g-xp-types 7)
  (hero-types h-xp-types 8)

  (hero-types place-types 1)
  (hero-types fortress-types 1)
  (hero-types node-types 1)
  (hero-types resource-types 1)


  	(heroic-types a-xp-types 2)
  	(heroic-types b-xp-types 3)
  	(heroic-types c-xp-types 5)
  	(heroic-types d-xp-types 6)
  	(heroic-types e-xp-types 8)
  	(heroic-types f-xp-types 10)
  	(heroic-types g-xp-types 15)
  	(heroic-types h-xp-types 20)

  	(heroic-types place-types 3)
  	(heroic-types fortress-types 3)
  	(heroic-types node-types 3)
  	(heroic-types resource-types 3)
)|#


(table cxp-per-destroy

  (hero-types a-xp-types 5)
  (hero-types b-xp-types 10)
  (hero-types c-xp-types 20)
  (hero-types d-xp-types 30)
  (hero-types e-xp-types 40)
  (hero-types f-xp-types 50)
  (hero-types g-xp-types 75)
  (hero-types h-xp-types 100)

  (hero-types place-types 20)
  (hero-types fortress-types 20)

  	(heroic-types a-xp-types 5)
  	(heroic-types b-xp-types 10)
  	(heroic-types c-xp-types 20)
  	(heroic-types d-xp-types 30)
  	(heroic-types e-xp-types 40)
  	(heroic-types f-xp-types 50)
  	(heroic-types g-xp-types 75)
  	(heroic-types h-xp-types 100)

  	(heroic-types place-types 20)
  	(heroic-types fortress-types 20)

	(u* riplegend 1000)

)


(table cxp-to-change-type
  (hero-types heroic-types 100)

  (heroic-types heroic-types 1000)
  (lranger larchmage 1000)

  ((iepic oepic lepic epic depic) (ilegend olegend dlegend llegend legend) 2000)

  ((iwizard owizard lwizard wizard) (iarchmage oarchmage larchmage archmage) 1500)

  (spear oslt 50)
  (oslt oschief 200)
  (oschief oswarlord 1000)

  ((gibambi gibsword gibmage gibpriest) gibhero 1000)

)

;; ZOC

(table mp-to-traverse-zoc
	(u* seed-types 0)
	(u* event-types 0)
	(u* riplegend 0)
  )

(table mp-to-enter-zoc
	(u* seed-types 0)
	(u* event-types 0)
	(u* riplegend 0)
	(u* lantern 0)

  )

(table mp-to-leave-zoc
	(u* seed-types 0)
	(u* event-types 0)
	(u* riplegend 0)
	(u* lantern 0)
)

(table mp-to-enter-own
	(u* seed-types 0)
	(u* event-types 0)
	(u* riplegend 0)
	(u* lantern 0)
)


;;; Transports will get you to the edge of the cell, but the
;;; passengers have to cross any borders on their own.




(table ferry-on-entry
	(u* u* 0)
	(cutter humanoid-types over-all)
	(ulitar sludge over-all)
	(dragonegg u* over-all)
)

(table ferry-on-departure
	(u* u* 0)
	(cutter humanoid-types over-all)
	(dragonegg u* over-all)

)

(table mp-to-leave-unit
	(sludge ulitar 99)
)


(table hp-min
	(u* ruin-types 1)
	(u* node-types 1)
	(u* resource-types 1)
	(u* riddle 1)
	(heroic-types riddle 0)
	(self-types riddle 0)

	(u* lantern 1)
)





(table protection
  ;; Ground units protect cities.
  (all-types all-types 50)

  (all-types place-types 0)
  (all-types node-types 0)
  (all-types fortress-types 100)
	(place-types enchantment-types 100)
	(fortress-types enchantment-types 100)
	(wall-types place-types 0)
	(place-types wall-types 100)

	(wall-types fortress-types 0)
	(fortress-types wall-types 100)

	(heroic-types high-mage-types 0)
	(high-mage-types (epic legend hero ranger knight) 100)

	(all-types phantompavilion 0)
	(phantompavilion all-types 100)

  )


;; THIS CAUSES ISSUES WITH CXP
;; (table defend-terrain-effect
;;  (land-types hills 85)
;;  (land-types forest 85)
;;  (land-types swamp 85)
;;  (land-types mountains 85)
;;)




(table occupant-combat
	(u* u* 0)
	(all-types fortress-types 100)
	(ranged-types fortress-types 100)
	(cutter (longbow bow sword thug champ) 100)
	(all-types place-types 100)
	(wall-types u* 100)
	(phantomwarrior phantompavilion 100)
)



(add explode-types detonate-with-attack true)

(table detonation-unit-range
	(fireball u* 1)
	(meteor u* 2)
	(wastemaker u* 1)
	(tsunami u* 2)
	(earthquake u* 2)
	(faultline u* 2)
)

(table detonation-damage-at
	(u* u* 0)
	(tsunami place-types 200)

	(fireball u* 14)
	(fireball no-resist-types 14)
	(fireball low-resist-types 11)
	(fireball mid-resist-types 7)
	(fireball high-resist-types 4)
	(fireball uber-resist-types 2)
	(fireball full-resist-types 0)
	(fireball chaos-immune-types 0)

	(meteor u* 33)
	(meteor no-resist-types 33)
	(meteor low-resist-types 25)
	(meteor mid-resist-types 17)
	(meteor high-resist-types 8)
	(meteor uber-resist-types 4)
	(meteor full-resist-types 0)
	(meteor chaos-immune-types 0)

	(wastemaker humanoid-types 200)
	(wastemaker place-types 200)	
	(wastemaker fortress-types 200)	

	(earthquake place-types 50)
	(earthquake fortress-types 200)
	(earthquake volcano 200)

	(faultline land-types 50)
	(faultline place-types 50)
	(faultline fortress-types 200)
	(faultline volcano 200)

	(u* self-types 0)

)

(table detonation-damage-adjacent
	(u* u* 0)
	(fireball u* 14)
	(fireball no-resist-types 14)
	(fireball low-resist-types 11)
	(fireball mid-resist-types 7)
	(fireball high-resist-types 4)
	(fireball uber-resist-types 2)
	(fireball full-resist-types 0)
	(fireball chaos-immune-types 0)

	(meteor u* 23)
	(meteor no-resist-types 23)
	(meteor low-resist-types 18)
	(meteor mid-resist-types 12)
	(meteor high-resist-types 6)
	(meteor uber-resist-types 3)
	(meteor full-resist-types 0)
	(meteor chaos-immune-types 0)

	(wastemaker humanoid-types 200)
	(wastemaker place-types 200)	
	(wastemaker fortress-types 200)	

	(earthquake place-types 50)
	(earthquake fortress-types 200)
	(earthquake volcano 200)

	(faultline land-types 50)
	(faultline place-types 50)
	(faultline fortress-types 200)
	(faultline volcano 200)

	(tsunami place-types 200)
	(u* self-types 0)

)

(table detonation-terrain-range
	(wastemaker (land steppe forest hills desert semi-desert) 1)
)


(table detonation-terrain-damage-chance
	(wastemaker (land steppe forest hills desert semi-desert mountains) 100)
)


(table terrain-damaged-type
	(land p-land 100)
	(steppe p-land 100)
	(forest p-forest 100)
	(hills p-hills 100)
	(desert p-land 100)
	(semi-desert p-land 100)
	(mountains p-mountains 100)
	(cliffs p-cliffs 100)
	(river p-river 100)
	(ice p-mountains 100)
	(swamp p-forest 100)
	(jungle p-forest 100)
	(deepforest p-forest 100)
	
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

;; ZOINK
(table wrecked-type-if-killed
	(humanoid-types death-types zombie)
	(humanoid-types death-types zombie)

	(humanoid-types deathknight skeleton)
	(humanoid-types deathknight skeleton)

	(humanoid-types vampyre ghoul)
	(humanoid-types vampyre ghoul)

	(heroic-types vampyre vampyre)

	(self-types vampyre vampyre)
	(humanoid-types greyone zombie)

	(heroic-types greyone spectre)
	((oepic depic epic lepic) greyone vampyre)
	((olegend dlegend llegend legend) greyone demon)
	(self-types greyone lich)

	(place-types death-types deadcity)
	(place-types greyone deadcity)
	(place-types ghoul deadcity)
	(place-types vampyre deadcity)
	(place-types lich deadcity)
	(place-types skeleton deadcity)
	(place-types deathknight deadcity)

	(crusader greyone deathknight)
	(crusader vampyre deathknight)
	(crusader lich deathknight)
	(crusader spectre deathknight)

	(citadel wastemaker castle)
	(keep wastemaker castle)
	(small-types wastemaker phantomwarrior)
	(medium-types wastemaker phantomwarrior)
	(large-types wastemaker phantombeast)
	(huge-types wastemaker phantomshoggoth)
	(gargantuan-types wastemaker nameless)
	(place-types wastemaker phantompavilion)
	((cutter whaler) wastemaker phantomship)
)



(table occupant-multiplies-acp
	(bless place-types 2.00)
	(sanctify fortress-types 2.00)
)

(table stack-neighbor-allows-capture-by
	(guardian u* 0)
)

(table unit-consumption-per-size
)

(table unit-consumption-to-grow
	(u* gold 9999)
)

(table material-to-fire
	(hatchling flames 3)
	(wyrm flames 2)
	(gwyrm flames 1)
;;	(cutter flames 1)
;;	(whaler flames 1)
	(mage-types mana 5)
	(high-mage-types charge 1)
)

(table consumption-per-fire
	(hatchling flames 3)
	(wyrm flames 2)
	(gwyrm flames 1)
;;	(cutter flames 1)
;;	(whaler flames 1)
	(mage-types mana 5)
	(high-mage-types charge 1)

)

(table consumption-per-attack
	(charm-types mana 1)
)

(table hit-by
	(u* flames true)
	(u* mana true)
	(u* charge true)
)

(table material-to-attack
	(charm-types mana 1)
)

;;(table acp-to-change-type
;;	(hatchling wyrm 1)
;;	(wyrm gwyrm 1)
;;	(crusader cavalry 1)
;;)

(table material-to-change-type
	(dragonegg timer 10)
	(hatchling timer 20)
	(wyrm timer 20)
)

(table out-length
  ;; Net consumers of supply should never give any up automatically.
	(bless m* 1)
	(mage-types study 2)
	(mage-types mana 0)
	(high-mage-types mana 0)

)

(table in-length
  ;; Supply to ground units can go several hexes away
	(place-types m* 1)
	(place-types study 2)
	(mage-types mana 0)
	(high-mage-types mana 0)
	(bless mana 1)
)




(add all-types lost-vanish-chance 0)

;;; The world.

;; (world 360 (year-length 12) (axial-tilt 22))


;; (set calendar '(usual month))

;; (set season-names
  ;; 0 is January, 3 is April, 6 is July, 9 is October
;;  ((0 2 "winter") (3 5 "spring") (6 8 "summer") (9 11 "autumn")))

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

	(lantern study 2000)
	(lantern timer 8)

)


(scorekeeper 1
  (title "Attrition Victory")
  (keep-score false)
  (applies-to (not "independent"))
  (do last-side-wins)
)

(set scorefile-name "1")

(doctrine rrr
    (construction-run (u* 1))
    (resupply-percent 0)
    (rearm-percent 0)
    (repair-percent 0)
)

(side-defaults
    (default-doctrine rrr)
;;    (doctrines
;;        (city-u* city-unit-doctrine)
;;        (base base-doctrine)
;;        (shipyard shipyard-doctrine)
;;	)
)


(set country-radius-max 5)
(set country-separation-min 15)
(set edge-terrain ice)

(add u* start-with 0)
(add sword start-with 2)
(add halberd start-with 1)
(add bow start-with 1)
(add cavalry start-with 2)
(add city3 start-with 2)
(add mage start-with 2)
(add hero start-with 1)
(add wizard start-with 1)
(add tower start-with 2)
(add scout start-with 3)
(add catapult start-with 1)
(add lantern start-with 1)

(table independent-density
  (faultline (land steppe forest semi-desert) (25 25 25 25))
  (banditseed (land steppe forest semi-desert) (65 50 50 25))
  (fortseed (hills steppe forest semi-desert) (50 25 25 15))
  (dragonseed (mountains hills forest steppe) (30 15 10 5))
  (heroseed (land steppe forest semi-desert desert jungle swamp) (5 5 5 5 5 5 5))
  (archseed (land steppe forest semi-desert desert hills swamp) (8 8 8 8 8 8 8))
  (rangerseed (land steppe forest semi-desert desert jungle swamp) (5 5 5 5 5 5 5))
  (knightseed (land steppe forest semi-desert desert hills swamp) (5 5 5 5 5 5 5))

  (riplegend (hills forest desert mountains swamp jungle) (1 1 1 1 1 1))

  (city1 (land steppe forest mountains hills semi-desert) (50 50 50 10 25 25))
  (city2 (land steppe forest mountains hills semi-desert) (25 25 25 10 25 25))
  (city3 (land steppe forest mountains hills semi-desert) (15 15 15 5 15 15))
  (city4 (land steppe forest mountains hills semi-desert) (25 25 25 10 25 25))
  (ohamlet (land steppe forest mountains hills semi-desert) (15 15 15 10 15 15))
  (ovillage (land steppe forest mountains hills semi-desert) (15 15 15 10 15 15))
  (citadel (land steppe forest mountains hills semi-desert) (15 15 15 15 15 15))
  (lab (land steppe forest mountains hills semi-desert) (20 20 20 20 20 20))

  (lcity (jungle hills swamp) (10 10 50))
  (dcity (steppe mountains hills semi-desert) (5 40 20 5))
  (ocity (steppe land) (30 30))
  (icity (steppe land jungle mountains hills) (20 20 40 10 10))

  (enode mountains 75)
  (cnode desert 75)
  (snode steppe 75)
  (lnode land 75)
  (dnode (forest hills swamp) (75 75 75))
  (temple (mountains desert steppe forest hills swamp) (20 20 20 20 20 20))

  (goldmine (hills mountains) (50 150))
  (ironmine (hills mountains wasteland) (50 150 100))
  (quarry (mountains hills) (150 150))
  (lumber (jungle forest deepforest) (150 150 300))

  (deadcity (wasteland desert) (50 10))
  (dino1 (land steppe forest hills) (7 7 7 7))
  (dino2 (land steppe forest hills) (7 7 7 7))
  (fheart forest 10)
  (jheart forest 10)
  (lair mountains 8)
  (pit sea 8)

  (thurastes sea 3)
  (squid sea 8)
  (dajawyrm sea 8)
  (dajaturtle sea 4)
  (gibambi sea 4)
  (churambi sea 4)
  (ulitar sea 4)
  (plesiosaur sea 4)
  (leviathan sea 4)
  (mold sea 4)
  (sludge sea 4)
  (tentacle sea 5)
  
  (zombie (desert forest) (30 30))
  (spectre (desert forest) (20 20))
  (vampyre (desert forest) (8 8))
  (lich (desert forest) (5 5))

  (iblis desert 3)
  (chaoscreature desert 3)
  (demon desert 8)

  (phantombeast steppe 4)
  (phantomwarrior steppe 8)

  (bears forest 60)
  (wolves forest 60)
  (tribesman forest 20)
  (ferak forest 60)
  (royalferak forest 60)


  (tyrannosaur (forest swamp) (5 10))
  (apatosaur (forest swamp) (7 12))
  (pterodactyl (forest swamp) (10 15))

)

(define cell-t* (sea shallows swamp desert semi-desert land forest deepforest jungle steppe wasteland hills mountains))


(add cell-t* alt-percentile-min 	(  	0  	5 	10  	15  	15	15  	45
   45	45	70	70	85  	85))

(add cell-t* alt-percentile-max 	( 	5  	10  	15  	45  	45	45  	70
   70	70	85	85	100 	100))

(add cell-t* wet-percentile-min 	(  	0   	0  	0   	0  	20	40   	0
   60	70   	0	90	0	33))

(add cell-t* wet-percentile-max 	(	100 	100 	100	20  	40	100 	60
   70	100	90	100	33 	100))

;;;	Some defns for the fractal percentile generator.

(set alt-blob-density 2000)
;;(set alt-blob-height 500)
(set alt-blob-size 100)
(set alt-smoothing 4)
(set wet-blob-density 2000)
(set wet-blob-size 100)
(set wet-smoothing 4)

(table adjacent-terrain-effect
	((semi-desert desert) (forest jungle deepforest swamp sea shallows) land)
	(desert (land steppe) semi-desert)
	(deepforest (land semi-desert jungle steppe) forest)
	(forest jungle land)
)

(table adjacent-terrain-border
	(mountains (land steppe forest swamp desert semi-desert wasteland) cliffs)
)

(table adjacent-terrain-border-chance
	(mountains (land steppe forest swamp desert semi-desert wasteland) 20)
)

(table favored-terrain
  (land-types land-t* 100)
  (deep-types sea 100)
  (amphibious-types t* 100)
  (water-types sea 100)
  (air-types t* 100)
  (sludge sea 0)
  (lantern t* 100)
)



;; DEFINE PLURAL TYPES AND SINGLE TYPES

(set action-notices '(

	((capture u* place-types) (actee " is taken."))
	((destroy u* place-types) (actee " falls."))

	((capture u* heroic-types) (actee " joins your cause!"))

	((create-at u* spell-types) (actor " casts " actee "!"))
	((create-at u* summon-types) (actor " begins summoning " actee "!"))

	((unit-completed summon-types) (actor " has been successfully summoned!"))

	((unit-type-changed hero-types hero) (actor " have performed such heroic deeds to be renowned throughout the land as heroes!"))
	((unit-type-changed hero-types knight) (actor " are the scourge of their enemies and named knights by their lord!"))
	((unit-type-changed hero-types ranger) (actor " have assumed the mantle of the ranger."))



	((change-type hero-types hero) (actor " have performed such heroic deeds to be renowned throughout the land as a heroes!"))
	((change-type hero-types knight) (actor " are the scourge of their enemies and named knights by their lord!"))
	((change-type hero-types ranger) (actor " have assumed the mantle of the ranger."))

	((fire-at mage-types u*) (actor " wracks " actee " with magick energy!"))
	((fire-at (wizard archmage) u*) (actor " wracks " actee " with eldritch energy!"))
	((fire-into mage-types u*) (actor " wracks " actee " with magick energy!"))
	((fire-into (wizard archmage) u*) (actor " wracks " actee " with eldritch energy!"))


	((fire dragon-types u*) (actor " breathes stone-cracking fire upon " actee "!"))
	((destroy dragon-types u*) (actor " eats " actee "!"))
	((capture charm-types u*) (actee " is beguiled through magic!"))
	((retreat scout u*) (actor " sneaks away from " actee "!"))
	((retreat scout u*) (actor " sneaks away from " actee "!"))

	((destroy death-types basic-types) (actor " turns " actee " into zombies!"))
	((destroy death-types specific-types) (actor " turns " actee " into zombies!"))

	((destroy deathknight basic-types) (actor " reanimates dead " actee " as skeletons!"))
	((destroy deathknight specific-types) (actor " reanimates dead " actee " as skeletons!"))

	((destroy vampyre basic-types) (actor " turns " actee " into ghouls!"))
	((destroy vampyre specific-types) (actor " turns " actee " into ghouls!"))

	((capture vampyre basic-types) (actor " dominates " actee "!"))
	((capture vampyre specific-types) (actor " dominates " actee "!"))

	((destroy vampyre (hero epic legend)) (actor " drains " actee " of life, creating another vampyre!"))
	((destroy vampyre self-types) (actor " drains " actee " of life, creating another vampyre!"))

	((destroy greyone basic-types) ("The evil one supplements his army with more undead followers!"))
	((destroy greyone specific-types) ("The evil one supplements his army with more undead followers!"))
	((destroy greyone (hero ranger knight)) ("The black lord makes another lieutenant out of " actee "!"))
	((destroy greyone epic) ("The thrice-damned creates a new servent out of the broken body of " actee "!"))
	((destroy greyone legend) ("Using foul arts, the Grey One sacrifices the soul of " actee " for the services of a demon!"))
	((destroy greyone self-types (actee " becomes more amenable to the Master of the Dead after joining his ranks!"))

	((destroy death-types place-types) (actee " is overrun by the undead and infested with evil energies!"))
	((destroy (greyone ghoul vampyre lich skeleton deathknight) place-types) (actee " is overrun by the undead and infested with evil energies!"))
	((destroy (greyone vampyre lich spectre) deathknight) (actee " is corrupted by darkness!"))

	((destroy wastemaker (keep citadel)) ("The essence of phantom changes " actee " into a mystical place!"))
	((destroy wastemaker place-types) ("The essence of phantom turns " actee " into another pavilion of the Phantom Dance!"))
	((destroy wastemaker small-types) ("The essence of phantom reduces " actee " into a flock of Phantom Birds!"))
	((destroy wastemaker medium-types) ("The essence of phantom fades " actee " until they become Phantom Warriors!"))
	((destroy wastemaker large-types) ("The essence of phantom fades " actee " into a great Phantom Beast!"))
	((destroy wastemaker huge-types) ("The essence of phantom turns " actee " into a Phantom Giant!"))
	((destroy wastemaker gargantuan-types) ("The essence of phantom overwhelms " actee ", leaving in its place a Phantom Shoggoth!"))	
	((destroy wastemaker (cutter whaler)) ("The essence of phantom weaves the " actee " into a Phantom Ship!"))

))
)


(game-module (design-notes
  "It is ambitious"
  ))

(imf "ang-hunchback-buff" ((32 32) (file "dg_monster132.gif" std 0 0)))
(imf "ang-gnome-robe" ((32 32) (file "dg_monster132.gif" std 0 1)))
(imf "ang-troll-sword" ((32 32) (file "dg_monster132.gif" std 0 2)))
(imf "ang-hobgoblin-fist2" ((32 32) (file "dg_monster132.gif" std 0 3)))
(imf "ang-mouse-white" ((32 32) (file "dg_monster132.gif" std 0 4)))
(imf "ang-demon-staff" ((32 32) (file "dg_monster132.gif" std 0 5)))
(imf "ang-devil-arms" ((32 32) (file "dg_monster132.gif" std 0 6)))
(imf "ang-devil-orange" ((32 32) (file "dg_monster132.gif" std 0 7)))
(imf "ang-codhead-ankh" ((32 32) (file "dg_monster132.gif" std 0 8)))
(imf "ang-longhead-sword" ((32 32) (file "dg_monster132.gif" std 0 9)))
(imf "ang-fishhead-sword" ((32 32) (file "dg_monster132.gif" std 0 10)))
(imf "ang-3eye-ankh" ((32 32) (file "dg_monster132.gif" std 0 11)))
(imf "ang-wolfman-ankh" ((32 32) (file "dg_monster132.gif" std 0 12)))
(imf "ang-cyclops-ankh" ((32 32) (file "dg_monster132.gif" std 0 13)))
(imf "ang-gnoll-ankh" ((32 32) (file "dg_monster132.gif" std 0 14)))
(imf "ang-goblin-ankh" ((32 32) (file "dg_monster132.gif" std 0 15)))

(imf "ang-drow-kirk" ((32 32) (file "dg_monster132.gif" std 1 0)))
(imf "ang-drow-purple" ((32 32) (file "dg_monster132.gif" std 1 1)))
(imf "ang-troll-greatsword" ((32 32) (file "dg_monster132.gif" std 1 2)))
(imf "ang-goblin-barb" ((32 32) (file "dg_monster132.gif" std 1 3)))
(imf "ang-mouse-gray" ((32 32) (file "dg_monster132.gif" std 1 4)))
(imf "ang-demon-dagger" ((32 32) (file "dg_monster132.gif" std 1 5)))
(imf "ang-devil-sword" ((32 32) (file "dg_monster132.gif" std 1 6)))
(imf "ang-devil-orange-sword" ((32 32) (file "dg_monster132.gif" std 1 7)))
(imf "ang-codhead-sword" ((32 32) (file "dg_monster132.gif" std 1 8)))
(imf "ang-longhead-ankh" ((32 32) (file "dg_monster132.gif" std 1 9)))
(imf "ang-fishhead-ankh" ((32 32) (file "dg_monster132.gif" std 1 10)))
(imf "ang-3eye-staff" ((32 32) (file "dg_monster132.gif" std 1 11)))
(imf "ang-wolfman-staff" ((32 32) (file "dg_monster132.gif" std 1 12)))
(imf "ang-cyclops-glowhand" ((32 32) (file "dg_monster132.gif" std 1 13)))
(imf "ang-gnoll-glowhand" ((32 32) (file "dg_monster132.gif" std 1 14)))
(imf "ang-goblin-staff" ((32 32) (file "dg_monster132.gif" std 1 15)))

(imf "ang-drow-glowhand" ((32 32) (file "dg_monster132.gif" std 2 0)))
(imf "ang-drow-staff" ((32 32) (file "dg_monster132.gif" std 2 1)))
(imf "ang-hobgoblin-buff" ((32 32) (file "dg_monster132.gif" std 2 2)))
(imf "ang-goblin-club" ((32 32) (file "dg_monster132.gif" std 2 3)))
(imf "ang-mouse-brown" ((32 32) (file "dg_monster132.gif" std 2 4)))
(imf "ang-demon-buff" ((32 32) (file "dg_monster132.gif" std 2 5)))
(imf "ang-devil-sword2" ((32 32) (file "dg_monster132.gif" std 2 6)))
(imf "ang-devil-orange-scimitar" ((32 32) (file "dg_monster132.gif" std 2 7)))
(imf "ang-codhead-cross" ((32 32) (file "dg_monster132.gif" std 2 8)))
(imf "ang-longhead-dagger" ((32 32) (file "dg_monster132.gif" std 2 9)))
(imf "ang-fishhead-staff" ((32 32) (file "dg_monster132.gif" std 2 10)))
(imf "ang-3eye-dagger" ((32 32) (file "dg_monster132.gif" std 2 11)))
(imf "ang-wolfman-cape" ((32 32) (file "dg_monster132.gif" std 2 12)))
(imf "ang-cyclops-staff" ((32 32) (file "dg_monster132.gif" std 2 13)))
(imf "ang-gnoll-dagger" ((32 32) (file "dg_monster132.gif" std 2 14)))
(imf "ang-goblin-cape" ((32 32) (file "dg_monster132.gif" std 2 15)))

(imf "ang-drow-gold" ((32 32) (file "dg_monster132.gif" std 3 0)))
(imf "ang-drow-beard" ((32 32) (file "dg_monster132.gif" std 3 1)))
(imf "ang-hobgoblin-black" ((32 32) (file "dg_monster132.gif" std 3 2)))
(imf "ang-minotaur-buff" ((32 32) (file "dg_monster132.gif" std 3 3)))
(imf "ang-mouse-orange" ((32 32) (file "dg_monster132.gif" std 3 4)))
(imf "ang-demon-sword" ((32 32) (file "dg_monster132.gif" std 3 5)))
(imf "ang-devil-shield" ((32 32) (file "dg_monster132.gif" std 3 6)))
(imf "ang-devil-orange-ankh" ((32 32) (file "dg_monster132.gif" std 3 7)))
(imf "ang-codhead-staff" ((32 32) (file "dg_monster132.gif" std 3 8)))
(imf "ang-longhead-glowhand2" ((32 32) (file "dg_monster132.gif" std 3 9)))
(imf "ang-fishhead-glowhand" ((32 32) (file "dg_monster132.gif" std 3 10)))
(imf "ang-3eye-sword" ((32 32) (file "dg_monster132.gif" std 3 11)))
(imf "ang-wolfman-cross" ((32 32) (file "dg_monster132.gif" std 3 12)))
(imf "ang-cyclops-sword" ((32 32) (file "dg_monster132.gif" std 3 13)))
(imf "ang-gnoll-monk" ((32 32) (file "dg_monster132.gif" std 3 14)))
(imf "ang-goblin-2scimitar" ((32 32) (file "dg_monster132.gif" std 3 15)))

(imf "ang-gnome-2glowhand" ((32 32) (file "dg_monster132.gif" std 4 0)))
(imf "ang-drow-2glowhand" ((32 32) (file "dg_monster132.gif" std 4 1)))
(imf "ang-hobgoblin-fist" ((32 32) (file "dg_monster132.gif" std 4 2)))
(imf "ang-minotaur-axe" ((32 32) (file "dg_monster132.gif" std 4 3)))
(imf "ang-ettin" ((32 32) (file "dg_monster132.gif" std 4 4)))
(imf "ang-demon-shield" ((32 32) (file "dg_monster132.gif" std 4 5)))
(imf "ang-devil-staff" ((32 32) (file "dg_monster132.gif" std 4 6)))
(imf "ang-devil-orange-staff" ((32 32) (file "dg_monster132.gif" std 4 7)))
(imf "ang-codhead-staff2" ((32 32) (file "dg_monster132.gif" std 4 8)))
(imf "ang-longhead-sword2" ((32 32) (file "dg_monster132.gif" std 4 9)))
(imf "ang-fishhead-sword2" ((32 32) (file "dg_monster132.gif" std 4 10)))
(imf "ang-3eye-shield" ((32 32) (file "dg_monster132.gif" std 4 11)))
(imf "ang-wolfman-sword2" ((32 32) (file "dg_monster132.gif" std 4 12)))
(imf "ang-cyclops-cross" ((32 32) (file "dg_monster132.gif" std 4 13)))
(imf "ang-gnoll-shield" ((32 32) (file "dg_monster132.gif" std 4 14)))
(imf "ang-goblin-shield" ((32 32) (file "dg_monster132.gif" std 4 15)))

(imf "ang-drow-ankh" ((32 32) (file "dg_monster132.gif" std 5 0)))
(imf "ang-hunchback-green" ((32 32) (file "dg_monster132.gif" std 5 1)))
(imf "ang-hobgoblin-staff" ((32 32) (file "dg_monster132.gif" std 5 2)))
(imf "ang-minotaur-sword" ((32 32) (file "dg_monster132.gif" std 5 3)))
(imf "ang-cyclops-4arm" ((32 32) (file "dg_monster132.gif" std 5 4)))
(imf "ang-demon-glowhand" ((32 32) (file "dg_monster132.gif" std 5 5)))
(imf "ang-devil-glowhand" ((32 32) (file "dg_monster132.gif" std 5 6)))
(imf "ang-devil-orange-2glowhand" ((32 32) (file "dg_monster132.gif" std 5 7)))
(imf "ang-codhead-sword2" ((32 32) (file "dg_monster132.gif" std 5 8)))
(imf "ang-longhead-glowhand" ((32 32) (file "dg_monster132.gif" std 5 9)))
(imf "ang-fishhead-shield" ((32 32) (file "dg_monster132.gif" std 5 10)))
(imf "ang-3eye-staff2" ((32 32) (file "dg_monster132.gif" std 5 11)))
(imf "ang-wolfman-sword3" ((32 32) (file "dg_monster132.gif" std 5 12)))
(imf "ang-cyclops-sword2" ((32 32) (file "dg_monster132.gif" std 5 13)))
(imf "ang-gnoll-sword2" ((32 32) (file "dg_monster132.gif" std 5 14)))
(imf "ang-goblin-sword2" ((32 32) (file "dg_monster132.gif" std 5 15)))

(imf "ang-ooze-green" ((32 32) (file "dg_monster332.gif" std 0 0)))
(imf "ang-jellyfish-blue" ((32 32) (file "dg_monster332.gif" std 0 1)))
(imf "ang-jellyfish-maroon" ((32 32) (file "dg_monster332.gif" std 0 2)))
(imf "ang-jellyfish-pink" ((32 32) (file "dg_monster332.gif" std 0 3)))
(imf "ang-planer-fire" ((32 32) (file "dg_monster332.gif" std 0 4)))
(imf "ang-ent-beige" ((32 32) (file "dg_monster332.gif" std 0 5)))
(imf "ang-ent-green" ((32 32) (file "dg_monster332.gif" std 0 6)))
(imf "ang-bug-beetle-white" ((32 32) (file "dg_monster332.gif" std 0 7)))
(imf "ang-bug-fly-head" ((32 32) (file "dg_monster332.gif" std 0 8)))
(imf "ang-spider-red2" ((32 32) (file "dg_monster332.gif" std 0 9)))
(imf "ang-marmot-green" ((32 32) (file "dg_monster332.gif" std 0 10)))
(imf "ang-horsehead-glowhand" ((32 32) (file "dg_monster332.gif" std 0 11)))
(imf "ang-dog-brown" ((32 32) (file "dg_monster332.gif" std 0 12)))

(imf "ang-ooze-blue" ((32 32) (file "dg_monster332.gif" std 1 0)))
(imf "ang-jellyfish-orange" ((32 32) (file "dg_monster332.gif" std 1 1)))
(imf "ang-jellyfish-black" ((32 32) (file "dg_monster332.gif" std 1 2)))
(imf "ang-jellyfish-white" ((32 32) (file "dg_monster332.gif" std 1 3)))
(imf "ang-planer-mystic" ((32 32) (file "dg_monster332.gif" std 1 4)))
(imf "ang-ent-purple" ((32 32) (file "dg_monster332.gif" std 1 5)))
(imf "ang-ent-pink" ((32 32) (file "dg_monster332.gif" std 1 6)))
(imf "ang-bug-beetle-brown" ((32 32) (file "dg_monster332.gif" std 1 7)))
(imf "ang-bug-fly" ((32 32) (file "dg_monster332.gif" std 1 8)))
(imf "ang-spider-black" ((32 32) (file "dg_monster332.gif" std 1 9)))
(imf "ang-marmot-red" ((32 32) (file "dg_monster332.gif" std 1 10)))
(imf "ang-horsehead-scimitar" ((32 32) (file "dg_monster332.gif" std 1 11)))
(imf "ang-dog-orange" ((32 32) (file "dg_monster332.gif" std 1 12)))

(imf "ang-ooze-black" ((32 32) (file "dg_monster332.gif" std 2 0)))
(imf "ang-jellyfish-brown" ((32 32) (file "dg_monster332.gif" std 2 1)))
(imf "ang-jellyfish-umber" ((32 32) (file "dg_monster332.gif" std 2 2)))
(imf "ang-jellyfish-gray" ((32 32) (file "dg_monster332.gif" std 2 3)))
(imf "ang-ent-blue" ((32 32) (file "dg_monster332.gif" std 2 4)))
(imf "ang-ent-red" ((32 32) (file "dg_monster332.gif" std 2 5)))
(imf "ang-ent-white" ((32 32) (file "dg_monster332.gif" std 2 6)))
(imf "ang-bug-beetle-blue" ((32 32) (file "dg_monster332.gif" std 2 7)))
(imf "ang-spider-purple" ((32 32) (file "dg_monster332.gif" std 2 8)))
(imf "ang-spider-green" ((32 32) (file "dg_monster332.gif" std 2 9)))
(imf "ang-marmot-purple-blue" ((32 32) (file "dg_monster332.gif" std 2 10)))
(imf "ang-horsehead-glowhand2" ((32 32) (file "dg_monster332.gif" std 2 11)))
(imf "ang-dog-gray" ((32 32) (file "dg_monster332.gif" std 2 12)))

(imf "ang-ooze-purple" ((32 32) (file "dg_monster332.gif" std 3 0)))
(imf "ang-jellyfish-brown2" ((32 32) (file "dg_monster332.gif" std 3 1)))
(imf "ang-jellyfish-beige" ((32 32) (file "dg_monster332.gif" std 3 2)))
(imf "ang-planer-fog" ((32 32) (file "dg_monster332.gif" std 3 3)))
(imf "ang-ent-flame" ((32 32) (file "dg_monster332.gif" std 3 4)))
(imf "ang-ent-orange" ((32 32) (file "dg_monster332.gif" std 3 5)))
(imf "ang-ent-gray" ((32 32) (file "dg_monster332.gif" std 3 6)))
(imf "ang-bug-beetle-gray" ((32 32) (file "dg_monster332.gif" std 3 7)))
(imf "ang-spider-red" ((32 32) (file "dg_monster332.gif" std 3 8)))
(imf "ang-marmot-vanilla" ((32 32) (file "dg_monster332.gif" std 3 9)))
(imf "ang-birdhead-glowhand2" ((32 32) (file "dg_monster332.gif" std 3 10)))
(imf "ang-horsehead-glowhand-staff" ((32 32) (file "dg_monster332.gif" std 3 11)))
(imf "ang-hard-worker" ((32 32) (file "dg_monster332.gif" std 3 12)))

(imf "ang-ooze-brown" ((32 32) (file "dg_monster332.gif" std 4 0)))
(imf "ang-jellyfish-beige2" ((32 32) (file "dg_monster332.gif" std 4 1)))
(imf "ang-jellyfish-turq" ((32 32) (file "dg_monster332.gif" std 4 2)))
(imf "ang-planer-water" ((32 32) (file "dg_monster332.gif" std 4 3)))
(imf "ang-ent-brown" ((32 32) (file "dg_monster332.gif" std 4 4)))
(imf "ang-ent-beige2" ((32 32) (file "dg_monster332.gif" std 4 5)))
(imf "ang-ent-black" ((32 32) (file "dg_monster332.gif" std 4 6)))
(imf "ang-bug-beetle-green" ((32 32) (file "dg_monster332.gif" std 4 7)))
(imf "ang-spider-blue" ((32 32) (file "dg_monster332.gif" std 4 8)))
(imf "ang-marmot-blue" ((32 32) (file "dg_monster332.gif" std 4 9)))
(imf "ang-froghead-orange" ((32 32) (file "dg_monster332.gif" std 4 10)))
(imf "ang-horsehead-ankh" ((32 32) (file "dg_monster332.gif" std 4 11)))
(imf "ang-dog-gray2" ((32 32) (file "dg_monster332.gif" std 4 12)))

(imf "ang-cube-gelatinous" ((32 32) (file "dg_monster332.gif" std 5 0)))
(imf "ang-jellyfish-purple" ((32 32) (file "dg_monster332.gif" std 5 1)))
(imf "ang-jellyfish-green" ((32 32) (file "dg_monster332.gif" std 5 2)))
(imf "ang-planer-earth" ((32 32) (file "dg_monster332.gif" std 5 3)))
(imf "ang-ent-brown2" ((32 32) (file "dg_monster332.gif" std 5 4)))
(imf "ang-ent-turq" ((32 32) (file "dg_monster332.gif" std 5 5)))
(imf "ang-bug-purple" ((32 32) (file "dg_monster332.gif" std 5 6)))
(imf "ang-bug-beetle-red" ((32 32) (file "dg_monster332.gif" std 5 7)))
(imf "ang-spider-brown" ((32 32) (file "dg_monster332.gif" std 5 8)))
(imf "ang-marmot-black" ((32 32) (file "dg_monster332.gif" std 5 9)))
(imf "ang-froghead-gold" ((32 32) (file "dg_monster332.gif" std 5 10)))
(imf "ang-demon-rainbow" ((32 32) (file "dg_monster332.gif" std 5 11)))
(imf "ang-dog-beige" ((32 32) (file "dg_monster332.gif" std 5 12)))

(imf "ang-chest-small-wood" ((32 32) (file "dg_misc32.gif" std 0 0)))
(imf "ang-coins-small-orange" ((32 32) (file "dg_misc32.gif" std 0 1)))
(imf "ang-gems-crystal-blue" ((32 32) (file "dg_misc32.gif" std 0 2)))
(imf "ang-scroll-pent-dark" ((32 32) (file "dg_misc32.gif" std 0 3)))
(imf "ang-tool-pick-dark" ((32 32) (file "dg_misc32.gif" std 0 4)))
(imf "ang-blob-oil-black" ((32 32) (file "dg_misc32.gif" std 0 5)))
(imf "ang-tool-broken-staff" ((32 32) (file "dg_misc32.gif" std 0 6)))
(imf "ang-bones-animal-full" ((32 32) (file "dg_misc32.gif" std 0 7)))
(imf "ang-hole-small-green" ((32 32) (file "dg_misc32.gif" std 0 8)))
(imf "ang-hole-spike-red" ((32 32) (file "dg_misc32.gif" std 0 9)))
(imf "ang-tool-pick-gold" ((32 32) (file "dg_misc32.gif" std 0 10)))
(imf "ang-scroll-eye-blue" ((32 32) (file "dg_misc32.gif" std 0 11)))
(imf "ang-scroll-demon-horn" ((32 32) (file "dg_misc32.gif" std 0 12)))
(imf "ang-scroll-crown-gem" ((32 32) (file "dg_misc32.gif" std 0 13)))
(imf "ang-scroll-eye-hole" ((32 32) (file "dg_misc32.gif" std 0 14)))
(imf "ang-scroll-gray-pent" ((32 32) (file "dg_misc32.gif" std 0 15)))
(imf "ang-scroll-ankh-gold" ((32 32) (file "dg_misc32.gif" std 0 16)))

(imf "ang-chest-large-wood" ((32 32) (file "dg_misc32.gif" std 1 0)))
(imf "ang-coins-small-silver" ((32 32) (file "dg_misc32.gif" std 1 1)))
(imf "ang-gems-diamond-red" ((32 32) (file "dg_misc32.gif" std 1 2)))
(imf "ang-potion-flask-dark" ((32 32) (file "dg_misc32.gif" std 1 3)))
(imf "ang-tool-pick-blue" ((32 32) (file "dg_misc32.gif" std 1 4)))
(imf "ang-hole-large-black" ((32 32) (file "dg_misc32.gif" std 1 5)))
(imf "ang-bones-skull" ((32 32) (file "dg_misc32.gif" std 1 6)))
(imf "ang-bones-animal-small" ((32 32) (file "dg_misc32.gif" std 1 7)))
(imf "ang-hole-small-red" ((32 32) (file "dg_misc32.gif" std 1 8)))
(imf "ang-hole-spike-purple" ((32 32) (file "dg_misc32.gif" std 1 9)))
(imf "ang-tool-shovel-gold" ((32 32) (file "dg_misc32.gif" std 1 10)))
(imf "ang-scroll-eye-red" ((32 32) (file "dg_misc32.gif" std 1 11)))
(imf "ang-scroll-door-green" ((32 32) (file "dg_misc32.gif" std 1 12)))
(imf "ang-scroll-cross-lips" ((32 32) (file "dg_misc32.gif" std 1 13)))
(imf "ang-scroll-gray-pent2" ((32 32) (file "dg_misc32.gif" std 1 14)))
(imf "ang-scroll-sword-red" ((32 32) (file "dg_misc32.gif" std 1 15)))
(imf "ang-scroll-ankh-red" ((32 32) (file "dg_misc32.gif" std 1 16)))

(imf "ang-chest-small-wood2" ((32 32) (file "dg_misc32.gif" std 2 0)))
(imf "ang-coins-small-gold" ((32 32) (file "dg_misc32.gif" std 2 1)))
(imf "ang-gems-diamond-blue" ((32 32) (file "dg_misc32.gif" std 2 2)))
(imf "ang-potion-flask-brown" ((32 32) (file "dg_misc32.gif" std 2 3)))
(imf "ang-tool-shovel-gray" ((32 32) (file "dg_misc32.gif" std 2 4)))
(imf "ang-hole-spike-yellow" ((32 32) (file "dg_misc32.gif" std 2 5)))
(imf "ang-bones-femur-broke" ((32 32) (file "dg_misc32.gif" std 2 6)))
(imf "ang-cup-chalice-green" ((32 32) (file "dg_misc32.gif" std 2 7)))
(imf "ang-hole-small-blue" ((32 32) (file "dg_misc32.gif" std 2 8)))
(imf "ang-hole-spike-lblue" ((32 32) (file "dg_misc32.gif" std 2 9)))
(imf "ang-tool-lamp-gold" ((32 32) (file "dg_misc32.gif" std 2 10)))
(imf "ang-scroll-sword-hit" ((32 32) (file "dg_misc32.gif" std 2 11)))
(imf "ang-scroll-clone-t" ((32 32) (file "dg_misc32.gif" std 2 12)))
(imf "ang-scroll-cross-hand" ((32 32) (file "dg_misc32.gif" std 2 13)))
(imf "ang-scroll-demon-angry" ((32 32) (file "dg_misc32.gif" std 2 14)))
(imf "ang-scroll-sword-green" ((32 32) (file "dg_misc32.gif" std 2 15)))
(imf "ang-scroll-door-broke" ((32 32) (file "dg_misc32.gif" std 2 16)))

(imf "ang-chest-large-wood2" ((32 32) (file "dg_misc32.gif" std 3 0)))
(imf "ang-coins-small-blue" ((32 32) (file "dg_misc32.gif" std 3 1)))
(imf "ang-gems-emerald-green" ((32 32) (file "dg_misc32.gif" std 3 2)))
(imf "ang-tool-spike" ((32 32) (file "dg_misc32.gif" std 3 3)))
(imf "ang-tool-shovel-gray2" ((32 32) (file "dg_misc32.gif" std 3 4)))
(imf "ang-door-trap-open" ((32 32) (file "dg_misc32.gif" std 3 5)))
(imf "ang-bones-ogre-full" ((32 32) (file "dg_misc32.gif" std 3 6)))
(imf "ang-star-gold-blue" ((32 32) (file "dg_misc32.gif" std 3 7)))
(imf "ang-hole-small-purple" ((32 32) (file "dg_misc32.gif" std 3 8)))
(imf "ang-hole-water-blue" ((32 32) (file "dg_misc32.gif" std 3 9)))
(imf "ang-scroll-pent-dark2" ((32 32) (file "dg_misc32.gif" std 3 10)))
(imf "ang-scroll-sword-dam" ((32 32) (file "dg_misc32.gif" std 3 11)))
(imf "ang-scroll-up-down" ((32 32) (file "dg_misc32.gif" std 3 12)))
(imf "ang-scroll-skull" ((32 32) (file "dg_misc32.gif" std 3 13)))
(imf "ang-scroll-armor-red" ((32 32) (file "dg_misc32.gif" std 3 14)))
(imf "ang-scroll-cross-skull" ((32 32) (file "dg_misc32.gif" std 3 15)))
(imf "ang-scroll-eye-potion" ((32 32) (file "dg_misc32.gif" std 3 16)))

(imf "ang-chest-small-metal" ((32 32) (file "dg_misc32.gif" std 4 0)))
(imf "ang-coins-small-green" ((32 32) (file "dg_misc32.gif" std 4 1)))
(imf "ang-scroll-pent-light" ((32 32) (file "dg_misc32.gif" std 4 2)))
(imf "ang-tool-lamp" ((32 32) (file "dg_misc32.gif" std 4 3)))
(imf "ang-tool-shovel-blue" ((32 32) (file "dg_misc32.gif" std 4 4)))
(imf "ang-symbol-red" ((32 32) (file "dg_misc32.gif" std 4 5)))
(imf "ang-bones-man-full" ((32 32) (file "dg_misc32.gif" std 4 6)))
(imf "ang-ball-crystal-bronze" ((32 32) (file "dg_misc32.gif" std 4 7)))
(imf "ang-hole-small-orange" ((32 32) (file "dg_misc32.gif" std 4 8)))
(imf "ang-hole-water-green" ((32 32) (file "dg_misc32.gif" std 4 9)))
(imf "ang-scroll-pent-dark3" ((32 32) (file "dg_misc32.gif" std 4 10)))
(imf "ang-scroll-armor-green" ((32 32) (file "dg_misc32.gif" std 4 11)))
(imf "ang-scroll-demon-confuse" ((32 32) (file "dg_misc32.gif" std 4 12)))
(imf "ang-scroll-castle-man" ((32 32) (file "dg_misc32.gif" std 4 13)))
(imf "ang-scroll-bread" ((32 32) (file "dg_misc32.gif" std 4 14)))
(imf "ang-scroll-cross-pent" ((32 32) (file "dg_misc32.gif" std 4 15)))
(imf "ang-scroll-fist-armor" ((32 32) (file "dg_misc32.gif" std 4 16)))

(imf "ang-chest-large-metal" ((32 32) (file "dg_misc32.gif" std 5 0)))
(imf "ang-gems-smooth-multi" ((32 32) (file "dg_misc32.gif" std 5 1)))
(imf "ang-scroll-pent-light2" ((32 32) (file "dg_misc32.gif" std 5 2)))
(imf "ang-tool-torch" ((32 32) (file "dg_misc32.gif" std 5 3)))
(imf "ang-hole-small-brown" ((32 32) (file "dg_misc32.gif" std 5 4)))
(imf "ang-symbol-yellow" ((32 32) (file "dg_misc32.gif" std 5 5)))
(imf "ang-bones-man-full2" ((32 32) (file "dg_misc32.gif" std 5 6)))
(imf "ang-treasure-varied" ((32 32) (file "dg_misc32.gif" std 5 7)))
(imf "ang-hole-small-gray" ((32 32) (file "dg_misc32.gif" std 5 8)))
(imf "ang-hole-water-red" ((32 32) (file "dg_misc32.gif" std 5 9)))
(imf "ang-scroll-pent-light3" ((32 32) (file "dg_misc32.gif" std 5 10)))
(imf "ang-scroll-armor-blue" ((32 32) (file "dg_misc32.gif" std 5 11)))
(imf "ang-scroll-map" ((32 32) (file "dg_misc32.gif" std 5 12)))
(imf "ang-scroll-cross-lips2" ((32 32) (file "dg_misc32.gif" std 5 13)))
(imf "ang-scroll-star" ((32 32) (file "dg_misc32.gif" std 5 14)))
(imf "ang-scroll-eye-invisible" ((32 32) (file "dg_misc32.gif" std 5 15)))
(imf "ang-scroll-fist-armor2" ((32 32) (file "dg_misc32.gif" std 5 16)))

(imf "ang-chest-broken-wood" ((32 32) (file "dg_misc32.gif" std 6 0)))
(imf "ang-gems-smooth-purple" ((32 32) (file "dg_misc32.gif" std 6 1)))
(imf "ang-scroll-pent-dark4" ((32 32) (file "dg_misc32.gif" std 6 2)))
(imf "ang-tool-pick" ((32 32) (file "dg_misc32.gif" std 6 3)))
(imf "ang-egg-easter" ((32 32) (file "dg_misc32.gif" std 6 4)))
(imf "ang-symbol-green" ((32 32) (file "dg_misc32.gif" std 6 5)))
(imf "ang-bones-hobbit-full" ((32 32) (file "dg_misc32.gif" std 6 6)))
(imf "ang-rubble-stone" ((32 32) (file "dg_misc32.gif" std 6 7)))
(imf "ang-hole-spike-green" ((32 32) (file "dg_misc32.gif" std 6 8)))
(imf "ang-hole-fire" ((32 32) (file "dg_misc32.gif" std 6 9)))
(imf "ang-scroll-pent-light4" ((32 32) (file "dg_misc32.gif" std 6 10)))
(imf "ang-scroll-star-yellow" ((32 32) (file "dg_misc32.gif" std 6 11)))
(imf "ang-scroll-boomerang" ((32 32) (file "dg_misc32.gif" std 6 12)))
(imf "ang-scroll-trap-hole" ((32 32) (file "dg_misc32.gif" std 6 13)))
(imf "ang-scroll-eye-door" ((32 32) (file "dg_misc32.gif" std 6 14)))
(imf "ang-scroll-eye-gold" ((32 32) (file "dg_misc32.gif" std 6 15)))
(imf "ang-scroll-crown-fire" ((32 32) (file "dg_misc32.gif" std 6 16)))

(imf "ang-druid-gray-staff" ((32 32) (file "dg_classm32.gif" std 0 0)))
(imf "ang-druid-brown-staff" ((32 32) (file "dg_classm32.gif" std 0 1)))
(imf "ang-druid-no-staff" ((32 32) (file "dg_classm32.gif" std 0 2)))
(imf "ang-druid-halfling-staff" ((32 32) (file "dg_classm32.gif" std 0 3)))
(imf "ang-druid-yellow-staff" ((32 32) (file "dg_classm32.gif" std 0 4)))
(imf "ang-druid-dwarf-staff" ((32 32) (file "dg_classm32.gif" std 0 5)))
(imf "ang-druid-oni-staff" ((32 32) (file "dg_classm32.gif" std 0 6)))
(imf "ang-druid-ogre-staff" ((32 32) (file "dg_classm32.gif" std 0 7)))
(imf "ang-druid-gnome-staff" ((32 32) (file "dg_classm32.gif" std 0 8)))
(imf "ang-druid-drow-staff" ((32 32) (file "dg_classm32.gif" std 0 9)))
(imf "ang-druid-troll-staff" ((32 32) (file "dg_classm32.gif" std 0 10)))

(imf "ang-enchanter-brown-staff" ((32 32) (file "dg_classm32.gif" std 1 0)))
(imf "ang-enchanter-orange-staff" ((32 32) (file "dg_classm32.gif" std 1 1)))
(imf "ang-enchanter-purple-staff" ((32 32) (file "dg_classm32.gif" std 1 2)))
(imf "ang-enchanter-halfling-staff" ((32 32) (file "dg_classm32.gif" std 1 3)))
(imf "ang-enchanter-yellow-staff" ((32 32) (file "dg_classm32.gif" std 1 4)))
(imf "ang-enchanter-dwarf-staff" ((32 32) (file "dg_classm32.gif" std 1 5)))
(imf "ang-enchanter-oni-staff" ((32 32) (file "dg_classm32.gif" std 1 6)))
(imf "ang-enchanter-ogre-staff" ((32 32) (file "dg_classm32.gif" std 1 7)))
(imf "ang-enchanter-gnome-staff" ((32 32) (file "dg_classm32.gif" std 1 8)))
(imf "ang-enchanter-drow-staff" ((32 32) (file "dg_classm32.gif" std 1 9)))
(imf "ang-enchanter-troll-staff" ((32 32) (file "dg_classm32.gif" std 1 10)))

(imf "ang-mage-blue-glowhand2" ((32 32) (file "dg_classm32.gif" std 2 0)))
(imf "ang-mage-white-glowhand2" ((32 32) (file "dg_classm32.gif" std 2 1)))
(imf "ang-mage-red-glowhand" ((32 32) (file "dg_classm32.gif" std 2 2)))
(imf "ang-mage-halfling-glowhand" ((32 32) (file "dg_classm32.gif" std 2 3)))
(imf "ang-mage--black-glowhand" ((32 32) (file "dg_classm32.gif" std 2 4)))
(imf "ang-mage-dwarf-glowhand" ((32 32) (file "dg_classm32.gif" std 2 5)))
(imf "ang-mage-oni-glowhand" ((32 32) (file "dg_classm32.gif" std 2 6)))
(imf "ang-mage-ogre-glowhand" ((32 32) (file "dg_classm32.gif" std 2 7)))
(imf "ang-mage-gnome-glowhand" ((32 32) (file "dg_classm32.gif" std 2 8)))
(imf "ang-mage-drow-glowhand" ((32 32) (file "dg_classm32.gif" std 2 9)))
(imf "ang-mage-troll-glowhand" ((32 32) (file "dg_classm32.gif" std 2 10)))

(imf "ang-paladin-right-sword" ((32 32) (file "dg_classm32.gif" std 3 0)))
(imf "ang-paladin-left-sword" ((32 32) (file "dg_classm32.gif" std 3 1)))
(imf "ang-paladin-left-scimitar" ((32 32) (file "dg_classm32.gif" std 3 2)))
(imf "ang-paladin-halfling-sword" ((32 32) (file "dg_classm32.gif" std 3 3)))
(imf "ang-paladin-cross-shield" ((32 32) (file "dg_classm32.gif" std 3 4)))
(imf "ang-paladin-dwarf-scimitar" ((32 32) (file "dg_classm32.gif" std 3 5)))
(imf "ang-paladin-oni-sword" ((32 32) (file "dg_classm32.gif" std 3 6)))
(imf "ang-paladin-ogre-sword" ((32 32) (file "dg_classm32.gif" std 3 7)))
(imf "ang-paladin-gnome-sword" ((32 32) (file "dg_classm32.gif" std 3 8)))
(imf "ang-paladin-drow-sword" ((32 32) (file "dg_classm32.gif" std 3 9)))
(imf "ang-paladin-troll-sword" ((32 32) (file "dg_classm32.gif" std 3 10)))

(imf "ang-cleric-ankh-green" ((32 32) (file "dg_classm32.gif" std 4 0)))
(imf "ang-cleric-ankh-lblue" ((32 32) (file "dg_classm32.gif" std 4 1)))
(imf "ang-cleric-ankh-gray" ((32 32) (file "dg_classm32.gif" std 4 2)))
(imf "ang-cleric-halfling-ankh" ((32 32) (file "dg_classm32.gif" std 4 3)))
(imf "ang-cleric-ankh-pale" ((32 32) (file "dg_classm32.gif" std 4 4)))
(imf "ang-cleric-dwarf-ankh" ((32 32) (file "dg_classm32.gif" std 4 5)))
(imf "ang-cleric-oni-ankh" ((32 32) (file "dg_classm32.gif" std 4 6)))
(imf "ang-cleric-ogre-ankh" ((32 32) (file "dg_classm32.gif" std 4 7)))
(imf "ang-cleric-gnome-ankh" ((32 32) (file "dg_classm32.gif" std 4 8)))
(imf "ang-cleric-drow-ankh" ((32 32) (file "dg_classm32.gif" std 4 9)))
(imf "ang-cleric-troll-ankh" ((32 32) (file "dg_classm32.gif" std 4 10)))

(imf "ang-ranger-no-quiver" ((32 32) (file "dg_classm32.gif" std 5 0)))
(imf "ang-ranger-quiver-sword" ((32 32) (file "dg_classm32.gif" std 5 1)))
(imf "ang-ranger-quiver-blonde" ((32 32) (file "dg_classm32.gif" std 5 2)))
(imf "ang-ranger-halfling-sword" ((32 32) (file "dg_classm32.gif" std 5 3)))
(imf "ang-ranger-quiver-sword2" ((32 32) (file "dg_classm32.gif" std 5 4)))
(imf "ang-ranger-dwarf-sword" ((32 32) (file "dg_classm32.gif" std 5 5)))
(imf "ang-ranger-oni-staff" ((32 32) (file "dg_classm32.gif" std 5 6)))
(imf "ang-ranger-ogre-staff" ((32 32) (file "dg_classm32.gif" std 5 7)))
(imf "ang-ranger-gnome-dagger" ((32 32) (file "dg_classm32.gif" std 5 8)))
(imf "ang-ranger-drow-sword" ((32 32) (file "dg_classm32.gif" std 5 9)))
(imf "ang-ranger-troll-sword" ((32 32) (file "dg_classm32.gif" std 5 10)))

(imf "ang-thief-blue-dagger" ((32 32) (file "dg_classm32.gif" std 6 0)))
(imf "ang-thief-red-dagger" ((32 32) (file "dg_classm32.gif" std 6 1)))
(imf "ang-thief-black-dagger" ((32 32) (file "dg_classm32.gif" std 6 2)))
(imf "ang-thief-halfling-dagger" ((32 32) (file "dg_classm32.gif" std 6 3)))
(imf "ang-thief-blue-hood" ((32 32) (file "dg_classm32.gif" std 6 4)))
(imf "ang-thief-dwarf-dagger" ((32 32) (file "dg_classm32.gif" std 6 5)))
(imf "ang-thief-oni-dagger" ((32 32) (file "dg_classm32.gif" std 6 6)))
(imf "ang-thief-ogre-dagger" ((32 32) (file "dg_classm32.gif" std 6 7)))
(imf "ang-thief-gnome-sword" ((32 32) (file "dg_classm32.gif" std 6 8)))
(imf "ang-thief-drow-dagger" ((32 32) (file "dg_classm32.gif" std 6 9)))
(imf "ang-thief-troll-dagger" ((32 32) (file "dg_classm32.gif" std 6 10)))

(imf "ang-fighter-quiver-sword" ((32 32) (file "dg_classm32.gif" std 7 0)))
(imf "ang-fighter-quiver-staff" ((32 32) (file "dg_classm32.gif" std 7 1)))
(imf "ang-fighter-quiver-sword2" ((32 32) (file "dg_classm32.gif" std 7 2)))
(imf "ang-fighter-halfling-axe" ((32 32) (file "dg_classm32.gif" std 7 3)))
(imf "ang-fighter-quivere-sword3" ((32 32) (file "dg_classm32.gif" std 7 4)))
(imf "ang-fighter-dwarf-dagger" ((32 32) (file "dg_classm32.gif" std 7 5)))
(imf "ang-fighter-oni-sword" ((32 32) (file "dg_classm32.gif" std 7 6)))
(imf "ang-fighter-ogre-sword" ((32 32) (file "dg_classm32.gif" std 7 7)))
(imf "ang-fighter-gnome-sword" ((32 32) (file "dg_classm32.gif" std 7 8)))
(imf "ang-fighter-drow-sword" ((32 32) (file "dg_classm32.gif" std 7 9)))
(imf "ang-fighter-troll-axe" ((32 32) (file "dg_classm32.gif" std 7 10)))

(imf "ang-lightning-silver-up" ((32 32) (file "dg_effects32.gif" std 0 0)))
(imf "ang-lightning-red-up" ((32 32) (file "dg_effects32.gif" std 0 1)))
(imf "ang-lightning-turq-up" ((32 32) (file "dg_effects32.gif" std 0 2)))
(imf "ang-lightning-gold-up" ((32 32) (file "dg_effects32.gif" std 0 3)))
(imf "ang-fireball-single-red" ((32 32) (file "dg_effects32.gif" std 0 4)))
(imf "ang-fireball-multi-red" ((32 32) (file "dg_effects32.gif" std 0 5)))
(imf "ang-fireball-gray-red" ((32 32) (file "dg_effects32.gif" std 0 6)))
(imf "ang-fireball-gray-silver" ((32 32) (file "dg_effects32.gif" std 0 7)))
(imf "ang-powerball-big-red" ((32 32) (file "dg_effects32.gif" std 0 8)))
(imf "ang-arrow-red-up" ((32 32) (file "dg_effects32.gif" std 0 9)))
(imf "ang-arrow-green-dur" ((32 32) (file "dg_effects32.gif" std 0 10)))

(imf "ang-lightning-silver-side" ((32 32) (file "dg_effects32.gif" std 1 0)))
(imf "ang-lightning-red-side" ((32 32) (file "dg_effects32.gif" std 1 1)))
(imf "ang-lightning-turq-side" ((32 32) (file "dg_effects32.gif" std 1 2)))
(imf "ang-lightning-gold-side" ((32 32) (file "dg_effects32.gif" std 1 3)))
(imf "ang-fireball-single-green" ((32 32) (file "dg_effects32.gif" std 1 4)))
(imf "ang-fireball-multi-green" ((32 32) (file "dg_effects32.gif" std 1 5)))
(imf "ang-fireball-gray-green" ((32 32) (file "dg_effects32.gif" std 1 6)))
(imf "ang-fireball-silver-gray" ((32 32) (file "dg_effects32.gif" std 1 7)))
(imf "ang-powerball-big-green" ((32 32) (file "dg_effects32.gif" std 1 8)))
(imf "ang-arrow-red-right" ((32 32) (file "dg_effects32.gif" std 1 9)))
(imf "ang-arrow-green-ddr" ((32 32) (file "dg_effects32.gif" std 1 10)))

(imf "ang-lightning-silver-ddl" ((32 32) (file "dg_effects32.gif" std 2 0)))
(imf "ang-lightning-red-ddl" ((32 32) (file "dg_effects32.gif" std 2 1)))
(imf "ang-lightning-turq-ddl" ((32 32) (file "dg_effects32.gif" std 2 2)))
(imf "ang-lightning-gold-ddl" ((32 32) (file "dg_effects32.gif" std 2 3)))
(imf "ang-fireball-single-blue" ((32 32) (file "dg_effects32.gif" std 2 4)))
(imf "ang-fireball-multi-blue" ((32 32) (file "dg_effects32.gif" std 2 5)))
(imf "ang-fireball-gray-blue" ((32 32) (file "dg_effects32.gif" std 2 6)))
(imf "ang-fireball-all-color" ((32 32) (file "dg_effects32.gif" std 2 7)))
(imf "ang-powerball-big-blue" ((32 32) (file "dg_effects32.gif" std 2 8)))
(imf "ang-arrow-red-down" ((32 32) (file "dg_effects32.gif" std 2 9)))
(imf "ang-arrow-green-ddl" ((32 32) (file "dg_effects32.gif" std 2 10)))

(imf "ang-lightning-silver-ddr" ((32 32) (file "dg_effects32.gif" std 3 0)))
(imf "ang-lightning-red-ddr" ((32 32) (file "dg_effects32.gif" std 3 1)))
(imf "ang-lightning-turq-ddr" ((32 32) (file "dg_effects32.gif" std 3 2)))
(imf "ang-lightning-gold-ddr" ((32 32) (file "dg_effects32.gif" std 3 3)))
(imf "ang-fireball-single-turq" ((32 32) (file "dg_effects32.gif" std 3 4)))
(imf "ang-fireball-multi-turq" ((32 32) (file "dg_effects32.gif" std 3 5)))
(imf "ang-fireball-gray-purple" ((32 32) (file "dg_effects32.gif" std 3 6)))
(imf "ang-thunderball-blue-orange" ((32 32) (file "dg_effects32.gif" std 3 7)))
(imf "ang-powerball-big-yellow" ((32 32) (file "dg_effects32.gif" std 3 8)))
(imf "ang-arrow-red-left" ((32 32) (file "dg_effects32.gif" std 3 9)))
(imf "ang-arrow-green-dul" ((32 32) (file "dg_effects32.gif" std 3 10)))

(imf "ang-lightning-gray-up" ((32 32) (file "dg_effects32.gif" std 4 0)))
(imf "ang-lightning-green-up" ((32 32) (file "dg_effects32.gif" std 4 1)))
(imf "ang-lightning-bronze-up" ((32 32) (file "dg_effects32.gif" std 4 2)))
(imf "ang-lightning-orange-up" ((32 32) (file "dg_effects32.gif" std 4 3)))
(imf "ang-fireball-single-bronze" ((32 32) (file "dg_effects32.gif" std 4 4)))
(imf "ang-fireball-multi-bronze" ((32 32) (file "dg_effects32.gif" std 4 5)))
(imf "ang-fireball-orange-red" ((32 32) (file "dg_effects32.gif" std 4 6)))
(imf "ang-thunderball-blue-purple" ((32 32) (file "dg_effects32.gif" std 4 7)))
(imf "ang-powerball-big-purple" ((32 32) (file "dg_effects32.gif" std 4 8)))
(imf "ang-arrow-red-dur" ((32 32) (file "dg_effects32.gif" std 4 9)))
(imf "ang-arrow-gray-up" ((32 32) (file "dg_effects32.gif" std 4 10)))

(imf "ang-lightning-gray-side" ((32 32) (file "dg_effects32.gif" std 5 0)))
(imf "ang-lightning-green-side" ((32 32) (file "dg_effects32.gif" std 5 1)))
(imf "ang-lightning-bronze-side" ((32 32) (file "dg_effects32.gif" std 5 2)))
(imf "ang-lightning-orange-side" ((32 32) (file "dg_effects32.gif" std 5 3)))
(imf "ang-fireball-single-purple" ((32 32) (file "dg_effects32.gif" std 5 4)))
(imf "ang-fireball-multi-gray" ((32 32) (file "dg_effects32.gif" std 5 5)))
(imf "ang-fireball-yellow-green" ((32 32) (file "dg_effects32.gif" std 5 6)))
(imf "ang-thunderball-blue-yellow" ((32 32) (file "dg_effects32.gif" std 5 7)))
(imf "ang-powerball-big-turq" ((32 32) (file "dg_effects32.gif" std 5 8)))
(imf "ang-arrow-red-ddr" ((32 32) (file "dg_effects32.gif" std 5 9)))
(imf "ang-arrow-gray-right" ((32 32) (file "dg_effects32.gif" std 5 10)))

(imf "ang-lightning-gray-ddl" ((32 32) (file "dg_effects32.gif" std 6 0)))
(imf "ang-lightning-green-ddl" ((32 32) (file "dg_effects32.gif" std 6 1)))
(imf "ang-lightning-bronze-ddl" ((32 32) (file "dg_effects32.gif" std 6 2)))
(imf "ang-lightning-orange-ddl" ((32 32) (file "dg_effects32.gif" std 6 3)))
(imf "ang-fireball-single-yellow" ((32 32) (file "dg_effects32.gif" std 6 4)))
(imf "ang-fireball-multi-yellow" ((32 32) (file "dg_effects32.gif" std 6 5)))
(imf "ang-fireball-turq-blue" ((32 32) (file "dg_effects32.gif" std 6 6)))
(imf "ang-thunderball-blue-green" ((32 32) (file "dg_effects32.gif" std 6 7)))
(imf "ang-powerball-burst-red" ((32 32) (file "dg_effects32.gif" std 6 8)))
(imf "ang-arrow-red-ddl" ((32 32) (file "dg_effects32.gif" std 6 9)))
(imf "ang-arrow-gray-down" ((32 32) (file "dg_effects32.gif" std 6 10)))

(imf "ang-lightning-gray-ddr" ((32 32) (file "dg_effects32.gif" std 7 0)))
(imf "ang-lightning-green-ddr" ((32 32) (file "dg_effects32.gif" std 7 1)))
(imf "ang-lightning-bronze-ddr" ((32 32) (file "dg_effects32.gif" std 7 2)))
(imf "ang-lightning-orange-ddr" ((32 32) (file "dg_effects32.gif" std 7 3)))
(imf "ang-fireball-single-orange" ((32 32) (file "dg_effects32.gif" std 7 4)))
(imf "ang-fireball-multi-purple" ((32 32) (file "dg_effects32.gif" std 7 5)))
(imf "ang-fireball-lblue-purple" ((32 32) (file "dg_effects32.gif" std 7 6)))
(imf "ang-thunderball-blue-blue" ((32 32) (file "dg_effects32.gif" std 7 7)))
(imf "ang-powerball-burst-green" ((32 32) (file "dg_effects32.gif" std 7 8)))
(imf "ang-arrow-red-dul" ((32 32) (file "dg_effects32.gif" std 7 9)))
(imf "ang-arrow-gray-left" ((32 32) (file "dg_effects32.gif" std 7 10)))

(imf "ang-lightning-black-up" ((32 32) (file "dg_effects32.gif" std 8 0)))
(imf "ang-lightning-blue-up" ((32 32) (file "dg_effects32.gif" std 8 1)))
(imf "ang-lightning-purple-up" ((32 32) (file "dg_effects32.gif" std 8 2)))
(imf "ang-lightning-rainbow-up" ((32 32) (file "dg_effects32.gif" std 8 3)))
(imf "ang-fireball-single-silver" ((32 32) (file "dg_effects32.gif" std 8 4)))
(imf "ang-fireball-multi-orange" ((32 32) (file "dg_effects32.gif" std 8 5)))
(imf "ang-fireball-gray-lblue" ((32 32) (file "dg_effects32.gif" std 8 6)))
(imf "ang-thunderball-blue-red" ((32 32) (file "dg_effects32.gif" std 8 7)))
(imf "ang-powerball-burst-blue" ((32 32) (file "dg_effects32.gif" std 8 8)))
(imf "ang-arrow-green-up" ((32 32) (file "dg_effects32.gif" std 8 9)))
(imf "ang-arrow-gray-dur" ((32 32) (file "dg_effects32.gif" std 8 10)))

(imf "ang-lightning-black-side" ((32 32) (file "dg_effects32.gif" std 9 0)))
(imf "ang-lightning-blue-side" ((32 32) (file "dg_effects32.gif" std 9 1)))
(imf "ang-lightning-purple-side" ((32 32) (file "dg_effects32.gif" std 9 2)))
(imf "ang-lightning-rainbow-side" ((32 32) (file "dg_effects32.gif" std 9 3)))
(imf "ang-fireball-single-gray" ((32 32) (file "dg_effects32.gif" std 9 4)))
(imf "ang-fireball-multi-lblue" ((32 32) (file "dg_effects32.gif" std 9 5)))
(imf "ang-fireball-gray-bronze" ((32 32) (file "dg_effects32.gif" std 9 6)))
(imf "ang-thunderball-blue-only" ((32 32) (file "dg_effects32.gif" std 9 7)))
(imf "ang-powerball-burst-yellow" ((32 32) (file "dg_effects32.gif" std 9 8)))
(imf "ang-arrow-green-right" ((32 32) (file "dg_effects32.gif" std 9 9)))
(imf "ang-arrow-gray-ddr" ((32 32) (file "dg_effects32.gif" std 9 10)))

(imf "ang-lightning-black-ddl" ((32 32) (file "dg_effects32.gif" std 10 0)))
(imf "ang-lightning-blue-ddl" ((32 32) (file "dg_effects32.gif" std 10 1)))
(imf "ang-lightning-purple-ddl" ((32 32) (file "dg_effects32.gif" std 10 2)))
(imf "ang-lightning-rainbow-ddl" ((32 32) (file "dg_effects32.gif" std 10 3)))
(imf "ang-fireball-single-black" ((32 32) (file "dg_effects32.gif" std 10 4)))
(imf "ang-fireball-multi-silver" ((32 32) (file "dg_effects32.gif" std 10 5)))
(imf "ang-fireball-silver-lblue" ((32 32) (file "dg_effects32.gif" std 10 6)))
(imf "ang-thunderball-green-only" ((32 32) (file "dg_effects32.gif" std 10 7)))
(imf "ang-powerball-burst-purple" ((32 32) (file "dg_effects32.gif" std 10 8)))
(imf "ang-arrow-green-down" ((32 32) (file "dg_effects32.gif" std 10 9)))
(imf "ang-arrow-gray-ddl" ((32 32) (file "dg_effects32.gif" std 10 10)))

(imf "ang-lightning-black-ddr" ((32 32) (file "dg_effects32.gif" std 11 0)))
(imf "ang-lightning-blue-ddr" ((32 32) (file "dg_effects32.gif" std 11 1)))
(imf "ang-lightning-purple-ddr" ((32 32) (file "dg_effects32.gif" std 11 2)))
(imf "ang-lightning-rainbow-ddr" ((32 32) (file "dg_effects32.gif" std 11 3)))
(imf "ang-fireball-single-rainbow" ((32 32) (file "dg_effects32.gif" std 11 4)))
(imf "ang-fireball-multi-black" ((32 32) (file "dg_effects32.gif" std 11 5)))
(imf "ang-fireball-silver-bronze" ((32 32) (file "dg_effects32.gif" std 11 6)))
(imf "ang-thunderball-red-only" ((32 32) (file "dg_effects32.gif" std 11 7)))
(imf "ang-powerball-burst-lblue" ((32 32) (file "dg_effects32.gif" std 11 8)))
(imf "ang-arrow-green-left" ((32 32) (file "dg_effects32.gif" std 11 9)))
(imf "ang-arrow-gray-dul" ((32 32) (file "dg_effects32.gif" std 11 10)))

(imf "ang-dragon-baby-blue" ((32 32) (file "dg_dragon32.gif" std 0 0)))
(imf "ang-dragon-young-blue" ((32 32) (file "dg_dragon32.gif" std 0 1)))
(imf "ang-dragon-adult-blue" ((32 32) (file "dg_dragon32.gif" std 0 2)))
(imf "ang-dragon-mature-blue" ((32 32) (file "dg_dragon32.gif" std 0 3)))
(imf "ang-dragon-wyrm-red" ((32 32) (file "dg_dragon32.gif" std 0 4)))
(imf "ang-dragon-ear-black" ((32 32) (file "dg_dragon32.gif" std 0 5)))
(imf "ang-dragon-lich-plain" ((32 32) (file "dg_dragon32.gif" std 0 6)))

(imf "ang-dragon-baby-gold" ((32 32) (file "dg_dragon32.gif" std 1 0)))
(imf "ang-dragon-young-gold" ((32 32) (file "dg_dragon32.gif" std 1 1)))
(imf "ang-dragon-adult-gold" ((32 32) (file "dg_dragon32.gif" std 1 2)))
(imf "ang-dragon-mature-gold" ((32 32) (file "dg_dragon32.gif" std 1 3)))
(imf "ang-dragon-wyrm-blue" ((32 32) (file "dg_dragon32.gif" std 1 4)))
(imf "ang-dragon-ear-gray" ((32 32) (file "dg_dragon32.gif" std 1 5)))
(imf "ang-dragon-lich-red" ((32 32) (file "dg_dragon32.gif" std 1 6)))

(imf "ang-dragon-baby-white" ((32 32) (file "dg_dragon32.gif" std 2 0)))
(imf "ang-dragon-young-white" ((32 32) (file "dg_dragon32.gif" std 2 1)))
(imf "ang-dragon-adult-white" ((32 32) (file "dg_dragon32.gif" std 2 2)))
(imf "ang-dragon-mature-white" ((32 32) (file "dg_dragon32.gif" std 2 3)))
(imf "ang-dragon-wyrm-white" ((32 32) (file "dg_dragon32.gif" std 2 4)))
(imf "ang-dragon-ear-blue" ((32 32) (file "dg_dragon32.gif" std 2 5)))
(imf "ang-dragon-lich-blue" ((32 32) (file "dg_dragon32.gif" std 2 6)))

(imf "ang-dragon-baby-green" ((32 32) (file "dg_dragon32.gif" std 3 0)))
(imf "ang-dragon-young-green" ((32 32) (file "dg_dragon32.gif" std 3 1)))
(imf "ang-dragon-adult-green" ((32 32) (file "dg_dragon32.gif" std 3 2)))
(imf "ang-dragon-mature-green" ((32 32) (file "dg_dragon32.gif" std 3 3)))
(imf "ang-dragon-wyrm-gray" ((32 32) (file "dg_dragon32.gif" std 3 4)))
(imf "ang-dragon-ear-frosted" ((32 32) (file "dg_dragon32.gif" std 3 5)))
(imf "ang-dragon-lich-green" ((32 32) (file "dg_dragon32.gif" std 3 6)))

(imf "ang-dragon-baby-gray" ((32 32) (file "dg_dragon32.gif" std 4 0)))
(imf "ang-dragon-young-gray" ((32 32) (file "dg_dragon32.gif" std 4 1)))
(imf "ang-dragon-adult-gray" ((32 32) (file "dg_dragon32.gif" std 4 2)))
(imf "ang-dragon-mature-gray" ((32 32) (file "dg_dragon32.gif" std 4 3)))
(imf "ang-dragon-wyrm-green" ((32 32) (file "dg_dragon32.gif" std 4 4)))
(imf "ang-dragon-ear-white" ((32 32) (file "dg_dragon32.gif" std 4 5)))
(imf "ang-dragon-lich-gray" ((32 32) (file "dg_dragon32.gif" std 4 6)))

(imf "ang-dragon-baby-red" ((32 32) (file "dg_dragon32.gif" std 5 0)))
(imf "ang-dragon-young-red" ((32 32) (file "dg_dragon32.gif" std 5 1)))
(imf "ang-dragon-adult-red" ((32 32) (file "dg_dragon32.gif" std 5 2)))
(imf "ang-dragon-mature-red" ((32 32) (file "dg_dragon32.gif" std 5 3)))
(imf "ang-dragon-wyrm-bronze" ((32 32) (file "dg_dragon32.gif" std 5 4)))
(imf "ang-dragon-ear-red" ((32 32) (file "dg_dragon32.gif" std 5 5)))
(imf "ang-dragon-lich-red2" ((32 32) (file "dg_dragon32.gif" std 5 6)))

(imf "ang-dragon-baby-bronze" ((32 32) (file "dg_dragon32.gif" std 6 0)))
(imf "ang-dragon-young-bronze" ((32 32) (file "dg_dragon32.gif" std 6 1)))
(imf "ang-dragon-adult-bronze" ((32 32) (file "dg_dragon32.gif" std 6 2)))
(imf "ang-dragon-mature-bronze" ((32 32) (file "dg_dragon32.gif" std 6 3)))
(imf "ang-dragon-purple-smoke" ((32 32) (file "dg_dragon32.gif" std 6 4)))
(imf "ang-dragon-dark-green" ((32 32) (file "dg_dragon32.gif" std 6 5)))
(imf "ang-dragon-lich-blue2" ((32 32) (file "dg_dragon32.gif" std 6 6)))

(imf "ang-dragon-baby-rainbow" ((32 32) (file "dg_dragon32.gif" std 7 0)))
(imf "ang-dragon-young-rainbow" ((32 32) (file "dg_dragon32.gif" std 7 1)))
(imf "ang-dragon-adult-rainbow" ((32 32) (file "dg_dragon32.gif" std 7 2)))
(imf "ang-dragon-mature-rainbow" ((32 32) (file "dg_dragon32.gif" std 7 3)))
(imf "ang-dragon-pink-smoke" ((32 32) (file "dg_dragon32.gif" std 7 4)))
(imf "ang-dragon-wyrm-black" ((32 32) (file "dg_dragon32.gif" std 7 5)))
(imf "ang-dragon-lich-green2" ((32 32) (file "dg_dragon32.gif" std 7 6)))


(imf "ang-wraith-blue-flame" ((32 32) (file "dg_undead32.gif" std 0 0)))
(imf "ang-skeleton-blue-half" ((32 32) (file "dg_undead32.gif" std 0 1)))
(imf "ang-wight-blue-half" ((32 32) (file "dg_undead32.gif" std 0 2)))
(imf "ang-ghost-black-red" ((32 32) (file "dg_undead32.gif" std 0 3)))
(imf "ang-skeleton-side-arm" ((32 32) (file "dg_undead32.gif" std 0 4)))
(imf "ang-skeleton-left-dagger" ((32 32) (file "dg_undead32.gif" std 0 5)))
(imf "ang-skeleton-shield-sword" ((32 32) (file "dg_undead32.gif" std 0 6)))
(imf "ang-skeleton-double-glowhand" ((32 32) (file "dg_undead32.gif" std 0 7)))
(imf "ang-skeleton-left-scimitar" ((32 32) (file "dg_undead32.gif" std 0 8)))

(imf "ang-wraith-red-flame" ((32 32) (file "dg_undead32.gif" std 1 0)))
(imf "ang-skeleton-red-half" ((32 32) (file "dg_undead32.gif" std 1 1)))
(imf "ang-wight-red-half" ((32 32) (file "dg_undead32.gif" std 1 2)))
(imf "ang-ghost-gray-black" ((32 32) (file "dg_undead32.gif" std 1 3)))
(imf "ang-skeleton-up-arm" ((32 32) (file "dg_undead32.gif" std 1 4)))
(imf "ang-skeleton-right-dagger" ((32 32) (file "dg_undead32.gif" std 1 5)))
(imf "ang-skeleton-cape-sword" ((32 32) (file "dg_undead32.gif" std 1 6)))
(imf "ang-vampire-black-gold" ((32 32) (file "dg_undead32.gif" std 1 7)))
(imf "ang-skeleton-right-flail" ((32 32) (file "dg_undead32.gif" std 1 8)))

(imf "ang-wraith-green-flame" ((32 32) (file "dg_undead32.gif" std 2 0)))
(imf "ang-skeleton-green-half" ((32 32) (file "dg_undead32.gif" std 2 1)))
(imf "ang-wight-green-half" ((32 32) (file "dg_undead32.gif" std 2 2)))
(imf "ang-ghost-blue-black" ((32 32) (file "dg_undead32.gif" std 2 3)))
(imf "ang-skeleton-jazz-hands" ((32 32) (file "dg_undead32.gif" std 2 4)))
(imf "ang-skeleton-left-scimitar2" ((32 32) (file "dg_undead32.gif" std 2 5)))
(imf "ang-skeleton-cape-dagger" ((32 32) (file "dg_undead32.gif" std 2 6)))
(imf "ang-vampire-blue-gold2" ((32 32) (file "dg_undead32.gif" std 2 7)))
(imf "ang-skeleton-glowhand-staff" ((32 32) (file "dg_undead32.gif" std 2 8)))

(imf "ang-wraith-purple-flame" ((32 32) (file "dg_undead32.gif" std 3 0)))
(imf "ang-skeleton-purple-half" ((32 32) (file "dg_undead32.gif" std 3 1)))
(imf "ang-wight-purple-half" ((32 32) (file "dg_undead32.gif" std 3 2)))
(imf "ang-ghost-purple-black" ((32 32) (file "dg_undead32.gif" std 3 3)))
(imf "ang-skeleton-jazz-fist" ((32 32) (file "dg_undead32.gif" std 3 4)))
(imf "ang-skeleton-right-scimitar" ((32 32) (file "dg_undead32.gif" std 3 5)))
(imf "ang-skeleton-cape-scimitar" ((32 32) (file "dg_undead32.gif" std 3 6)))
(imf "ang-vampire-glowhand-red" ((32 32) (file "dg_undead32.gif" std 3 7)))
(imf "ang-skeleton-dual-demon" ((32 32) (file "dg_undead32.gif" std 3 8)))

(imf "ang-wraith-orange-flame" ((32 32) (file "dg_undead32.gif" std 4 0)))
(imf "ang-skeleton-orange-half" ((32 32) (file "dg_undead32.gif" std 4 1)))
(imf "ang-wight-orange-half" ((32 32) (file "dg_undead32.gif" std 4 2)))
(imf "ang-wight-brown-green" ((32 32) (file "dg_undead32.gif" std 4 3)))
(imf "ang-skeleton-really-happy" ((32 32) (file "dg_undead32.gif" std 4 4)))
(imf "ang-skeleton-two-dagger" ((32 32) (file "dg_undead32.gif" std 4 5)))
(imf "ang-skeleton-dagger-scimitar" ((32 32) (file "dg_undead32.gif" std 4 6)))
(imf "ang-vampire-purple-disco" ((32 32) (file "dg_undead32.gif" std 4 7)))
(imf "ang-skeleton-shrug-demon" ((32 32) (file "dg_undead32.gif" std 4 8)))

(imf "ang-wraith-gray-flame" ((32 32) (file "dg_undead32.gif" std 5 0)))
(imf "ang-skeleton-gray-half" ((32 32) (file "dg_undead32.gif" std 5 1)))
(imf "ang-wight-black-half" ((32 32) (file "dg_undead32.gif" std 5 2)))
(imf "ang-wight-blue-glow" ((32 32) (file "dg_undead32.gif" std 5 3)))
(imf "ang-skeleton-really-depressed" ((32 32) (file "dg_undead32.gif" std 5 4)))
(imf "ang-skeleton-two-scimitar" ((32 32) (file "dg_undead32.gif" std 5 5)))
(imf "ang-skeleton-glowhand-staff2" ((32 32) (file "dg_undead32.gif" std 5 6)))
(imf "ang-skeleton-apehead-dagger" ((32 32) (file "dg_undead32.gif" std 5 7)))
(imf "ang-skeleton-wave-pitchfork" ((32 32) (file "dg_undead32.gif" std 5 8)))

(imf "ang-wraith-black-flame" ((32 32) (file "dg_undead32.gif" std 6 0)))
(imf "ang-skeleton-black-half" ((32 32) (file "dg_undead32.gif" std 6 1)))
(imf "ang-ghost-black-gray" ((32 32) (file "dg_undead32.gif" std 6 2)))
(imf "ang-wight-red-flame" ((32 32) (file "dg_undead32.gif" std 6 3)))
(imf "ang-skeleton-slightly-confused" ((32 32) (file "dg_undead32.gif" std 6 4)))
(imf "ang-skeleton-dagger-shield" ((32 32) (file "dg_undead32.gif" std 6 5)))
(imf "ang-skeleton-double-glowhand2" ((32 32) (file "dg_undead32.gif" std 6 6)))
(imf "ang-skeleton-shield-axe" ((32 32) (file "dg_undead32.gif" std 6 7)))
(imf "ang-skeleton-axe-shield" ((32 32) (file "dg_undead32.gif" std 6 8)))


(imf "ang-fighter-small-sword" ((32 32) (file "dg_uniques32.gif" std 0 0)))
(imf "ang-fishman-purple-bigear" ((32 32) (file "dg_uniques32.gif" std 0 1)))
(imf "ang-3eye-glowhand-staff" ((32 32) (file "dg_uniques32.gif" std 0 2)))
(imf "ang-saurial-gray-shield" ((32 32) (file "dg_uniques32.gif" std 0 3)))
(imf "ang-medusa-green-yellow" ((32 32) (file "dg_uniques32.gif" std 0 4)))
(imf "ang-dragon-brown-silver" ((32 32) (file "dg_uniques32.gif" std 0 5)))
(imf "ang-vampire-blue-gold" ((32 32) (file "dg_uniques32.gif" std 0 6)))
(imf "ang-lich-purple-red" ((32 32) (file "dg_uniques32.gif" std 0 7)))
(imf "ang-alien-blue-bighead" ((32 32) (file "dg_uniques32.gif" std 0 8)))

(imf "ang-ogre-shield-sword" ((32 32) (file "dg_uniques32.gif" std 1 0)))
(imf "ang-dwarf-gold-shield" ((32 32) (file "dg_uniques32.gif" std 1 1)))
(imf "ang-3eye-gray-sword" ((32 32) (file "dg_uniques32.gif" std 1 2)))
(imf "ang-fishman-skull-shield" ((32 32) (file "dg_uniques32.gif" std 1 3)))
(imf "ang-mage-green-glowhand" ((32 32) (file "dg_uniques32.gif" std 1 4)))
(imf "ang-dragon-red-frill" ((32 32) (file "dg_uniques32.gif" std 1 5)))
(imf "ang-hobbit-ankh-green" ((32 32) (file "dg_uniques32.gif" std 1 6)))
(imf "ang-eye-floating-red" ((32 32) (file "dg_uniques32.gif" std 1 7)))
(imf "ang-demon-horn-sword" ((32 32) (file "dg_uniques32.gif" std 1 8)))

(imf "ang-lord-purple-diseased" ((32 32) (file "dg_uniques32.gif" std 2 0)))
(imf "ang-orc-halberd-shield" ((32 32) (file "dg_uniques32.gif" std 2 1)))
(imf "ang-fishman-purple-sword" ((32 32) (file "dg_uniques32.gif" std 2 2)))
(imf "ang-saurial-black-trident" ((32 32) (file "dg_uniques32.gif" std 2 3)))
(imf "ang-king-glowhand-staff" ((32 32) (file "dg_uniques32.gif" std 2 4)))
(imf "ang-dragon-blue-silver" ((32 32) (file "dg_uniques32.gif" std 2 5)))
(imf "ang-angel-flame-wing" ((32 32) (file "dg_uniques32.gif" std 2 6)))
(imf "ang-spider-red-ugly" ((32 32) (file "dg_uniques32.gif" std 2 7)))
(imf "ang-skeleton-cape-glowhand" ((32 32) (file "dg_uniques32.gif" std 2 8)))

(imf "ang-hobgoblin-sword-cape" ((32 32) (file "dg_uniques32.gif" std 3 0)))
(imf "ang-orc-flail-shield" ((32 32) (file "dg_uniques32.gif" std 3 1)))
(imf "ang-dwarf-glowhand-staff" ((32 32) (file "dg_uniques32.gif" std 3 2)))
(imf "ang-fishman-crystal-staff" ((32 32) (file "dg_uniques32.gif" std 3 3)))
(imf "ang-fighter-chromatic-shield" ((32 32) (file "dg_uniques32.gif" std 3 4)))
(imf "ang-alien-beige-naked" ((32 32) (file "dg_uniques32.gif" std 3 5)))
(imf "ang-angel-black-wing" ((32 32) (file "dg_uniques32.gif" std 3 6)))
(imf "ang-lich-glowhand-cape" ((32 32) (file "dg_uniques32.gif" std 3 7)))
(imf "ang-hellhound-spiked-tail" ((32 32) (file "dg_uniques32.gif" std 3 8)))

(imf "ang-fishman-plate-armor" ((32 32) (file "dg_uniques32.gif" std 4 0)))
(imf "ang-orc-red-angry" ((32 32) (file "dg_uniques32.gif" std 4 1)))
(imf "ang-ogre-red-staff" ((32 32) (file "dg_uniques32.gif" std 4 2)))
(imf "ang-ant-big-red" ((32 32) (file "dg_uniques32.gif" std 4 3)))
(imf "ang-mage-red-skin" ((32 32) (file "dg_uniques32.gif" std 4 4)))
(imf "ang-minotaur-plate-sword" ((32 32) (file "dg_uniques32.gif" std 4 5)))
(imf "ang-dragon-green-gray" ((32 32) (file "dg_uniques32.gif" std 4 6)))
(imf "ang-blob-pink-red" ((32 32) (file "dg_uniques32.gif" std 4 7)))
(imf "ang-hellhound-two-head" ((32 32) (file "dg_uniques32.gif" std 4 8)))

(imf "ang-marmot-angry-staff" ((32 32) (file "dg_uniques32.gif" std 5 0)))
(imf "ang-orc-cross-plate" ((32 32) (file "dg_uniques32.gif" std 5 1)))
(imf "ang-fighter-gray-skull" ((32 32) (file "dg_uniques32.gif" std 5 2)))
(imf "ang-drow-cape-scimitar" ((32 32) (file "dg_uniques32.gif" std 5 3)))
(imf "ang-ogre-red-glowhand" ((32 32) (file "dg_uniques32.gif" std 5 4)))
(imf "ang-drow-pouch-scimitar" ((32 32) (file "dg_uniques32.gif" std 5 5)))
(imf "ang-angel-white-wing" ((32 32) (file "dg_uniques32.gif" std 5 6)))
(imf "ang-blob-purple-clear" ((32 32) (file "dg_uniques32.gif" std 5 7)))
(imf "ang-alien-red-fist" ((32 32) (file "dg_uniques32.gif" std 5 8)))

(imf "ang-orc-red-scimitar" ((32 32) (file "dg_uniques32.gif" std 6 0)))
(imf "ang-orc-angry-staff" ((32 32) (file "dg_uniques32.gif" std 6 1)))
(imf "ang-demon-orange-flail" ((32 32) (file "dg_uniques32.gif" std 6 2)))
(imf "ang-elemental-humanoid-flame" ((32 32) (file "dg_uniques32.gif" std 6 3)))
(imf "ang-elemental-humanoid-basalt" ((32 32) (file "dg_uniques32.gif" std 6 4)))
(imf "ang-mage-blue-glowhand" ((32 32) (file "dg_uniques32.gif" std 6 5)))
(imf "ang-mage-rainbow-glowhand" ((32 32) (file "dg_uniques32.gif" std 6 6)))
(imf "ang-king-purple-glowhand" ((32 32) (file "dg_uniques32.gif" std 6 7)))
(imf "ang-lord-blue-drow" ((32 32) (file "dg_uniques32.gif" std 6 8)))

(imf "ang-king-red-orc" ((32 32) (file "dg_uniques32.gif" std 7 0)))
(imf "ang-fighter-gold-gold" ((32 32) (file "dg_uniques32.gif" std 7 1)))
(imf "ang-spider-black-ugly" ((32 32) (file "dg_uniques32.gif" std 7 2)))
(imf "ang-elemental-humanoid-water" ((32 32) (file "dg_uniques32.gif" std 7 3)))
(imf "ang-elemental-humanoid-air" ((32 32) (file "dg_uniques32.gif" std 7 4)))
(imf "ang-lord-purple-fishman" ((32 32) (file "dg_uniques32.gif" std 7 5)))
(imf "ang-lord-green-staff" ((32 32) (file "dg_uniques32.gif" std 7 6)))
(imf "ang-angel-scimitar-cape" ((32 32) (file "dg_uniques32.gif" std 7 7)))
(imf "ang-drow-no-armor" ((32 32) (file "dg_uniques32.gif" std 7 8)))

(imf "ang-marmot-glowing-purple" ((32 32) (file "dg_uniques32.gif" std 8 0)))
(imf "ang-dwarf-orange-glowhand2" ((32 32) (file "dg_uniques32.gif" std 8 1)))
(imf "ang-saurial-gray-club" ((32 32) (file "dg_uniques32.gif" std 8 2)))
(imf "ang-dragon-three-head" ((32 32) (file "dg_uniques32.gif" std 8 3)))
(imf "ang-dragon-white-gold" ((32 32) (file "dg_uniques32.gif" std 8 4)))
(imf "ang-phoenix-yellow-gold" ((32 32) (file "dg_uniques32.gif" std 8 5)))
(imf "ang-king-half-skeleton" ((32 32) (file "dg_uniques32.gif" std 8 6)))
(imf "ang-king-skeleton-staff" ((32 32) (file "dg_uniques32.gif" std 8 7)))
(imf "ang-mage-hippie-glowhand" ((32 32) (file "dg_uniques32.gif" std 8 8)))

(imf "ang-orc-defiant-sword" ((32 32) (file "dg_uniques32.gif" std 9 0)))
(imf "ang-gnome-two-glowhand" ((32 32) (file "dg_uniques32.gif" std 9 1)))
(imf "ang-saurial-gray-axe" ((32 32) (file "dg_uniques32.gif" std 9 2)))
(imf "ang-thief-black-brown" ((32 32) (file "dg_uniques32.gif" std 9 3)))
(imf "ang-gnoll-two-sword" ((32 32) (file "dg_uniques32.gif" std 9 4)))
(imf "ang-monster-eye-head" ((32 32) (file "dg_uniques32.gif" std 9 5)))
(imf "ang-dragon-purple-blue" ((32 32) (file "dg_uniques32.gif" std 9 6)))
(imf "ang-monster-who-knows" ((32 32) (file "dg_uniques32.gif" std 9 7)))
(imf "ang-coat-sword-unicorn" ((32 32) (file "dg_uniques32.gif" std 9 8)))


(imf "ang-hobbit-purple-diseased" ((32 32) (file "dg_people32.gif" std 0 0)))
(imf "ang-jester-orange-bell" ((32 32) (file "dg_people32.gif" std 0 1)))
(imf "ang-man-brown-drunk" ((32 32) (file "dg_people32.gif" std 0 2)))

(imf "ang-man-cap-pitchfork" ((32 32) (file "dg_people32.gif" std 1 0)))
(imf "ang-man-brown-hypnotized" ((32 32) (file "dg_people32.gif" std 1 1)))
(imf "ang-man-blonde-beard" ((32 32) (file "dg_people32.gif" std 1 2)))

(imf "ang-man-green-hunchback" ((32 32) (file "dg_people32.gif" std 2 0)))
(imf "ang-man-green-diseased" ((32 32) (file "dg_people32.gif" std 2 1)))
(imf "ang-lord-red-cape" ((32 32) (file "dg_people32.gif" std 2 2)))

(imf "ang-man-purple-diseased" ((32 32) (file "dg_people32.gif" std 3 0)))
(imf "ang-lord-blue-cape" ((32 32) (file "dg_people32.gif" std 3 1)))
(imf "ang-lord-brown-cape" ((32 32) (file "dg_people32.gif" std 3 2)))


(imf "ang-archer-blonde-shortbow" ((32 32) (file "dg_humans32.gif" std 0 0)))
(imf "ang-drow-ankh-green" ((32 32) (file "dg_humans32.gif" std 0 1)))
(imf "ang-orc-cape-sword" ((32 32) (file "dg_humans32.gif" std 0 2)))
(imf "ang-drow-cross-shield" ((32 32) (file "dg_humans32.gif" std 0 3)))
(imf "ang-ogre-glowhand-staff" ((32 32) (file "dg_humans32.gif" std 0 4)))
(imf "ang-mage-double-glowhand" ((32 32) (file "dg_humans32.gif" std 0 5)))

(imf "ang-thief-red-cape" ((32 32) (file "dg_humans32.gif" std 1 0)))
(imf "ang-dwarf-sword-skirt" ((32 32) (file "dg_humans32.gif" std 1 1)))
(imf "ang-mage-white-glowhand" ((32 32) (file "dg_humans32.gif" std 1 2)))
(imf "ang-drow-purple-glowhand" ((32 32) (file "dg_humans32.gif" std 1 3)))
(imf "ang-drow-stripe-shield" ((32 32) (file "dg_humans32.gif" std 1 4)))
(imf "ang-drow-cape-dagger" ((32 32) (file "dg_humans32.gif" std 1 5)))

(imf "ang-dwarf-ankh-staff" ((32 32) (file "dg_humans32.gif" std 2 0)))
(imf "ang-man-cape-apehead" ((32 32) (file "dg_humans32.gif" std 2 1)))
(imf "ang-fighter-sword-dagger" ((32 32) (file "dg_humans32.gif" std 2 2)))
(imf "ang-ninja-jumpsuit-dagger" ((32 32) (file "dg_humans32.gif" std 2 3)))
(imf "ang-woman-white-glowhand" ((32 32) (file "dg_humans32.gif" std 2 4)))
(imf "ang-drow-cape-staff" ((32 32) (file "dg_humans32.gif" std 2 5)))

(imf "ang-ogre-staff-glowhand" ((32 32) (file "dg_humans32.gif" std 3 0)))
(imf "ang-archer-longbow-green" ((32 32) (file "dg_humans32.gif" std 3 1)))
(imf "ang-dwarf-double-glowhand" ((32 32) (file "dg_humans32.gif" std 3 2)))
(imf "ang-drow-black-glowhand" ((32 32) (file "dg_humans32.gif" std 3 3)))
(imf "ang-fighter-ankh-cape" ((32 32) (file "dg_humans32.gif" std 3 4)))
(imf "ang-fighter-gold-helmet" ((32 32) (file "dg_humans32.gif" std 3 5)))

(imf "ang-fighter-staff-beard" ((32 32) (file "dg_humans32.gif" std 4 0)))
(imf "ang-fighter-dagger-apehead" ((32 32) (file "dg_humans32.gif" std 4 1)))
(imf "ang-dwarf-brown-staff" ((32 32) (file "dg_humans32.gif" std 4 2)))
(imf "ang-thief-double-dagger" ((32 32) (file "dg_humans32.gif" std 4 3)))
(imf "ang-hobbit-staff-glowhand" ((32 32) (file "dg_humans32.gif" std 4 4)))
(imf "ang-fighter-gold-cross" ((32 32) (file "dg_humans32.gif" std 4 5)))

(imf "ang-fighter-gold-cross2" ((32 32) (file "dg_humans32.gif" std 5 0)))
(imf "ang-fighter-gold-sword" ((32 32) (file "dg_humans32.gif" std 5 1)))
(imf "ang-ogre-chain-axe" ((32 32) (file "dg_humans32.gif" std 5 2)))
(imf "ang-fighter-double-scimitar" ((32 32) (file "dg_humans32.gif" std 5 3)))
(imf "ang-priest-ankh-apehead" ((32 32) (file "dg_humans32.gif" std 5 4)))
(imf "ang-ogre-gold-cross" ((32 32) (file "dg_humans32.gif" std 5 5)))

(imf "ang-dwarf-orange-glowhand" ((32 32) (file "dg_humans32.gif" std 6 0)))
(imf "ang-dwarf-scimitar-beard" ((32 32) (file "dg_humans32.gif" std 6 1)))
(imf "ang-dwarf-blue-cape" ((32 32) (file "dg_humans32.gif" std 6 2)))
(imf "ang-mage-purple-glowhand" ((32 32) (file "dg_humans32.gif" std 6 3)))
(imf "ang-drow-black-glowhand2" ((32 32) (file "dg_humans32.gif" std 6 4)))
(imf "ang-fighter-sword-dagger2" ((32 32) (file "dg_humans32.gif" std 6 5)))


(imf "ang-man-brown-stone" ((32 32) (file "dg_monster632.gif" std 0 0)))
(imf "ang-man-blue-gem" ((32 32) (file "dg_monster632.gif" std 0 1)))
(imf "ang-man-flaming-arms" ((32 32) (file "dg_monster632.gif" std 0 2)))
(imf "ang-mushhead-single-sword" ((32 32) (file "dg_monster632.gif" std 0 3)))
(imf "ang-goblin-red-scimitar" ((32 32) (file "dg_monster632.gif" std 0 4)))
(imf "ang-hand-purple-skeleton" ((32 32) (file "dg_monster632.gif" std 0 5)))
(imf "ang-tornado-gold-flame" ((32 32) (file "dg_monster632.gif" std 0 6)))
(imf "ang-tornado-gray-regular" ((32 32) (file "dg_monster632.gif" std 0 7)))
(imf "ang-saurial-ankh-staff" ((32 32) (file "dg_monster632.gif" std 0 8)))
(imf "ang-saurial-single-scimitar" ((32 32) (file "dg_monster632.gif" std 0 9)))
(imf "ang-saurial-cross-cloak" ((32 32) (file "dg_monster632.gif" std 0 10)))
(imf "ang-turtle-green-brown" ((32 32) (file "dg_monster632.gif" std 0 11)))
(imf "ang-scorpian-pink-giant" ((32 32) (file "dg_monster632.gif" std 0 12)))

(imf "ang-man-pink-stone" ((32 32) (file "dg_monster632.gif" std 1 0)))
(imf "ang-man-cloak-stone" ((32 32) (file "dg_monster632.gif" std 1 1)))
(imf "ang-man-dressed-stone" ((32 32) (file "dg_monster632.gif" std 1 2)))
(imf "ang-mushhead-single-scimitar" ((32 32) (file "dg_monster632.gif" std 1 3)))
(imf "ang-goblin-green-glowhand" ((32 32) (file "dg_monster632.gif" std 1 4)))
(imf "ang-skeleton-eye-purple" ((32 32) (file "dg_monster632.gif" std 1 5)))
(imf "ang-tornado-blue-flame" ((32 32) (file "dg_monster632.gif" std 1 6)))
(imf "ang-tornado-turq-flame" ((32 32) (file "dg_monster632.gif" std 1 7)))
(imf "ang-saurial-scimitar-red" ((32 32) (file "dg_monster632.gif" std 1 8)))
(imf "ang-saurial-polearm-shirtless" ((32 32) (file "dg_monster632.gif" std 1 9)))
(imf "ang-saurial-axe-shield" ((32 32) (file "dg_monster632.gif" std 1 10)))
(imf "ang-turtle-pink-brown" ((32 32) (file "dg_monster632.gif" std 1 11)))
(imf "ang-scorpian-yellow-giant" ((32 32) (file "dg_monster632.gif" std 1 12)))

(imf "ang-man-gray-stone" ((32 32) (file "dg_monster632.gif" std 2 0)))
(imf "ang-man-fang-stone" ((32 32) (file "dg_monster632.gif" std 2 1)))
(imf "ang-man-turq-spacesuit" ((32 32) (file "dg_monster632.gif" std 2 2)))
(imf "ang-mushhead-flail-shield" ((32 32) (file "dg_monster632.gif" std 2 3)))
(imf "ang-goblin-blue-sword" ((32 32) (file "dg_monster632.gif" std 2 4)))
(imf "ang-head-purple-skeleton" ((32 32) (file "dg_monster632.gif" std 2 5)))
(imf "ang-tornado-lblue-flame" ((32 32) (file "dg_monster632.gif" std 2 6)))
(imf "ang-tornado-black-regular" ((32 32) (file "dg_monster632.gif" std 2 7)))
(imf "ang-saurial-lblue-frozen" ((32 32) (file "dg_monster632.gif" std 2 8)))
(imf "ang-saurial-red-pitchfork" ((32 32) (file "dg_monster632.gif" std 2 9)))
(imf "ang-saurial-barbarian-fist" ((32 32) (file "dg_monster632.gif" std 2 10)))
(imf "ang-turtle-gray-brown" ((32 32) (file "dg_monster632.gif" std 2 11)))
(imf "ang-scorpian-gray-giant" ((32 32) (file "dg_monster632.gif" std 2 12)))

(imf "ang-man-purple-stone" ((32 32) (file "dg_monster632.gif" std 3 0)))
(imf "ang-dino-red-frilled" ((32 32) (file "dg_monster632.gif" std 3 1)))
(imf "ang-man-cobalt-spacesuit" ((32 32) (file "dg_monster632.gif" std 3 2)))
(imf "ang-mushhead-staff-cloak" ((32 32) (file "dg_monster632.gif" std 3 3)))
(imf "ang-goblin-pink-plate" ((32 32) (file "dg_monster632.gif" std 3 4)))
(imf "ang-dino-claw-hand" ((32 32) (file "dg_monster632.gif" std 3 5)))
(imf "ang-tornado-blue-lightning" ((32 32) (file "dg_monster632.gif" std 3 6)))
(imf "ang-tornado-varicolored-strange" ((32 32) (file "dg_monster632.gif" std 3 7)))
(imf "ang-saurial-club-uni" ((32 32) (file "dg_monster632.gif" std 3 8)))
(imf "ang-saurial-half-ghost" ((32 32) (file "dg_monster632.gif" std 3 9)))
(imf "ang-serpent-blue-sea" ((32 32) (file "dg_monster632.gif" std 3 10)))
(imf "ang-turtle-brown-green" ((32 32) (file "dg_monster632.gif" std 3 11)))
(imf "ang-scorpian-blue-giant" ((32 32) (file "dg_monster632.gif" std 3 12)))

(imf "ang-man-gloweye-stone" ((32 32) (file "dg_monster632.gif" std 4 0)))
(imf "ang-man-sleeveless-spacesuit" ((32 32) (file "dg_monster632.gif" std 4 1)))
(imf "ang-man-red-spacesuit" ((32 32) (file "dg_monster632.gif" std 4 2)))
(imf "ang-mushhead-sword-dagger" ((32 32) (file "dg_monster632.gif" std 4 3)))
(imf "ang-goblin-yellow-halberd" ((32 32) (file "dg_monster632.gif" std 4 4)))
(imf "ang-dino-gray-tusk" ((32 32) (file "dg_monster632.gif" std 4 5)))
(imf "ang-tornado-purple-lightning" ((32 32) (file "dg_monster632.gif" std 4 6)))
(imf "ang-saurial-tunic-staff" ((32 32) (file "dg_monster632.gif" std 4 7)))
(imf "ang-man-blue-shield" ((32 32) (file "dg_monster632.gif" std 4 8)))
(imf "ang-saurial-glowhand-staff" ((32 32) (file "dg_monster632.gif" std 4 9)))
(imf "ang-elemental-lblue-water" ((32 32) (file "dg_monster632.gif" std 4 10)))
(imf "ang-turtle-blue-brown" ((32 32) (file "dg_monster632.gif" std 4 11)))
(imf "ang-scorpian-brown-giant" ((32 32) (file "dg_monster632.gif" std 4 12)))

(imf "ang-man-electric-stone" ((32 32) (file "dg_monster632.gif" std 5 0)))
(imf "ang-man-melting-spacesuit" ((32 32) (file "dg_monster632.gif" std 5 1)))
(imf "ang-man-staff-spacesuit" ((32 32) (file "dg_monster632.gif" std 5 2)))
(imf "ang-mushhead-glowhand-staff" ((32 32) (file "dg_monster632.gif" std 5 3)))
(imf "ang-goblin-gray-plate" ((32 32) (file "dg_monster632.gif" std 5 4)))
(imf "ang-dino-green-tusk" ((32 32) (file "dg_monster632.gif" std 5 5)))
(imf "ang-tornado-orange-lightning" ((32 32) (file "dg_monster632.gif" std 5 6)))
(imf "ang-saurial-gray-naked" ((32 32) (file "dg_monster632.gif" std 5 7)))
(imf "ang-saurial-blue-melting" ((32 32) (file "dg_monster632.gif" std 5 8)))
(imf "ang-saurial-crown-staff" ((32 32) (file "dg_monster632.gif" std 5 9)))
(imf "ang-eye-red-blue" ((32 32) (file "dg_monster632.gif" std 5 10)))
(imf "ang-turtle-gold-brown" ((32 32) (file "dg_monster632.gif" std 5 11)))
(imf "ang-scorpian-black-giant" ((32 32) (file "dg_monster632.gif" std 5 12)))


(imf "ang-mage-black-caped" ((32 32) (file "dg_abyss.gif" std 0 0)))
(imf "ang-thief-black-cloak" ((32 32) (file "dg_abyss.gif" std 0 1)))
(imf "ang-fighter-cross-shield" ((32 32) (file "dg_abyss.gif" std 0 2)))
(imf "ang-mage-black-cloak" ((32 32) (file "dg_abyss.gif" std 0 3)))

(imf "ang-mage-red-cloak" ((32 32) (file "dg_abyss.gif" std 1 0)))
(imf "ang-fighter-ape-head" ((32 32) (file "dg_abyss.gif" std 1 1)))
(imf "ang-fighter-skull-shield" ((32 32) (file "dg_abyss.gif" std 1 2)))
(imf "ang-mage-two-sword" ((32 32) (file "dg_abyss.gif" std 1 3)))

(imf "ang-fighter-plain-shield" ((32 32) (file "dg_abyss.gif" std 2 0)))
(imf "ang-fighter-skinny-axe" ((32 32) (file "dg_abyss.gif" std 2 1)))
(imf "ang-fighter-red-leggings" ((32 32) (file "dg_abyss.gif" std 2 2)))

(imf "ang-pirate-red-cap" ((32 32) (file "dg_abyss.gif" std 3 0)))
(imf "ang-fighter-animal-fur" ((32 32) (file "dg_abyss.gif" std 3 1)))
(imf "ang-fighter-lblue-duel" ((32 32) (file "dg_abyss.gif" std 3 2)))

(imf "spec-city-bubble1" ((32 32) (file "korea.gif" std 4 5)))
(imf "spec-city-bubble2" ((32 32) (file "korea.gif" std 4 6)))
(imf "spec-phantom-blob" ((32 32) (file "korea.gif" std 4 7)))
(imf "spec-phantom-nameless" ((32 32) (file "korea.gif" std 4 8)))

(imf "spec-phantom-castle" ((32 32) (file "korea.gif" std 5 5)))
(imf "spec-golem-wood" ((32 32) (file "korea.gif" std 5 6)))
(imf "spec-golem-stone" ((32 32) (file "korea.gif" std 5 7)))
(imf "spec-golem-iron" ((32 32) (file "korea.gif" std 5 8)))
(imf "spec-golem-brass" ((32 32) (file "korea.gif" std 5 9)))

(imf "spec-phantom-ship" ((32 32) (file "korea.gif" std 6 5)))
(imf "spec-phantom-pavilion" ((32 32) (file "korea.gif" std 6 6)))
(imf "spec-phantom-beast" ((32 32) (file "korea.gif" std 6 7)))
(imf "spec-phantom-wisp" ((32 32) (file "korea.gif" std 6 8)))
(imf "spec-phantom-warrior" ((32 32) (file "korea.gif" std 6 9)))

(imf "spec-mine-gray" ((32 32) (file "korea.gif" std 7 0)))
(imf "spec-mine-brown" ((32 32) (file "korea.gif" std 7 1)))
(imf "spec-mine-green" ((32 32) (file "korea.gif" std 7 2)))
(imf "spec-tree-brown" ((32 32) (file "korea.gif" std 7 3)))
(imf "spec-volcano-flow" ((32 32) (file "korea.gif" std 7 4)))
(imf "spec-wildlife-bear" ((32 32) (file "korea.gif" std 7 5)))
(imf "spec-fish-levi" ((32 32) (file "korea.gif" std 7 6)))
(imf "spec-fish-bighead" ((32 32) (file "korea.gif" std 7 7)))
(imf "spec-fish-spot" ((32 32) (file "korea.gif" std 7 8)))
(imf "spec-tent2" ((32 32) (file "korea.gif" std 7 9)))

(imf "tolk-river" ((44 48 border) (file "tolk-rivers.gif")))

(imf "tolk-hills" ((1 1) 38250 43095 14025))
(imf "tolk-jungle" ((1 1) 53040 54825 27795))
(imf "tolk-swamp" ((1 1) 62475 53550 39270))
(imf "tolk-dunes" ((1 1) 65025 59925 37740))
(imf "tolk-plains" ((1 1) 35700 47940 12495))
(imf "tolk-forest" ((1 1) 17595 30345 10455))
(imf "tolk-mountains" ((1 1) 43350 31365 24255))
(imf "tolk-ice" ((1 1) 57375 62475 63495))

(imf "tolk-hills" ((12 13 terrain) (x 8 14 0) (file "advt12x13.gif" std 0 0)))
(imf "tolk-jungle" ((12 13 terrain) (x 8 14 0) (file "advt12x13.gif" std 0 1)))
(imf "tolk-swamp" ((12 13 terrain) (x 8 14 0) (file "advt12x13.gif" std 0 2)))
(imf "tolk-dunes" ((12 13 terrain) (x 8 14 0) (file "advt12x13.gif" std 0 3)))
(imf "tolk-plains" ((12 13 terrain) (x 8 14 0) (file "advt12x13.gif" std 0 4)))
(imf "tolk-forest" ((12 13 terrain) (x 8 14 0) (file "advt12x13.gif" std 0 5)))
(imf "tolk-mountains" ((12 13 terrain) (x 8 14 0) (file "advt12x13.gif" std 0 6)))
(imf "tolk-ice" ((12 13 terrain) (x 8 14 0) (file "advt12x13.gif" std 0 7)))

(imf "tolk-mountains" ((24 26 terrain) (x 8 26 0) (file "tolk-terrain24x26.gif" std 0 0)))
(imf "tolk-hills" ((24 26 terrain) (x 8 26 0) (file "tolk-terrain24x26.gif" std 0 1)))
(imf "tolk-ocean" ((24 26 terrain) (x 8 26 0) (file "tolk-terrain24x26.gif" std 0 2)))
(imf "tolk-lake" ((24 26 terrain) (x 8 26 0) (file "tolk-terrain24x26.gif" std 0 3)))
(imf "tolk-forest" ((24 26 terrain) (x 8 26 0) (file "tolk-terrain24x26.gif" std 0 4)))
(imf "tolk-plains" ((24 26 terrain) (x 8 26 0) (file "tolk-terrain24x26.gif" std 0 5)))
(imf "tolk-jungle" ((24 26 terrain) (x 8 26 0) (file "tolk-terrain24x26.gif" std 0 6)))
(imf "tolk-ice" ((24 26 terrain) (x 8 26 0 terrain) (file "tolk-terrain24x26.gif" std 0 7)))
(imf "tolk-dunes" ((24 26 terrain) (x 8 26 0 terrain) (file "tolk-terrain24x26.gif" std 0 8)))
(imf "tolk-swamp" ((24 26 terrain) (x 8 26 0 terrain) (file "tolk-terrain24x26.gif" std 0 9)))
(imf "tolk-wasteland" ((24 26 terrain) (x 8 26 0 terrain) (file "tolk-terrain24x26.gif" std 0 10)))
(imf "tolk-steppe" ((24 26 terrain) (x 8 26 0 terrain) (file "tolk-terrain24x26.gif" std 0 11)))

(imf "tolk-mountains" ((44 48 terrain) (x 8 46 0) (file "tolk-terrain.gif" std 0 0)))
(imf "tolk-hills" ((44 48 terrain) (x 8 46 0) (file "tolk-terrain.gif" std 0 1)))
(imf "tolk-ocean" ((44 48 terrain) (x 8 46 0) (file "tolk-terrain.gif" std 0 2)))
(imf "tolk-lake" ((44 48 terrain) (x 8 46 0) (file "tolk-terrain.gif" std 0 3)))
(imf "tolk-forest" ((44 48 terrain) (x 8 46 0) (file "tolk-terrain.gif" std 0 4)))
(imf "tolk-plains" ((44 48 terrain) (x 8 46 0) (file "tolk-terrain.gif" std 0 5)))
(imf "tolk-jungle" ((44 48 terrain) (x 8 46 0) (file "tolk-terrain.gif" std 0 6)))
(imf "tolk-ice" ((44 48 terrain) (x 8 46 0) (file "tolk-terrain.gif" std 0 7)))
(imf "tolk-dunes" ((44 48 terrain) (x 8 46 0) (file "tolk-terrain.gif" std 0 8)))
(imf "tolk-swamp" ((44 48 terrain) (x 8 46 0) (file "tolk-terrain.gif" std 0 9)))
(imf "tolk-wasteland" ((44 48 terrain) (x 8 46 0) (file "tolk-terrain.gif" std 0 10)))
(imf "tolk-steppe" ((44 48 terrain) (x 8 46 0) (file "tolk-terrain.gif" std 0 11)))

(imf "tolk-hills" ((32 32 tile) (file "tolk-terrain32x32.gif" 2 2)))
(imf "tolk-jungle" ((32 32 tile) (file "tolk-terrain32x32.gif" 2 36)))
(imf "tolk-dunes" ((32 32 tile) (file "tolk-terrain32x32.gif" 2 70)))
(imf "tolk-ocean" ((32 32 tile) (file "tolk-terrain32x32.gif" 2 104)))
(imf "tolk-plains" ((32 32 tile) (file "tolk-terrain32x32.gif" 2 138)))
(imf "tolk-forest" ((32 32 tile) (file "tolk-terrain32x32.gif" 2 172)))
(imf "tolk-mountains" ((32 32 tile) (file "tolk-terrain32x32.gif" 2 206)))
(imf "tolk-lake" ((32 32 tile) (file "tolk-terrain32x32.gif" 2 240)))
(imf "tolk-wasteland" ((32 32 tile) (file "tolk-terrain32x32.gif" 36 2)))
(imf "tolk-ice" ((32 32 tile) (file "tolk-terrain32x32.gif" 36 36)))
(imf "tolk-swamp" ((32 32 tile) (file "tolk-terrain32x32.gif" 36 70)))
(imf "tolk-steppe" ((32 32 tile) (file "tolk-terrain32x32.gif" 36 104)))

(imf "opal-swordsman-sho" ((88 88) (file "opal-88x88-1.gif" std 0 0)))

(imf "opal-swordsman-liz" ((88 88) (file "opal-88x88-1.gif" std 1 0)))

(imf "opal-swordsman-ari" ((88 88) (file "opal-88x88-1.gif" std 2 0)))

(imf "opal-swordsman-oni" ((88 88) (file "opal-88x88-1.gif" std 3 0)))

(imf "spec-keepruin" ((32 32) (file "spec1.gif" std 6 5)))
(imf "spec-citadelruin" ((32 32) (file "spec1.gif" std 6 6)))
(imf "spec-castle" ((32 32) (file "spec1.gif" std 6 7)))
(imf "spec-fort" ((32 32) (file "spec1.gif" std 6 8)))
(imf "spec-doorway" ((32 32) (file "spec1.gif" std 6 9)))

(imf "spec-tower"  ((32 32) (file "spec1.gif" std 9 0)))
(imf "spec-keep"  ((32 32) (file "spec1.gif" std 9 1)))
(imf "spec-citadel"  ((32 32) (file "spec1.gif" std 9 2)))
(imf "spec-needle"  ((32 32) (file "spec1.gif" std 9 3)))
(imf "spec-hamlet"  ((32 32) (file "spec1.gif" std 9 4)))
(imf "spec-ogvillage"  ((32 32) (file "spec1.gif" std 9 5)))
(imf "spec-lair"  ((32 32) (file "spec1.gif" std 9 6)))
(imf "spec-heart"  ((32 32) (file "spec1.gif" std 9 7)))
(imf "spec-node"  ((32 32) (file "spec1.gif" std 9 8)))
(imf "spec-spell"  ((32 32) (file "spec1.gif" std 9 9)))

(imf "spec-blank"  ((8 8) (file "spec8.gif" std 0 0)))
(imf "spec-al"  ((8 8) (file "spec8.gif" std 1 0)))
(imf "spec-ar"  ((8 8) (file "spec8.gif" std 2 0)))
(imf "spec-con"  ((8 8) (file "spec8.gif" std 3 0)))
(imf "spec-sho"  ((8 8) (file "spec8.gif" std 4 0)))
(imf "spec-oni"  ((8 8) (file "spec8.gif" std 5 0)))
(imf "spec-emp"  ((8 8) (file "spec8.gif" std 6 0)))
(imf "spec-bed"  ((8 8) (file "spec8.gif" std 7 0)))
(imf "spec-cit"  ((8 8) (file "spec8.gif" std 8 0)))
(imf "spec-und"  ((8 8) (file "spec8.gif" std 9 0)))
(imf "spec-liz"  ((8 8) (file "spec8.gif" std 10 0)))
(imf "spec-kra"  ((8 8) (file "spec8.gif" std 11 0)))

(imf "ba-archer-dwarf" ((32 32) (file "battles1.gif" std 8 2)))
(imf "ba-archer-drow" ((32 32) (file "battles1.gif" std 8 3)))
(imf "ba-swamp" ((32 32) (file "battles1.gif" std 8 4)))
(imf "ba-castle" ((32 32) (file "battles1.gif" std 8 6)))

(imf "ba-orc-mage-a" ((32 32) (file "battles1.gif" std 7 0)))
(imf "ba-orc-mage-b" ((32 32) (file "battles1.gif" std 7 1)))
(imf "ba-orc-mage-c" ((32 32) (file "battles1.gif" std 7 2)))
(imf "ba-orc-archer" ((32 32) (file "battles1.gif" std 7 3)))
(imf "ba-cannon" ((32 32) (file "battles1.gif" std 7 4)))

(imf "isoph-mountains" ((32 32 tile) (file "isoph32x32.gif" 2 2)))
(imf "isoph-hills" ((32 32 tile) (file "isoph32x32.gif" 2 36)))
(imf "isoph-forest" ((32 32 tile) (file "isoph32x32.gif" 2 70)))
(imf "isoph-light-forest" ((32 32 tile) (file "isoph32x32.gif" 2 104)))

(imf "isoph-mountains" ((44 48 terrain) (x 8 46 0) (file "isoph44x48.gif" std 0 0)))
(imf "isoph-hills" ((44 48 terrain) (x 8 46 0) (file "isoph44x48.gif" std 0 1)))
(imf "isoph-light-forest" ((44 48 terrain) (x 8 46 0) (file "isoph44x48.gif" std 0 2)))
(imf "isoph-forest" ((44 48 terrain) (x 8 46 0) (file "isoph44x48.gif" std 0 3)))

(imf "kiwiterr-mountains" ((32 32 tile) (file "kiwiterr32x32.gif" 2 2)))
(imf "kiwiterr-hills" ((32 32 tile) (file "kiwiterr32x32.gif" 2 36)))
(imf "kiwiterr-forest" ((32 32 tile) (file "kiwiterr32x32.gif" 2 70)))
(imf "kiwiterr-jungle" ((32 32 tile) (file "kiwiterr32x32.gif" 2 104)))
(imf "kiwiterr-grass" ((32 32 tile) (file "kiwiterr32x32.gif" 2 138)))
(imf "kiwiterr-swamp" ((32 32 tile) (file "kiwiterr32x32.gif" 2 172)))
(imf "kiwiterr-rubble" ((32 32 tile) (file "kiwiterr32x32.gif" 2 206)))

(imf "kiwiterr-mountains" ((44 48 terrain) (x 8 46 0) (file "kiwiterr44x48.gif" std 0 0)))
(imf "kiwiterr-hills" ((44 48 terrain) (x 8 46 0) (file "kiwiterr44x48.gif" std 0 1)))
(imf "kiwiterr-forest" ((44 48 terrain) (x 8 46 0) (file "kiwiterr44x48.gif" std 0 2)))
(imf "kiwiterr-jungle" ((44 48 terrain) (x 8 46 0) (file "kiwiterr44x48.gif" std 0 3)))
(imf "kiwiterr-grass" ((44 48 terrain) (x 8 46 0) (file "kiwiterr44x48.gif" std 0 4)))
(imf "kiwiterr-swamp" ((44 48 terrain) (x 8 46 0) (file "kiwiterr44x48.gif" std 0 5)))
(imf "kiwiterr-rubble" ((44 48 terrain) (x 8 46 0) (file "kiwiterr44x48.gif" std 0 6)))

(imf "kiwiterr-hills" ((1 1) 140 156 8))
(imf "kiwiterr-mountains" ((1 1) 43350 31365 24255))
(imf "kiwiterr-forest" ((1 1) 17595 30345 10455))
(imf "kiwiterr-jungle" ((1 1) 41 165 16))
(imf "kiwiterr-plains" ((1 1) 35700 47940 12495))
(imf "kiwiterr-swamp" ((1 1) 13107 39321 0))
(imf "kiwiterr-rubble" ((1 1) 156 132 123))
