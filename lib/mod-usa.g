(game-module "mod-usa"
  (title "Modern Times - USA")
  (base-module "modern")
  (variants (see-all true))
  )

(area 150 60 (restrict 800 320 0 210))

(unit-defaults (hp 0))

;;; This module brings in both cities and terrain.

(include "u-e50-1998")

(area (restrict reset) (latitude 86) (longitude -229))

(unit-defaults reset)

(set sides-min 1)
(set sides-max 1)

(side 1 (name "USA"))

(unit-defaults (s 1) (hp 1))

(unit "Albuquerque")
(unit "Atlanta")
(unit "Boston")
(unit "Buffalo")
(unit "Chicago")
(unit "Cincinnati")
(unit "Cleveland")
(unit "Dallas")
(unit "Denver")
(unit "Detroit")
(unit "El Paso")
(unit "Ft Worth")
(unit "Houston")
(unit "Indianapolis")
(unit "Jacksonville")
(unit "Kansas City")
(unit "Los Angeles")
(unit "Miami")
(unit "Milwaukee")
(unit "Minneapolis-St. Paul")
(unit "Nashville")
(unit "New Orleans")
(unit "New York")
(unit "Norfolk")
(unit "Oklahoma City")
(unit "Omaha")
(unit "Philadelphia")
(unit "Phoenix")
(unit "Pittsburgh")
(unit "Portland")
(unit "Rochester")
(unit "Sacramento")
(unit "Salt Lake City")
(unit "San Antonio")
(unit "San Diego")
(unit "San Francisco")
(unit "St. Louis")
(unit "Seattle")
(unit "Tampa")
(unit "Tucson")
(unit "Tulsa")
(unit "Washington DC")
(unit "Wichita")

(unit-defaults reset)

(oil-platform 27 20 1)
(oil-field 32 20 1)
(oil-field 28 22 1)

(area (aux-terrain road
  "150a"
  "150a"
  "150a"
  "150a"
  "150a"
  "150a"
  "150a"
  "7ae23ae118a"
  "8a@12ak4su5aD117a"
  "9aD11aj5a>3suf117a"
  "9al2su8aj10a@D116a"
  "9aj3a>sw5sr11aG29su86a"
  "9af6a@16aj30a>2su82a"
  "10aD6a>u14aj34a>2su78a"
  "10af8a@13af38a>u76a"
  "11aF8sN3su9a@39a>u74a"
  "11af13a>7suaD14ae25a>{2su37ai31a"
  "12a@21a>v15aF3su21aj3a>2su33af8ai22a"
  "13aD22aD14af4a>11su8aj6akNsu31aD2ae4aj22a"
  "13aj22af15aD16a>7s|6sr3a>sy20ak4s{uaf3aD3af22a"
  "13af23aD14af24af12af6ak2su10aj4afa>uD2af4aD21a"
  "14aD22af15aD24aD12aD3aks|3sT7ak{r5aD2aC3sV3sz21a"
  "14aj23aD6ak5suaf24aj12af3ajaj3an6akrf6aj3aD2af2akr21a"
  "14af23af3ak2sr6a>sV4su18aj13aH2ajaj3aj>3swstuaD5af3afk2sNsr22a"
  "15aD8aksu3aksu6aDksr11af5a>6su6akw2sx13s|Nwraj3aj5a>su>x6sV3sY27a"
  "15af6aksr2a>2sr2a>5sxr14aD12a>5sra@2aD11akr2a@aj3af8a@aD4akzaksxT26a"
  "16a@4akr17aD14af21a@aj10akr4a@j4aD8a>|u2akrlsrakM26a"
  "17aD2akr18af15aD21a@j9akr6aCu3aj9aja>s|s|s{sr27a"
  "17af2aj20aD6ak7sx14sw5s{sO3su4akr8a@>2sr7aksr3ajalsr29a"
  "18aFwz20aj2ak3sr8a@14aD3akr6a>2sqdu9aD9akr5ajaj31a"
  "18ajaA20al2sr13aD13af2akr8aksq2aF8sz7aksr6ajaf31a"
  "18ab2a@19aj16af14aFsr7aksr4aj8af5aksr8aj2aD30a"
  "22a@17akr17aD13af7aksr6af9aDak2sr9akr2af30a"
  "23a@15akr18aj14aD4aksr9aD5ak2s|sr10aksr4aFsu27a"
  "24a@13akr19aj14af2aksr11af3aksr2aj9ak2sr6aj2a<26a"
  "25a@11akr18amaj15a<ab14aDakr3akx9sr9aj29a"
  "26a>u8akr19aj>r3ak13su14afajak2sra@17akr29a"
  "28a@5aksr2ak4s{12s|5sr14a@14aBtsr5a>u14akr30a"
  "29a>u3al4sr4af12aj21aD14a@9a>u11akr31a"
  "31a>swtu9aD11aj21af15a@10a>u5ak3sz32a"
  "34a@a>7suf11aj22a@15aD11aJ4sr3aj32a"
  "35a@9a>P10af23aD14af8ak2sr@7aj32a"
  "36aD10a>u9a@22af15aD5aksr4a>w5sr32a"
  "36ad12sPak7sP7ak14sX14aj3aksr8a>su35a"
  "50a>r8a@6aj14aj@13aj3aj5ak6sq<34a"
  "61a>su2akr14ajaD12aj3ad5sr42a"
  "64a>st7su7ajaf12aj52a"
  "75a@6aj2a@11aluak10su38a"
  "76a>3suaj3a@k9svaFr11a>3swsu31a"
  "81a@jak2sM10a>r17a@a@30a"
  "82a?sr33a@a@29a"
  "119a@a@28a"
  "120aDa@27a"
  "120aj2a@26a"
  "120af3a@25a"
  "121a@3aD24a"
  "122a@2af24a"
  "123a@2aD23a"
  "124a>sr23a"
  "150a"
  ))
