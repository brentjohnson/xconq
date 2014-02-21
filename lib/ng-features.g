;; Namers for geographical features.

(namer generic-island-names (grammar root 10
  (root (or 1 (foo "Ile d'" short-generic-names)
            2 (short-generic-names " Isle")
            10 (short-generic-names " Island")
            ))
  (foo "")  ; works around a bug
  ))

(namer generic-lake-names (grammar root 10
  (root (or 1 (foo "Lac d'" short-generic-names)
            2 (short-generic-names " Lake")
  	   10 (foo "Lake " short-generic-names)
            ))
  (foo "")  ; works around a bug
  ))

(namer generic-bay-names (grammar root 10
  (root (or 1 (foo "Bay of " short-generic-names)
            5 (short-generic-names " Bay")
            ))
  (foo "")  ; works around a bug
  ))

(namer generic-sea-names (grammar root 10
  (root (or 5 (foo "Sea of " short-generic-names)
            1 (short-generic-names "'s Sea")
            ))
  (foo "")  ; works around a bug
  ))

(namer generic-continent-names (grammar root 10
  (root (or 1 (foo "Terre d'" short-generic-names)
  	   2 (foo "Land of " short-generic-names)
            10 (short-generic-names "'s Land")
            ))
  (foo "")  ; works around a bug
  ))

(namer generic-mountain-names (grammar root 10
  (root (or 1 (short-generic-names " Range")
            1 (short-generic-names " Mountains")
            ))
  (foo "")  ; works around a bug
  ))

(namer generic-desert-names (grammar root 10
  (root (short-generic-names " Desert")
            )
  (foo "")  ; works around a bug
  ))

(namer generic-forest-names (grammar root 10
  (root (short-generic-names " Forest")
            )
  (foo "")  ; works around a bug
  ))

(include "ng-weird")
