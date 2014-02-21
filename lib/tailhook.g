(game-module "tailhook"
  (title "Carrier Battles")
  (version "1.0")
  (base-game true)
  (blurb "Pacific theatre, early World War II (1942) by Keir Novik.")
  (instructions 
    ("Destroy the enemy fleet and capture his bases."
     "Type '?' for help."
     "(I especially recommend reading the notes in the \"modules\""
     "section and at the end of each unit description.)""")
    )
  (variants 
    (world-seen true)
    (see-all false)
    (sequential false)
    (world-size (40 40 360))
#|
    ("Night" night 
      (true
        (world (day-length 24))
	(set initial-day-part 18) ; 6am
	))
|#
    ("Billy Mitchell battle" mitchell-battle
      (true
        (add u* start-with 0)
;; Ideally America should have 1 CV, 3 DD, and carrier-borne aircraft;
;; and Japan 1 BB, 2 CA, 6 DD, 2 sub, and seaplanes.
        (add (DD CV) start-with (3 1))
        (add (F4F-3 A6M2) start-with 8)
        (add (SBD D3A1) start-with 8)
        (add (TBD B5N) start-with 8)
;
        (add (sub DD CA BB) start-with (2 6 2 1))
        (add seaplanes start-with 1)
        (add OS2U start-with 4)
	))
    ("Conventional battle" conventional-battle
      (true
        (add u* start-with 0)
        (add seaplanes start-with 1)
        (add OS2U start-with 4)
        (add (sub DD CA BB) start-with (2 6 2 1))
	))
    ("Convoy battle" convoy-battle
      (true
        (add u* start-with 0)
;; Ideally America should have the subs, seaplane base, and flying
;; boats; and Japan the transports and destroyers.
        (add (transport DD) start-with (8 2))
        (add sub start-with 4)
;
        (add seaplane-base start-with 1)
        (add (PBY-5 H6K4) start-with 2)
	))
    ("Larger islands" larger-islands
      (true
;                 (sea shallows swamp desert plains forest mountains ice))
        (add cell-t* alt-percentile-min (  0  68  69  70  70  70  93  99))
        (add cell-t* alt-percentile-max ( 68  69  71  93  93  93  99 100))
;; This is the stdterr default.
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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Unit definition ;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Places
(unit-type port (image-name "anchor-2")
  (help "A naval port town with an airfield"))
(unit-type town (image-name "town20")
  (help "A small town with no airfield or port facilities"))
(unit-type airfield (image-name "airbase")
  (help "A large airfield near a town"))
(unit-type airstrip (image-name "airbase")
  (help "A single short runway, suitable only for light aircraft"))
(unit-type seaplane-base (image-name "anchor")
  (help "A seaplane and flying boat base at a small town"))
;; or use engineer or pyramid
(define base airfield)
(define city port)
(define places (port town airfield airstrip seaplane-base))

;; Ships
(unit-type CV (name "carrier") (image-name "cv")
  (help "A full-size aircraft carrier"))
(unit-type CVE (name "light carrier") (image-name "cv")
  (help "A smaller aircraft carrier, often for escort duties"))
(unit-type BB (name "battleship") (image-name "bb")
  (help "A heavily-armoured, heavily-armed warship"))
(unit-type CA (name "heavy cruiser") (image-name "bb")
  (help "A heavily-armoured cruiser"))
(unit-type CL (name "light cruiser") (image-name "bb")
  (help "A lightly-armoured cruiser"))
(unit-type DD (name "destroyer") (image-name "dd")
  (help "A small ship that protects against submarines"))
(unit-type sub (name "submarine") (image-name "sub")
  (help ""))
(unit-type oiler (name "oiler") (image-name "ap")
  (help "A non-combatant ship for fueling other vessels"))
(unit-type tender (name "tender") (image-name "ap")
  (help "A non-combatant ship for tending submarines and seaplanes"))
(unit-type transport (name "transport") (image-name "ap")
  (help "A non-combatant ship for carrying troops and supplies"))
(define auxiliary-ships (oiler tender transport))
(define heavy-ships (BB CA CL))
(define carriers (CV CVE))
(define ships (append carriers heavy-ships DD sub auxiliary-ships))

;; Carrier fighters (VF)
(unit-type F2A (name "F2A flight") (image-name "fighter-2")
  (help "A flight of Brewster F2A-3 Buffalo carrier fighters"))
(unit-type F4F-3 (name "F4F-3 flight") (image-name "fighter-2")
  (help "A flight of Grumman F4F-3 Wildcat carrier fighters"))
(unit-type F4F-4 (name "F4F-4 flight") (image-name "fighter-2")
  (help "A flight of Grumman F4F-4 Wildcat carrier fighters"))
(unit-type A6M2 (name "A6M2 flight") (image-name "fighter-2")
  (help "A flight of Mitsubishi A6M2 Model 21 Zeke carrier fighters"))
(unit-type A6M3 (name "A6M3 flight") (image-name "fighter-2")
  (help "A flight of Mitsubishi A6M3 Model 32 Zeke carrier fighters"))
(define F4F (F4F-3 F4F-4))
(define A6M (A6M2 A6M3))
(define american-carrier-fighters (append F2A F4F))
(define japanese-carrier-fighters A6M)
(define carrier-fighters 
  (append american-carrier-fighters japanese-carrier-fighters))
;
;; Land-based pursuit fighters
(unit-type P-38 (name "P-38 flight") (image-name "fighter-2")
  (help "A flight of Lockheed P-38G Lightning fighters"))
(unit-type P-39 (name "P-39 flight") (image-name "fighter-2")
  (help "A flight of Bell P-39D/F or P-400 Airacobra fighters"))
(unit-type P-40E (name "P-40E flight") (image-name "fighter-2")
  (help "A flight of Curtiss P-40E fighters"))
(unit-type P-40F (name "P-40F flight") (image-name "fighter-2")
  (help "A flight of Curtiss P-40F Warhawk fighters"))
(define P-40 (P-40E P-40F))
(define american-land-fighters (append P-38 P-39 P-40))
(define japanese-land-fighters nil)
(define land-fighters 
  (append american-land-fighters japanese-land-fighters))
;
(define american-fighters 
  (append american-carrier-fighters american-land-fighters))
(define japanese-fighters 
  (append japanese-carrier-fighters japanese-land-fighters))
(define fighters (append carrier-fighters land-fighters))

;; Reconnaissance/observation seaplanes (VSO)
(unit-type OS2U (name "OS2U") (image-name "seaplane")
  (help "A single Vought OS2U-3 Kingfisher seaplane"))
(unit-type E13A (name "E13A") (image-name "seaplane")
  (help "A single Aichi E13A1a Jake seaplane"))
(unit-type E7K (name "E7K") (image-name "seaplane")
  (help "A single Kawanishi E7K2 Alf seaplane"))
(unit-type F1M (name "F1M") (image-name "seaplane")
  (help "A single Mitsubishi F1M2 Pete seaplane"))
(unit-type A6M2-N (name "A6M2-N") (image-name "seaplane")
  (help "A single Nakajima A6M2-N Model 11 Rufe seaplane"))
(define american-seaplanes OS2U)
(define japanese-seaplanes (E13A E7K F1M A6M2-N))
(define seaplanes 
  (append american-seaplanes japanese-seaplanes))

;; Light bombers (one engine)
(unit-type SB2U (name "SB2U flight") (image-name "stuka")
  (help "A flight of Vought SB2U-3 Vindicator carrier dive bombers"))
(unit-type SBD (name "SBD flight") (image-name "stuka")
  (help "A flight of Douglas SBD-3/4 Dauntless carrier dive bombers"))
(unit-type A-24 (name "A-24 flight") (image-name "stuka")
  (help "A flight of Douglas A-24 Dauntless dive bombers"))
(unit-type TBD (name "TBD flight") (image-name "torpedo-bomber-2")
  (help "A flight of Douglas TBD-1 Devastator carrier torpedo bombers"))
(unit-type TBF (name "TBF flight") (image-name "torpedo-bomber-2")
  (help "A flight of Grumman TBF-1 Avenger carrier torpedo bombers"))
(unit-type D3A1 (name "D3A1 flight") (image-name "stuka")
  (help "A flight of Aichi D3A1 Val carrier dive bombers"))
(unit-type D3A2 (name "D3A2 flight") (image-name "stuka")
  (help "A flight of Aichi D3A2 Val carrier dive bombers"))
(unit-type B5N (name "B5N flight") (image-name "torpedo-bomber-2")
  (help "A flight of Nakajima B5N2 Kate carrier torpedo bombers"))
(unit-type D4Y (name "D4Y") (image-name "1e-2")
  (help "A single Yokosuke D4Y1-C Judy carrier patrol bomber"))
(define D3A (D3A1 D3A2))
(define american-light-bombers (append SB2U SBD A-24 TBD TBF))
(define japanese-light-bombers (append D3A B5N D4Y))
(define light-bombers 
  (append american-light-bombers japanese-light-bombers))

;; Medium bombers (two engines)
(unit-type PBY-5 (name "PBY-5") (image-name "pby-2")
  (help "A single Consolidated PBY-5 Catalina flying boat"))
(unit-type PBY-5A (name "PBY-5A") (image-name "pby-2")
  (help "A single Consolidated PBY-5A Catalina flying boat"))
(unit-type A-20 (name "A-20 flight") (image-name "2e")
  (help "A flight of Douglas A-20A Havoc medium bombers"))
(unit-type B-25 (name "B-25 flight") (image-name "2e")
  (help "A flight of North American B-25C/D Mitchell medium bombers"))
(unit-type B-26 (name "B-26 flight") (image-name "2e")
  (help "A flight of Martin B-26A Marauder medium bombers"))
(unit-type Hudson (name "Hudson flight") (image-name "2e")
  (help "A flight of Lockheed Hudson Mk IIIA medium bombers"))
(unit-type G4M (name "G4M flight") (image-name "2e")
  (help "A flight of Mitsubishi G4M1 Model 11 Betty medium bombers"))
(define PBY (PBY-5 PBY-5A))
(define american-medium-bombers (append PBY A-20 B-25 B-26 Hudson))
(define japanese-medium-bombers G4M)
(define medium-bombers 
  (append american-medium-bombers japanese-medium-bombers))

;; Heavy bombers (four engines)
(unit-type B-17 (name "B-17 flight") (image-name "4e-2")
  (help "A flight of Boeing B-17E Flying Fortress heavy bombers"))
(unit-type PB4Y (name "PB4Y") (image-name "4e-2")
  (help "A single Consolidated PB4Y-1 Liberator heavy bomber"))
(unit-type H6K4 (name "H6K4") (image-name "pby-2")
  (help "A single Kawanishi H6K4 Mavis flying boat"))
(unit-type H6K5 (name "H6K5") (image-name "pby-2")
  (help "A single Kawanishi H6K5 Mavis flying boat"))
; Could add H8K Emily as well; one was shot down on 10 Mar 42 near
; Midway Island.
(define H6K (H6K4 H6K5))
(define american-heavy-bombers (B-17 PB4Y))
(define japanese-heavy-bombers H6K)
(define heavy-bombers
  (append american-heavy-bombers japanese-heavy-bombers))

;; Pontoon aircraft
(define american-flying-boats PBY)
(define japanese-flying-boats H6K)
(define flying-boats 
  (append american-flying-boats japanese-flying-boats))
;
(define american-pontoon-aircraft 
  (append american-seaplanes american-flying-boats))
(define japanese-pontoon-aircraft
  (append japanese-seaplanes japanese-flying-boats))
(define pontoon-aircraft (append seaplanes flying-boats))

;; Land-based aircraft
(define american-land-bombers 
  (remove PBY-5 
     (append A-24 american-medium-bombers american-heavy-bombers)
     ))
(define japanese-land-bombers G4M)
(define land-bombers 
  (append american-land-bombers japanese-land-bombers))
(define land-aircraft (append land-fighters land-bombers))

;; Carrier-borne aircraft
(define american-carrier-bombers (remove A-24 american-light-bombers))
(define japanese-carrier-bombers japanese-light-bombers)
(define carrier-bombers 
  (append american-carrier-bombers japanese-carrier-bombers))
(define carrier-aircraft (append carrier-fighters carrier-bombers))

;; Level bombers
(define american-level-bombers
  (append A-20 Hudson PBY american-heavy-bombers))
(define japanese-level-bombers nil)
(define level-bombers 
  (append american-level-bombers japanese-level-bombers))

;; Dive bombers (VB/VS)
(define american-dive-bombers (append SB2U SBD A-24))
(define japanese-dive-bombers D3A)
(define dive-bombers 
  (append american-dive-bombers japanese-dive-bombers))

;; Torpedo bombers (VT)
(define american-torpedo-bombers (TBD TBF B-25 B-26))
(define japanese-torpedo-bombers (append B5N G4M H6K))
(define torpedo-bombers 
  (append american-torpedo-bombers japanese-torpedo-bombers))

;; Aircraft groups
(define american-aircraft 
  (append american-fighters american-seaplanes american-light-bombers
	  american-medium-bombers american-heavy-bombers))
(define japanese-aircraft 
  (append japanese-fighters japanese-seaplanes japanese-light-bombers
	  japanese-medium-bombers japanese-heavy-bombers))
;
(define aircraft (append american-aircraft japanese-aircraft))
;
(define single-aircraft (append seaplanes D4Y PBY H6K PB4Y))
(define aircraft-flights (remove single-aircraft aircraft))


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Material definition ;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(material-type fuel (char "o") (help "Aviation gasoline"))
(material-type bombs (char "b") (help "A bomb of roughly 250 kg"))
(material-type torps (char "!") (help "An 800 kg torpedo"))


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Terrain definition ;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Terrain compatible with earth-50km.g
(include "stdterr")

(set edge-terrain sea)

;                 (sea shallows swamp desert plains forest mountains ice))

;; Smaller islands than stdterr (water above sea level, though)
(add cell-t* alt-percentile-min (  0  90  95  95  95  95  98 100))
(add cell-t* alt-percentile-max ( 90  95  96  98  98  98  99 100))

(area (cell-width 50000)) ; 50km cells


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Capacitities ;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(table unit-size-as-occupant
  (u* u* 999)
  (carrier-aircraft carriers 1)
  (seaplanes heavy-ships 1)
  (pontoon-aircraft seaplane-base 1)
  ((append carrier-aircraft land-aircraft) (airstrip airfield) 1)
  ((append B-26 heavy-bombers) airstrip 999)
  (u* port 1)
  )

(add heavy-ships capacity 2)
(add (CVE CV) capacity (8 28))
(add airstrip capacity 24)
(add seaplane-base capacity 12)
(add (airfield port) capacity 48)

(add cell-t* capacity 1)
(table unit-size-in-terrain
  ;; Any number of aircraft and ships can be in a hex
  (aircraft cell-t* 0)
  (ships cell-t* 0)
  ;; Places takes up entire cell.
  (places cell-t* 1)
  )


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Vision ;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; At this scale, aircraft and submarines in the next hex should be
;; invisible.
(table see-chance-adjacent
  (u* sub 0)
;  (u* aircraft 0)
  )

(table vision-night-effect (u* t* 0))

#|
(table see-mistake-chance
  (u* aircraft 1000)
  (aircraft (remove sub ships) 1000)
  )
|#

(table looks-like
  (seaplanes seaplanes 2)
  (flying-boats flying-boats 2)
  (fighters fighters 2)
  (light-bombers light-bombers 2)
  (medium-bombers medium-bombers 2)
  (heavy-bombers heavy-bombers 2)
;
  (auxiliary-ships carriers 1)
  (carriers auxiliary-ships 1)
  (DD heavy-ships 1)
  (CL (CA BB) 1)
  (CA BB 1)
  (BB (DD CL CA) 1)
  )
;; How can we prevent enemy unit names from being seen?

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Actions ;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(add ships acp-per-turn 10)
(add places acp-per-turn 0)
(add ships acp-min -10)

; So places can defend themselves
(add places acp-min -100)
(add places acp-per-turn 100)
(add places acp-max 0)

(add F2A acp-per-turn 54)
(add F2A acp-min -54)
(add F4F-3 acp-per-turn 62)
(add F4F-3 acp-min -62)
(add F4F-4 acp-per-turn 54)
(add F4F-4 acp-min -54)
(add A6M2 acp-per-turn 69)
(add A6M2 acp-min -69)
(add A6M3 acp-per-turn 77)
(add A6M3 acp-min -77)
(add P-38 acp-per-turn 70)
(add P-38 acp-min -70)
(add P-39 acp-per-turn 65)
(add P-39 acp-min -65)
(add P-40E acp-per-turn 103)
(add P-40E acp-min -103)
(add P-40F acp-per-turn 100)
(add P-40F acp-min -100)

(add OS2U acp-per-turn 40)
(add OS2U acp-min -40)
(add E13A acp-per-turn 46)
(add E13A acp-min -46)
(add E7K acp-per-turn 38)
(add E7K acp-min -38)
(add F1M acp-per-turn 46)
(add F1M acp-min -46)
(add A6M2-N acp-per-turn 61)
(add A6M2-N acp-min -61)

(add SB2U acp-per-turn 51)
(add SB2U acp-min -51)
(add SBD acp-per-turn 58)
(add SBD acp-min -58)
(add A-24 acp-per-turn 58)
(add A-24 acp-min -58)
(add TBD acp-per-turn 43)
(add TBD acp-min -43)
(add TBF acp-per-turn 48)
(add TBF acp-min -48)
(add D3A1 acp-per-turn 61)
(add D3A1 acp-min -61)
(add D3A2 acp-per-turn 61)
(add D3A2 acp-min -61)
(add B5N acp-per-turn 54)
(add B5N acp-min -54)
(add D4Y acp-per-turn 88)
(add D4Y acp-min -88)

(add PBY-5 acp-per-turn 38)
(add PBY-5 acp-min -38)
(add PBY-5A acp-per-turn 38)
(add PBY-5A acp-min -38)
(add A-20 acp-per-turn 98)
(add A-20 acp-min -98)
(add B-25 acp-per-turn 78)
(add B-25 acp-min -78)
(add B-26 acp-per-turn 88)
(add B-26 acp-min -88)
(add Hudson acp-per-turn 68)
(add Hudson acp-min -68)
(add G4M acp-per-turn 65)
(add G4M acp-min -65)

(add B-17 acp-per-turn 70)
(add B-17 acp-min -70)
(add PB4Y acp-per-turn 67)
(add PB4Y acp-min -67)
(add H6K4 acp-per-turn 46)
(add H6K4 acp-min -46)
(add H6K5 acp-per-turn 54)
(add H6K5 acp-min -54)


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Movement ;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(add places speed 0)
(add u* free-mp 200)
(add u* acp-to-move 10)

(table mp-to-enter-terrain
  (u* t* 10)
  (u* river 0) ; no extra cost to cross rivers
  (ships t* 999)
  (ships sea 10)
  (ships shallows 20)
  (aircraft (mountains ice) 20) ; mountains take longer to fly over
  )

;; Aircraft cannot sortie again until two turns (hours) after they
;; land.
(table mp-to-enter-unit 
  (aircraft u* 200)
  (ships u* 10)
  )

;; Allow friendly and enemy units to share cells
;(table mp-to-enter-own (u* u* 0))


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Material properties ;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Only aircraft really need fuel on this scale.
(table base-production 
  ((append carriers places) (bombs torps) 100)
  ((append heavy-ships carriers places) fuel 2000)
  (town m* 0)
  )

(table material-to-move (aircraft fuel 1))
(table consumption-per-move (aircraft fuel 1))

;; Aircraft splash immediately if they run out of fuel, except planes
;; with pontoons which can float.
(table hp-per-starve ((remove pontoon-aircraft aircraft) fuel 1.00))

;; Aircraft on the ground or in a carrier need no fuel.
(table consumption-as-occupant (aircraft fuel 0))

(table unit-storage-x
  (places m* 999)
  (carriers m* 999)
  (heavy-ships fuel 999)
  (town m* 0)
  (torpedo-bombers torps 1)
  (dive-bombers bombs 2)
  ((append PBY A-20 Hudson) bombs 2)
  (PB4Y bombs 4)
  (B-17 bombs 8)
;;
  (F2A fuel 32)
  (F4F-3 fuel 28)
  (F4F-4 fuel 42)
  (A6M2 fuel 64)
  (A6M3 fuel 50)
  (P-38 fuel 58)
  (P-39 fuel 36)
  (P-40E fuel 28)
  (P-40F fuel 30)
  (OS2U fuel 26)
  (E13A fuel 44)
  (E7K fuel 44)
  (F1M fuel 16)
  (A6M2-N fuel 36)
  (SB2U fuel 38)
  (SBD fuel 32)
  (A-24 fuel 32)
  (TBD fuel 14)
  (TBF fuel 40)
  (D3A1 fuel 30)
  (D3A2 fuel 28)
  (B5N fuel 42)
  (D4Y fuel 80)
  (PBY-5 fuel 100)
  (PBY-5A fuel 78)
  (A-20 fuel 22)
  (B-25 fuel 50)
  (B-26 fuel 34)
  (Hudson fuel 52)
  (G4M fuel 124)
  (B-17 fuel 66)
  (PB4Y fuel 76)
  (H6K4 fuel 126)
  (H6K5 fuel 140)
  )
;; Aircraft should hold an even amount of fuel, otherwise it's too easy
;; to crash.

;; For aircraft, base-consumption = (acp-per-turn - 1)/10 (rounding down)
(table base-consumption
  (F2A fuel 5)
  (F4F-3 fuel 6)
  (F4F-4 fuel 5)
  (A6M2 fuel 6)
  (A6M3 fuel 7)
  (P-38 fuel 6)
  (P-39 fuel 6)
  (P-40E fuel 9)
  (P-40F fuel 9)
  (OS2U fuel 3)
  (E13A fuel 4)
  (E7K fuel 3)
  (F1M fuel 4)
  (A6M2-N fuel 6)
  (SB2U fuel 5)
  (SBD fuel 5)
  (A-24 fuel 5)
  (TBD fuel 4)
  (TBF fuel 4)
  (D3A1 fuel 6)
  (D3A2 fuel 6)
  (B5N fuel 5)
  (D4Y fuel 8)
  (PBY-5 fuel 3)
  (PBY-5A fuel 3)
  (A-20 fuel 9)
  (B-25 fuel 7)
  (B-26 fuel 8)
  (Hudson fuel 6)
  (G4M fuel 6)
  (B-17 fuel 7)
  (PB4Y fuel 6)
  (H6K4 fuel 4)
  (H6K5 fuel 5)
  )

(table in-length
  ;; Only aircraft can get supplies
  (u* m* -1)
  (aircraft m* 0)
  )

(table out-length
  ;; Only places and carriers can share supplies
  (u* m* -1)
  ((append carriers heavy-ships places) m* 0)
  (town m* -1)
  )

;; Check that aircraft can't resupply from other aircraft (seen in play!).

;; Unfortunately, aircraft can resupply by staying in the same cell as
;; a supplier.


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Combat ;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(add CV hp-max 30)
(add CVE hp-max 15)
(add BB hp-max 40)
(add CA hp-max 12)
(add CL hp-max 6)
(add DD hp-max 2)
(add auxiliary-ships hp-max 10)
(add places hp-max 100)

(table acp-to-defend 
   (u* u* 1)
   (aircraft u* 5)
   )

(table acp-to-attack 
   (u* u* 1)
   (aircraft u* 5)
;; Only fighters can initiate attack on aircraft
   (u* aircraft 0)
   (fighters aircraft 5)
;; Don't let auxiliaries commit suicide on move-to orders
   (auxiliary-ships ships 0)
   (transport places 10)
   (places u* 0)
   )

(table protection
  ((append carriers heavy-ships) aircraft 50)
  (places u* 50)
  )

(table hit-chance
  ;; aircraft vs. aircraft
  (aircraft aircraft 10)
  (fighters aircraft 80)
  (fighters medium-bombers 60)
  (fighters heavy-bombers 40)
  (fighters fighters 40)
  (single-aircraft aircraft 3)

;  (dive-bombers aircraft 100) ; for testing protection

  ;; aircraft vs. ship
  (aircraft ships 0)
  (dive-bombers ships 80)
  (dive-bombers (CL CA) 40)
  (dive-bombers (DD sub) 20)
  (torpedo-bombers ships 20)
  (torpedo-bombers CL 10)
  (torpedo-bombers (DD sub) 5)
  (level-bombers ships 20)
  (level-bombers (DD sub) 10)
  ((append PBY PB4Y H6K) sub 40) ;; depth charges

;; D3A1s placed 87% of their bombs on target during attacks on
;; cruisers HMS Cornwall and HMS Dorsetshire, and 82% on carrier HMS
;; Hermes.

  ;; ship vs. aircraft
  (ships aircraft 5)        ; light AAA
  ((CL CA CVE) aircraft 10) ; medium AAA
  ((CV BB) aircraft 20)     ; heavy AAA

  ;; Can't destroy places on this scale, but we need to hit places to
  ;; damage the aircraft there.
  (u* places 0)
  (places u* 0)
  ((append level-bombers dive-bombers transport) places 100)

  ;; Places vs. aircraft
  (places aircraft 5)                    ; light AAA
  ((seaplane-base airstrip) aircraft 10) ; medium AAA
  ((airfield port) aircraft 20)          ; heavy AAA

  ;; ship vs. ship (represents gun penetration and armour)
  (ships ships 80)
  (ships sub 0)
  (auxiliary-ships ships 0)
  (CA BB 20)
  (CL BB 10) ;; torpedoes
  (CL CA 40)
  (CL sub 20) ;; depth charges
  (CV BB 0)
  (CV CA 40)
  (CVE (BB CA) 0)
  (CVE CL 40)
  (DD (BB CA) 10) ;; torpedoes
  (DD CL 40)
  (DD sub 40) ;; depth charges
  (sub ships 20)
  (sub DD 10)
  (sub sub 0)
  )

(table damage
  (u* u* 1)
  (CA u* 1d3)
  (BB u* 4d3)
  ;; bombs
  ((append dive-bombers level-bombers) u* 1d3)
  ;; torpedoes
  ((append torpedo-bombers sub) u* 2d4+3)
  (CL BB 2d4+3)
  (DD (BB CA) 2d4+3)
  )

(table retreat-chance
  (fighters fighters 50)
)

(table consumption-per-attack
  ((append dive-bombers level-bombers) bombs 2)
  (torpedo-bombers torps 1)
  )

(table hit-by
  (u* m* 0)
  ((append ships places) bombs 2)
  (ships torps 1)
  )

;; Fighters in a cell defend other aircraft
;;(table stack-protection (fighters aircraft 80))

;; Aircraft on the ground/deck are helpless
(table occupant-combat (aircraft u* 0))

;; Transports carry troops, which can try to capture places
(table capture-chance 
   (transport places 10)
   (transport port 5)
   )

;; Can't destroy places on this scale
(add places hp-recovery 100.00)

;; Units are crippled at half-hp
(add oiler acp-damage-effect
   ((1 5) (5 5) (6 10) (10 10)))
(add tender acp-damage-effect
   ((1 5) (5 5) (6 10) (10 10)))
(add transport acp-damage-effect
   ((1 5) (5 5) (6 10) (10 10)))
(add DD acp-damage-effect
   ((1 5) (2 10)))
(add CL acp-damage-effect
   ((1 5) (3 5) (4 10) (6 10)))
(add CVE acp-damage-effect
   ((1 5) (7 5) (8 10) (15 10)))
(add CA acp-damage-effect
   ((1 5) (6 5) (7 10) (12 10)))
(add CV acp-damage-effect
   ((1 5) (15 5) (16 10) (30 10)))
(add BB acp-damage-effect
   ((1 5) (20 5) (21 10) (40 10)))


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Messages ;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(set action-notices '(
  ((destroy u* aircraft) (actor " shoots down " actee "!"))
  ((destroy u* ships) (actor " sinks " actee "!"))
  ))

(set event-notices '(
  ((unit-starved aircraft) (0 " runs out of fuel and crashes!"))
  ))

(set event-narratives '(
  ((unit-starved aircraft) (0 " ran out of fuel and crashed"))
  ))


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Default strategy ;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;(doctrine default-doctrine (resupply-percent 60))
;(side-defaults (default-doctrine default-doctrine))


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Time and dates ;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(world  
  (axial-tilt 22)
  (year-length 8760) ; 1942 was not a leap year
  )

(set calendar '(usual hour))
(set initial-date-min "6:00 18 March 1942")
(set initial-date-max "18:00 8 August 1942")
(set initial-date "6:00 7 May 1942")


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Side defaults ;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Default is carrier battle
(add u* start-with 0)
(add (F4F-3 A6M2) start-with 8)
(add (SBD D3A1) start-with 8)
(add (TBD B5N) start-with 8)
(add (DD CV) start-with (3 1))

(table favored-terrain
  (u* t* 0)
  (ships sea 100)
  (places land-t* 100)
  )

(add sea country-terrain-min 40)
(set country-radius-min 4)
(set country-separation-min 20)
(set country-separation-max 60)

(set advantage-min 1)
(set advantage-default 1)
(set advantage-max 4)

;; Everybody starts out full.
(table unit-initial-supply (u* m* 9999))

(set terrain-seen true)
(add places see-always true)
(add u* see-occupants false)
;(add places already-seen 100)
;(add u* see-occupants true)

(set sides-min 2)
(set sides-max 2)

(add american-aircraft possible-sides "american")
(add japanese-aircraft possible-sides "japanese")

(set side-library '(
  ((name "USA") (noun "American") (class "american") 
   (emblem-name "flag-usa")
   (unit-namers
    (DD  "us-dd-names")
    (CL  "us-cl-names")
    (CA  "us-ca-names")
    (CVE "us-cve-names")
    (CV  "us-cv-names")
    (BB  "us-bb-names"))
   )
  ((name "Japan") (noun "Japanese") (class "japanese") 
   (emblem-name "flag-japan")
   (unit-namers
    (DD  "japanese-dd-names")
    (CL  "japanese-cl-names")
    (CA  "japanese-ca-names")
    (CVE "japanese-cve-names")
    (CV  "japanese-cv-names")
    (BB  "japanese-bb-names"))
   )
  ))
(include "ng-ships")

(add u* point-value 0)
(add oiler point-value 1)
(add transport point-value 5)
(add BB point-value 25)
(add carriers point-value (25 100))
(add seaplane-base point-value 50)
(add airstrip point-value 75)
(add airfield point-value 100)
(add port point-value 200)

(scorekeeper (do last-side-wins))


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Notes ;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Ships and places don't need 8x8 icons, since they're only really
;; useful for occupants, and our ships will spend most of their time
;; at sea.  However, they are very useful for distinguishing aircraft
;; in airfields or carriers.

;; New 32x32 single-engined airplane.  16x16 is aircraft.imf/1e
(imf "1e-2" ((8 8)
  (mono "00/00/10/30/10/10/38/00")
  (mask "ff/ff/ff/ff/ff/ff/ff/ff")))
(imf "1e-2" ((16 16)
  (mono "0000/0000/0000/0000/0000/0180/0180/4180/4180/7ff0/7ff0/4180/4180/0180/0180/0000")
  (mask "0000/0000/0000/0000/03c0/03c0/e3c0/e3c0/fff8/fff8/fff8/fff8/e3c0/e3c0/03c0/03c0")))
(imf "1e-2" ((32 32)
  (mono
   "00000000/00000000/00000000/00000000/00000000/00000000/00000000/00000000"
   "00000000/00000000/00004000/0000e000/0001e000/0001e000/0001e000/0c01e000"
   "0c01e000/0c01e000/1fffff00/3fffff80/1fffff00/0c01e000/0c01e000/0c01e000"
   "0001e000/0001e000/0001e000/0000e000/00004000/00000000/00000000/00000000")
  (mask
   "00000000/00000000/00000000/00000000/00000000/00000000/00000000/00000000"
   "00000000/00004000/0000e000/0001f000/0003f000/0003f000/0c03f000/1e03f000"
   "1e03f000/1fffff00/3fffff80/7fffffc0/3fffff80/1fffff00/1e03f000/1e03f000"
   "0c03f000/0003f000/0003f000/0001f000/0000e000/00004000/00000000/00000000")))

;; New two-engined airplane
(imf "2e" ((8 8)
  (mono "00/00/38/04/18/20/3c/00")
  (mask "ff/ff/ff/ff/ff/ff/ff/ff")))
(imf "2e" ((16 16)
  (mono "0000/0000/0000/00c0/00c0/00e0/00c0/60c0/7ff8/7ff8/60c0/00c0/00e0/00c0/00c0/0000")
  (mask "0000/0000/0000/0000/03c0/03c0/e3c0/e3c0/fff8/fff8/fff8/fff8/e3c0/e3c0/03c0/03c0")))
#|
(imf "2e" ((32 32)
  (mono
   "00000000/00000000/00000000/00000000/00000000/00000000/00000000/00001000"
   "00003800/00007800/00007800/00007800/00007e00/00007e00/08007800/1c007800"
   "1c007800/1fffffe0/3ffffff0/1fffffe0/1c007800/1c007800/08007800/00007e00"
   "00007e00/00007800/00007800/00007800/00003800/00001000/00000000/00000000")
  (mask
   "00000000/00000000/00000000/00000000/00000000/00000000/00001000/00003800"
   "00007c00/0000fc00/0000fc00/0000fe00/0000ff00/0800ff00/1c00fe00/3e00fc00"
   "3fffffe0/3ffffff0/7ffffff8/3ffffff0/3fffffe0/3e00fc00/1c00fe00/0800ff00"
   "0000ff00/0000fe00/0000fc00/0000fc00/00007c00/00003800/00001000/00000000")))
|#

;; Touched up the 16x16 wingtips and tailplane (orig:
;; standard.imf/4e), new 8x8 icon.
(imf "4e-2" ((8 8)
  (mono "00/00/24/24/3c/04/04/00")
  (mask "ff/ff/ff/ff/ff/ff/ff/ff")))
(imf "4e-2" ((16 16)
  (mono "0000/0060/0060/0070/0060/0070/6060/7ffe/7ffe/6060/0070/0060/0070/0060/0060/0000")
  (mask "0060/00f0/00f0/00f8/00f0/60f8/fffe/ffff/ffff/fffe/60f8/00f0/00f8/00f0/00f0/0060")))
(imf "4e-2" ((32 32)
  (mono
   "00000000/00000000/00000000/00000000/00001000/00003800/00007800/00007800"
   "00007e00/00007e00/00007800/00007800/00007e00/10007e00/38007800/38007800"
   "3ffffff0/7ffffff8/3ffffff0/38007800/38007800/10007e00/00007e00/00007800"
   "00007800/00007e00/00007e00/00007800/00007800/00003800/00001000/00000000")
  (mask
   "00000000/00000000/00000000/00001000/00003800/00007c00/0000fc00/0000fe00"
   "0000ff00/0000ff00/0000fe00/0000fe00/1000ff00/3800ff00/7c00fe00/7ffffff0"
   "7ffffff8/fffffffc/7ffffff8/7ffffff0/7c00fe00/3800ff00/1000ff00/0000fe00"
   "0000fe00/0000ff00/0000ff00/0000fe00/0000fc00/00007c00/00003800/00001000")))

;; Added a circle to misc.imf/anchor
(imf "anchor-2" ((16 16)
  (mono "0000/07c0/1830/2388/4284/4384/8102/87c2/8102/8102/b11a/b11a/5ff4/27c8/1830/07c0")
  (mask "0000/07c0/1ff0/3ff8/7ffc/7ffc/fffe/fffe/fffe/fffe/fffe/fffe/7ffc/3ff8/1ff0/07c0")))
(imf "anchor-2" ((32 32)
  (mono
   "00000000/000ff000/00300c00/00c00300/01000080/0203c040/04066020/08042010"
   "10042008/1007e008/2003c004/20018004/40018002/400ff002/400ff002/40018002"
   "40018002/40018002/46018062/478181e2/278181e4/278181e4/13c3c3c8/10fbdf08"
   "087ffe10/040ff020/02018040/01000080/00c00300/00300c00/000ff000/00000000")
  (mask
   "000ff000/003ffc00/00f00f00/01c00380/0303c0c0/0607e060/0c0ff030/180e7018"
   "300ff00c/300ff00c/6007e006/6003c006/c00ff003/c01ff803/c01ff803/c00ff003"
   "c003c003/c603c063/cf83c1f3/cfc3c3f3/6fc3c3f6/6fc3c3f6/37ffffec/33ffffcc"
   "18ffff18/0c7ffe30/060ff060/030180c0/01c00380/00f00f00/003ffc00/000ff000")))

;; New 8x8 fighter.  32x32 and 16x16 icons are standard.imf/fighter
(imf "fighter-2" ((8 8)
  (mono "00/00/3c/20/38/20/20/00")
  (mask "ff/ff/ff/ff/ff/ff/ff/ff")))
(imf "fighter-2" ((16 16)
  (mono "0000/0000/0000/6000/6380/7fe0/1fc0/0018/1830/18f0/1ffc/07f8/0380/0600/0c00/0000")
  (mask "0000/0000/6000/f380/ffe0/fff0/7fe8/1ffc/3cf8/3ffc/3ffe/1ffc/07f8/0f80/1e00/0c00")))
(imf "fighter-2" ((32 32)
  (mono
   "00000000/00000000/00000000/00000000/00000000/30000000/38000000/78000400"
   "78078400/7ffffe00/3ffffc00/03fff400/001e0000/00000000/00000000/00000780"
   "00000f80/00001f00/03403e00/07c07ec0/07c0fce0/07ffffe0/0fffffe0/0e7fffe0"
   "003fffc0/001f80c0/003f0000/007e0000/00f80000/01f00000/01e00000/00000000")
  (mask
   "00000000/00000000/00000000/00000000/30000000/78000000/7c000400/fc078e00"
   "fffffe00/ffffff00/7ffffe00/3ffffe00/03fff400/001e0000/00000780/00000fc0"
   "00001fc0/03403f80/07e07fc0/0fe0ffe0/0ffffff0/0ffffff0/1ffffff0/1ffffff0"
   "0e7fffe0/003fffe0/007f80c0/00ff0000/01fe0000/03f80000/03f00000/01e00000")))

;; 16x16 icon is aircraft.imf/cargo-plane
(imf "pby-2" ((8 8)
  (mono "00/00/38/24/38/20/20/00")
  (mask "ff/ff/ff/ff/ff/ff/ff/ff")))
(imf "pby-2" ((16 16)
  (mono "0000/0000/0000/000c/601c/6038/7ffc/7ffe/3ffe/1ffc/0380/0700/0c00/0000/0000/0000")
  (mask "0000/0000/001e/f03e/f07e/fffe/ffff/ffff/ffff/7fff/3ffe/1fc0/1f80/1e00/0000/0000")))

;; 16x16 icon is aircraft.imf/pby
(imf "seaplane" ((8 8)
  (mono "00/00/18/24/24/24/18/00")
  (mask "ff/ff/ff/ff/ff/ff/ff/ff")))
(imf "seaplane" ((16 16)
  (mono "0000/0000/0000/0000/0000/0000/0000/0008/61f8/6028/7ff0/03fe/03fc/0000/0000/0000")
  (mask "0000/0000/0000/0000/0000/0000/001c/f3fc/f3fc/fffc/ffff/ffff/07ff/07fe/0000/0000")))

;; New icon of Ju87 Stuka dive bomber
(imf "stuka" ((8 8)
  (mono "00/00/38/24/24/24/38/00")
  (mask "ff/ff/ff/ff/ff/ff/ff/ff")))
(imf "stuka" ((16 16)
  (mono "0200/0700/0e00/0e00/0700/0380/01e0/00f0/00f0/01f0/03b8/0918/1800/3c00/0e00/0600")
  (mask "0700/0f80/1f00/1f00/0f80/07e0/03f0/01f8/01f8/03f8/0ffc/1fbc/3d18/7e00/3f00/0f00")))
(imf "stuka" ((32 32)
  (mono
   "00000000/000c0000/001e0000/003e0000/007e0000/00fc0000/00fc0000/00fc0000"
   "007e0000/003f0000/001f8000/000ff800/0007fc00/0003fe00/0001ff00/0001ff80"
   "0000ff80/0000ff80/0000ff80/0003ffc0/0007ffe0/0007e7f0/0003c3f0/000181f0"
   "002000e0/00700000/00e00000/01f80000/00bc0000/003c0000/001c0000/00000000")
  (mask
   "000c0000/001e0000/003f0000/007f0000/00ff0000/01fe0000/01fe0000/01fe0000"
   "00ff0000/007f8000/003ff800/001ffc00/000ffe00/0007ff00/0003ff80/0003ffc0"
   "0001ffc0/0001ffc0/0003ffc0/0007ffe0/000ffff0/000ffff8/0007e7f8/0023c3f8"
   "007181f0/00f800e0/01f80000/03fc0000/01fe0000/00fe0000/003e0000/001c0000")))

;; 16x16 icon is aircraft.imf/torpedo-bomber
(imf "torpedo-bomber-2" ((8 8)
  (mono "00/00/7c/10/10/10/10/00")
  (mask "ff/ff/ff/ff/ff/ff/ff/ff")))
(imf "torpedo-bomber-2" ((16 16)
  (mono "0000/0000/0000/0000/0000/0000/6000/6000/70f2/7ffe/7ffe/00a2/0ff8/0000/0000/0000")
  (mask "0000/0000/0000/0000/0000/f000/f000/f9ff/ffff/ffff/ffff/ffff/1fff/1ffc/0000/0000")))


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Notes ;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; IJN aircraft, sorted by manufacturer

(add D3A1 notes '(
;; Aichi D3A1 Val carrier dive bomber
  " Cruising speed: 160 kt at 3,000 m (184 mph at 9,845 ft)"""
  "  Maximum speed: 209 kt at 3,000 m (240 mph at 9,845 ft)"""
  "Service ceiling: 9,300 m (30,050 ft)"""
  "   Wing loading: 104.6 kg/sq m (21.4 lb/sq ft)"""
  "  Power loading: 3.65 kg/hp (8 lb/hp)"""
  "       Armament: Two fixed 7.7 mm Type 97 machine guns"""
  "                 One flexible 7.7 mm Type 92 machine gun"""
  "                 One 250 kg bomb"""
  "                 Two 60 kg bombs"""
  "          Range: 795 naut miles (915 st miles)"""
  ""
  "Two-seat single radial-engined fixed-undercarriage low-wing"
  "monoplane (oval planform) mid-tail carrier-borne dive-bomber."
  ))		   

(add D3A2 notes '(
;; Aichi D3A2 Val carrier dive bomber
  " Cruising speed: 160 kt at 3,000 m (184 mph at 9,845 ft)"""
  "  Maximum speed: 232 kt at 6,200 m (267 mph at 20,340 ft)"""
  "Service ceiling: 10,500 m (34,450 ft)"""
  "   Wing loading: 108.9 kg/sq m (22.3 lb/sq ft)"""
  "  Power loading: 2.92 kg/hp (6.4 lb/hp)"""
  "       Armament: Two fixed 7.7 mm Type 97 machine guns"""
  "                 One flexible 7.7 mm Type 92 machine gun"""
  "                 One 250 kg bomb"""
  "                 Two 60 kg bombs"""
  "          Range: 730 naut miles (840 st miles)"""
  ""
  "Two-seat single radial-engined fixed-undercarriage low-wing"
  "monoplane (oval planform) mid-tail carrier-borne dive-bomber."
  ))		   

(add E13A notes '(
;; Aichi E13A1a Jake seaplane
  " Cruising speed: 120 kt at 2,000 m (138 mph at 6,560 ft)"""
  "  Maximum speed: 203 kt at 2,180 m (234 mph at 7,155 ft)"""
  "Service ceiling: 8,730 m (28,640 ft)"""
  "   Wing loading: 101.1 kg/sq m (20.7 lb/sq ft)"""
  "  Power loading: 3.4 kg/hp (7.6 lb/hp)"""
  "       Armament: One flexible 7.7 mm Type 92 machine gun"""
  "          Range: 1,128 naut miles (1,298 st miles)"""
  ""
  "Three-seat single radial-engined low-wing monoplane high-tail"
  "twin-float reconnaissance seaplane.  No crew protection.  Could"
  "also carry one 250 kg bomb, four 60 kg bombs, or depth charges."
  ))		   

(add E7K notes '(
;; Kawanishi E7K2 Alf seaplane
  " Cruising speed: 100 kt at 1,000 m (115 mph at 3,280 ft)"""
  "  Maximum speed: 149 kt at 2,000 m (171 mph at 6,560 ft)"""
  "Service ceiling: 7,060 m (23,165 ft)"""
  "   Wing loading: 75.7 kg/sq m (15.5 lb/sq ft)"""
  "  Power loading: 3.8 kg/hp (8.4 lb/hp)"""
  "       Armament: One fixed 7.7 mm Type 92 machine gun"""
  "                 Two flexible 7.7 mm Type 92 machine guns"""
  "      Endurance: 11.32 hr"""
  ""
  "Three-seat single radial-engined staggered biplane high-tail"
  "twin-float reconnaissance seaplane.  Could also carry four 30 kg or"
  "two 60 kg bombs."
  ))		   


;; Is this the Kawanishi Zero-2 "Emily" 4-engined bomber (flying
;; boat) mentioned by Morison?
(add H6K4 notes '(
;; Kawanishi H6K4 Mavis flying boat
  " Cruising speed: 120 kt at 4,000 m (138 mph at 13,125 ft)"""
  "  Maximum speed: 183.5 kt at 4,000 m (211 mph at 13,125 ft)"""
  "Service ceiling: 9,610 m (31,530 ft)"""
  "   Wing loading: 100 kg/sq m (20.5 lb/sq ft)"""
  "  Power loading: 4.3 kg/hp (9.5 lb/hp)"""
  "       Armament: Four flexible 7.7 mm Type 92 machine guns"""
  "                 One flexible 20 mm Type 99 Model 1 cannon"""
  "                 Two 800 kg torpedoes or up to 1,000 kg of bombs"""
  "   Normal range: 2,590 naut miles (2,981 st miles)"""
  ""
  "Nine-seat quad radial-engined parasol-wing twin high-tail"
  "reconnaissance flying-boat.  No armour or fuel tank protection."
  ))		   

(add H6K5 notes '(
;; Kawanishi H6K5 Mavis flying boat
  " Cruising speed: 140 kt at 4,000 m (161 mph at 13,125 ft)"""
  "  Maximum speed: 208 kt at 6,000 m (239 mph at 19,685 ft)"""
  "Service ceiling: 9,560 m (31,365 ft)"""
  "   Wing loading: 102.9 kg/sq m (21.1 lb/sq ft)"""
  "  Power loading: 3.4 kg/hp (7.4 lb/hp)"""
  "       Armament: Four flexible 7.7 mm Type 92 machine guns"""
  "                 One flexible 20 mm Type 99 Model 1 cannon"""
  "                 Two 800 kg torpedoes or up to 1,000 kg of bombs"""
  "   Normal range: 2,667 naut miles (3,070 st miles)"""
  ""
  "Nine-seat quad radial-engined parasol-wing twin high-tail"
  "reconnaissance flying-boat.  No armour or fuel tank protection."
  ))		   

(add F1M notes '(
;; Mitsubishi F1M2 Pete seaplane
  " Cruising speed: about 120 kt (138 mph)"""
  "  Maximum speed: 200 kt at 3,440 m (230 mph at 11,285 ft)"""
  "Service ceiling: 9,440 m (30,970 ft)"""
  "   Wing loading: 86.3 kg/sq m (17.7 lb/sq ft)"""
  "  Power loading: 2.9 kg/hp (6.4 lb/hp)"""
  "       Armament: Two fixed 7.7 mm Type 97 machine guns"""
  "                 One flexible 7.7 mm Type 92 machine gun"""
  "          Range: 400 naut miles (460 st miles)"""
  ""
  "Two-seat single radial-engined staggered biplane mid-tail"
  "observation float seaplane.  Could also carry two 60 kg bombs." 
  ))		   

(add A6M2 notes '(
;; Mitsubishi A6M2 Model 21 Zeke carrier fighter
  " Cruising speed: 180 kt (207 mph)"""
  "  Maximum speed: 288 kt at 4,550 m (331.5 mph at 14,930 ft)"""
  "Service ceiling: 10,000 m (32,810 ft)"""
  "   Wing loading: 107.4 kg/sq m (22 lb/sq ft)"""
  "  Power loading: 2.5 kg/hp (5.5 lb/hp)"""
  "       Armament: Two fixed 7.7 mm Type 97 machine guns (500 rpg)"""
  "                 Two fixed 20 mm Type 99 Model 1 cannon (60 rpg)"""
  "          Range: 1,010 naut miles (1,160 st miles) clean"""
  "                 1,675 naut miles (1,930 st miles) with drop tank"""
  ""
  "Single-seat single radial-engined retractable-undercarriage"
  "low-wing monoplane (rounded folding-wingtips) mid-tail"
  "carrier-borne fighter.  No armour or fuel tank protection.  Could"
  "also carry two 60 kg bombs or a 330 L (87 US gal) drop tank."
  ))		   

(add A6M3 notes '(
;; Mitsubishi A6M3 Model 32 Zeke carrier fighter
  " Cruising speed: 200 kt (230 mph)"""
  "  Maximum speed: 294 kt at 6,000 m (338 mph at 19,685 ft)"""
  "Service ceiling: 11,050 m (36,250 ft)"""
  "   Wing loading: 118.1 kg/sq m (24.2 lb/sq ft)"""
  "  Power loading: 2.3 kg/hp (5 lb/hp)"""
  "       Armament: Two fixed 7.7 mm Type 97 machine guns (500 rpg)"""
  "                 Two fixed 20 mm Type 99 Model 1 cannon (100 rpg)"""
  "  Maximum range: 1,284 naut miles (1,477 st miles) with drop tank"""
  ""
  "Single-seat single radial-engined retractable-undercarriage"
  "low-wing monoplane (square wingtips) mid-tail carrier-borne"
  "fighter.  No armour or fuel tank protection.  Could also carry two"
  "60 kg bombs or a 330 L (87 US gal) drop tank."
  ))		   

(add G4M notes '(
;; Mitsubishi G4M1 Model 11 Betty medium bomber
  " Cruising speed: 170 kt at 3,000 m (196 mph at 9,845 ft)"""
  "  Maximum speed: 231 kt at 4,200 m (266 mph at 13,780 ft)"""
  "Service ceiling: about 9,000 m (29,500 ft)"""
  "   Wing loading: 121.6 kg/sq m (24.9 lb/sq ft)"""
  "  Power loading: 3.1 kg/hp (6.8 lb/hp)"""
  "       Armament: Three flexible 7.7 mm Type 92 machine guns"""
  "                 One flexible 20 mm Type 99 Model 1 cannon"""
  "                 One 800 kg torpedo or equivalent in bombs"""
  "  Maximum range: 3,256 naut miles (3,749 st miles)"""
  ""
  "Seven-seat twin radial-engined retractable-undercarriage mid-wing"
  "monoplane (sharply-tapered planform) high-tail land-based bomber. "
  "No armour or fuel tank protection."
  ))		   

(add B5N notes '(
;; Nakajima B5N2 Kate carrier torpedo bomber
  " Cruising speed: 140 kt at 3,000 m (161 mph at 9,845 ft)"""
  "  Maximum speed: 204 kt at 3,600 m (235 mph at 11,810 ft)"""
  "Service ceiling: 8,260 m (27,100 ft)"""
  "   Wing loading: 100.8 kg/sq m (20.6 lb/sq ft)"""
  "  Power loading: 3.8 kg/hp (8.4 lb/hp)"""
  "       Armament: One flexible 7.7 mm Type 92 machine gun"""
  "                 One 800 kg torpedo or equivalent in bombs"""
  "  Maximum range: 1,075 naut miles (1,237 st miles)"""
  ""
  "Three-seat single radial-engined retractable-undercarriage folding"
  "low gull-wing monoplane mid-tail carrier-borne torpedo bomber."
  ))		   

(add A6M2-N notes '(
;; Nakajima A6M2-N Model 11 Rufe seaplane
  " Cruising speed: 160 kt (184 mph)"""
  "  Maximum speed: 235 kt at 5,000 m (270.5 mph at 16,405 ft)"""
  "Service ceiling: 10,000 m (32,810 ft)"""
  "   Wing loading: 109.7 kg/sq m (22.5 lb/sq ft)"""
  "  Power loading: 2.6 kg/hp (5.7 lb/hp)"""
  "       Armament: Two fixed 7.7 mm Type 97 machine guns (500 rpg)"""
  "                 Two fixed 20 mm Type 99 Model 1 cannon (60 rpg)"""
  "          Range: 621 naut miles (715 st miles) normal"""
  "                 963 naut miles (1,107 st miles) maximum"""
  ""
  "Single-seat single radial-engined low-wing monoplane"
  "(rounded-wingtips) mid-tail float seaplane fighter.  No armour or"
  "fuel tank protection.  Could also carry two 60 kg bombs."
  ))		   

(add D4Y notes '(
;; Yokosuke D4Y1-C Judy carrier patrol bomber
;; (D4Y1 Model 11 with camera)
  " Cruising speed: 230 kt at 3,000 m (265 mph at 9,845 ft)"""
  "  Maximum speed: 298 kt at 4,750 m (343 mph at 15,585 ft)"""
  "Service ceiling: 9,900 m (32,480 ft)"""
  "   Wing loading: 154.7 kg/sq m (31.7 lb/sq ft)"""
  "  Power loading: 3 kg/hp (6.7 lb/hp)"""
  "       Armament: Two fixed 7.7 mm Type 97 machine guns"""
  "                 One flexible 7.92 mm Type 1 machine gun"""
  "  Maximum range: 2,100 naut miles (2,417 st miles)"""
  ""
  "Two-seat single inline-engined low-wing monoplane high-tail"
  "carrier-based reconnaissance airplane.  No armour or fuel tank"
  "protection."
  ))		   


;; USAAF/RAAF aircraft, sorted by manufacturer

(add P-39 notes '(
;; Bell P-39D/F and P-400 Airacobra fighter
  " Cruising speed: 231 mph clean"""
  "                 196 mph with drop tank"""
  "  Maximum speed: 368 mph at 12,000 ft"""
  "Service ceiling: 32,100 ft"""
  "   Wing loading: 35.2 lb/sq ft"""
  "  Power loading: 6.5 lb/hp"""
  "       Armament: One fixed 37 mm M4 cannon (30 rpg)"""
  "                 (Replaced by 20 mm Hispano (60 rpg) in P-400)"""
  "                 Two fixed 0.50\" M2 machine guns (200 rpg)"""
  "                 Four fixed 0.30\" M2 machine guns (1000 rpg)"""
  "          Range: 600 st miles clean"""
  "                 1,100 st miles with drop tank"""
  ""
  "Single-seat single rear inline-engined retractable"
  "tricycle-undercarriage low-wing monoplane high-tail land-based"
  "fighter.  Pilot armour and self-sealing fuel tanks.  Could also"
  "carry one 500 lb bomb or a 75 US gal drop tank." 
  ))

(add B-17 notes '(
;; Boeing B-17E Flying Fortress heavy bomber
  " Cruising speed: 195-223 mph"""
;; Call the cruising speed 210 mph
  "  Maximum speed: 318 mph at 25,000 ft"""
  "Service ceiling: 36,600 ft"""
  "   Wing loading: 28.4 lb/sq ft"""
  "  Power loading: 8.4 lb/hp"""
  "       Armament: Eight flexible 0.50\" M2 machine guns"""
  "                 One flexible 0.30\" M2 machine gun"""
  "                 Up to 8,000 lb in bombs"""
  "          Range: 2,000 st miles with 4,000 lb bombload"""
  "                 3,300 st miles maximum"""
  ""
  "Ten-seat glass-nosed quad radial-engined"
  "retractable-undercarriage low-wing monoplane high-tail land-based"
  "heavy bomber.  Crew armour and self-sealing fuel tanks." 
  ))

(add P-40E notes '(
;; Curtiss P-40E fighter
  " Cruising speed: 308 mph"""
  "  Maximum speed: 362 mph at 15,000 ft"""
  "Service ceiling: 29,000 ft"""
  "   Wing loading: 35.1 lb/sq ft"""
  "  Power loading: 7.2 lb/hp"""
  "       Armament: Six fixed 0.50\" M2 machine guns"""
  "          Range: 650 st miles clean"""
  "                 850 st miles with 52 US gal drop tank"""
  "                 1,400 st miles with 170 US gal drop tank"""
  ""
  "Single-seat single inline-engined retractable-undercarriage"
  "low-wing monoplane high-tail land-based fighter.  Pilot armour and"
  "self-sealing fuel tanks.  Could also carry one 500 lb bomb or a"
  "drop tank."
  ))

(add P-40F notes '(
;; Curtiss P-40F Warhawk fighter
  " Cruising speed: 300 mph"""
  "  Maximum speed: 364 mph at 10,000 ft"""
  "Service ceiling: 34,400 ft"""
  "   Wing loading: 36.0 lb/sq ft"""
  "  Power loading: 6.5 lb/hp"""
  "       Armament: Six fixed 0.50\" M2 machine guns"""
  "          Range: 700 st miles clean"""
  "                 875 st miles with 52 US gal drop tank"""
  "                 1,500 st miles with 170 US gal drop tank"""
  ""
  "Single-seat single inline-engined retractable-undercarriage"
  "low-wing monoplane high-tail land-based fighter.  Pilot armour and"
  "self-sealing fuel tanks.  Could also carry one 500 lb bomb or a"
  "drop tank.  Differs from the P-40E principally in having the"
  "Alison engine replace by a Packard-built Rolls-Royce Merlin."
  ))

(add A-24 notes '(
;; Douglas A-24 Dauntless dive bomber; same as SBD-3 less tailhook
  " Cruising speed: 173 mph"""
  "  Maximum speed: 250 mph at 17,200 ft"""
  "Service ceiling: 26,000 ft"""
  "   Wing loading: 30.3 lb/sq ft"""
  "  Power loading: 9.8 lb/hp"""
  "       Armament: Two fixed 0.50\" M2 machine guns"""
  "                 Two flexible 0.30\" M2 machine guns"""
  "                 Up to 1,200 lb in bombs"""
  "          Range: 950 st miles with normal bombload"""
  "                 1,300 st miles clean"""
  ""
  "Two-seat single radial-engined retractable-undercarriage low"
  "inverted gull-wing monoplane (perforated flaps) mid-tail land-based"
  "dive bomber.  Armour and self-sealing fuel tanks.  Identical to"
  "the SBD-3 except for the tailhook."
  ))

(add A-20 notes '(
;; Douglas A-20A Havoc medium bomber
  " Cruising speed: 295 mph"""
  "  Maximum speed: 347 mph at 12,400 ft"""
  "Service ceiling: 28,175 ft"""
  "   Wing loading: 42.6 lb/sq ft"""
  "  Power loading: 6.2 lb/hp"""
  "       Armament: Four fixed 0.30\" M2 machine guns"""
  "                 Three flexible 0.30\" M2 machine guns"""
  "                 Up to 2,400 lb in bombs"""
  "          Range: 525 st miles with 2,400 lb bombload"""
  "                 675 st miles with 1,200 lb bombload"""
  "                 2,000 st miles maximum"""
  ""
  "Three-seat glass-nosed twin underwing-mounted radial-engined"
  "retractable tricycle-undercarriage mid-wing monoplane (rectangular"
  "fuselage cross-section) high-tail land-based medium bomber."
  "No self-sealing fuel tanks."
  ))

(add Hudson notes '(
;; Lockheed Hudson Mk IIIA medium bomber
  " Cruising speed: 205 mph"""
  "  Maximum speed: 253 mph at 15,000 ft"""
  "Service ceiling: 26,500 ft"""
  "   Wing loading: 37.2 lb/sq ft"""
  "  Power loading: 8.5 lb/hp"""
  "       Armament: Two fixed 0.303\" Browning machine guns"""
  "                 Two flexible 0.303\" Browning machine guns"""
  "          Range: 1,550 st miles with 1,400 lb bombload"""
  "                 2,800 st miles maximum"""
  ""
  "Five-seat twin radial-engined retractable-undercarriage mid-wing"
  "monoplane (sharply-tapered planform) twin high-tail land-based"
  "medium bomber." 
  ))

(add P-38 notes '(
;; Lockheed P-38G Lightning fighter
  " Cruising speed: 219 mph clean"""
  "                 211 mph with drop tanks"""
  "  Maximum speed: 400 mph at 25,000 ft"""
  "Service ceiling: 39,000 ft"""
  "   Wing loading: 48.2 lb/sq ft"""
  "  Power loading: 6.0 lb/hp"""
  "       Armament: One fixed 20 mm Hispano-Suiza Mk II cannon (150 rpg)"""
  "                 Four fixed 0.50\" M2 machine guns (500 rpg)"""
  "          Range: 850 st miles clean"""
  "                 1,750 st miles with drop tanks"""
  ""
  "Single-seat twin inline-engined retractable tricycle-undercarriage"
  "twin-boom mid-wing monoplane twin high-tail land-based fighter."
  "Could carry up to 2,000 lb of bombs, or two 150 US gal drop"
  "tanks."  
  ))

(add B-26 notes '(
;; Martin B-26A Marauder medium bomber
  " Cruising speed: 265 mph with 3,000 lb bombload"""
  "                 243 mph without bombs or torpedo"""
  "  Maximum speed: 313 mph at 15,000 ft"""
  "Service ceiling: 23,500 ft"""
  "   Wing loading: 47.1 lb/sq ft"""
  "  Power loading: 7.7 lb/hp"""
  "       Armament: Two flexible 0.30\" M2 machine guns"""
  "                 Three flexible 0.50\" M2 machine guns"""
  "                 Up to 5,800 lb in bombs and torpedo"""
  "          Range: 1,000 st miles with 3,000 lb bombload"""
  "                 2,600 st miles maximum"""
  ""
  "Seven-seat glass-nosed twin underwing-mounted radial-engined"
  "retractable tricycle-undercarriage high-wing monoplane (circular"
  "fuselage cross-section) high-tail land-based medium bomber."
  "Armour and self-sealing fuel tanks.  Needs much longer airstrips"
  "than other aircraft its size."
  ))

(add B-25 notes '(
;; North American B-25C/D Mitchell medium bomber
  " Cruising speed: 233 mph at 15,000 ft"""
  "  Maximum speed: 284 mph at 15,000 ft"""
  "Service ceiling: 24,000 ft"""
  "   Wing loading: 51.6 lb/sq ft"""
  "  Power loading: 9.3 lb/hp"""
  "       Armament: Six flexible 0.50\" M2 machine guns"""
  "                 Up to 5,200 lb in bombs, or one 22\" 2,000 lb torpedo"""
  "          Range: 1,500 st miles with 3,000 lb bombload"""
  ""
  "Five-seat glass-nosed twin underwing-mounted radial-engined"
  "retractable tricycle-undercarriage mid gull-wing monoplane twin"
  "high-tail land-based medium bomber."
  ))


;; USN aircraft, sorted by manufacturer

(add F2A notes '(
;; Brewster F2A-3 Buffalo carrier fighter
  " Cruising speed: 161 mph"""
  "  Maximum speed: 321 mph at 16,500 ft"""
  "Service ceiling: 33,200 ft"""
  "   Wing loading: 30.2 lb/sq ft"""
  "  Power loading: 5.3 lb/hp"""
  "       Armament: Four fixed 0.50\" M2 machine guns"""
  "   Normal Range: 965 st miles"""
  ""
  "Single-seat single radial-engined retractable-undercarriage"
  "mid-wing monoplane mid-tail (oval planform) carrier-borne"
  "fighter.  Armour for pilot and fuel tanks." 
  ))		   

(add PBY-5 notes '(
;; Consolidated PBY-5 Catalina flying boat
  " Cruising speed: 115 mph"""
  "  Maximum speed: 189 mph at 7,000 ft"""
  "Service ceiling: 18,100 ft"""
  "   Wing loading: 34000/1400 lb/sq ft"""
  "  Power loading: 34000/2400 lb/hp"""
  "       Armament: Two flexible 0.50\" M2 machine guns"""
  "                 Two flexible 0.30\" M2 machine guns"""
  "                 Up to four 1,000 lb bombs or four 325 lb depth charges"""
  "          Range: 2,990 st miles"""
  ""
  "Seven to nine-seat double radial-engined parasol-wing monoplane"
  "parasol-tail flying boat, with two outboard floats which fold to"
  "make the wingtips." 
  ))

(add PBY-5A notes '(
;; Consolidated PBY-5A Catalina flying boat
  " Cruising speed: 113 mph"""
  "  Maximum speed: 175 mph at 7,000 ft"""
  "Service ceiling: 13,000 ft"""
;  "   Wing loading: 35420/1400 lb/sq ft"""
;  "  Power loading: 35420/2400 lb/hp"""
  "       Armament: Two flexible 0.50\" M2 machine guns"""
  "                 Two flexible 0.30\" M2 machine guns"""
  "                 Up to four 1,000 lb bombs or four 325 lb depth charges"""
  "          Range: 2,350 st miles"""
  ""
  "Seven to nine-seat double radial-engined retractable"
  "tricycle-undercarriage parasol-wing monoplane parasol-tail"
  "amphibious flying boat, with two outboard floats which fold to"
  "make the wingtips." 
  ))

(add PB4Y notes '(
;; Consolidated PB4Y-1 Liberator heavy bomber (same as B-24D)
  " Cruising speed: 200 mph"""
  "  Maximum speed: 303 mph at 25,000 ft"""
  "Service ceiling: 32,000 ft"""
  "   Wing loading: 52.5 lb/sq ft"""
  "  Power loading: 11.5 lb/hp"""
  "       Armament: Eight flexible 0.50\" M2 machine guns"""
  "                 Up to 8,800 lb in bombs"""
  "          Range: 2,300 st miles with 5,000 lb bombload"""
  "                 3,500 st miles maximum"""
  ""
  "Nine to ten-seat quad radial-engined retractable"
  "tricycle-undercarriage high-wing monoplane twin high-tail"
  "land-based heavy bomber." 
  ))

(add TBD notes '(
;; Douglas TBD-1 Devastator carrier torpedo bomber
  " Cruising speed: 128 mph"""
  "  Maximum speed: 206 mph at 8,000 ft"""
  "Service ceiling: 19,700 ft"""
;  "   Wing loading: 10194/422 lb/sq ft"""
;  "  Power loading: 10194/900 lb/hp"""
  "       Armament: One fixed 0.30\" M2 machine gun"""
  "                 One flexible 0.30\" M2 machine gun"""
  "                 One 1,000 lb torpedo or 1,000 lb bomb"""
  "          Range: 435 st miles with torpedo"""
  "                 716 st miles with 1,000 lb bomb"""
  ""
  "Three-seat single radial-engined semi-retractable undercarriage"
  "folding low-wing monoplane high-tail carrier-borne torpedo"
  "bomber." 
  ))

(add SBD notes '(
;; Douglas SBD-3/4 Dauntless carrier dive bomber
;; Same as A-24 with tailhook
  " Cruising speed: 173 mph"""
  "  Maximum speed: 250 mph at 17,200 ft"""
  "Service ceiling: 26,000 ft"""
  "   Wing loading: 30.3 lb/sq ft"""
  "  Power loading: 9.8 lb/hp"""
  "       Armament: Two fixed 0.50\" M2 machine guns"""
  "                 Two flexible 0.30\" M2 machine guns"""
  "                 Up to 1,200 lb in bombs"""
  "          Range: 950 st miles with normal bombload"""
  "                 1,300 st miles clean"""
  ""
  "Two-seat single radial-engined retractable-undercarriage low"
  "inverted gull-wing monoplane (perforated flaps) mid-tail"
  "carrier-borne dive bomber.  Armour and self-sealing fuel tanks."
  "Identical to the A-24 except for the tailhook."
  ))

(add F4F-3 notes '(
;; Grumman F4F-3 Wildcat carrier fighter
  " Cruising speed: 185 mph"""
  "  Maximum speed: 330 mph at 22,000 ft"""
  "Service ceiling: 31,000 ft"""
  "   Wing loading: 28.7 lb/sq ft"""
  "  Power loading: 6.2 lb/hp"""
  "       Armament: Four fixed 0.50\" M2 machine guns"""
  "          Range: 845 st miles"""
  ""
  "Single-seat single radial-engined retractable-undercarriage"
  "mid-wing monoplane high-tail carrier-borne fighter.  Although"
  "slower and much less maneuvrable than the Zero, proper tactics like"
  "the \"Thatch weave\" and its rugged construction allowed it to hold"
  "its own."
  ))		   

(add F4F-4 notes '(
;; Grumman F4F-4 Wildcat carrier fighter
  " Cruising speed: 161 mph"""
  "  Maximum speed: 320 mph at 18,800 ft"""
  "Service ceiling: 34,000 ft"""
  "   Wing loading: 30.7 lb/sq ft"""
  "  Power loading: 6.7 lb/hp"""
  "       Armament: Six fixed 0.50\" M2 machine guns (240 rpg)"""
  "          Range: 830 st miles clean"""
  "                 1,275 st miles with drop tanks"""
  ""
  "Single-seat single radial-engined retractable-undercarriage"
  "folding mid-wing monoplane high-tail carrier-borne fighter."
  "Although slower than its predecessor, the extra guns and addition"
  "of armour, self-sealing protection for the fuel tanks, and"
  "wing-folding mechanism made it an improvement overall.  The"
  "hinge line was reportedly experimentally determined by Roy Grumman"
  "with the help of an eraser into which two bent paperclips were"
  "inserted, representing the wings."
  ))		   

(add TBF notes '(
;; Grumman TBF-1 Avenger carrier torpedo bomber
  " Cruising speed: 145 mph"""
  "  Maximum speed: 271 mph at 12,000 ft"""
  "Service ceiling: 22,400 ft"""
;  "   Wing loading: 15905/490 lb/sq ft"""
;  "  Power loading: 15905/1700 lb/hp"""
  "       Armament: One fixed 0.50\" M2 machine gun"""
  "                 One flexible 0.50\" M2 machine gun"""
  "                 One flexible 0.30\" M2 machine gun"""
  "                 One 2,000 lb torpedo"""
  "          Range: 1,215 st miles with torpedo"""
  "                 1,450 st miles clean"""
  ""
  "Three-seat single radial-engined retractable-undercarriage"
  "folding mid-wing monoplane high-tail carrier-borne torpedo bomber."
  ))

(add SB2U notes '(
;; Vought SB2U-3 Vindicator carrier dive bomber
  " Cruising speed: 152 mph"""
  "  Maximum speed: 243 mph at 9,500 ft"""
  "Service ceiling: 23,600 ft"""
;  "   Wing loading: 9421/305 lb/sq ft"""
;  "  Power loading: 9421/825 lb/hp"""
  "       Armament: One fixed 0.50\" M2 machine gun"""
  "                 One flexible 0.50\" M2 machine gun"""
  "                 Up to ? lb in bombs"""
  "          Range: 1,120 st miles"""
  ""
  "Two-seat single radial-engined retractable-undercarriage folding"
  "low-wing monoplane high-tail dive bomber."
  ))

(add OS2U notes '(
;; Vought OS2U-3 Kingfisher seaplane
  " Cruising speed: 119 mph at 5,000 ft"""
  "  Maximum speed: 164 mph at 5,500 ft"""
  "Service ceiling: 13,000 ft"""
;  "   Wing loading: 6000/262 lb/sq ft"""
;  "  Power loading: 6000/450 lb/hp"""
  "       Armament: One fixed 0.30\" M2 machine gun"""
  "                 One flexible 0.30\" M2 machine gun"""
  "          Range: 805 st miles"""
  ""
  "Two-seat single radial-engined mid-wing monoplane high-tail float"
  "seaplane." 
  ))


;; Ships

(add oiler notes '( 
  " Displacement: 10,000 tons"""
  "Maximum speed: 18 kt"""
  "     Armament: One 5\" gun"""
  "               Light AA armament"""
  ""
  "A non-combatant fuel tanker."
  ))

(add tender notes '(
  " Displacement: 10,000 tons"""
  "Maximum speed: 18 kt"""
  "     Armament: One 5\" gun"""
  "               Light AA armament"""
  ""
  "A non-combatant submarine and seaplane tender."
  ))

(add transport notes '(
  " Displacement: 10,000 tons"""
  "Maximum speed: 18 kt"""
  "     Armament: One 5\" gun"""
  "               Light AA armament"""
  ""
  "A non-combatant to carry troops and supplies.  These soldiers can"
  "be used to attack enemy-held towns and airfields."
  ))

(add DD notes '(
  " Displacement: 2,000 tons"""
  "Maximum speed: 36 kt"""
  "     Armament: Five 5\" guns"""
  "               Light AA armament"""
  "               Torpedo tubes"""
  "               Depth charge racks"""
  ""
  "A small multi-role ship, with the primary purpose of protecting"
  "other ships from submarines.  Its secondary roles are ship-to-ship"
  "combat, scouting, transport, and rescue.  Destroyers carry torpedoes"
  "to attack heavily-armoured warships, against which its guns"
  "are useless.  The torpedoes are especially useful in night"
  "battles."
  ))

(add sub notes '(
;  " Displacement: ? tons"""
;  "Maximum speed: ? kt"""
  "     Armament: One 5\" gun"""
  "               Torpedo tubes"""
  ""
  "The primary purpose of submarines is interdicting merchant"
  "shipping, although they can also be useful against large warships,"
  "supplying beleaguered garrisons, and in commando raids and other"
  "covert operations.  They are vulnerable to aircraft, and to ships"
  "which carry depth charges such as destroyers and light cruisers."
  ))

(add CL notes '(
  " Displacement: 6,000 tons"""
  "Maximum speed: 34 kt"""
  "     Armament: Six 6\" guns"""
  "               Medium AA armament"""
  "               Torpedo tubes"""
  "               Depth charge racks"""
  "               Two seaplanes"""
  ""
  "A lightly-armoured cruiser, traditionally employed for"
  "scouting and as a destroyer leader.  Gradually it became more"
  "specialized in protecting capital ships from aircraft.  Its"
  "torpedoes are used to attack battleships, against which its guns"
  "are useless."
  ))

(add CA notes '(
  " Displacement: 12,000 tons"""
  "Maximum speed: 33 kt"""
  "     Armament: Nine 8\" guns"""
  "               Six 5\" guns"""
  "               Medium AA armament"""
  "               Two seaplanes"""
  ""
  "A heavily-armoured cruiser, traditionally employed to raid merchant"
  "shipping and as a scout for squadrons of battleships."
  ))

(add BB notes '(
  " Displacement: 40,000 tons"""
  "Maximum speed: 28 kt"""
  "     Armament: Nine 16\" guns"""
  "               Sixteen 6\" guns"""
  "               Heavy AA armament"""
  "               Two seaplanes"""
  ""
  "A direct descendent of the Dreadnought, it is designed to be the"
  "ultimate floating gun platform.  Its main armament is all the same"
  "calibre to ease gunnery, and its armour is proof against all but the"
  "largest shells.  Designed to box toe-to-toe with the biggest and"
  "meanest, it can both deal out and take a frightening amount of"
  "damage.  " 
  ))

(add CVE notes '(
  " Displacement: 15,000 tons"""
  "Maximum speed: 32 kt"""
  "     Armament: Six 6\" guns"""
  "               Medium AA armament"""
  "               Forty aircraft"""
  ""
  "Smaller and more lightly armed than the fleet aircraft carrier,"
  "the light carrier is often used for escort duties."
  ))

(add CV notes '(
  " Displacement: 30,000 tons"""
  "Maximum speed: 32 kt"""
  "     Armament: Eight 6\" guns"""
  "               Heavy AA armament"""
  "               Eighty aircraft"""
  ""
  "Lightly-armoured, it relies on its aircraft and attendant ships to"
  "provide the necessary protection.  Like the battleship, it is a"
  "large ship and can adsorb quite a bit of punishment."
  ))


(game-module (notes (
  ""
  "When airplanes land on an aircraft carrier, they stop themselves by"
  "catching (\"trapping\") one of a set of wires laid across the rear"
  "of the deck with a hook attached to the tail of the airplane -- "
  "the tailhook."""
  ""
  "This game is small scale (30 miles/hex and 1 hour/turn) and quite"
  "detailed, aiming to capture the flavour of aircraft carrier"
  "operations in the"
  "Pacific Ocean in early World War II.  Out of all the carrier battles"
  "fought in the Pacific, only five had the opponents so evenly"
  "matched as to make the outcome uncertain.  They occurred in mid to"
  "late 1942: Coral Sea (4-8 May), Midway (4-6 June), Eastern Solomans"
  "(24-25 August), Santa Cruz Islands (26 October), and Guadalcanal"
  "(12-15 November)."""
  ""
  "In creating this game I started from \"flattop\", "
  "changing the name to avoid confusion.  All of the aircraft fall into"
  "one of several categories which behave similarly, individual types"
  "within each class differing only in range, cruise speed, and where"
  "they may land.  For now, the make and model have little more effect than"
  "adding character."""
  ""
  "Aircraft categories include fighters, seaplanes, flying boats,"
  "heavy bombers, medium bombers, dive bombers, and torpedo bombers."
  "Only aircraft with the word \"carrier\" appearing in their short"
  "description (e.g. \"A flight of Grumman F4F-3 Wildcat carrier"
  "fighters\") are capable of landing on aircraft carriers.  Only"
  "seaplanes and flying boats can land at seaplane bases, but they"
  "cannot land at airfields.  Seaplanes can also fly from cruisers and"
  "battleships."""
  ""
  "In this game all undamaged ships travel at the same speed, and"
  "aircraft fly at their cruise speed (not their maximum speed, "
  "which will eventually affect combat).  Only aircraft need fuel and"
  "ammunition at this scale."""
  ""
  ""
  "  Detailed aircraft gun statistics:"""
  ""
  "              Calibre  Projectile  Rate of      Muzzle    Effective"""
  "Name           (mm)     Mass (g)  Fire (rpm)  Vel. (m/s)  Range (m)"""
 ".30\" M2          7.62      9.9      1200         835"""
  "Type 92          7.7      11.3       600         762         600"""
  "Type 97          7.7      11.3      1000         750         600"""
 ".303\" Browning   7.7      11.3      1140         745"""
  "Type 1           7.92     11.5      1000         789         600"""
 ".50\" M2         12.7      48.5       750         870"""
  "Hispano Mk.II   20       130         600         880"""
  "Type 99 mod 1   20       142         490         600         800"""
  "37 mm M4        37       608         140         610"""
  ""
  "  Primary sources:"
  ""
  "S.E. Morison, History of USN Operations in WWII (OUP: London,"
  "1949)."""
  "R.J. Francillon, Japanese Aircraft of the Pacific War (Putnam:"
  "London, 1979)."""
  "G. Swanborough and P.M. Bowers, United States Navy Aircraft"
  "Since 1911 (Putnam: London, 1990)."""
  "http://www.csd.uwo.ca/~pettypi/elevon/baugher_us/"""
  "http://www.combinedfleet.com/"
  "http://www.uss-salem.org/"""
  ""
  "  Other sources:"
  ""
  "G. Swanborough and P.M. Bowers, United States Military Aircraft"
  "Since 1909 (Putnam: London, 1989)."""
  "D. Carpenter and N. Polmar, Submarines of the Imperial Japanese"
  "Navy (Naval Institute Press: Annapolis, 1986)."""
  "http://www.csd.uwo.ca/~pettypi/elevon/gustin_military/"""
  ""
  ""
  "Keir Novik"""
  "(K.E.Novik@qmw.ac.uk)"""
  "July 1999"
  )))

(game-module (design-notes (
  "The length scale was chosen because of the availability of a good"
  "50 km (30 miles) per hex map.  The time scale was chosen because"
  "move-to orders don't work well for units that move less than one"
  "hex a turn, hence anything shorter than one hour per turn would make"
  "ships unrealistically fast.  It would be interesting to add more"
  "ship units, for example distinguishing between different classes of"
  "carriers.  However, there are already a ridiculous number of units,"
  "and the variations in aircraft are more significant.  I could reduce"
  "the number of models of aircraft (e.g. P-40E/P-40F --> P-40), but I"
  "have the data already.  Attacking aircraft in places and carriers"
  "needs to be tuned.  Night effects and vision mistakes should be"
  "added."
  )))

#|
                               Max   Cruise  Normal   Rel.
                              Speed  Speed   Range   Speed   Range
                              (mph)  (mph)  (miles)         (hexes)
 Carrier fighters (VF)
Brewster F2A-3 Buffalo         321    161     965    5.37     32.2
Grumman F4F-3 Wildcat          330    185     845    6.17     28.2
Grumman F4F-4 Wildcat          320    161    1275    5.37     42.5
Mitsubishi A6M2 Model 21 Zeke  331.5  207    1930    6.90     64.3
Mitsubishi A6M3 Model 32 Zeke  338    230    1477    7.67     49.2

 Land-based pursuit fighters
Lockheed P-38G Lightning       400    211    1750    7.03     58.3
Bell P-39D/F Airacobra         368    196    1100    6.53     36.7
Curtiss P-40E                  362    308     850   10.27     28.3
Curtiss P-40F Warhawk          364    300     875   10.00     29.2

 Reconnaissance/observation seaplanes (VSO/VO)
Vought OS2U-3 Kingfisher       164    119     805    3.97     26.8
Aichi E13A1a Jake              234    138    1298    4.60     43.3
Kawanishi E7K2 Alf             171    115    1300    3.83     43.3
Mitsubishi F1M2 Pete           230            460    4.6      15.3
Nakajima A6M2-N Rufe           270.5  184    1107    6.13     36.9

 Light bombers
Vought SB2U-3 Vindicator       243    152    1120    5.07     37.3
Douglas SBD-3/4 Dauntless      250    173     950    5.77     31.7
Douglas A-24 Dauntless         250    173     950    5.77     31.7
Douglas TBD-1 Devastator       206    128     435    4.27     14.5
Grumman TBF-1 Avenger          271    145    1215    4.83     40.5
Aichi D3A1 Val                 240    184     915    6.13     30.5
Aichi D3A2 Val                 267    184     840    6.13     28.0
Nakajima B5N2 Kate             235    161    1237    5.37     41.2
Yokosuke D4Y1-C Judy           343    265    2417    8.83     80.6

 Medium bombers
Consolidated PBY-5 Catalina    189    115    2990    3.83     99.7
Consolidated PBY-5A Catalina   175    113    2350    3.77     78.3
Douglas A-20A Havoc            347    295     675    9.83     22.5
N. American B-25C/D Mitchell   284    233    1500    7.77     50.0
Martin B-26A Marauder          313    265    1000    8.83     33.3
Lockheed Hudson Mk IIIA        253    205    1550    6.83     51.7
Mitsubishi G4M1 Model 11 Betty 266    196    3749    6.53    125.0

 Heavy bombers
Boeing B-17E Flying Fortress   318    210    2000    7.00     66.7
Consolidated PB4Y-1 Liberator  303    200    2300    6.67     76.7
Kawanishi H6K4 Mavis           211    138    2981    4.60    126.0
Kawanishi H6K5 Mavis           239    161    3070    5.37    140.3

Cruise speed is roughly 60% max speed for aircraft.  Carriers can do
40mph flank, so we estimate 30mph (50kph) sustained with zigzagging.

Rel. speed = cruise speed / 30miles / 1 hour/turn
  (i.e. hexes/turn at cruise speed)

Sustained rate of turn is inversely proportional to power loading and
wing loading.
(I.e \dot{xi} \prop [(power loading)(wing loading)]^{-1})

 Battles:
Coral Sea (4-8 May 42) {F4F-3, SBD-2/3, TBD-1; A6M2, B5N2, D3A1,
  F1M2}
Midway (4-6 Jun 42) {F4F-4 (only VMF-221 had F4F-3), SBD-1/2/3, TBD-1; 
  A6M2, B5N2, D3A1} 
Eastern Solomans (24-25 Aug 42) {F4F-4, SBD-3, TBF-1; A6M2, B5N2,
  D3A1, A6M2-N}
Santa Cruz Islands (26 Oct 42) {F4F-4, SBD-3, TBF-1; A6M2, B5N2,
  D3A2?, D4Y1-C?}
Guadalcanal (12-15 Nov 42) {F4F-4, SBD-3, TBF-1; A6M2/3, B5N2,
  D3A2?, D4Y1-C?}

Lexington VB-2 was SBD-2; VS-2 was SBD-3
Saratoga VB-3 and VS-3 were SBD-3
Yorktown VB-5 and VS-5 were SBD-3
Enterprise VB-6 was SBD-2; VS-6 was SBD-3

|#
