(game-module "ww2-eur-42"
  (title "WWII Europe 1942")
  (version "1.1")
  (blurb "Europe in January 1942. Strategic level game.")
  (base-module "ww2-adv")
  (variants
   (world-seen true)
   (see-all true)
   (sequential false)
   ("Neutral Spain" neutral-spain
    "Spain is neutral and its territory cannot be violated."
    (true
     (area add 34 19 6 6 terrain neutral)
     (area add 30 24 8 2 terrain neutral)
     (area add 40 23 2 2 terrain neutral)
     (area add 33 21 1 3 terrain neutral)
     (area add 40 22 terrain neutral)
     (unit "Cordoba" (hp 0))
     (unit "Las Palmas" (hp 0))
     (unit "Tangiers" (hp 0))
     (unit "Barcelona" (hp 0))
     (unit "Bilbao" (hp 0))
     (unit "Valencia" (hp 0))
     (unit "Zaragoza" (hp 0))
     (unit "Madrid" (hp 0))
     ))
   ("Neutral Turkey" neutral-turkey
    "Turkey is neutral and its territory cannot be violated."
    (true
     (area add 68 19 17 4 terrain neutral)
     (area add 82 18  5 2 terrain neutral)
     (area add 71 23  4 1 terrain neutral)
     (area add 65 23  3 1 terrain neutral)
     (area add 74 18  2 1 terrain neutral)
     (area add 85 20  1 2 terrain neutral)
     (area add 66 24 terrain neutral)
     (area add 66 22 terrain neutral)
     (area add 67 20 terrain neutral)
     (area add 70 18 terrain neutral)
     (area add 72 18 terrain neutral)
     (area add 78 18 terrain neutral)
     (unit "Ankara" (hp 0))
     (unit "Erzurum" (hp 0))
     (unit "Izmir" (hp 0))
     (unit "Jerevan" (hp 0))
     (unit "Adana" (hp 0))
     (unit "Istanbul" (hp 0))
     ))
   )
  )

;; No random syntheses needed (except for unit numbering/naming).

(set synthesis-methods '(name-units-randomly))

;;; Terrain.

(include "eur-100km")

;;; Set the key points controlling temperature.

(area (temperature-year-cycle (
  ((37 30) (0   0) (1   0) (3 10) (7 20) (11  5))  ; Paris
  ((47 34) (0   0) (1   0) (3  8) (7 18) (11  1))  ; Berlin
  ((52 23) (0   5) (1   8) (3 15) (7 25) (11 10))  ; Rome
  ((58 42) (0 -10) (2  -4) (5 15) (7 17) (9 6) (11 -5))  ; Leningrad
  ((67 38) (0 -15) (2  -4) (5  8) (7 17) (9 6) (11 -7))  ; Moscow
  ((65 32) (0  -7) (2  -1) (5 15) (7 20) (9 10) (11 -4))  ; Kiev
  ((77 31) (0  -5) (1  -5) (5 20) (7 25) (11  0))  ; Stalingrad
  ((36 22) (0   5) (1   5) (5 15) (7 25) (11  5))  ; Madrid
  ((40 42) (0   0)                (7 20) (11  0))  ; Stockholm
  ((67 23) (0   5)         (5 20) (7 24) (9 16) (11  8))  ; Istanbul
  ((66 23) (0   5)         (5 20) (7 24) (9 16) (11  8))  ; 
  ((83 23) (0   3) (2   5) (5 24) (7 23) (9 14) (11  0))  ; Tbilisi
  ((35 15) (0  12)         (5 21) (7 23) (11 14))  ; Casablanca
  ((52 18) (0  10)         (5 23) (7 27) (11 11))  ; Tunis
  ((60 1)  (0  20)                (7 40) (11 20))  ; Sahara
  ((87 5)  (0  20)                (7 40) (11 20))  ; Arabia
  )))

(set temperature-moderation-range 1)

;; Can't put this in eur-100km, since these types are not in standard game.

(area (aux-terrain beach
  "90a"
  "90a"
  "36agq2ai5acy6ai35a"
  "36acL2ady5ab6adq34a"
  "41ab48a"
  "45ae44a"
  "45acL43a"
  "52aoq36a"
  "40aei2ai7ab<36a"
  "34ai6a=2ab45a"
  "34adq54a"
  "25agq4ae3mei53a"
  "26a<5aAE=JM53a"
  "31aeoO}b<53a"
  "32aE<b<54a"
  "32adq56a"
  "71aeie16a"
  "34acy36aKsT15a"
  "4agq29ab17aei17ab<b15a"
  "4ae<36am8ac}a?q22ai11a"
  "4acL24ai11ab<4akq2ab<19aei3adq10a"
  "30adq8ae7ab25a?}14a"
  "5acy33acL13aie6aoqaoq7ab<13a"
  "6ahq42ai4adsLac}2ab<ab<22a"
  "6ae<42ab9ab<28a"
  "5amkT31aoq26acyaei18a"
  "5ab=dq30ab<eiam21aeib2a?q17a"
  "41acMab<6ae4agq8aKq6aeacy11a"
  "34ae17acL4a<8ab<7aJqdq10a"
  "34agL40ab<12a"
  "35a<42ae11a"
  "76aeicL10a"
  "62a2m13a=12a"
  "62ab=<25a"
  "90a"
  "90a"
  "90a"
  "90a"
  "90a"
  "90a"
  "90a"
  "90a"
  "90a"
  "90a"
  "90a"
  ))

;; Switzerland.

(area add 44 28 3 2 terrain neutral)

;; Sweden.

(area add 41 41 6 4 terrain neutral)
(area add 43 39 5 2 terrain neutral)
(area add 44 38 2 1 terrain neutral)
(area add 47 43 terrain neutral)
(area add 47 41 terrain neutral)
(area add 42 40 terrain neutral)
(area add 50 40 terrain neutral)

;; Ireland.

(area add 24 34 3 2 terrain neutral)
(area add 24 36 terrain neutral)
(area add 25 33 terrain neutral)

(set sides-min 10)
(set sides-max 10)

;;; All the participants and major neutrals.
;;; (should include all the "minor powers" eventually)
;;; (should edit out the uninvolved sides Japan & China,
;;; but would need to modify people/control layers)

(side 1 uk (name "UK") (noun "Brit") (adjective "British")
  (emblem-name "flag-uk"))
(side 2 fr (name "France") (adjective "French")
  (emblem-name "flag-france"))
(side 3 us (name "USA") (adjective "American")
  (emblem-name "flag-usa"))
(side 4 de (name "Germany") (adjective "German")
  (emblem-name "flag-swastika"))
(side 5 it (name "Italy") (adjective "Italian")
  (emblem-name "flag-italy"))
(side 6 jp (name "Japan") (adjective "Japanese")
  (emblem-name "flag-japan"))
(side 7 su (name "USSR") (adjective "Soviet")
  (emblem-name "flag-ussr"))
(side 8 zh (name "China") (adjective "Chinese")
  (emblem-name "flag-china-old"))
(side 9 es (name "Spain") (noun "Spaniard") (adjective "Spanish")
  (emblem-name "flag-spain"))
(side 10 tr (name "Turkey") (noun "Turk") (adjective "Turkish")
  (emblem-name "flag-turkey"))

(area (people-sides
  "a67X22a"
  "26Xb10X11a10XhX9h21a"
  "25Xb11X10a7X3a2h2X9h20a"
  "25X3b9X3aX7a7X2a14h19a"
  "25X3b10X2a2X6a2Xa2XaX3a14h18a"
  "26X2b13XaX5a5XaX4a14h17a"
  "27X3b11X2aX2a7X6a15h16a"
  "24X2b2X3b11X3a8X6a16h15a"
  "24Xa2b2X4b9X2a2Xe2X2eX2e6a16h14a"
  "24X3a2X5b5Xa13e7a17h13a"
  "24X3a2X6b3X2a11e10a17h12a"
  "25Xa5X4b2X3a11e10a18h11a"
  "30X2b4X2c2a13e9a18h10a"
  "33XcX4c2a19e2a19h9a"
  "31X12c21e18h8a"
  "d32X11c3a18e8hX9h7a"
  "d34X9c3a5f3a11e3hXh3X8hXh6a"
  "d2Xd31X9c7fXf3a11e3Xh3X9hXh5a"
  "dX3d31X9c6f2X7a7e8X9hXh4a"
  "5d25X8j8c3X3f3X7a5e10X8hXh3a"
  "5d25X12j6XcX4f3X5a4ek12X7hXh2a"
  "5d26X2a9j7Xf2X4f2X7a3k3X4k5X7hXha"
  "6d25X2a8j8X2f3X4fX5a2XkX17k3bXh"
  "a6d24X2a7j3Xj6Xf5Xf3X3a5X18k4b"
  "2a5d11Xa13X2a6j17Xf4X3a2XkX17k4b"
  "3a4d10Xa15Xa6j17Xf4X2a2XfX19k3b"
  "4a3Xd27Xb9X8c2X3f6Xa5XkXkX2k2Xk3c5k3b"
  "5a37X11c3X2f8X2aXf8X5c7b"
  "6a30X3j14c22X2b2X3c8b"
  "7a28X19c4Xb20X2c9b"
  "8a27X20c9X3f12X11b"
  "9a26X20c3f6X5f5X2b3X11b"
  "10a19Xj4X21c6f3X6f20b"
  "11a24X20c16f11b5a3b"
  "12a23X20c16f8bXbX8a"
  "13a18Xj3X19c18f8b2X8a"
  "14a16Xj3X2j19c17f9b2X7a"
  "15a18X3j19c18f9b2X6a"
  "16a17X4j19c17f10b2X5a"
  "17a16X4j19c18f9b3X4a"
  "18a15X4j20c17f10b3X3a"
  "19a14X4j20c18f10b3X2a"
  "20a13X4j21c17f11b4X"
  "21a13X3j21c18f11b3X"
  "22a68X"
))

(area (control-sides
  "a67X22a"
  "26Xb10X4e7a10XhX9h21a"
  "25Xb11X4e6a7X4eh2X9h20a"
  "25X3b9X3eX7a7X4e12h19a"
  "25X3b10X2e2X6a2Xa2XeX8e9h18a"
  "26X2b13XeX5a5XeX9e9h17a"
  "27X3b11X2eX2a7X12e9h16a"
  "24X2b2X3b11X3e8X13e9h15a"
  "24Xa2b2X4b9X2e2Xe2X2eX14e10h14a"
  "24X3a2X5b5X28e10h13a"
  "24X3a2X6b3X30e10h12a"
  "25Xa5X4b2X32e10h11a"
  "30X2b4X33e11h10a"
  "33XeX35e11h9a"
  "31X40e11h8a"
  "d32X6e5c3a26eX9h7a"
  "d34X2e7c3a5f17eXe3X8hXh6a"
  "d2Xd31X2e7c7fXf14e3X2ehX9hXh5a"
  "dX3d31Xe8c6f2X2f12e4Xh3X9hXh4a"
  "5d25X8j8c3X3f3X2f10e10X8hXh3a"
  "5d25X12j6XeX4f3X2f7ek12X7hXh2a"
  "5d26X2a9j7Xf2X4f2X2f5e3k3X4k5X7hXha"
  "6d25X2a8j8X2f3X4fX2f3e2XkX17k3bXh"
  "a6d24X2a7j3Xj6Xf5Xf3Xf2e5X18k4b"
  "2a5d11Xa13X2a6j17Xf4X3e2XkX17k4b"
  "3a4d10Xa15Xa6j17Xf4X2e2XfX19k3b"
  "4a3Xd27Xb9X8a2X3f6Xe5XkXkX2k2Xk3a5k3b"
  "5a37X11a3X2f8X2eXf8X5a7b"
  "6a30X3j14a22X2b2X3a8b"
  "7a28X19a4Xb20X2a9b"
  "8a27X20a9X3b12X11b"
  "9a26X20a3f6X5b5X2b3X11b"
  "10a19Xj4X21a6f3X2b3f21b"
  "11a24X20a16f11b5X3b"
  "12a23X20a16f8bXb8Xb"
  "13a18Xj3X19a18f8b10X"
  "14a16Xj3X21a17f9b9X"
  "15a18X22a18f9b8X"
  "16a17X23a17f10b7X"
  "17a16X23a18f9b7X"
  "18a15X24a17f10b6X"
  "19a14X24a18f10b5X"
  "20a13X25a17f11b4X"
  "21a13X24a18f11b3X"
  "22a68X"
))

;; Sides not participating.

(side fr (active false))
(side jp (active false))
(side zh (active false))
(side es (active false))
(side tr (active false))

;; Allies.

(side uk (trusts (uk 1) (fr 1) (us 1) (su 1)))
(side us (trusts (uk 1) (fr 1) (us 1) (su 1)))
(side su (trusts (uk 1) (fr 1) (us 1) (su 1)))

(side fr (trusts (uk 1) (fr 1) (us 1) (su 1)) (controlled-by 4))

;; Axis.

(side de (trusts (de 1) (it 1)))
(side it (trusts (de 1) (it 1)))

(scorekeeper (do last-alliance-wins))

(side uk (initial-center-at 33 34 #|London|#))
(side us (initial-center-at  4 23 #|Washington|#))
(side su (initial-center-at 67 38 #|Moscow|#))
(side de (initial-center-at 47 34 #|Berlin|#))
(side it (initial-center-at 52 23 #|Rome|#))

;; (should set up agreements applying to particular dates)

(set initial-date "Jan 1942")

;; Need this to be month 0 so that the temperature list above
;; gets the right values.

(set initial-year-part 11)

(infantry 33 33 uk)
(infantry 32 33 uk)
(infantry 32 34 uk)
(infantry 65 12 uk)
(interceptor 34 33 uk)
(armor 33 33 uk)
(armor 64 12 uk)
(interceptor 32 35 uk)
(bomber 32 35 uk)
(bomber 33 34 uk (in "London"))
(convoy 34 34 uk)
(fleet 31 40 uk)
(fleet 25 29 uk)
(fleet 57 15 uk)

(infantry  1 22 us)
(convoy 4 25 us)

(infantry 37 32 de)
(infantry 37 33 de)
(infantry 31 30 de)
(infantry 72 29 de)
(infantry 71 29 de)
(infantry 71 27 de)
(infantry 68 32 de)
(infantry 67 34 de)
(infantry 65 36 de)
(infantry 64 38 de)
(infantry 62 40 de)
(infantry 61 40 de)
(infantry 60 25 de)
(armor 60 40 de)
(armor 63 39 de)
(armor 70 27 de)
(armor 72 30 de)
(armor 70 30 de)
(armor 69 31 de)
(armor 68 33 de)
(armor 66 35 de)
(armor 65 37 de)
(armor 64 11 de (n "Afrika Korps"))
(infantry 59 40 de)
(infantry 58 41 de)
(infantry 57 42 de)
(bomber 37 31 de)
(bomber 39 32 de)
(bomber 60 35 de (in "Minsk"))
(bomber 65 32 de (in "Kiev"))
(sub-fleet 10 23 de)
(sub-fleet 17 29 de)
(sub-fleet 18 40 de)
(sub-fleet 24 17 de)
(sub-fleet 34 29 de)
(sub-fleet 35 31 de)

(infantry 63 11 it)

(infantry 65 39 su)
(infantry 60 41 su)
(infantry 74 30 su)
(infantry 73 31 su)
(infantry 68 35 su)
(infantry 67 36 su)
(infantry 67 37 su)
(infantry 66 38 su)
(infantry 74 29 su)
(infantry 72 27 su)
(infantry 62 41 su)
(infantry 72 31 su)
(infantry 70 31 su)
(infantry 70 32 su)
(infantry 68 34 su)
(infantry 66 37 su)
(infantry 65 38 su)
(infantry 63 40 su)
(infantry 61 41 su)
(infantry 59 41 su)
(armor 64 39 su)
(armor 71 31 su)
(armor 69 33 su)
(armor 66 36 su)
(air-force 67 38 su (in "Moscow"))
(air-force 58 42 su (in "Leningrad"))
(air-force 73 30 su (in "Rostov"))
(convoy 57 43 su)
(fleet 70 26 su)

(town 87 20 0 (n "Tabriz"))
(town 32 20 0 (n "Lisbon"))
(town 35 15 0 (n "Casablanca"))
(town 31 23 0 (n "Porto"))

(base 64 13 uk (n "Benghazi") (os it))
(base 34 33 uk (n "Dover"))
(base 35 18 uk (n "Gibraltar"))
(base 79 14 uk (n "Haifa"))
(base 58 15 uk (n "Malta"))
(base 26 43 uk (n "Scapa Flow"))
(base 34 34 uk (n "Southend"))
(base 77 12 uk (n "Port Said"))
(base 78 11 uk (n "Suez"))
(base 68 13 uk (n "Tobruk") (os it))
(town 74 13 uk (n "Alexandria") (tp (convoy 36)))
(town 80 6 uk (n "Aswan") (tp (infantry 6)))
(town 78 8 uk (n "Asyut") (tp (interceptor 24)))
(town 25 37 uk (n "Belfast") (tp (interceptor 24)))
(town 31 34 uk (n "Bristol") (tp (asw-fleet 36)))
(town 76 10 uk (n "Cairo") (tp (armor 24)))
(town 30 34 uk (n "Cardiff") (tp (infantry 6)))
(town 28 38 uk (n "Edinburgh") (tp (infantry 6)))
(town 31 36 uk (n "Hull") (tp (convoy 36)))
(town 80 14 uk (n "Jerusalem"))
(town 30 37 uk (n "Newcastle") (tp (bomber 24)))
(city 30 35 uk (n "Birmingham") (tp (infantry 6)))
(city 27 38 uk (n "Glasgow") (tp (fleet 36)))
(city 29 36 uk (n "Manchester") (tp (armor 24)))
(capital 33 34 uk (n "London") (tp (fleet 36)))

(base 79 18 fr (n "Aleppo"))
(base 80 16 fr (n "Damascus"))
(base 36 13 fr (n "Marrakech"))
(town 45 18 fr (n "Algiers"))
(town 41 27 fr (n "Lyon"))
(town 43 25 fr (n "Marseilles"))
(town 45 25 fr (n "Nice"))
(town 42 17 fr (n "Oran"))
(town 39 25 fr (n "Toulouse"))

(town 5 20 us (n "Atlanta"))
(town 2 26 us (n "Buffalo"))
(town 6 20 us (n "Charleston"))
(town 3 21 us (n "Memphis"))
(town 5 22 us (n "Norfolk"))
(city 4 26 us (n "Boston") (tp (fleet 36)))
(city 4 19 us (n "Houston"))
(city 5 19 us (n "New Orleans") (tp (infantry 1)))
(city 4 25 us (n "New York") (tp (convoy 36)))
(city 1 24 us (n "Chicago") (tp (interceptor 24)))
(city 4 24 us (n "Philadelphia") (tp (fleet 36)))
(city 3 24 us (n "Pittsburgh") (tp (bomber 24)))
(city 2 24 us (n "Detroit") (tp (armor 12)))
(city 2 22 us (n "St. Louis") (tp (infantry 6)))
(capital 4 23 us (n "Washington") (tp (cv-fleet 36)))

(base 55 42 de (n "Tallinn") (os 0))
(town 64 20 de (n "Athens") (os 0))
(town 58 26 de (n "Belgrade") (os 0))
(town 36 27 de (n "Bordeaux") (os fr))
(town 41 35 de (n "Bremen"))
(town 38 32 de (n "Brussels") (os 0))
(town 64 26 de (n "Bucharest") (tp (infantry 6)))
(town 56 29 de (n "Budapest") (tp (infantry 6)))
(town 54 32 de (n "Cracow") (os 0))
(town 51 35 de (n "Danzig") (tp (convoy 36)))
(town 69 30 de (n "Dnepropetrovsk") (os su))
(town 42 33 de (n "Dortmund") (tp (armor 24)))
(town 41 33 de (n "Essen") (tp (interceptor 24)))
(town 44 32 de (n "Frankfurt") (tp (infantry 6)))
(town 53 36 de (n "Konigsberg") (tp (infantry 6)))
(town 58 32 de (n "L'vov") (os 0))
(town 35 31 de (n "Le Havre") (os fr))
(town 47 32 de (n "Leipzig") (tp (bomber 24)))
(town 60 35 de (n "Minsk") (os su))
(town 47 30 de (n "Munich") (tp (bomber 24)))
(town 34 29 de (n "Nantes") (os fr))
(town 66 28 de (n "Odessa") (os su) (tp (convoy 36)))
(town 40 42 de (n "Oslo") (os 0) (tp (convoy 36)))
(town 49 31 de (n "Prague") (os 0) (tp (armor 24)))
(town 55 38 de (n "Riga") (os 0))
(town 62 24 de (n "Sofia") (os 0))
(town 48 35 de (n "Stettin"))
(town 45 30 de (n "Stuttgart") (tp (infantry 6)))
(town 63 22 de (n "Thessaloniki") (os 0))
(town 52 30 de (n "Vienna") (tp (infantry 6)))
(town 58 36 de (n "Vilnius") (os 0))
(town 55 34 de (n "Warsaw") (os 0) (tp (armor 24)))
(town 53 27 de (n "Zagreb") (os 0))
(city 44 37 de (n "Copenhagen") (os 0))
(city 42 35 de (n "Hamburg") (tp (sub-fleet 36)))
(city 65 32 de (n "Kiev") (os su))
(city 38 34 de (n "Rotterdam") (os 0))
(capital 47 34 de (n "Berlin") (tp (interceptor 24)))
(capital 37 30 de (n "Paris") (os fr))

(town 50 25 it (n "Florence") (tp (armor 24)))
(town 46 26 it (n "Genoa") (tp (armor 24)))
(town 55 18 it (n "Palermo") (tp (infantry 6)))
(town 57 22 it (n "Taranto") (tp (infantry 6)))
(town 44 27 it (n "Torino") (tp (infantry 6)))
(town 51 28 it (n "Trieste") (tp (infantry 6)))
(town 60 12 it (n "Tripoli") (tp (infantry 6)))
(town 52 18 it (n "Tunis"))
(town 50 27 it (n "Venice") (tp (armor 24)))
(city 47 27 it (n "Milan") (tp (interceptor 24)))
(capital 52 23 it (n "Rome") (tp (infantry 6)))

(base 64 39 su (n "Kalinin"))
(town 81 28 su (n "Astrakhan") (tp (infantry 6)))
(town 86 23 su (n "Baku") (tp (infantry 6)))
(town 80 23 su (n "Batum") (tp (infantry 6)))
(town 67 35 su (n "Br'ansk") (tp (interceptor 24)))
(town 68 39 su (n "Jaroslavi") (tp (interceptor 24)))
(town 84 21 su (n "Jerevan"))
(town 73 37 su (n "Kazanh") (tp (infantry 6)))
(town 69 32 su (n "Kharkov") (tp (armor 24)))
(town 75 27 su (n "Maikop") (tp (infantry 6)))
(town 71 26 su (n "Sevastopol") (tp (infantry 6)))
(town 77 31 su (n "Stalingrad") (tp (armor 24)))
(town 71 30 su (n "Stalino") (tp (infantry 6)))
(town 83 23 su (n "Tbilisi") (tp (infantry 6)))
(town 72 38 su (n "Sverdlovsk") (tp (infantry 6)))
(town 70 40 su (n "Perm") (tp (infantry 6)))
(town 69 41 su (n "Nizhni Tagilsk") (tp (infantry 6)))
(town 76 33 su (n "Saratov") (tp (infantry 6)))
(town 78 32 su (n "Orenburg") (tp (infantry 6)))
(town 68 36 su (n "Tula") (tp (infantry 6)))
(town 72 35 su (n "Penza") (tp (infantry 6)))
(city 70 38 su (n "Gorky") (tp (infantry 6)))
(city 75 35 su (n "Kuybyshev") (tp (interceptor 24)))
(city 58 42 su (n "Leningrad") (tp (infantry 6)))
(city 73 30 su (n "Rostov") (tp (armor 24)))
(capital 67 38 su (n "Moscow") (tp (armor 24)))

(base 35 20 es (n "Cordoba"))
(base 30 8 es (n "Las Palmas"))
(base 36 16 es (n "Tangiers"))
(town 41 23 es (n "Barcelona"))
(town 35 25 es (n "Bilbao"))
(town 39 21 es (n "Valencia"))
(town 38 23 es (n "Zaragoza"))
(capital 36 22 es (n "Madrid"))

(town 72 21 tr (n "Ankara"))
(town 81 21 tr (n "Erzurum"))
(town 68 20 tr (n "Izmir"))
(town 76 19 tr (n "Adana"))
(city 67 23 tr (n "Istanbul"))

(game-module (instructions (
  "It is January of 1942, and the lines are clearly drawn. "
  "Germany and Italy together dominate the continent of Europe, "
  "while the British and Soviet nations are in dire straits. "
  "But a powerful new adversary has joined the fray; the United "
  "States, provoked by the Japanese attack on Pearl Harbor, has "
  "declared war on the entire Axis, and at this moment its top "
  "strategists are conferring with their British and Soviet "
  "counterparts to decide how best to defeat the Axis in Europe."
  ""
  "So the clock is ticking; can the Germans and Italians achieve "
  "victory on their Eastern Front and then invade Britain before "
  "the American war machine gets up to speed?  They will have a "
  "difficult time; the front line in Russia is very long and a "
  "successful counteroffensive can cut off a dozen units at once."
  ""
  "Ground units must stay within range of supply centers or become "
  "paralyzed for lack of oil."
  ""
  )))
