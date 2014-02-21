(game-module "civ2"
  (title "Civilization II")
  (version "1.0")
  (blurb "Emulation of Civ2. Uses the Civ2 combat model, which differs from most other Xconq games.")
  (instructions (
    "Research scientific advances in order to produce more powerful units."
    "Make settlers that can build new cites and colonize all available land. "
    "Do not forget to make military units, or suffer the consequences!"
    ))
  (variants
    (world-seen false)
    (see-all false)
    (sequential true)
    (world-size (80 50 360))
    ("Alternate economy" model1
     "Use a different algorithm for moving materials among units."
     (true (set backdrop-model 1)))
    ("AltEcon uses doctrine" model1doctrine
     "When using alternate economy, don't drain units below doctrine levels."
     (true (set backdrop-ignore-doctrine 0))
     (false (set backdrop-ignore-doctrine 1)))
    )
  )

;;; Unit types.

;; Mobile units. (51)

(unit-type aegis-cruiser (name "AEGIS Cruiser") (image-name "cruiser")
  (attack 8) (defend 8) (acp-per-turn 5) (hp-max 30))
(unit-type alpine-troops (name "Alpine Troops") (image-name "soldiers")
  (attack 5) (defend 5) (acp-per-turn 3) (hp-max 20))
(unit-type archers (name "Archers") (image-name "archer")
  (attack 3) (defend 2) (acp-per-turn 3) (hp-max 10))
(unit-type armor (name "Armor") (image-name "tank")
  (attack 10) (defend 5) (acp-per-turn 9) (hp-max 30))
(unit-type artillery (name "Artillery") (image-name "howitzer")
  (attack 10) (defend 1) (acp-per-turn 3) (hp-max 20))
(unit-type battleship (name "Battleship") (image-name "bb")
  (attack 12) (defend 12) (acp-per-turn 4) (hp-max 40))
(unit-type bomber (name "Bomber") (image-name "4e")
  (attack 12) (defend 1) (acp-per-turn 8) (hp-max 20))
(unit-type cannon (name "Cannon") (image-name "cannon")
  (attack 8) (defend 1) (acp-per-turn 3) (hp-max 20))
(unit-type caravan (name "Caravan") (image-name "camel")
  (attack 0) (defend 1) (acp-per-turn 3) (hp-max 10))
(unit-type caravel (name "Caravel") (image-name "barge")
  (attack 2) (defend 1) (acp-per-turn 3) (hp-max 10))
(unit-type carrier (name "Carrier") (image-name "cv")
  (attack 1) (defend 9) (acp-per-turn 5) (hp-max 40))
(unit-type catapult (name "Catapult") (image-name "catapult")
  (attack 6) (defend 1) (acp-per-turn 3) (hp-max 10))
(unit-type cavalry (name "Cavalry") (image-name "cavalry")
  (attack 8) (defend 3) (acp-per-turn 6) (hp-max 20))
(unit-type chariot (name "Chariot") (image-name "wagon")
  (attack 3) (defend 1) (acp-per-turn 6) (hp-max 10))
(unit-type cruise-missile (name "Cruise Missile") (image-name "cruise-missile")
  (attack 20) (defend 0) (acp-per-turn 12) (hp-max 10))
(unit-type cruiser (name "Cruiser") (image-name "ca")
  (attack 6) (defend 6) (acp-per-turn 5) (hp-max 30))
(unit-type crusaders (name "Crusaders") (image-name "cavalry")
  (attack 5) (defend 1) (acp-per-turn 6) (hp-max 10))
(unit-type destroyer (name "Destroyer") (image-name "dd")
  (attack 4) (defend 4) (acp-per-turn 6) (hp-max 30))
(unit-type diplomat (name "Diplomat") (image-name "man")
  (attack 0) (defend 0) (acp-per-turn 6) (hp-max 10))
(unit-type dragoons (name "Dragoons") (image-name "soldiers")
  (attack 5) (defend 2) (acp-per-turn 6) (hp-max 20))
(unit-type elephants (name "Elephants") (image-name "elephant")
  (attack 4) (defend 1) (acp-per-turn 6) (hp-max 10))
(unit-type engineers (name "Engineers") (image-name "engineer")
  (attack 0) (defend 2) (acp-per-turn 6) (hp-max 20))
(unit-type explorers (name "Explorers") (image-name "explorer")
  (attack 0) (defend 1) (acp-per-turn 3) (hp-max 10))
(unit-type fanatics (name "Fanatics") (image-name "explorer")
  (attack 4) (defend 4) (acp-per-turn 3) (hp-max 20))
(unit-type fighter (name "Fighter") (image-name "fighter")
  (attack 4) (defend 2) (acp-per-turn 10) (hp-max 20))
(unit-type freight (name "Freight") (image-name "truck")
  (attack 0) (defend 1) (acp-per-turn 3) (hp-max 10))
(unit-type frigate (name "Frigate") (image-name "frigate")
  (attack 4) (defend 2) (acp-per-turn 4) (hp-max 210))
(unit-type galleon (name "Galleon") (image-name "frigate")
  (attack 0) (defend 2) (acp-per-turn 4) (hp-max 20))
(unit-type helicopter (name "Helicopter") (image-name "helicopter")
  (attack 10) (defend 3) (acp-per-turn 6) (hp-max 20))
(unit-type horsemen (name "Horsemen") (image-name "cavalry")
  (attack 2) (defend 1) (acp-per-turn 6) (hp-max 10))
(unit-type howitzer (name "Howitzer") (image-name "arty")
  (attack 12) (defend 2) (acp-per-turn 6) (hp-max 30))
(unit-type ironclad (name "Ironclad") (image-name "battleship")
  (attack 4) (defend 4) (acp-per-turn 4) (hp-max 30))
(unit-type knights (name "Knights") (image-name "knight")
  (attack 4) (defend 2) (acp-per-turn 6) (hp-max 10))
(unit-type legion (name "Legion") (image-name "legion")
  (attack 4) (defend 2) (acp-per-turn 3) (hp-max 10))
(unit-type marines (name "Marines") (image-name "soldiers")
  (attack 8) (defend 5) (acp-per-turn 3) (hp-max 20))
(unit-type mech-infantry (name "Mechanized Infantry") (image-name "puma")
  (attack 6) (defend 6) (acp-per-turn 9) (hp-max 30))
(unit-type musketeer (name "Musketeer") (image-name "soldier-acw")
  (attack 3) (defend 3) (acp-per-turn 3) (hp-max 20))
(unit-type nuclear-missile (name "Nuclear Missile") (image-name "icbm")
  (attack 99) (defend 0) (acp-per-turn 16) (hp-max 10))
(unit-type paratroopers (name "Paratroopers") (image-name "swordman")
  (attack 6) (defend 4) (acp-per-turn 3) (hp-max 20))
(unit-type partisans (name "Partisans") (image-name "swordman")
  (attack 4) (defend 4) (acp-per-turn 3) (hp-max 20))
(unit-type phalanx (name "Phalanx") (image-name "swordman")
  (attack 1) (defend 2) (acp-per-turn 3) (hp-max 10))
(unit-type pikemen (name "Pikemen") (image-name "swordman")
  (attack 1) (defend 2) (acp-per-turn 3) (hp-max 10))
(unit-type riflemen (name "Riflemen") (image-name "soldiers")
  (attack 5) (defend 4) (acp-per-turn 3) (hp-max 20))
(unit-type settlers (name "Settlers") (image-name "settlers")
  (attack 0) (defend 1) (acp-per-turn 3) (hp-max 20))
(unit-type spy (name "Spy") (image-name "man")
  (attack 0) (defend 0) (acp-per-turn 9) (hp-max 10))
(unit-type stealth-bomber (name "Stealth Bomber") (image-name "stealth-bomber")
  (attack 14) (defend 3) (acp-per-turn 12) (hp-max 20))
(unit-type stealth-fighter (name "Stealth Fighter") (image-name "stealth-plane")
  (attack 8) (defend 3) (acp-per-turn 14) (hp-max 20))
(unit-type submarine (name "Submarine") (image-name "sub")
  (attack 10) (defend 2) (acp-per-turn 3) (hp-max 30))
(unit-type transport (name "Transport") (image-name "ap")
  (attack 0) (defend 3) (acp-per-turn 5) (hp-max 30))
(unit-type trireme (name "Trireme") (image-name "trireme")
  (attack 1) (defend 1) (acp-per-turn 3) (hp-max 10))
(unit-type warriors (name "Warriors") (image-name "warrior")
  (attack 1) (defend 1) (acp-per-turn 3) (hp-max 10))

;; City types.

(unit-type city (name "City") (image-name "parthenon")
  (acp-per-turn 0))

(add city advanced true)
(add city use-own-cell true)
(add city acp-independent true)		;	And they are also acp-independent.

;; City improvements. (38)

(unit-type airport (name "Airport") (image-name "airbase")
  )
(unit-type aqueduct (name "Aqueduct") (image-name "parthenon")
  )
(unit-type bank (name "Bank") (image-name "bank")
  )
(unit-type barracks (name "Barracks") (image-name "stockade")
  )
(unit-type capitalization (name "Capitalization") (image-name "bank")
  )
(unit-type cathedral (name "Cathedral") (image-name "cathedral")
  )
(unit-type city-walls (name "City Walls") (image-name "walltown")
  )
(unit-type coastal-fortress (name "Coastal Fortress") (image-name "walltown")
  )
(unit-type colosseum (name "Colosseum") (image-name "parthenon")
  )
(unit-type courthouse (name "Courthouse") (image-name "parthenon")
  )
(unit-type factory (name "Factory") (image-name "facility")
  )
(unit-type granary (name "Granary") (image-name "granary")
  )
(unit-type harbor (name "Harbor") (image-name "anchor")
  )
(unit-type hydro-plant (name "Hydro Plant") (image-name "parthenon")
  )
(unit-type library (name "Library") (image-name "parthenon")
  )
(unit-type mfg-plant (name "Manufacturing Plant") (image-name "refinery")
  )
(unit-type marketplace (name "Marketplace") (image-name "parthenon")
  )
(unit-type mass-transit (name "Mass Transit") (image-name "parthenon")
  )
(unit-type nuclear-plant (name "Nuclear Plant") (image-name "nuclear-plant")
  )
(unit-type offshore-platform (name "Offshore Platform") (image-name "oil-platform")
  )
(unit-type palace (name "Palace") (image-name "parthenon")
  )
(unit-type police-station (name "Police Station") (image-name "facility")
  )
(unit-type port-facility (name "Port Facility") (image-name "anchor")
  )
(unit-type power-plant (name "Power Plant") (image-name "refinery")
  )
(unit-type recycling-center (name "Recycling Center") (image-name "refinery")
  )
(unit-type research-lab (name "Research Lab") (image-name "facility")
  )
(unit-type sam-missile-battery (name "SAM Missile Battery") (image-name "facility")
  )
(unit-type sdi-defense (name "SDI Defense") (image-name "radar")
  )
(unit-type sewer-system (name "Sewer System") (image-name "facility")
  )
(unit-type solar-plant (name "Solar Plant") (image-name "facility")
  )
(unit-type ss-component (name "Space Ship Component") (image-name "space-transport")
  )
(unit-type ss-module (name "Space Ship Module") (image-name "space-transport")
  )
(unit-type ss-structural (name "Space Ship Structural") (image-name "space-transport")
  )
(unit-type stock-exchange (name "Stock Exchange") (image-name "parthenon")
  )
(unit-type superhighways (name "Superhighways") (image-name "parthenon")
  )
(unit-type supermarket (name "Supermarket") (image-name "facility")
  )
(unit-type temple (name "Temple") (image-name "parthenon")
  )
(unit-type university (name "University") (image-name "facility")
  )

;; Wonders. (28)

(unit-type adam-smith-company (name "Adam Smith's Trading Company") (image-name "beholder")
  )
(unit-type apollo-program (name "Apollo Program") (image-name "beholder")
  )
(unit-type colossus (name "Colossus") (image-name "beholder")
  )
(unit-type copernicus-observatory (name "Copernicus' Observatory") (image-name "radar")
  )
(unit-type cure-for-cancer (name "Cure for Cancer") (image-name "beholder")
  )
(unit-type darwins-voyage (name "Darwin's Voyage") (image-name "beholder")
  )
(unit-type eiffel-tower (name "Eiffel Tower") (image-name "tower")
  )
(unit-type great-library (name "Great Library") (image-name "beholder")
  )
(unit-type great-wall (name "Great Wall") (image-name "beholder")
  )
(unit-type hanging-gardens (name "Hanging Gardens") (image-name "beholder")
  )
(unit-type hoover-dam (name "Hoover Dam") (image-name "beholder")
  )
(unit-type isaac-newtons-college (name "Isaac Newton's College") (image-name "beholder")
  )
(unit-type js-bachs-cathedral (name "J.S. Bach's Cathedral") (image-name "cathedral")
  )
(unit-type king-richards-crusade (name "King Richard's Crusade") (image-name "beholder")
  )
(unit-type leonardos-workshop (name "Leonardo's Workshop") (image-name "tower")
  )
(unit-type lighthouse (name "Lighthouse") (image-name "tower")
  )
(unit-type magellans-expedition (name "Magellan's Expedition") (image-name "beholder")
  )
(unit-type manhattan-project (name "Manhattan Project") (image-name "beholder")
  )
(unit-type marco-polos-embassy (name "Marco Polo's Embassy") (image-name "beholder")
  )
(unit-type michelangelos-chapel (name "Michelangelo's Chapel") (image-name "beholder")
  )
(unit-type oracle (name "Oracle") (image-name "beholder")
  )
(unit-type pyramids (name "Pyramids") (image-name "pyramid")
  )
(unit-type seti-program (name "SETI Program") (image-name "beholder")
  )
(unit-type shakespeares-theatre (name "Shakespeare's Theatre") (image-name "beholder")
  )
(unit-type statue-of-liberty (name "Statue of Liberty") (image-name "beholder")
  )
(unit-type sun-tzus-war-academy (name "Sun Tzu's War Academy") (image-name "beholder")
  )
(unit-type united-nations (name "United Nations") (image-name "beholder")
  )
(unit-type womens-suffrage (name "Womens' Suffrage") (image-name "beholder")
  )

;; Other types of units.

(unit-type airbase (name "Airbase") (image-name "airbase")
  (help "rural airport/airbase"))

(unit-type fortress (name "Fortress") (image-name "walltown")
  (help "countryside fortifications"))

(unit-type village (name "Village") (image-name "hut"))

;;; Material types.

(material-type food (name "Food") (image-name "sheaf") (resource-icon 1))
(material-type shields (name "Shields") (image-name "shield") (resource-icon 2))
(material-type arrows (name "Arrows") (image-name "arrows") (resource-icon 3))
(material-type taxes (name "Taxes") (resource-icon 4) (treasury true))
(material-type luxuries (name "Luxuries"))
(material-type science (name "Science") (treasury true))

;;; Advance types. (89)

(advance-type advanced-flight (name "Advanced Flight"))
(advance-type alphabet (name "Alphabet"))
(advance-type amphibious-warfare (name "Amphibious Warfare"))
(advance-type astronomy (name "Astronomy"))
(advance-type atomic-theory (name "Atomic Theory"))
(advance-type automobile (name "Automobile"))
(advance-type banking (name "Banking"))
(advance-type bridge-building (name "Bridge Building"))
(advance-type bronze-working (name "Bronze Working"))
(advance-type ceremonial-burial (name "Ceremonial Burial"))
(advance-type chemistry (name "Chemistry"))
(advance-type chivalry (name "Chivalry"))
(advance-type code-of-laws (name "Code of Laws"))
(advance-type combined-arms (name "Combined Arms"))
(advance-type combustion (name "Combustion"))
(advance-type communism (name "Communism"))
(advance-type computers (name "Computers"))
(advance-type conscription (name "Conscription"))
(advance-type construction (name "Construction"))
(advance-type corporation (name "Corporation"))
(advance-type currency (name "Currency"))
(advance-type democracy (name "Democracy"))
(advance-type economics (name "Economics"))
(advance-type electricity (name "Electricity"))
(advance-type electronics (name "Electronics"))
(advance-type engineering (name "Engineering"))
(advance-type environmentalism (name "Environmentalism"))
(advance-type espionage (name "Espionage"))
(advance-type explosives (name "Explosives"))
(advance-type feudalism (name "Feudalism"))
(advance-type flight (name "Flight"))
(advance-type fundamentalism (name "Fundamentalism"))
(advance-type fusion-power (name "Fusion Power"))
(advance-type future-tech (name "Future Technology"))
(advance-type genetic-engineering (name "Genetic Engineering"))
(advance-type guerrilla-warfare (name "Guerrilla Warfare"))
(advance-type gunpowder (name "Gunpowder"))
(advance-type horseback-riding (name "Horseback Riding"))
(advance-type industrialization (name "Industrialization"))
(advance-type invention (name "Invention"))
(advance-type iron-working (name "Iron Working"))
(advance-type labor-union (name "Labor Union"))
(advance-type the-laser (name "The Laser"))
(advance-type leadership (name "Leadership"))
(advance-type literacy (name "Literacy"))
(advance-type machine-tools (name "Machine Tools"))
(advance-type magnetism (name "Magnetism"))
(advance-type map-making (name "Map Making"))
(advance-type masonry (name "Masonry"))
(advance-type mass-production (name "Mass Production"))
(advance-type mathematics (name "Mathematics"))
(advance-type medicine (name "Medicine"))
(advance-type metallurgy (name "Metallurgy"))
(advance-type miniaturization (name "Miniaturization"))
(advance-type mobile-warfare (name "Mobile Warfare"))
(advance-type monarchy (name "Monarchy"))
(advance-type monotheism (name "Monotheism"))
(advance-type mysticism (name "Mysticism"))
(advance-type navigation (name "Navigation"))
(advance-type nuclear-fission (name "Nuclear Fission"))
(advance-type nuclear-power (name "Nuclear Power"))
(advance-type philosophy (name "Philosophy"))
(advance-type physics (name "Physics"))
(advance-type plastics (name "Plastics"))
(advance-type polytheism (name "Polytheism"))
(advance-type pottery (name "Pottery"))
(advance-type radio (name "Radio"))
(advance-type railroad (name "Railroad"))
(advance-type recycling (name "Recycling"))
(advance-type refining (name "Refining"))
(advance-type refrigeration (name "Refrigeration"))
(advance-type republic (name "Republic"))
(advance-type robotics (name "Robotics"))
(advance-type rocketry (name "Rocketry"))
(advance-type sanitation (name "Sanitation"))
(advance-type seafaring (name "Seafaring"))
(advance-type space-flight (name "Space Flight"))
(advance-type stealth (name "Stealth"))
(advance-type steam-engine (name "Steam Engine"))
(advance-type steel (name "Steel"))
(advance-type superconductor (name "Superconductor"))
(advance-type tactics (name "Tactics"))
(advance-type theology (name "Theology"))
(advance-type theory-of-gravity (name "Theory of Gravity"))
(advance-type trade (name "Trade"))
(advance-type the-university (name "The University"))
(advance-type warrior-code (name "Warrior Code"))
(advance-type wheel (name "The Wheel"))
(advance-type writing (name "Writing"))


;;; Terrain types.

;; We represent special resource types as distinct types of terrain.

(terrain-type ocean (image-name "civ-ocean")
  (help "deep ocean"))
(terrain-type ocean/fish (image-name "civ-fish")
  (help "ocean with extra food and trade goods"))
(terrain-type ocean/whales (image-name "civ-whales")
  (help "ocean with extra food and trade goods"))
(terrain-type swamp (image-name "civ-swamp")
  (help "standing water and dense undergrowth"))
(terrain-type peat-swamp (image-name "civ-peat")
  (help "standing water and dense undergrowth"))
(terrain-type spice-swamp (image-name "civ-spice")
  (help "standing water and dense undergrowth"))
(terrain-type desert (image-name "civ-desert")
  (help "dry and sandy or rocky terrain"))
(terrain-type oasis (image-name "civ-oasis")
  (help "desert with extra food and shields"))
(terrain-type oil-desert (image-name "civ-oil-d")
  (help "desert with extra shields"))
(terrain-type plains (image-name "civ-plains")
  (help "flat or rolling countryside or steppe"))
(terrain-type buffalo-plains (image-name "civ-buffalo")
  (help "flat or rolling countryside or steppe"))
(terrain-type wheat-plains (image-name "civ-wheat")
  (help "flat or rolling countryside or steppe"))
(terrain-type grassland (image-name "civ-grassland"))
(terrain-type lush-grassland (image-name "civ-lush"))
(terrain-type forest (image-name "civ-forest")
  (help "dense forest"))
(terrain-type pheasant-forest (image-name "civ-pheasant")
  (help "dense forest"))
(terrain-type silk-forest (image-name "civ-silk")
  (help "dense forest"))
(terrain-type jungle (image-name "civ-jungle"))
(terrain-type fruit-jungle (image-name "civ-fruit"))
(terrain-type gems-jungle (image-name "civ-gems"))
(terrain-type hills (image-name "civ-hills")
  (help "moderately varying terrain"))
(terrain-type coal-hills (image-name "civ-coal")
  (help "moderately varying terrain"))
(terrain-type wine-hills (image-name "civ-wine")
  (help "moderately varying terrain"))
(terrain-type mountains (image-name "civ-mountains")
  (help "high elevation and/or rugged terrain"))
(terrain-type gold-mountains (image-name "civ-gold")
  (help "high elevation and/or rugged terrain"))
(terrain-type iron-mountains (image-name "civ-iron")
  (help "high elevation and/or rugged terrain"))
(terrain-type tundra (image-name "civ-tundra"))
(terrain-type furs-tundra (image-name "civ-furs"))
(terrain-type musk-ox-tundra (image-name "civ-musk-ox"))
(terrain-type glacier (image-name "civ-glacier")
  (help "permanent glacier fields"))
(terrain-type ivory-glacier (image-name "civ-ivory")
  (help "permanent glacier fields with ivory"))
(terrain-type oil-glacier (image-name "civ-oil-g")
  (help "permanent glacier fields with oil"))

(terrain-type river (image-name "river-conn")
  (subtype connection) (subtype-x river-x))

;; Road comes after river so it gets drawn over river by default.

(terrain-type road (image-name "dirt-road")
  (subtype connection) (subtype-x road-x))

(terrain-type rail-line (image-name "railroad") (char "R")
  (subtype connection) (subtype-x road-x))

;;; Definitions.

(define units (
  aegis-cruiser
  alpine-troops
  archers
  armor
  artillery
  battleship
  bomber
  cannon
  caravan
  caravel
  carrier
  catapult
  cavalry
  chariot
  cruise-missile
  cruiser
  crusaders
  destroyer
  diplomat
  dragoons
  elephants
  engineers
  explorers
  fanatics
  fighter
  freight
  frigate
  galleon
  helicopter
  horsemen
  howitzer
  ironclad
  knights
  legion
  marines
  mech-infantry
  musketeer
  nuclear-missile
  paratroopers
  partisans
  phalanx
  pikemen
  riflemen
  settlers
  spy
  stealth-bomber
  stealth-fighter
  submarine
  transport
  trireme
  warriors
  ))

(define unarmed (
  caravan
  diplomat
  engineers
  explorers
  freight
  galleon
  settlers
  spy
  transport
))

(define improvements (
  airport
  aqueduct
  bank
  barracks
  capitalization
  cathedral
  city-walls
  coastal-fortress
  colosseum
  courthouse
  factory
  granary
  harbor
  hydro-plant
  library
  mfg-plant
  marketplace
  mass-transit
  nuclear-plant
  offshore-platform
  palace
  police-station
  port-facility
  power-plant
  recycling-center
  research-lab
  sam-missile-battery
  sdi-defense
  sewer-system
  solar-plant
  ss-component
  ss-module
  ss-structural
  stock-exchange
  superhighways
  supermarket
  temple
  university
  ))

(define wonders (
  adam-smith-company
  apollo-program
  colossus
  copernicus-observatory
  cure-for-cancer
  darwins-voyage
  eiffel-tower
  great-library
  great-wall
  hanging-gardens
  hoover-dam
  isaac-newtons-college
  js-bachs-cathedral
  king-richards-crusade
  leonardos-workshop
  lighthouse
  magellans-expedition
  manhattan-project
  marco-polos-embassy
  michelangelos-chapel
  oracle
  pyramids
  seti-program
  shakespeares-theatre
  statue-of-liberty
  sun-tzus-war-academy
  united-nations
  womens-suffrage
  ))

(define ground (
  alpine-troops
  archers
  armor
  artillery
  cannon
  caravan
  catapult
  cavalry
  chariot
  crusaders
  diplomat
  dragoons
  elephants
  engineers
  explorers
  fanatics
  freight
  horsemen
  howitzer
  knights
  legion
  marines
  mech-infantry
  musketeer
  paratroopers
  partisans
  phalanx
  pikemen
  riflemen
  settlers
  spy
  warriors
  ))

(define aircraft (
  bomber
  cruise-missile
  fighter
  helicopter
  nuclear-missile
  stealth-bomber
  stealth-fighter
  ))

(define naval (
  aegis-cruiser
  battleship
  caravel
  carrier
  cruiser
  destroyer
  frigate
  galleon
  ironclad
  submarine
  transport
  trireme
  ))

(define products (food shields arrows))

(define water (
  ocean
  ocean/fish
  ocean/whales
  ))

(define land (
  swamp
  peat-swamp
  spice-swamp
  plains
  buffalo-plains
  wheat-plains
  grassland
  lush-grassland
  forest
  pheasant-forest
  silk-forest
  jungle
  fruit-jungle
  gems-jungle
  desert
  oasis
  oil-desert
  hills
  coal-hills
  wine-hills
  mountains
  gold-mountains
  iron-mountains
  tundra
  furs-tundra
  musk-ox-tundra
  glacier
  ivory-glacier
  oil-glacier
  ))

(define cell-t* (append water land))

(define basic-t* (ocean swamp desert plains grassland forest jungle hills mountains tundra glacier))

(add water liquid true)

(table drawable-terrain
  (t* t* true)
  (river water false)
  (river (glacier ivory-glacier oil-glacier) false)
  )

;;; River generation.

;; Civ-useful rivers are the large and broad sort, so they "start" in
;; open areas usually.

(add (swamp plains grassland forest jungle) river-chance 10.00)

(set edge-terrain glacier)

;;; Some defns for the fractal percentile generator.

(set alt-blob-density 20000)
(set alt-blob-height 1000)
(set alt-blob-size 60)
(set alt-smoothing 2)
(set wet-blob-density 4000)
(set wet-blob-size 60)
(set wet-smoothing 0)

(add basic-t* alt-percentile-min (  0  40  40  40  40  40  40  80  80  95  95))
(add basic-t* alt-percentile-max ( 40  50  80  80  80  80  80  90  95 100 100))
(add basic-t* wet-percentile-min (  0  80   0  15  45  70  85   0   0   0  50))
(add basic-t* wet-percentile-max (100 100  15  45  70  85 100  50 100  50 100))

(table terrain-density
  (ocean (ocean/fish ocean/whales) 500)
  (swamp (peat-swamp spice-swamp) 1000)
  (desert (oasis oil-desert) 1000)
  (plains (buffalo-plains wheat-plains) 1000)
  (grassland lush-grassland 5000)
  (forest (pheasant-forest silk-forest) 1000)
  (jungle (fruit-jungle gems-jungle) 1000)
  (hills (coal-hills wine-hills) 1000)
  (mountains (gold-mountains iron-mountains) 1000)
  (tundra (furs-tundra musk-ox-tundra) 1000)
  (glacier (ivory-glacier oil-glacier) 1000)
  )

(table vanishes-on
  (u* t* false)
  (ground water true)
  (city water true)
  )

;;; Setup.

(set sides-min 4)
(set sides-max 7)

(add settlers start-with 1)

(table favored-terrain
  (settlers land 100)
  )

(table independent-density
  ;; Don't put villages in special resource locations.
  (village basic-t* 400)
  (village (desert mountains tundra) 100)
  (village (ocean glacier) 0)
  )

(set country-separation-min 21)
(set country-separation-max 60)
(set country-radius-min 5)

(add city already-seen 1)

(add taxes initial-treasury 50)

;; Start the game in 4000 BC.

(set initial-date "-4000")

;; (The intervals correspond to a game of medium difficulty.)

(set calendar '(usual
  (  0  60 year 50)  ; 3000 years
  ( 60 100 year 25)  ; 1000 years
  (100 150 year 20)  ; 1000 years
  (150 200 year 10)  ; 500 years
  (200 250 year  5)  ; 250 years
  (250 325 year  2)  ; 150 years
  (325 450 year  1)  ; 125 years
  ))

;; Make the game end in 2025 AD.

(set last-turn 451)

;; Each wonder must be unique.
;; (This is not quite accurate - in real CivII, multiple incomplete
;; wonders are allowed)

(add wonders type-in-game-max 1)

;;; Unit-unit capacities.

(table unit-size-as-occupant
  (u* u* 9999)
  ;; Cities can hold lots of units.
  (units city 1)
  (ground (caravel frigate galleon transport trireme) 1)
  (aircraft carrier 1)
  ((cruise-missile nuclear-missile) submarine 1)
  )

(add (caravel frigate galleon transport trireme) capacity (3 2 4 8 2))
(add (carrier submarine) capacity 8)
(add city capacity 100)

(table unit-capacity-x
  ;; Only one of each sort of improvement or wonder.
  (city improvements 1)
  (city wonders 1)
  )

;; Certain improvements raise the limit on city size.

(table size-limit-without-occupant
  (city aqueduct 8)
  (city sewer-system 12)
  )

;;; Unit-terrain capacities.

;; Limit units to 4 in one cell.
;; Cities cover the entire cell, however.

(table unit-size-in-terrain
  (units t* 4)
  (city t* 16)
  (airbase t* 4)
  (fortress t* 16)
  (village t* 16)
  )

(add t* capacity 16)

;  aircrafts can always overfly cities

(table terrain-capacity-x
  (u* t* 0)
  (aircraft cell-t* 2)
  )

;;; Unit-material capacities.

(table unit-storage-x
  (city food 999)
  (city shields 999)
  (city luxuries 999)
  )

(table gives-to-treasury
  (city (taxes science) true)
  )

;;; Vision.

;; (is this correct?)
(add city see-always 1)

;; Airplanes can see farther.

(add aircraft vision-range 2)
(add (cruise-missile nuclear-missile) vision-range 1)

;; Cities can see everything in their radius.

(add city vision-range 2)

;;; Material production and consumption.

;; City uses adjacent and next-to-adjacent cells.

(add city reach 2)

;; This is for city use only, cell material layers should be empty.

(table production-from-terrain
  (t* m* 0)
  (ocean products (1 0 2))
  (ocean/fish products (3 0 2))
  (ocean/whales products (2 2 3))
  (swamp products (1 0 0))
  (peat-swamp products (1 4 0))
  (spice-swamp products (3 0 4))
  (desert products (0 1 0))
  (oasis products (3 1 0))
  (oil-desert products (0 4 0))
  (plains products (1 1 0))
  (buffalo-plains products (1 3 0))
  (wheat-plains products (3 1 0))
  (grassland products (2 0 0))
  (lush-grassland products (2 1 0))
  (forest products (1 2 0))
  (pheasant-forest products (3 2 0))
  (silk-forest products (1 2 3))
  (jungle products (1 0 0))
  (fruit-jungle products (4 0 1))
  (gems-jungle products (1 0 4))
  (hills products (1 0 0))
  (coal-hills products (1 2 0))
  (wine-hills products (1 0 4))
  (mountains products (0 1 0))
  (gold-mountains products (0 1 6))
  (iron-mountains products (0 4 0))
  (tundra products (1 0 0))
  (furs-tundra products (2 0 3))
  (musk-ox-tundra products (3 1 0))
  (glacier products (0 0 0))
  (ivory-glacier products (1 1 4))
  (oil-glacier products (0 4 0))
  (river products (0 0 1))
  )

(table unit-consumption-per-size
  (city food 2)
  )

(table unit-consumption-to-grow
  (city food 10)
  )

(table consumption-per-cp
  (u* shields 1)
  (city shields 0)
  )

(table conversion
  (arrows (taxes luxuries science) 100)
  )

(table conversion-default
  (arrows (taxes luxuries science) (40 0 60))
  )

;;; Unit production and repair.

;; Cities can create and build everything but cities
;; Settlers can only create and build cities

(table can-create
  (u* u* 0)
  (settlers city 1)
  (engineers city 1)
  (city units 1)
  (city improvements 1)
  (city wonders 1)
  )

(table can-build
  (u* u* 0)
  (settlers city 1)
  (engineers city 1)
  (city units 1)
  (city improvements 1)
  (city wonders 1)
  )

(table acp-to-create
  (u* u* 0)
  (settlers city 1)
  (engineers city 1)
  (city units 1)
  (city improvements 1)
  (city wonders 1)
  )

(table acp-to-build
  (u* u* 0)
  (settlers city 1)
  (engineers city 1)
  (city units 1)
  (city improvements 1)
  (city wonders 1)
  )

(table cp-per-build
  (u* u* 1)
  )

(table cp-on-creation
  (u* u* 1)
  )

(add aegis-cruiser cp 100)
(add alpine-troops cp 50)
(add archers cp 30)
(add armor cp 80)
(add artillery cp 50)
(add battleship cp 160)
(add bomber cp 120)
(add cannon cp 40)
(add caravan cp 50)
(add caravel cp 40)
(add carrier cp 160)
(add cavalry cp 60)
(add catapult cp 40)
(add chariot cp 30)
(add cruise-missile cp 60)
(add cruiser cp 80)
(add crusaders cp 40)
(add destroyer cp 60)
(add diplomat cp 30)
(add dragoons cp 50)
(add elephants cp 40)
(add engineers cp 40)
(add explorers cp 30)
(add fanatics cp 20)
(add fighter cp 60)
(add freight cp 50)
(add frigate cp 50)
(add galleon cp 40)
(add helicopter cp 120)
(add horsemen cp 20)
(add howitzer cp 70)
(add ironclad cp 60)
(add knights cp 40)
(add legion cp 40)
(add marines cp 60)
(add mech-infantry cp 50)
(add musketeer cp 30)
(add nuclear-missile cp 160)
(add paratroopers cp 60)
(add partisans cp 50)
(add phalanx cp 20)
(add pikemen cp 20)
(add riflemen cp 40)
(add settlers cp 40)
(add spy cp 30)
(add stealth-bomber cp 160)
(add stealth-fighter cp 80)
(add submarine cp 60)
(add transport cp 50)
(add trireme cp 40)
(add warriors cp 10)
;; improvements
(add airport cp 160)
(add aqueduct cp 80)
(add bank cp 120)
(add barracks cp 40)
(add capitalization cp 600)
(add cathedral cp 120)
(add city-walls cp 80)
(add coastal-fortress cp 80)
(add colosseum cp 100)
(add courthouse cp 80)
(add factory cp 200)
(add granary cp 60)
(add harbor cp 60)
(add hydro-plant cp 240)
(add library cp 80)
(add mfg-plant cp 320)
(add marketplace cp 80)
(add mass-transit cp 160)
(add nuclear-plant cp 160)
(add offshore-platform cp 160)
(add palace cp 100)
(add police-station cp 60)
(add port-facility cp 80)
(add power-plant cp 160)
(add recycling-center cp 200)
(add research-lab cp 160)
(add sam-missile-battery cp 160)
(add sdi-defense cp 200)
(add sewer-system cp 120)
(add solar-plant cp 320)
(add ss-component cp 160)
(add ss-module cp 320)
(add ss-structural cp 80)
(add stock-exchange cp 160)
(add superhighways cp 200)
(add supermarket cp 80)
(add temple cp 40)
(add university cp 160)
;; wonders
(add adam-smith-company cp 400)
(add apollo-program cp 600)
(add colossus cp 200)
(add copernicus-observatory cp 300)
(add cure-for-cancer cp 600)
(add darwins-voyage cp 400)
(add eiffel-tower cp 300)
(add great-library cp 300)
(add great-wall cp 300)
(add hanging-gardens cp 200)
(add hoover-dam cp 600)
(add isaac-newtons-college cp 400)
(add js-bachs-cathedral cp 400)
(add king-richards-crusade cp 300)
(add leonardos-workshop cp 400)
(add lighthouse cp 200)
(add magellans-expedition cp 400)
(add manhattan-project cp 600)
(add marco-polos-embassy cp 200)
(add michelangelos-chapel cp 400)
(add oracle cp 300)
(add pyramids cp 200)
(add seti-program cp 600)
(add shakespeares-theatre cp 300)
(add statue-of-liberty cp 400)
(add sun-tzus-war-academy cp 300)
(add united-nations cp 600)
(add womens-suffrage cp 600)

(table advance-needed-to-build
  (u* a* false)
  ;; units
  (aegis-cruiser rocketry true)
  (alpine-troops tactics true)
  (archers warrior-code true)
  (armor mobile-warfare true)
  (artillery machine-tools true)
  (battleship automobile true)
  (bomber advanced-flight true)
  (cannon metallurgy true)
  (caravan trade true)
  (caravel navigation true)
  (carrier advanced-flight true)
  (cavalry tactics true)
  (catapult mathematics true)
  (chariot wheel true)
  (cruise-missile rocketry true)
  (cruiser steel true)
  (crusaders monotheism true)
  (destroyer electricity true)
  (diplomat writing true)
  (dragoons leadership true)
  (elephants polytheism true)
  (engineers explosives true)
  (explorers seafaring true)
  (fanatics fundamentalism true)
  (fighter flight true)
  (freight corporation true)
  (frigate magnetism true)
  (galleon magnetism true)
  (helicopter combined-arms true)
  (horsemen horseback-riding true)
  (howitzer robotics true)
  (ironclad steam-engine true)
  (knights chivalry true)
  (legion iron-working true)
  (marines amphibious-warfare true)
  (mech-infantry labor-union true)
  (musketeer gunpowder true)
  (nuclear-missile rocketry true)
  (paratroopers combined-arms true)
  (partisans guerrilla-warfare true)
  (phalanx bronze-working true)
  (pikemen feudalism true)
  (riflemen conscription true)
  ; no prereqs for settlers
  (spy espionage true)
  (stealth-bomber stealth true)
  (stealth-fighter stealth true)
  (submarine combustion true)
  (transport industrialization true)
  (trireme map-making true)
  ; no prereqs for warriors
  ;; improvements
  (airport radio true)
  (aqueduct construction true)
  (bank banking true)
  ; no prereqs for barracks
  (capitalization corporation true)
  (cathedral monotheism true)
  (city-walls masonry true)
  (coastal-fortress metallurgy true)
  (colosseum construction true)
  (courthouse code-of-laws true)
  (factory industrialization true)
  (granary pottery true)
  (harbor seafaring true)
  (hydro-plant electronics true)
  (library writing true)
  (mfg-plant robotics true)
  (marketplace currency true)
  (mass-transit mass-production true)
  (nuclear-plant nuclear-power true)
  (offshore-platform miniaturization true)
  (palace masonry true)
  (police-station communism true)
  (port-facility amphibious-warfare true)
  (power-plant refining true)
  (recycling-center recycling true)
  (research-lab computers true)
  (sam-missile-battery rocketry true)
  (sdi-defense the-laser true)
  (sewer-system sanitation true)
  (solar-plant environmentalism true)
  (ss-component plastics true)
  (ss-module superconductor true)
  (ss-structural space-flight true)
  (stock-exchange economics true)
  (superhighways automobile true)
  (supermarket refrigeration true)
  (temple ceremonial-burial true)
  (university the-university true)
  ;; wonders
  (adam-smith-company economics true)
  (apollo-program space-flight true)
  (colossus bronze-working true)
  (copernicus-observatory astronomy true)
  (cure-for-cancer genetic-engineering true)
  (darwins-voyage railroad true)
  (eiffel-tower steam-engine true)
  (great-library literacy true)
  (great-wall masonry true)
  (hanging-gardens pottery true)
  (hoover-dam electronics true)
  (isaac-newtons-college theory-of-gravity true)
  (js-bachs-cathedral theology true)
  (king-richards-crusade engineering true)
  (leonardos-workshop invention true)
  (lighthouse map-making true)
  (magellans-expedition navigation true)
  (manhattan-project nuclear-fission true)
  (marco-polos-embassy trade true)
  (michelangelos-chapel monotheism true)
  (oracle mysticism true)
  (pyramids masonry true)
  (seti-program computers true)
  (shakespeares-theatre medicine true)
  (statue-of-liberty democracy true)
  (sun-tzus-war-academy feudalism true)
  (united-nations communism true)
  (womens-suffrage industrialization true)
  ;; others
  (airbase radio true)
  (fortress construction true)
  )

; is this necessary?
(table occupant-can-construct
  (u* u* true)
  )

(add archers obsolete gunpowder)
(add artillery obsolete robotics)
(add bomber obsolete stealth)
(add cannon obsolete machine-tools)
(add caravan obsolete corporation)
(add caravel obsolete magnetism)
(add catapult obsolete metallurgy)
(add cavalry obsolete mobile-warfare)
(add chariot obsolete polytheism)
(add cruiser obsolete superconductor)
(add crusaders obsolete leadership)
(add diplomat obsolete espionage)
(add dragoons obsolete tactics)
(add elephants obsolete monotheism)
(add explorers obsolete guerrilla-warfare)
(add fighter obsolete stealth)
(add frigate obsolete electricity)
(add galleon obsolete industrialization)
(add horsemen obsolete chivalry)
(add ironclad obsolete electricity)
(add knights obsolete leadership)
(add legion obsolete gunpowder)
(add musketeer obsolete conscription)
(add phalanx obsolete feudalism)
(add pikemen obsolete gunpowder)
(add settlers obsolete explosives)
(add trireme obsolete navigation)
(add warriors obsolete feudalism)

;;; Research.

(set side-can-research true)

;; Set every scientific advance to cost the same.

(add a* rp 10)

;; One research point costs one science.

(table advance-consumption-per-rp
  (a* m* 0)
  (a* science 1)
  )

(table advance-needed-to-research
  (a* a* false)
  (advanced-flight (machine-tools radio) true)
  ; no prereqs for alphabet
  (amphibious-warfare (tactics navigation) true)
  (astronomy (mysticism mathematics) true)
  (atomic-theory (theory-of-gravity physics) true)
  (automobile (combustion steel) true)
  (banking (trade republic) true)
  (bridge-building (iron-working construction) true)
  ; no prereqs for bronze-working
  ; no prereqs for ceremonial-burial
  (chemistry (the-university medicine) true)
  (chivalry (feudalism horseback-riding) true)
  (code-of-laws (alphabet) true)
  (combined-arms (mobile-warfare advanced-flight) true)
  (combustion (refining explosives) true)
  (communism (philosophy industrialization) true)
  (computers (miniaturization mass-production) true)
  (conscription (democracy metallurgy) true)
  (construction (masonry currency) true)
  (corporation (economics industrialization) true)
  (currency (bronze-working) true)
  (democracy (banking invention) true)
  (economics (banking the-university) true)
  (electricity (metallurgy magnetism) true)
  (electronics (electricity corporation) true)
  (engineering (wheel construction) true)
  (environmentalism (recycling space-flight) true)
  (espionage (democracy communism) true)
  (explosives (gunpowder chemistry) true)
  (feudalism (warrior-code monarchy) true)
  (flight (combustion theory-of-gravity) true)
  (fundamentalism (monotheism conscription) true)
  (fusion-power (nuclear-power superconductor) true)
  (future-tech (fusion-power recycling) true)
  (genetic-engineering (medicine corporation) true)
  (guerrilla-warfare (communism tactics) true)
  (gunpowder (invention iron-working) true)
  ; no prereqs for horseback-riding
  (industrialization (railroad banking) true)
  (invention (engineering literacy) true)
  (iron-working (bronze-working warrior-code) true)
  (labor-union (mass-production guerrilla-warfare) true)
  (the-laser (nuclear-power mass-production) true)
  (leadership (chivalry gunpowder) true)
  (literacy (writing code-of-laws) true)
  (machine-tools (steel tactics) true)
  (magnetism (iron-working physics) true)
  (map-making (alphabet) true)
  ; no prereqs for masonry
  (mass-production (automobile corporation) true)
  (mathematics (alphabet masonry) true)
  (medicine (philosophy trade) true)
  (metallurgy (gunpowder the-university) true)
  (miniaturization (machine-tools electronics) true)
  (mobile-warfare (automobile tactics) true)
  (monarchy (ceremonial-burial code-of-laws) true)
  (monotheism (philosophy polytheism) true)
  (mysticism (ceremonial-burial) true)
  (navigation (seafaring astronomy) true)
  (nuclear-fission (mass-production atomic-theory) true)
  (nuclear-power (nuclear-fission electronics) true)
  (philosophy (mysticism literacy) true)
  (physics (navigation literacy) true)
  (plastics (refining space-flight) true)
  (polytheism (ceremonial-burial horseback-riding) true)
  (radio (flight electricity) true)
  (railroad (steam-engine bridge-building) true)
  (recycling (mass-production democracy) true)
  (refining (chemistry corporation) true)
  (refrigeration (sanitation electricity) true)
  (republic (code-of-laws literacy) true)
  (robotics (plastics computers) true)
  (rocketry (advanced-flight electronics) true)
  (sanitation (medicine engineering) true)
  (seafaring (map-making pottery) true)
  (space-flight (computers rocketry) true)
  (stealth (superconductor robotics) true)
  (steam-engine (physics invention) true)
  (steel (metallurgy industrialization) true)
  (superconductor (the-laser plastics) true)
  (tactics (leadership conscription) true)
  (the-university (mathematics philosophy) true)
  (theology (monotheism feudalism) true)
  (theory-of-gravity (astronomy the-university) true)
  (trade (currency code-of-laws) true)
  ; no prereqs for warrior code
  (wheel (horseback-riding) true)
  (writing (alphabet) true)
  )

;;; Movement.

;; Don't allow negative acp.

(add u* acp-min 0)

(add u* free-acp 1)

;; Cities and improvements etc don't move, but regular units do.

(add city speed 0)
(add improvements speed 0)
(add wonders speed 0)

(add units speed 100)

;; Any unit gets enough free moves to make at least one move
;; into any cell.

(add u* free-mp 8)

(table mp-to-enter-terrain
  (u* t* 99)
  (ground (desert oasis oil-desert plains buffalo-plains wheat-plains grassland lush-grassland tundra furs-tundra musk-ox-tundra) 3)
  (ground (swamp peat-swamp spice-swamp forest pheasant-forest silk-forest jungle fruit-jungle gems-jungle hills coal-hills wine-hills glacier ivory-glacier oil-glacier) 6)
  (ground (mountains gold-mountains iron-mountains) 9)
  ;; Exceptionally mobile units.
  ((alpine-troops explorers partisans) land 3)
  (aircraft t* 1)
  (naval water 1)
  (ground (river road rail-line) 0)
  )

(table mp-to-traverse
  (u* (river road rail-line) 99)
;;  (ground river 1)	;	This caused ground units to follow rivers into the sea and then drown.	
  (ground road 1)
  (ground rail-line 1)
  )

(table zoc-range
  (ground ground 1)
  (city ground 1)
  ;; Units that ignore zones of control.
  (ground (caravan diplomat engineers freight partisans spy) 0)
  ((caravan diplomat engineers freight partisans spy) ground 0)
  (city (caravan diplomat engineers freight partisans spy) 0)
  ((caravan diplomat engineers freight partisans spy) city 0)
  )

;; Can only enter/leave ZOC, not move around in.

(table mp-to-traverse-zoc
  (ground ground -1)
  (ground city -1)
  )

;; This is tricky; if the entry cost is the default of -1 (which
;; is so units on different sides can't stack in the same cell),
;; then it applies to the range of 1 defined above, then keeps
;; units from getting adjacent.  Instead, we rely on the traversal
;; table to prevent a unit from traversing from a cell adjacent
;; to an enemy to the enemy's cell itself.

(table mp-to-enter-zoc
  (ground ground 0)
  (ground city 0)
  ;; Units that ignore ZOC still need to disallow cell-sharing.
  (ground (caravan diplomat engineers freight partisans spy) -1)
  ((caravan diplomat engineers freight partisans spy) ground -1)
  (city (caravan diplomat engineers freight partisans spy) -1)
  ((caravan diplomat engineers freight partisans spy) city -1)
  )

;;; Combat.

;; Use the Civ-like combat model.

(set combat-model 1)

;; Any mobile unit can be a veteran (should be only combat units?)

(add units cxp-max 1)

(table damage
  (units units 1)
  ((aegis-cruiser artillery battleship bomber cruiser fighter) units 2)
  ((helicopter howitzer stealth-bomber stealth-fighter submarine) units 2)
  (cruise-missile units 3)
  (carrier units 4)
  ;; Limits on naval attacks on shore.
  (naval ground 1)
  ((submarine transport) ground 0)
  ;; No damage/hp to city stuff.
  (u* city 0)
  (u* improvements 0)
  (u* wonders 0)
  ;; (why is this necessary?)
  (units village 1)
  )

;; (should only be military units?)

(table hit-chance
	(u* u* 0)
	(units units 100)
	(unarmed u* 0)
	(units village 100) ; Also unarmed units should be able to go after villages.
  )

;;	The protection table has no effect on model 1 combat, but is still important for the capture chance.
;;	By setting protection (units places 0) all occupants must be destroyed before a city is captured.
;;	Without such protection, the city is captured when the first occupant is destroyed, and all other
;;	occupants perish. 

(table protection					;	100 (no protection) by default!
	(units city 0)				;	Cities can't be captured as long as they have defenders.
)

(table transport-affects-defense
	(city units 150)				;	Cities increase occs def value by 50%
)

(table neighbour-affects-defense
	(city-walls units 200)		;	City walls increase occs def value by 100%
)

;; Settlers entirely disappear into the city they build (or capture?)

(table constructor-absorbed-by (settlers city true) (engineers city true))
(table hp-to-garrison (settlers city 20) (engineers city 20))

;; Any unit can capture an undefended city.

(table capture-chance
	(units city 0)
	(ground city 100)
)

;; Things that can happen when you overrun a village.

(add village encounter-result (
  '(unit warriors "The village inhabitants join you as mercenary warriors.")
  '(unit (warriors 0) "You encounter Barbarian warriors!")
  '(unit settlers "The village inhabitants become settlers.")
  '(unit city "The village inhabitants decide to join your civilization.")
  '(vanish nil "The village is deserted.")
  ))

;;; Other actions.

;; Disbanding is possible, and always takes just one turn.

(add units hp-per-disband 100)

;; Terrain alteration.

(table acp-to-add-terrain
  ((settlers engineers) (road rail-line) 1)
  )

;;; Backdrop.

;; Certain improvements restore units to full health immediately.

(table auto-repair
  (airport aircraft 100.00)
  (barracks ground 100.00)
  (port-facility naval 100.00)
  )

;;; Scoring.

(scorekeeper (do last-side-wins))

(set advantage-min 1)
(set advantage-default 1)
(set advantage-max 5)

;; Settlers have a tiny bit of value because they can build cities, but
;; cities/population are the real measure of success.

(add settlers point-value 1)
(add engineers point-value 1)
(add city point-value 10)

;;; Doctrine.

; (add city ai-peace-garrison 1)		;	Minimal number of defenders assigned to a city.
(add city ai-war-garrison 2)		;	Minimal number of defenders assigned to a city.

(doctrine settler-doctrine
  ;; Settlers/engineers only build one thing at a time.
  (construction-run (city 1))
  )

(doctrine city-doctrine
  ;; Cities can only have one of each of these, so don't try for more.
  (construction-run (improvements 1) (wonders 1))
  )

(side-defaults
  (doctrines ((settlers engineers) settler-doctrine) (city city-doctrine))
  )

(add settlers colonizer true)
(add engineers colonizer true)

(add city ai-minimal-size-goal 8)

; Somebody must run the Barbarians!

(set indepside-has-ai true)

;;; Naming.

(namer egyptian-names (random
  "Thebes" "Memphis" "Oryx" "Heliopolis" "Gaza"
  "Alexandria" "Byblos" "Cairo" "Coptos" "Edfu"
  "Pithom" "Busiris" "Athribis" "Mendes" "Tanis" "Abydos"
  ))

(namer aztec-names (random
  "Tenochtitlan" "Chiauhtia" "Chapultepec" "Coatepec" "Ayotzinco"
  "Itzapalapa" "Iztapam" "Mitxcoac" "Tacubaya" "Tecamac"
  "Tepezinco" "Ticoman" "Tlaxcala" "Xaltocan" "Xicalango" "Zumpanco"
  ))

(namer roman-names (random
  "Rome" "Caesarea" "Carthago" "Nicopolis" "Byzantium"
  "Brundisium" "Syracuse" "Antioch" "Palmyra" "Cyrene"
  "Gordion" "Tyrus" "Jerusalem" "Seleucia" "Ravenna" "Artaxata"
  ))

(namer greek-names (random
  "Athens" "Sparta" "Corinth" "Delphi" "Eretria"
  "Pharsalos" "Argos" "Mycenae" "Herakleia" "Antioch"
  "Ephesos" "Rhodes" "Knossos" "Troy" "Pergamon" "Miletos"
  ))

(namer mongolian-names (random
  "Samarkand" "Bokhara" "Nishapur" "Karakorum" "Kashgar"
  "Tabriz" "Aleppo" "Kabul" "Ormuz" "Basra"
  "Khanbalyk" "Khorasan" "Shangtu" "Kazan" "Quinsay" "Kerman"
  ))

(namer indian-names (random
  "Delhi" "Bombay" "Madras" "Bangalore" "Calcutta"
  "Lahore" "Karachi" "Kolhapur" "Jaipur" "Hyderabad"
  "Bengal" "Chittagong" "Punjab" "Dacca" "Indus" "Ganges"
  ))

(namer chinese-names (random
  "Peking" "Shanghai" "Canton" "Nanking" "Tsingtao"
  "Hangchow" "Tientsin" "Tatung" "Macao" "Anyang"
  "Shantung" "Chinan" "Kaifeng" "Ningpo" "Paoting" "Yangchow"
  ))

(namer french-names (random
  "Paris" "Orleans" "Lyons" "Tours" "Chartres"
  "Bordeaux" "Rouen" "Avignon" "Marseilles" "Grenoble"
  "Dijon" "Amiens" "Cherbourg" "Poitiers" "Toulouse" "Bayonne"
  ))

(namer german-names (random
  "Berlin" "Leipzig" "Hamburg" "Bremen" "Frankfurt"
  "Bonn" "Nuremberg" "Cologne" "Munich" "Hannover"
  "Munich" "Stuttgart" "Heidelberg" "Salzburg" "Konigsberg" "Brandenburg"
  ))

(namer english-names (random
  "London" "Coventry" "Birmingham" "Dover" "Nottingham"
  "York" "Liverpool" "Brighton" "Oxford" "Reading"
  "Exeter" "Cambridge" "Hastings" "Canterbury" "Banbury" "Newcastle"
  ))

(namer russian-names (random
  "Moscow" "Leningrad" "Kiev" "Minsk" "Smolensk"
  "Odessa" "Sevastopol" "Tblisi" "Yakutsk" "Vladivostok"
  "Novograd" "Krasnoyarsk" "Riga" "Rostov" "Astrakhan"
  ))

(namer zulu-names (random
  "Zimbabwe" "Ulundi" "Bapedi" "Hlobane" "Isandhlwana"
  "Intombe" "Mpondo" "Ngome" "Swazi" "Tugela" "Umtata"
  "Umfolozi" "Ibabanago" "Isipezi" "Amatikulu" "Zunguin"
  ))

(namer babylonian-names (random
  "Babylon" "Sumer" "Uruk" "Nineveh" "Ashur"
  "Ellipi" "Akkad" "Eridu" "Kish" "Nippur"
  "Shuruppak" "Zariqum" "Izibia" "Nimrud" "Arbela" "Zamua"
  ))

(namer american-names (random
  "Washington" "New York" "Boston" "Philadelphia" "Atlanta"
  "Chicago" "Buffalo" "St. Louis" "Detroit" "New Orleans"
  "Baltimore" "Denver" "Cincinatti" "Dallas" "Los Angeles" "Las Vegas"
  ))

(namer independent-names (random
  "Apollonia" "Aquileia" "Artaxata" "Berytus" "Bysanthium"
  "Caesarea" "Capua" "Corinthus" "Cyrene" "Durcotorum" "Dyrrachium"
  "Eburacum" "Ephesus" "Gazaca" "Ierusalem" "Leptis Magna" "Lugudunum"
  "Magontiacum" "Massilia" "Mediolanum" "Memphis" "Messana"
  "Narbo Martius" "Nicaea" "Nicomedia" "Nova Carthago" "Numantia"
  "Palmyra" "Pergamum" "Petra" "Ravenna" "Rhodos" "Sais" "Salamis"
  "Salonae" "Sinope" "Susa" "Syracusae" "Tarentum" "Tarraco" "Thebae"
  "Thessalonica" "Tingis" "Trapezus" "Tyrus" "Utica"
  ))

(add city namer "independent-names")

;;  Use short generic names if we run out of cities.

(include "ng-weird")
(set default-namer "short-generic-names")

;;; The possible sides.

(set side-library '(

  ((noun "Egyptian")
   (emblem-name "flag-egypt")  ; hmmm...
   (unit-namers (city "egyptian-names")))

  ((noun "Aztec")
   (emblem-name "flag-mexico")
   (unit-namers (city "aztec-names")))

  ((noun "Roman")
   (emblem-name "flag-italy")
   (unit-namers (city "roman-names")))

  ((plural-noun "Greek")
   (emblem-name "flag-greece")
   (unit-namers (city "greek-names")))

  ((noun "Mongol")
   (adjective "Mongolian")
   (emblem-name "flag-mongolia")
   (unit-namers (city "mongolian-names")))
  
  ((noun "Chinese")  
   (emblem-name "flag-china")
   (unit-namers (city "chinese-names")))

  ((noun "Indian")
   (emblem-name "flag-india")
   (unit-namers (city "indian-names")))
  
  ((noun "French")
   (emblem-name "flag-france")
   (unit-namers (city "french-names")))
  
  ((noun "German")
   (emblem-name "flag-germany")
   (unit-namers (city "german-names")))

  ((noun "Russian")
   (emblem-name "flag-russia")
   (unit-namers (city "russian-names")))
  
  ((noun "English")
   (emblem-name "flag-uk")  
   (unit-namers (city "english-names")))
  
  ((noun "Zulu")
   (emblem-name "flag-zimbabwe")
   (unit-namers (city "zulu-names")))
  
  ((noun "Babylonian")
   (emblem-name "flag-iraq")  
   (unit-namers (city "babylonian-names")))
  
  ((noun "American")
   (emblem-name "flag-usa")
   (unit-namers (city "american-names")))

  ))

; Give old indepside a civ-like name.

(side 0 (name "the Barbarians"))
(side 0 (adjective "Barbarian"))


;; Use the Trident images from Freeciv

(add aegis-cruiser image-name "trident-aegis-cruiser")
(add alpine-troops image-name "trident-alpine-troops")
(add archers image-name "trident-archers")
(add armor image-name "trident-armor")
(add artillery image-name "trident-artillery")
(add battleship image-name "trident-battleship")
(add bomber image-name "trident-bomber")
(add cannon image-name "trident-cannon")
(add caravan image-name "trident-caravan")
(add caravel image-name "trident-caravel")
(add carrier image-name "trident-carrier")
(add catapult image-name "trident-catapult")
(add cavalry image-name "trident-cavalry")
(add chariot image-name "trident-chariot")
(add cruise-missile image-name "trident-cruise-missile")
(add cruiser image-name "trident-cruiser")
(add crusaders image-name "trident-crusaders")
(add destroyer image-name "trident-destroyer")
(add diplomat image-name "trident-diplomat")
(add dragoons image-name "trident-dragoons")
(add elephants image-name "trident-elephants")
(add engineers image-name "trident-engineers")
(add explorers image-name "trident-explorers")
(add fanatics image-name "trident-fanatics")
(add fighter image-name "trident-fighter")
(add freight image-name "trident-freight")
(add frigate image-name "trident-frigate")
(add galleon image-name "trident-galleon")
(add helicopter image-name "trident-helicopter")
(add horsemen image-name "trident-horsemen")
(add howitzer image-name "trident-howitzer")
(add ironclad image-name "trident-ironclad")
(add knights image-name "trident-knights")
(add legion image-name "trident-legion")
(add marines image-name "trident-marines")
(add mech-infantry image-name "trident-mech-infantry")
(add musketeer image-name "trident-musketeer")
(add nuclear-missile image-name "trident-nuclear-missile")
(add paratroopers image-name "trident-paratroopers")
(add partisans image-name "trident-partisans")
(add phalanx image-name "trident-phalanx")
(add pikemen image-name "trident-pikemen")
(add riflemen image-name "trident-riflemen")
(add settlers image-name "trident-settlers")
(add spy image-name "trident-spy")
(add stealth-bomber image-name "trident-stealth-bomber")
(add stealth-fighter image-name "trident-stealth-fighter")
(add submarine image-name "trident-submarine")
(add transport image-name "trident-transport")
(add trireme image-name "trident-trireme")
(add warriors image-name "trident-warriors")
