(game-module "metz-1944"
  (title "Metz 1944")
  (version "1.0")
  (blurb "The allied drive on Metz in the fall of 1944.")
  (base-module "ww2-bn")
  (variants
   (world-seen true)
   (see-all false)
   (sequential true)
   )
  )

;(set terrain-seen true)

(add u* vision-range 2)

;; Limited air support and intelligence, so everybody is line-of-sight.

(add u* can-see-behind false)

(table eye-height
  ;; Everybody has *something* to stand on to see further.
  (u* t* 4)
  )

;; Trees go up to about 20m in height.

(add (bocage forest) thickness 20)

; not really...
(scorekeeper (do last-side-wins))

(add u* point-value 0)
(add village point-value 1)
(add fort point-value 10)
(add town point-value 50)

(table unit-initial-supply add (village m* 0))

(set initial-date "8:00 7 Sep 1944")

(set initial-day-part 1.50)

;; The game lasts up to three months.

(set last-turn 270)

(add u* start-with 0)

(side 1 (name "America") (adjective "American") (class "allied")
   (color "blue") (emblem-name "white-star"))

(side 2 (name "Germany") (adjective "German") (class "german")
   (color "black") (emblem-name "german-cross"))

(set sides-min 2)
(set sides-max 2)

(set synthesis-methods nil)

(area 36 27)

(area (terrain
 (by-name
    (sea 0) (clear 1) (bocage 2) (hill 3) (forest 4)
    (swamp 5) (flooded 6) (beach 7) (city 8) (river 9)
    (road 10) (sec-road 11) (railroad 12))
  "23b13a"
  "24b12a"
  "25b11a"
  "18bi7b10a"
  "27b9a"
  "6be9be7be3b8a"
  "6be9be8be3b7a"
  "6be10be7be4b6a"
  "2be3b3e9be6be5b5a"
  "2b2e3be10be6be6b4a"
  "3b2e5be8be13b3a"
  "19be14b2a"
  "35ba"
  "22bi13b"
  "a21b2i12b"
  "2a5be11be3bi12b"
  "3a4b3e6be2be16b"
  "4a3b4e2be5be6be9b"
  "5a3b4e3be2b2e16b"
  "6a3b4e7be15b"
  "7a3b3e3be3be15b"
  "8a3b3e6be15b"
  "9a3b3e6be14b"
  "10a4b2e5be14b"
  "11a25b"
  "12a24b"
  "13a23b"
  ))

(area (aux-terrain river
  "36a"
  "18ai17a"
  "18ahq16a"
  "18agL16a"
  "18agL16a"
  "18acT16a"
  "19ahq15a"
  "19acT15a"
  "20ahq14a"
  "20acT14a"
  "21ahq13a"
  "21agL13a"
  "21agT13a"
  "21agM13a"
  "20aeoL13a"
  "20agM<13a"
  "20agL14a"
  "20agL14a"
  "20acT14a"
  "21ahq13a"
  "21agL13a"
  "21acT13a"
  "22ahq12a"
  "22acT12a"
  "23ahq11a"
  "23acL11a"
  "36a"
  ))

(area (aux-terrain road
  "36a"
  "17aie17a"
  "ai15afaD16a"
  "af12ak3sTj16a"
  "2aD10akr3a2j16a"
  "2af9akr4ajf2ai13a"
  "3aDk7sz5afaDaj13a"
  "c2sxr4ak2sr6aDfaj13a"
  "4a>4sv9afaDj13a"
  "10aFu3ae4aDfj13a"
  "10afa@3a@3afaE13a"
  "11aDa>u2a@3aDj13a"
  "11af3a@2a@2a2j13a"
  "c11sR3sN2sNs~j13a"
  "13aD6akrK7sq5a"
  "13af6ajkrD12a"
  "14a@5a2jaf12a"
  "15a@4a2j2aD11a"
  "16a@3a2f2af11a"
  "17a@ksu2D2a@10a"
  "18aE2aEj3aD9a"
  "17akr2abf3af9a"
  "17aj5aD3aD8a"
  "17ab5af3af8a"
  "24aD3aD7a"
  "24ab3ab7a"
  "36a"
  ))

(area (aux-terrain railroad
  "36a"
  "9ae10ai15a"
  "10aFu8aj15a"
  "10aja>su3aksr15a"
  "10af4a>2sz17a"
  "11a@6aj17a"
  "ae10a@5aj5ai11a"
  "2a>u5ak3sN4sv5aj11a"
  "4a>4sv9aD4aj11a"
  "10a@8af4aj11a"
  "11aD8a@3aj11a"
  "11af9aD2aj11a"
  "12aD8anakr11a"
  "12af8aj>r12a"
  "13a@6akv14a"
  "14a@5aja@13a"
  "15a@4aj2a>2su9a"
  "16a@3aj6a>2su5a"
  "17a@2af10a>2sqa"
  "18aFsuD14a"
  "18aj2aE14a"
  "17akr2af14a"
  "17aj4aD13a"
  "17aj4af13a"
  "17ab5aD12a"
  "23ab12a"
  "36a"
  ))

(area (elevations
  "4*200,3*350,300,4*350,300,350,2*180,3*160,2*170,2*220,13a"
  "6*200,2*350,300,3*350,2*300,200,180,170,2*160,180,2*200,2*220,12a"
  "7*200,4*250,350,2*300,200,180,2*170,2*160,180,200,3*220,11a"
  "6*200,5*250,200,3*300,200,180,3*170,2*180,4*220,10a"
  "7*200,5*250,4*300,200,4*170,180,200,3*220,250,9a"
  "2*250,6*200,250,2*200,4*250,2*200,5*170,180,200,3*220,250,8a"
  "300,3*250,5*200,2*250,2*200,250,4*200,180,3*170,2*190,220,250,220,2*250,7a"
  "170,300,2*250,6*200,250,4*200,250,300,200,5*170,190,200,250,2*220,2*250,6a"
  "2*170,300,200,250,7*200,4*250,3*300,180,2*170,2*180,200,250,2*220,3*250,5a"
  "2*170,2*300,2*250,7*200,4*250,2*300,180,2*170,2*180,170,200,3*220,3*250,4a"
  "3*170,2*300,2*250,5*200,3*250,230,2*250,2*300,180,2*170,2*190,200,3*220,4*250,3a"
  "3*170,2*300,170,250,5*200,4*250,230,250,2*300,3*170,190,2*200,3*220,5*250,2a"
  "4*170,2*300,200,250,6*200,2*250,230,250,200,300,3*170,190,2*200,4*220,5*250,a"
  "5*170,300,170,2*250,2*200,170,2*200,4*250,200,300,4*170,2*200,3*220,7*250,"
  "a4*170,2*300,3*250,5*200,4*250,5*170,2*200,3*220,7*250,"
  "2a3*170,3*300,3*250,4*200,3*250,200,300,4*170,2*200,3*220,7*250,"
  "3a3*170,4*300,2*250,4*200,2*250,200,2*300,3*170,3*200,2*220,7*250,"
  "4a2*170,5*300,2*250,3*200,2*250,200,3*170,200,2*170,3*200,220,7*250,"
  "5a2*170,4*300,3*250,3*200,250,2*300,2*170,300,200,170,3*200,2*220,6*250,"
  "6a2*170,4*300,3*250,2*200,2*250,4*170,250,3*170,2*200,220,6*250,"
  "7a2*170,4*300,3*250,2*200,2*250,300,2*170,300,250,3*170,200,2*220,5*250,"
  "8a2*170,4*300,3*250,2*200,250,300,2*170,300,250,3*170,2*200,220,5*250,"
  "9a2*170,4*300,3*250,2*200,250,200,2*170,300,250,3*170,200,2*220,4*250,"
  "10a2*170,4*300,3*250,200,250,200,2*170,2*250,3*170,2*200,220,4*250,"
  "11a2*170,4*300,6*200,2*170,300,250,2*170,2*200,2*220,3*250,"
  "12a6*170,5*200,2*170,2*250,3*170,200,3*220,2*250,"
  "13a5*170,5*200,3*170,2*250,4*170,3*220,250,"
  ))

(inf-mot 13 14 1 (n "1/10") (sym 100500101) (m 18))
(inf-mot 14 12 1 (n "1/11") (sym 100500111) (m 18))
(inf-mot 12 16 1 (n "1/2") (sym 100500021) (m 18))
(inf-mot 8 20 1 (n "1/357") (sym 109003571) (m 18))
(inf-mot 5 22 1 (n "1/358") (sym 109003581) (m 18))
(inf-mot 3 25 1 (n "1/359") (sym 109003591) (m 18))
(inf-mot 12 14 1 (n "2/10") (sym 100500102) (m 18))
(inf-mot 13 12 1 (n "2/11") (sym 100500112) (m 18))
(inf-mot 11 16 1 (n "2/2") (sym 100500022) (m 18))
(inf-mot 7 21 1 (n "2/357") (sym 109003572) (m 18))
(inf-mot 5 23 1 (n "2/358") (sym 109003582) (m 18))
(inf-mot 3 24 1 (n "2/359") (sym 109003592) (m 18))
(inf-mot 11 14 1 (n "3/10") (sym 100500103) (m 18))
(inf-mot 12 12 1 (n "3/11") (sym 100500113) (m 18))
(inf-mot 10 16 1 (n "3/2") (sym 100500023) (m 18))
(inf-mot 7 22 1 (n "3/357") (sym 109003573) (m 18))
(inf-mot 5 24 1 (n "3/358") (sym 109003583) (m 18))
(inf-mot 2 25 1 (n "3/359") (sym 109003593) (m 18))
(inf-armored 8 13 1 (n "23") (sym 120700230) (m 18))
(inf-armored 6 14 1 (n "38") (sym 120700380) (m 18))
(inf-armored 7 14 1 (n "48") (sym 120700480) (m 18))
(cav-mech 8 23 1 (sym 109000000) (m 18))
(arty-towed 10 14 1 (sym 100500190) (m 18))
(arty-towed 10 15 1 (sym 100500210) (m 18))
(arty-towed 11 13 1 (sym 100500460) (m 18))
(arty-towed 11 12 1 (sym 100500500) (m 18))
(arty-towed 3 22 1 (sym 109003430) (m 18))
(arty-towed 3 21 1 (sym 109003440) (m 18))
(arty-towed 3 20 1 (sym 109003450) (m 18))
(arty-towed 3 22 1 (sym 109009150) (m 18))
(arty-armored 7 15 1 (n "434") (sym 120704340) (m 18))
(arty-armored 7 16 1 (n "440") (sym 120704400) (m 18))
(arty-armored 6 16 1 (n "489") (sym 120704890) (m 18))
(tank 6 17 1 (n "17") (sym 120700170) (m 18))
(tank 9 12 1 (n "31") (sym 120700310) (m 18))
(tank 6 15 1 (n "40") (sym 120700400) (m 18))
(hq-inf-div 10 13 1 (sym 100500000) (m 180))
(hq-inf-div 2 23 1 (sym 109000000) (m 180))
(hq-armor-div 5 16 1 (sym 120700000) (m 180))
(hq-inf-corps 3 18 1 (n "XX") (m 180))
(supply-depot 1 14 1)
(village 3 19 1 (n "Etain"))

(inf 15 20 2 (m 18))
(inf 16 20 2 (m 18))
(inf 13 24 2 (m 18))
(inf 14 23 2 (m 18))
(inf 16 18 2 (m 18))
(inf 17 17 2 (m 18))
(inf 18 15 2 (m 18))
(inf 18 14 2 (m 18))
(inf 18 11 2 (m 18))
(inf 19 12 2 (m 18))
(inf 22 10 2 (m 18))
(inf 22 9 2 (m 18))
(inf 23 7 2 (m 18))
(inf 23 6 2 (m 18))
(inf 23 11 2 (m 18))
(inf 23 12 2 (m 18))
(inf 23 13 2 (m 18))
(inf 22 14 2 (m 18))
(inf 22 12 2 (m 18))
(hq-inf-div 21 14 2 (n "462") (m 180))
(hq-gren-div 22 11 2 (n "17 SS") (m 180))
(hq-gren-div 23 8 2 (n "3") (m 180))
(hq-gren-div 16 22 2 (n "559 VG") (m 180))
(hq-inf-corps 24 12 2 (n "XIII SS") (m 180))
(fort 20 15 2 (n "Deroulede") (m 18) (os 1))
(fort 20 10 2 (n "Driant") (m 18) (os 1))
(fort 19 13 2 (n "Jeanne d'Arc") (m 18) (os 1))
(fort 20 14 2 (n "Plappeville") (m 18) (os 1))
(fort 23 10 2 (n "St Privat") (m 18) (os 1))
(fort 22 8 2 (n "Verdun") (m 18) (os 1))
(village 12 20 2 (n "Briey") (os 1))
(village 9 18 2 (n "Conflans") (os 1))
(village 12 13 2 (n "Mars-la-Tour") (os 1))
(village 21 5 2 (n "Pagny") (os 1))
(village 18 23 2 (n "Thionville") (os 1))
(town 22 13 2 (n "Metz") (m 180) (os 1))

(arty-rr 23 14 2)

(arty-cd-open (in "Jeanne d'Arc") (s 2))
(arty-cd-open (in "Driant") (s 2))

#|
;;; Americans.

(hq-inf-corps 5 15 1 (n "XX"))

(unit-defaults (s 1))

(unit-defaults (@ 10 14))

(hq-inf-div -1 0 (sym 100500000))
(inf-mot 0 0 (sym 100500021) (n "1/2"))
(inf-mot 0 0 (sym 100500022) (n "2/2"))
(inf-mot 1 0 (sym 100500023) (n "3/2"))
(inf-mot 1 0 (sym 100500101) (n "1/10"))
(inf-mot 5 0 (sym 100500102) (n "2/10"))
(inf-mot 2 0 (sym 100500103) (n "3/10"))
(inf-mot 2 0 (sym 100500111) (n "1/11"))
(inf-mot 3 0 (sym 100500112) (n "2/11"))
(inf-mot 3 0 (sym 100500113) (n "3/11"))
(arty-towed 4 0 (sym 100500190))
(arty-towed 4 0 (sym 100500210))
(arty-towed 4 0 (sym 100500460))
(arty-towed 4 0 (sym 100500500))

(unit-defaults (@ 3 14))

(hq-inf-div -1 0 (sym 109000000))
(inf-mot 0 0 (sym 109003571) (n "1/357"))
(inf-mot 0 0 (sym 109003572) (n "2/357"))
(inf-mot 1 0 (sym 109003573) (n "3/357"))
(inf-mot 1 0 (sym 109003581) (n "1/358"))
(inf-mot 2 0 (sym 109003582) (n "2/358"))
(inf-mot 2 0 (sym 109003583) (n "3/358"))
(inf-mot 3 0 (sym 109003591) (n "1/359"))
(inf-mot 3 0 (sym 109003592) (n "2/359"))
(inf-mot 4 0 (sym 109003593) (n "3/359"))
(cav-mech 5 0 (sym 109000000))
(arty-towed 6 0 (sym 109003430))
(arty-towed 6 0 (sym 109003440))
(arty-towed 6 0 (sym 109003450))
(arty-towed 6 0 (sym 109009150))

(unit-defaults (@ 10 20))

(hq-armor-div 0 0 (sym 120700000))
(inf-armored 0 0 (sym 120700230) (n "23"))
(inf-armored 1 0 (sym 120700380) (n "38"))
(inf-armored 1 0 (sym 120700480) (n "48"))
(arty-armored 2 0 (sym 120704340) (n "434"))
(arty-armored 2 0 (sym 120704400) (n "440"))
(arty-armored 3 0 (sym 120704890) (n "489"))
(tank 3 0 (sym 120700170) (n "17"))
(tank 4 0 (sym 120700310) (n "31"))
(tank 4 0 (sym 120700400) (n "40"))

(unit-defaults (@ 0 0))

;;; Germany.

(hq-inf-corps 32 14 2 (n "XIII SS"))

|#
