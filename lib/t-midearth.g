(game-module "t-midearth"
  (base-module "3rd-age")
  (title "Western Lands in Middle Earth")
  (blurb "Map of Western Lands in Middle Earth")
  (version "2.0.0")
)

;  The feature area layer is just used to position feature names on this map.
;  Borders will therefore appear strange if drawn.
(set feature-boundaries false)

(world 360)
(area 95 80)
(area (terrain
  (by-name
    (sea 0) (swamp 1) (semi-desert 2) (plain 3) (forest 4)
    (mountain 5) (ice 6) (road 7) (river 8) (ford 9))
  "56g39a"
  "g10a2c8a35cg38a"
  "g10a2c8a36cg37a"
  "g10a3c8a36cg36a"
  "g12a3c6a37cg35a"
  "g13a3c4a39cg34a"
  "g20a40cg33a"
  "g21a40cg32a"
  "g10a4d8a40cg31a"
  "g8a6da8d40cg30a"
  "g2ad3a4d3fd3a6d6e35cg29a"
  "g5a4d2f16d2ed35cg28a"
  "g5a2d4f19d4f32cg27a"
  "g6a3d2f22d3f4c3f24cg26a"
  "g8ad2f8d2e14d5f2d2f24cg25a"
  "g6a3d3f7d4e15d2f3dg2f23cg24a"
  "g4a6d2f8d4e14dfgf3dg2f3cf19cg23a"
  "g5a5d2f9d3e11d5fgf4d5f3d17cg22a"
  "g5a5de2f9d2e12df4egf4d3f6d16cg21a"
  "g6a4de2f9d2e5de10de2f2d5ed2c4d16cg20a"
  "g6a5de2f8d3e3d3e10de2f7ecf4df5c3f8cg19a"
  "g6a6de2f7d4e2d3e10de2f8e6d9f8cg18a"
  "g7a5de3f5d4eae14de2f9ea15d7cg17a"
  "g8a6d3f6d2e2ae14de2fd8e17d6cg16a"
  "g9a8d2f6d3e4de10d2f2d3e3f3e19d3cg15a"
  "g11ad2a8de10de9d2f3d8e20d3cg14a"
  "g19a2d2e9dedf4d3edef4d7e21d3cg13a"
  "g12a8d2fe7de10d3e2f4d7e21d3cg12a"
  "g13a7df2e7d2e11de2f5d6e22d3cg11a"
  "g13a7dfe9ded4e6d3f5d6e23d3cg10a"
  "g14a5defe21dgf3db3d5e23d4cg9a"
  "g14a5de2fe19degf4db3d4e20d5e3cg8a"
  "g15a5de2fe3f15defgf7d5e20d4e4cg7a"
  "g18a3de2fe18de3f6d3e22dea3e4cg6a"
  "g19a3defe13db5dfg2f5d5e17d6a2e5cg5a"
  "g20a3d2e2da11db3de2fd3e3d7e15d7a7cg4a"
  "g21a6da15de3f3e3d7e15d7a8cg3a"
  "g22a5da16de2fd3e3d4e17d7ad8cg2a"
  "g25a2da3d2e11de2f8d5c13d3f6ad9cga"
  "g28a3de10d3e2fe7d7c12d3f2a3dad9cg"
  "ag27a15de3f5e3d8c12d2fa6d9cg"
  "2ag26a2da12d2e2f6e4d7c10d5f6d8cg"
  "3ag28a13de3f5e4d7c21d8cg"
  "4ag28a12de3f5e6d6c20d8cg"
  "5ag27a3da3d2e4de3f10d6c20d8cg"
  "6ag30a4d2e5df12d5c19d9cg"
  "7ag29a25d2e4c10d2c2d12cg"
  "8ag28a23d3fe5c7d18cg"
  "9ag27a12d2f9dfa2f3b3c2d22cg"
  "10ag27a10d2e3f8d2f4b2cf10c7f7cg"
  "11ag28a6d4e5f9d2b2dfc18f6cg"
  "12ag26a5d3e5fg2f10db2d2f3cf2d2c4f12cg"
  "13ag26a5d12f3de8df2cf2cd3c2f13cg"
  "14ag25a8d2e8fg2fe2de5df5c2dc2f13cg"
  "15ag25a3df4d2f4d6fg4f4d2f4c3d2fd12cg"
  "16ag25ad2f11df4d3fgfg4df3c4d2f2d11cg"
  "17ag24ae2fe16df5ded2f2c4d3f6d7cg"
  "18ag23ae2fe4de17ded4f15d5cg"
  "19ag22aef3e7d3a2d2f10df12da4df4cg"
  "20ag21a3e6d6a2df11df9d5a3d2f3cg"
  "21ag20a2e5dad7adf12df8d5a3d2f3cg"
  "22ag18a4d3ad10adf5d3a4df15df4cg"
  "23ag36a2d5a6df15df4cg"
  "24ag36adada9df7d8f4cg"
  "25ag37afa10d9f2d3c3d2cg"
  "26ag38a10d4f2d3c9dcg"
  "27ag38a11d2c6d3c6dg"
  "28ag38a11dc5d8c2dg"
  "29ag37a15d12cg"
  "30ag37a15d11cg"
  "31ag37a14d11cg"
  "32ag36a14d11cg"
  "33ag34a17d9cg"
  "34ag33a18d8cg"
  "35ag28a11d5c7d7cg"
  "36ag27a8d9c9d4cg"
  "37ag26a2d3a4d9c10d2cg"
  "38ag30a4d10c11dg"
  "39ag23a10d11c10dg"
  "40a55g"
))
(area (aux-terrain road
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "30ai64a"
  "30af64a"
  "31a@63a"
  "32aD13ai48a"
  "28ak3sv10ak2sr48a"
  "27akr4aF7sqcr51a"
  "25agsr5aj61a"
  "26a@6af61a"
  "27a>u5a@60a"
  "29a>2sq2aD59a"
  "33acsv59a"
  "36a@58a"
  "37a@57a"
  "38a@56a"
  "39a@55a"
  "40aD54a"
  "40af54a"
  "41a<53a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "68aku25a"
  "54ae12akva>sq22a"
  "55a@11aja@25a"
  "56a>3su6af2a>suai20a"
  "61a>2su3a@4a>r20a"
  "65a@3aD25a"
  "66a@2aj25a"
  "67aDc|q24a"
  "67ajaj25a"
  "67ajaj25a"
  "67afaf25a"
  "68aDaD24a"
  "63ac4sraj24a"
  "70aj24a"
  "70af24a"
  "71aD23a"
  "71af23a"
  "72a@22a"
  "73a@21a"
  "74a@20a"
  "75a>su17a"
  "78a@16a"
  "79a@15a"
  "80a@14a"
  "81a@13a"
  "82a@12a"
  "83a>u10a"
  "85a@9a"
  "86a>u7a"
  "88a>u5a"
  "90a@4a"
  "91a<3a"
))
(area (aux-terrain river
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "17acy76a"
  "18ahq75a"
  "18acT24agq49a"
  "19ahq23acT49a"
  "19agL24ahq48a"
  "19agL18ae2mi2agL6acq40a"
  "19agL18agM2=2agT14ac}i31a"
  "19agL7ai10agL4acOq6acy6ab?y30a"
  "19agL7ady8aeoL5agq7ady7ahq29a"
  "19agL8ahq7agM<e2m2acT8ady6acT29a"
  "20a<8agL7agLagM=<2ady8ady6ahq28a"
  "29acT7agLaeHmi3ahq8ady5agL28a"
  "30ady6acTagM2=3acT9ad}4moL28a"
  "31ady6ahqgL6ahq9ab4=?T28a"
  "32ahq5agXoL6acT15ady27a"
  "32acL4aeoM=<7ahq15ady26a"
  "32agq4agM<9agL16ady2ae2mi19a"
  "32agL4agL10agL17ad}moM=?q18a"
  "28ae3moL3aeoT10agL18ab2=<21a"
  "29a4=<2aeoM?}3mi5agL43a"
  "36agM<ab4=5acT43a"
  "35aeoL13ady42a"
  "35agM<14ad}mi39a"
  "35agL16ab=Cq38a"
  "35agL18acXi37a"
  "35agL19abCq36a"
  "35agL20acXi35a"
  "35acL21abCq34a"
  "59agL34a"
  "48ae10agL34a"
  "48ae<9agL34a"
  "45ae2moL9acT34a"
  "39ae5moM2=<10abai32a"
  "39agM5=<15ady31a"
  "40a<22ady30a"
  "64ady29a"
  "65ad}28a"
  "66abJq26a"
  "47aei18ahL26a"
  "47agM18ae<26a"
  "47agL18agL26a"
  "47acT18acT26a"
  "48ahq18ahq25a"
  "48acL18agL25a"
  "68agL25a"
  "69a<25a"
  "95a"
  "95a"
  "95a"
  "78agq15a"
  "78agL15a"
  "78agL15a"
  "78agL15a"
  "68ae9moL15a"
  "69a10=<15a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
))
(area (aux-terrain ford
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "46am48a"
  "46ab<47a"
  "95a"
  "95a"
  "41acq52a"
  "95a"
  "95a"
  "95a"
  "32ae62a"
  "33a<61a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "48acq45a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "67ai27a"
  "66acr27a"
  "95a"
  "67acq26a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
  "95a"
))
(area (features (
   (1 "bridge" "Brandywine %T")
   (2 "bridge" "Harad %T")
   (3 "bridge" "Tharbad %T")
   (4 "bridge" "The Last %T")
   (5 "bridge" "???")
   (6 "island" "Carrock")
   (7 "ford" "%T of Bruinen")
   (8 "ford" "Sarn %T")
   (9 "island" "Himling")
   (10 "island" "Tolfalas")
   (11 "island" "Cair Andros")
   (12 "river" "Lhun")
   (13 "river" "Baranduin")
   (14 "river" "Gwathlo")
   (15 "river" "Bruinen")
   (16 "river" "Glanduin")
   (17 "river" "Isen")
   (18 "river" "Anduin")
   (19 "river" "Anduin")
   (20 "river" "Celduin")
   (21 "river" "Carnen")
   (22 "river" "Harnen")
   (23 "gulf" "Icebay of Forchet")
   (24 "gulf" "Gulf of Lhun")
   (25 "gulf" "Havens of Umbar")
   (26 "mountains" "Ered Luin")
   (27 "mountains" "Ered Luin")
   (28 "river" "Isen")
   (29 "mountains" "Hithaeglir")
   (30 "mountains" "Ered Mithrin")
   (31 "mountains" "Ered Nimrais")
   (32 "mountains" "Iron Hills")
   (33 "mountains" "Ered Lithui")
   (34 "mountains" "Ephel Duath")
   (35 "hills" "Emyn Uial")
   (36 "hills" "North Downs")
   (37 "hills" "South Downs")
   (38 "hills" "Weather %T")
   (39 "hills" "North Downs")
   (40 "hills" "Ettenmoors")
   (41 "hills" "Emyn Muil")
   (42 "hills" "Pinnath Gelin")
   (43 "hills" "Emyn Arnen")
   (44 "forest" "Old %T")
   (45 "forest" "Mirkwood")
   (46 "forest" "Lorien")
   (47 "forest" "Fanghorn")
   (48 "forest" "Trollshaws")
   (49 "lake" "Nenuial")
   (50 "ford" "%T of Isen")
   (51 "lake" "Sea of Rhun")
   (52 "lake" "Sea of Nurnen")
   (53 "battlefield" "Galadden Fields")
   (54 "battlefield" "Field of Celebrant")
   (55 "battlefield" "Dagorlad")
   (56 "swamp" "Nin in Eilph")
   (57 "swamp" "Dead Marches")
   (58 "swamp" "Nindalph")
   (59 "region" "Forlindon")
   (60 "river" "Gwathlo")
   (61 "region" "Harlindon")
   (62 "region" "The Shire")
   (63 "region" "Minhwraith")
   (64 "region" "Northern Waste")
   (65 "region" "Forodwaith")
   (66 "region" "Angmar")
   (67 "region" "Withered Heath")
   (68 "region" "East Blight")
   (69 "region" "The Wold")
   (70 "region" "The Brown Lands")
   (71 "region" "Enedwaith")
   (72 "region" "Druwaith Iaur")
   (73 "region" "Dunland")
   (74 "region" "Eregion")
   (75 "region" "Rohan")
   (76 "region" "Anorien")
   (77 "region" "North Ithilien")
   (78 "region" "South Ithilien")
   (79 "region" "Gorgoroth")
   (80 "region" "Mordor")
   (81 "region" "Andrast")
   (82 "region" "Anfalas")
   (83 "region" "Belfalas")
   (84 "region" "Gondor")
   (85 "region" "South Gondor")
   (86 "region" "Nurn")
   (87 "region" "Near Harad")
   (88 "region" "Lebennin")
   (89 "river" "Mithelthel")
  )
  "95a"
  "27a15*64,53a"
  "27a16*64,52a"
  "27a17*64,51a"
  "28a16*64,51a"
  "29a15*64,51a"
  "95a"
  "95a"
  "38a5*65,52a"
  "15ax22a7*65,50a"
  "3aj7a3{a3x14a4*66,2a9*65,48a"
  "10a2{20a6*66,a9*65,47a"
  "8a4{19a4~4*66,5a5*65,a5*67,40a"
  "10a2{22a3~3*66,a3:a4*65,a6*67,39a"
  "10a2{8a2?14a5~2a2:2a2*65,2a5*67,39a"
  "7a3W3{7a4?15a2~3a3:48a"
  "5a6W2{8a4?14a3~3a3:3a:43a"
  "6a5W2{9a3?11a4D3~4a5:43a"
  "6a5Wa2{9a2?12a5D2~4a3:44a"
  "7a4Wa2{5am3a2?16aD2~2a5I44a"
  "7a5Wa2{4am3a3?3a3@11a2~a6I6a<5a3<28a"
  "7a6Wa2{7a3?3a3@11a2~a7I6a9<27a"
  "8a5Wa3{6a3?M16a2~a8I41a"
  "9a5Wa3{6a2?2M11a89,4a2~g8I8av31a"
  "10a4W4a2{13aB5a89,4a2~2a8I8av30a"
  "12aW2y9a5Z5aB7a2p2~3a8I38a"
  "13a7y4a5Z2b4aB4a2L3a~4a7I38a"
  "13a5Y3a2|a7ZH6a2ea2h2a2~4a7I37a"
  "14a5Y2a|2a7Z2H12a2~5a6I2a2u33a"
  "14a6Ya|3a7ZHa4A6a3~5a6I37a"
  "15a5Ya|4a7Z11a2~3aQ3a5I37a"
  "15a5Ya2|6a3Zi9a2*74,2~4aQ3a4I37a"
  "16a5Ya2|9ai8a2*74,3~7a5I36a"
  "19a3Ya2|7an10a2*74,3~3as2a3I3*68,20aO14a"
  "20a3Ya|7an4adaT3a2*74,4~2as2a5I68,16a6O13a"
  "21a3Y5a4[5adaT4a2~a3J3a7I15a7O12a"
  "22a2Y5a5[2aX4a2q2a3~3J3a7I15a7O12a"
  "23aY5a6[aX9a2~a3J3a4I17a7O12a"
  "29a7[10a2~8a5*70,16a6O12a"
  "29a7[5a2*73,3a2~K4a3R7*70,15a2O3aO11a"
  "29a7[3*71,2a3*73,a3~5Ka2R8*70,14aO16a"
  "29a2[a4[4*71,a3*73,2a2~6K4a7*70,30a"
  "32a4[5*71,4*73,a3~5K3a69,7*70,30a"
  "33a3[6*71,3*73,a3~5K3a3*69,6*70,29a"
  "33a3[a6*71,3*73,a3~6*75,2a2*69,6*70,29a"
  "37a7*71,3*73,a~8*75,4a5*70,29a"
  "37a7*71,4a2N8*75,4a2E31a"
  "37a7*71,5a10*75,a4E3a2S26a"
  "37a7*71,2}3a2;9*75,2a2E3U3S25a"
  "38a2*71,3*72,7a3;7*75,3a2V2U2a=10a7=8a"
  "40a4*72,6a5;5*75,3*76,t2V2*77,>a18=7a"
  "39a5*72,3a8;5*75,4*76,tV2*77,2>3*79,=4*80,4=2*80,11a"
  "40a5*72,2a10;3*75,a5*76,3*77,>5*79,10*80,10a"
  "40a5*72,5a11;a2*76,a2*76,l2*77,>5*79,10*80,9a"
  "41a3*72,F10a11;2*76,2*77,2>4*79,11*80,8a"
  "42a72,2F11a;4a6;84,2*77,a>3*79,a11*80,8a"
  "42a4F6a3*82,7a6*88,Ga2>2*79,2a11*80,8a"
  "42a4F3a7*82,6a6*88,Ga4>2*86,a10*80,8a"
  "42a5F2a5*82,8a7*88,2*78,>6*86,6*80,P10a"
  "42a3F4a2*82,6a2*83,3a7*88,2*78,>7*86,2a5P9a"
  "42a2F81,5a82,7a2*83,2a7*88,3*78,>7*86,a5P9a"
  "42a3*81,14a2*83,2a3*88,3a4*78,>7*86,14a"
  "60a2*83,5a2*85,4*78,>8*86,13a"
  "61a83,aka5*85,4*78,>7*86,2>11a"
  "63aka6*85,4*78,9>11a"
  "65a7*85,3*78,4>16a"
  "66a7*85,22a"
  "67a6*85,22a"
  "67a6*85,22a"
  "68a5*85,2w3ac16a"
  "79ac15a"
  "95a"
  "80a5*87,10a"
  "80a6*87,9a"
  "80a7*87,8a"
  "81a6*87,8a"
  "66a3z13a5*87,8a"
  "65a4z26a"
  "95a"
  "95a"
))
