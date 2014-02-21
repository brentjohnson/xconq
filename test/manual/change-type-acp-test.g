(game-module "change-type-acp-test"
    (title "Change-Type ACP Tester")
    (version "1.0")
    (blurb "change-type ACP test written by Eric McDonald.")
    (notes (
	"Tests changes to the unit's actor state from a change-type action."
    ))
    (variants (see-all true))
)

(include "stdterr")

(unit-type unit-acp-1 (image-name "adventurer") (acp-per-turn 1))
(unit-type unit-acp-2 (image-name "adventurer") (acp-per-turn 2)
    (start-with 1))
(unit-type unit-acp-3 (image-name "adventurer") (acp-per-turn 3))
(unit-type unit-acp-4 (image-name "adventurer") (acp-per-turn 4))
(unit-type unit-acp-6 (image-name "adventurer") (acp-per-turn 6))
(unit-type unit-acp-indep1 (image-name "adventurer") 
    (acp-independent true) (advanced true))
(unit-type unit-acp-indep2 (image-name "adventurer") 
    (acp-independent true) (advanced true))
(unit-type unit-acp-indep3 (image-name "adventurer") 
    (acp-independent true) (advanced true))
(unit-type unit-acpless1 (image-name "adventurer") (acp-per-turn 0)) 
(unit-type unit-acpless2 (image-name "adventurer") (acp-per-turn 0)) 
(unit-type unit-acpless3 (image-name "adventurer") (acp-per-turn 0)) 

(add unit-acp-indep1 auto-upgrade-to unit-acp-1)
(add unit-acp-indep2 auto-upgrade-to unit-acpless2)
(add unit-acp-indep3 auto-upgrade-to unit-acp-4)
(add unit-acpless1 auto-upgrade-to unit-acp-3)
(add unit-acpless2 auto-upgrade-to unit-acpless3)
(add unit-acpless3 auto-upgrade-to unit-acp-indep3)

(table can-change-type-to
    (unit-acp-2 unit-acp-indep1 true)
    (unit-acp-indep1 unit-acp-1 true)
    (unit-acp-1 unit-acpless1 true)
    (unit-acpless1 unit-acp-3 true)
    (unit-acp-3 unit-acp-6 true)
    (unit-acp-6 unit-acp-indep2 true)
    (unit-acp-indep2 unit-acpless2 true)
    (unit-acpless2 unit-acpless3 true)
    (unit-acpless3 unit-acp-indep3 true)
    (unit-acp-indep3 unit-acp-4 true)
    (unit-acp-4 unit-acp-2 true)
)

(table acp-to-change-type
    (unit-acp-2 unit-acp-indep1 1)
    (unit-acp-1 unit-acpless1 1)
    (unit-acp-3 unit-acp-6 2)
    (unit-acp-6 unit-acp-indep2 6)
    (unit-acp-4 unit-acp-2 4)
)

(set synthesis-methods '(
    make-fractal-percentile-terrain
    make-countries
))
