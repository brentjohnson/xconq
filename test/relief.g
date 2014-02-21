(game-module "relief"
  (blurb "A test game for displaying something resembling relief maps.")
  (variants (world-size 60 30 360))
  )

(unit-type C (name "caveman") (char "C") (image-name "person"))

(terrain-type a (image-name "sea1"))
(terrain-type b (image-name "sea2"))
(terrain-type c (image-name "sea3"))
(terrain-type d (image-name "sea4"))
(terrain-type e (image-name "sea5"))
(terrain-type f (image-name "sea6"))
(terrain-type g (image-name "sea7"))
(terrain-type h (image-name "sea8"))
(terrain-type i (image-name "sea9"))
(terrain-type j (image-name "sea10"))
(terrain-type k (image-name "sea11"))
(terrain-type l (image-name "sea12"))
(terrain-type m (image-name "sea13"))
(terrain-type n (image-name "sea14"))
(terrain-type o (image-name "land1"))
(terrain-type p (image-name "land2"))
(terrain-type q (image-name "land3"))
(terrain-type r (image-name "land4"))
(terrain-type s (image-name "land5"))
(terrain-type t (image-name "land6"))

;; Program's defaults.

(set alt-blob-density 1000)
(set alt-blob-height 500)
(set alt-blob-size 30)
(set alt-smoothing 2)

(set alt-blob-density 5000)
(set alt-blob-height 500)
(set alt-blob-size 50)
(set alt-smoothing 5)

(add t* alt-percentile-min ( 0  5 10 15 20 25 30 35 40 45 50 55 60 65 70 75 80 85 90  95 ))
(add t* alt-percentile-max ( 5 10 15 20 25 30 35 40 45 50 55 60 65 70 75 80 85 90 95 100 ))
(add t* wet-percentile-min   0)
(add t* wet-percentile-max 100)

(add t* elevation-min 0)
(add t* elevation-max 1000)

(area (cell-width 100))

(add C acp-per-turn 1)

(set see-all true)

(set sides-min 1)

(imf "sea1" ((1 1) 0 0 40000))
(imf "sea2" ((1 1) 0 0 42000))
(imf "sea3" ((1 1) 0 0 44000))
(imf "sea4" ((1 1) 0 0 46000))
(imf "sea5" ((1 1) 0 0 48000))
(imf "sea6" ((1 1) 0 0 50000))
(imf "sea7" ((1 1) 0 0 52000))
(imf "sea8" ((1 1) 0 0 54000))
(imf "sea9" ((1 1) 0 0 56000))
(imf "sea10" ((1 1) 0 0 58000))
(imf "sea11" ((1 1) 0 0 60000))
(imf "sea12" ((1 1) 5000 10000 62000))
(imf "sea13" ((1 1) 10000 20000 64000))
(imf "sea14" ((1 1) 15000 30000 64000))

(imf "land1" ((1 1) 10000 40000 10000))
(imf "land2" ((1 1) 20000 45000 20000))
(imf "land3" ((1 1) 30000 50000 30000))
(imf "land4" ((1 1) 40000 53000 40000))
(imf "land5" ((1 1) 50000 57000 50000))
(imf "land6" ((1 1) 60000 60000 60000))
