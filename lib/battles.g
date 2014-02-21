(game-module "battles"

  (title "Star Fleet Battles")

  (version "1.00")

  (blurb "This was just a test of item units, now it's a game based roughly on 
       the old Star Fleet Battles boardgame.")

  (variants

     (sequential true)

     (world-seen true)

     (see-all false)

  )



  )



(set terrain-seen true)

(set indepside-has-ai false)

(set protection-resists-capture false)

;; (set self-required true)





;; Each system produces 3/2/1/0 Ship Status points, giving the player some idea of how well the ship is functioning, as well as, maybe, providing the AI with a reason to flee.



;; Skeleton-crewed ships (Captured and ST3) with reduced effectiveness of occupants as well as reduced ACP from eng

;; Consumption-per-attack of marines?



;; SHIPS



(unit-type enterprise (name "Constitution Class") (image-name "kb-st-ship-enterprise")

  (acp-per-turn 0) (hp-max 100) (vision-range 1) (acp-to-fire 5) (range 5)  

  (point-value 50)

  (help "A Constitution class cruiser.  Fast and durable, it is the workhorse of the Federation fleet."))



(unit-type reliant (name "Miranda Class") (image-name "kb-st-ship-reliant")

  (acp-per-turn 0) (hp-max 70) (vision-range 1) (acp-to-fire 5) (range 5)  

  (point-value 40)

  (help "A Miranda class light cruiser.  Still a powerful combat vessel, but relying more on manueverability than firepower to overcome opponents."))



(unit-type excelsior (name "Excelsior Class") (image-name "kb-st-ship-excelsior")

  (acp-per-turn 0) (hp-max 150) (vision-range 1) (acp-to-fire 5) (range 5)  

  (point-value 60)

  (help "An Excelsior class battlecruiser.  The most powerful Federation starship in regular service."))



(unit-type oberth (name "Oberth Class") (image-name "kb-st-ship-oberth")

  (acp-per-turn 0) (hp-max 30) (vision-range 1)

  (point-value 20)

  (help "An Oberth class research vessel.  Lightly shielded and poorly armed, but equipped with powerful sensors."))



(unit-type ktinga (name "K'Tinga Class") (image-name "kb-kling-ship-ktinga")

  (acp-per-turn 0) (hp-max 85) (vision-range 1) (acp-to-fire 5) (range 5)  

  (point-value 50)

  (help "A K'Tinga class heavy cruiser.  This is a modern, heavy Klingon Warship.  Not quite as resiliant or well-armed as an Enterprise-class starship but equipped with a cloaking device."))



(unit-type d7 (name "D7 Class") (image-name "kb-kling-ship-d7")

  (acp-per-turn 0) (hp-max 60) (vision-range 1) (acp-to-fire 5) (range 5)  

  (point-value 30)

  (help "A D7 class cruiser.  An older ship, once considered a heavy cruiser, still maintained within the Klingon fleet for picket and homeguard duty."))



(unit-type bop (name "Bird of Prey Class") (image-name "kb-kling-ship-bop")

  (acp-per-turn 0) (hp-max 30) (vision-range 1) (acp-to-fire 5) (range 5)  

  (point-value 20)

  (help "A Bird of Prey class patrol ship.  A small ship, lightly armed, that can be a potent combat vessel with liberal use of its cloaking device."))



(unit-type cloaked-ktinga (name "Cloaked K'Tinga Class") (image-name "kb-kling-cloak-ktinga")

  (acp-per-turn 0) (hp-max 85) (vision-range 1)

  (point-value 50)

  (help "A cloaked K'Tinga class heavy cruiser.  While cloaked this ship gains no protection from shields and cannot fire weaponry, but is very hard to spot."))



(unit-type cloaked-d7 (name "Cloaked D7 Class") (image-name "kb-kling-cloak-d7")

  (acp-per-turn 0) (hp-max 60) (vision-range 1)

  (point-value 30)

  (help "A cloaked D7 class cruiser.  While cloaked this ship gains no protection from shields and cannot fire weaponry, but is very hard to spot."))



(unit-type cloaked-bop (name "Cloaked Bird of Prey Class") (image-name "kb-kling-cloak-bop")

  (acp-per-turn 0) (hp-max 30) (vision-range 1) (acp-to-fire 5) (range 3)  

  (point-value 20)

  (help "A cloaked Bird of Prey class patrol ship.  While cloaked this ship gains no protection from shields and cannot fire weaponry, but is very hard to spot."))



(unit-type transport (name "Transport") (image-name "kb-st-ship-transport")

  (acp-per-turn 0) (hp-max 20) (vision-range 1)

  (point-value 10)

  (help "A transport vessel."))



;; STATIONS



(unit-type fedstation1 (name "Outpost") (image-name "kb-st-station-ds9")

  (acp-per-turn 0) (hp-max 200) (vision-range 1)

  (point-value 80)

  (help "A deep space station with full defensive capabilities and heavy shields.  Should only be approached with a fleet intent on assault.  Can only be captured by ships of heavy cruiser or higher class."))



(unit-type research (name "Research Station") (image-name "kb-st-station-research")

  (acp-per-turn 0) (hp-max 100) (vision-range 1)

  (point-value 40)

  (help "A Federation research station."))


(unit-type shipyard (name "Shipyard") (image-name "kb-st-station-yard")

  (acp-per-turn 1) (hp-max 200) (vision-range 1)

  (point-value 80) (speed 0)

  (help "A drydock and shipyard for the creation of new starships."))



(unit-type pt (name "Photon Torpedo") (image-name "kb-explosion-orange")

  (acp-per-turn 6) (hp-max 1) (vision-range 0) (cp 1)

  (point-value 0) (detonate-on-death 100) (acp-to-detonate 1) (hp-per-detonation 1)

  (help "Photon torpedoes.  They can cause heavy damage to enemy ships but aren't as accurate as beam weapons."))



(unit-type fedmarker (name "Ship") (image-name "ba-sensor-contact")

  (acp-independent true) (hp-max 1)

  (point-value 50)

  (help "A mass with emitting energy and having the characteristics of a ship seen at long range."))



(unit-type fedmarker-1 (name "Unidentified Mass") (image-name "ba-sensor-contact-1")

  (acp-independent true) (hp-max 1)

  (point-value 50)

  (help "A mass with no energy readings seen at long range.  Likely a hulk with an ejected core."))



(unit-type fedmarker-2 (name "Unstable Energy Readings") (image-name "ba-sensor-contact-2")

  (acp-independent true) (hp-max 1)

  (point-value 50)

  (help "A mass with strange energy readings seen at long range.  Likely a hulk with an reactor core at critical levels.")

)



(unit-type victorypoint (name "Victory Point") (image-name "ba-vp-blue")

  (acp-independent true) (hp-max 7)

  (point-value 1) (see-always true)

  (help "Capture these to win in a Conquest game."))





;; Operational Systems Types



(unit-type phaser (name "Phaser Bank") (image-name "ba-ray-green")

  (acp-per-turn 2) (hp-max 10) (vision-range 0) (acp-to-fire 1) (range 8)  

  (point-value 3) (detonate-on-death 5) (hit-falloff-range 3)

  (ai-tactical-range 8)

  (help "A fully-operational phaser bank."))



(unit-type disruptor (name "Light Disruptor") (image-name "ba-3dash-green")

  (acp-per-turn 3) (hp-max 10) (vision-range 0) (acp-to-fire 1) (range 5)  

  (point-value 3) (detonate-on-death 5) (hit-falloff-range 2)

  (ai-tactical-range 5)

  (help "A fully-operational phaser bank."))



(unit-type ptlauncher (name "Better Photon Torpedo Launcher") (image-name "ba-ray-green")

  (acp-per-turn 1)  (hp-max 10) (vision-range 0) (hit-falloff-range 3) 

  (point-value 3)

  (help "A fully-operational photon torpedo launcher."))



(unit-type ptlauncher1 (name "Photon Torpedo Launcher") (image-name "ba-star-green")

  (acp-per-turn 3) (acp-max 9)  (hp-max 10) (vision-range 0) (acp-to-fire 6) (range 6) 

  (point-value 3) (detonate-on-death 3)

  (ai-tactical-range 6)

  (help "A fully-operational photon torpedo launcher."))



(unit-type sensors (name "Sensors") (image-name "ba-sensors-green")

  (acp-independent true) (hp-max 10) (vision-range 12)  

  (point-value 3)

  (help "Ship's tactical sensors, without which it is practically blind."))



(unit-type lrsensors (name "LR Sensors") (image-name "ba-sensors-green")

  (acp-independent true) (hp-max 10) (vision-range 35)  

  (point-value 3)

  (help "Ship's long-range sensors, giving general information about distant vessels."))



(unit-type damagecontrol (name "Medical and Damage Control") (image-name "ba-cross-green")

  (acp-independent true) (hp-max 10) (vision-range 0)  

  (point-value 3)

  (help "The repair systems of a ship, to keep it and its crew spaceworthy."))



(unit-type engineering (name "Engineering Section") (image-name "ba-wrench-green")

  (acp-independent true) (hp-max 10) (vision-range 0)

  (point-value 3) (detonate-on-death 5)

  (help "The bowels of a ship, for its longterm health."))



(unit-type shield (name "Shields") (image-name "ba-oval-green")

  (acp-independent true) (hp-max 50) (vision-range 0)

  (point-value 3)

  (help "Particle and ray shielding to protect a ship from enemy fire."))



(unit-type lshield (name "Shields") (image-name "ba-oval-green")

  (acp-independent true) (hp-max 30) (vision-range 0)

  (point-value 3)

  (help "Particle and ray shielding to protect a ship from enemy fire."))



(unit-type hshield (name "Shields") (image-name "ba-oval-green")

  (acp-independent true) (hp-max 75) (vision-range 0)

  (point-value 5)

  (help "Particle and ray shielding to protect a ship from enemy fire."))



(unit-type hphaser (name "Heavy Phaser Bank") (image-name "ba-ray-green")

  (acp-per-turn 2) (hp-max 20) (vision-range 0) (acp-to-fire 1) (range 10)  

  (point-value 5) (detonate-on-death 5) (hit-falloff-range 4)

  (ai-tactical-range 10)

  (help "A fully-operational heavy phaser bank.  Longer-ranged and deadlier."))



(unit-type hdisruptor (name "Heavy Disruptor") (image-name "ba-2dash-green")

  (acp-per-turn 2) (hp-max 20) (vision-range 0) (acp-to-fire 1) (range 6)  

  (point-value 5) (detonate-on-death 5) (hit-falloff-range 4)

  (ai-tactical-range 6)

  (help "A fully-operational heavy disruptor.  Longer-ranged and deadlier."))



(unit-type constructbay (name "Construction Bay") (image-name "ba-wrench-green")

  (acp-per-turn 1) (hp-max 20) (vision-range 0)

  (point-value 0) (colonizer true)

  (help "For repairing destroyed systems and building new ships."))





;; Damaged Systems Types



(unit-type damphaser (name "Damaged Phaser Bank") (image-name "ba-ray-yellow")

  (acp-per-turn 1) (hp-max 10) (vision-range 0) (acp-to-fire 1) (range 8)  

  (point-value 2) (auto-upgrade-to phaser) (detonate-on-death 5) (hit-falloff-range 3)

  (ai-tactical-range 8)

  (help "A damaged phaser bank with reduced range, reduced power and able to get off only one shot a turn."))



(unit-type damdisruptor (name "Damaged Light Disruptor") (image-name "ba-3dash-yellow")

  (acp-per-turn 2) (hp-max 10) (vision-range 0) (acp-to-fire 1) (range 5)  

  (point-value 2) (detonate-on-death 5) (hit-falloff-range 2) (auto-upgrade-to disruptor) 

  (ai-tactical-range 5)

  (help "A damaged disruptor."))



(unit-type damptlauncher1 (name "Damaged Photon Torpedo Launcher") (image-name "ba-star-yellow")

  (acp-per-turn 2) (acp-max 9)  (hp-max 10) (vision-range 0) (acp-to-fire 6) (range 6) 

  (point-value 3) (detonate-on-death 3) (auto-upgrade-to ptlauncher1) (hit-falloff-range 3) 

  (ai-tactical-range 6)

  (help "A damaged photon torpedo launcher."))



(unit-type damsensors (name "Damaged Sensors") (image-name "ba-sensors-yellow")

  (acp-independent true) (hp-max 10) (vision-range 5)  

  (point-value 2) (auto-upgrade-to sensors)

  (help "Damaged ship's sensors, with only half the range."))



(unit-type damlrsensors (name "Damaged LR Sensors") (image-name "ba-sensors-yellow")

  (acp-independent true) (hp-max 10) (vision-range 18)  

  (point-value 2) (auto-upgrade-to lrsensors)

  (help "Damaged long-range sensors, with only half the range."))



(unit-type damdamagecontrol (name "Damaged Medical and Damage Control") (image-name "ba-cross-yellow")

  (acp-independent true) (hp-max 10) (vision-range 0)  

  (point-value 2) (auto-upgrade-to damagecontrol)

  (help "Damaged repair systems of a ship, reduced to half capability."))



(unit-type damengineering (name "Damaged Engineering Section") (image-name "ba-wrench-yellow")

  (acp-independent true) (hp-max 10) (vision-range 0)

  (point-value 2) (auto-upgrade-to engineering) (detonate-on-death 5)

  (help "Damaged reactors and overtaxed engineering crews, reduced to half capability."))





(unit-type damshield (name "Damaged Shields") (image-name "ba-oval-yellow")

  (acp-independent true) (hp-max 25) (vision-range 0)

  (point-value 2) (auto-upgrade-to shield)

  (help "Damaged particle and ray shielding, only half strength."))



(unit-type damlshield (name "Damaged Shields") (image-name "ba-oval-yellow")

  (acp-independent true) (hp-max 15) (vision-range 0)

  (point-value 2) (auto-upgrade-to lshield)

  (help "Damaged particle and ray shielding, only half strength."))



(unit-type damhshield (name "Damaged Shields") (image-name "ba-oval-yellow")

  (acp-independent true) (hp-max 40) (vision-range 0)

  (point-value 5) (auto-upgrade-to hshield)

  (help "Particle and ray shielding to protect a ship from enemy fire."))



(unit-type damhphaser (name "Damaged Heavy Phaser Bank") (image-name "ba-ray-yellow")

  (acp-per-turn 1) (hp-max 20) (vision-range 0) (acp-to-fire 1) (range 10)  

  (point-value 3) (detonate-on-death 5) (hit-falloff-range 4)

  (ai-tactical-range 10)

  (help "Damaged particle and ray shielding, only half strength."))





;; KO'd Systems Types



(unit-type kophaser (name "Knocked Out Phaser Bank") (image-name "ba-ray-red")

  (acp-independent true) (hp-max 5) (vision-range 0) 

  (point-value 1) (auto-upgrade-to damphaser) (detonate-on-death 5)

  (help "A knocked-out phaser bank.  Unable to fire but still reparable."))



(unit-type kodisruptor (name "Knocked Out Light Disruptor") (image-name "ba-3dash-red")

  (acp-independent true) (hp-max 5) (vision-range 0) 

  (point-value 1) (detonate-on-death 5) (auto-upgrade-to damdisruptor)

  (help "A knocked-out disruptor."))



(unit-type koptlauncher1 (name "Damaged Photon Torpedo Launcher") (image-name "ba-star-red")

  (acp-independent true) (hp-max 5) (vision-range 0)

  (point-value 1) (auto-upgrade-to ptlauncher1) 

  (help "A knocked-out photon torpedo launcher."))



(unit-type kosensors (name "Knocked Out Sensors") (image-name "ba-sensors-red")

  (acp-independent true) (hp-max 5) (vision-range 3)  

  (point-value 1) (auto-upgrade-to damsensors)

  (help "Knocked out ship's sensors.  Barely operational, with spotty results but still reparable."))



(unit-type kolrsensors (name "Knocked Out LR Sensors") (image-name "ba-sensors-red")

  (acp-independent true) (hp-max 5) (vision-range 0)  

  (point-value 1) (auto-upgrade-to damlrsensors)

  (help "Knocked out long-range sensors.  Not operational but reparable."))



(unit-type kodamagecontrol (name "Knocked Out Medical and Damage Control") (image-name "ba-cross-red")

  (acp-independent true) (hp-max 5) (vision-range 0)  

  (point-value 1) (auto-upgrade-to damdamagecontrol)

  (help "The repair systems of a ship, barely able to keep itself together, much less the rest of the ship."))



(unit-type koengineering (name "Knocked Out Engineering Section") (image-name "ba-wrench-red")

  (acp-independent true) (hp-max 5) (vision-range 0)

  (point-value 1) (auto-upgrade-to damengineering) (detonate-on-death 5)

  (help "The foundations of a ship reduced to utter chaos."))



(unit-type koshield (name "Knocked Out Shields") (image-name "ba-oval-red")

  (acp-independent true) (hp-max 5) (vision-range 0) 

  (point-value 1) (auto-upgrade-to damshield)

  (help "Particle and ray shielding.  Failing but still reparable."))



(unit-type kolshield (name "Knocked Out Shields") (image-name "ba-oval-red")

  (acp-independent true) (hp-max 5) (vision-range 0) 

  (point-value 1) (auto-upgrade-to damlshield)

  (help "Particle and ray shielding.  Failing but still reparable."))



(unit-type kohshield (name "Knocked Out Shields") (image-name "ba-oval-red")

  (acp-independent true) (hp-max 10) (vision-range 0) 

  (point-value 1) (auto-upgrade-to damhshield)

  (help "Particle and ray shielding.  Failing but still reparable."))



(unit-type kohphaser (name "Knocked Out Heavy Phaser Bank") (image-name "ba-ray-red")

  (acp-independent true) (hp-max 10) (vision-range 0)

  (point-value 2) (detonate-on-death 5) 

  (help "A knocked-out heavy phaser bank."))





;; Destroyed Systems Types



(unit-type desphaser (name "Destroyed Phaser Bank") (image-name "ba-ray-black")

  (acp-independent true) (hp-max 1) (vision-range 0) 

  (point-value 0)

  (help "An utterly destroyed phaser bank.  Irreparable."))



(unit-type desdisruptor (name "Destroyed Light Disruptor") (image-name "ba-3dash-black")

  (acp-independent true) (hp-max 1) (vision-range 0) 

  (point-value 0)

  (help "A destroyed disruptor."))



(unit-type desptlauncher1 (name "Destroyed Photon Torpedo Launcher") (image-name "ba-star-black")

  (acp-independent true) (hp-max 1) (vision-range 0)

  (point-value 3)

  (help "A destroyed photon torpedo launcher."))



(unit-type dessensors (name "Destroyed Sensors") (image-name "ba-sensors-black")

  (acp-independent true) (hp-max 1) (vision-range 0)  

  (point-value 0)

  (help "Destroyed ship's sensors.  Irreparable."))



(unit-type deslrsensors (name "Destroyed LR Sensors") (image-name "ba-sensors-black")

  (acp-independent true) (hp-max 1) (vision-range 0)  

  (point-value 0)

  (help "Destroyed long-range sensors.  Irreparable."))





(unit-type desdamagecontrol (name "Destroyed Medical and Damage Control") (image-name "ba-cross-black")

  (acp-independent true) (hp-max 1) (vision-range 0)  

  (point-value 0)

  (help "The destroyed repair systems of a ship.  Irreparable."))



(unit-type desengineering (name "Destroyed Engineering Section") (image-name "ba-wrench-black")

  (acp-independent true) (hp-max 1) (vision-range 0)

  (point-value 0)

  (help "Only a starbase can repair this."))



(unit-type desshield (name "Destroyed Shields") (image-name "ba-oval-black")

  (acp-independent true) (hp-max 1) (vision-range 0)

  (point-value 0)

  (help "Destroyed particle and ray shielding.  Irreparable."))



(unit-type deslshield (name "Destroyed Shields") (image-name "ba-oval-black")

  (acp-independent true) (hp-max 1) (vision-range 0)

  (point-value 0)

  (help "Destroyed particle and ray shielding.  Irreparable."))



(unit-type deshshield (name "Destroyed Shields") (image-name "ba-oval-black")

  (acp-independent true) (hp-max 1) (vision-range 0)

  (point-value 0)

  (help "Destroyed particle and ray shielding.  Irreparable."))



(unit-type deshphaser (name "Destroyed Heavy Phaser Bank") (image-name "ba-ray-black")

  (acp-independent true) (hp-max 1) (vision-range 0)

  (point-value 0) 

  (help "A destroyed heavy phaser bank."))





;; Off-line Systems Types



(unit-type offphaser (name "Offline Phaser Bank") (image-name "ba-ray-tan")

  (acp-independent true) (hp-max 4) (vision-range 0)  (char "a")

  (point-value 3) (wrecked-type kophaser)

  (help "A deactivated phaser bank.  It'll take 1-3 turns to come back on-line.  If hit, it stands a good chance of fusing and goes immediately to knocked out."))



(unit-type offdisruptor (name "Offline Light Disruptor") (image-name "ba-3dash-tan")

  (acp-independent true) (hp-max 4) (vision-range 0)  (char "b")

  (point-value 3) (wrecked-type kodisruptor)

  (help "An offline disruptor."))



(unit-type offptlauncher1 (name "Offline Photon Torpedo Launcher") (image-name "ba-star-tan")

  (acp-independent true) (hp-max 4) (vision-range 0)  (char "c")

  (point-value 3) (wrecked-type koptlauncher1)

  (help "A fully-operational photon torpedo launcher."))



(unit-type offsensors (name "Offline Sensors") (image-name "ba-sensors-tan")

  (acp-independent true) (hp-max 4) (vision-range 0) (char "d")

  (point-value 3) (wrecked-type kosensors)

  (help "Ship's tactical sensors, without which it is practically blind."))



(unit-type offlrsensors (name "Offline LR Sensors") (image-name "ba-sensors-tan")

  (acp-independent true) (hp-max 4) (vision-range 0) (char "e")

  (point-value 3) (wrecked-type kolrsensors)

  (help "Ship's long-range sensors, giving general information about distant vessels."))



(unit-type offdamagecontrol (name "Offline Medical and Damage Control") (image-name "ba-cross-tan")

  (acp-independent true) (hp-max 4) (vision-range 0) (char "f")

  (point-value 3) (wrecked-type kodamagecontrol)

  (help "The repair systems of a ship, to keep it and its crew spaceworthy."))



(unit-type offengineering (name "Offline Engineering Section") (image-name "ba-wrench-tan")

  (acp-independent true) (hp-max 4) (vision-range 0) (char "g")

  (point-value 3) (wrecked-type koengineering)

  (help "The bowels of a ship, for its longterm health."))



(unit-type offshield (name "Offline Shields") (image-name "ba-oval-tan")

  (acp-independent true) (hp-max 4) (vision-range 0) (char "h")

  (point-value 3) (wrecked-type koshield)

  (help "Deactivated shielding.  It'll take 1-5 turns to come back on-line.  If hit, it stands a good chance of fusing and goes immediately to knocked out.."))



(unit-type offlshield (name "Offline Shields") (image-name "ba-oval-tan")

  (acp-independent true) (hp-max 4) (vision-range 0) (char "i")

  (point-value 3) (wrecked-type kolshield)

  (help "Deactivated shielding.  It'll take 1-5 turns to come back on-line.  If hit, it stands a good chance of fusing and goes immediately to knocked out.."))



(unit-type offhshield (name "Offline Shields") (image-name "ba-oval-tan")

  (acp-independent true) (hp-max 4) (vision-range 0) (char "j")

  (point-value 3) (wrecked-type kohshield)

  (help "Deactivated shielding.  It'll take 1-5 turns to come back on-line.  If hit, it stands a good chance of fusing and goes immediately to knocked out.."))



(unit-type offhphaser (name "Offline Heavy Phaser Bank") (image-name "ba-ray-tan")

  (acp-independent true) (hp-max 8) (vision-range 0) (char "k")

  (point-value 0) 

  (help "An offline heavy phaser bank."))



;; Hulks



(unit-type p-exhulk (name "Excelsior Class Hulk (H)")(image-name "kb-st-smold-excelsior")

  (acp-independent true) (hp-max 1) (vision-range 0)

  (point-value 0)

  (help "A destroyed Excelsior class battlecruiser.  Considered hot and may explode at any moment."))



(unit-type p-enthulk (name "Constitution Class Hulk (H)") (image-name "kb-st-smold-enterprise")

  (acp-independent true) (hp-max 1) (vision-range 0)

  (point-value 0) (hp-per-detonation 100)

  (help "A destroyed Constitution class cruiser.  Considered hot and may explode at any moment."))



(unit-type p-relhulk (name "Miranda Class Hulk (H)") (image-name "kb-st-smold-reliant")

  (acp-independent true)  (hp-max 1) (vision-range 0)

  (point-value 0) (hp-per-detonation 100)

  (help "A destroyed Miranda class light cruiser.  Considered hot and may explode at any moment."))



(unit-type p-fshulk1 (name "Smashed Outpost (H)") (image-name "kb-st-smold-ds9")

  (acp-independent true)  (hp-max 1) (vision-range 0)

  (point-value 0)  (hp-per-detonation 100)

  (help "A destroyed Federation outpost.  Considered hot and may explode at any moment."))



(unit-type p-rshulk1 (name "Smashed Research Station (H)") (image-name "kb-st-smold-research")

  (acp-independent true)  (hp-max 1) (vision-range 0)

  (point-value 0)  (hp-per-detonation 100)

  (help "A destroyed Federation research station.  Considered hot and may explode at any moment."))



(unit-type p-obhulk (name "Oberth Class Hulk (H)") (image-name "kb-st-smold-oberth")

  (acp-per-turn 0) (hp-max 1) (vision-range 0)

  (point-value 0) (hp-per-detonation 100)

  (help "An Oberth class research vessel.  Lightly shielded and poorly armed, but equipped with powerful sensors."))



(unit-type p-kthulk (name "K'Tinga Class Hulk (H)") (image-name "kb-kling-smold-ktinga")

  (acp-independent true) (hp-max 1) (vision-range 0)

  (point-value 0) (hp-per-detonation 100)

  (help "A destroyed K'Tinga class heavy cruiser.  Considered hot and may explode at any moment."))



(unit-type p-d7hulk (name "D7 Class Hulk (H)") (image-name "kb-kling-smold-d7")

  (acp-independent true) (hp-max 60) (vision-range 0)

  (point-value 0) (hp-per-detonation 100)

  (help "A destroyed D7 class cruiser.  Considered hot and may explode at any moment."))



(unit-type p-bophulk (name "Bird of Prey Class Hulk (H)") (image-name "kb-kling-smold-bop")

  (acp-independent true) (hp-max 1) (vision-range 0)

  (point-value 0) (hp-per-detonation 100)

  (help "A destroyed Bird of Prey class patrol ship.  Cold, stable  and salvagable."))



(unit-type exhulk (name "Excelsior Class Hulk (C)")(image-name "kb-st-wreck-excelsior")

  (acp-independent true) (hp-max 1) (vision-range 0)

  (point-value 0)

  (help "A destroyed Excelsior class battlecruiser.  Cold, stable  and salvagable."))



(unit-type enthulk (name "Constitution Class Hulk (C)")(image-name ("kb-st-wreck-enterprise" 

   "kb-st-wreck-enterprise1" "kb-st-wreck-enterprise2"))

  (acp-independent true) (hp-max 1) (vision-range 0)

  (point-value 0)

  (help "A destroyed Constitution class cruiser.  Cold, stable  and salvagable."))



(unit-type relhulk (name "Miranda Class Hulk (C)") (image-name ("kb-st-wreck-reliant" 

   "kb-st-wreck-reliant1" "kb-st-wreck-reliant2"))

  (acp-independent true) (hp-max 1) (vision-range 0)

  (point-value 0)

  (help "A destroyed Miranda class light cruiser.  Cold, stable  and salvagable."))



(unit-type obhulk (name "Oberth Class Hulk (C)") (image-name "kb-st-wreck-oberth")

  (acp-per-turn 0) (hp-max 1) (vision-range 0)

  (point-value 0)

  (help "An Oberth class research vessel.  Lightly shielded and poorly armed, but equipped with powerful sensors."))



(unit-type fshulk1 (name "Smashed Outpost (C)") (image-name "kb-st-wreck-ds9")

  (acp-independent true)  (hp-max 1) (vision-range 0)

  (point-value 0)

  (help "A destroyed Federation outpost.  Cold, stable  and salvagable."))



(unit-type rshulk1 (name "Smashed Research Station (C)") (image-name "kb-st-wreck-research")

  (acp-independent true)  (hp-max 1) (vision-range 0)

  (point-value 0)

  (help "A destroyed Federation research station.  Cold, stable  and salvagable."))



(unit-type kthulk (name "K'Tinga Class Hulk (C)") (image-name "kb-kling-wreck-ktinga")

  (acp-independent true) (hp-max 1) (vision-range 0)

  (point-value 0)

  (help "A destroyed K'Tinga class heavy cruiser.  Cold, stable  and salvagable."))



(unit-type d7hulk (name "D7 Class Hulk (C)") (image-name "kb-kling-wreck-d7")

  (acp-independent true) (hp-max 1) (vision-range 0)

  (point-value 0)

  (help "A destroyed D7 class cruiser.  Cold, stable  and salvagable."))



(unit-type bophulk (name "Bird of Prey Class Hulk (C)") (image-name "kb-kling-wreck-bop")

  (acp-independent true) (hp-max 1) (vision-range 0)

  (point-value 0)

  (help "A destroyed Bird of Prey class patrol ship.  Cold, stable  and salvagable."))





;; HULLS



(unit-type exhull (name "Excelsior Class Hull")(image-name "kb-st-ship-excelsior")

  (acp-per-turn 6) (hp-max 150) (vision-range 0) (acp-to-fire 6) (range 7)

  (point-value 20) (auto-upgrade-to excelsior)

  (help "A newly built Excelsior Class battlecruiser, about to bring its systems online."))



(unit-type enthull (name "Constitution Class Hull")(image-name "kb-st-ship-enterprise")

  (acp-per-turn 8) (hp-max 100) (vision-range 0) (acp-to-fire 8) (range 7)

  (point-value 12) (auto-upgrade-to enterprise)

  (help "A newly built Constitution Class cruiser, about to bring its systems online."))



(unit-type relhull (name "Miranda Class Hull") (image-name "kb-st-ship-reliant")

  (acp-per-turn 10) (hp-max 70) (vision-range 0) (acp-to-fire 10) (range 7)

  (point-value 8) (auto-upgrade-to reliant)

  (help "A newly built Miranda Class light cruiser, about to bring its systems online."))



(unit-type obhull (name "Oberth Class Hull") (image-name "kb-st-ship-oberth")

  (acp-per-turn 8) (hp-max 30) (vision-range 0) (acp-to-fire 8) (range 7)

  (point-value 6) (auto-upgrade-to oberth)

  (help "A newly built Oberth class research vessel, about to bring its systems online."))





;; Strange Stuff



(unit-type fedcpc (name "Fed Command Prefix Code") (image-name "ba-commands-tan")

  (acp-per-turn 3) (acp-max 9) (hp-max 3) (vision-range 0) (acp-to-fire 4) (range 3)  

  (point-value 0)

  (help "The code necessary to remotely command a Federation ship to take up to three systems off-line."))



(unit-type genesis (name "Genesis Device") (image-name "ba-commands-tan")

  (acp-per-turn 1) (acp-max 1) (hp-max 1) (vision-range 0) (acp-to-detonate 1) 

  (point-value 0) (hp-per-detonation 100)

  (help "Capable of producing a Genesis Wave, which uses unstable protomatter to create a habitable planet."))



(define ship-types (enterprise reliant d7 ktinga fedstation1 excelsior oberth 
    research bop shipyard))



(add ship-types naval true)



(define cloaked-ship-types (cloaked-d7 cloaked-ktinga cloaked-bop))



(define hulk-types (enthulk relhulk fshulk1 exhulk obhulk d7hulk kthulk bophulk))



(define hull-types (enthull relhull exhull obhull))



(define p-hulk-types (p-enthulk p-relhulk p-fshulk1 p-exhulk p-obhulk p-d7hulk p-kthulk p-bophulk))



(define operational-systems-types (phaser sensors damagecontrol engineering shield lrsensors

   ptlauncher ptlauncher1 disruptor lshield hshield hphaser constructbay))



(define damaged-systems-types (damphaser damsensors damdamagecontrol damengineering damshield

   damlrsensors damptlauncher1 damdisruptor damlshield damhshield damhphaser))



(define ko-systems-types (kophaser kosensors kodamagecontrol koengineering koshield

   kolrsensors koptlauncher1 kodisruptor kolshield kohshield kohphaser))



(define destroyed-systems-types (desphaser dessensors desdamagecontrol desengineering desshield

  deslrsensors desptlauncher1 desdisruptor deslshield deshshield deshphaser))



(define offline-systems-types (offphaser offsensors offdamagecontrol offengineering offshield

  offlrsensors offptlauncher1 offdisruptor offlshield offhshield offhphaser))





(define all-systems-types (append operational-systems-types damaged-systems-types ko-systems-types destroyed-systems-types offline-systems-types))



(define ranged-weapon-types (phaser damphaser disruptor damdisruptor hphaser damhphaser))



(define shield-types (shield damshield lshield damlshield hshield damhshield))


;; TRICKY


;;(table attack-range
;;   (ship-types u* 7)
;;)

;;(table attack-range-min
;;   (ship-types u* 3)
;;)

;;(add ship-types range 7)
;;(add ship-types range-min 3)


;; MATERIAL



(material-type repair (name "Repair Points") (image-name "ba-cross-green")

  (help "Represents the ability to return a system to operability."))



(material-type torps (name "Photon Torpedoes") (image-name "ba-cross-red")

  (help "Represents the torpedoes stored in a ship's magazine."))



(material-type si (name "Shield Integrity") (image-name "ba-cross-yellow")

  (help "Represents a shield's current integrity."))



(material-type online (name "Online") (image-name "ba-cross-yellow")

  (help "Represents an offline system's turns remaining before coming online."))



(material-type construction (name "Construction Points") (image-name "ba-cross-black")

  (help "Represents a stations ability to rebuild destroyed systems or build a new hull."))



(material-type cloakable (name "Cloaking Device") (image-name "ba-cross-black")

  (help "Represents a cloaking device's readiness."))





;; SYSTEM REPAIR



(table unit-storage-x

	(damaged-systems-types repair 10)

	(ko-systems-types repair 10)

	(damagecontrol repair 10)



	(ptlauncher torps 1)

	(pt torps 1)

	(excelsior torps 80)

	(enterprise torps 40)

	(reliant torps 20)

	(ktinga torps 20)

	(d7 torps 20)

	(cloaked-ktinga torps 20)

	(cloaked-d7 torps 20)



	((d7 ktinga bop) cloakable 100)	



	(fedstation1 torps 500)
	(shipyard torps 500)

	(constructbay construction 300)
	(shipyard construction 300)



	(research torps 75)



	(ptlauncher1 torps 3)

	(damptlauncher1 torps 3)



	(offline-systems-types online 1)



;;	(ship-types si 100)

;;	(shield si 6)

;;	(damshield si 6)



	(fedcpc online 1)



	(p-hulk-types online 1)



	(oberth online 2)



)



(table supply-on-creation

	(pt torps 1)

	(excelsior torps 80)

	(enterprise torps 40)

	(reliant torps 20)



	(offline-systems-types online 0)



;;	(ship-types si 100)



;;	(all-systems-types si 10)



	(fedcpc online 0)



	(fedstation1 torps 500)
	(shipyard torps 500)

	(constructbay construction 50)
	(shipyard construction 50)

	(research torps 75)



	(oberth online 2)

	

)



(table unit-initial-supply

	(pt torps 1)



	(excelsior torps 80)

	(enterprise torps 40)

	(reliant torps 20)



	(ktinga torps 20)

	(d7 torps 20)

	(cloaked-ktinga torps 20)

	(cloaked-d7 torps 20)



	(offline-systems-types online 0)

	

;;	(ship-types si 100)



;;	(all-systems-types si 10)



	(fedcpc online 0)



	(fedstation1 torps 500)
	(shipyard torps 500)

	(constructbay construction 50)
	(shipyard construction 50)

	(research torps 75)



	(oberth online 2)



)





(table base-production

	(damagecontrol repair 5)

	(damdamagecontrol repair 3)

	(kodamagecontrol repair 2)



;;	(ship-types si 5)



	(fedcpc online 0)



	(oberth online 2)



	(constructbay construction 10)
	(shipyard construction 10)



	((ktinga d7) cloakable 34)

	(bop cloakable 50)



)



(table base-consumption



;;	(shield si 6)

;;	(damshield si 6)



	(offline-systems-types online 1)



	(fedcpc online 1)



	(p-hulk-types online 1)

)



(table hp-per-starve



;; Generally takes 3 turns to bring systems online, sometimes less, sometimes more



	(offline-systems-types online 150)

	(fedcpc online 100)



	(p-hulk-types online 25)

)	





(table wrecked-type-if-starved



	(offshield online shield)

	(offphaser online phaser)

	(offptlauncher1 online ptlauncher1)

	(offsensors online sensors)

	(offlrsensors online lrsensors)

	(offengineering online engineering)

	(offdamagecontrol online damagecontrol)

	(offdisruptor online disruptor)

	(offlshield online lshield)

	(offhphaser online hphaser)



	(p-relhulk online relhulk)

	(p-enthulk online enthulk)

	(p-exhulk online exhulk)

	(p-fshulk1 online fshulk1)

	(p-d7hulk online d7hulk)

	(p-kthulk online kthulk)

	(p-obhulk online obhulk)

	(p-rshulk1 online rshulk1)





)	







(table out-length

  (u* repair -1)

  (damagecontrol repair 0)

  (damdamagecontrol repair 0)



  (ship-types torps 0)

;;  (ship-types si 0)



  (cloaked-ship-types torps 0)



  (fedstation1 torps 1)
  (shipyard torps 1)

  (research torps 1)



)



(table in-length

  (ptlauncher torps 1)

  (ptlauncher1 torps 1)

  (damptlauncher1 torps 1)



  (all-systems-types si 1)



  (ship-types torps 0)

  (cloaked-ship-types torps 0)



)



(table material-to-change-type

	(damaged-systems-types repair 10)

	(ko-systems-types repair 10)

	((d7 ktinga bop) cloakable 100)

)



(table occupants-to-change-type

	(ship-types engineering 1)

)

(table complete-occs-on-completion
    (hull-types engineering 100)
    (hull-types damagecontrol 100)
    (hull-types sensors 100)

    (exhull hphaser 200)
    (exhull hshield 100)
    (exhull ptlauncher1 200)

    (obhull phaser 100)
    (obhull lshield 100)
    (obhull lrsensors 100)

    (relhull phaser 100)
    (relhull shield 100)
    (relhull ptlauncher1 100)

    (enthull phaser 200)
    (enthull shield 100)
    (enthull ptlauncher1 200)

)

	



;; TERRAIN





(terrain-type deepspace (image-name "ba-space")

  (help "Deep space"))



(terrain-type nebula (image-name "ba-neb")

  (help "Static discharge causes shields to fail."))



(terrain-type corona (image-name "yellow")

  (help "Heat and gravitic stress will damage a hull."))



(terrain-type star (image-name "orange")

  (help "Nothing can survive the this terrain."))



(terrain-type inhospitable (image-name "gray")

  (help "A planetoid or asteroid without the capability of sustaining life."))



(terrain-type hospitable (image-name "green")

  (help "A planetoid or asteroid with the capability of sustaining life."))





(add deepspace liquid true)



;;; Unit-unit capacities.





(define non-combat-ship-types (oberth))

(define patrol-ship-types (bop))

(define light-cruiser-types (reliant d7 cloaked-d7))

(define heavy-cruiser-types (enterprise ktinga cloaked-ktinga))

(define battle-cruiser-types (excelsior))

(define station-types (shipyard fedstation1 research))





(add non-combat-ship-types capacity 13)

(add patrol-ship-types capacity 13)

(add light-cruiser-types capacity 15)

(add heavy-cruiser-types capacity 17)

(add battle-cruiser-types capacity 17)

(add station-types capacity 17)

(add shipyard capacity 0)



(add hulk-types capacity 17)

(add p-hulk-types capacity 17)



(add hull-types capacity 17)



(table unit-size-as-occupant

  ;; Disable occupancy by default.

	(u* u* 99)

	(all-systems-types ship-types 2)

	(all-systems-types cloaked-ship-types 2)

	(all-systems-types station-types 2)

	(all-systems-types hulk-types 2)

	(all-systems-types p-hulk-types 2)

	(all-systems-types hull-types 2)

	(fedcpc ship-types 99)

	(fedcpc oberth 1)

	(genesis ship-types 1)

	(genesis cloaked-ship-types 1)



)



(table occupant-max



	(u* u* 0)	



	(hulk-types all-systems-types 99)	

	(p-hulk-types all-systems-types 99)	



	(u* damaged-systems-types 99)	

	(u* ko-systems-types 99)	

	(u* destroyed-systems-types 99)	

	(u* offline-systems-types 99)	



	(ship-types engineering 1)

	(ship-types damagecontrol 1)

	(ship-types sensors 1)



	(hull-types u* 10)



	(cloaked-ship-types engineering 1)

	(cloaked-ship-types damagecontrol 1)

	(cloaked-ship-types sensors 1)



	(station-types engineering 0)

	(station-types damagecontrol 1)

	(station-types shield 1)

	(station-types sensors 1)

	(station-types lrsensors 1)



	(fedstation1 hphaser 2)

	(fedstation1 phaser 2)

	(fedstation1 shield 0)

	(fedstation1 hshield 1)



	(research phaser 2)

	(research constructbay 1)

	(shipyard phaser 2)


	(enterprise phaser 2)

	(enterprise ptlauncher1 2)

	(enterprise shield 1)

	

	(reliant phaser 1)

	(reliant ptlauncher1 1)

	(reliant shield 1)



	(excelsior ptlauncher1 2)

	(excelsior hphaser 2)

	(excelsior hshield 1)



	(oberth phaser 1)

	(oberth lrsensors 1)

	(oberth lshield 1)

	(oberth fedcpc 1)



	(d7 disruptor 2)

	(d7 ptlauncher1 1)

	(d7 lshield 1)



	(cloaked-d7 disruptor 2)

	(cloaked-d7 ptlauncher1 1)

	(cloaked-d7 lshield 1)



	(bop disruptor 1)

	(bop ptlauncher1 1)

	(bop lshield 1)



	(cloaked-bop disruptor 1)

	(cloaked-bop ptlauncher1 1)

	(cloaked-bop lshield 1)



	(hulk-types damagecontrol 0)

	(hulk-types damdamagecontrol 0)

	(hulk-types kodamagecontrol 0)



	(p-hulk-types damagecontrol 0)

	(p-hulk-types damdamagecontrol 0)

	(p-hulk-types kodamagecontrol 0)



	(ship-types genesis 1)

	(cloaked-ship-types genesis 1)

	

)



;;; Unit-terrain capacities.



(table unit-size-in-terrain

  (u* t* 100)

)



(add t* capacity 100)



(add inhospitable capacity 1)



;;; Movement.



(table mp-to-enter-terrain

  (u* t* 2)

  (hull-types t* 99)

  (pt deepspace 1)

  (u* inhospitable 99)

  (u* hospitable 99)

)



;; (add all-systems-types speed 0)





(table mp-to-leave-terrain

)



(add ship-types hp-recovery 0)

(add hulk-types hp-recovery 0)

(add p-hulk-types hp-recovery 0)

(add cloaked-ship-types hp-recovery 0)

(add all-systems-types hp-recovery 125)

(add offline-systems-types hp-recovery 0)

(add shield hp-recovery 1000)

(add damshield hp-recovery 500)

(add lshield hp-recovery 600)

(add damlshield hp-recovery 300)

(add hshield hp-recovery 1500)

(add damhshield hp-recovery 750)



(add victorypoint hp-recovery 100)





;; VISION



(table see-chance

   (damsensors u* 75)

   (kosensors u* 50)



   (u* cloaked-ship-types 5)

   (lrsensors cloaked-ship-types 35)

   (sensors cloaked-ship-types 10)



)



(table see-chance-adjacent

   (u* cloaked-ship-types 25)

   (lrsensors cloaked-ship-types 50)

   (sensors cloaked-ship-types 30)



)





(table see-mistake-chance

   (lrsensors ship-types 10000)

   (damlrsensors ship-types 10000)



   (lrsensors hulk-types 10000)

   (damlrsensors hulk-types 10000)



   (lrsensors p-hulk-types 10000)

   (damlrsensors p-hulk-types 10000)

)



(table looks-like

   (u* u* 0)

   (ship-types fedmarker 10000)



   (hulk-types fedmarker-1 10000)



   (p-hulk-types fedmarker-2 10000)

)



(table visibility

   (sensors nebula 0)

   (damsensors nebula 0)

   (kosensors nebula 0)

   (lrsensors nebula 0)

   (damlrsensors nebula 0)

   (kolrsensors nebula 0)

   (ship-types nebula 30)

)





;; OFFLINE SYSTEM ACTIVATION



(table accident-hit-chance

;;	(offline-systems-types t* 2500)

	(shield nebula 10000)

	(damshield nebula 10000)

)



(table wrecks-on

	(ship-types hospitable true)

	(cloaked-ship-types hospitable true)

)



(table accident-damage

;;        (offline-systems-types t* 10)

	(shield nebula 100)

	(damshield nebula 100)

)



;; CLOAKING



(table can-change-type-to

	(ktinga cloaked-ktinga true)

	(cloaked-ktinga ktinga true)

	(d7 cloaked-d7 true)

	(cloaked-d7 d7 true)

	(bop cloaked-bop true)

	(cloaked-bop bop true)

)



(table acp-to-change-type

	(ktinga cloaked-ktinga 6)

	(cloaked-ktinga ktinga 1)

	(d7 cloaked-d7 6)

	(cloaked-d7 d7 1)

	(bop cloaked-bop 6)

	(cloaked-bop bop 1)

)





(set combat-model 0)





(table acp-to-attack

  (u* u* 99)



;; Ships can perform boarding actions



  (ship-types ship-types 6)

  (ship-types hulk-types 6)

  (ship-types victorypoint 5)



  (pt ship-types 1)

)



(table acp-to-defend

   (u* u* 99)

)







(table fire-hit-chance

	(u* u* 0)



	(phaser u* 90)

	(damphaser u* 90)



	(hphaser u* 80)

	(damhphaser u* 80)



	(disruptor u* 90)

	(damdisruptor u* 90)



	(ptlauncher1 u* 90)

	(damptlauncher1 u* 90)



	(ship-types ship-types 75)

	(u* victorypoint 0)

	(ship-types victorypoint 100)



	(fedcpc u* 100)
	
	(hull-types u* 100)
)





(table fire-damage

	(ship-types ship-types 1)	



	(ship-types victorypoint 2)

	(battle-cruiser-types victorypoint 4)

	(heavy-cruiser-types victorypoint 3)



	(phaser ship-types 2d10)

	(phaser cloaked-ship-types 2d10)

	(phaser all-systems-types 2d6)

	(phaser shield-types 2d10)

	(phaser offline-systems-types 1d10)



	(damphaser ship-types 2d10)

	(damphaser cloaked-ship-types 2d10)

	(damphaser all-systems-types 2d6)

	(damphaser shield-types 2d10)

	(damphaser offline-systems-types 1d10)



	(hphaser ship-types 3d10)

	(hphaser cloaked-ship-types 3d10)

	(hphaser all-systems-types 3d6)

	(hphaser shield-types 3d10)

	(hphaser offline-systems-types 2d10)



	(damhphaser ship-types 3d10)

	(damhphaser cloaked-ship-types 3d10)

	(damhphaser all-systems-types 3d6)

	(damhphaser shield-types 3d10)

	(damhphaser offline-systems-types 2d10)



	(disruptor ship-types 2d6)

	(disruptor cloaked-ship-types 2d6)

	(disruptor all-systems-types 2d4)

	(disruptor shield-types 2d6)

	(disruptor offline-systems-types 1d8)



	(damdisruptor ship-types 2d6)

	(damdisruptor cloaked-ship-types 2d6)

	(damdisruptor all-systems-types 2d4)

	(damdisruptor shield-types 2d6)

	(damdisruptor offline-systems-types 1d8)



	(ptlauncher1 ship-types 3d8+15)

	(ptlauncher1 cloaked-ship-types 3d8+15)

	(ptlauncher1 all-systems-types 2d10+5)

	(ptlauncher1 shield-types 3d8+5)

	(ptlauncher1 offline-systems-types 1d8+2)



	(damptlauncher1 ship-types 3d8+15)

	(damptlauncher1 cloaked-ship-types 3d8+15)

	(damptlauncher1 all-systems-types 2d10+5)

	(damptlauncher1 shield-types 3d8+5)

	(damptlauncher1 offline-systems-types 1d8+2)



	(fedcpc ship-types 1)

	(fedcpc operational-systems-types 100)



	(u* hulk-types 0)

	(u* p-hulk-types 0)
	
;; HULL DAMAGE FOR AI WEIGHT

    (exhull u* 200)
    (enthull u* 100)
    (relhull u* 50)
    (obhull u* 25)

)



(table hit-at-max-range-effect

	(phaser u* 75)

	(damphaser u* 75)



	(hphaser u* 75)

	(damhphaser u* 75)



	(disruptor u* 66)

	(damdisruptor u* 66)



	(ptlauncher1 u* 55)

	(damptlauncher1 u* 55)

)





(table hit-chance

	(u* u* 0)

	(ship-types ship-types 50)

	(ship-types victorypoint 100)

	(ship-types hulk-types 100)

	(pt ship-types 50)

	(pt all-systems-types 50)



)







(table damage

	(u* u* 0)

	(ship-types ship-types 1d13)



	(pt ship-types 3d8+8)

	(pt all-systems-types 2d10+1)



	(ship-types victorypoint 2)

	(battle-cruiser-types victorypoint 4)

	(heavy-cruiser-types victorypoint 3)

)



;; CONSTRUCTION



(table acp-to-create

	(shipyard (relhull exhull enthull) 1)

)

(table can-create
	(shipyard (relhull exhull enthull) 1)

)



(table cp-on-creation

	(shipyard (relhull exhull enthull) 1)

)



(table acp-to-build

	(shipyard (relhull exhull enthull) 1)

)

(table can-build
	(shipyard (relhull exhull enthull) 1)

)


(table cp-per-build

	(shipyard (relhull exhull enthull) 1)

)



(table create-range

	(shipyard (relhull exhull enthull) 1)

)



(table build-range

	(shipyard (relhull exhull enthull) 1)

)



(table consumption-on-creation

;;    (u* construction 9999)

    (relhull construction 50)

    (obhull construction 50)

    (enthull construction 100)

    (exhull construction 200)
)



(add u* cp 1)



(table detonation-accident-chance

	(pt t* 10000)



	(p-hulk-types t* 2500)

)



(table detonate-on-approach-range

	(pt u* 1)

)





(table accident-damage

   (pt t* 1)

)



(table material-to-move

   (pt torps 1)

)



(table material-to-attack

   (pt torps 1)

)



(table consumption-per-attack

   (pt torps 1)

)



(table consumption-per-fire

   (ptlauncher1 torps 1)

   (damptlauncher1 torps 1)

)





(table hit-by

   (u* torps true)

)







;; CAPTURE



(table capture-chance

	(ship-types ship-types 20)



	(non-combat-ship-types patrol-ship-types 0)

	(non-combat-ship-types light-cruiser-types 0)

	(non-combat-ship-types heavy-cruiser-types 0)

	(non-combat-ship-types battle-cruiser-types 0)

	(non-combat-ship-types station-types 0)



	(patrol-ship-types light-cruiser-types 10)

	(patrol-ship-types heavy-cruiser-types 0)

	(patrol-ship-types battle-cruiser-types 0)

	(patrol-ship-types station-types 0)



	(light-cruiser-types heavy-cruiser-types 0)

	(light-cruiser-types battle-cruiser-types 0)

	(light-cruiser-types fedstation1 0)

	(heavy-cruiser-types battle-cruiser-types 0)

	(heavy-cruiser-types fedstation1 5)

	(battle-cruiser-types fedstation1 10)



	(ship-types hulk-types 100)

	(ship-types all-systems-types 100)

	(ship-types victorypoint 100)



)





(table occupant-allows-capture-of

	(shield ship-types 0)

	(damshield ship-types 0)

	(lshield ship-types 0)

	(damlshield ship-types 0)



)



(table scuttle-chance

	(u* u* 0)

)





(table ferry-on-entry

	(u* u* 0)

;;	(people-types item-types over-all)

)



(table ferry-on-departure

	(u* u* 0)

;;	(people-types item-types over-all)

)



(table mp-to-leave-unit

	(all-systems-types ship-types 99)

	(all-systems-types station-types 99)

	(fedcpc ship-types 99)



	(all-systems-types hulk-types 99)

	(all-systems-types p-hulk-types 99)

	(all-systems-types cloaked-ship-types 99)



)



(table hp-min

  ;; Destroyed systems take up space

  (u* destroyed-systems-types 1)



  )



(table protection



  (ship-types operational-systems-types 12)

  (ship-types damaged-systems-types 12)

  (ship-types offline-systems-types 15)

  (ship-types ko-systems-types 6)

  (ship-types destroyed-systems-types 0)

  (ship-types fedcpc 0)



  (battle-cruiser-types operational-systems-types 10)

  (battle-cruiser-types damaged-systems-types 10)

  (battle-cruiser-types offline-systems-types 12)

  (battle-cruiser-types ko-systems-types 5)

  (battle-cruiser-types destroyed-systems-types 0)

  (battle-cruiser-types fedcpc 0)



  (station-types operational-systems-types 10)

  (station-types damaged-systems-types 10)

  (station-types offline-systems-types 12)

  (station-types ko-systems-types 5)

  (station-types destroyed-systems-types 0)

  (station-types fedcpc 0)



  (cloaked-ship-types operational-systems-types 12)

  (cloaked-ship-types damaged-systems-types 12)

  (cloaked-ship-types offline-systems-types 9)

  (cloaked-ship-types ko-systems-types 6)

  (cloaked-ship-types destroyed-systems-types 0)



  (hulk-types all-systems-types 0)



  (p-hulk-types all-systems-types 0)



  (all-systems-types hulk-types 100)



  (all-systems-types p-hulk-types 100)



  (ship-types shield 80)

  (ship-types damshield 65)



  (station-types shield 80)

  (station-types damshield 65)



  (station-types hshield 90)

  (station-types damhshield 75)



  (ship-types lshield 80)

  (ship-types damlshield 65)



  (ship-types hshield 90)

  (ship-types damhshield 75)



  (cloaked-ship-types shield 0)

  (cloaked-ship-types damshield 0)



  (cloaked-ship-types lshield 0)

  (cloaked-ship-types damlshield 0)



  (cloaked-ship-types hshield 0)

  (cloaked-ship-types damhshield 0)



  (all-systems-types shield 80)

  (all-systems-types damshield 65)



  (all-systems-types lshield 80)

  (all-systems-types damlshield 65)



  (all-systems-types hshield 80)

  (all-systems-types damhshield 65)



  (operational-systems-types ship-types 88)

  (damaged-systems-types ship-types 88)

  (ko-systems-types ship-types 94)

  (offline-systems-types ship-types 85)

  (destroyed-systems-types ship-types 100)



  (operational-systems-types battle-cruiser-types 90)

  (damaged-systems-types ship-types 90)

  (ko-systems-types ship-types 95)

  (offline-systems-types ship-types 88)

  (destroyed-systems-types ship-types 100)



  (operational-systems-types station-types 90)

  (damaged-systems-types station-types 90)

  (ko-systems-types station-types 95)

  (offline-systems-types station-types 88)

  (destroyed-systems-types station-types 100)



  (shield ship-types 0)

  (damshield ship-types 0)



  (lshield ship-types 0)

  (damlshield ship-types 0)



  (hshield ship-types 0)

  (damhshield ship-types 0)



  (shield station-types 0)

  (damshield station-types 0)



  (hshield station-types 0)

  (damhshield station-types 0)



  (shield all-systems-types 0)

  (damshield all-systems-types 0)



;;  (shield destroyed-systems-types 100)

;;  (damshield destroyed-systems-types 100)

)







(table occupant-combat

	(u* u* 0)

	(all-systems-types ship-types 100)

	(all-systems-types station-types 100)

	(fedcpc ship-types 100)

)

(table vanishes-on
    (all-systems-types t* true)
)



(table occupant-vision

	(u* u* 0)

	(all-systems-types ship-types 100)

	(all-systems-types station-types 100)



	(all-systems-types cloaked-ship-types 75)



	(all-systems-types p-hulk-types 0)

	(all-systems-types hulk-types 0)



)





(table occupant-escape-chance

	(u* u* 0)

)



;; ENGINES





(table occupant-adds-acp

	(battle-cruiser-types engineering 6)

	(battle-cruiser-types damengineering 4)

	(battle-cruiser-types koengineering 2)



	(heavy-cruiser-types engineering 8)

	(heavy-cruiser-types damengineering 6)

	(heavy-cruiser-types koengineering 2)



	(non-combat-ship-types engineering 8)

	(non-combat-ship-types damengineering 6)

	(non-combat-ship-types koengineering 2)



	(light-cruiser-types engineering 10)

	(light-cruiser-types damengineering 8)

	(light-cruiser-types koengineering 2)



	(patrol-ship-types engineering 12)

	(patrol-ship-types damengineering 10)

	(patrol-ship-types koengineering 2)



;; The D7 is outdated



	(d7 engineering 8)

	(d7 damengineering 6)

	(d7 koengineering 2)



;; Cloaking takes juice



	(cloaked-d7 engineering 6)

	(cloaked-d7 damengineering 4)

	(cloaked-d7 koengineering 2)



	(cloaked-ktinga engineering 6)

	(cloaked-ktinga damengineering 4)

	(cloaked-ktinga koengineering 2)



	(cloaked-bop engineering 6)

	(cloaked-bop damengineering 4)

	(cloaked-bop koengineering 2)



)





(table wrecked-type-if-killed

	(phaser fedcpc offphaser)

	(sensors fedcpc offsensors)

	(shield fedcpc offshield)

	(damagecontrol fedcpc offdamagecontrol)

	(engineering fedcpc offengineering)

	(lrsensors fedcpc offlrsensors)

	(ptlauncher1 fedcpc offptlauncher1)

	(hphaser fedcpc offhphaser)

	(lshield fedcpc offlshield)

	(hshield fedcpc offhshield)



)



(add phaser wrecked-type damphaser)

(add damphaser wrecked-type kophaser)

(add kophaser wrecked-type desphaser)

(add offphaser wrecked-type kophaser)



(add disruptor wrecked-type damdisruptor)

(add damdisruptor wrecked-type kodisruptor)

(add kodisruptor wrecked-type desdisruptor)

(add offdisruptor wrecked-type kodisruptor)



(add sensors wrecked-type damsensors)

(add damsensors wrecked-type kosensors)

(add kosensors wrecked-type dessensors)

(add offsensors wrecked-type kosensors)



(add shield wrecked-type damshield)

(add damshield wrecked-type koshield)

(add koshield wrecked-type desshield)

(add offshield wrecked-type koshield)



(add lshield wrecked-type damlshield)

(add damlshield wrecked-type kolshield)

(add kolshield wrecked-type deslshield)

(add offlshield wrecked-type kolshield)



(add damagecontrol wrecked-type damdamagecontrol)

(add damdamagecontrol wrecked-type kodamagecontrol)

(add kodamagecontrol wrecked-type desdamagecontrol)

(add offdamagecontrol wrecked-type kodamagecontrol)



(add engineering wrecked-type damengineering)

(add damengineering wrecked-type koengineering)

(add koengineering wrecked-type desengineering)

(add offengineering wrecked-type koengineering)



(add lrsensors wrecked-type damlrsensors)

(add damlrsensors wrecked-type kolrsensors)

(add kolrsensors wrecked-type deslrsensors)

(add offlrsensors wrecked-type kolrsensors)



(add ptlauncher1 wrecked-type damptlauncher1)

(add damptlauncher1 wrecked-type koptlauncher1)

(add koptlauncher1 wrecked-type desptlauncher1)

(add offptlauncher1 wrecked-type koptlauncher1)



(add hphaser wrecked-type damhphaser)

(add damhphaser wrecked-type kohphaser)

(add kohphaser wrecked-type deshphaser)

(add offhphaser wrecked-type kohphaser)



(add hshield wrecked-type damhshield)

(add damhshield wrecked-type kohshield)

(add kohshield wrecked-type deshshield)

(add offhshield wrecked-type kohshield)



(add enterprise wrecked-type p-enthulk)

(add reliant wrecked-type p-relhulk)

(add excelsior wrecked-type p-exhulk)

(add fedstation1 wrecked-type p-fshulk1)

(add research wrecked-type p-rshulk1)

(add d7 wrecked-type p-d7hulk)

(add cloaked-d7 wrecked-type p-d7hulk)

(add bop wrecked-type p-bophulk)

(add cloaked-bop wrecked-type p-bophulk)

(add ktinga wrecked-type kthulk)

(add oberth wrecked-type p-obhulk)





;; END



(table detonation-unit-range

	(u* u* 0)

	(p-hulk-types u* 3)

	(station-types u* 4)

	(cloaked-ship-types u* 3)

	(pt u* 1)



)



(table detonation-damage-at

	(u* u* 0)



	(p-hulk-types ship-types 30)

	(p-hulk-types cloaked-ship-types 30)

	(p-hulk-types all-systems-types 8)



	(engineering all-systems-types 10)

	(damengineering all-systems-types 10)



	(engineering ship-types 10)

	(engineering light-cruiser-types 10)

	(engineering heavy-cruiser-types 15)

	(engineering battle-cruiser-types 20)



	(damengineering ship-types 20)

	(damengineering light-cruiser-types 20)

	(damengineering heavy-cruiser-types 25)

	(damengineering battle-cruiser-types 35)



	(koengineering ship-types 35)

	(koengineering light-cruiser-types 35)

	(koengineering heavy-cruiser-types 50)

	(koengineering battle-cruiser-types 75)



	(phaser ship-types 4)

	(phaser light-cruiser-types 4)

	(phaser heavy-cruiser-types 5)

	(phaser battle-cruiser-types 6)



	(damphaser ship-types 4)

	(damphaser light-cruiser-types 4)

	(damphaser heavy-cruiser-types 5)

	(damphaser battle-cruiser-types 6)



	(kophaser ship-types 4)

	(kophaser light-cruiser-types 4)

	(kophaser heavy-cruiser-types 5)

	(kophaser battle-cruiser-types 6)



	(disruptor ship-types 4)

	(disruptor light-cruiser-types 4)

	(disruptor heavy-cruiser-types 5)

	(disruptor battle-cruiser-types 6)



	(damdisruptor ship-types 4)

	(damdisruptor light-cruiser-types 4)

	(damdisruptor heavy-cruiser-types 5)

	(damdisruptor battle-cruiser-types 6)



	(kodisruptor ship-types 4)

	(kodisruptor light-cruiser-types 4)

	(kodisruptor heavy-cruiser-types 5)

	(kodisruptor battle-cruiser-types 6)



	(ptlauncher1 ship-types 10)

	(ptlauncher1 light-cruiser-types 10)

	(ptlauncher1 heavy-cruiser-types 10)

	(ptlauncher1 battle-cruiser-types 10)



	(damptlauncher1 ship-types 10)

	(damptlauncher1 light-cruiser-types 10)

	(damptlauncher1 heavy-cruiser-types 10)

	(damptlauncher1 battle-cruiser-types 10)



	(hphaser ship-types 25)

	(hphaser station-types 25)

	(hphaser battle-cruiser-types 25)



	(damhphaser ship-types 25)

	(damhphaser station-types 25)

	(damhphaser battle-cruiser-types 25)



)



(table detonation-damage-adjacent

	(u* u* 0)

	(p-hulk-types cloaked-ship-types 20)

	(p-hulk-types ship-types 20)

	(p-hulk-types all-systems-types 8)



	(pt u* 4)



)



;; GENESIS



(table detonation-terrain-range

	(genesis (nebula inhospitable) 3)

)





(table detonation-terrain-damage-chance

	(genesis (nebula inhospitable) 100)

)





(table terrain-damaged-type

	(nebula hospitable 100)

	(inhospitable hospitable 100)

)









(add u* lost-vanish-chance 0)



;;; The world.





(add deepspace alt-percentile-min 0)

(add deepspace alt-percentile-max 100)

(add deepspace wet-percentile-min 0)

(add deepspace wet-percentile-max 100)





;;;	Some defns for the fractal percentile generator.



(set alt-blob-density 10000)

(set alt-blob-height 500)

(set alt-blob-size 100)

(set alt-smoothing 4)

(set wet-blob-density 2000)

(set wet-blob-size 100)



(add excelsior start-with 1)

(add enterprise start-with 2)

(add reliant start-with 4)

(add oberth start-with 1)

(add fedstation1 start-with 1)

(add shipyard start-with 1)

(add research start-with 1)

(add phaser start-with 13)

(add ptlauncher1 start-with 10)

(add sensors start-with 10)

(add lrsensors start-with 3)

(add engineering start-with 8)

(add damagecontrol start-with 10)

(add shield start-with 7)

(add lshield start-with 1)

(add hphaser start-with 4)

(add hshield start-with 2)



(add u* match-transport-side true)



(add ship-types can-be-self true)

(add ship-types self-resurrects true)

(add cloaked-ship-types can-be-self true)

(add cloaked-ship-types self-resurrects true)





(table favored-terrain

  (u* t* 0)

  (ship-types deepspace 100)

  (victorypoint deepspace 100)

  (victorypoint nebula 100)



	)





;; WORDING



(set action-notices '(



    ((hit phaser u*) (actor " damages " actee "."))

    ((destroy phaser operational-systems-types) ("DIRECT HIT!!  " actor " damages " actee "!"))

    ((destroy phaser damaged-systems-types) ("DIRECT HIT!!  " actor " knocks out " actee "!"))

    ((destroy phaser ko-systems-types) ("DIRECT HIT!!  " actor " destroys " actee "!"))



    ((hit damphaser u*) (actor " damages " actee "."))

    ((destroy damphaser operational-systems-types) ("DIRECT HIT!!  " actor " damages " actee "!"))

    ((destroy damphaser damaged-systems-types) ("DIRECT HIT!!  " actor " knocks out " actee "!"))

    ((destroy damphaser ko-systems-types) ("DIRECT HIT!!  " actor " destroys " actee "!"))



    ((hit ptlauncher1 u*) (actor " damages " actee "."))

    ((destroy ptlauncher1 operational-systems-types) ("DIRECT HIT!!  " actor " damages " actee "!"))

    ((destroy ptlauncher1 damaged-systems-types) ("DIRECT HIT!!  " actor " knocks out " actee "!"))

    ((destroy ptlauncher1 ko-systems-types) ("DIRECT HIT!!  " actor " destroys " actee "!"))



    ((hit damptlauncher1 u*) (actor " damages " actee "."))

    ((destroy damptlauncher1 operational-systems-types) ("DIRECT HIT!!  " actor " damages " actee "!"))

    ((destroy damptlauncher1 damaged-systems-types) ("DIRECT HIT!!  " actor " knocks out " actee "!"))

    ((destroy damptlauncher1 ko-systems-types) ("DIRECT HIT!!  " actor " destroys " actee "!"))



    ((destroy fedcpc phaser) ("Phaser bank powering down."))

    ((destroy fedcpc ptlauncher1) ("Photon torpedo tube deactivating."))

    ((destroy fedcpc sensors) ("Sensor input interrupted."))

    ((destroy fedcpc lrsensors) ("Long-range sensor input interrupted."))

    ((destroy fedcpc engineering) ("Engineering reports all systems powering down."))

    ((destroy fedcpc damagecontrol) ("Damage control systems are off line."))

    ((destroy fedcpc shield) ("Shields are dropping."))



    ((hit fedcpc ship-types) ("Inputting command prefix code."))



    ((destroy u* offline-systems-types) ("Damage from " actor " causes " actee " to fuse!"))



    ((destroy u* operational-systems-types) (actor " damages " actee "!"))

    ((destroy u* damaged-systems-types) ("Hit!  "actor " knocks out " actee "!"))

    ((destroy u* ko-systems-types) ("DIRECT HIT!!" actor " destroys " actee "!"))



;;    ((hit fighter-types u*) (actor " strikes " actee "."))

;;    ((hit carrier-fighter-types u*) (actor " strikes " actee "."))

;;    ((capture u* place-types) (actor " takes " actee "."))



;;    ((disband mine-u* self done) (actor " is totally cleared."))

;;    ((liberate paratroops facility-u*) (actor " storms and liberates " actee "!"))

))



;; GENERAL CONDITIONS





(set country-radius-max 20)

(set country-separation-min 10)

(set edge-terrain inhospitable)







(game-module (design-notes

  "Based on th old Star Fleet Battles boardgame"

  ))

(imf "spec-blank" ((8 8) (file "spec8.gif" std 0 0)))

(imf "kb-st-ship-enterprise" ((64 64) (file "kobayashi.gif" std 0 0)))

(imf "kb-st-ship-reliant" ((64 64) (file "kobayashi.gif" std 0 1)))

(imf "kb-st-ship-excelsior" ((64 64) (file "kobayashi.gif" std 0 3)))

(imf "kb-st-ship-oberth" ((64 64) (file "kobayashi.gif" std 0 2)))

(imf "kb-kling-ship-ktinga" ((64 64) (file "kobayashi.gif" std 3 0)))

(imf "kb-kling-ship-d7" ((64 64) (file "kobayashi.gif" std 2 0)))

(imf "kb-kling-ship-bop" ((64 64) (file "kobayashi.gif" std 2 1)))

(imf "kb-kling-cloak-d7" ((64 64) (file "kobayashi.gif" std 8 1)))

(imf "kb-kling-cloak-ktinga" ((64 64) (file "kobayashi.gif" std 4 4)))

(imf "kb-kling-cloak-bop" ((64 64) (file "kobayashi.gif" std 3 1)))

(imf "kb-st-station-research" ((64 64) (file "kobayashi.gif" std 1 5)))

(imf "kb-st-station-ds9" ((64 64) (file "kobayashi.gif" std 2 4)))

(imf "kb-st-station-yard" ((64 64) (file "kobayashi.gif" std 8 4)))



(imf "kb-explosion-orange" ((64 64) (file "kobayashi.gif" std 1 8)))



(imf "kb-st-wreck-enterprise" ((64 64) (file "kob-wreck.gif" std 0 0)))

(imf "kb-st-wreck-enterprise1" ((64 64) (file "kob-wreck.gif" std 1 0)))

(imf "kb-st-wreck-enterprise2" ((64 64) (file "kob-wreck.gif" std 2 0)))

(imf "kb-st-smold-enterprise" ((64 64) (file "kob-wreck.gif" std 3 0)))



(imf "kb-st-wreck-reliant" ((64 64) (file "kob-wreck.gif" std 0 1)))

(imf "kb-st-wreck-reliant1" ((64 64) (file "kob-wreck.gif" std 1 1)))

(imf "kb-st-wreck-reliant2" ((64 64) (file "kob-wreck.gif" std 2 1)))

(imf "kb-st-smold-reliant" ((64 64) (file "kob-wreck.gif" std 3 1)))



(imf "kb-st-wreck-excelsior" ((64 64) (file "kob-wreck.gif" std 0 4)))

(imf "kb-st-smold-excelsior" ((64 64) (file "kob-wreck.gif" std 3 4)))



(imf "kb-st-wreck-research" ((64 64) (file "kob-wreck.gif" std 1 5)))

(imf "kb-st-smold-research" ((64 64) (file "kob-wreck.gif" std 3 5)))



(imf "kb-st-wreck-oberth" ((64 64) (file "kob-wreck.gif" std 0 6)))

(imf "kb-st-smold-oberth" ((64 64) (file "kob-wreck.gif" std 3 6)))



(imf "kb-st-wreck-ds9" ((64 64) (file "kob-wreck.gif" std 0 7)))

(imf "kb-st-smold-ds9" ((64 64) (file "kob-wreck.gif" std 3 7)))



(imf "kb-kling-wreck-d7" ((64 64) (file "kob-wreck.gif" std 0 2)))

(imf "kb-kling-smold-d7" ((64 64) (file "kob-wreck.gif" std 3 2)))



(imf "kb-kling-wreck-ktinga" ((64 64) (file "kob-wreck.gif" std 0 3)))

(imf "kb-kling-wreck-ktinga2" ((64 64) (file "kob-wreck.gif" std 1 3)))

(imf "kb-kling-smold-ktinga" ((64 64) (file "kob-wreck.gif" std 3 3)))



(imf "kb-kling-wreck-bop" ((64 64) (file "kob-wreck.gif" std 0 8)))

(imf "kb-kling-smold-bop" ((64 64) (file "kob-wreck.gif" std 3 8)))



(imf "kb-st-ship-transport" ((64 64) (file "kobayashi.gif" std 1 6)))





(imf "ba-sensors-green" ((32 32) (file "battles1.gif" std 0 0)))

(imf "ba-sensors-yellow" ((32 32) (file "battles1.gif" std 0 1)))

(imf "ba-sensors-red" ((32 32) (file "battles1.gif" std 0 2)))

(imf "ba-sensors-black" ((32 32) (file "battles1.gif" std 0 3)))

(imf "ba-sensors-tan" ((32 32) (file "battles1.gif" std 0 4)))

(imf "ba-ray-green" ((32 32) (file "battles1.gif" std 0 5)))

(imf "ba-ray-yellow" ((32 32) (file "battles1.gif" std 0 6)))

(imf "ba-ray-red" ((32 32) (file "battles1.gif" std 0 7)))

(imf "ba-ray-black" ((32 32) (file "battles1.gif" std 0 8)))

(imf "ba-ray-tan" ((32 32) (file "battles1.gif" std 0 9)))



(imf "ba-wrench-green" ((32 32) (file "battles1.gif" std 1 0)))

(imf "ba-wrench-yellow" ((32 32) (file "battles1.gif" std 1 1)))

(imf "ba-wrench-red" ((32 32) (file "battles1.gif" std 1 2)))

(imf "ba-wrench-black" ((32 32) (file "battles1.gif" std 1 3)))

(imf "ba-wrench-tan" ((32 32) (file "battles1.gif" std 1 4)))

(imf "ba-oval-green" ((32 32) (file "battles1.gif" std 1 5)))

(imf "ba-oval-yellow" ((32 32) (file "battles1.gif" std 1 6)))

(imf "ba-oval-red" ((32 32) (file "battles1.gif" std 1 7)))

(imf "ba-oval-black" ((32 32) (file "battles1.gif" std 1 8)))

(imf "ba-oval-tan" ((32 32) (file "battles1.gif" std 1 9)))



(imf "ba-cross-green" ((32 32) (file "battles1.gif" std 2 0)))

(imf "ba-cross-yellow" ((32 32) (file "battles1.gif" std 2 1)))

(imf "ba-cross-red" ((32 32) (file "battles1.gif" std 2 2)))

(imf "ba-cross-black" ((32 32) (file "battles1.gif" std 2 3)))

(imf "ba-cross-tan" ((32 32) (file "battles1.gif" std 2 4)))

(imf "ba-star-green" ((32 32) (file "battles1.gif" std 2 5)))

(imf "ba-star-yellow" ((32 32) (file "battles1.gif" std 2 6)))

(imf "ba-star-red" ((32 32) (file "battles1.gif" std 2 7)))

(imf "ba-star-black" ((32 32) (file "battles1.gif" std 2 8)))

(imf "ba-star-tan" ((32 32) (file "battles1.gif" std 2 9)))



(imf "ba-3dash-green" ((32 32) (file "battles1.gif" std 3 0)))

(imf "ba-3dash-yellow" ((32 32) (file "battles1.gif" std 3 1)))

(imf "ba-3dash-red" ((32 32) (file "battles1.gif" std 3 2)))

(imf "ba-3dash-black" ((32 32) (file "battles1.gif" std 3 3)))

(imf "ba-3dash-tan" ((32 32) (file "battles1.gif" std 3 4)))

(imf "ba-2dash-green" ((32 32) (file "battles1.gif" std 3 5)))

(imf "ba-2dash-yellow" ((32 32) (file "battles1.gif" std 3 6)))

(imf "ba-2dash-red" ((32 32) (file "battles1.gif" std 3 7)))

(imf "ba-2dash-black" ((32 32) (file "battles1.gif" std 3 8)))

(imf "ba-2dash-tan" ((32 32) (file "battles1.gif" std 3 9)))



(imf "ba-commands-tan" ((32 32) (file "battles1.gif" std 9 0)))

(imf "ba-misc-v" ((32 32) (file "battles1.gif" std 9 1)))



(imf "ba-neb" ((1 1) 26214 13107 0))



(imf "ba-space" ((24 26 terrain) (x 8 26 0) (file "scifit24x26.gif" std 0 0)))

(imf "ba-neb" ((24 26 terrain) (x 8 26 0) (file "scifit24x26.gif" std 0 1)))



(imf "ba-space" ((44 48 terrain) (x 8 46 0) (file "scifit44x48.gif" std 0 0)))

(imf "ba-neb" ((44 48 terrain) (x 8 46 0) (file "scifit44x48.gif" std 0 1)))



(imf "ba-space" ((32 32 tile) (file "scifit32x32.gif" 0 0)))

(imf "ba-neb" ((32 32 tile) (file "scifit32x32.gif" 0 1)))



(imf "ba-sensor-contact-1" ((32 32) (file "battles1.gif" std 8 0)))

(imf "ba-sensor-contact-2" ((32 32) (file "battles1.gif" std 8 1)))

(imf "ba-sensor-contact" ((32 32) (file "battles1.gif" std 8 5)))



(imf "ba-vp-blue" ((32 32) (file "battles1.gif" std 9 2)))

