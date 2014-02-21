(game-module "solar" 
  (title "Solar Regions Explorer")
  (blurb "Matthew Skala's demonstration game for backdrop model 1")
  (instructions (
    "This is still very much a testbed; most stuff is unsupported.  The AI"
    "hasn't a clue how to play, and probably won't for quite a while,"
    "because the game depends on a lot of tricky rule (ab)use."
    ""
    "The first thing you'll probably want to do is wait, until you have"
    "at least about 100 support.  Then you'll be able to build your first"
    "Apollo mission.  Those burn support faster than you can produce it,"
    "so you can't have one active all the time, but they also produce a"
    "lot of science, which you'll need in order to build Mariner missions."
    "Mariner missions are for exploring planets in the first two orbits from"
    "the star.  Park one on a planet and it'll radio science back home; in"
    "the long run, that approach is cheaper than increasing your science by"
    "way of Apollo missions.  Voyager missions are similar but generate"
    "even more science; however, they only work in the outer rings of solar"
    "systems, and they will drain your support if used too close to your"
    "colony.  Read the help for other unit types to find out about their"
    "special properties."
  ))
  (variants
    (see-all false)
    (world-size (240 120 1440))
))

; BACKDROP

(set backdrop-model 1)
(set backdrop-ignore-doctrine true)

; TERRAIN TYPES

; Terrain works like this:
; two-letter code, one-digit (or n or no) series
; series is 0 for empty space, 1 for empty space in an orbit with planet
; 2..6 for planet or protoplanet
; n for nebula
; no for nebula with orbit
; code is ds for deep space, st for star, nb for nebula
; is for inner solar system
; ms for mid molar system
; os for outer solar system
; replace series with "all" for glob list

; special types:
; ds1 is similar to ds0 but more likely to become a star
; ms5 is asteroid belt; eats the rest of the ms series

; define nebula first because it overrides space in fractal-terrain
(terrain-type dsn (help "nebula in deep space"))
(terrain-type isn (help "nebula in inner solar system"))
(terrain-type msn (help "nebula in middle solar system"))
(terrain-type osn (help "nebula in outer solar system"))
(terrain-type isno (help "nebula in inner solar system w/orbit"))
(terrain-type msno (help "nebula in middle solar system w/orbit"))
(terrain-type osno (help "nebula in outer solar system w/orbit"))

(terrain-type ds0 (help "deep space, type 0"))
(terrain-type ds1 (help "deep space, type 1"))
(terrain-type ds2 (help "deep space, type 2"))
(terrain-type ds3 (help "deep space, type 3"))
(terrain-type ds4 (help "deep space, type 4"))
(terrain-type ds5 (help "deep space, type 5"))
(terrain-type ds6 (help "deep space, type 6"))

(terrain-type is0 (help "empty space in inner solar system"))
(terrain-type is1 (help "space w/orbit in inner solar system"))
(terrain-type is2 (help "hot, thin-atmosphere planet (Mercury type)"))
(terrain-type is3 (help "volcanic-surface planet"))
(terrain-type is4 (help "thick-atmosphere (Venus type)"))
(terrain-type is5 (help "Earth-like planet, less hospitable"))
(terrain-type is6 (help "Earth-like planet, more hospitable"))

(terrain-type ms0 (help "empty space in middle solar system"))
(terrain-type ms1 (help "space w/orbit in middle solar system"))
(terrain-type ms2 (help "asteroid-like planet (Ceres type)"))
(terrain-type ms3 (help "cold planet, minimal atmosphere (Luna type)"))
(terrain-type ms4 (help "icy planet"))
(terrain-type ms5 (help "asteroid belt"))
(terrain-type ms6 (help "desert planet, thin atmosphere (Mars type)"))

(terrain-type os0 (help "empty space in outer solar system"))
(terrain-type os1 (help "space w/orbit in outer solar system"))
(terrain-type os2 (help "asteroid-like planet (Ceres type)"))
(terrain-type os3 (help "cold gas giant (Neptune type)"))
(terrain-type os4 (help "gas giant with ring (Saturn type)"))
(terrain-type os5 (help "warm gas giant (Jupiter type)"))
(terrain-type os6 (help "terrestrial planet in outer solar system"))

(terrain-type st0 (help "yellow/white main sequence star"))
(terrain-type st1 (help "red main sequence star"))
(terrain-type st2 (help "blue main sequence star"))
(terrain-type st3 (help "white dwarf star"))
(terrain-type st4 (help "red giant star"))
(terrain-type st5 (help "red supergiant star"))
(terrain-type st6 (help "blue giant star"))

(terrain-type orbit (subtype connection) (image-name "solar-orbit")
  (help "dynamical space shared by one or more planets"))

(terrain-type warp-path (subtype connection) (subtype-x road-x)
  (image-name "cyan")
  (help "feature of deep space allowing hyperwarp travel"))

; TERRAIN TYPE GROUPS

(define dsall (ds0 ds1 ds2 ds3 ds4 ds5 ds6 dsn))
(define isall (is0 is1 is2 is3 is4 is5 is6 isn isno))
(define msall (ms0 ms1 ms2 ms3 ms4 ms5 ms6 msn msno))
(define osall (os0 os1 os2 os3 os4 os5 os6 osn osno))
(define stall (st0 st1 st2 st3 st4 st5 st6))

(define nebula (dsn isn msn osn isno msno osno))

(define gas-giants (os3 os4 os5))
(define connections (orbit warp-path))

; IMAGES

(add dsall image-name "deep-space")
(add nebula image-name "solar-nebula")

(add is2 image-name "solar-reddish")
(add is3 image-name "solar-volcanic")
(add is4 image-name "solar-venus")
(add is5 image-name "solar-earthsea")
(add is6 image-name "solar-earthland")

(add ms2 image-name "solar-onerock")
(add ms3 image-name "solar-moon")
(add ms4 image-name "solar-cracked")
(add ms5 image-name "solar-asteroids")
(add ms6 image-name "solar-mars")

(add os2 image-name "solar-onerock")
(add os3 image-name "solar-neptune")
(add os4 image-name "solar-saturn")
(add os5 image-name "solar-jupiter")
(add os6 image-name "solar-cracked")

(add st0 image-name "solar-wstar")
(add st1 image-name "solar-rstar")
(add st2 image-name "solar-bstar")
(add st3 image-name "solar-wdwarf")
(add st4 image-name "solar-rsuper")
(add st5 image-name "solar-rgiant")
(add st6 image-name "solar-bgiant")

(add (ds0 ds1) image-name "deep-space")
; (add (ds0 ds1) image-name "gray") 
; (add (is0 is1) image-name "aquamarine")
; (add (ms0 ms1) image-name "cyan")
; (add (os0 os1) image-name "cornflower-blue")
(add (is0 ms0 os0) image-name "deep-space")
(add (is1 ms1 os1) image-name "deep-space")

(imf "deep-space" ((44 48 terrain) (x 8 46 0) (file "solar.gif" std 0 0)))

(imf "solar-nebula" ((44 48 terrain) (x 8 46 0) (file "solar.gif" std 0 1)))

(imf "solar-wstar" ((44 48 terrain) (file "solar.gif" std 0 2)))
(imf "solar-rstar" ((44 48 terrain) (file "solar.gif" std 1 2)))
(imf "solar-bstar" ((44 48 terrain) (file "solar.gif" std 2 2)))
(imf "solar-wdwarf" ((44 48 terrain) (file "solar.gif" std 3 2)))
(imf "solar-rsuper" ((44 48 terrain) (file "solar.gif" std 4 2)))
(imf "solar-rgiant" ((44 48 terrain) (file "solar.gif" std 5 2)))
(imf "solar-bgiant" ((44 48 terrain) (file "solar.gif" std 6 2)))

(imf "solar-asteroids" ((44 48 terrain) (x 8 46 0) (file "solar.gif" std 0 3)))
; Earth
(imf "solar-earthland" ((44 48 terrain) (x 2 46 0) (file "solar.gif" std 0 4)))
; Callisto, Io, Venus radar
(imf "solar-volcanic" ((44 48 terrain) (x 3 46 0)  (file "solar.gif" std 4 4)))
; Luna, Luna, Mimas
(imf "solar-moon" ((44 48 terrain) (x 3 46 0)  (file "solar.gif" std 0 5)))
; Dione, Europa
(imf "solar-cracked" ((44 48 terrain) (x 2 46 0)  (file "solar.gif" std 4 5)))
; Europa, Ganymede, Mars, Rhea
(imf "solar-reddish" ((44 48 terrain) (x 4 46 0)  (file "solar.gif" std 0 6)))
; Amalthea, Thebe, Amalthea, Ida
(imf "solar-onerock" ((44 48 terrain) (x 4 46 0)  (file "solar.gif" std 4 6)))
; Earth
(imf "solar-earthsea" ((44 48 terrain) (file "solar.gif" std 0 7)))
; Mars
(imf "solar-mars" ((44 48 terrain) (file "solar.gif" std 4 7)))
; Saturn
(imf "solar-saturn" ((44 48 terrain) (x 2 46 0)  (file "solar.gif" std 0 8)))
; Jupiter
(imf "solar-jupiter" ((44 48 terrain) (file "solar.gif" std 4 8)))
; Neptune
(imf "solar-neptune" ((44 48 terrain) (file "solar.gif" std 0 9)))
; Venus
(imf "solar-venus" ((44 48 terrain) (x 2 46 0)  (file "solar.gif" std 4 9)))

(imf "solar-orbit" ((1 1) 65535 65535 65535))
(imf "solar-orbit" ((44 48 connection 2) (file "thinline.gif" 2 2)))

; Apollo 
(imf "solar-apollo" ((88 88) (file "solunits.gif" std 0 0)))
; Mariner
(imf "solar-mariner" ((88 88) (file "solunits.gif" std 1 0)))
; Voyager
(imf "solar-voyager" ((88 88) (file "solunits.gif" std 2 0)))
; colour-edited fishing lure
(imf "solar-leech-adult" ((88 88) (file "solunits.gif" std 3 0)))
; colour-edited fishing lure
(imf "solar-leech-baby" ((88 88) (file "solunits.gif" std 4 0)))
(imf "solar-rad-black" ((88 88) (file "solunits.gif" std 0 1)))
(imf "solar-rad-pink" ((88 88) (file "solunits.gif" std 1 1)))
; XMM-Newton x-ray telescope
(imf "solar-xmm-newton" ((88 88) (file "solunits.gif" std 2 1)))
; 659Hz tuning fork out of Wikipedia
(imf "solar-tuning-fork" ((88 88) (file "solunits.gif" std 3 1)))
; Delta Clipper DC-X experimental rocket
(imf "solar-delta-clipper" ((88 88) (file "solunits.gif" std 4 1)))

; MATERIALS

(material-type fuel)
(material-type support)
(material-type science)
(material-type neutronium)

; UNITS

; habitations
(unit-type outpost)
(unit-type settlement)
(unit-type colony)
(unit-type civilization)
(unit-type cloud-city)
(unit-type domed-city)
(define habitations (outpost settlement colony civilization
  cloud-city domed-city))

; exploration missions
;    Apollo - doesn't actually move, just soaks up money and produces science.
(unit-type Apollo)
;    Mariner - explores inner and mid solar system
(unit-type Mariner)
;    Voyager - explores outer solar system and near deep space
(unit-type Voyager)
(define missions (Apollo Mariner Voyager))

; stations/facilities
;    neutronium station
(unit-type neu-station)
;    neutronium plant
(unit-type neu-plant)
;    unstable neutronium plant
(unit-type u-neu-plant)
;    transport base
(unit-type transport-base)
;    deep space relay
(unit-type relay)
;    starbase
(unit-type starbase)
(define stations (neu-station neu-plant u-neu-plant transport-base relay
  starbase))

; in-system ships
;    fighter
(unit-type fighter)
;    orbital shuttle
(unit-type orbital-shuttle)
;    solar sail
(unit-type solar-sail)
;    interplanetary barge
(unit-type barge)
(define in-system (fighter orbital-shuttle solar-sail barge))


; ground units
;    marines
(unit-type marines)
;    colonists
(unit-type colonists)
;    engineers
(unit-type engineers)
(define ground (marines colonists engineers))

; starships
;    sleeper - very slow, unarmed, only transports colonists
(unit-type sleeper)
;    cheap
;    fast
;    armed
;    capacious
;    fast and armed
;    fast and capacious
;    armed and capacious
;    cheap hyperwarp
;    armed hyperwarp
;    capacious hyperwarp
(define starships (sleeper))

; independents
;    black hole
(unit-type black-hole)
;    giant space leech
(unit-type gsl)
;    gsl egg
(unit-type gsl-egg)

; UNIT TABLE DEFAULTS

; I think these generally are system defaults anyway, but mentioned for
; safety.  I want to describe units one unit at a time instead of one table
; at a time, so I start with defaults and then use table add.

(table favored-terrain (u* t* 0))
(table independent-density (u* t* 0))
(table acp-to-change-type (u* u* 0))

(table unit-storage-x (u* m* 0))
(table base-production (u* m* 0))
(table base-consumption (u* m* 0))
(table occupant-base-production (u* m* -1))

(table acp-to-create (u* u* 0))
(table acp-to-build (u* u* 0))
(table consumption-per-cp (u* m* 0))
(table cp-on-creation (u* u* 0)) ; we want precise cp costs, dammit!

(table in-length (u* m* -1))
(table out-length (u* m* -1))

(table terrain-to-unit-in-length (u* m* -1) (u* science 0))
(table terrain-to-unit-out-length (t* (fuel science) 0))
(table unit-to-terrain-in-length (t* (fuel science) 0))
(table unit-to-terrain-out-length (u* m* -1) (u* science 0))

(add t* capacity 12)
(add connections capacity 0)
(table unit-size-in-terrain (u* t* 1))
(table terrain-capacity-x (u* t* 0))

; movement just isn't allowed unless we explicitly specify it
(table mp-to-enter-terrain (u* t* 999))
(table mp-to-traverse (u* t* 0))

; DEFAULT VISION

; these numbers are carefully tweaked.  The idea is that in-system units can
; only see units within their own systems, but can see stars from afar, and
; the near sides of nebulas.
(area (cell-width 10))
; default vision range is basically to the other side of the solar system
(add u* vision-range 6)
(add u* can-see-behind 0)
(add t* thickness 0)
(add dsall thickness 100)
(add nebula thickness 1000)
(add stall thickness 10000)

; SCIENCE PRODUCTION

; this is here because it specifies defaults; see "material diffusion" later
(table base-production add (u* science 1))

; HABITATIONS
; habitations like to live on planets in the first couple rings of the system
(add habitations image-name
   ("parthenon" "city19" "city20" "city22" "space-station" "domed-city"))

(add (outpost domed-city cloud-city) cp (10 20 30))

; they tend to be found on planets, according to type
(table favored-terrain add
  (habitations is2 (0  0  25 2 0  0))
  (habitations is3 (0  0  10 1 0  0))
  (habitations is4 (0  0   1 0 2  2))
  (habitations is5 (1 10 100 0 0  0))
  (habitations is6 (1 10 100 0 0  0))

  (habitations ms4 (1  1  20 2 0  5))
  (habitations ms6 (0  0   0 5 0 50))

  (habitations os3 (0  0   0 0 2  0))
  (habitations os4 (0  0   0 0 2  0))
  (habitations os5 (0  0   0 0 2  0))
  (habitations os6 (0  0   2 0 0  1))
)

(table independent-density add
  (habitations is2 (100  200 1000 100    0    0))
  (habitations is3 ( 50  100  500 100    0    0))
  (habitations is4 (  0    0  100  10 1000  500))
  (habitations is5 (200 1000 3000 100    0    0))
  (habitations is6 (200 1000 3000   0    0    0))

  (habitations ms4 (100  200 1000 100    0  500))
  (habitations ms6 ( 20   50  100 100    0 3000))

  (habitations os3 (  0    0    0   0 1000    0))
  (habitations os4 (  0    0    0   0 1000    0))
  (habitations os5 (  0    0    0   0 1000    0))
  (habitations os6 ( 20   40  200  20    0  100))
)

; and they're most productive on hospitable planets
(table productivity add
  (habitations is2 ( 80%  80%  80%  80%  30% 100%))
  (habitations is3 ( 30%  30%  30%  30%  30%  80%))
  (habitations is4 ( 10%  10%  10%  10%  80%  80%))
  (habitations is5 (100% 100% 100% 100%  30%  80%))
  (habitations is6 (115% 115% 115% 115%  30%  80%))

  (habitations ms2 (  5%   5%   5%   5%   5%  30%))
  (habitations ms3 ( 10%  10%  10%  10%   5%  50%))
  (habitations ms4 ( 30%  30%  30%  30%  10%  80%))
  (habitations ms5 (  5%   5%   5%   5%  30%  50%))
  (habitations ms6 ( 80%  80%  80%  80%  30% 150%))

  (habitations os2 (  5%   5%   5%   5%   5%  30%))
  (habitations os3 ( 10%  10%  10%  10% 100%  10%))
  (habitations os4 ( 10%  10%  10%  10% 100%  20%))
  (habitations os5 (  5%   5%   5%   5% 125%   5%))
  (habitations os6 ( 30%  30%  30%  30%  10%  80%))
)

; and they can't exist except on planets
(table vanishes-on add
   (habitations t* 1)
   (habitations
    (is2 is3 is4 is5 is6 ms2 ms3 ms4 ms5 ms6 os2 os3 os4 os5 os6) 0)
)

; habitations are ACP-independent and can't move
(add habitations acp-independent true)
(add habitations speed 0)

; they store a fair bit of material
(table unit-storage-x add
  (outpost m* 50)
  ((settlement domed-city) m* 250)
  ((colony cloud-city) m* 1000)
  (civilization m* 5000)
  (habitations science 10000)
)

; they produce support and all but the smallest also produce fuel
(table base-production add
   ((outpost domed-city cloud-city) support 1)
   ((settlement colony civilization) support (2 5 10))
   ((settlement colony civilization) fuel (1 1 3))
)

; they can pick up fuel from current cell, and the larger ones can pick up
; science from some distance away
(table terrain-to-unit-in-length add
  (habitations fuel 0)
  ((outpost cloud-city) science 0)
  ((settlement domed-city) science 1)
  (colony science 2)
  (civilization science 3)
)

; they can hold lots of other units, but nothing else can be in the cell
; (mostly for cosmetic reasons), and they can't hold other habitations
(add habitations capacity 100)
(table unit-size-in-terrain add (habitations t* 12))
(table unit-size-as-occupant add (habitations habitations 999))

; smallest habitations may not be able to see far, cloud cities have trouble
; because of, well, clouds, others can see right to the edge of the map
; but terrain thickness means that despite having wide range, all of these
; can only see stars and the near sides of nebulae, outside the local system
(add habitations vision-range (60 120 240 240 10 240))

; habitations can refuel other units in same cell, can generally receive
; stuff from far away, can provide support pretty far but can't receive it
; except from same cell
(table in-length add
  (habitations m* 36)
  (habitations support 0)
)
(table out-length add
  (habitations fuel 0)
  (habitations support 12)
  (habitations science 4)
  (habitations neutronium 12)
)

; HABITATION UPGRADES

(table acp-to-change-type add
  (outpost settlement 1)
  (settlement colony 1)
  (colony civilization 1)
)
(add outpost auto-upgrade-to settlement)
(add settlement auto-upgrade-to colony)
(add colony auto-upgrade-to civilization)
(table material-to-change-type add
   (outpost (support science) 50)
   (settlement (support science) 250)
   (colony (support science) 1000)
   ((outpost settlement colony) fuel 45)
   (colony neutronium 1)
)

; HABITATION CONSTRUCTION CAPABILITY

(table can-create add
  (habitations (append missions in-system ground starships) 1)
  (outpost starships 0)
)
(table can-build add
  (habitations (append missions in-system ground starships) 1)
  (outpost starships 0)
)
(table acp-to-create add
  (habitations (append missions in-system ground starships) 1)
  (outpost starships 0)
)
(table acp-to-build add
  (habitations (append missions in-system ground starships) 1)
  (outpost starships 0)
)

; AUTO-REPAIR BY HABITATIONS

(table consumption-per-repair add (u* (fuel support) 0.5))
(table auto-repair add (habitations u* 0.5))

; STATIONS/FACILITIES
; (define stations (neu-station neu-plant u-neu-plant transport-base relay
;   starbase))

(add neu-station notes (
  "Admiral Berzelius examined the briefing report with extreme distaste."
  "He could feel his ulcer starting to act up again.  \"So all three Phi Cetus"
  "neutronium plants have gone unstable.\""
  ""
  "The assembled commanders shifted uneasily in their seats.  \"I'm afraid"
  "so, sir,\" said the author of the report. \"We're evacuating the"
  "population from the Phi Cetus system right now.\""
  ""
  "\"Well, we've got a war to fight out here!  How are we going to fuel the"
  "Fourth Fleet?  I don't have the engineers to build another set of"
  "neutronium plants before the Gzidians overrun the front, even if those"
  "idiots in Congress could fast-track approve us to put the plants in"
  "Akdor where there's actually enough solar energy to run them.  Can anyone"
  "here tell me where we're going to get a supply of neutronium on short"
  "notice?  Can't have hyperwarp without neutronium, dammit!\""
  ""
  "From the far end of the table a timid voice wavered, \"Well, sir,"
  "neutronium does occur naturally.  We don't have to manufacture it."
  "If we built a collection station within two parhex of a black hole,"
  "we'd have a virtually unlimited supply; and there's a nice convenient"
  "black hole right alongside Nebula N31.  The station can beam-refuel"
  "ships that come within a parhex, without their needing to dock.\""
  ""
  "\"Fine.  Get on it.\""
  ""
  "The Admiral's second-in-command shook his head.  \"Sir, it's not that"
  "easy.  You know the no-return radius for a black hole with our current"
  "drive technology, don't you?\""
  ""
  "\"Of course I know that.  Two parhex.\"  He paused, and then said"
  "quietly, \"Oh.\"  After thinking about it a few more seconds, he said,"
  "\"Well, then, we'll have to get on the subspace to Congress for that"
  "Akdor approval and make do somehow.  We've lost too many good men in this"
  "war already, and God damn it, I will not allow any of you to volunteer for"
  "a suicide mission.\""
))

(add neu-plant image-name "solar-rad-black")
(add u-neu-plant image-name "solar-rad-pink")
(add neu-station image-name "solar-xmm-newton")

(define buildable-stations (remove u-neu-plant stations))

(add buildable-stations cp (30 10 20 30 50))

(table unit-storage-x add
  (stations fuel (0 100 100 1000 3000 10000))
  (stations support (0 10 10 1000 0 0))
  (stations science (0 100 100 1000 3000 10000))
  (stations neutronium (5000 100 100 100 300 1000))
)

(table base-consumption add
  ((neu-plant u-neu-plant) fuel 15)
  ((neu-plant u-neu-plant) support 1)
)

(table base-production add
  (starbase fuel 5)
  (neu-plant neutronium 3)
  (u-neu-plant neutronium 1)
)

(table unit-size-in-terrain add
  ((neu-station transport-base relay starbase) t* 7)
)

(add stations hp-max 10)

(table hp-per-starve add (neu-plant (fuel support) 2.50))

(table in-length add
  (neu-station neutronium 2)
  ((neu-plant u-neu-plant) fuel 0)
  ((neu-plant u-neu-plant) support 4)
  (transport-base (fuel science neutronium) 4)
  (relay (fuel science neutronium) 8)
  (starbase (fuel science neutronium) 12)
)

(table out-length add
  (neu-station neutronium 2)
  (transport-base (fuel science neutronium) 4)
  (relay (fuel science neutronium) 8)
  (starbase (fuel science neutronium) 12)
)

(table terrain-to-unit-in-length add
  (neu-station neutronium 2)
  ((neu-plant u-neu-plant) fuel 1)
)

(table unit-to-terrain-out-length add
  ((neu-station neu-plant u-neu-plant) neutronium 0)
)

; MISSIONS

; Apollo - explore space within the hex (moons, etc.)  Produces lots of
; science, but eats support.  Can be disbanded.

; Mariner - explore inner and middle solar system.  Produces science when on
; a planet cell, radios it back home

; Voyager - explore outer solar system and nearby deep space.  Produces lots
; of science on planet cells and radios it back home.  Soaks up support if
; it can get it - so you want to send it out of range for that.  Can produce
; own fuel, so can go arbitrarily far away, but will eventually die without
; support.

(add missions image-name ("solar-apollo" "solar-mariner" "solar-voyager"))

(add missions cp (10 15 35))
(table consumption-per-cp add
   (missions (support fuel) 1)
   (missions science (0 5 10))
)

(table unit-storage-x add
  (Apollo (fuel support) 30)
  (Mariner (fuel support) (12 5))
  (Voyager (fuel support) (20 70))
)

(table base-consumption add
  (Apollo (fuel support) (5 10))
  (Mariner (fuel support) (1 3))
  (Voyager support 7)
)

(table consumption-as-occupant add
  (Mariner (fuel support) (0% 35%))
  (Voyager support 29%)
)

(table base-production add
  (missions science (20 50 100))
  (Voyager fuel 1)
)

(table occupant-base-production add
  ((Mariner Voyager) (fuel science) 0)
)

(table productivity add
   (Mariner t* 5%)
   (Mariner (is1 is2 is3  is4  is5  is6 isn isno)
            (10% 50% 75% 100% 150% 250% 30%  40%))
   (Mariner (ms1 ms2 ms3  ms4 ms5  ms6 msn msno)
            (10% 50% 75% 100% 30% 250% 30%  40%))
   (Voyager t* 10%)
   (Voyager (os1 os2  os3  os4  os5  os6 osn osno)
            (10% 50%  75% 100% 150% 250% 30%  40%))
   (Voyager dsall 20%)
   (Voyager dsn 30%)
)

(table unit-size-in-terrain add
   (missions t* 1)
   (Mariner (is1  is2  is3  is4  is5  is6 isn isno)
            (  1    7    7    7    7    7   4    4))
   (Mariner (ms1  ms2  ms3  ms4  ms5  ms6 msn msno)
            (  1    7    7    7    4    7   4    4))
   (Voyager (os1  os2  os3  os4  os5  os6 osn osno)
            (  1    7    7    7    7    7   4    4))
)

(add missions hp-max 3)

(table hp-per-starve add
  ((Apollo Mariner) (fuel support) 1.00)
  (Voyager (fuel support) 0.04)
)

(table in-length add
  (Apollo (fuel support) 0)
  (Mariner (fuel support science) (0 8 5))
  (Voyager (fuel support science) (0 8 5))
)

(table out-length add
  (Mariner science 8)
  (Voyager science 36)
)

(table terrain-to-unit-in-length add (missions science (1 -1 0)))

(add missions acp-per-turn (1 3 4))
(add missions free-mp (0 1 2))

(add Apollo acp-to-disband 1)
(add Apollo hp-per-disband 3)

(table consumption-per-move add (missions fuel 1))

(table mp-to-enter-unit add (missions u* 999))

(table mp-to-enter-terrain add
  (Mariner isall 3)
  (Mariner msall 3)
  (Mariner orbit 1)
  (Voyager isall 4)
  (Voyager msall 4)
  (Voyager osall 4)
  (Voyager dsall 4)
  (Voyager orbit 1)
)

; IN-SYSTEM UNITS

; FIXME images for in-system units
(add orbital-shuttle image-name "solar-delta-clipper")

(add in-system cp (10 15 50 50))
(table consumption-per-cp add
   (in-system (support fuel) 3)
)

(table unit-storage-x add
  (in-system fuel (50 20 100 20))
  (in-system support 1)
)

; fighters burn a fair bit of fuel; others only use it when they move
(table base-consumption add
  (in-system support 1)
  (fighter fuel 2)
)
(table consumption-as-occupant add
  (in-system support 100%)
  (fighter fuel 50%)
)

; solar sails don't consume fuel, they produce it!
(table base-production add (solar-sail fuel 4))
(table occupant-base-production add (solar-sail fuel 0))
(table productivity add
  (solar-sail isall 150%)
  (solar-sail msall 100%)
  (solar-sail osall 50%)
)

(table unit-size-in-terrain add (solar-sail t* 5))

(add in-system hp-max (3 5 10 10))
(table hp-per-starve add
  (fighter fuel 1.50)
  (in-system support 0.00)
)

(table in-length add (in-system (fuel support) 0))
(table out-length add (solar-sail fuel 0))
(table terrain-to-unit-in-length add (solar-sail fuel 1))

(add in-system acp-per-turn (4 4 1 2))
(add in-system free-mp (2 2 0 1))
(add (solar-sail barge) acp-max 4)

(table consumption-per-move add ((fighter orbital-shuttle barge) fuel 1))

(table mp-to-enter-unit add (in-system habitations 1))

(table mp-to-enter-terrain add
  (in-system isall 4)
  (in-system msall 4)
  (in-system osall 4)
  (orbital-shuttle t* 999)
  (in-system orbit 1)
)

(table unit-size-as-occupant add
  (u* in-system 999)
  (ground in-system 1)
)

(add in-system capacity (0 2 1 8))

; STARSHIPS

(add sleeper image-name "solar-tuning-fork")

(add sleeper vision-range 1)

(add starships cp 20)
(table consumption-per-cp add
   (starships (support fuel) 5)
   (starships science 100)
)

(table unit-storage-x add
   (sleeper fuel 50)
)

(table base-production add
  (starships science 3)
)

(add starships hp-max 10)

(table in-length add (starships fuel 0))

(add starships acp-per-turn 2)
(add starships free-mp 0)
(add sleeper acp-max 8)

(table consumption-per-move add (sleeper fuel 1))

(table mp-to-enter-unit add (starships habitations 1))

(table mp-to-enter-terrain add
  (starships isall 2)
  (starships msall 2)
  (starships osall 2)
  (starships orbit 1)
  (starships dsall 8)
)

; sleeper can carry 6 units of colonists - only
(table unit-size-as-occupant add
  (u* starships 999)
  (colonists sleeper 1)
)
(add starships capacity 6)

; GROUND UNITS

; marines, colonists, engineers

(add ground cp (10 50 25))

(table base-production add (ground science 0))

(table consumption-per-cp add
   (ground (support fuel) 2)
   (colonists science 10)
)

(add ground hp-max 10)

(add ground acp-per-turn 1)

(add ground move-range 0)

(table mp-to-enter-unit add (ground u* 1))

(table mp-to-enter-terrain add (ground t* 0))
(table mp-to-leave-terrain add (ground t* 999))
(table ferry-on-departure add (u* ground over-nothing))
(table ferry-on-entry add (u* ground over-nothing))

; COLONIST CONSTRUCTION

(table can-create add (colonists (outpost domed-city cloud-city) 1))
(table can-build add (colonists (outpost domed-city cloud-city) 1))
(table acp-to-create add (colonists (outpost domed-city cloud-city) 1))
(table acp-to-build add (colonists (outpost domed-city cloud-city) 1))
(table constructor-absorbed-by add 
    (colonists (outpost domed-city cloud-city) true))
(table hp-to-garrison add (colonists (outpost domed-city cloud-city) 10))

; disbanding colonists is listed here because what you want to do is move
; colonists to somewhere you need support, then disband them, where they become
; support.
(add colonists acp-to-disband 1)
(add colonists hp-per-disband 10)
(table recycleable-material add (colonists (fuel support) 100))

; ENGINEER CONSTRUCTION

(table can-create add (engineers buildable-stations 1))
(table can-build add (engineers buildable-stations 1))
(table acp-to-create add (engineers buildable-stations 1))
(table acp-to-build add (engineers buildable-stations 1))
(table constructor-absorbed-by add (engineers buildable-stations true))
(table hp-to-garrison add (engineers buildable-stations 10))

; BLACK HOLES
; (add black-hole image-name "solar-bgiant")
; default "black hole" image is not too bad
(table favored-terrain add (black-hole ds0 100))
(table independent-density add (black-hole ds0 5))

; once you move into a black hole's gravity well, you ain't goin' nowhere
; gravity well has a range of 2 units
(table zoc-range add (black-hole u* 2))
; it's all too easy to enter the well
(table mp-to-enter-zoc add (u* black-hole 0))
; and we'll assume that you can do tricky orbit things to move around inside
(table mp-to-traverse-zoc add (u* black-hole 0))
; but you can never leave
(table mp-to-leave-zoc add (u* black-hole 999))

; but black holes produce lots of neutronium, and dump it into terrain
(table base-production add (black-hole neutronium 50))
(table unit-to-terrain-out-length add (black-hole neutronium 0))

; so if you place a neu-station within two cells, it'll pick it up from the
; hole, and you can use it to export neutronium to passing ships that don't
; need to enter the well in order to fill up.  However, "within two cells"
; means it has to be within the hole's gravity well.  So to build that
; station you're going to have to condemn a starship crew to death. 
; Ideally, you want to have somebody else build the station, and then you
; can build one of your own outside the gravity well to suck up the
; neutronium, because stations also exchange with their terrain.

; TODO - effect of black hole radiation on units within range

; black holes produce lots of nasty radiation, some of which manifests as
; occasional random detonations.  TODO: they also tend to react badly to the
; approach of hyperwarp-equipped starships
(table detonation-accident-chance add (black-hole t* 0.10%))
(add black-hole acp-to-detonate 1)
(add black-hole hp-per-detonation 0)
(table detonation-terrain-range add (black-hole t* 2))
(table detonation-terrain-damage-chance add
   (black-hole t* 30%)
   (black-hole st3 5%) ; dwarf stars are tenacious
   (black-hole (warp-path orbit) 0%)
   (u* is0 0%)
)
; this transition table is not particularly scientifically-based; the
; general idea is simply that the black hole randomizes things, makes
; them nastier, and eventually eliminates that.
(table terrain-damaged-type
   ; deep space just gets emptied - note the "empty" value for this and
   ; others is "inner solar system", because the black hole's gravity well
   ; is a "small" kind of place and (due to gravity fields) actually easy to
   ; maneuver in; you just can't leave.
   (dsall is0 1)

   ; inner planets get demoted and then vanish
   ((is1 is3 isn isno) is0 1)
   (is2 isall (1 0 0 1 0 0 0 0 0))
   (is4 isall (1 0 1 1 0 0 0 0 0))
   (is5 isall (1 0 1 1 1 0 0 0 0))
   (is6 isall (1 0 1 1 1 1 0 0 0))

   ; mid-range planets get killed and then vanish
   ((ms0 ms1 ms2 ms5 msn msno) is0 1)
   (ms3 msall (1 0 1 0 0 1 0 0 0))
   (ms4 msall (1 0 1 0 0 1 0 0 0))
   (ms6 msall (1 0 1 0 0 1 0 0 0))

   ; gas giants become colder and then turn into nebulae before vanishing
   ((os0 os1 os2 osn osno) is0 1)
   (os3 osn 1)
   (os4 os3 1)
   ; but warm ones can be ignited (Clarke's 2001 "Lucifer" scenerio) into
   ; small stars instead
   (os5 (os3 st3) 1)
   ; and terrestrial-type planets are treated as if they were inner planets
   (os6 isall (1 0 1 1 1 1 0 0 0))

   ; main sequence stars are unstable, and randomly change colour or
   ; become dwarfs
   ((st0 st1 st2) (st0 st1 st2 st3) 1)
   ; dwarfs blow up into nebulas (which will then vanish)
   (st3 isn 1)
   ; giants get colour randomized
   ((st4 st5 st6) (st4 st5 st6) 1)
   ; or else shrink while retaining colour
   ((st4 st5) st1 1)
   (st6 st2 1)
)

; THE GIANT SPACE LEECH

; leech eggs consume a little bit of fuel, can starve and die
;    produce a little bit of support, which they don't exchange
;    when enough support is accumulated, auto-upgrade-to GSL
;    tends to be captured if attacked
;    hard to see
; GSL consumes lots of fuel, but won't die if starved
;    also consumes neutronium if it's available
;    produces support, uses it to create eggs
;    tends to retreat if attacked
;    if possible: can't be owned by a side, becomes independent if an owned
;    egg hatches
;    has limited attack capability, tends to kill small things like shuttles
;    and probes

; Strategic notes: if a GSL shows up in your system it's initially no big
; deal.  But then it starts to lay eggs.  You could pick up the eggs and
; move them somewhere (maybe to an enemy system!) but they will eventually
; hatch and cause mayhem.  If enough GSLs are hanging around, they get in the
; way and eat up all your fuel.  They are hard to kill, but you can attack
; them and make them retreat.  The coolest anti-GSL tactic is to back the beast
; into a black hole's gravity well, where it will eventually succumb to the
; radiation.

; FRACTAL TERRAIN GENERATION

(add t* alt-percentile-min 100)
(add t* alt-percentile-max 0)
(add t* wet-percentile-min 100)
(add t* wet-percentile-max 0)
(add (ds0 ds1) alt-percentile-min 0)
(add (ds0 ds1) alt-percentile-max 100)
(add ds0 wet-percentile-min 0)
(add ds0 wet-percentile-max 94)
(add dsn wet-percentile-min 85)
(add dsn wet-percentile-max 96)
(add ds1 wet-percentile-min 94)
(add ds1 wet-percentile-max 100)
(add dsn alt-percentile-min 0)
(add dsn alt-percentile-max 75)

(set edge-terrain ds0)

; RANDOM TERRAIN ALTERATION

(table terrain-density
   (ds0 stall 5)
   (ds0 dsall (0 0 500 250 150 100 50 0))
   ; ds1 is for stars inside pockets in nebula; make them richer
   (ds1 stall 25)
   (ds1 dsall (0 0 500 300 300 200 100 0))
)

; ADJACENT TERRAIN RULES

(table adjacent-terrain-effect
   ; promote ds according to what it's next to
   (ds0 stall is0)
   (ds0 isall ms0)
   (ds0 msall os0)
   (ds1 stall is0) ; exception: ds1 has same fate as ds0
   (ds1 isall ms0)
   (ds1 msall os0)
   (ds2 stall is2)
   (ds2 isall ms2)
   (ds2 msall os2)
   (ds3 stall is3)
   (ds3 isall ms3)
   (ds3 msall os3)
   (ds4 stall is4)
   (ds4 isall ms4)
   (ds4 msall os4)
   (ds5 stall is5)
   (ds5 isall ms5)
   (ds5 msall os5)
   (ds6 stall is6)
   (ds6 isall ms6)
   (ds6 msall os6)
   (dsn stall isn)
   (dsn isall msn)
   (dsn msall osn)

   ; promote ms according to what it's next to
   (ms0 stall is0)
   (ms1 stall is1)
   (ms2 stall is2)
   (ms3 stall is3)
   (ms4 stall is4)
   (ms5 stall is5)
   (ms6 stall is6)
   (msn stall isn)
   
   ; promote os according to what it's next to
   (os0 stall is0)
   (os0 isall ms0)
   (os1 stall is1)
   (os1 isall ms1)
   (os2 stall is2)
   (os2 isall ms2)
   (os3 stall is3)
   (os3 isall ms3)
   (os4 stall is4)
   (os4 isall ms4)
   (os5 stall is5)
   (os5 isall ms5)
   (os6 stall is6)
   (os6 isall ms6)
   (osn stall isn)
   (osn stall msn)
   
   ; mark orbits as special
   (is0 (is1 is2 is3 is4 is5 is6 isno) is1)
   (ms0 (ms1 ms2 ms3 ms4 ms5 ms6 msno) ms1)
   (os0 (os1 os2 os3 os4 os5 os6 osno) os1)
   (isn (is1 is2 is3 is4 is5 is6 isno) isno)
   (msn (ms1 ms2 ms3 ms4 ms5 ms6 msno) msno)
   (osn (os1 os2 os3 os4 os5 os6 osno) osno)
   
   ; ms5 is special - asteroid belt; eats entire orbit
   ((ms0 ms1 ms2 ms3 ms4 ms6 msn) ms5 ms5)
)

(table adjacent-terrain-connection
   (isall isall orbit)
   (msall msall orbit)
   (osall osall orbit)
   (is0 is0 non-terrain)
   (ms0 ms0 non-terrain)
   (ms5 ms5 non-terrain)
   (os0 os0 non-terrain)
   (isn isn non-terrain)
   (msn msn non-terrain)
   (osn osn non-terrain)
)

; DRAWABLE TERRAIN

(table drawable-terrain
  (orbit isall false)
  (orbit msall false)
  (orbit osall false)
  (orbit (is1 ms1 os1 isno msno osno) true)
)

; WARP STRINGS

(table road-chance
  (u* u* 0)
  (black-hole black-hole 25)
)

(table road-into-chance
  (t* t* 0)
  (t* dsall 100)
)

(add black-hole road-to-edge-chance 100)
(add black-hole spur-range 3)
(add black-hole spur-chance 50)
(set edge-road-density 50)

; FEATURE NAMING

; "liquid" is only used for naming; it includes basically everything except
; nebula
(add t* liquid true)
(add nebula liquid false)

(include "ng-features")

(include "town-names")
; (include "ng-chinese")
; (include "ng-japanese")
(include "ng-star")
(include "ng-weird")

(set feature-types '(
   (continents nebula 4)

   (st0 star 1 1) (st1 star 1 1) (st2 star 1 1) (st3 star 1 1)
   (st4 star 1 1) (st5 star 1 1) (st6 star 1 1)

   (is2 planet 1 1) (is3 planet 1 1) (is4 planet 1 1)
   (is5 planet 1 1) (is6 planet 1 1)

   (ms2 planet 1 1) (ms3 planet 1 1) (ms4 planet 1 1)
   (ms5 belt 1) (ms6 planet 1 1)

   (os2 planet 1 1) (os3 planet 1 1) (os4 planet 1 1)
   (os5 planet 1 1) (os6 planet 1 1)
))

(set feature-namers '(
   (nebula nebula-names)
   (star star-names)
;   (planet random-town-names) ; FIXME
;   (belt generic-desert-names) ; FIXME
))

(namer alpha-code (grammar root 6
   (root (or 5 (letters digits)
             3 (letters "-" digits)
             2 (digits letters)
             1 (letters digits letters)
             ))
   (letters (or 10 (letter)
                3 (letter letter)
                1 (letter letter letter)
                ))
   (letter (or 5 "A" 9 "B" 9 "C" 3 "D" 1 "E" 3 "F" 3 "G" 3 "H" 1 "I" 3 "J"
               3 "K" 3 "L" 9 "M" 3 "N" 1 "O" 3 "P" 9 "Q" 3 "R" 3 "S" 3 "T"
               1 "U" 3 "V" 9 "W" 9 "X" 5 "Y" 9 "Z"))
   (digits (or 5 (digit)
               10 (digit digit)
               3 (digit digit digit)
               2 (digit digit digit digit)
               ))
   (digit (or "0" "1" "2" "3" "4" "5" "6" "7" "8" "9"))
))

(namer nebula-names (grammar root 10
   (root (or 3 (short-generic-names " Nebula")
             1 (random-town-names " Nebula")
             2 (foo "Nebula " alpha-code)
             ))
   (foo "")  ; works around a bug?
))

; COUNTRIES

(set country-radius-min 100)
(add colony start-with 1)

; MATERIAL DIFFUSION THROUGH CELLS
; fuel
;    all cells can hold a lot of it
;    stars produce a fair bit
;    radiating gas-giants produce a little
;    diffuses to neighbouring cells
;    most cells consume 1 unit per turn
;    nebulas and non-radiating gas giants produce fuel, but it doesn't
;    diffuse out of them
; support
;    only handled by units, terrain doesn't touch it
; science
;    units produce science
;    science doesn't diffuse across the map, but units tend to exchange it
;    with terrain, note that this does mean you may end up supplying your enemy
; neutronium
;    terrain doesn't produce, consume, or diffuse it, but can store it

(table terrain-storage-x
  (t* (fuel science neutronium) 5000)
  ; nebulae and gas giants produce fuel, so shouldn't store too much lest
  ; they become bonanzas
  (nebula fuel 20)
  (gas-giants fuel 100)
)

(table terrain-consumption
  (t* fuel 1)
  ; fuel disappears faster in deep space
  (dsall fuel 5)
  (nebula fuel 2)
)

(table terrain-production
  (stall fuel (120 100 150 70 70 150 180))
  (nebula fuel 3)
  (gas-giants fuel (7 13 20))
)

; fuel is the only thing that diffuses terrain-to-terrain
; terrain-to-terrain-in-length defaults to large, which is fine
(table terrain-to-terrain-out-length
  ; 
  (t* fuel 1)
  ; nebulae and os3 (non-radiating) gas giants don't diffuse it out
  (nebula fuel -1)
  (os3 fuel -1)
)

; POINT VALUES
; not clear whether last-side-wins is really the best way to do this

(scorekeeper (title "") (do last-side-wins))

(add habitations point-value (100 200 300 400 250 250))
(add missions point-value 10)
(add stations point-value (100 20 0 20 50 100))
