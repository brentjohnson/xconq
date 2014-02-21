(game-module "advterr"
  (blurb "Set of terrain types used for advances games")
  )

(terrain-type sea
  (capacity 16)
  (char ".")
  (help "ocean and salt lakes")
  (image-name "sea")
  (liquid 1)
  (wet-percentile-max 100)
  )

(terrain-type lake
  (capacity 16)
  (char ",")
  (help "fresh water lake")
  (image-name "shallows")
  (liquid 1)
  (wet-percentile-max 100)
  )

(terrain-type swamp
  (alt-percentile-max 2)
  (capacity 16)
  (char "=")
  (elevation-max 2)
  (help "fresh water swamp")
  (image-name "adv-swamp")
  (wet-percentile-max 100)
  )

(terrain-type salt-marsh 
  (alt-percentile-max 2)
  (capacity 16)
  (elevation-max 2)
  (char "#")
  (help "salt marsh")
  (image-name "swamp")
  (wet-percentile-max 100)
  )

(terrain-type desert
  (alt-percentile-max 90)
  (alt-percentile-min 2)
  (capacity 16)
  (char "~")
  (elevation-max 2000)
  (elevation-min 3)
  (help "sand or rock desert")
  (image-name "adv-desert")
  (wet-percentile-max 20)
  )

(terrain-type semi-desert
  (alt-percentile-max 90)
  (alt-percentile-min 2)
  (capacity 16)
  (char "!")
  (elevation-max 2000)
  (elevation-min 3)
  (help "arid semi-desert")
  (image-name "adv-semi-desert")
  (wet-percentile-max 20)
  )

(terrain-type steppe
  (alt-percentile-max 90)
  (alt-percentile-min 2)
  (capacity 16)
  (char "*")
  (elevation-max 2000)
  (elevation-min 3)
  (help "steppe")
  (image-name "adv-steppe")
  (river-chance 500)
  (wet-percentile-max 60)
  (wet-percentile-min 20)
  )

(terrain-type plain
  (alt-percentile-max 90)
  (alt-percentile-min 2)
  (capacity 16)
  (char "+")
  (elevation-max 2000)
  (elevation-min 3)
  (help "flat or rolling grassland")
  (image-name "adv-plain")
  (river-chance 500)
  (wet-percentile-max 60)
  (wet-percentile-min 20)
  )

(terrain-type forest
  (alt-percentile-max 90)
  (alt-percentile-min 2)
  (capacity 16)
  (char "%")
  (elevation-max 2000)
  (elevation-min 3)
  (help "forest")
  (image-name "adv-forest")
  (river-chance 800)
  (wet-percentile-max 100)
  (wet-percentile-min 60)
  )

(terrain-type mountain
  (alt-percentile-max 99)
  (alt-percentile-min 90)
  (capacity 16)
  (char "^")
  (elevation-max 6000)
  (elevation-min 2001)
  (help "mountains or rugged terrain")
  (image-name "adv-mountains")
  (river-chance 1200)
  (wet-percentile-max 100)
  )

(terrain-type ice
  (alt-percentile-max 100)
  (alt-percentile-min 99)
  (capacity 16)
  (char "_")
  (elevation-max 9000)
  (elevation-min 6001)
  (help "permanent ice fields")
  (image-name "adv-ice")
  (wet-percentile-max 100)
  )

(terrain-type road
  (capacity 16)
  (char ">")
  (subtype connection)
  (subtype-x road-x)
  (image-name "road")
  )

(terrain-type river
  (capacity 16)
  (char "<")
  (subtype border)
  (subtype-x river-x)
  (image-name "river")
  )

(terrain-type ford 
  (capacity 16)
  (char "|")
  (subtype border)
  (image-name "beach")
)

(define land-t* (desert semi-desert steppe plain forest mountain))

(define cell-t* (sea lake swamp salt-marsh desert semi-desert steppe plain forest mountain ice))

(imf "adv-desert" ((1 1) 65535 65535 0))
(imf "adv-desert" ((8 8 tile)
  (color (pixel-size 2)
   (palette
    (0 65535 65535 0)
    (1 52428 26214 0)
    (2 52428 26214 13107))
   "0001/0000/0100/0000/0010/0000/2000/0000")
  (mono "01/00/10/00/04/00/40/00")))

(imf "adv-forest" ((1 1) 0 21845 0))
(imf "adv-forest" ((8 8 tile)
  (color (pixel-size 4)
   (palette
    (0 0 34952 0)
    (1 0 30583 0)
    (2 0 43690 0)
    (3 0 8738 0)
    (4 26214 13107 0))
   "33000000/11300330/11333143/13143141/11141311/13311113/34131332/14131322")
  (mono "3f/d9/d6/b7/fb/9e/69/eb")))

(imf "adv-ice" ((1 1) white))
(imf "adv-ice" ((8 8 tile)
  (color (pixel-size 1) (palette (0 white) (1 26214 65535 65535))
   "48/b6/08/51/a6/18/21/44")
  (mono "00/00/00/00/00/00/00/00")))

(imf "adv-lake" ((1 1) 0 65535 65535))

(imf "adv-mountains" ((1 1) 26214 13107 0))
(imf "adv-mountains" ((8 8 tile)
  (color (pixel-size 2)
   (palette
    (0 26214 13107 0)
    (1 39321 13107 0)
    (3 black))
   "d030/400f/03cc/0df0/3430/d00c/0c0c/37c0")
  (mono "84/03/1a/2c/44/82/22/58")))

(imf "adv-plain" ((1 1) 0 56797 0))
(imf "adv-plain" ((8 8 tile)
  (color (pixel-size 1) (palette (0 0 56797 0) (1 0 30583 0))
   "00/50/00/00/00/0a/00/00")
  (mono "00/50/00/00/00/0a/00/00")))

(imf "adv-river" ((1 1) 0 0 61166))
(imf "adv-river" ((8 8 tile)
  (color (pixel-size 1) (palette (0 0 0 61166))
   "00/00/00/00/00/00/00/00")
  (mono "dd/77/dd/77/dd/77/dd/77")))

(imf "adv-road" ((1 1) 32768 32768 32768))
(imf "adv-road" ((8 8 tile)
  (color (pixel-size 1) (palette (0 39321 26214 0) (1 13107 13107 0))
   "11/40/0a/80/21/04/90/04")
  (mono "aa/55/aa/55/aa/55/aa/55")))

(imf "adv-salt-marsh" ((1 1) 13107 52428 39321))
(imf "adv-salt-marsh" ((8 8 tile)
  (color (pixel-size 2)
   (palette
    (0 26214 65535 65535)
    (1 13107 52428 39321)
    (2 39321 65535 65535))
   "5625/9595/2556/9565/5589/6565/8955/6595")
  (mono "00/00/00/00/00/00/00/00")))

(imf "adv-sea" ((1 1) 0 39321 65535))
(imf "adv-sea" ((8 8 tile)
  (color (pixel-size 2)
   (palette
    (0 13107 65535 65535)
    (1 0 39321 65535)
    (2 0 13107 65535))
   "5555/5551/5549/5526/5155/4955/2655/5555")
  (mono "00/00/00/00/00/00/00/00")))

(imf "adv-semi-desert" ((1 1) 65535 39321 26214))
(imf "adv-semi-desert" ((8 8 tile)
  (color (pixel-size 1) (palette (0 0 34952 0) (1 65535 39321 26214))
   "ff/fd/ff/df/ff/f7/ff/7f")
  (mono "00/02/00/20/00/08/00/80")))

(imf "adv-steppe" ((1 1) 65535 39321 0))
(imf "adv-steppe" ((8 8 tile)
  (color (pixel-size 1) (palette (0 0 43690 0) (1 65535 39321 0))
   "f5/b5/bf/ff/5f/5b/fb/ff")
  (mono "0a/4a/40/00/a0/a4/04/00")))

(imf "adv-swamp" ((1 1) 0 52428 26214))
(imf "adv-swamp" ((8 8 tile)
  (color (pixel-size 4)
   (palette
    (0 0 13107 0)
    (1 0 17476 0)
    (2 0 13107 65535)
    (3 0 26214 65535)
    (4 0 52428 26214))
   "04444404/44444444/34434443/44444444/44441414/44444444/42424244/44444444")
  (mono "00/00/00/00/00/00/00/00")))

;;;	Some defns for the fractal percentile generator.

(set alt-blob-density 10000)
(set alt-blob-height 500)
(set alt-blob-size 100)
(set alt-smoothing 4)
(set wet-blob-density 2000)
(set wet-blob-size 100)

;;;	River generation.

;;	Rivers are most likely to start in the mountains or forests.

;; (add (plain steppe forest mountain) river-chance (5.00 5.00 8.00 12.00))

;;	Rivers empty into lakes, swamps or salt-marshes if they don't reach the sea.

;; (set river-sink-terrain (lake swamp salt-marsh))

;;;	Road generation.

(table road-into-chance
  (land-t* land-t* 100)
  ;; 	No roads across ice fields.
  (land-t* ice 0)
  ;; 	Try to get a road back out into the plains.
  (cell-t* (plain steppe) 100)
  ;; 	Be reluctant to run through hostile terrain.
  (plain (desert forest mountain) (40 30 20))
  )

(set edge-terrain ice)
