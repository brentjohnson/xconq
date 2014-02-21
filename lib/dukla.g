(game-module "dukla"
  (title "Battle at Dukla")
  (version "1.0")
  (base-module "pgu")
  (blurb "September 1944. The 38th Soviet Army attacks through the Dukla pass, on the border between Poland and Slovakia.")
  (variants
	  (see-all false)
	  (world-seen true)
	  (sequential false)
	   ("Capture the Town" capture-the-town
	    "Each side's goal is to capture the other side's town."
	    (true
	      (add town start-with 2)
	      (add u* point-value 1)
	      (add town point-value 100)
	      ))
  )
)

;  The feature area layer is just used to position feature names on this map.
;  Borders will therefore appear strange if drawn.
(set feature-boundaries false)

(set synthesis-methods '(make-countries))

(world 360 (daylight-fraction 50) (twilight-fraction 60))

(area 64 40 (cell-width 1000))
(area (terrain
 (by-name
    (gully 0) (fortification 1) (port 2) (stream 3) (slope 4)
    (impassable-river 5) (clear 6) (city 7) (airfield 8) (forest 9)
    (bocage 10) (hill 11) (mountain 12) (sand 13) (swamp 14)
    (ocean 15) (rough 16) (empty 17) (road 18) (river 19))
  "gh4ga9ga18gj9g19a"
  "g2h3g2a8g2a15g3j3gh5gr18a"
  "rghgj3gj2a8ga5g2i6g4j3g3h3gr17a"
  "r2agj2g3ja8g2a2h12g4j2gh4g2r16a"
  "2rgag2j3gja8g2h2a7g3a2g5j6g2r15a"
  "2rgag3jgj2g2a6g3hg4ag2a3g2ajg4j5g3r14a"
  "3rgag2jg3j2dah10gh2g2a6ga3gj6g3r13a"
  "3rga2gj2gjgd2ga10gj3god5g2a5gd3g4r12a"
  "4rga7gd2g2a9g2jgo2d7g2ag2dg2dg4r11a"
  "4rga6gd2gj2ga8g2jodo2d4g2ogad5gd5r10a"
  "5rga6gdg3jg2a8gjgd2od4g2o2ga2g2j2g5r9a"
  "5rga6gd2gj4ga7gjgd2od5go2gag3j2g6r8a"
  "6r2ga4gj8g2a4gh3gdgod5go2ga6g6r7a"
  "6r2g2a3gjgj8g2a4ghgd2gd8ga4gjg7r6a"
  "7r3ga3g3jgj7ga6gd2gd8ga3g2jg7r5a"
  "7r4ga5g4j3g2a2gj4gd3gd7ga6g8r4a"
  "8r4gagh4g3j3ga3g3j2gdgh2d5gh2a6g8r3a"
  "8r3gjagh5gjg2jga4g2j2gdghd4g3jag2j4g9r2a"
  "9rgl2j2a4gh4g2ja4gjgj2djgd7gaj6g9ra"
  "9r2gl2ga2g2j6gjg2a5gdgljd7ga2d5g10r"
  "a9r6gag3j4ghg2ha6gjljdgj5ga2gd4g9r"
  "2a8rgjgl2g2a2gj2gj2g3ha6gjgdjg3j3ga2g2d3g9r"
  "3a8rgj2l2j2g2ag3j2g2iha8gdg5j2ga3gd3g8r"
  "4a7rgj2l2jglga2gj7ga7g2d2g3jga5gd2g8r"
  "5a7r2gjgj2gl2jag2jgh4g2agl5gd4gjga3j2gd2g7r"
  "6a6r7g2lja3g4j3gaglghglgd4gjga4jgd2g7r"
  "7a6r6g2jl2ad4g3j2gag2l2g2ld6gag3j2d2g6r"
  "8a5r8ga2gd4gj4ga5g2ldgd4ga4gd3g6r"
  "9a5r3ga3g2a3gd4g4ja6glgd2l3ga4gd2jg5r"
  "10a4rg2ag2agagj2gd5g3jaj8glj3gag2j2g2jg5r"
  "11a4rga4ga2ljg2dg2j4gja2j2l3gj4l2ga2jl3jgj4r"
  "12a3rga3gj3ljgd3g2j4ga2j4l2g3l2jgag2ljl2jg4r"
  "13a3ra2g3j3l2gd4gj4ga3j6gj4g2a5ljg3r"
  "14a2r3g3jlgjgd10ga2gj5a7ga2g4lj3r"
  "15a2r2g2ljgj2g2d2gi8g3aj3ga7gh2a3l2j2r"
  "16ar2g2l2j3gd2l3gl2gh4gjg2j2ga8gha2gl2g2r"
  "17ar2g3lj3gd3g4l4gjg2jl2j2ga9g2aglgr"
  "18a4glgj3gl2g4ljgjg2j4l3g2a2g2ljlj3g2agr"
  "19a6gj3glg3ljg3j8l3gagj5l4gag"
  "20a13gl3g2j8l3ga6gl4gag"
  ))
(area (aux-terrain road
  "e@10ajn22a2j2i24a"
  "a@D9a2b@8akq11ad2t~am22a"
  "2aA12aH7aj15adR|P21a"
  "3a@11aj>su2aksr17a=a@20a"
  "4aD10af3a@ov22a>u18a"
  "4af9aekL3a=BX23a@17a"
  "5a@9a?q5a?P23a<16a"
  "6a@17a@39a"
  "7a@17a@38a"
  "8a@17a@37a"
  "9a@17aD5acq29a"
  "10a@16an36a"
  "11a@15alNu34a"
  "12aD14aj2a@33a"
  "12aj14aj3a>u31a"
  "12ah}m12aj5a>u7ae21a"
  "8aksy2a2EFu10aj7a>y2aiaksN8sq11a"
  "8abad2srdra>u8aj8ad2stsr22a"
  "19a>u6aj36a"
  "21a>su3an36a"
  "24a>uanD35a"
  "26a>tQ35a"
  "29a@34a"
  "30a@33a"
  "31a@32a"
  "32a@31a"
  "33aD30a"
  "33af30a"
  "34aD29a"
  "34af29a"
  "35aD28a"
  "35aj28a"
  "34akt2su25a"
  "34aj4a@24a"
  "34af5a@2akuk8su9a"
  "35a@5a>sra=9a@8a"
  "36aD19a>u6a"
  "36af21a@5a"
  "37aD21aD4a"
  "37af21af4a"
  ))
(area (aux-terrain river
  "64a"
  "5ac2su7acu46a"
  "9a@8a@45a"
  "csu7a@8a>su42a"
  "3aD7a@10a>u40a"
  "3af8aFu10a>4sy34a"
  "4aD7aja@14adu33a"
  "4af7an2a@15a@32a"
  "5aD6aj<2a>u14aH31a"
  "5af6af5a@13aj@30a"
  "6aD6aD5a>u11ajaD29a"
  "6af6ab7a@10ajaf29a"
  "7a@14a>u8aj2aD28a"
  "8a>u14a>y6af2af28a"
  "10a@14aj7aD2a@27a"
  "11a@13aj7af3aH26a"
  "12aD12aj8aD2aj<25a"
  "12af12af8an2af26a"
  "13a@12a@7aj<2aD25a"
  "14a@12a>u5ab3af25a"
  "15a@13aD9aD24a"
  "16a>u11af9aj24a"
  "18a>u10a@8af24a"
  "20a>u9a@8a@23a"
  "22aD9a@8aD22a"
  "22an10a@7af22a"
  "22aj@10aD7aFu5ai14a"
  "21akra@9af7abaD4af14a"
  "17ae3aj3a@9aD8ab5aD13a"
  "15acya>2sz4aD8af14af13a"
  "16aj4ab4aj9aD14aD12a"
  "16aj9af9af14af12a"
  "15acr10aD9aD14aBq10a"
  "27aj9ab3ak3su7a@10a"
  "26akr11acsr4aD7a>u8a"
  "26af19ab9a@7a"
  "27a<20ae8a>su4a"
  "48acP10a>u2a"
  "50aD11aDa"
  "50ab11aba"
  ))
(area (features (
   ( 1 "feature" "Dukla")
  )
  "64a"
  "a2b61a"
  "2ab61a"
  "21a2b41a"
  "20a2b42a"
  "20a3b41a"
  "64a"
  "64a"
  "64a"
  "64a"
  "64a"
  "64a"
  "64a"
  "64a"
  "64a"
  "64a"
  "64a"
  "64a"
  "64a"
  "64a"
  "25aba2b35a"
  "26a3b35a"
  "29ab34a"
  "64a"
  "64a"
  "64a"
  "64a"
  "64a"
  "64a"
  "64a"
  "64a"
  "64a"
  "64a"
  "64a"
  "64a"
  "64a"
  "64a"
  "64a"
  "64a"
  "64a"
  ))

