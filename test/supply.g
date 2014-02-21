; This is a silly test module.

(game-module "supply"
    (title "Supply rules test module")
    (blurb "For debugging the new supply system")
    )

(set see-all true)

(unit-type infantry
    (image-name "soldiers")
    (start-with 6)
    )

(unit-type town
    (image-name "town20")
    (start-with 2)
    )

(terrain-type plains)
(terrain-type mountains)

; Two types of materials: food and ammunition.
(material-type food)
(material-type ammo)

(table unit-size-in-terrain
    (town t* 0)
    (infantry t* 1)
    )

(table acp-to-attack
    (infantry u* 2)
    )

(table hit-chance
    (infantry u* 50)
    (town u* 0)
    )

(add (infantry town) hp-max (2 3))

(table damage
    (u* u* 1)
    )

; Need also food to attack.
(table material-to-attack
    (infantry m* 1)
    )
    
(table consumption-per-attack
    (u* ammo 1)
    )

(table hit-by
    (u* ammo 1)
    )

(table consumption-per-move
    (infantry food 1)
    )

(table unit-storage-x
    (town m* (30 30))
    (infantry m* (10 3))
    )

(table supply-in-weight
    (infantry m* 1)
    )

; Give more to starving troops
(table supply-starve-weight
    (infantry m* 10)
    )

; Food accounts for 80% in the supply statistics.
(table supply-importance
    (infantry food 4)
    (infantry ammo 1)
    )

(table control-range
    (u* t* 0)
    )

(table unit-initial-supply
    (u* m* 9999)
    )

(table supply-deterioration
    (t* m* 1)
    ; Can't get supplies to mountains.
    (mountains food 99)
    )

; Must control cell to run supply through it.
(table supply-enemy-interdiction
    (t* m* 99)
    )
    
(table supply-neutral-interdiction
    (t* m* 99)
    )

; Enemy infantry units can cut supply lines.
; supply-interdiction-at will be redundant in this case once the control layer
; is properly implemented.
(table supply-interdiction-at
    (infantry t* 99)
    )

(table supply-interdiction-adjacent
    (infantry t* 5)
    )

; If supply lines weaken, transfer capacity goes down also.
(table supply-capacity-threshold
    (infantry food 6)
    )
    
(table supply-capacity-deterioration
    (infantry food 0)
    )

; Food can be distributed up to 10 hexes away but to get more ammo infantry
; must be less than 3 hexes from a town (makes no sense but who cares).
(table supply-potential
    (town m* (11 3))
    )

(table supply-in-max
    (infantry food 5)
    (infantry ammo 1)
    )

(table base-consumption
    (infantry food 2)
    )

; Lose one hp each turn if starving.
(table hp-per-starve
    (infantry food 1.00)
    )

(table material-to-move
    (infantry food 1)
    )

; One town can support 3 infantry (if they do not move!)...
(table base-production
    (town food 6)
    )

(table supply-out-max
    (town m* 9999)
    )

(table supply-out-threshold
    (town m* 0)
    )

(table supply-in-threshold
    (infantry m* 9999)
    )

(add infantry acp-per-turn 3)

(add u* free-mp 99)

(table mp-to-enter-terrain
    (infantry mountains 3)
    )

(add town acp-per-turn 1)
(add town speed 0)

(add infantry cp 5)

(table acp-to-create
    (town infantry 1)
    )

(table supply-on-creation
    (u* m* 9999)
    )

(table acp-to-build
    (town infantry 1)
    )

(set event-notices '(
  ((unit-starved infantry) (0 " runs out of supplies and dies."))
  ))

(set country-separation-min 12)
(set country-separation-max 13)
(set country-radius-min 4)
(set country-radius-max 4)

(set synthesis-methods '(make-random-terrain make-countries make-initial-materials))

(scorekeeper (do last-side-wins))
