(game-module "coral-sea"
  (title "Coral Sea")
  (version "1.0")
  (blurb "Battle in the Coral Sea in May 1942.")
  (instructions "The Japanese must attempt to capture Port Moresby.")
  (base-module "ww2-div-pac")
  (variants
   (world-seen true)
   (see-all true)
   (sequential false)
   )
  )

(set synthesis-methods nil)

(set initial-date "1 May 1942")

(scorekeeper (do last-side-wins))

(set sides-min 3)
(set sides-max 3)

(side 3 (active false))

;; Real goal is Port Moresby...

(add u* point-value 0)
(add port point-value 10)
(add (cl ca) point-value 1)
(add (cvl cvs) point-value 2)
(add cv point-value 5)

;  The feature area layer is just used to position feature names on this map.
;  Borders will therefore appear strange if drawn.
(set feature-boundaries false)

(world 800)

(area 130 90)

(area (terrain
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "38ac91a"
  "130a"
  "60abab67a"
  "58aca2b68a"
  "57ab72a"
  "130a"
  "56ab73a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "96ac33a"
  "73ab22a2c32a"
  "67a2b8ab52a"
  "72acab55a"
  "67ab5acba2b52a"
  "7a4d62ab56a"
  "5a7d2a2d114a"
  "5a8d33ad83a"
  "7a7d4a6d106a"
  "4a2d3a6d3a10d14adb5ad80a"
  "11a4d2a14d11ab8a2d77a"
  "3a3d4a10d6f8d19a2d75a"
  "3a4d5a11d6f7d19ad74a"
  "19a8d4f7d15a2dad73a"
  "22a7d3f9d13a2dad72a"
  "12ad2a2d6ab8d2f9d2a4b5a2d74a"
  "16ad6a3b7d5f7dabda7d7ad66a"
  "16ad11a6d7f5d2ab2a4d9a2d64a"
  "28aba6d10f3d16a2d51ac11a"
  "28aba8d8f2d18a2dad48a2c10a"
  "29a16d3fd21a2da2d43a2c10a"
  "27aba18d2f2d24a2dad40ac10a"
  "10aba2bdab12ab3a9d3a3d3fd18ada2d5ad50a"
  "14ad2ab11ab5a6db5a4df2d20ad2a2dad49a"
  "11ab3ab25a2bab4a3d2f2d2a2b2ad60a2c5a"
  "11ab2ab26a3bab5a3d2fd3ad21a2d40ac5a"
  "41ad3b9a6d2ad67a"
  "42a3b13a4d68a"
  "16ad26ad2bab14abdb15ac49a"
  "17a2da3d20a2d2b18ab64a"
  "20a3d4a2d14a2d3b82a"
  "18a15d10a3d3b81a"
  "18ac14d10a4d4b79a"
  "18ac2d3c8d12a4d4b78a"
  "18ac2d4c7dad11a5d3b77a"
  "18a3d5c6d13ad2c3d3b76a"
  "19ad7c6d12a2d2c3d3b75a"
  "20a9c5d12ad4c2da2b48ac25a"
  "21a10c4d11ad5cda2b74a"
  "22a10c5d9a2d4c2da2b10ab35ad26a"
  "23a11c5d8a6cda3b45a2d25a"
  "24a12c4d7a7cda3b6ab38a2d2ac21a"
  "25a17c5a8cda4b69a"
  "26a17c4a9cda4b45acac20a"
  "27a30cd2a4b66a"
  "28a30cdb2a5b43ac19a"
  "29a33c2a5b61a"
  "30a33c3a4b42ac17a"
  "31a34c2a6b57a"
  "32a35c2a6b27ac5ac5ac15a"
  "33a35c2a7b26a2c5ac19a"
  "34a35c2a7b27a2c23a"
  "35a36ca7b28a2c21a"
  "36a36c2ab2a2b51a"
  "37a37c4a2b50a"
  "38a38ca4b49a"
  "39a38ca3b49a"
  "40a38c2ab49a"
  "41a39c3ab46a"
  "42a39ca2b46a"
  "43a39c48a"
  "44a39c47a"
  "130a"
))

(area (features (
   ( 1 "continent" "Australia")
   ( 2 "island" "Bougainville")
   ( 5 "island" "Choiseul")
   ( 6 "island" "Christmas")
   ( 7 "island" "Viti Levu")
   ( 8 "island" "Vanua Levu")
   ( 9 "island" "Guadalcanal")
   ( 10 "island" "New Caledonia")
   ( 11 "island" "New Guinea")
   ( 12 "island" "Samoa")
   ( 13 "island" "Santa Catalina")
   ( 23 "island" "New Britain")
   ( 24 "island" "New Ireland")
   ( 25 "island" "Santa Isabel")
   ( 26 "island" "Malaita")
   ( 27 "island" "San Cristobal")
   ( 28 "island" "New Georgia")
   ( 29 "island" "Espiritu Santo")
   ( 30 "island" "Efate")
   ( 31 "island" "Ponape")
   ( 32 "island" "Malekula")
   ( 33 "island" "Trobriand")
   ( 34 "island" "Fergusson")
   ( 35 "island" "Tagula")
   ( 36 "island" "Rennell")
  )
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "130a"
  "7a4l119a"
  "7a5l118a"
  "7a6l117a"
  "8a6l4a6l106a"
  "9a6l3a10l102a"
  "10a5l2a14l20a2y77a"
  "14a20l19a2y75a"
  "15a21l19ay74a"
  "19a19l15a2xay73a"
  "22a19l13a2xay72a"
  "24a19l11a2x74a"
  "25a20l4a7x7ac66a"
  "28a18l5a4x9a2c64a"
  "29a20l16a2c51ah11a"
  "29a19l18a2caf48a2h10a"
  "28a21l21a2fa2z43a2h10a"
  "28a23l24a2za{40ah10a"
  "33a9l3a7l20a2}5a{50a"
  "35a6l6a7l20a}2a2ja{49a"
  "49a7l67a2i5a"
  "51a6l25a2|40ai5a"
  "54a6l70a"
  "58a4l68a"
  "16ab26ab86a"
  "16a2b2a3b20a2b85a"
  "16a3ba3b4a2b14a2b85a"
  "18a12b13a3b84a"
  "18a14b11a4b83a"
  "18a14b12a4b82a"
  "18a15b12a5b80a"
  "18a16b11a6b79a"
  "20a14b11a7b78a"
  "20a15b11a7b77a"
  "21a15b10a7b77a"
  "22a16b7a9b49a~26a"
  "23a15b7a9b49a2~25a"
  "24a15b6a10b49a2~24a"
  "25a18b4a9b74a"
  "26a17b4a10b73a"
  "27a31b72a"
  "28a31b51a:19a"
  "29a33b68a"
  "30a33b67a"
  "31a34b65a"
  "32a35b35ak27a"
  "33a35b35a2k25a"
  "34a35b36a2k23a"
  "35a36b36a2k21a"
  "36a36b58a"
  "37a37b56a"
  "38a38b54a"
  "39a38b53a"
  "40a38b52a"
  "41a39b50a"
  "42a39b49a"
  "43a39b48a"
  "44a39b47a"
  "130a"
))

(a-inf-div 50 38 1)
(dd 106 10 1)
(ca 63 15 1 (n "Kent"))
(ca 107 9 1 (n "New Orleans"))
(ca 63 15 1 (n "Northampton"))
(ca 107 9 1 (n "Portland"))
(cv 108 9 1 (n "Lexington"))
(cv 108 9 1 (n "Yorktown"))
(airfield 41 36 1 (n "Thursday Island"))
(airfield 51 37 1)
(port 55 20 1 (n "Cairns"))
(port 51 37 1 (n "Port Moresby") (point-value 50))
(town 107 10 1 (n "Noumea"))
(town 63 14 1 (n "Townsville"))

(j-inf-div 53 49 2)
(aa 52 49 2)
(dd 39 75 2)
(cl 54 50 2)
(cl 54 50 2)
(ca 52 50 2 (n "Aoba"))
(ca 52 50 2 (n "Furutaka"))
(ca 37 75 2)
(cvl 53 50 2 (n "Shoho"))
(cvs 67 42 2 (n "Chitose"))
(cvs 53 50 2)
(cv 39 74 2 (n "Shokaku"))
(cv 38 74 2 (n "Zuikaku"))
(airfield 51 52 2 (n "Kavieng"))
(airfield 47 43 2 (n "Lae"))
(port 38 75 2 (n "Truk"))
(town 54 49 2 (n "Rabaul"))
