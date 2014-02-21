(game-module "space-civ"
  (title "Space Civilization")
  (blurb "Advanced space-colonizing game by Lincoln Peters.")
  (instructions "Boldly go where no one has gone before!")
  (notes "This game is similar to Civilization, but on a galactic scale. Each side starts with one M-class (earthlike) planet and technology circa Earth 2000. The ideas used in the game come from many different sources (you might see quite a bit of Star Trek in it, for example).")
  (version "0.5")
  (variants
    (world-seen)
    (see-all)
    (sequential)
    (world-size (120 60 720))
    ("Last side wins" last-side-wins 
     "If set, the last civilization left in the game wins"
     true
     (true
       (scorekeeper (do last-side-wins))))
    ("Last alliance wins" last-alliance-wins
     "If set, the last group of allied civilizations left in the game wins"
     (true
       (scorekeeper (do last-alliance-wins))))
    ("More starters" more-starters
     "Give each side all of the essential units immediately"
     (true
       (add telescope start-with 1)
       (add probe start-with 6)
       (add shipyard start-with 1)
       (add antimatter-plant start-with 1)
       (add shuttle start-with 1)
       (add engineers start-with 2)))
    ("More inhabited worlds" more-inhabited-worlds
     "Generate more civilizations on inhabitable planets"
     (true

(table independent-density
  (civ-0 g-planet 60)
  (civ-0 h-planet 80)
  (civ-0 l-planet 240)
  (civ-0 m-planet 720)
  (civ-0 n-planet 480)

  (civ-1 g-planet 30)
  (civ-1 h-planet 45)
  (civ-1 l-planet 120)
  (civ-1 m-planet 360)
  (civ-1 n-planet 180)

  (civ-2 g-planet 4)
  (civ-2 h-planet 6)
  (civ-2 l-planet 8)
  (civ-2 m-planet 24)
  (civ-2 n-planet 12)

  (civ-3 m-planet 2)

)))


  )
)

;; Economy settings added by M. Skala, 2005/02/19; these settings seem to
;; make the game a lot more playable.
(set backdrop-model 1)
(set backdrop-ignore-doctrine true)

(include "ng-weird")

;; Terrain is set up to be a little too simple.  It's OK for testing, but not release.
(terrain-type vacuum (image-name "vacuum") (occurrence 100))
	;; Needs nebulae, and some way to make planets tend to be near
	;; stars.  Perhaps nebulae could be used as a coating type?
        ;; Also, perhaps if multiple maps becomes possible, there could be
        ;; individual maps for galactic, stellar, and planetary level activity?
        ;; We might need our own synthesis-method for star systems.

;; Unlike in most space games, planets are defined as terrain types (not unit types).
;; Planet pictures, however, are designed for units and don't render very nicely.
(terrain-type a-planet (image-name "gas-giant")
  (help "A-class world: gas giant that radiates heat"))
(terrain-type b-planet (image-name "gas-giant")
  (help "B-class world: non-radiant gas giant"))
(terrain-type c-planet (image-name "moon")
  (help "C-class world: metallic surface and unbrethable atmosphere"))
(terrain-type d-planet (image-name "moon")
  (help "D-class world: a large captured asteroid"))
(terrain-type e-planet (image-name "moon")
  (help "E-class world: highly volcanic"))
(terrain-type f-planet (image-name "moon")
  (help "F-class world: developing planet, mostly molten"))
(terrain-type g-planet (image-name "moon")
  (help "G-class world: habitable but very little water"))
(terrain-type h-planet (image-name "planet")
  (help "H-class world: has vegetation but unbrethable atmosphere"))
(terrain-type i-planet (image-name "moon")
  (help "I-class world: molten surface, very dense atmoshpere"))
(terrain-type j-planet (image-name "moon")
  (help "J-class world: small moon with no atmosphere"))
(terrain-type k-planet (image-name "moon")
  (help "K-class world: thin atmosphere, no water"))
(terrain-type l-planet (image-name "planet")
  (help "L-class world: inhabited but no vegetation"))
(terrain-type m-planet (image-name "planet")
  (help "M-class world: lots of water, complex ecosystem"))
(terrain-type n-planet (image-name "planet")
  (help "N-class world: covered by water, supports aquatic life"))

(terrain-type star
  (help "What a planet orbits"))

(define living-worlds (g-planet h-planet l-planet m-planet n-planet))
(define dead-worlds (a-planet b-planet c-planet d-planet e-planet f-planet i-planet j-planet k-planet))

(add t* capacity 4)

;; If a planet is inhabited, it will support a civilization.
#| Perhaps there are not enough civilizations defined;
   the theory that this is based on does not take pre-industrial civilizations into account.

Pre-level 0 civilizations might be:
-Sumerian 
-Babylonian (too similar to Sumerian?)
-Egyptian (not sure about this; too much disagreement about Egyptian history)
-Greek
-Roman
-Islamic (not medieval; that would be a downgrade from Roman)
-Renaissance
-Pre-Industrial Revolution

There would still be problems, such as how to model civilizations that are divided into nations.
(the above problem already exists with the current Level 0 civilizations)
|#

#| This is *almost* ready to be used...
   (specifically, needs a lot of fact-checking and revisions)
(unit-type civ-1 (image-name "tribe")
  (help "Level 1 civilization: dawn of civilization")
  (notes "A Level 1 civilization is less than 1,000 years old, and most closely resembles ancient Sumer."))
(unit-type civ-2 (image-name "village-2")
  (help "Level 2 civilization: boasts greater art and religion")
  (notes "A Level 2 civilization is still primitive, but may boast a more powerful church.  It most closely resembles ancient Babylon.")))
(unit-type civ-3 (image-name "pyramid")
  (help "Level 3 civilization: boasts advanced in mathematics")
  (notes "A Level 3 civilization benefits from advannnces, especially in mathematics, that allow for new architectural and technological advances.  It most closely resembles ancient Egypt."))
(unit-type civ-4 (image-name "parthenon")
  (help "Level 4 civilization: boasts further advances in mathematics, esp. geometry")
  (notes "A Level 4 civilization benefits from knowledge of geometry, and while not necessarily as driven to build wonders as a Level 3 civilization might be, it does show improvements in seafaring and architecture.  It most closely resembles ancient Greece."))
(unit-type civ-5 (image-name "village")
  (help "Level 5 civilization: boasts significant advances in government")
  (notes "The most impressive feature of a Level 5 civilization is the republic, which is one of the most effective form of government known even today.  It also boasts technological advances that improve the lives of the entire population, such as the aqueduct.  It most closely resembles ancient Rome."))
(unit-type civ-6 (image-name "city15")
  (help "Level 6 civilization: boasts advances in economics"))
  ;; (need notes; I need to research ancient Islamic civilization!)
(unit-type civ-7 (image-name "town20")
  (help "Level 7 civilization: boasts advances in writing and metalworking")
  (notes "A Level 7 civilization is best remembered for creating the printing press.  It also boasts advances in metalworking, which lay the foundation for the industrial advances of the next two levels.  It most closely resembles Europe during the Rennaissance era."))
(unit-type civ-8 (image-name "city18")
  (help "Level 8 civilization: boasts advances in transportation")
  (notes "A Level 8 civilization is best known for advances in terrestrial transportation, such as sailing ships, that allow it to extend its reach to distant lands.  It is also on the brink of industrialization.  It most closely resembles Europe circa 1500-1850AD."))
(unit-type civ-9 (image-name "city19")
  (help "Level 9 civilization: the industrial revolution begins")
  (notes "A Level 9 civilization begins with industrialization.  It is marked by significant advances in metalworking, agriculture, medicine, and literacy.  It most closely resembles Europe and North America circa 1850-1950AD."))
(unit-type civ-10 (image-name "city20")
  (help "Level 10 civilization: the information age begins")
  (notes "A Level 10 civilization begins with the discovery and spread of computers.  With computers, technology in this civilization advances at an unparalleled rate, dwarfing even that of the Industrial Revolution.  It most closely resembles Earth circa 1950-2000AD."))
(unit-type civ-11 (image-name "town22")
  (help "Level 11 civilization: free of fossil fuels")
  (notes "The key factor that differentiates a Level 11 civilization from a Level 10 civilization is that Level 11 is completely independent from fossil fuels.  All of its energy demands are satisfied by renewable sources such as solar power."))
(unit-type civ-12 (image-name "city22")
  (help "Level 12 civilization: the space age begins")
  (notes "A Level 12 civilization begins when the civilization successfully establishes a colony on another world within its solar system.  Once this happens, it is certain that the civilization will remain in space for a very long time."))
(unit-type civ-13 (image-name "ancient-yellow-castle")
  (help "Level 13 civilization: civilization on a galactic scale")
  (notes "A Level 13 civilization begin when, having colonized all colonizable worlds within its own solar system, the civilization pushes outward and encompasses other star systems.  If nothing gets in its way, it could eventually grow to encompass the entire galaxy!"))
|#

;; I'm giving up on this model; it completely omitted pre-industrial civilizations...
(unit-type civ-0 (image-name "parthenon")
  (hp-max 25) (hp-recovery 0.1) (cp 10) (point-value 128)
  (help "Level-0 civilization: dependent on fossil fuels")
  (notes "A Level 0 civilization is dependent on fossil fuels for its energy, and may be divided along racial (subspecies) lines.  It can survive for about 500 years, but will fall apart if it cannot develop renewable energy sources.  It is also vulnerable to natural disasters and may be weakened by them."))
(unit-type civ-1 (image-name "city19")
  (hp-max 50) (hp-recovery 0.2) (cp 20) (point-value 256)
  (help "Level-1 civilization: beyond fossil fuel, and a unified race")
  (notes "By Level 1, a civilization is unified as a single species and is independent of fossil fuels.  At this point, no natural disaster could destroy it, and it is resilient to alien invsasions."))
(unit-type civ-2 (image-name "city20")
  (hp-max 100) (hp-recovery 0.5) (cp 30) (point-value 512)
  (help "Level-2 civilization: encompasses an entire star system"))
(unit-type civ-3 (image-name "city22")
  (hp-max 200) (hp-recovery 1) (cp 40) (point-value 1024)
  (help "Level-3 civilization: has the power to go beyond deep space"))


#| This was my original idea, but it didn't work as I had hoped...
(unit-type civilization (image-name "city20")
  (hp-max 25) (hp-recovery 0.1) (cp 10)
  (help "Sentient inhabitants of a habitable world"))
  ;; The level of a civilization is determined by its facilities.
|#


(unit-type terraforming-station (image-name "town22")
  (hp-max 4) (hp-recovery 0.1) (acp-per-turn 1) (cp 50) (point-value 16)
  (help "Station that transforms a planet"))

(unit-type ruins (image-name "ruins")
  (hp-max 25) (point-value 0)
  (help "Ruins of a dead civilization"))

(define civilizations (civ-0 civ-1 civ-2 civ-3))
(add civilizations wrecked-type ruins)

;; Artificial colonies
(unit-type settlement (image-name "domed-city")
  (hp-max 10) (hp-recovery 0.2) (cp 10) (point-value 32)
  (help "Artificial colony for inhospitable worlds"))
(unit-type space-colony (image-name "space-station")
  (hp-max 10) (hp-recovery 0.2) (cp 5) (point-value 24)
  (help "A city in outer space"))

;; Orbital facilities
(unit-type shipyard (image-name "spaceyard")
  (hp-max 10) (acp-per-turn 1) (cp 20) (point-value 8)
  (help "Your only way to build large spaceships"))
(unit-type dysen-sphere (image-name "death-star")
  (hp-max 100) (hp-recovery 0.1) (cp 80) (point-value 96)
  (help "The ultimate solar collector"))

;; Deep space stations
(unit-type outpost (image-name "saucerpad")
  (hp-max 15) (acp-per-turn 0) (cp 25) (point-value 4)
  (help "Increases the range at which you can trade materials"))

;; Ground facilities
(unit-type telescope (image-name "spysat")
  (hp-max 1) (cp 10) (point-value 0)
  (help "Orbital telescope sees things far away"))
(unit-type spaceport (image-name "airbase")
  (hp-max 2) (cp 10) (point-value 0)
  (help "Landing site for small spacecraft; allows long-distance trade"))
(unit-type elevator (image-name "wiz-guild")
  (hp-max 3) (cp 30) (point-value 2)
  (help "Space elevator: the ultimate path from land to space"))
(unit-type arcology (image-name "tower")
  (hp-max 5) (cp 20) (point-value 1)
  (help "An entire city enclosed in a very tall building"))

(unit-type antimatter-plant (image-name "facility")
  (hp-max 4) (cp 20) (point-value 1)
  (help "Produces matter and antimatter for starships"))
(unit-type torpedo-industry (image-name "shell")
  (hp-max 4) (cp 20) (point-value 1)
  (help "Produces photon torpedoes"))
(unit-type ore-processor (image-name "iron-mine")
  (hp-max 3) (cp 15) (point-value 0)
  (help "Streamlines the production of ores"))
(unit-type academy (image-name "corps-hq")
  (hp-max 3) (cp 15) (point-value 0)
  (help "Boosts the production of personnel"))
(unit-type wcs (image-name "city30")
  (hp-max 2) (cp 30) (point-value 0)
  (help "Ensures good weather; doubles food production"))

(unit-type solar-plant (image-name "refinery")
  (hp-max 2) (cp 25) (point-value 0)
  (help "Network of solar panels; frees a planet from fossil fuels"))

;; Small spacecraft
(unit-type probe (image-name "spysat")
  (hp-max 1) (acp-per-turn 3) (cp 3) (point-value 0)
  (help "Slow spacecraft used for exploration"))
(unit-type shuttle (image-name "sr71")
  (hp-max 2) (acp-per-turn 2) (cp 8) (point-value 1)
  (help "Small spacecraft for short-range transport"))

;; Test units
#| No longer needed.
(unit-type cargoliner (image-name "gxy-st")
  (hp-max 3) (acp-per-turn 4) (cp 20)
  (help "Large spacecraft for transporting people and supplies"))
(unit-type mothership (image-name "mothership")
  (hp-max 5) (acp-per-turn 6) (cp 30)
  (help "Carries lots of people and smaller spacecraft"))
  ; Is this a good idea?
|#

;; Federation starships
(unit-type vws (image-name "4e")
  (hp-max 1) (acp-per-turn 3) (cp 12)
  (help "Vulcan warp shuttle: small, fast, and cheap"))
(unit-type s-deadalus (image-name "gxy-lc")
  (hp-max 4) (acp-per-turn 5) (cp 20)
  (help "Deadalus-class starship: primitive, slow, and weak"))
(unit-type s-oberth (image-name "gxy-f1")
  (hp-max 3) (acp-per-turn 6) (cp 25)
  (help "Oberth-class starship: the preferred science vessel"))
(unit-type s-constitution1 (image-name "gxy-cc")
  (hp-max 6) (acp-per-turn 6) (cp 30)
  (help "Constitution-class starship: well-rounded, best of its age"))
(unit-type s-constitution2 (image-name "gxy-cc")
  (hp-max 7) (acp-per-turn 7) (cp 35)
  (help "Constitution-class starship, new and improved"))
(unit-type s-miranda (image-name "gxy-st")
  (hp-max 8) (acp-per-turn 6) (cp 40)
  (help "Miranda-class starship: built for power"))
(unit-type s-constellation (image-name "gxy-cc")
  (hp-max 7) (acp-per-turn 8) (cp 45)
  (help "Constellation-class starship: preferred deep-space vessel"))
(unit-type s-excelsior (image-name "gxy-cc")
  (hp-max 9) (acp-per-turn 9) (cp 50)
  (help "Excelsior-class starship: superior to Constitution-class"))
(unit-type s-ambassador (image-name "gxy-cc")
  (hp-max 10) (acp-per-turn 10) (cp 55)
  (help "Ambassador-class starship: first ship with detachable hulls"))
(unit-type s-nebula (image-name "gxy-st")
  (hp-max 14) (acp-per-turn 12) (cp 60)
  (help "Nebula-class starship: built for power, but less than Galaxy-class"))
(unit-type s-galaxy (image-name "gxy-cc")
  (hp-max 15) (acp-per-turn 12) (cp 65)
  (help "Galaxy-class starship: most powerful starship to date"))

;; Other units
(unit-type engineers (image-name "worker")
  (hp-max 1) (acp-per-turn 1) (cp 6)
  (help "Builds colonies and space stations"))
(unit-type ground-troops (image-name "soldiers")
  (hp-max 1) (acp-per-turn 1) (cp 4)
  (help "Soldiers that capture and defend worlds"))

(add civilizations namer "generic-names")

;; Basic resources
(material-type food (resource-icon 1) (image-name "green")
  (help "Sustains life on a planet or station"))
(material-type fuel (resource-icon 2) (image-name "red")
  (help "Hydrogen-based fuel: powers short-range spacecraft"))
(material-type ores (resource-icon 3) (image-name "grey")
  (help "Used to build anything"))
(material-type solar (resource-icon 4) (image-name "yellow")
  (help "Used to power civilizations"))

;; Advanced resources
;; (should personnel be a basic resource?)
(material-type petroleum
  (help "Powers level 0 civilizations, but runs out quickly"))
(material-type matter
  (help "Warp-drive matter fuel: needed for interstellar travel"))
(material-type antimatter
  (help "Warp-drive antimatter fuel: needed for interstellar travel"))
(material-type personnel (resource-icon 4) (image-name "blue")
  (help "Required to operate virtually everything"))
(material-type torpedoes
  (help "Photon torpedoes: can be fired at enemies from a distance"))

;; Special resources
(material-type protomatter
  (help "Required to terraform a planet"))

; The idea behind having ores _and_ personnel is a limiting-reagent 
; problem.  The intent is to prevent any one construction task from 
; sucking up all resources.

(define worlds (living-worlds dead-worlds))
(define stations (settlement space-colony))

(define places (civilizations stations))
(define stars (star))
(define sr-spacecraft (shuttle))
(define lr-spacecraft (vws s-deadalus s-oberth s-constitution1
                       s-constitution2 s-miranda s-constellation 
                       s-excelsior s-ambassador s-nebula s-galaxy))
(define armies (ground-troops))
(define spacecraft (sr-spacecraft lr-spacecraft))
(define facilities (telescope spaceport elevator arcology antimatter-plant 
                    torpedo-industry ore-processor academy solar-plant wcs))
(define o-facilities (shipyard dysen-sphere outpost))

(define lr-fuel (matter antimatter))

;; Hints for the AI
(add facilities facility true)
(add armies ground true)
(add engineers colonizer true)
(add sr-spacecraft air true)
(add lr-spacecraft naval true)



;;; ADVANCES
#| Not yet ready to be used...
;; (Note that the levels of advances are in no way related to the levels of civilization.
;; It is simply easier to track the development of virtually everything using levels.)
;; Level 1

(advance-type solar-plant
  (help "Renewable energy for civilizations")
  (help "Solar energy replaces petroleum products in Level 1 civilizations.  It is essential for even a Level 0 civilization to have energy, but unlike petroleum, solar energy can never be depleted."))
(advance-type dilithium
  (help "Superconductor for antimatter reactors")
  (help "Dilithium is an essential ingredient for controlling an antimatter reaction.  It is the only known crystal that can conduct the energy produced by such a reactor."))
(advance-type fuel-cell
  (help "Hydrogen-based fuel")
  (help "Hydrogen is used by fuel cells for most short-range travel.  Although the fuel cell is not as efficent as an antimatter reactor, it is much easier to build."))
(advance-type reusable-spacecraft
  (help "Spacecraft that can take off and land with little fuss")
  (notes "In order to make space travel viable enough to even think about colonization, spacecraft need to be completely reusable.  Modern space shuttles don't count because they have to drop their booster rockets and external tank in order to reach orbit."))
(advance-type quantum-physics
  (help "The laws of motion for really small objects")
  (notes "Before many futuristic machines like the transporter can be invented, it is necessary to discover the laws of physics that apply to both the microscopic and the macroscopic world."))
(advance-type cold-fusion
  (help "The most powerful form of nuclear energy")
  (notes "Cold fusion allows nuclear reactions to produce far more energy than old-style fission, and it produces no radioactive waste."))
(advance-type biotech
  (help "The technology to create and control living organisms"
  (notes "A good knowledge of biotechnology makes it possible to construct completely self-sufficient biosystems.  It is essential to colonize inhospitable worlds."))
(advance-type isolinear-rod
  (help "High-speed semiconductor for powerful computers")
  (notes "Isolinear rods replace silicon wafers in computers.  They allow for a far greater density of transistors, allowing computers to perform tasks of the speed and size needed for transporters, among other things."))

;; Level 2

(advance-type antimatter
  (help "The ultimate energy source")
  (help "When normal matter and antimatter are mixed, they are both converted into pure energy.  This reaction is the only way to produce enough energy to travel faster than light."))
(advance-type pressure-dome
  (help "Domed, self-contained cities")
  (notes "Domed cities are the only way to colonize an inhospitable world, as they allow a city to have its own atmosphere.  When combined with agricultural facilities, the pressure dome makes any planet habitable."))
(advance-type warp-field
  (help "Controlled space-time distortion")
  (notes "Warp fields allow a starship to move through space faster than the speed of light.  It is the single most important advance for a galactic civilization."))
(advance-type artificial-gravity
  (help "Good for astronaut's health")
  (notes "Artificial gravity is necessary on starships because it forces the body to maintain its normal bone density.  Otherwise, astronauts would rather quickly develop osteoporosis."))
(advance-type gas-exchange-device
  (help "Recirculates oxygen")
  (notes "Gas Exchange Devices, or GED's, transform carbon dioxide gas into oxygen gas, making it possible to run spaceships without arboretums or re-supplying oxygen."))
(advance-type terraforming
  (help "The ability to transform an entire planet")
  (help "Terraforming technology makes it possible to transform the most inhospitable worlds into paradises.  Of course, no such trnsformation every happens overnight."))
(advance-type materialization
  (help "Ordered conversion of energy into matter")
  (notes "Materialization technology allows anything to be produced from pure energy.  However, it requires tremendous energy and a computer than can handle the patterns."))
(advance-type deflector-shield
  (help "Energy bubble that shields against most weaponry")
  (notes "A spaceship equipped with a deflector shield is far less vulnerable in combat.  It is also less prone to accidents such as stray asteroids.")))

;; Level 3

(advance-type transporter
  (help "Dematerialization of an object, followed by rematerialization somewhere else")
  (notes "The transporter allows for easy movement of people supplies over short distances without shuttles.  However, it does not work at distances greater than about 1,000 kilometers."))
(advance-type solar-fusion
  (help "The fusion that happens in a star")
  (notes "A thourogh understanding of solar fusion allows for new technologies such as terraforming and photon-based devices.  In the military, the best-known application is the photon torpedo."))
;; (need intermediate advances for the replicator)

;; Level 4

(advance-type photon-torpedo
  (help "Long-range weapon based on light energy")
  (notes "The photon torpedo is the most effective means of fleet combat.  It allows a starships to attack from a distance and do twice as much damage as with phasers.  On the other hand, the advantage is unclear if your opponent has photon torpedoes."))
(advance-type replicator
  (help "Machine that converts energy into matter")
  (notes "The ability to store transporter patterns in long-term computer memory allowed for the creation of the replicator.  It made it possible to replicate things such as food and some starship parts, thus allowing starships to run longer missions without having to resupply."))


(table advance-needed-to-research
  (a* a* false)

  (antimatter cold-fusion true)
  (pressure-dome biotech true)
  (warp-field quantum-physics true)
  (artificial-gravity quantum-physics true)
  (gas-exchange-device biotech true)
  (terraforming biotech true)
  (materialization quantum-physics true)
  (transporter materialization true)
  (transporter isolinear-rod true)
  (solar-fusion cold-fusion true)
  (photon-torpedo quantum-physics true)
  (photon-torpedo solar-fusion true)
  (replicator transporter true)
  

)

Still need:
-Antimatter
-Deflector shield
-Detachable hull (that'll be _really_ hard!)
-Synthehol?

It may also become necessary to use the tech code due to the number of small, unpredictable advances that would be necessary for many futuristic units.
It could also represent things like time to make blueprints and such.
(won't the tech/toolup code need to be re-worked for that to be doable?)

|#

;;; SIDES


;; Taken from the "Life Forms" listing in the _Star Trek Encyclopedia_
;; Some of the noncorporeal (e.g. Calamarain) and/or more primitive (e.g. denebian slime devil) species have been omitted.
(set side-library '(
  ((noun "Acamarian"))
  ((noun "Aldean"))
  ((noun "Andorian"))
  ((noun "Angosian"))
  ((noun "Antican"))
  ((noun "Antidean"))
  ((noun "Arbazan"))
  ((noun "Argelian"))
  ((noun "Argosian"))
  ((noun "Bajoran"))
  ((noun "Bandi") (plural-noun "Bandi"))
  ((noun "Barolian"))
  ((noun "Barzan"))
  ((noun "Benzite"))
  ((noun "Berellian"))
  ((noun "Betazoid"))
  ((noun "Bolian"))
  ((noun "Breen") (plural-noun "Breen"))
  ((noun "Brekkian"))
  ((noun "Bynar"))
  ((noun "Caldonian"))
  ((noun "Calnoth"))
  ((noun "Capellan"))
  ((noun "Cardassian"))
  ((noun "Chrysalian"))
  ((noun "Cytherian"))
  ((noun "Dachlyds"))
  ((noun "Deltan"))
  ((noun "Dopterian"))
  ((noun "Edo"))
  ((noun "Ekosian"))
  ((noun "Elasian"))
  ((noun "Excalabian"))
  ((noun "Fabrini") (plural-noun "Fabrini"))
  ((noun "Ferengi"))
  ((noun "Frunalain"))
  ((noun "Gemarian"))
  ((noun "Gorn"))
  ((noun "Haliian"))
  ((noun "Halkan"))
  ((noun "Human"))
  ((noun "Hupyrian"))
  ((noun "Husnock"))
  ((noun "Iconian"))
  ((noun "Iotian"))
  ((noun "J'naii") (plural-noun "J'naii"))
  ((noun "Jaradan"))
  ((noun "Kalandan"))
  ((noun "Kelvan"))
  ((noun "Kerelian"))
  ((noun "Klingon"))
  ((noun "Kobliad"))
  ((noun "Ktarian"))
  ((noun "Legaran"))
  ((noun "Lenarian"))
  ((noun "Leyron"))
  ((noun "Ligonian"))
  ((noun "Malcorian"))
  ((noun "Malurian"))
  ((noun "Medusan"))
  ((noun "Melkotian"))
  ((noun "Menthar"))
  ((noun "Metron"))
  ((noun "Mikulak"))
  ((noun "Minosian"))
  ((noun "Mintakan"))
  ((noun "Miradorn"))
  ((noun "Mizarian"))
  ((noun "Moropan"))  ;; is this correct spelling of species name?
  ((noun "Nausicaan"))
  ((noun "Orion"))
  ((noun "Ornaran"))
  ((noun "Pakled"))
  ((noun "Paxan"))
  ((noun "Pentaran"))
  ((noun "Promellian"))
  ((noun "Rakhari") (plural-noun "Rakhari"))
  ((noun "Rigelian"))
  ((noun "Romulan"))
  ((noun "Rutian"))
  ((noun "Satarran"))
  ((noun "Scalosian"))
  ((noun "Selay") (plural-noun "Selay"))
  ((noun "Tagran"))
  ((noun "Takaran"))
  ((noun "Talarian"))
  ((noun "Talosian"))
  ((noun "Tamarian"))
  ((noun "Tanugan"))
  ((noun "Tarellian"))
  ((noun "Tellarite"))
  ((noun "Tholian"))
  ((noun "Trill"))
  ((noun "Troyian"))
  ((noun "Ullian"))
  ((noun "Valerian"))
  ((noun "Ventaxian"))
  ((noun "Vian"))
  ((noun "Vorgon"))
  ((noun "Vulcan"))
  ((noun "Yridian"))
  ((noun "Zakdorn"))
  ((noun "Zaldan"))
  ((noun "Zeon"))
  ((noun "Zetarian"))
  ((noun "Zibalian"))
))

(set sides-min 1)

;;; ENVIRONMENT

(add t* occurrence 1)
(add vacuum occurrence 100)

(add civ-2 start-with 1)
;; (add oil-plant start-with 1)

(add m-planet country-terrain-min 1)

(table favored-terrain
  (u* t* 0)
  (civilizations m-planet 100)
  (o-facilities vacuum 100)
)

(add civilizations advanced true)
(add stations advanced true)
(add civilizations acp-independent true)
(add stations acp-independent true)
(add shipyard acp-independent true)
(add civilizations acp-per-turn 0)
(add stations acp-per-turn 0)
(add shipyard acp-per-turn 0)

;; Solar energy is available in any part of the universe, but in varying quantities.
;; Food and ores, however, are restricted to planets.
;; (shouldn't availability of solar depend on proximity to stars?)

(table production-from-terrain
  ;; Planets provide food, fuel, and ores, depending on their type.
  (a-planet fuel 100)
  (b-planet fuel 50)
  (c-planet fuel 10)
  (c-planet ores 2)
  (d-planet ores 10)
  (e-planet fuel 15)
  (e-planet ores 20)
  (f-planet fuel 5)
  (f-planet ores 10)
  (g-planet food 1)
  (g-planet fuel 15)
  (g-planet ores 15)
  (g-planet personnel 1)
  (h-planet food 1)
  (h-planet fuel 10)
  (h-planet ores 30)
  (h-planet personnel 1)
  (i-planet fuel 20)
  (i-planet ores 15)
  (j-planet fuel 20)
  (j-planet ores 20)
  (k-planet fuel 5)
  (k-planet ores 25)
  (l-planet food 2)
  (l-planet fuel 10)
  (l-planet ores 20)
  (l-planet personnel 1)
  (m-planet food 8)
  (m-planet fuel 25)
  (m-planet ores 20)
  (m-planet personnel 2)
  (n-planet food 4)
  (n-planet fuel 10)
  (n-planet ores 5)
  (n-planet personnel 1)

  ;; There is solar energy in space...
  (t* solar 1)
  (vacuum solar 2)
  ;; ...but it is most concentrated in stars.
  (star solar 100)
)

;; Different levels of civilizations can grow to different sizes.
;; The higher the level, the larger it can get, so it is desirable to
;; upgrade to higher civilizations as quickly as possible.
(add civ-0 minimal-size-goal 1)
(add civ-1 minimal-size-goal 2)
(add civ-2 minimal-size-goal 4)
(add civ-3 minimal-size-goal 8)
;; (add civilization minimal-size-goal 6)
(add space-colony minimal-size-goal 1)
(add settlement minimal-size-goal 2)

(add civ-0 reach 1)
(add civ-1 reach 2)
(add civ-2 reach 4)
(add civ-3 reach 8)
;; (add civilization reach 8)
(add space-colony reach 2)
(add settlement reach 2)

#| Spaceports should extend the reach of a civilization, but that is not
   possible with Xconq (yet).  The following tables were a feeble attempt to
   make spaceports useful for stuff other than longer supply lines.
(table size-limit-without-occupant
  (civilizations spaceport 6)
)

(table occ-multiply-maxcells
  (civilization spaceport 200)
)
|#

(table size-limit-without-occupant
  (civilizations spaceport 6)	;; Need a spaceport to grow beyond 6 billion people.
  (civilizations elevator 12)	;; Need a space elevator to grow beyond 12 billion people.
)

(table occ-add-maxcells
  (civilizations spaceport 1)	;; A spaceport allows a civilization to cover an extra planet.
)

(table occ-multiply-maxcells
  (civilizations elevator 200)	;; A space elevator allows a civilization to cover twice as many planets.
)

(add u* free-acp 1)

(add civilizations speed 0)
(add settlement speed 0)
(add space-colony speed 0)
(add facilities speed 0)
(add o-facilities speed 0)

(table vanishes-on
  (armies vacuum true)
  (engineers vacuum true)
  (settlement vacuum true)

  (stations t* true)
  (stations vacuum false)
  (settlement t* false)
  (settlement vacuum true)

  (o-facilities t* true)
  (o-facilities vacuum false)

  (u* star true)
  (dysen-sphere t* true)
  (dysen-sphere star false)

)

(table mp-to-enter-terrain
  (armies vacuum 99)
  (engineers vacuum 99)

  (u* star 99)
)

(table independent-density
  ;; There is a small probability that a planet will be inhabited, if it can
  ;; support life.
  ;; It is also possible, although highly unlikely, that a level 3
  ;; civilization will be found somewhere at the beginning of the game.
  ;; (that would be like finding an abandoned aircraft carrier on a beach!)

  (civ-0 g-planet 15)
  (civ-0 h-planet 20)
  (civ-0 l-planet 60)
  (civ-0 m-planet 180)
  (civ-0 n-planet 120)

  (civ-1 g-planet 10)
  (civ-1 h-planet 15)
  (civ-1 l-planet 40)
  (civ-1 m-planet 120)
  (civ-1 n-planet 60)

  (civ-2 g-planet 2)
  (civ-2 h-planet 3)
  (civ-2 l-planet 4)
  (civ-2 m-planet 12)
  (civ-2 n-planet 6)

  (civ-3 m-planet 1)

  #|
  (civilization g-planet 10)
  (civilization h-planet 15)
  (civilization l-planet 40)
  (civilization m-planet 120)
  (civilization n-planet 60)
  |#
)

;;; CONSTRUCTION

(table acp-to-create
  (civilizations ground-troops 1)
  (civilizations engineers 1)
  (civilizations facilities 1)
  (civilizations sr-spacecraft 1)
  (civilizations probe 1)

  (stations ground-troops 1)
  (stations sr-spacecraft 1)
  (stations engineers 1)
  (stations facilities 1)
  (stations probe 1)

  (shipyard sr-spacecraft 1)
  (shipyard lr-spacecraft 1)

;  (spaceport shuttle 1)

  (engineers settlement 1)
  (engineers civ-1 1)
  (engineers space-colony 1)
  (engineers o-facilities 1)
  (engineers terraforming-station 1)

  (civ-0 civ-1 1)
  (civ-1 civ-2 1)
  (civ-2 civ-3 1)
)

(table acp-to-build
  (civilizations ground-troops 1)
  (civilizations engineers 1)
  (civilizations facilities 1)
  (civilizations sr-spacecraft 1)
  (civilizations probe 1)
  (stations ground-troops 1)

  (shipyard sr-spacecraft 1)
  (shipyard lr-spacecraft 1)

  (stations engineers 1)
  (stations shuttle 1)
  (stations facilities 1)
  (stations probe 1)

;  (spaceport shuttle 1)

  (engineers settlement 1)
  (engineers civ-1 1)
  (engineers space-colony 1)
  (engineers o-facilities 1)
  (engineers terraforming-station 1)

  (civ-0 civ-1 1)
  (civ-1 civ-2 1)
  (civ-2 civ-3 1)
)

(table create-range
  (engineers space-colony 1)
  (engineers o-facilities 1)
)

(table build-range
  (engineers space-colony 1)
  (engineers o-facilities 1)
)

;;; MATERIALS

(table unit-storage-x
#| (should this be in a terrain-storage table?)
  (a-planet fuel 30000)
  (b-planet fuel 25000)
  (c-planet fuel 10000)
  (c-planet ores 1000)
  (d-planet ores 500)
  (e-planet fuel 5000)
  (e-planet ores 3000)
  (f-planet fuel 1000)
  (f-planet ores 4000)
  (g-planet food 500)
  (g-planet fuel 1000)
  (g-planet ores 8000)
  (g-planet personnel 100)
  (h-planet food 300)
  (h-planet fuel 1000)
  (h-planet ores 1000)
  (h-planet personnel 60)
  (i-planet fuel 2000)
  (i-planet ores 1000)
  (j-planet fuel 500)
  (j-planet ores 600)
  (k-planet fuel 1000)
  (k-planet ores 800)
  (l-planet food 500)
  (l-planet fuel 1200)
  (l-planet ores 1500)
  (l-planet personnel 100)
  (m-planet food 2000)
  (m-planet fuel 5000)
  (m-planet ores 2000)
  (m-planet personnel 400)
  (n-planet food 1500)
  (n-planet fuel 1500)
  (n-planet ores 500)
  (n-planet personnel 250)
|#

  (civ-0 food 300)
  (civ-1 food 600)
  (civ-2 food 1200)
  (civ-3 food 2400)

  (civ-0 fuel 100)
  (civ-1 fuel 200)
  (civ-2 fuel 400)
  (civ-3 fuel 800)

  (civ-0 ores 300)
  (civ-1 ores 600)
  (civ-2 ores 1200)
  (civ-3 ores 2400)

  (civ-0 personnel 300)
  (civ-1 personnel 600)
  (civ-2 personnel 1200)
  (civ-3 personnel 2400)

  (civ-0 petroleum 500)
  (civ-1 solar 1000)
  (civ-2 solar 2000)
  (civ-3 solar 4000)

  (civ-0 lr-fuel 10)
  (civ-1 lr-fuel 100)
  (civ-2 lr-fuel 1000)
  (civ-3 lr-fuel 10000)
  
  (civ-0 torpedoes 20)
  (civ-1 torpedoes 40)
  (civ-2 torpedoes 80)
  (civ-3 torpedoes 160)

#|
  (civilization food 2400)
  (civilization fuel 800)
  (civilization ores 2400)
  (civilization personnel 2400)
  (civilization petroleum 500)
  (civilization solar 4000)
  (civilization lr-fuel 4000)
|#

  (settlement food 300)
  (settlement fuel 150)
  (settlement ores 300)
  (settlement personnel 200)
  (settlement solar 750)
  (settlement lr-fuel 2000)

  (space-colony food 100)
  (space-colony fuel 1500)
  (space-colony ores 100)
  (space-colony personnel 20)
  (space-colony solar 1000)
  (space-colony lr-fuel 8000)

  (shipyard fuel 800)
  (shipyard ores 300)
  (shipyard lr-fuel 3000)
  (shipyard personnel 200)
  (shipyard torpedoes 100)
  (shipyard solar 100)

  (dysen-sphere solar 1000)

  (outpost fuel 500)
  (outpost ores 1800)
  (outpost personnel 50)
  (outpost lr-fuel 1000)
  (outpost torpedoes 50)
  (outpost solar 100)

  (spaceport m* 200)
  (spaceport food 0)
  
  (terraforming-station protomatter 1000)

  (shuttle fuel 18)

#|
  (cargoliner fuel 100)
  (cargoliner ores 50)
  (mothership fuel 200)
|#

  (s-deadalus lr-fuel 100)
  (s-oberth lr-fuel 150)
  (s-constitution1 lr-fuel 250)
  (s-constitution2 lr-fuel 300)
  (s-miranda lr-fuel 200)
  (s-constellation lr-fuel 600)
  (s-excelsior lr-fuel 400)
  (s-ambassador lr-fuel 500)
  (s-nebula lr-fuel 800)
  (s-galaxy lr-fuel 1000)

  ;; Starships that can carry a shuttle should have fuel reserves for them.
  (s-deadalus fuel 64)
  (s-oberth fuel 80)
  (s-constitution1 fuel 128)
  (s-constitution2 fuel 196)
  (s-miranda fuel 128)
  (s-constellation fuel 196)
  (s-excelsior fuel 256)
  (s-ambassador fuel 384)
  (s-nebula fuel 256)
  (s-galaxy fuel 512)
  
  ;; Starships only can carry photon torpedoes if they were invented after
  ;; photon torpedoes.
  (s-constitution1 torpedoes 10)
  (s-constitution2 torpedoes 20)
  (s-miranda torpedoes 50)
  (s-constellation torpedoes 25)
  (s-excelsior torpedoes 40)
  (s-ambassador torpedoes 80)
  (s-nebula torpedoes 200)
  (s-galaxy torpedoes 500)
  
  ;; Facilities that produce advanced resources can stockpile for later use.
  ;; (also helpful if the civilization or colony has a low capacity for the
  ;; material in question!)
  ;; Antimatter plants should produce equal amounts of matter and antimatter,
  ;; but it seems that matter is supplied to starships less readily than
  ;; antimatter, so try to compensate on the supply side.
  (antimatter-plant lr-fuel 100)
  (torpedo-industry torpedoes 50)

;  (engineers ores 25)
)

(table base-production
  ;; Artificial colonies must be equipped to make their own supplies.
  (settlement food 3)
  (settlement personnel 1)
  (space-colony food 2)
  (space-colony fuel 5)
  (space-colony ores 1)
  (space-colony personnel 1)

  (shipyard fuel 1)

  (arcology food 4)
  (arcology fuel 10)
  (arcology personnel 1)

  (antimatter-plant lr-fuel 60)
  (torpedo-industry torpedoes 10)
  
  (terraforming-station protomatter 1)

  ;; Solar plants actually produce petroleum as quickly as it is consumed.
  ;; This is unrealistic, but it does allow a civilization to function without
  ;; additional complexities.
  ;; (What if there were an oil-plant and a solar-plant that both produced a
  ;; generic energy material, but the oil-plant required petroleum and would
  ;; starve without it?)
  (solar-plant petroleum 1)

  (dysen-sphere solar 100)
)

(table unit-initial-supply
  ;; Most civilizations start out with nothing.  However, level 0
  ;; civilizations are dependant on petroleum, and therefore need a supply
  ;; of oil that will last them until they can upgrade to level 1.
  (civ-0 petroleum 500)

  ;; In case "More starters" is used, there are some units that need to be fueled,
  ;; otherwise they'll die immediately!
  (sr-spacecraft fuel 9999)
  (lr-spacecraft lr-fuel 9999)
)

(table base-consumption
;  (shipyard fuel 1)

  (sr-spacecraft fuel 1)
  (lr-spacecraft lr-fuel 1)

  (civ-0 petroleum 1)
)

(table supply-on-completion
  ;; See above for explanation
  (civ-0 petroleum 500)
)

(table occupant-multiply-production
  (solar-plant solar 200)	;; This is useful to Level >0 civilizations.
  (ore-processor ores 200)
  (academy personnel 200)
  (wcs food 200)
)

(table consumption-as-occupant
  ;; Nothing consumes anything if it's docked in something else.
  (u* m* 0)
)

(table material-to-move
  (sr-spacecraft fuel 1)
  (lr-spacecraft lr-fuel 1)
)

(table consumption-per-move
  (sr-spacecraft fuel 1)
  (lr-spacecraft lr-fuel 1)
)

(table unit-consumption-per-size
  (civilizations food 1)
  (stations food 1)
)

#| This is not allowed by Xconq (yet).
(table unit-production-per-size
  (civilizations ores 1)
  (stations ores 1)
)
|#

(table unit-consumption-to-grow
  (civilizations food 50)
  (stations food 50)
)

(table hp-per-starve
  (sr-spacecraft fuel 2.00)
  (lr-spacecraft lr-fuel 1.00)
)

(set event-notices '(
  ((unit-starved sr-spacecraft) (0 " runs out of fuel and disappears!"))
  ((unit-starved lr-spacecraft) (0 " runs out of fuel and disappears!"))
))

(table unit-consumption-per-cp
  ;; Solar is required for everything.  However, there should be some way
  ;; to allow a civ-0 to use petroleum instead of solar.

  (u* ores 5)
  (u* solar 1)
  (solar-plant solar 0)
  (facilities ores 10)
  (facilities solar 2)
  (armies ores 2)
  (armies personnel 2)
  (armies solar 1)
  (engineers ores 1)
  (engineers personnel 4)
  (engineers solar 1)
  (sr-spacecraft ores 10)
;  (sr-spacecraft personnel 1)
  (lr-spacecraft ores 25)
;  (lr-spacecraft personnel 1)
  (civilizations ores 5)


)

#| Needs more work than I'm willing to put in right now.
(table material-to-create
  (u* ores 1)
  (u* personnel 1)

  (civilizations ores 0)
  (civilizations personell 0)
)
|#

#| Can't seem to do this either.
(table consumption-per-create
  (u* ores 1)
  (u* personnel 1)
)
|#

(table material-to-build
  (civilizations solar 1)
  (civ-0 solar 0)
  (civ-0 petroleum 1)
)

(table consumption-per-build
  (civilizations solar 1)
  (civ-0 solar 0)
  (civ-0 petroleum 1)
)

; The setup with out-length and in-length allows every stationery unit to
; exchange any material except food (exchanging food causes weird things 
; to happen).  All units can exchange anything with an adjacent unit, but 
; shipyards can receive any supplies from 2 cells away (and anything can 
; send them there up to 2 cells away).  Spaceports and outposts increase
; their range to 8, with no theoretical upper limit.

(table out-length
  (civilizations m* 2)			;; Anything can send 2 cells away.
  (stations m* 1)			;; But stations can only send 1 cell away.
  (dysen-sphere solar 8)		;; The Dysen Sphere can send lots of solar energy to distant worlds.
  (spaceport m* 8)			;; A spaceport allows anything to go 8 cells.
  (outpost m* 8)			;; Outposts relay supplies up to 8 cells.
  (lr-spacecraft m* -1)			;; Starships should not automatically give materials.
  (lr-spacecraft fuel 0)		;; ...except fuel to shuttles.
  (antimatter-plant lr-fuel 2)		;; Allow antimatter plants to send fuel wherever it can be stored.
  (torpedo-industry torpedoes 2)	;; Likewise for torpedo industries.
  (u* food 0)				;; Food never travels between units.
)

(table in-length
  (civilizations m* 1)			;; Planets can receive from adjacent units.
  (stations m* 1)			;; Same for stations.
  (shipyard m* 2)			;; Shipyards can receive from 2 cells away.
  (spaceport m* 8)			;; A spaceport can receive from 8 cells away.
  (outpost m* 8)			;; Outposts relay supplies up to 8 cells.
  (u* food 0)				;; Food never travels between units.
)

;;; ENGINEERING

;; Terraforming

(table acp-to-add-terrain
  (terraforming-station living-worlds 1)
)

(table acp-to-remove-terrain
  (terraforming-station dead-worlds 1)
)

(table material-to-add-terrain
  (terraforming-station protomatter 1000)
)

(table consumption-per-add-terrain
  (living-worlds protomatter 1000)
)

(add terraforming-station acp-to-disband 1)
(add terraforming-station hp-per-disband 4)

;;; VISION

(add civilizations initial-seen-radius 4)
(add telescope vision-range 8)

;; Short-range sensors
(add s-deadalus vision-range 2)
(add s-oberth vision-range 4)
(add s-constitution1 vision-range 3)
(add s-constitution2 vision-range 4)
(add s-miranda vision-range 3)
(add s-constellation vision-range 5)
(add s-excelsior vision-range 5)
(add s-ambassador vision-range 6)
(add s-nebula vision-range 7)
(add s-galaxy vision-range 8)

;; Long-range sensors
(add telescope spy-chance 10000)
(add lr-spacecraft spy-chance 10000)

(add telescope spy-range 32)

(add s-deadalus spy-range 4)
(add s-oberth spy-range 8)
(add s-constitution1 spy-range 6)
(add s-constitution2 spy-range 8)
(add s-miranda spy-range 6)
(add s-constellation spy-range 10)
(add s-excelsior spy-range 10)
(add s-ambassador spy-range 12)
(add s-nebula spy-range 14)
(add s-galaxy spy-range 16)

(table spy-quality
  (lr-spacecraft lr-spacecraft 75)
  (lr-spacecraft sr-spacecraft 50)
  (lr-spacecraft civilizations 100)
)

;;; UNIT-UNIT RELATIONSHIPS

(doctrine engineer-doctrine
  (construction-run (u* 1)))

(doctrine place-doctrine
  (construction-run (facilities 1)))

(side-defaults
  (doctrines (engineers engineer-doctrine)
  (civilizations place-doctrine)
  (stations place-doctrine))
)

;;; CAPACITANCE

(add civilizations capacity 32)
(add stations capacity 16)
(add shipyard capacity 4)
(add outpost capacity 2)
(add terraforming-station capacity 4)

(add shuttle capacity 1)
; (add cargoliner capacity 8)
; (add mothership capacity 12)
(add lr-spacecraft capacity 1)
(add s-constitution1 capacity 2)
(add s-constitution2 capacity 2)
(add s-excelsior capacity 3)
(add s-ambassador capacity 4)
(add s-galaxy capacity 8)
;(add spaceport capacity 32)

#|
(table terrain-capacity-x
  (engineers living-worlds 1)
  (engineers dead-worlds 1)
  (settlement dead-worlds 1)
)
|#

(table unit-size-in-terrain
  (u* t* 1)
  (civilizations t* 4)
  (terraforming-station t* 4)
)

(table unit-size-as-occupant
  (u* u* 999)
  (armies civilizations 1)
  (armies lr-spacecraft 1)
  (armies stations 1)
  (engineers u* 1)
  (facilities u* 0)
  (facilities o-facilities 999)


  (sr-spacecraft u* 999)
  (sr-spacecraft stations 1)
  (sr-spacecraft o-facilities 1)
  (sr-spacecraft civilizations 1)
  (sr-spacecraft lr-spacecraft 1)
  (lr-spacecraft u* 999)
  (lr-spacecraft o-facilities 1)
  (lr-spacecraft space-colony 4)

  (probe u* 1)
  (probe terraforming-station 999)

  (o-facilities u* 999)

  (armies shuttle 1)
  (engineers shuttle 1)

  (civ-1 civ-0 0)
  (civ-2 civ-1 0)
  (civ-3 civ-2 0)
)

(table occupant-max
  (armies civilizations 24)
  (engineers civilizations 16)
  (sr-spacecraft civilizations 8)
  (probe civilizations 12)
  
  (armies stations 8)
  (engineers stations 6)
  (sr-spacecraft stations 12)
  (probe stations 4)
)

;;; COMBAT

(table acp-to-attack
  (u* u* 1)
  (engineers u* 0)
  (probe u* 0)
)

(add lr-spacecraft acp-to-fire 1)
(add s-deadalus acp-to-fire 0)
(add s-oberth acp-to-fire 0)

(add s-constitution1 range 3)
(add s-constitution2 range 4)
(add s-miranda range 3)
(add s-constellation range 5)
(add s-excelsior range 5)
(add s-ambassador range 6)
(add s-nebula range 7)
(add s-galaxy range 8)

(table acp-to-capture
;; (engineers dead-worlds 1)
)

(table hp-to-garrison
  (engineers civilizations 1)
  (engineers stations 1)
  (engineers o-facilities 1)

  (civ-0 civ-1 25)
  (civ-1 civ-2 50)
  (civ-2 civ-3 100)
)

(table hit-chance
  (u* u* 50)
  (probe u* 0)
  (engineers u* 0)
)

(table damage
  (u* u* 1)
  (probe u* 0)
  (engineers u* 0)
)

(table fire-damage
  (lr-spacecraft u* 2)
)

(table capture-chance
  (armies civilizations 20)
  (armies stations 30)

  (s-constitution1 civilizations 15)
  (s-constitution2 civilizations 15)
  (s-excelsior civilizations 20)
  (s-ambassador civilizations 25)
  (s-galaxy civilizations 30)
)

(table material-to-attack
  (lr-spacecraft antimatter 1)
)

(table consumption-per-attack
  (lr-spacecraft antimatter 1)
)

(table material-to-fire
  (lr-spacecraft torpedoes 1)
)

(table consumption-per-fire
  (lr-spacecraft torpedoes 1)
)

(table hit-by
  (u* antimatter 1)
  (u* torpedoes 1)
)

(table protection
  (armies civilizations 0.40)
  (armies stations 0.50)

  (lr-spacecraft armies 0.00)
)

(table stack-protection
  ;; Few things can wipe out a large military force, even with futuristic technology.
  (armies armies 0.50)
  (armies engineers 0.00)
)

;;; REPAIR

(table auto-repair
  (shipyard sr-spacecraft 2.00)
  (shipyard lr-spacecraft 1.00)

  (outpost sr-spacecraft 1.00)
  (outpost lr-spacecraft 0.50)

  (civilizations armies 1.00)		;; Currently useless; the only army has 1 HP!
)

;;; TOOLING

(table acp-to-toolup
  (u* u* 1)
)

(table tp-to-build
  ;; Make sure that it defaults to zero!
  (u* u* 0)

  ;; It is generally more difficult to build constructive units than to
  ;; produce destructive units.  Therefore, engineers require more tooling
  ;; than armies.
  (civilizations armies 1)
  (civilizations engineers 2)

  ;; The more advanced a starship, the more tooling needed to build it.
  (shipyard vws 4)
  (shipyard s-deadalus 12)
  (shipyard s-oberth 14)
  (shipyard s-constitution1 18)
  (shipyard s-constitution2 20)
  (shipyard s-miranda 22)
  (shipyard s-constellation 24)
  (shipyard s-excelsior 28)
  (shipyard s-ambassador 30)
  (shipyard s-nebula 34)
  (shipyard s-galaxy 38)

  ;; No tooling needed for facilities (they *are* tooling, in their own way).
)

(table tp-max
  ;; Make sure that it defaults to zero!
  (u* u* 0)

  ;; It is generally more difficult to build constructive units than to
  ;; produce destructive units.  Therefore, engineers require more tooling
  ;; than armies.
  (civilizations armies 1)
  (civilizations engineers 2)

  ;; The more advanced a starship, the more tooling needed to build it.
  (shipyard vws 4)
  (shipyard s-deadalus 12)
  (shipyard s-oberth 14)
  (shipyard s-constitution1 18)
  (shipyard s-constitution2 20)
  (shipyard s-miranda 22)
  (shipyard s-constellation 24)
  (shipyard s-excelsior 28)
  (shipyard s-ambassador 30)
  (shipyard s-nebula 34)
  (shipyard s-galaxy 38)

  ;; No tooling needed for facilities (they *are* tooling, in their own way).
)

#| Can't seem to make tooling depend on materials.  That needs to be fixed.
(table material-to-toolup
  (u* ores 1)
)

(table consumption-to-toolup
  (u* ores 1)
)
|#

;;; UPGRADING

(table can-change-type-to 0
  (civ-0 civ-1 true)
  (civ-1 civ-2 true)
  (civ-2 civ-3 true)
)

(table acp-to-change-type
  (civ-0 civ-1 1)
  (civ-1 civ-2 1)
  (civ-2 civ-3 1)
)

;;; SYNTHESIS

(set synthesis-methods '(make-random-terrain make-countries make-independent-units make-initial-materials name-units-randomly))

