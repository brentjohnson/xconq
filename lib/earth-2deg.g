(game-module "earth2"
  (blurb "Earth, at 2 deg/cell")
  (variants
   (world-seen true)
   (see-all false)
   ;; The following is useful for hacking on the terrain
   (standalone true
    (true
     (set synthesis-methods nil)
     (add u* start-with 0)
     (add city start-with 1)
     (add u* independent-near-start 0)
     (table independent-density (u* t* 0))
     ))
    )
  )

(world 180)

(area 180 64)

(area (terrain
 (by-name
    (sea 0) (shallows 1) (swamp 2) (desert 3) (plains 4)
    (forest 5) (mountains 6) (ice 7) (road 8) (river 9))
  "180h"
  "5e8f8a5f36ae6f5ae62f3a3f15a4fea13f3e"
  "6e8f8a5faf29ae4a2e3f2e2a2ea2ef2efe56f3a2f16a3fgfa2f4a7f2e"
  "7e8f7a9f27ae5aea3f4a11efe2f6e12f3efef4e2f2e12f9a2f19agf8a4fg3f"
  "f2eg6e8f5a9f27a2e5aeae4a23e3f22e10f10a2f18a2gf9afa3f2g"
  "2f2eg8e8fa11f24a2ea2e4a2e2a44ea8e2f2e3f11a2f16a2g15a3fg"
  "2eg2eg9ea6fa5fef2ef25aea2e2a64e2f3e8af15agag18a2f"
  "2afg2eg9e11f2e5a2e27a53e5d8e2f2e46af"
  "afa2eg13e3a4f3ea2e2a3e24a8eg43e8d8ef4e46a"
  "a3fe3g12eaeae2f6eae28a6e3g10ea5e2a3ea20e6d14e2ae44a"
  "2aef2e3g12eaea6ea3e29a9eaegeg2e6a2ge2a3ea16eg21e3a2e43a"
  "2a2ege5g12ea5e31a6e5a2eaeg3e2ae3ageg2a13e2ge2ge2ge4g13e6a2e43a"
  "3aeg3de3g16e32a6e4ae2ae2ae2a8e2g2a10e15g8e2a3e6ae44a"
  "3a2eg3da2g13ef2e32a6e8ae2aea7e3g3a6e4g2e12g9e3a2e5a2e43a"
  "5a2eg2dedg12ef3e33a2e4a4e9a3e2g4egeg9e3ge13g8e2a2e2a4e43a"
  "6a2e5d11e2f2e39ae3de7ae5ae3d2eg9e4ge14g6e6a4e44a"
  "8a2e5d12e35a6e3gd3e2a2e7ae3d3eg12e2g3h9gf7e4ae47a"
  "10aeda4d10e35a2e5g13d4a6dae2g4e2ge8g3h4g2f8e52a"
  "12ada4d3e6ae34aedg18d3e7d2ae7g3d5e3gh4gf9e3ae47a"
  "13adadg3e7aeae31a2e22deda6d3a4e5d6e2f6g2f6e52a"
  "14adae2g2e41ae25da8d6a3d8e7f7e2ae49a"
  "17a2ege7a3e31ae26da10d4a2d12e3f5e4ae39ae9a"
  "18a4e3a2f4a2ea2eae24ae27d2ae7d6a7e4a2e3fe2ae46ae7a"
  "20a5e2f12ae22ae28d2a7d7a5e6ae4f8ae48a"
  "22a2efe2f5ae29ae28d2a5d9a4e7a3e2f7a2e47a"
  "26afef11ae22ae5d2e3d3e6d2e3d4edadge12a3e8a2e3f7ae11ae35a"
  "29a2f5a2e4ae21a3e2d25e16a2e9aeaef7aeae3ae41a"
  "30a2f3a6ef23aef5e2f2ef15e2ge4d12ae9ae2ae6afae14ae31a"
  "32a3fefgf2e3f23afefe4fe5f3e2fef5e3ge2d14ae8ae12aef43a"
  "35afeg2f2e4f22a5fe2a13f4e3ge2d23afef6ae48a"
  "36aeg11f30a11f5ege2d22af2aef5aef47a"
  "35ae2g11f31a11f6ed24af2ae4ae3f46a"
  "31ae3ae2g13f29a11fea4e26a2f5aefgfa3f23ae18a"
  "36a2g15f28a11f5e27afe5a3faf5a2faf35a"
  "36ad2g7fe10f25a6fe4f4e30ae2f6a2f2af2a5fe31a"
  "37a2g17f3e24a2fef2efe2f4e33ae7af6a2fg2f2aeae25a"
  "38ad2g15f4e23aef7ef4e35aeaf2af3af5a4fe5ae22a"
  "39a2g6fe3fe3f5e24a14e54af27a"
  "40ae2g4f2e2f2e2f5e24a15e3ae41a2f3af30a"
  "41a3g3f2efefe2f5e25a14e3aef39a3ef2a2f13ae15a"
  "42ad4gfefef2ef4e27ad12e3a2f38a6e2a2e28a"
  "44ad2gf3ef7e28a3de2d5e4a2f36a3e3d3ea3e27a"
  "45ad2g11e29a6d4e4a3f3ae31a2e10d3e25a"
  "45ad2g10e30a7d3e4a3f33a3e11d4e7ae16a"
  "46adg10e31a6d3e5aef33a16d3e23a"
  "46adg9e33a3d5e40a16d4e22a"
  "47adg9e33a2d5e41a17d3e21a"
  "47aeg9e34a5e43ae15d3e21a"
  "48aeg8e35a4e44ae11de2d3e21a"
  "48aeg7e84a5e5ade3d3e21a"
  "48ae2g5e98a5e21a"
  "48aeg6e99a4e13ae7a"
  "49afg3e118a2e6a"
  "49afg3e103a2e12ae8a"
  "49a2fg2e104ae12a2e7a"
  "49af2ge118ae8a"
  "50afg2e117ae8a"
  "50afge127a"
  "51af2e126a"
  "52a2e126a"
  "53a3e124a"
  "180a"
  "180a"
  "180h"
  ))

(town 73 34 0 (n "Accra"))
(town 69 49 0 (n "Algiers"))
(town 82 50 0 (n "Ankara"))
(town 86 55 0 (n "Astrachan'"))
(town 77 50 0 (n "Athens"))
(town 90 47 0 (n "Baghdad"))
(town 121 38 0 (n "Bangkok"))
(town 36 34 0 (n "Bogota"))
(town 107 40 0 (n "Bombay"))
(town 76 54 0 (n "Bucharest"))
(town 74 55 0 (n "Budapest"))
(town 6 57 0 (n "Calgary"))
(town 93 15 0 (n "Cape Town"))
(town 62 47 0 (n "Casablanca"))
(town 63 38 0 (n "Dakar"))
(town 96 28 0 (n "Dar es Salaam"))
(town 60 59 0 (n "Glasgow"))
(town 83 59 0 (n "Gorky"))
(town 125 54 0 (n "Haerbin"))
(town 121 42 0 (n "Hanoi"))
(town 29 42 0 (n "Havana"))
(town 125 43 0 (n "Hong Kong"))
(town 170 42 0 (n "Honolulu"))
(town 109 40 0 (n "Hyderabad"))
(town 114 57 0 (n "Irkutsk"))
(town 78 52 0 (n "Istanbul"))
(town 127 29 0 (n "Jakarta"))
(town 86 47 0 (n "Jerusalem"))
(town 92 41 0 (n "Jiddah/Mecca"))
(town 76 56 0 (n "Kiev"))
(town 95 57 0 (n "Kujbyshev"))
(town 120 45 0 (n "Kunming"))
(town 75 35 0 (n "Lagos"))
(town 40 25 0 (n "Lima"))
(town 60 50 0 (n "Lisbon"))
(town 111 38 0 (n "Madras"))
(town 46 29 0 (n "Manaus"))
(town 132 38 0 (n "Manila"))
(town 97 18 0 (n "Maputo"))
(town 20 41 0 (n "Mexico City"))
(town 27 44 0 (n "Miami"))
(town 70 53 0 (n "Milan"))
(town 17 54 0 (n "Minneapolis-St. Paul"))
(town 94 31 0 (n "Nairobi"))
(town 103 58 0 (n "Novosibirsk"))
(town 100 57 0 (n "Omsk"))
(town 141 15 0 (n "Perth"))
(town 96 16 0 (n "Port Elizabeth"))
(town 151 27 0 (n "Port Moresby"))
(town 57 16 0 (n "Porto Alegre"))
(town 118 40 0 (n "Rangoon"))
(town 59 27 0 (n "Recife"))
(town 73 51 0 (n "Rome"))
(town 124 36 0 (n "Saigon"))
(town 48 14 0 (n "Santiago"))
(town 135 52 0 (n "Sapporo"))
(town 128 50 0 (n "Seoul"))
(town 125 32 0 (n "Singapore"))
(town 36 55 0 (n "St. John's"))
(town 75 61 0 (n "St. Petersburg"))
(town 91 59 0 (n "Sverdlovsk"))
(town 98 52 0 (n "Tashkent"))
(town 86 52 0 (n "Tbilisi"))
(town 92 48 0 (n "Tehran"))
(town 72 55 0 (n "Vienna"))
(town 129 53 0 (n "Vladivostok"))
(town 123 47 0 (n "Wuhan"))
(city 121 52 0 (n "Beijing"))
(city 69 57 0 (n "Berlin"))
(city 54 14 0 (n "Buenos Aires"))
(city 85 44 0 (n "Cairo"))
(city 112 42 0 (n "Calcutta"))
(city 39 36 0 (n "Caracas"))
(city 20 52 0 (n "Chicago"))
(city 66 59 0 (n "Copenhagen"))
(city 18 48 0 (n "Dallas"))
(city 107 45 0 (n "Delhi"))
(city 12 50 0 (n "Denver"))
(city 81 55 0 (n "Doneck/Kharkov"))
(city 66 56 0 (n "Dortmund/Essen"))
(city 19 46 0 (n "Houston"))
(city 95 18 0 (n "Johannesburg"))
(city 61 57 0 (n "London"))
(city 8 47 0 (n "Los Angeles"))
(city 63 51 0 (n "Madrid"))
(city 156 12 0 (n "Melbourne"))
(city 28 55 0 (n "Montreal"))
(city 79 59 0 (n "Moscow"))
(city 27 51 0 (n "New York"))
(city 133 48 0 (n "Osaka"))
(city 64 55 0 (n "Paris"))
(city 57 21 0 (n "Rio de Janeiro"))
(city 3 50 0 (n "San Francisco"))
(city 3 55 0 (n "Seattle"))
(city 127 47 0 (n "Shanghai"))
(city 69 61 0 (n "Stockholm"))
(city 158 15 0 (n "Sydney"))
(city 136 49 0 (n "Tokyo"))
(city 74 57 0 (n "Warsaw"))
(city 27 49 0 (n "Washington DC"))
