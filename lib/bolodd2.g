(game-module "bolodd2"
  (title "AD&D Bolo")
  (version "0.1")
  (blurb "Tank battles, Dungeons-and-Dragons style. By Lincoln Peters.")
  (instructions "If you build the deadliest arsenal the fastest, you will rule this little corner of the universe!")
  (notes (
"This game is loosely based on Bolo (an old tank game for Macintosh), and rebuilt in a high-tech fantasy/adventure setting, where showing any weakness toward an enemy could mean your quick and painful death.
"
"The style is that of Dungeons and Dragons, and as such, some units have special kinds of attack mechanisms such as fire and acid, which do varying amounts of damage to different units.  The fire robot, for example, can wreak havoc on a water-based unit, whereas a frost cannon could wreak havoc on a fire-based unit, and whereas a ground unit such as a battledroid could easily wreck any piece of artillery if it gets close enough.
"
"It is important to colonize, just like in Civ, and it is important to conquer, just like in the Standard game.  However, it is foolhardy to put too much focus on either, as a horde of engineers cannot stand up against even a single helicopter, and there are not enough independent bases to effectively use all the land that you might own without building new bases.  One strategy is to focus on colonization early in the game, but to focus the efforts of all newly-built and captured bases on military units.  Depending on the specific game setup (especially the size of the map), the big show-down will likely be a great battle in which each side throws countless units of wondrous power against each other.  Of course, they must also guard their borders well, or risk that a surprise attack will capture their undefended homeland.
"
"It is impossible to acurately predict the outcome of any battle.  Of course, the odds could be heavily in favor of one side, depending on what kind of units are fighting each other (e.g. goblins vs. cacofiends), but there is almost always a chance that the underdog will prevail.  The only known exception to this rule is if one side runs out of ammo.  There are countless strategies that one could use in this game, many of which I am so far unaware of.  If anyone wants to create a strategy guide for this game, please let me know.
"
"The AI (as of January 2003) is incredibly bad at this game, and often fails to do any colonization, even if it builds a horde of engineers and no military!  For the best gaming experience, be sure to play against a human opponent.
"  
"WARNING: This game will NOT work properly if played on any version of Xconq prior to 7.5!
"
" -- Lincoln Peters (sampln@sbcglobal.net)"
  ))
  
  (design-notes (
"* This game uses a time.g-like upgrade mechanism to work around Xconq's inability to mix acp-independent units with tech and tooling.  If this could be fixed, the code (and the game) could be much simplified.
"
"* Additional complications that result from the time.g-like upgrade mechanism are that new bases do not inherit the supplies or tooling of old bases (let alone the size).  Perhaps if the change type mechanism worked, the upgrade process would be less difficult.
"
"* There is no way for Xconq to understand poison.  As a workaround, poisonous units (i.e. the spider) do quadruple damage; they should actually do 1d6 damage with each hit, plus 1d4 damage per turn thereafter until the victim is either destroyed or repaired.
"
"* Explosives such as the acid ball and firebomb should have effects on terrain that last for several turns.  For example, a firebomb that detonates in a forest should set the forest on fire, and any unit that survived the blast must either leave the forest or suffer 2d4 fire damage each turn it remains in the forest.  Firey units such as the phoenix should be immune to the forest fire damage.  Similar results should come from the acid ball, as well as anything that explodes on an ice tile.
"
"* Satellites (e.g. the meteor platform) and controllers (e.g. the meteor controller) do not interact correctly.  The satellite should automatically give everything it produces to a global treasury, and the controller should automatically take the material from the treasury when it fires at an opponent."
"* Many of the higher-level units received little testing, as they are very difficult to build when play-testing.  It is possible that some of them will behave unexpectedly.
"
"* There are not enough high-level units.  Perhaps I should find a guide to the AD&D monsters and work from there.
"

"Otherwise, I tried to build this game in such a way that new units will be easy to add.  As long as the new units are categorized correctly, there should be little difficulty in ensuring that newly-defined units work as expected."
  ))

  (variants
    ;; Many of these variants are enaled by default because the game tends to
    ;; end long before anyone can gain access to the more powerful units.
    ;; The default setup here will set the game up so that it is possible to
    ;; build Level 4+ units before the end, but it is difficult to do so.
    ;; (Perhaps the world size should be *really* big if one expects to gain
    ;; access to the valar before the end).
    (world-size (120 60 720))
    (real-time)
    (world-seen)
    (see-all)
    (sequential)
    ("Fast development" fast-tech
     "Perform technological development at 5x speed" true
     (true
       (table tech-per-develop (u* u* 5.00))))
    ("Fast toolup" fast-toolup
     "Perform toolup at 2x speed" true
     (true
       (table tp-per-toolup (u* u* 2))))
    ("Fast construction" fast-build
     "Perform construction at 2x speed" true
     (true
       (table cp-on-creation (u* u* 2))
       (table cp-per-build (u* u* 2))))
    ("Fast growth" fast-grow
     "Allow bases to grow after stockpiling 10 food instead of 25" true
     (true
       (table unit-consumption-to-grow (bases food 10))))

    ("Mostly water" mostly-water
     "Generate a map that is 70% water" true
     (true
     		; Try to adjust the percentiles so that the world is 30% land
	        
     ;                               		sea 	lak	swa 	sal 	des 	sed	ste	pla 	for 	mou 	ice
; ---------------------------------------------------------------------

(add cell-t* alt-percentile-min 	(  	0  	69 	70  	70	71  	71  	71	71	71  	93  	99	))
(add cell-t* alt-percentile-max 	( 	69  	70  	71  	71	93  	93  	93	93	93  	99 	100	))
(add cell-t* wet-percentile-min 	(  	0   	0  	50   	50	0  	5	15  	20	80   	0   	0	))
(add cell-t* wet-percentile-max 	(	100 	100 	100	100	5  	15	20 	80	100 	100 	100	))
    ))
#| This variant doesn't work correctly.
    ("Half water" half-water
     "Generate a map that is 50% water"
     (true
     		; Try to adjust the percentiles so that the world is 50% land
	        
     ;                               		sea 	lak	swa 	sal 	des 	sed	ste	pla 	for 	mou 	ice
; ---------------------------------------------------------------------

(add cell-t* alt-percentile-min 	(  	0  	49 	50  	50	51  	51  	51	51	51  	93  	99	))
(add cell-t* alt-percentile-max 	( 	49  	50  	51  	51	73  	73  	73	73	73  	99 	100	))
(add cell-t* wet-percentile-min 	(  	0   	0  	50   	50	0  	5	15  	20	80   	0   	0	))
(add cell-t* wet-percentile-max 	(	100 	100 	100	100	5  	15	20 	80	100 	100 	100	))
    ))
|#
    ("Day/Night" day-night
     "Effect a day/night cycle every 24 turns" true
     (true
       (world (day-length 24))
       (table vision-night-effect
         (u* t* 50)			;; Most units have their visibility reduced by 50% at night...
         (tower t* 100)			;; ...but not towers.
       )

       ;; Make the battlebots and undead ineffective during the day, just for variety.
       (add battlebot-1 acp-per-turn 1)
       (add battlebot-2 acp-per-turn 1)
       (add battlebot-3 acp-per-turn 1)
       (add undead acp-per-turn 1)
       (table night-multiplies-acp add
         (battlebot-1 t* 500)
         (battlebot-2 t* 400)
         (battlebot-3 t* 300)
         (undead t* 400)	;; Might need unit-specific exceptions here.
       )
    ))
    ("More bases" more-bases
     "Generate a world with twice as many bases" true
     (true
       (table independent-density (base-1 plain 400))))
    ("No surprises" no-surprises
     "Produce a world with no minefields, walls, or towers" true
     (true
       (table independent-density add ((tower wall mine) t* 0))))
    ("Paced development" paced-tech
     "Restrict the development rate of certain important units"
     (true
       ;; Restrict the development speed of bases so that only up to 4 bases can
       ;; develop a next-level base together (2 bases in the fast-tech variant).
       (add base-1 tech-per-turn-max 4)
       (add base-2 tech-per-turn-max 8)
       (add base-3 tech-per-turn-max 12)
       (add base-4 tech-per-turn-max 16)
       (add base-5 tech-per-turn-max 20)
       (add base-6 tech-per-turn-max 24)
     ))
   ("Alternate economy" model1
    "Use a different algorithm for moving materials among units."
    (true (set backdrop-model 1)))
   ("AltEcon uses doctrine" model1doctrine
    "When using alternate economy, don't drain units below doctrine levels."
    (true (set backdrop-ignore-doctrine 0))
    (false (set backdrop-ignore-doctrine 1)))
  )
)

(include "ng-swedish")
(include "ng-features")
(include "advterr")

(scorekeeper (do last-side-wins))

(define t-water (sea lake))

(define land (swamp salt-marsh desert semi-desert steppe plain forest mountain))

;                               		sea 	lak	swa 	sal 	des 	sed	ste	pla 	for 	mou 	ice
; ---------------------------------------------------------------------

(add cell-t* alt-percentile-min 	(  	0  	89 	0  	20	21  	21  	21	21	21  	90  	99	))
(add cell-t* alt-percentile-max 	( 	20  	90  	0  	23	90  	90  	90	90	89  	99 	100	))
(add cell-t* wet-percentile-min 	(  	0   	0  	20   	20	0  	10	20  	30	70   	0   	0	))
(add cell-t* wet-percentile-max 	(	100 	100 	100	100	10  	20	30 	70	100 	100 	100	))

(add river subtype connection)	;; This allows ships to travel up and down rivers.


;; Places

#| There needs to be a better way to handle bases.  With the current setup, it
   is virtually impossible to build any units of 4th level or higher before the
   game ends.
|#

(unit-type base-1 (image-name "ancient-village1")
  (hp-max 100) (acp-per-turn 1) (cp 50)
  (help "Level 1 base")
  (notes "This is the most basic kind of base, and as such it has the most basic abilities.  It is probably best to establish oneself with such bases and then upgrade as soon as possible."))
(unit-type base-2 (image-name "ancient-green-city")
  (hp-max 200) (acp-per-turn 2) (cp 50)
  (help "Level 2 base"))
(unit-type base-3 (image-name "ancient-blue-city")
  (hp-max 300) (acp-per-turn 3) (cp 50)
  (help "Level 3 base"))
(unit-type base-4 (image-name "ancient-white-castle")
  (hp-max 400) (acp-per-turn 4) (cp 50)
  (help "Level 4 base"))
(unit-type base-5 (image-name "ancient-yellow-castle")
  (hp-max 500) (acp-per-turn 5) (cp 50)
  (help "Level 5 base"))
(unit-type base-6 (image-name "ancient-black-city")
  (hp-max 600) (acp-per-turn 6) (cp 50)
  (help "Level 6 base"))
  
(unit-type tower (image-name "ancient-tower")
  (hp-max 500) (acp-per-turn 4) (acp-min -3) (cp 25)
  (help "Guards an area with deadly force")
  (notes "Towers are very useful for fighting off invasions and for building stockpiles of explosives.  They have twice the vision range and firing range of any mobile unit, they are very hard to destroy, and can hold a rather large garrison.  Towers cannot be captured; the only way to take control of an area so guarded is to destroy the tower."))
(unit-type wall (image-name "walltown")
  (hp-max 1000) (acp-per-turn 0) (cp 25)
  (description-format (side " wall at " position))
  (help "Impedes enemy movement; very hard to destroy")
  (notes "The most effective way to protect an area from invaders is to surround it with massive walls.  These walls are very expensive to build, but even the most powerful artillery would take quite a while to punch a hole in such walls.  Among the most formidable defenses one could build are a base and a tower surrounded by a ring of walls that is large enough that no enemies could fire over them and hit the base or tower, but still small enough that the tower could shoot back at anyone who might dare to attack."))

;; Construction units

(unit-type engineer (image-name "builder")
  (hp-max 10) (acp-per-turn 3) (acp-min -2) (cp 20)
  (help "Builds bases, towers, and walls")
  (notes "Engineers are the only units that can build new bases, towers, and walls.  As such, they are indispensable to any empire.  An empire that neglects its engineers is likely to find that a rival will quickly grow larger and conquer it.  On the other hand, an empire that focuses entirely on engineering and neglects its military could be brought to ruin by a single battledroid!"))
(unit-type miner (image-name "worker")
  (hp-max 5) (acp-per-turn 3) (acp-min -2) (cp 10)
  (help "Lays minefields")
  (notes "One of the more sinister strategies to defend one's territory is to lay mines.  Anyone who knows where they are can easily avoid them, but any unsuspecting invader who sets off a mine is in for a painful surprise.  Care should be taken, however, when laying minefields, as they can turn fertile land into an inhospitable wasteland."))

;; Transports

(unit-type boat (image-name "ap")
  (hp-max 10) (acp-per-turn 3) (acp-min -2) (cp 10)
  (help "Carries anything across water, but doesn't defend well")
  (notes "By far, the easiest way to travel across a body of water is by boat.  Boats are cheap, easy to build, and carry a lot.  However, they cannot defend themselves very well, and even the most formidable land army can be destroyed by a sinking boat."))
(unit-type carrier (image-name "cv")
  (hp-max 15) (acp-per-turn 4) (acp-min -3) (cp 20)
  (help "Carries all aircraft except motherships")
  (notes "Aircraft cannot take off from and land on ordinary boats.  The simplest way to bring an air force across a large body of water is to load the aircraft onto a carrier and sail to the destination.  The carrier is not quite as vulnerable as an ordinary boat, but it is still easily destroyed, especially by submarines."))
(unit-type choplifter (image-name "cargo-chopper")
  (hp-max 15) (acp-per-turn 6) (acp-min -5) (cp 15)
  (help "Carries anything up to tank-size")
  (notes "Choplifters have an advantage over boats simply because they are faster and therefore more difficult to intercept.  They can traverse both land and water with ease, but they have a shorter range than boats and cannot carry anything larger than a tank."))
(unit-type platform (image-name "halftrack")
  (hp-max 30) (acp-per-turn 3) (acp-min -2) (cp 20)
  (help "Carries artillery and supplies into battle")
  (notes "A few pieces of artillery can make short work of most opponents, but artillery alone moves slowly and has a relatively low capacity for fuel and ammunition.  The platform moves rather quickly, stores any kind of ammunition, and can carry dozens of guns.  Platforms can even be equipped to produce simple explosives during a battle!"))
(unit-type mothership (image-name "mothership")
  (hp-max 50) (acp-per-turn 8) (acp-min -7) (cp 100)
  (help "Moves fast and carries anything anywhere")
  (notes "The most effective invasions tend to be conducted using at least one mothership for moving equipment and supplies.  A mothership can carry any kind of ground units and aircraft, as well as lots of fuel and ammo.  It moves extremely quickly and is much more difficult to destroy than a boat or a carrier.  On the other hand, if a mothership is destroyed, whatever forces it was carrying either find themselves destroyed or stranded in enemy territory."))

;; Level 1 military units

(unit-type tank (image-name "tank")
  (hp-max 50) (acp-per-turn 4) (acp-min -3) (cp 50)
  (help "Your center of power")
  (notes "An ordinary tank might seem primitive compared to some of the other machines of war available in this game, but it can gain combat experience and become as deadly as anything else.  It is theoretically possible for a tank to be so battle-hardened that it could destroy a cacofiend with a single shot."))
(unit-type battledroid (image-name "mech")
  (hp-max 25) (acp-per-turn 4) (acp-min -3) (cp 10)
  (help "Simple robotic foot-soldiers")
  (notes "Ordinary battledroids are useful for any combat on the ground, but as one might imagine, they fall easily in the shadow of greater machines.  Nevertheless, a swarm of battledroids could destroy even the biggest and meanest machines that might oppose it."))
(unit-type destroyer (image-name "dd")
  (hp-max 50) (acp-per-turn 4) (cp 20)
  (help "Small, cheap combat-ready boats")
  (notes "The destroyer is the least expensive naval fighting unit.  It excels at hunting down and destroying submarines, although it is not nearly as sneaky."))
(unit-type minesweeper (image-name "hovercraft")
  (hp-max 10) (acp-per-turn 4) (acp-min -3) (cp 15)
  (help "Safely detonates enemy minefields, but otherwise vulnerable")
  (notes "The minesweeper is the only unit that can step on a land mine and suffer no damage from the explosion.  It is a good idea to send at least one minesweeper into an area if you suspect it to be mined.  However, a minesweeper is quite vulnerable to everything other than mines, so they should always be used with discretion.  One strategy would be to use spy planes or helicopters to watch for enemies while the minesweepers do their thing."))

;; Level 2 military units

(unit-type spy-plane (image-name "delta")
  (hp-max 10) (acp-per-turn 6) (acp-min -5) (cp 20)
  (help "Useful for exploration and spying")
  (notes "This long-range aircraft is fast and difficult to spot.  It is very useful for mapping out enemy territory when planning an invasion, although it can also be used to paradrop battlebots (goblins, orcs, ogres).  A few battlebots can wreak havoc (depending on how well-defended an area is) and distract the enemy while you gather your real invasion force on their border."))
(unit-type helicopter (image-name "helicopter")
  (hp-max 25) (acp-per-turn 5) (acp-min -4) (cp 25)
  (help "Deadly against ground units")
  (notes "A helicopter has as much firepower as a tank, but it moves quickly and is hard to hit.  Helicopters are very useful for surprise invasions, but they are far more vulnerable to fighters than ordinary tanks."))
(unit-type bomber (image-name "4e")
  (hp-max 15) (acp-per-turn 6) (acp-min -5) (cp 15)
  (help "Carries explosives into the heart of the enemy")
  (notes "Bombers are long-range aircraft that are not built for combat, but are well suited to carry all kinds of explosives.  They can quickly fly up close to a fortification, release up to a dozen missiles, and fly away."))
(unit-type fighter (image-name "fighter")
  (hp-max 10) (acp-per-turn 8) (acp-min -7) (cp 15)
  (help "The best defense against air attacks")
  (notes "Fighters are designed to excel at destroying all kinds of aircraft.  They are fast and accurate, and can make short work of any bomber or helicopter.  A small squadron of fighters could even destroy a mothership!"))
(unit-type battleship (image-name "bb")
  (hp-max 100) (acp-per-turn 4) (acp-min -3) (cp 50)
  (help "The most powerful naval unit")
  (notes "The battleship is a very powerful naval unit, being about twice as powerful as a tank.  It may not have much to fear from naval units, but there are all sorts of higher-tech units that could spell its doom."))
(unit-type submarine (image-name "sub")
  (hp-max 20) (acp-per-turn 3) (acp-min -2) (cp 20)
  (help "Sneaky underwater craft that blows up transport boats")
  (notes "Submarines are quiet underwater craft that excel at hunting and destroying all kinds of boats.  A submarine has the potential to destroy an ordinary boat with a single shot, and an aircraft carrier with two shots!"))
(unit-type battlebot-1 (image-name "monsters-ninja") (name "goblin")
  (hp-max 10) (acp-per-turn 5) (acp-min -4) (cp 10)
  (help "Small but fast battledroids")
  (notes "These robots are smaller than ordinary battledroids, but they move and attack more quickly than any other kind of battledroid.  They can also be paradropped from spy planes, and so can be very effective at launching surprise attacks."))
(unit-type battlebot-2 (image-name "monsters-orc3") (name "orc")
  (hp-max 25) (acp-per-turn 4) (acp-min -3) (cp 10)
  (help "Larger but slower battledroids")
  (notes "These robots are roughly equivalent to ordinary battledroids, but they can be paradropped from spy planes, making them very effective at launching surprise attacks."))
(unit-type battlebot-3 (image-name "monsters-ax-troll") (name "ogre")
  (hp-max 50) (acp-per-turn 3) (acp-min -2) (cp 10)
  (help "Huge, lumbering battledroids")
  (notes "These robots are much larger than ordinary battledroids, but they move and attack more slowly as a result.  However, they can be paradropped from spy planes, and so can be very effective at launching surprise attacks."))

#| Levels 3 and 4 needs more units.  Consider:
Troll
Wyvern
Fire Salamander
Ice Salamander
|#

;; Level 3 military units

(unit-type werewolf (image-name "monsters-gorilla")
  (hp-max 75) (acp-per-turn 5) (cp 40)
  (help "Very fast and very deadly")
  (notes "A werewolf is a battledroid that merges the abilities of a large humanoid battledroid with a canine.  It moves as almost as fast as a helicopter and it is tougher than most armored tanks!"))
(unit-type spider (image-name "monsters-alien-spider")
  (hp-max 40) (acp-per-turn 4) (cp 30)
  (help "Walking rust-inducing machines")
  (notes "Spiders are insect-like robots that have razor-sharp legs and teeth.  They also can pierce armor and inject an oxidizing (rusting) agent into an opponent, effectively poisoning it."))
(unit-type doppleganger (image-name "monsters-blue-man")
  (hp-max 25) (acp-per-turn 4) (cp 25)
  (help "Illusory battledroids")
  (notes "A doppleganger's most noteworthy ability is that it can trick enemies into thinking that it is something significantly less dangerous.  Once the enemy figures out what it's opponent REALLY is, however, the doppleganger has usually had enough time to rip the enemy to shreds."))
(unit-type ankheg (image-name "beetle")	;; Need a better image
  (hp-max 50) (acp-per-turn 3) (cp 50)
  (help "Burrowing insectoid robot")
  (notes "The ankheg is a veritable land submarine.  It is tremendously strong, and can burrow through earth like a submarine can float through water.  It is also nearly impossible to spot until its right in front of its opponent.  It has one drawback, however, in that it is only effective in open land, and is almost useless in forests and mountains."))

;; Level 4 military units

(unit-type centipede (image-name "monsters-giant-spider")
  (hp-max 50) (acp-per-turn 3) (acp-min -2) (cp 40)
  (help "Large fighting machine that feeds on enemy metal")
  (notes "Centipedes are scavengers that can strip metal from any wreck and bring it to the nearest supply point for re-use.  It is also battle-ready so that it can create its own wrecks, rather than rely on its allies to produce wrecks for it.  Furthermore, it can be tooled to build undead units on the spot, using scrap metal collected from any wrecks it might find!"))
(unit-type air (image-name "wiz-air")
  (hp-max 100) (acp-per-turn 4) (acp-min -3) (cp 100)
  (help "Robotic construct made of air")
  (notes "This massive battledroid is actually a swirling mass of gases, held together by magnets and force-fields.  It is very hard to destroy, and it can fire lightning bolts at its enemies."))
(unit-type water (image-name "monsters-octopus")
  (hp-max 100) (acp-per-turn 4) (acp-min -3) (cp 100)
  (help "Robotic construct made of water")
  (notes "This massive battledroid is actually a mass of liquid (mostly water), held together by force-fields and surface tension.  It is very hard to destroy, and it can fire a lightning bolts at its enemies."))
(unit-type earth (image-name "monsters-club-troll3")
  (hp-max 200) (acp-per-turn 3) (acp-min -2) (cp 100)
  (help "Robotic construct made of earth")
  (notes "This massive battledroid is actually a mass of rock, held together by force-fields and muscle wires.  It is very hard to destroy, and it can do massive damage to enemies by punching them or by stepping on them."))
(unit-type fire (image-name "monsters-balrog2")
  (hp-max 150) (acp-per-turn 4) (acp-min -3) (cp 100)
  (help "Robotic construct made of fire")
  (notes "This massive battledroid is actually a mass of burning gases, held together by force-fields.  It is very hard to destroy, and it can spew an arc of searing flame at its enemies."))
(unit-type cacofiend (image-name "monsters-vampire")
  (hp-max 400) (acp-per-turn 3) (acp-min -2) (cp 200)
  (help "A battledroid the size of the Empire State Building")
  (notes "The sight of a cacofiend is sure to strike fear into one's enemies, unless the enemies have their own cacofiends (or worse!).  This battledroid is as tall as a tower, has wings that can carry it over water, and it has fists that do as much damage as falling 16-ton boulders.  It can even carry a few rockets!"))

#| Level 5 needs some more powerful elemental units, otherworldly powers, and such.  Perhaps:
Beholders
Dragons
Genies
|#

;; Level 5 military units

(unit-type phoenix (image-name "monsters-pterodactyl")
  (hp-max 100) (acp-per-turn 6) (acp-min -5) (cp 75)
  (help "Firey aircraft; resistant to all except cold-based weapons")
  (notes "The phoenix is as more to ground units than a helicopter, more deadly to other aircraft than a fighter, and nearly immune to every kind of weapon except cold-based artillery and explosives.  The frost cannon is among the few weapons that it fears."))
(unit-type stormcloud (image-name "city30")
  (hp-max 150) (acp-per-turn 4) (acp-min -3) (cp 100)
  (help "Floating fortress that fires lightning bolts")
  (notes "Any military force that lacks fighters or other anti-aircraft equipment will be in big trouble if attacked by a stormcloud.  It floats in the air at extremely high altitude, making it hard to see.  It also fires lightning bolts at everything below, making it a veritable flying submarine."))

;; Level 6 military units

(unit-type valar (image-name "monsters-red-sorcerer")
  (hp-max 800) (acp-per-turn 8) (acp-min -7) (cp 400)
  (help "A machine that can create (and destroy) virtually anything")
  (notes "The valar is a machine that can alter any terrain, build allies for itself on-the-fly, and is in itself a juggernaut of destruction.  Only another valar or a huge army with numbers beyond reckoning could challenge a valar and hope to survive."))
(unit-type dark-valar (image-name "monsters-nazgul")
  (hp-max 800) (acp-per-turn 8) (acp-min -7) (cp 400)
  (help "A machine built to destroy anything that opposes it")
  (notes "A dark valar foregoes all of an ordinary valar's constructive abilities in order to become the ultimate destructive force.  It is even more destructive on its own than an ordinary valar, but it is unable to make terrain alterations or build allies for itself.  Few things other than another valar could face such a monster and survive."))
  ;; (the dark valar isn't destructive enough!)

#| There should be more undead units, just to keep it interesting.  Perhaps:
Ghoul
Skeleton
Shadow
Vampire
Lich
Banshee
|#

;; "Undead" military units

(unit-type zombie (image-name "monsters-undead")
  (hp-max 25) (acp-per-turn 4) (cp 3)
  (help "Battledroid built on-the-fly from scrap metals")
  (notes "One of the trickiest but potentially most deadly tactics available is to have a centipede collecting scrap metal from dead units and build an undead unit such as the zombie while the battle is still going!  It is difficult, however, as it is very important to protect the centipede so that it can do its job.  As with any unit, the zombie can be destroyed with a single blow if it is hit while under construction."))

;; Artillery

(unit-type howitzer (image-name "pz-how")
  (hp-max 5) (acp-per-turn 1) (cp 20)
  (help "Fires huge shells at enemies")
  (notes "A typical howitzer fires large shells that are twice as powerful as those fired by tanks or helicopters."))
(unit-type acid-gun (image-name "pz-at")
  (hp-max 5) (acp-per-turn 1) (cp 20)
  (help "Sprays acid at enemies")
  (notes "This piece of artillery sprays its target with a powerful acid.  This is devastating to everything except air and water robots."))
(unit-type flame-gun (image-name "pz-at")
  (hp-max 5) (acp-per-turn 1) (cp 20)
  (help "Shoots fireballs at enemies")
  (notes "This piece of artillery shoots a searing fireball at its target.  This is especially devastating to water robots."))
(unit-type lightning-gun (image-name "pz-at")
  (hp-max 5) (acp-per-turn 1) (cp 20)
  (help "Fires bolts of lightning at enemies")
  (notes "This piece of artillery fires lightning bolts at its target.  This is devastating to everything except air and fire robots."))
(unit-type frost-gun (image-name "pz-at")
  (hp-max 5) (acp-per-turn 1) (cp 20)
  (help "Fires extremely cold liquid at enemies")
  (notes "This piece of artillery fires a column of numbing frost at its target.  This is especially devastating to fire robots."))
(unit-type pulse-cannon (image-name "pz-at")
  (hp-max 5) (acp-per-turn 1) (cp 20)
  (help "Repels enemies using a shockwave")
  (notes "This piece of artillery fires shockwaves at its target.  The shockwave does no damage (usually), but it throws the target away from the cannon, even into hostile terrain.  This could be used, for example, to throw enemy battledroids into a body of water, thereby instantly destroying it."))
(unit-type anti-aircraft (image-name "pz-at")
  (hp-max 5) (acp-per-turn 1) (cp 20)
  (help "Built to kill aircraft")
  (notes "An alternative to using fighters to protect one's airspace is to equip one's bases and towers with anti-aircraft artillery.  It is designed to excel at shooting down anything that flies, though it can also be effective against ground units."))

;; (the controller/satellite system doesn't seem to work properly; and I don't
;; know if it ever will.)

;; Controllers

(unit-type lightning-controller (image-name "pz-flak")
  (hp-max 10) (acp-per-turn 4) (cp 40)
  (help "Directs lightning to strike enemies")
  (notes "If the appropriate satellites are put in place, this device can cause an enemy unit to be struck by lightning."))
(unit-type meteor-controller (image-name "pz-flak")
  (hp-max 10) (acp-per-turn 4) (cp 40)
  (help "Directs meteors to pound enemies into dust")
  (notes "If the appropriate satellites are put in place, this device can cause meteorites to come crashing down on an enemy.  This may very well be the ULTIMATE weapon!"))
(unit-type hailstorm-controller (image-name "pz-flak")
  (hp-max 10) (acp-per-turn 4) (cp 40)
  (help "Directs large hailstones to bash enemies")
  (notes "If the appropriate satellites are put in place, this device can create a hailstorm in the vicinity of an enemy unit."))

;; Generic explosives

(unit-type mine (image-name "minefield")
  (hp-max 1) (acp-per-turn 0) (cp 3)
  (help "Explodes when an enemy steps on it")
  (notes "One of the more sinister strategies to defend one's territory is to lay mines.  Anyone who knows where they are can easily avoid them, but any unsuspecting invader who sets off a mine is in for a painful surprise.  Care should be taken, however, when laying minefields, as they can turn fertile land into an inhospitable wasteland."))
(unit-type rocket (image-name "rocket")
  (hp-max 1) (acp-per-turn 6) (cp 5)
  (help "Cruise missile, does heaviest damage to point of impact")
  (notes "A rocket moves fast, has a fairly long range, and can destroy most mobile units with a single blow.  Larger units, bases, and towers can be destroyed with multiple rockets.  Some mobile units, such as tanks and platforms, can even be equipped to build rockets during combat!"))
(unit-type orb (image-name "wiz-manicon")
  (hp-max 1) (acp-per-turn 4) (cp 10)
  (help "Slower than a rocket, but twice as deadly")
  (notes "An orb is slower than a rocker and has shorter range, but it does much more damage.  One orb can instantly vaporize any mobile unit (except a cacofiend or a Level 5+ unit), and five orbs are enough to level most bases."))

;; Special explosives

(unit-type acid-ball (image-name "fireball")
  (hp-max 1) (acp-per-turn 6) (cp 8)
  (help "Explodes into a ball of deadly acid")
  (notes "This is an explosive, just like a rocket or orb, but its strength is that its explosion releases a huge cloud of acid that will eat away at anything it strikes.  A single acid ball can also transform a small island into a uninhabitable wasteland."))
(unit-type firebomb (image-name "fireball")
  (hp-max 1) (acp-per-turn 6) (cp 8)
  (help "Incinerates virtually anything")
  (notes "When a firebomb explodes, it releases an incredibly intense heat, capable of melting almost any metal and incinerating everything else.  It also has the destructive power to ruin land, but to a lesser degree than the acid ball."))

;; (still need electrical and freezing explosives)

(unit-type magma-drill (image-name "mininuke")
  (hp-max 1) (acp-per-turn 4) (cp 16)
  (help "Burrows into the core of the planet, releases magma onto the battlefield")
  (notes "This weapon is at least as deadly as a meteor shower.  When it strikes the ground, it burrows into the Earth's core, creating an artificial volcano that can wipe out any military forces and any ecosystem.  It is even powerful enough to tranform the tallest mountains into rubble!"))

;; Basic Facilities

(unit-type granary (image-name "granary") (cp 15)
  (help "Improves food production"))
(unit-type refinery (image-name "oil-derrick") (cp 15)
  (help "Improves fuel production"))
(unit-type armory (image-name "fort") (cp 15)
  (help "Improves ammo production"))

;; Advanced Facilities

(unit-type acid-factory (image-name "refinery") (cp 25)
  (help "Chemical plant that produces refined acid"))
(unit-type oil-plant (image-name "refinery") (cp 25)
  (help "Chemical plant that produces explosive liquids"))
(unit-type solar-complex (image-name "facility") (cp 25)
  (help "Generates electricity"))
(unit-type ice-factory (image-name "facility") (cp 25)
  (help "Chemical plant that produces extremely cold liquids"))

;; Satellites

(unit-type lightning-platform (image-name "spysat") (cp 50)
  (help "Orbital satellite that creates thunderstorms"))
(unit-type ice-platform (image-name "spysat") (cp 50)
  (help "Orbital satellite that creates hailstorms"))
(unit-type meteor-platform (image-name "spysat") (cp 75)
  (help "Orbital satellite that collects and hurls meteors"))

;; Wrecks

(unit-type ruins (image-name "ancient-ruins")
  (hp-max 100) (acp-per-turn 0) (cp 1) (possible-sides "independent")
  (description-format ("ruins of " name " at " position))
  (help "Remains of a destroyed base or tower")
  (notes "This is what remains after a base or tower has been subjected to a powerful enough attack that it has been completely leveled.  Ruins are  worthless to all but the centipedes; unless they are to be 'recycled,' they must be cleared or allowed to decay before the land can be re-used."))
(unit-type small-wreck (image-name "tank-wrecked")	;; Need a better image
  (hp-max 10) (acp-per-turn 0) (cp 1) (possible-sides "independent")
  (description-format ("the small wreck at " position))
  (help "Remains of a destroyed small unit"))
(unit-type medium-wreck (image-name "tank-wrecked")
  (hp-max 20) (acp-per-turn 0) (cp 1) (possible-sides "independent")
  (description-format ("the medium wreck at " position))
  (help "Remains of a destroyed mid-sized unit"))
(unit-type large-wreck (image-name "tank-wrecked")
  (hp-max 30) (acp-per-turn 0) (cp 1) (possible-sides "independent")
  (description-format ("the large wreck at " position))
  (help "Remains of a destroyed large unit"))

(add (small-wreck medium-wreck large-wreck) notes "A wreck may be harvested for scrap metal by a centipede, but otherwise it is nothing more than a stumbling block that lingers after a battle.")

;;; MATERIALS

;;; Basic materials

(material-type fuel (image-name "yellow") (resource-icon 2)
  (help "Powers everything"))
(material-type ammo (image-name "black") (resource-icon 3)
  (help "Used by most ranged weapons"))
(material-type metal (image-name "blue") (resource-icon 4)
  (help "Used to build everything"))

;; Advanced materials

(material-type acid
  (help "Dissolves anything except air and water"))
(material-type flame
  (help "Burns anything except metal"))
(material-type electricity
  (help "Burns electronics"))
(material-type frost
  (help "Extinguishes fire, freezes water"))
(material-type meteors
  (help "Huge boulders that smash anything!"))

(material-type scrap
  (help "Used to create undead units"))

;; Backdrop materials

(material-type food (image-name "green") (resource-icon 1)
  (help "Used to feed and grow bases and towers"))

(define places (base-1 base-2 base-3 base-4 base-5 base-6 tower wall))
(define constructors (engineer miner))
(define transports (boat carrier choplifter platform mothership))
(define military-1 (tank battledroid destroyer minesweeper))
(define military-2 (spy-plane helicopter bomber fighter battleship submarine battlebot-1 battlebot-2 battlebot-3))
(define military-3 (werewolf spider doppleganger ankheg))
(define military-4 (centipede air water earth fire cacofiend))
(define military-5 (phoenix stormcloud))
(define military-6 (valar dark-valar))
(define undead (zombie))
(define artillery (howitzer acid-gun flame-gun lightning-gun frost-gun pulse-cannon anti-aircraft))
(define controllers (lightning-controller meteor-controller hailstorm-controller))
(define explosives-1 (mine rocket orb))
(define explosives-2 (acid-ball firebomb magma-drill))
(define facilities (granary refinery armory acid-factory oil-plant solar-complex ice-factory))
(define facilities-1 (granary refinery armory))
(define facilities-2 (acid-factory oil-plant solar-complex ice-factory))
(define satellites (lightning-platform ice-platform meteor-platform))
(define wrecks (ruins small-wreck medium-wreck large-wreck))

(define bases (base-1 base-2 base-3 base-4 base-5 base-6))
(define naval (boat carrier destroyer battleship submarine))
(define aircraft (choplifter mothership spy-plane helicopter bomber fighter phoenix stormcloud))
(define walkers (engineer miner battledroid battlebot-1 battlebot-2 battlebot-3 werewolf spider doppleganger ankheg centipede air water earth fire cacofiend zombie))
;; (should there be a special category of "diggers" for underground units like the ankheg?)
(define wheeled (platform tank minesweeper))

(define movers (engineer miner boat carrier choplifter platform mothership tank battledroid destroyer minesweeper spy-plane helicopter bomber fighter battleship submarine battlebot-1 battlebot-2 battlebot-3 werewolf spider doppleganger centipede air water earth fire cacofiend phoenix stormcloud valar dark-valar zombie howitzer acid-gun flame-gun lightning-gun frost-gun pulse-cannon anti-aircraft lightning-controller meteor-controller hailstorm-controller rocket orb acid-ball firebomb magma-drill))

;; Units that use special weapons special combat methods fall under the following definitions...
(define acidic (ankheg acid-gun acid-ball))
(define burning (flame-gun meteor-controller firebomb fire phoenix))
(define electrical (lightning-gun lightning-platform air water stormcloud))
(define freezing (frost-gun hailstorm-controller))

;; Units that are based on certain special materials fall under the following definitions...
;; (There are no acid-based units yet)
(define gaseous (air stormcloud))
(define wet (water))
(define firey (fire phoenix))
(define nonmetallic (ankheg earth))

;; Some elemental units have innate protections against certain weaponry.
;; (needs to be further developed)
(define fireshielded (phoenix))

(define materials-1 (food fuel ammo metal))
(define materials-2 (acid flame electricity frost meteors))

(add bases notes "The base produces fuel and ammo, and it builds every kind of mobile unit.  Although some other units have limited construction capabilities, no empire of any size is complete without at least one base.")
;; (should have some way to tack on specific notes for each level of base)

;; Most places are advanced but NOT acp-independent.
;; (They would be acp-independent if it would not interfere with tech and tooling.)
(add bases advanced true)
(add bases reach 2)

(add bases wrecked-type ruins)
(add tower wrecked-type ruins)

;; In general, higher level units are larger, and so result in larger wrecks.
(add constructors wrecked-type small-wreck)
(add military-1 wrecked-type small-wreck)
(add military-2 wrecked-type medium-wreck)
(add military-3 wrecked-type medium-wreck)
(add military-4 wrecked-type large-wreck)
(add military-5 wrecked-type large-wreck)
(add artillery wrecked-type small-wreck)

;; There are a few exceptions, however...
(add battlebot-1 wrecked-type small-wreck)
(add battlebot-3 wrecked-type large-wreck)
(add tank wrecked-type medium-wreck)

(add places speed 0)
(add facilities speed 0)
(add satellites speed 0)

(add movers free-mp 1)

;;; INITIALIZATION

(add tank start-with 1)
(add base-1 start-with 1)
(add engineer start-with 1)
; (add boat start-with 1)

(table favored-terrain
  (u* t* 0)		;; Don't allow anyone to start in a place not explicitly allowed.
  (bases plain 100)	;; Bases function best in the plains.
  (tower mountain 100)	;; Towers are more effective on the high-ground.
  (boat t-water 100)	;; Boats must be in the water.
; (tank plain 100)	;; Tanks start in the open.
  (engineer plain 100)	;; As do engineers.
)

(set country-radius-min 3)			;; Might need adjustment
(set country-separation-min 50)			;; Might also need adjustment
(set country-separation-max 9999)		;; This shouldn't be necessary, but country-separation-min doesn't seem to work without it.

(add plain country-terrain-min 3)
(add mountain country-terrain-min 1)
(add lake country-terrain-min 1)
(add sea country-terrain-min 2)


(table independent-density
  (base-1 plain 100)
  ;; (tower mountain 50)

  ;; Just to make it interesting...
  (mine land 10)
  (wall land 10)
)


;;; MOTION

(table mp-to-enter-terrain
  ;; By default, they can move on land but not water...
  (u* land 1)
  (u* t-water 99)
  ;; But the opposite is the case for naval units...
  (naval land 99)
  (naval t-water 1)
  (naval river 0)
  ;; Aircraft and missiles can go anywhere...
  (aircraft t* 1)
  (explosives-1 t* 1)
  (explosives-2 t* 1)
  (aircraft river 0)
  (explosives-1 river 0)
  (explosives-2 river 0)

  ;; The speed of a unit often depends on the terrain it's moving on...
  (wheeled plain 1)
  (wheeled steppe 1)
  (wheeled semi-desert 2)
  (wheeled desert 2)
  (wheeled forest 3)
  (wheeled mountain 4)
  (wheeled swamp 4)
  (wheeled salt-marsh 4)

  (walkers plain 1)
  (walkers steppe 1)
  (walkers semi-desert 1)
  (walkers desert 2)
  (walkers forest 2)
  (walkers mountain 2)
  (walkers swamp 2)
  (walkers salt-marsh 2)
  
  (ankheg mountain 99)

  ;; Very few units can handle ice.
  (u* ice 99)
  (engineer ice 2)
  (aircraft ice 1)

  ;; Air and water robots can go anywhere.
  (air land 1)
  (air t-water 2)

  (water land 2)
  (water t-water 1)

  (cacofiend land 1)
  (cacofiend t-water 3)

  (fire river 2)		;; Fire robots must be especially careful when crossing rivers!
  (cacofiend river 0)		;; The cacofiend can just step over a river!

  ;; The valar can go anywhere unobstructed.
  (valar t* 1)
  (dark-valar t* 1)
  
  ;; Controllers can fire rapidly, but cannot move so quickly.
  (controllers t* 3)

  ;; Roads make anything move around more easily.
  (u* road 0)
)

(table mp-to-leave-terrain
  ;; To prevent accidents...
  (walkers t-water 99)
  (wheeled t-water 99)
  (naval land 99)

  (u* river 2)
  (naval river 0)

  (air t* 0)

  (water t* 0)

  (fire river 3)		;; Fire robots must be especially careful when crossing rivers!
  (cacofiend t* 0)		;; Cacofiends have wings.

  (valar t* 0)			;; The valar can go anywhere unobstructed.
)

#| This is meant so that if a pulse cannon or other such force lands in
   hostile terrain, it is instantly destroyed (tanks on a wrecked bridge, for
   example).  However, there seem to be some bugs in the movement code that
   demand that this behavior be temporarily disabled.
(table vanishes-on
  ;; In the event that a unit enters hostile terrain, it DIES!
  (u* t-water true)
  (aircraft t* false)
  (explosives-1 t* false)
  (explosives-2 t* false)
  (naval t-water false)
  (naval land true)
  (air t* false)		;; They fly when they need to!
  (water t* false)		;; They prefer to swim, but can walk!
  (cacofiend t* false)		;; They have wings, although they can't fly as fast as they walk.
  (valar t* false)
)
|#

(table mp-to-traverse
  (u* road 0)
  (naval road 99)
  (naval river 0)
)

;;; CAPACITANCE

(add t* capacity 16)

(table unit-size-in-terrain
  (u* t* 1)
  (aircraft t* 0)
  (places t* 16)
  (mine t* 99)		;; Mines are treated differently.
)

(table terrain-capacity-x
  ;; Mines use exclusive terrain capacity so that:
  ;; 1. One and only one mine may occupy a cell.
  ;; 2. The mine does not interfere with the movement of friendly units.
  (mine t* 1)

)

(add bases capacity 32)
(add tower capacity 16)
(add wall capacity 2)
(add ruins capacity 16)
(add boat capacity 8)
(add carrier capacity 12)
(add choplifter capacity 4)
(add platform capacity 16)
(add mothership capacity 32)

(add tank capacity 8)
(add battledroid capacity 4)
(add destroyer capacity 4)
(add spy-plane capacity 8)
(add helicopter capacity 8)
(add bomber capacity 12)
(add battleship capacity 16)
(add submarine capacity 16)
(add centipede capacity 8)

(add valar capacity 32)

(table unit-size-as-occupant
  (u* u* 99)
  (u* places 1)
  (facilities places 0)
  (satellites places 0)
  (places places 99)

  (constructors transports 1)
  (military-1 transports 1)
  (military-2 transports 1)
  (military-3 transports 1)
  (military-4 transports 2)
  (military-5 transports 2)
  (military-6 transports 4)	;; Is this appropriate?
  (artillery transports 1)

  (aircraft choplifter 99)

  (transports transports 99)	;; Generally, transports cannot carry other transports.
  (platform transports 2)	;; But anything with space can carry a platform.
  (platform platform 99)	;; Except other platforms.
  (aircraft transports 99)	;; Few things can transport aircraft.
  (aircraft mothership 1)	;; Among the few are motherships.
  (mothership mothership 99)	;; But motherships cannot carry other motherships.
  (naval transports 99)		;; Fewer things can transport ships.
  (naval mothership 2)		;; Only motherships can carry ships.
  (u* carrier 99)		;; By default, nothing is allowed on a carrier.
  (aircraft carrier 1)		;; Carriers can carry any kind of aircraft.
  (mothership carrier 99)	;; Except motherships.
  ;; (choplifter boat 2)	;; Choplifters can easily land on any boat.
  ;; (fighter boat 99)		;; But fighters cannot.
  ;; (bomber boat 99)		;; Nor bombers.
  (choplifter mothership 1)	;; Choplifters fit in a mothership easily.
  (mothership places 4)		;; Motherships are really big!

  (explosives-1 military-1 1)
  (explosives-1 military-2 1)
  (explosives-1 military-3 1)
  (explosives-1 military-4 1)

  (u* platform 99)
  (wheeled platform 2)
  (walkers platform 1)
  (explosives-1 platform 1)
  (explosives-2 platform 1)
  (artillery platform 1)
  (controllers platform 1)
  (choplifter platform 2)
  (helicopter platform 2)

  (u* spy-plane 99)
  (battlebot-1 spy-plane 1)
  (battlebot-2 spy-plane 2)
  (battlebot-3 spy-plane 4)

  (u* valar 1)			;; A valar can carry anything...
  (valar valar 99)		;; ...except other valars...
  (dark-valar valar 99)		;; ...and dark valars.

  (undead centipede 1)		;; Only centipedes can carry undead robots.

  (engineer places 1)		;; Any place can hold an engineer.
  (engineer transports 1)	;; As can any transport.

  (mine u* 99)			;; Nobody in their right mind would carry active mines!
)

(table unit-capacity-x
  (acid-gun acid-ball 4)
  (flame-gun firebomb 4)
  (tank magma-drill 1)
  (platform magma-drill 1)
  (cacofiend rocket 8)		;; Is this appropriate?

  ;; Bases always have capacity for one base of the next level up (so that they can build it).
  (base-1 base-2 1)
  (base-2 base-3 1)
  (base-3 base-4 1)
  (base-4 base-5 1)
  (base-5 base-6 1)
)

(table constructor-absorbed-by
  (bases bases true)
)

(table hp-to-garrison
  (bases bases 9999)	;; When a base has upgraded, the old base vanishes and is replaced by the new base.
)

;;; MATERIAL CAPACITANCE

(table unit-storage-x
  (places m* 9999)	;; Bases and towers can store anything...
  (places scrap 0)	;; ...except scrap metal...
  (places meteors 0)	;; ...and meteors.

  (wall m* 0)

  (engineer fuel 200)
  (engineer metal 100)
  (miner fuel 100)

  (tank fuel 100)
  (tank ammo 50)
  (tank metal 25)

  (battledroid fuel 50)
  (battledroid ammo 25)

  (destroyer fuel 200)
  (destroyer ammo 50)

  (minesweeper fuel 100)
  (minesweeper ammo 25)

  (boat fuel 200)
  (boat ammo 100)
  (boat metal 100)

  (carrier fuel 800)
  (carrier ammo 200)

  (choplifter fuel 48)
  (choplifter ammo 25)

  (platform fuel 200)
  (platform ammo 200)
  (platform metal 100)
  (platform acid 50)
  (platform flame 50)
  (platform electricity 50)
  (platform frost 50)

  (mothership fuel 400)
  (mothership ammo 400)

  (spy-plane fuel 96)
  (spy-plane ammo 12)

  (helicopter fuel 50)
  (helicopter ammo 25)

  (bomber fuel 36)
  (bomber ammo 10)

  (fighter fuel 48)
  (fighter ammo 15)

  (battleship fuel 400)
  (battleship ammo 200)

  (submarine fuel 100)
  (submarine ammo 25)

  (battlebot-1 fuel 50)
  (battlebot-1 ammo 10)

  (battlebot-2 fuel 100)
  (battlebot-2 ammo 20)

  (battlebot-3 fuel 200)
  (battlebot-3 ammo 40)

  (werewolf fuel 150)
  (werewolf ammo 30)

  (spider fuel 100)
  (spider ammo 25)

  (doppleganger fuel 50)
  (doppleganger ammo 15)
  
  (ankheg fuel 100)
  (ankheg acid 50)

  (centipede fuel 100)
  (centipede ammo 40)
  (centipede scrap 200)

  (air fuel 400)
  (air electricity 100)		;; Shoots lightning bolts

  (water fuel 400)
  (water electricity 100)	;; Shoots lightning bolts

  (earth fuel 400)
  (earth ammo 100)

  (fire fuel 400)
  (fire flame 100)		;; Shoots balls of fire

  (cacofiend fuel 800)
  (cacofiend ammo 200)

  (phoenix fuel 96)
  (phoenix flame 75)

  (stormcloud fuel 400)
  (stormcloud electricity 150)

  (valar fuel 2000)
  (valar ammo 1000)
  (valar metal 500)

  (dark-valar fuel 2000)
  (dark-valar ammo 2000)

  (howitzer fuel 80)
  (howitzer ammo 40)

  (acid-gun fuel 40)
  (acid-gun acid 40)

  (flame-gun fuel 40)
  (flame-gun flame 40)

  (lightning-gun fuel 40)
  (lightning-gun electricity 40)

  (frost-gun fuel 40)
  (frost-gun frost 40)

  (pulse-cannon fuel 40)
  (pulse-cannon ammo 40)	;; Should the ammo be different?

  (anti-aircraft fuel 80)
  (anti-aircraft ammo 40)	;; Should the ammo be different (perhaps scud missiles)?

  (lightning-controller fuel 40)
  (lightning-controller electricity 40)

  (hailstorm-controller fuel 40)
  (hailstorm-controller frost 40)

  (meteor-controller fuel 40)
  (meteor-controller meteors 40)

  (rocket fuel 24)
  (orb fuel 16)
  (acid-ball fuel 24)
  (firebomb fuel 24)
  (magma-drill fuel 16)

  (lightning-platform electricity 100)
  (meteor-platform meteors 100)
  (ice-platform frost 100)

  (ruins scrap 100)
  (small-wreck scrap 10)
  (medium-wreck scrap 20)
  (large-wreck scrap 30)
)

(table production-from-terrain
  (plain food 3)
  (plain fuel 3)
  (plain metal 1)

  (steppe food 2)
  (steppe fuel 4)
  (steppe metal 1)

  (forest food 2)
  (forest fuel 4)
  (forest metal 2)

  (semi-desert food 1)
  (semi-desert fuel 9)
  (semi-desert metal 1)

  (desert fuel 12)
  (desert metal 1)

  (swamp food 1)
  (swamp fuel 3)

  (salt-marsh food 1)
  (salt-marsh fuel 3)

  (mountain fuel 6)
  (mountain metal 4)

  (lake food 3)
  (lake fuel 3)

  (sea food 2)
  (sea fuel 6)

  (river food 1)

  (t* ammo 5)	;; Just to ensure that large bases produce more ammo than their smaller counterparts.
)

(table base-production
  (acid-factory acid 3)
  (oil-plant flame 3)
  (solar-complex electricity 3)
  (ice-factory frost 3)

  (lightning-platform electricity 10)
  (ice-platform frost 10)
  (meteor-platform meteors 1)

  ;; Towers are equipped to be self-succicient if they need to be.
  (tower fuel 6)		;; (used to be 4, but didn't work well with "Fast construction" variant)
  (tower metal 2)
  (tower ammo 2)

  ;; The valar are pretty much self-sufficient.
  (valar fuel 4)
  (valar ammo 4)
  (valar metal 4)

  (dark-valar fuel 4)
  (dark-valar ammo 8)
)

(table occupant-multiply-production
  (granary food 200)
  (refinery fuel 200)
  (armory ammo 200)
)

;; Supplies may travel from orbital platforms to controllers.
;; Controllers, however, cannot give resources to any other units.

#| This is (hopefully) being replaced by a more effective system...
(table out-length
  (satellites materials-2 9999)
  (controllers materials-2 -1)
)

(table in-length
  (controllers materials-2 9999)
  (satellites materials-2 -1)
)

|#

(add electricity treasury true)
(add frost treasury true)
(add meteors treasury true)

(table gives-to-treasury
  (u* m* false)					;; Only satellites can contribute to treasuries.
  (lightning-platform electricity true)
  (ice-platform frost true)
  (meteor-platform meteors true)
)

(table takes-from-treasury
  (u* m* false)					;; Only control stations can draw from treasuries.
  (lightning-controller electricity true)
  (hailstorm-controller frost true)
  (meteor-controller meteors true)
)

(table unit-consumption-per-size
  ;; Should have different consumption at different levels, but that would
  ;; require the entire food supply to be re-worked!
  (bases food 1)
)

(table unit-consumption-to-grow
  ;; (bases food 25)		;; Might need to play around with this number.

  (base-1 food 60)
  (base-2 food 50)
  (base-3 food 40)
  (base-4 food 30)
  (base-5 food 20)
  (base-6 food 10)
)

(table base-consumption
  (u* fuel 1)			;; All units require fuel...
  (places m* 0)			;; ...except places...
  (undead m* 0)			;; ...and undead.
  (facilities fuel 0)
  (satellites fuel 0)
)

(table consumption-as-occupant
  (u* fuel 0)
)

(table material-to-move
  ;; This is supposed to be unnecessary, but without this declaration, units
  ;; can move around even if they run out of fuel.
  (movers fuel 1)
  (undead fuel 0)
)

(table consumption-per-move
  (movers fuel 1)
  (undead fuel 0)
)

(table consumption-per-attack
  (u* ammo 1)			;; Almost everything fires ordinary bullets up close.

  ;; But some units can just punch their enemies with iron fists...
  (cacofiend ammo 0)
  (undead ammo 0)

  ;; ...or they use a different ammunition...
  (acidic ammo 0)
  (burning ammo 0)
  (freezing ammo 0)
  (electrical ammo 0)

  (acidic acid 1)
  (burning flame 1)
  (freezing frost 1)
  (electrical electricity 1)


  (engineer ammo 0)		;; Engineers don't attack; they "clear".
)

(table consumption-per-fire
  (tower ammo 1)
  (military-1 ammo 1)
  (military-2 ammo 1)

  ;; The Level 3 military units are a little more interesting!
  (air electricity 1)
  (water electricity 1)
  (fire flame 1)

  ;; Every kind of artillery uses a different ammunition
  (howitzer ammo 1)
  (acidic acid 1)
  (burning flame 1)
  (electrical electricity 1)
  (freezing frost 1)
  (pulse-cannon ammo 1)

  ;; Controllers work like artillery, but their ammunition comes from above.
  (lightning-controller electricity 1)
  (hailstorm-controller frost 1)
  (meteor-platform meteors 1)
)

(table hit-by
  (u* ammo 1)
  (u* acid 1)
  (u* flame 1)
  (u* electricity 1)
  (u* frost 1)
)

(table unit-initial-supply
  ;; Start fully armed and fueled, but work for the rest.
  ;; (may need some limitations)
  (u* fuel 1000)
  (u* ammo 1000)

  ;; Wrecks are always full of scrap metal.
  (wrecks scrap 9999)
)

(table supply-on-completion
  ;; This seems to be necessary because of the time.g-like upgrade mechanism.
  ;; If a newly-completed base does not start off with at least one unit of
  ;; food, it will starve before it can start farming.
  (bases food 1)
)

(table hp-per-starve
  ;; Most units will not die if they run out of fuel; they will just be
  ;; immobilized.  Aircraft, however, crash and die instantly if they run out
  ;; of fuel and cannot land safely.  Helicopters are exempt because they can
  ;; land anywhere.
  ;; (shouldn't helicopters die if they run out of fuel over water?)
  (aircraft fuel 100.00)
  (choplifter fuel 0.00)
  (helicopter fuel 0.00)

  (wrecks scrap 100.00)
)

;; Centipedes gather metal by collecting it from wrecks.
;; They can then use them to create "undead" units.

(table acp-to-extract
  (centipede scrap 1)
)

;;; CONSTRUCTION

(table can-create
  ;; A base can build anything except bases, towers, and undead robots.
  (bases places 0)
  (bases undead 0)

  (base-1 base-2 1)
  (base-2 base-3 1)
  (base-3 base-4 1)
  (base-4 base-5 1)
  (base-5 base-6 1)

  (bases facilities-1 1)

  (bases constructors 1)

  ;; (bases transports 1)
  (bases boat 1)
  ((base-2 base-3 base-4 base-5 base-6) carrier 1)
  ((base-2 base-3 base-4 base-5 base-6) choplifter 1)
  ((base-3 base-4 base-5 base-6) platform 1)
  ((base-4 base-5 base-5) mothership 1)

  ((base-1 base-2 base-3 base-4 base-5 base-6) military-1 1)
  ((base-2 base-3 base-4 base-5 base-6) military-2 1)
  ((base-3 base-4 base-5 base-6) military-3 1)
  ((base-4 base-5 base-6) military-4 1)
  ((base-5 base-6) military-5 1)
  (base-6 military-6 1)

  ;; Only Level 3 or higher bases have access to advanced artillery and supplies.
  (bases howitzer 1)
  ((base-3 base-4 base-5 base-6) artillery 1)
  ((base-3 base-4 base-5 base-6) facilities-2 1)
  (bases explosives-1 1)
  ((base-3 base-4 base-5 base-6) explosives-2 1)
  (base-3 magma-drill 0)

  ((base-3 base-4 base-5 base-6) controllers 1)
  ((base-3 base-4 base-5 base-6) satellites 1)

  ;; A valar can build anything that a base can (up to Level 4 military units).
  (valar u* 1)
  (valar places 0)
  (valar military-5 0)
  (valar military-6 0)
  (valar undead 0)

  ;; A tower can only build explosives.
  (tower explosives-1 4)
  (tower explosives-2 4)
  (tower facilities-2 4)

  ;; Engineers can build places.
  (engineer places 3)
  (engineer ruins 0)

  ;; Platforms can be equipped to produce some explosives.
  (platform explosives-1 3)
  (platform explosives-2 3)
  (platform orb 0)

  ;; Only miners can lay minefields.
  (u* mine 0)
  (miner mine 3)

  ;; Tanks can be equipped to build cruise missiles.
  (tank rocket 4)

  ;; Centipedes collect scrap metal and create undead robots.
  (centipede undead 3)

) ;  (anything else?)

(table can-build
  ;; A base can build anything except bases and towers.
  (bases u* 1)
  (bases places 0)
  (bases undead 0)

  (base-1 base-2 1)
  (base-2 base-3 1)
  (base-3 base-4 1)
  (base-4 base-5 1)
  (base-5 base-6 1)

  ;; A valar can build anything that a base can (up to Level 4 military units).
  (valar u* 1)
  (valar places 0)
  (valar military-5 0)
  (valar military-6 0)
  (valar undead 0)

  ;; A tower can only build explosives.
  (tower explosives-1 4)
  (tower explosives-2 4)
  (tower facilities-2 4)

  ;; Engineers can build places.
  (engineer places 3)
  (engineer ruins 0)

  ;; Only miners can lay minefields.
  (u* mine 0)
  (miner mine 3)

  ;; Platforms can be equipped to produce some explosives.
  (platform explosives-1 3)
  (platform explosives-2 3)
  (platform orb 0)

  ;; Tanks can be equipped to build cruise missiles.
  (tank rocket 4)

  ;; Centipedes collect scrap metal and create undead robots.
  (centipede undead 3)

) ;  (anything else?)

(table acp-to-create
  ;; A base can build anything except bases, towers, and undead robots.
  (bases places 0)
  (bases undead 0)

  (base-1 base-2 1)
  (base-2 base-3 1)
  (base-3 base-4 1)
  (base-4 base-5 1)
  (base-5 base-6 1)

  (bases facilities-1 1)

  (bases constructors 1)

  ;; (bases transports 1)
  (bases boat 1)
  ((base-2 base-3 base-4 base-5 base-6) carrier 1)
  ((base-2 base-3 base-4 base-5 base-6) choplifter 1)
  ((base-3 base-4 base-5 base-6) platform 1)
  ((base-4 base-5 base-5) mothership 1)

  ((base-1 base-2 base-3 base-4 base-5 base-6) military-1 1)
  ((base-2 base-3 base-4 base-5 base-6) military-2 1)
  ((base-3 base-4 base-5 base-6) military-3 1)
  ((base-4 base-5 base-6) military-4 1)
  ((base-5 base-6) military-5 1)
  (base-6 military-6 1)

  ;; Only Level 3 or higher bases have access to advanced artillery and supplies.
  (bases howitzer 1)
  ((base-3 base-4 base-5 base-6) artillery 1)
  ((base-3 base-4 base-5 base-6) facilities-2 1)
  (bases explosives-1 1)
  ((base-3 base-4 base-5 base-6) explosives-2 1)
  (base-3 magma-drill 0)

  ((base-3 base-4 base-5 base-6) controllers 1)
  ((base-3 base-4 base-5 base-6) satellites 1)

  ;; A valar can build anything that a base can (up to Level 4 military units).
  (valar u* 1)
  (valar places 0)
  (valar military-5 0)
  (valar military-6 0)
  (valar undead 0)

  ;; A tower can only build explosives.
  (tower explosives-1 4)
  (tower explosives-2 4)
  (tower facilities-2 4)

  ;; Engineers can build places.
  (engineer places 3)
  (engineer ruins 0)

  ;; Platforms can be equipped to produce some explosives.
  (platform explosives-1 3)
  (platform explosives-2 3)
  (platform orb 0)

  ;; Only miners can lay minefields.
  (u* mine 0)
  (miner mine 3)

  ;; Tanks can be equipped to build cruise missiles.
  (tank rocket 4)

  ;; Centipedes collect scrap metal and create undead robots.
  (centipede undead 3)

) ;  (anything else?)

(table acp-to-build
  ;; A base can build anything except bases and towers.
  (bases u* 1)
  (bases places 0)
  (bases undead 0)

  (base-1 base-2 1)
  (base-2 base-3 1)
  (base-3 base-4 1)
  (base-4 base-5 1)
  (base-5 base-6 1)

  ;; A valar can build anything that a base can (up to Level 4 military units).
  (valar u* 1)
  (valar places 0)
  (valar military-5 0)
  (valar military-6 0)
  (valar undead 0)

  ;; A tower can only build explosives.
  (tower explosives-1 4)
  (tower explosives-2 4)
  (tower facilities-2 4)

  ;; Engineers can build places.
  (engineer places 3)
  (engineer ruins 0)

  ;; Only miners can lay minefields.
  (u* mine 0)
  (miner mine 3)

  ;; Platforms can be equipped to produce some explosives.
  (platform explosives-1 3)
  (platform explosives-2 3)
  (platform orb 0)

  ;; Tanks can be equipped to build cruise missiles.
  (tank rocket 4)

  ;; Centipedes collect scrap metal and create undead robots.
  (centipede undead 3)

) ;  (anything else?)

(table create-range
  (places mine 3)
  (miner mine 1)
  (engineer wall 1)
)

(table build-range
  (places mine 3)
  (miner mine 1)
  (engineer wall 1)
)

(table consumption-per-built
  (u* metal 1)			;; by default

  (constructors metal 1)
  (transports metal 1)
  (military-1 metal 1)
  (military-2 metal 2)
  (military-4 metal 3)

  (undead metal 0)
  (undead scrap 1)

  (artillery metal 2)
  (controllers metal 2)
  (explosives-1 metal 1)
  (acid-ball acid 2)
  (firebomb flame 2)
  (magma-drill metal 4)

  (facilities metal 1)
  (satellites metal 2)

  (places metal 1)
)

;;; TECHNOLOGY

;; Bases and towers can develop units; nothing else can.

(table acp-to-develop
  (bases u* 1)

  (bases bases 0)	;; By default
  (base-1 base-2 1)
  (base-2 base-3 1)
  (base-3 base-4 1)
  (base-4 base-5 1)
  (base-5 base-6 1)

  (tower explosives-1 4)
  (tower explosives-2 4)
  (engineer base-1 3)
  (engineer tower 3)
  (centipede undead 3)
)

(table tech-per-develop
  (u* u* 1.00)
)

(add base-1 tech-to-build 100)
(add base-2 tech-to-build 150)
(add base-3 tech-to-build 200)
(add base-4 tech-to-build 250)
(add base-5 tech-to-build 300)
(add base-6 tech-to-build 350)

(add tower tech-to-build 50)

(add engineer tech-to-build 25)
(add miner tech-to-build 20)

(add boat tech-to-build 10)
(add carrier tech-to-build 100)
(add choplifter tech-to-build 100)
(add platform tech-to-build 50)
(add mothership tech-to-build 200)

(add tank tech-to-build 50)
(add battledroid tech-to-build 10)
(add destroyer tech-to-build 20)
(add minesweeper tech-to-build 20)

(add spy-plane tech-to-build 60)
(add helicopter tech-to-build 125)
(add bomber tech-to-build 75)
(add fighter tech-to-build 50)
(add battleship tech-to-build 30)
(add submarine tech-to-build 50)
(add battlebot-1 tech-to-build 15)
(add battlebot-2 tech-to-build 20)
(add battlebot-3 tech-to-build 25)

(add werewolf tech-to-build 75)
(add spider tech-to-build 100)
(add doppleganger tech-to-build 115)
(add ankheg tech-to-build 150)

(add centipede tech-to-build 150)
(add air tech-to-build 200)
(add water tech-to-build 200)
(add earth tech-to-build 200)
(add fire tech-to-build 200)
(add cacofiend tech-to-build 400)

(add phoenix tech-to-build 300)
(add stormcloud tech-to-build 400)

(add valar tech-to-build 800)
(add dark-valar tech-to-build 800)

(add zombie tech-to-build 20)

(add howitzer tech-to-build 25)
(add acid-gun tech-to-build 80)
(add flame-gun tech-to-build 80)
(add lightning-gun tech-to-build 80)
(add frost-gun tech-to-build 80)
(add pulse-cannon tech-to-build 80)
(add anti-aircraft tech-to-build 40)

(add lightning-controller tech-to-build 160)
(add meteor-controller tech-to-build 160)
(add hailstorm-controller tech-to-build 160)

(add rocket tech-to-build 25)
(add orb tech-to-build 75)

(add acid-ball tech-to-build 100)
(add firebomb tech-to-build 100)
(add magma-drill tech-to-build 200)

(add acid-factory tech-to-build 20)
(add oil-plant tech-to-build 20)
(add solar-complex tech-to-build 20)
(add ice-factory tech-to-build 20)

(add lightning-platform tech-to-build 150)
(add meteor-platform tech-to-build 150)
(add ice-platform tech-to-build 150)

(add base-1 tech-max 100)
(add base-2 tech-max 150)
(add base-3 tech-max 200)
(add base-4 tech-max 250)
(add base-5 tech-max 300)
(add base-6 tech-max 350)

(add tower tech-max 50)

(add engineer tech-max 25)
(add miner tech-max 20)

(add boat tech-max 10)
(add carrier tech-max 100)
(add choplifter tech-max 100)
(add platform tech-max 50)
(add mothership tech-max 200)

(add tank tech-max 50)
(add battledroid tech-max 10)
(add destroyer tech-max 20)
(add minesweeper tech-max 20)

(add spy-plane tech-max 60)
(add helicopter tech-max 125)
(add bomber tech-max 75)
(add fighter tech-max 50)
(add battleship tech-max 30)
(add submarine tech-max 50)
(add battlebot-1 tech-max 15)
(add battlebot-2 tech-max 20)
(add battlebot-3 tech-max 25)

(add werewolf tech-max 75)
(add spider tech-max 100)
(add doppleganger tech-max 115)
(add ankheg tech-max 150)

(add centipede tech-max 150)
(add air tech-max 200)
(add water tech-max 200)
(add earth tech-max 200)
(add fire tech-max 200)
(add cacofiend tech-max 400)

(add phoenix tech-max 300)
(add stormcloud tech-max 400)

(add valar tech-max 800)
(add dark-valar tech-max 800)

(add zombie tech-max 20)

(add howitzer tech-max 25)
(add acid-gun tech-max 80)
(add flame-gun tech-max 80)
(add lightning-gun tech-max 80)
(add frost-gun tech-max 80)
(add pulse-cannon tech-max 80)
(add anti-aircraft tech-max 40)

(add lightning-controller tech-max 160)
(add meteor-controller tech-max 160)
(add hailstorm-controller tech-max 160)

(add rocket tech-max 25)
(add orb tech-max 75)

(add acid-ball tech-max 100)
(add firebomb tech-max 100)
(add magma-drill tech-max 200)

(add acid-factory tech-max 20)
(add oil-plant tech-max 20)
(add solar-complex tech-max 20)
(add ice-factory tech-max 20)

(add lightning-platform tech-max 150)
(add meteor-platform tech-max 150)
(add ice-platform tech-max 150)


(table can-toolup-for
  (bases u* true)
  (tower u* true)
  (engineer u* true)
  (platform u* true)
  (tank u* true)
  (valar u* true)
)

(table acp-to-toolup
  (bases u* 1)
  (tower u* 4)
  (engineer u* 3)
  (platform u* 3)
  (tank u* 4)
  (valar u* 1)
)

(table tp-per-toolup
  (u* u* 1)
)

(table tp-to-build
  (u* base-1 10)
  (u* base-2 12)
  (u* base-3 14)
  (u* base-4 16)
  (u* base-5 18)
  (u* base-6 20)

  (u* tower 10)

  (u* engineer 10)
  (u* miner 15)

  (u* boat 2)
  (u* carrier 8)
  (u* choplifter 15)
  (u* mothership 50)

  (u* tank 30)
  (u* battledroid 5)
  (u* destroyer 5)
  (u* minesweeper 10)

  (u* spy-plane 30)
  (u* helicopter 20)
  (u* bomber 15)
  (u* fighter 10)
  (u* battleship 5)
  (u* submarine 6)
  (u* battlebot-1 1)
  (u* battlebot-2 2)
  (u* battlebot-3 3)

  (u* werewolf 15)
  (u* spider 25)
  (u* doppleganger 25)
  (u* ankheg 30)

  (u* centipede 40)
  (u* air 60)
  (u* water 60)
  (u* earth 60)
  (u* fire 60)
  (u* cacofiend 120)

  (u* phoenix 120)
  (u* stormcloud 120)

  (u* valar 200)
  (u* dark-valar 200)

  (u* zombie 10)

  (u* howitzer 10)
  (u* acid-gun 20)
  (u* flame-gun 20)
  (u* lightning-gun 20)
  (u* frost-gun 20)
  (u* pulse-cannon 20)
  (u* anti-aircraft 15)

  (u* lightning-controller 40)
  (u* meteor-controller 40)
  (u* hailstorm-controller 40)

  (u* rocket 20)
  (u* orb 30)

  (u* acid-ball 50)
  (u* firebomb 50)
  (u* magma-drill 100)

  ;; Just to make the "Help" screen a little clearer...
  (places places 0)
  (tower aircraft 0)
  (tower naval 0)
  (tower walkers 0)
  (tower wheeled 0)

  (platform aircraft 0)
  (platform naval 0)
  (platform walkers 0)
  (platform wheeled 0)
)

(table tp-max
  (u* base-1 10)
  (u* base-2 12)
  (u* base-3 14)
  (u* base-4 16)
  (u* base-5 18)
  (u* base-6 20)

  (u* tower 10)

  (u* engineer 10)
  (u* miner 15)

  (u* boat 2)
  (u* carrier 8)
  (u* choplifter 15)
  (u* mothership 50)

  (u* tank 30)
  (u* battledroid 5)
  (u* destroyer 5)
  (u* minesweeper 10)

  (u* spy-plane 30)
  (u* helicopter 20)
  (u* bomber 15)
  (u* fighter 10)
  (u* battleship 5)
  (u* submarine 6)
  (u* battlebot-1 1)
  (u* battlebot-2 2)
  (u* battlebot-3 3)

  (u* werewolf 15)
  (u* spider 25)
  (u* doppleganger 25)
  (u* ankheg 30)

  (u* centipede 40)
  (u* air 60)
  (u* water 60)
  (u* earth 60)
  (u* fire 60)
  (u* cacofiend 120)

  (u* phoenix 120)
  (u* stormcloud 120)

  (u* valar 200)
  (u* dark-valar 200)

  (u* zombie 10)

  (u* howitzer 10)
  (u* acid-gun 20)
  (u* flame-gun 20)
  (u* lightning-gun 20)
  (u* frost-gun 20)
  (u* pulse-cannon 20)
  (u* anti-aircraft 15)

  (u* lightning-controller 40)
  (u* meteor-controller 40)
  (u* hailstorm-controller 40)

  (u* rocket 20)
  (u* orb 30)

  (u* acid-ball 50)
  (u* firebomb 50)
  (u* magma-drill 100)

  ;; Just to make the "Help" screen a little clearer...
  (places places 0)
  (tower aircraft 0)
  (tower naval 0)
  (tower walkers 0)
  (tower wheeled 0)

  (platform aircraft 0)
  (platform naval 0)
  (platform walkers 0)
  (platform wheeled 0)
)

;; (Cross your fingers; I hope this all works properly!)

;;; TERRAIN ENGINEERING

(table acp-to-add-terrain
  (engineer road 3)
  (engineer river 3)
  (engineer steppe 2)
  (engineer plain 2)
  (engineer forest 2)

  (valar t* 1)
)

(table acp-to-remove-terrain
  (engineer road 3)
  (engineer river 3)

  (engineer semi-desert 1)
  (engineer desert 1)

  (valar t* 1)
)

;;; COMBAT

(table acp-to-attack
  (u* u* 0)
  (tower u* 1)
  (military-1 u* 1)
  (military-2 u* 1)
  (military-3 u* 1)
  (military-4 u* 1)
  (military-5 u* 1)
  (military-6 u* 1)

  (centipede wrecks 0)	;; Centipedes have more use for wrecks than anyone else.

  (engineer wrecks 1)
  ;; Artillery, controllers, and explosives are excluded because they attack differently.
)

(add tower acp-to-fire 2)
(add tank acp-to-fire 2)
(add helicopter acp-to-fire 2)
(add battleship acp-to-fire 2)
(add air acp-to-fire 2)
(add water acp-to-fire 2)
(add fire acp-to-fire 2)
(add phoenix acp-to-fire 2)
(add stormcloud acp-to-fire 2)
(add valar acp-to-fire 2)
(add dark-valar acp-to-fire 2)

(add artillery acp-to-fire 1)
(add controllers acp-to-fire 1)

(add tower range 8)
(add tank range 4)
(add helicopter range 3)
(add battleship range 4)
(add air range 4)
(add water range 4)
(add fire range 4)
(add phoenix range 4)
(add stormcloud range 6)
(add valar range 8)
(add dark-valar range 8)

(add artillery range 6)
(add controllers range 8)

(add tower fire-angle-max 100)
(add helicopter fire-angle-max 100)
(add artillery fire-angle-max 50)
(add anti-aircraft fire-angle-max 100)	;; Aircraft fly; the terrain doesn't protect them from enemy fire.

(table weapon-height
  (tower t* 10000)
  (aircraft t* 10000)
)

(add explosives-1 acp-to-detonate 1)
(add explosives-2 acp-to-detonate 1)
(add explosives-1 hp-per-detonation 1)
(add explosives-2 hp-per-detonation 1)
(add mine detonate-on-death 100)
;; Should some things explode when they die?

(table hit-chance
  (u* u* 50)
  (u* aircraft 25)

  ;; Certain units are built to kill aircraft.
  (fighter aircraft 75)
  (anti-aircraft aircraft 75)

  (u* places 75)	;; Most stationery targets are very easy to hit.
  (u* wall 100)		;; Walls are stationery and very hard to miss.
  (u* ruins 100)	;; Likewise for ruins.

  (u* mine 100)
) ; (should elaborate?)

(table capture-chance
  ;; Undead are excluded because they (theoretically) only have the power to destroy.
  (military-1 bases 20)
  (military-2 bases 40)
  (military-3 bases 60)
  (military-4 bases 80)
  (military-5 bases 90)
  (military-6 bases 100)
  (tank bases 50)

  ;; Whenever a place is captured, so are its facilities (but not its satellites).
  (military-1 facilities 100)
  (military-2 facilities 100)
  (military-3 facilities 100)
  (military-4 facilities 100)
  (military-5 facilities 100)
  (military-6 facilities 100)

  ;; Boats and aircraft can't capture.
  (aircraft bases 0)
  (naval bases 0)
  ;; But helicopters can.
  (helicopter bases 50)
)

(table independent-capture-chance
  ;; Anything without a commander is easy to take over.
  ;; (why doesn't this work?)
  (military-1 u* 100)
  (military-2 u* 100)
  (military-3 u* 100)
  (military-4 u* 100)
  (military-5 u* 100)
  (military-6 u* 100)

  (aircraft u* 0)
  (naval u* 0)
  (helicopter u* 100)
)

(table occupant-escape-chance
  ;; All units are smart enough to bail out if they're in something that's
  ;; about to be captured or destroyed.
  (u* u* 100)
)

(table damage
  (u* u* 1d6)			;; By default


  (transports u* 1d4)		;; Most transports have minimal defensive capability.
  (mothership u* 1d6)		;; But motherships are built for survival.

  (submarine naval 2d6)		;; Submarines use surprise to do double damage!
  (submarine submarine 1d6)	;; But they can't sneak up on other submarines so easily.
  (submarine destroyer 1d6)	;; Destroyers are built to hunt and kill subs; they are difficult to catch unaware.

  (destroyer submarine 2d6)	;; Destroyers are most skilled at destryoing subs.

  (fighter aircraft 3d4)	;; Fighters eat other aircraft for lunch!

  (spider u* 4d6)		;; Spiders rust their opponents, making them quite dangerous.
  (spider gaseous 1d6)		;; But rust is not effective against gaseous creatures.
  (spider wet 1d6)		;; Nor against water-based creatures.
  (spider firey 1d6)		;; Nor against fire-bases creatures.
  (spider nonmetallic 1d6)	;; Nor against non-metallic creatures.

  (doppleganger u* 3d6)		;; Dopplegangers have melee abilities far beyond those of ordinary battledroids.

  (earth u* 4d4)		;; Earth robots can slam their foes.
  (cacofiend u* 6d4)		;; Cacofiends can slam their foes with 16-ton arms!

  (valar u* 7d6)		;; The valar blasts everything it hits!
  (dark-valar u* 7d6)		;; As does the dark valar!

  (engineer wall 6d6)		;; A skilled demolisher can destroy a wall very quickly.
  (engineer wrecks 6d6)		;; Likewise for wrecks and ruins.

  (undead u* 2d4)		;; Undead are a little more fearsome than your ordinary battledroid.

  (acidic u* 4d6)		;; Acid hurts almost everything.
  (acidic air 2d6)		;; But not air robots.
  (acidic water 2d6)		;; Nor water robots.

  (burning u* 4d4)		;; Fire is slightly more painful to most things.
  (burning wet 7d4)		;; But especially to water robots.

  (electrical u* 4d6)		;; Electricity is disastrous to electronics.
  (electrical wall 2d6)		;; But not to walls.
  (electrical firey 2d6)	;; Nor to fire.
  (electrical nonmetallic 2d6)	;; Nor to earth.

  (freezing u* 2d6)		;; Frost is a minor hazard to most.
  (freezing gaseous 4d6)	;; But is very painful to air.
  (freezing wet 7d4)		;; And crippling to water.
  (freezing firey 7d6)		;; And devastating to fire.

  (u* fireshielded 1d2)		;; Fire-shielded units are highly resistant to nearly everything.
  (acidic fireshielded 1d4)	;; Acid is somewhat more effective against them.
  (burning fireshielded 0)	;; Fire has no effect on such a unit.
  (freezing fireshielded 6d6)	;; Cold-based weapons are their natural enemy.
  (electrical fireshielded 1d6)	;; Electrical weapons are relatively effective against them.
)

;; The numers below may need a little more work; so far, the games goes too fast to build and test any of the more powerful units.
(table fire-damage
  (u* u* 1d6)			;; By default
  (howitzer u* 2d6)		;; Howitzers are very painful vs. everything (although others may be even more so).

  (pulse-cannon u* 200)		;; A shockwave will blast anything if it can't back away.

  (anti-aircraft aircraft 3d4)	;; Anti-aircraft guns eat aircraft for lunch!

  (acidic u* 4d6)		;; Acid hurts almost everything.
  (acidic gaseous 2d6)		;; But not air.
  (acidic wet 2d6)		;; Nor water.

  (burning u* 4d4)		;; Fire is slightly more painful to most things.
  (burning wet 7d4)		;; But especially to water robots.

  (electrical u* 4d6)		;; Electricity is disastrous to electronics.
  (electrical wall 2d6)		;; But not to walls.
  (electrical firey 2d6)	;; Nor to fire.
  (electrical nonmetallic 2d6)	;; Nor to earth.

  (freezing u* 2d6)		;; Frost is a minor hazard to most.
  (freezing gaseous 4d6)	;; But is very painful to air.
  (freezing wet 7d4)		;; And crippling to water.
  (freezing firey 7d6)		;; And devastating to fire.

  (meteor-platform u* 7d6)	;; Nothing is resistant to falling boulders!

  (u* fireshielded 1d2)		;; Fire-shielded units are highly resistant to nearly everything.
  (acidic fireshielded 1d4)	;; Acid is somewhat more effective against them.
  (burning fireshielded 0)	;; Fire has no effect on such a unit.
  (freezing fireshielded 6d6)	;; Cold-based weapons are their natural enemy.
  (electrical fireshielded 1d6)	;; Electrical weapons are relatively effective against them.
)

(table attack-terrain-effect
  ;; Few units are affected by terrain, combat-wise, but there are some that
  ;; cannot function properly in certain terrain.
  
  ;; Ankhegs are only effective in the wide open spaces.
  (ankheg forest 0.25)
  (ankheg semi-desert 0.75)
  (ankheg desert 0.50)
  (ankheg mountain 0.05)
  (ankheg t-water 0.00)
)

;; (should define defend-terrain-effect?)

(table detonation-unit-range
  (mine u* 1)
  (rocket u* 1)
  (orb u* 2)
  (acidic u* 3)
  (firebomb u* 3)
  (magma-drill u* 4)
)

(table retreat-chance
  ;; A pulse cannon pushes its target back using a shockwave!
  (pulse-cannon movers 100)
  (pulse-cannon undead 0)	;; Undead are too dumb to escape (Come on, they need some kind of weakness!)
)

(table acp-for-retreat
  ;; A shockwave from a pulse cannon can even throw the target into hostile terrain!
  (pulse-cannon movers 999)
)

;; Detonation damage should be a dice spec like everything else, but Xconq doesn't support it.

(table detonation-damage-at
  (mine u* 25)
  (mine minesweeper 0)
  (rocket u* 50)
  (rocket places 100)
  (orb u* 100)
  (orb places 200)

  (acidic u* 300)
  (acidic air 50)
  (acidic water 50)

  (burning u* 150)
  (burning water 200)

  (magma-drill u* 400)
)

(table detonation-damage-adjacent
  (mine u* 10)
  (mine minesweeper 0)
  (rocket u* 25)
  (rocket places 50)
  (orb u* 50)
  (orb places 100)

  (acidic u* 50)
  (acidic air 25)
  (acidic water 25)

  (burning u* 50)
  (burning water 100)

  (magma-drill u* 200)	;; Lava flows are quite deadly

  (u* aircraft 0)	;; Aircraft cannot be collateral damage from a missile strike; they must be targeted directly.
)

(table detonation-terrain-damage-chance
  (mine t* 25)
  (rocket t* 50)
  (orb t* 75)
  (acidic t* 75)
  (burning t* 50)
  (magma-drill t* 100)

  ;; Desert is indestructible, but magma drills can level mountains.
  (u* desert 0)
  (u* mountain 0)
  (u* lake 0)
  (u* sea 0)
  (u* ice 0)
  (magma-drill mountain 50)
)

(table detonation-terrain-range
  (rocket t* 0)
  (orb t* 1)
  (acidic t* 2)
  (burning t* 2)
  (magma-drill t* 3)
)

(table terrain-damaged-type
  (plain semi-desert 50)
  (steppe semi-desert 50)
  (forest steppe 75)
  (semi-desert desert 25)
  (swamp semi-desert 50)
  (salt-marsh desert 50)
  (mountain desert 50)
  (lake swamp 50)
  (sea salt-marsh 50)

  ;; Need some way to produce effects such as flooding, fire, acid pools, and lava flows.
  ;; (especially if a glacier is destroyed, everything surounding it should be flooded for several turns)
)

(table protection
  ;; A place cannot be captured if it has armed defenders.
  (military-1 places 0.00)
  (military-2 places 0.00)
  (military-3 places 0.00)
  (military-4 places 0.00)
  (military-5 places 0.00)
  (military-6 places 0.00)

  ;; Missiles cannot be shot down if they're occupying a launcher (but the launcher can be destroyed)
  (u* explosives-1 0.00)
  (u* explosives-2 0.00)
) ;; (this should not provide protection against firing attacks, but apparently it does)

;;; REPAIR

(table auto-repair
  (bases u* 4.00)	;; (maybe should have different rates for different levels?)
  (tower u* 1.00)

  (valar u* 3.00)
)

(add bases hp-recovery 0.5)
(add tower hp-recovery 0.75)

;;; VISION

(add places see-always true)

(add u* vision-range 4)
(add tower vision-range 8)
(add mothership vision-range 8)
(add valar vision-range 8)
(add dark-valar vision-range 8)
(add artillery vision-range 6)
(add explosives-1 vision-range 1)
(add explosives-2 vision-range 2)

(add u* can-see-behind false)
(add places can-see-behind true)
(add aircraft can-see-behind true)
(add air can-see-behind true)
(add valar can-see-behind true)
(add dark-valar can-see-behind true)

(table see-chance
  ;; Some units are built to avoid being seen, but they all have a weakness...

  (u* mine 5)
  (minesweeper mine 75)

  (u* spy-plane 10)
  (fighter spy-plane 75)

  (u* submarine 5)
  (destroyer submarine 75)
  
  (u* ankheg 5)			;; Ankhegs remain nigh-invisible until adjacent to an opponent.

  (u* stormcloud 10)
  (fighter stormcloud 75)
)

(table visibility
  ;; Make everything a little harder to see in rough terrain.
  (u* forest 0.5)
  (u* mountain 0.75)
)

(table see-chance-adjacent
  ;; Places and units cannot be masked by terrain if they're right next to enemy eyes!
  (u* u* 200)
  ;; Mines, spy planes, submarines, etc. are normally hard to see, even up close.
  (u* mine 5)
  (minesweeper mine 75)

  (u* spy-plane 10)
  (fighter spy-plane 75)

  (u* submarine 5)
  (destroyer submarine 75)
  
  (u* ankheg 100)

  (u* stormcloud 10)
  (fighter stormcloud 75)
)

(table see-mistake-chance
  (u* doppleganger 75)		;; Doppleganger are illusory.
)

(table looks-like
  (doppleganger battledroid 1)	;; And they're much more dangerous than ordinary battledroids.
)

;;; COMBAT EXPERIENCE

(add u* cxp-max 10000)
(add bases cxp-max 0)
(add transports cxp-max 0)
;; Should other units gain CXP?

(table cxp-per-combat
  (u* u* 1)
)

(table hit-cxp-effect
  (u* u* 10000)
)

(table damage-cxp-effect
  (u* u* 10000)
)

;; Somehow, CXP should give units additional hitpoints.

;;; BACKDROP

(table road-into-chance
  (land land 100)
  ;; No roads across ice fields.
  (land-t* ice 0)
  ;; Try to get a road back out into the plains.
  (cell-t* (plain steppe) 100)
  ;; Be reluctant to run through hostile terrain.
  (plain (semi-desert desert forest mountain) (35 40 30 20))
  )

(table road-chance
  ;; (bases (tower bases) (80 100))
  ;; (tower (tower bases) ( 2   5))
  (places places 80)
  )

(add places road-to-edge-chance 100)
(add places spur-chance 100)
(add places spur-range 5)

(set edge-road-density 100)

(table attrition
  (wrecks t* 1.00)
)

(add bases namer "swedish-place-names")
(add tower namer "swedish-place-names")

(set feature-types '(peaks islands lakes bays seas continents 
			    (desert 10)(semi-desert 10)(forest 10)(mountain 5)))

(set feature-namers
  '((islands generic-island-names) (lakes generic-lake-names) (bays generic-bay-names)
  	(seas generic-sea-names) (continents generic-continent-names)
  	(desert generic-desert-names) (forest generic-forest-names)
  	(mountains generic-mountain-names)))

;; If a side gives up, its forces don't just vanish.
(add u* lost-vanish-chance 0)
(add u* lost-revolt-chance 10000)

