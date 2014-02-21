(game-module "crater-lake"
  (title "Crater Lake")
  (version "1.0")
  (blurb "A classic test-piece for one-on-one.")
  (instructions "Claim the towns in your vicinity, then go after the enemy.")
  (notes
    "This features two countries connected by a forested isthmus,"
	"plus an island with a town by a lake, surrounded by mountains."
    "Maneuver is therefore difficult."
  )
  (base-module "classic")
  (variants (world-seen true) (see-all false) (sequential false))
)

(set terrain-seen true)

;;; We don't want anything else added.

(set synthesis-methods nil)

(world 35)

(area 35 31)

(area (terrain (by-char ".,=~+%^_")
  "___________________________________"
  "....+,............................."
  "....+....+++++~~..................."
  "........+++++++++,................."
  "......++++++++++~,................."
  "......%%++++++++++,................"
  "...+%.++++++++++++................."
  ".......++++++++++++................"
  "......+++++++++++,.......,........."
  ".......%++++++++++........~~......."
  ".......%+++++++++.................."
  "..,+%..,%++++++++.................."
  "..+%%%+%%%++++++..................."
  "...%%%%%%%%%%+++%.................."
  "...,%%%%%,%%%%%%..................."
  ".....%%%....,%%%..................."
  "^%...........%%................+^^^"
  "^^^%%........%%%......+........+^^^"
  "^^^^%%......%%%%+.....+.......+^^^^"
  ".%^^^^%.....%%%%%++,+..........^^^%"
  ".+^^^^^%%...%%%%+++++..+.......^^^."
  ".+^^^%%^^%..%%%%++++++..+.......^^^"
  "+^^^%%%%+%..,%%+++++~+~..+......^^^"
  "^^^^^%%%%%.....++++++^~++++++....^^"
  "^^^^%%%%%........+++~^^^~~~~+....++"
  "+^++.%%%%.........,++~~~~~+~~~....."
  ".....%%+............++~~~~+........"
  "......%++.............++~+........."
  ".....,.++.........................."
  "........+%........................."
  "___________________________________"
))

(side 1 (noun "Paramagudi"))

(side 2 (noun "Vrigstader"))

(city 6 24 1 (n "Paramagudi"))

(city 20 10 2 (n "Vrigstad"))

(town 8 27 0 (n "Vryburg"))
(town 3 24 0 (n "Mistake Creek"))
(town 9 24 0 (n "Tooele"))
(town 6 22 0 (n "Bellows Falls"))
(town 10 22 0 (n "Atlasburg"))
(town 6 18 0 (n "Adobe Acres"))
(town 22 13 0 (n "Philomath"))
(town 1 9 0 (n "Waggaman"))
(town 24 9 0 (n "Yeehaw Junction"))
(town 8 8 0 (n "Ushtobe"))
(town 16 7 0 (n "Tillamook"))
(town 20 7 0 (n "Hayti"))
(town 24 7 0 (n "Bo Phloi"))
(town 20 4 0 (n "Buzzards Bay"))
