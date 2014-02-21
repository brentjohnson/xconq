(game-module "stdterr"
  (blurb "Standard set of terrain types shared by many game designs")
  )

(terrain-type sea (char ".")
  (help "deep ocean"))
(terrain-type shallows (char ",")
  (help "coastal waters and lakes"))
(terrain-type swamp (char "=")
  (help "standing water and dense undergrowth"))
(terrain-type desert (char "~")
  (help "dry and sandy or rocky terrain"))
(terrain-type plains (char "+")
  (help "flat or rolling countryside or steppe"))
(terrain-type forest (char "%")
  (help "dense forest or jungle"))
(terrain-type mountains (char "^")
  (help "high elevation and/or rugged terrain"))
(terrain-type ice (char "_")
  (help "permanent ice fields"))

(define land-t* (desert plains forest mountains))

(define cell-t* (sea shallows swamp desert plains forest mountains ice))

(terrain-type river (char "<")
  (subtype border) (subtype-x river-x))

;; Road comes after river so it gets drawn over river by default.

(terrain-type road (char ">")
  (subtype connection) (subtype-x road-x))

;; Different classes of terrain should not have overlapping elevation ranges.

(add (sea shallows swamp) elevation-min 0)
(add (sea shallows swamp) elevation-max (0 0 2))
(add (desert plains forest) elevation-min 3)
(add (desert plains forest) elevation-max 2000)
(add mountains elevation-min 2001)
(add mountains elevation-max 6000)
(add ice elevation-min 6001)
(add ice elevation-max 9000)

;; The elevations above are consistent with 100-km-across cells. */

(area (cell-width 100000))

(add (sea shallows) liquid true)

;;; Some defns for the fractal percentile generator.

(set alt-blob-density 10000)
(set alt-blob-height 500)
(set alt-blob-size 100)
(set alt-smoothing 3)
(set wet-blob-density 2000)
(set wet-blob-size 100)

(add cell-t* alt-percentile-min (  0  68  69  70  70  70  93  99))
(add cell-t* alt-percentile-max ( 68  69  71  93  93  93  99 100))
(add cell-t* wet-percentile-min (  0   0  50   0  20  80   0   0))
(add cell-t* wet-percentile-max (100 100 100  20  80 100 100 100))

;;; River generation.

;; Rivers are most likely to start in the mountains or forests.

(add (plains forest mountains) river-chance (5.00 8.00 12.00))

;; Rivers empty into lakes if they don't reach the sea.

(set river-sink-terrain shallows)

;;; Road generation.

(table road-into-chance
  (land-t* land-t* 100)
  ;; No roads across ice fields.
  (land-t* ice 0)
  ;; Try to get a road back out into the plains.
  (cell-t* plains 100)
  ;; Be reluctant to run through hostile terrain.
  (plains (desert forest mountains) (40 30 20))
  )

(set edge-terrain ice)

;;; Any attempts to use t* in other games should be aware of
;;; how many types are defined in this file.

