(game-module "future"
  (title "2200 AD")
  (blurb "Futuristic game by Jay Scott.")
  ;(version "1.0")
  (variants (world-seen true) (see-all false) (sequential false))
)

(unit-type hovercar (image-name "hovercraft") (char "h")
  (help "easily built, moves fast and captures cities"))
(unit-type groundcar (image-name "tank") (char "g")
  (help "slow but tough, captures cities"))
(unit-type saucer (char "s")
  (help "moves fast, easy to build, but short range"))
(unit-type defender (image-name "delta") (char "d")
  (help "heavy aircraft that defeats subs and saucers"))
(unit-type constructor (image-name "builder") (char "c")
  (help "builds bases anywhere"))
(unit-type mothership (char "m")
  (help "carries aircraft and ground units"))
(unit-type transport-sub (image-name "sub") (char "t")
  (help "quickly built, carries ground units"))
(unit-type attack-sub (image-name "sub") (char "a")
  (help "attacks cities and carries rockets"))
(unit-type rocket (char "R")
  (help "missile that can kill most units and hit cities"))
(unit-type base (char "/") (image-name "saucerpad")
  (help "airstrip + port but no production"))
(unit-type town (image-name "town22") (char "*")
  (help "produces but easily captured and may revolt"))
(unit-type city (image-name "city22") (char "@")
  (help "metropolis - hard to capture"))

(material-type fuel (help "used for both movement and combat"))

(terrain-type sea (char "."))
(terrain-type shallows (char ","))
(terrain-type swamp (char "="))
(terrain-type plains (char "+"))
(terrain-type forest (char "%"))
(terrain-type desert (char "~"))
(terrain-type mountains (char "^"))
(terrain-type ice (char "_"))
(terrain-type vacuum (char ":"))

(define h hovercar)
(define g groundcar)
(define s saucer)
(define d defender)
(define c constructor)
(define m mothership)
(define t transport-sub)
(define a attack-sub)
(define R rocket)
(define B base)
(define T town)
(define @ city)

(define movers ( h g s d c m t a R ))
(define cities ( B T @ ))

(define water-t* (sea shallows))
(define land ( swamp plains forest desert mountains ice ))

(add water-t* liquid true)

;;; Static relationships.

;; Unit-unit.

(table unit-capacity-x
  ;; (shouldn't motherships be able to carry rockets?)
  (m ( h g s d c ) ( 6 4 10 2 1 ))
  (a R 4)
  (t ( h g ) ( 2 1 ))
  )

(add cities capacity ( 8 32 32 ))

(table unit-size-as-occupant
  (u* u* 99)
  (movers cities 1)
  )

;; Unit-material.

(table unit-storage-x (u* fuel ( 30 40 16 16 30 40 200 150 30 500 1000 2000 )))

;;; Vision.

;; Subs have very good stealth.

(table visibility (( t a ) t* 0))

(add cities see-always 1)

;;; Actions.

(add movers acp-per-turn ( 3 1 8 4 3 6 2 3 10 ))
(add cities acp-per-turn ( 0 1 1 ))

;;; Movement.

(add cities speed 0)

(table mp-to-enter-terrain
  (u* t* 99)
  (h plains 1)
  (h shallows 2)
  (g ( plains desert ) 1)
  (( s d c m ) t* 1)
  (( t a ) ( sea shallows ) 1)
  (R t* 1)
  )

(table consumption-per-move (movers fuel 1))

;;; Construction.

;;               h g s d   c  m t a  R
(add movers cp ( 4 6 2 10 10 20 7 12 8 ))
(add B cp 3)

(table can-create
  (c B 1)
  ((T @) movers 1)
  )

(table can-build
  (c B 1)
  ((T @) movers 1)
  )

(table acp-to-create
  (c B 1)
  ((T @) movers 1)
  )

(table acp-to-build
  (c B 1)
  ((T @) movers 1)
  )

(table can-toolup-for
  (c B true)
  ((T @) movers true)
  )

(table acp-to-toolup
  (c B 1)
  ((T @) movers 1)
  )

;; Future hi-tech needs more elaborate infrastructure, so toolup costs
;; are high for some types.

(table tp-to-build
  (c B 1)
  ;;          h  g  s  d  c  m  t  a  R
  (T movers ( 1  1  1  5  9 10  1  2 25))
  (@ movers ( 1  1  1  5  9 10  1  2 25))
  )

(table tp-max
  (c B 1)
  (T movers ( 1  1  1  5  9 10  1  2 25))
  (@ movers ( 1  1  1  5  9 10  1  2 25))
  )

;;; Combat.

(add u* hp-max ( 1 1 1 2 2 5 2 2 1 10 20 40 ))

(table hit-chance 
  (h u* (  65  60  40  50  50  50   5   5  50  99  99  99 ))
  (g u* (  80  60  50  40  50  50  10  10  50  99  99  99 ))
  (s u* (  50  40  70  10  70  90  10  10  99  99  99  99 ))
  (d u* (  10  10  65  20  70  70  50  50  20  99  99  99 ))
  (c u* (  20  20  10   5  20  10   0   0  20   0   0   0 ))
  (m u* (  15  10  20   5  40  40   0   0  30   0   0   0 ))
  (t u* (   0   0   0   0   0   0   0   0   0   0   0   0 ))
  (a u* (  40  60  10  10  20  20  60  20   0  99  99  99 ))
  (R u* (  99  99  60  80  90  90  70  70  20  99  99  99 ))
  (B u* (  10  10  10  20   0   0   0  10   0   0   0   0 ))
  (T u* (  30  30  30  40   0   0   0  20   0   0   0   0 ))
  (@ u* (  50  50  50  50   0   0   0  50   0   0   0   0 ))
  ;; Guarantee that the rocket will be destroyed in its attack.
  (u* R 100)
  )

(table damage 
  (u* u* 1)
  (a cities 3)
  (R u* 4)
  (R cities 10)
  (@ u* 2)
  )

(table capture-chance
  (h cities ( 80 70 20 ))
  (g cities ( 90 80 30 ))
  )

(table consumption-per-attack (u* fuel 1))

(table hit-by (u* fuel 1))

;;; Other actions.

;; We can always disband units freely.

(add u* acp-to-disband 1)

(add u* hp-per-disband 100)

;;; Backdrop economy.

(table base-production (cities fuel 10))

(table base-consumption
  (( s d ) fuel 1)
  (u* fuel 1)	; not plausible, but helps machine players (???)
  )

(table hp-per-starve
  ;; High-tech stuff disintegrates without fuel.
  (u* fuel 1.00)
  )

(table out-length
  (movers fuel -1)  	; so low-capacity units don't lose fuel
  (cities fuel 1)
  )

(table in-length
  (u* fuel 1)
  )

;;; Scoring.

(add cities point-value ( 1 5 25 ))

(scorekeeper (do last-side-wins))

;;; Setup.

(add t* alt-percentile-min (   0  69  70  70  70  70  95  99   0 ))
(add t* alt-percentile-max (  69  70  72  95  95  95  99 100   0 ))
(add t* wet-percentile-min (   0   0  50  20  80   0   0   0   0 ))
(add t* wet-percentile-max ( 100 100 100  80 100  20 100 100   0 ))

(set edge-terrain ice)

(add cities start-with ( 0 2 1 ))

(table independent-density (T land 100))

(table favored-terrain
  (u* t* 0)
  (( T @ ) ( plains ) 100)
  )

;; A game's starting units will be full by default.

(table unit-initial-supply (u* m* 9999))


;;; Sides

(set sides-wanted 4)

;;; Names and features

(include "ng-features")

(add (base town city) namer "short-generic-names")

(set feature-types '(continents islands seas lakes bays
			    (desert 10)(forest 10)(mountains 5) peaks))

(set feature-namers
  '((islands generic-island-names) (lakes generic-lake-names) (bays generic-bay-names)
  	(seas generic-sea-names) (continents generic-continent-names)
  	(desert generic-desert-names) (forest generic-forest-names)
  	(mountains generic-mountain-names)))


;;; Documentation.

(game-module (notes (
"A science fiction Xconq period.
It's weirder and wilder than the historical periods provided with the game.
Lots of things happen faster, so the game is often shorter.
"
"This is in the public domain.
"
" -----  -----  strategy  -----  -----
"
"The game is designed so you need to have at least a few of every
kind of unit to do well in a full game.  (You may be able to get by
without transport subs if you start out on a big continent.)  If
anybody finds they can consistently do well without some kind of unit,
I want to hear how so I can fix it!
"
"Every unit has at least one nemesis which can destroy it relatively
easily.  Your goal should be to fight every battle at an advantage,
pitting each unit against its natural prey--saucers against hovercars,
defenders against saucers, hovercars against defenders.
"
"Rockets ensure that the game doesn't drag on too long.  Invasion is
risky, but when it works the invader wins quickly.  Rockets by contrast
are a slow but steady way to nibble at the enemy production base.
"
"Machine players are especially easy to defeat in this period.  If you're
lucky enough to start near one, you can blitz it with hovercars and
saucers and win in short order.  It takes longer if you're far from
the robot.  Gaining air superiority is usually the first step.
"
"The game hasn't been played by enough people for me to tell what
strategies are best in different circumstances.  If I've done my
job well, the best plan will depend in detail on the opponent and
the situation, and you'll have to think hard.
"
"I'd appreciate any comments.

	Jay Scott, August 1987.
	...bpa!swatsun!scott
	   ...seismo!bpa!swatsun!scott"
)))

(add hovercar notes '(
"Hovercar.  A fast-moving ground unit that easily captures towns.  A
hovercar floats on an antigravity field, so it can maneuver easily
even in mountainous terrain or shallow water (though not deep ocean).
Hovercars are invaluable in an invasion for their ability to take
cities quickly, but they are easily destroyed by flying saucers or
groundcars."
))

(add groundcar notes '(
"Groundcar.  A slow-moving unit which travels on treads, like today's
tanks.  It can afford to carry a large shield generator, which makes
it tough to destroy.  It can negotiate forests by pushing down the
trees, or burning them away with its gun, but it can't move in
mountains.  Groundcars are ideal for defending your homeland from
invasion.  If you have enough on hand, you can often bounce back even
after many of your towns are captured."
))

(add saucer notes '(
"Saucer.  The flying saucer is a weak but fast-moving aircraft.  Saucers
are very cheap to produce; one use is to overwhelm stronger units with
mob attacks.  They can even bombard a city to rubble, unless there's
a Defender around.  Saucers are also good for recon, within their limited
range."
))

(add defender notes '(
"Defender.  A heavy, delta-wing aircraft that's your only safe defense
against the enemy's swarms of saucers.  It uses aerodynamic lift to 
supplement its drive, allowing it to carry relatively massive
antisubmarine equipment.  But it's an easy target for ground units."
))

(add constructor notes '(
"Constructor.  An automated airborn factory that can build a base from
on-site materials in only one turn.  It can build a base anywhere, even
on water or ice.  That means, for instance, that you can build bridges
between islands for ground units.  The constructor is the only way
to produce the sophisticated equipment needed for a twenty-second
century base (you don't know how hard it is to refuel those saucers :-).
But it's vulnerable to attack, especially from saucers."
))

(add mothership notes '(
"Mother ship.  Expensive to build, but the only way to mount a major
invasion.  A mother ship can carry lots of saucers, lots of ground
units, a couple Defenders and a constructor, all at once.  (The constructor
is useful for building stepping stone bases toward the enemy.)
A mass attack is sure to bring down a mother ship, so it deserves
saucer patrols--but you may have to forego patrols if you're trying
for a surprise invasion."
))

(add transport-sub notes '(
"Transport sub.  Carries only a few ground units, but much cheaper to
produce than a mother ship.  This is how you get your troops to another
island early in the game.  And since, like subs in the WWII period,
it's invisible until bumped into, it's good for sneak raids on isolated
outposts.  A transport submarine can't attack anything by itself."
))

(add attack-sub notes '(
"Attack sub.  Strong against transport subs, and, if they're on the coast,
ground units and cities.  It stays underwater and pokes its weapons out,
which makes it relatively invulnerable to counterattack.  It's faster
than a transport sub, and good for wide-ranging exploration.  And to top
it off, it can carry several rockets to within easy range of enemy cities.
On the other hand, all submarines have to be careful of Defenders."
))

(add rocket notes '(
"Rocket.  Actually a remotely guided missile, which does a lot of damage
to its target but vaporizes itself in the process.  One rocket is
enough to take out most units--it's not quite enough by itself
to down a mother ship.  Three rockets together are sufficient to pound
a town back to a base.  Only occasionally will a rocket miss its target.
Besides stomping cities, they're useful for eliminating enemy groundcars
and Defenders just before an invasion.  Since rockets are easily
shot down if spied in mid-flight, it's sensible to launch them from
attack subs near their targets.  [Because of the way Xconq works,
messages about rockets don't always make sense.]"
))

(game-module (instructions (
    "A science fiction Xconq period."
    "It's weirder and wilder than the historical periods provided with the game."
    "Lots of things happen faster, so the game is often shorter."
  )))

(game-module (design-notes (
  )))
