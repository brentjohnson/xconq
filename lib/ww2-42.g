(game-module "ww2-42"
  (title "WWII World 1942")
  (version "1.0")
  (blurb "The world in January 1942. Strategic level game.")
  (base-module "ww2-adv")
  (variants
   (world-seen true)
   (see-all true)
   (sequential false)
   )
  (instructions (
   "In this game you play the national leader of your country during WWII."
   ))
  )

;;; Define basic terrain.

(include "earth-1deg")

;; We want the rivers.

(include "t-e1-river")

(set synthesis-methods '(name-units-randomly))

(include "ww2-sides")

;;; France is out of the picture now.

; (side 2 (active false))

;;; Define the nationalities of the people.

(include "p-e1-1938")

;;; Define the cities.

;; (should have more British bases - for instance at Bermuda and in West Indies)

(include "u-e1-1938")

(infantry 317 33 uk)
(infantry 219 81 uk)
(infantry 214 85 uk)
(infantry 225 89 uk)
(infantry 165 96 uk)
(infantry 123 117 uk)
(infantry 122 117 uk)
(infantry 121 117 uk)
(infantry 122 118 uk)
(infantry 121 118 uk)
(infantry 121 119 uk)
(infantry 121 120 uk)
(armor 165 95 uk)
(armor 122 119 uk)
(bomber 120 117 uk (in "Bristol"))
(bomber 119 121 uk (in "Newcastle"))
(bomber 119 121 uk (in "Newcastle"))
(interceptor 124 117 uk (in "Dover"))
(interceptor 124 117 uk (in "Dover"))
(interceptor 120 117 uk (in "Bristol"))
(interceptor 123 118 uk (in "London"))
(interceptor 123 118 uk (in "London"))
(interceptor 120 119 uk (in "Manchester"))
(interceptor 120 119 uk (in "Manchester"))
(interceptor 122 120 uk (in "Hull"))
(interceptor 122 120 uk (in "Hull"))
(fleet 119 117 uk)
(fleet 119 119 uk)
(fleet 121 121 uk)
(fleet 116 122 uk)
(fleet 117 125 uk)

(infantry 263 81 us)
(infantry 54 101 us)
(infantry 10 103 us)
(infantry 41 105 us)
(infantry 56 108 us)
(fleet 14 99 us)
(fleet 54 104 us)
(fleet 7 104 us)
(fleet 58 108 us)
(fleet 3 113 us)

(infantry 127 111 de)
(infantry 163 113 de)
(infantry 162 113 de)
(infantry 125 113 de)
(infantry 162 114 de)
(infantry 127 114 de)
(infantry 162 115 de)
(infantry 128 115 de)
(infantry 161 117 de)
(infantry 161 118 de)
(infantry 159 119 de)
(infantry 158 120 de)
(infantry 158 121 de)
(infantry 158 122 de)
(infantry 157 122 de)
(infantry 156 122 de)
(infantry 153 123 de)
(infantry 153 124 de)
(infantry 151 124 de)
(infantry 150 124 de)
(infantry 149 124 de)
(infantry 148 126 de)
(armor 162 116 de)
(armor 160 119 de)
(armor 152 124 de)
(air-force 126 112 de)
(air-force 129 113 de)
(air-force 129 114 de)
(air-force 126 114 de)
(air-force 129 116 de)
(air-force 132 118 de)
(bomber 128 113 de)
(bomber 138 117 de)
(bomber 142 119 de)
(interceptor 134 114 de)
(interceptor 127 115 de)
(interceptor 131 118 de)
(interceptor 129 118 de)
(interceptor 131 119 de)
(convoy 124 115 de)
(convoy 142 120 de)
(convoy 135 120 de)
(convoy 132 123 de)
(sub-fleet 126 110 de)
(sub-fleet 124 112 de)
(sub-fleet 127 118 de)
(sub-fleet 131 120 de)
(sub-fleet 130 120 de)
(sub-fleet 130 121 de)
(sub-fleet 125 127 de)
(sub-fleet 131 133 de)

(infantry 158 97 it)
(infantry 146 103 it)
(infantry 143 107 it)
(infantry 138 111 it)
(convoy 150 97 it)
(convoy 145 104 it)
(convoy 143 106 it)
(convoy 137 109 it)
(fleet 147 105 it)

(infantry 252 89 jp)
(infantry 253 96 jp)
(infantry 251 98 jp)
(infantry 249 97 jp)
(infantry 247 100 jp)
(infantry 244 100 jp)
(infantry 244 102 jp)
(infantry 271 102 jp)
(infantry 270 102 jp)
(infantry 247 103 jp)
(infantry 247 106 jp)
(infantry 251 107 jp)
(infantry 251 111 jp)
(convoy 269 99 jp)
(convoy 262 99 jp)
(convoy 271 100 jp)
(convoy 272 103 jp)
(fleet 268 99 jp)
(fleet 272 100 jp)
(fleet 270 100 jp)
(fleet 267 100 jp)
(cv-fleet 263 98 jp)
(cv-fleet 264 100 jp)
(sub-fleet 265 99 jp)
(sub-fleet 263 100 jp)
(sub-fleet 270 107 jp)

(infantry 259 110 su)
(infantry 165 112 su)
(infantry 260 113 su)
(infantry 164 113 su)
(infantry 164 114 su)
(infantry 163 115 su)
(infantry 163 116 su)
(infantry 252 117 su)
(infantry 162 117 su)
(infantry 238 118 su)
(infantry 162 118 su)
(infantry 161 119 su)
(infantry 160 120 su)
(infantry 159 120 su)
(infantry 160 121 su)
(infantry 159 122 su)
(infantry 158 123 su)
(infantry 157 123 su)
(infantry 156 123 su)
(infantry 155 123 su)
(infantry 154 123 su)
(infantry 154 124 su)
(infantry 152 125 su)
(infantry 152 125 su)
(infantry 150 126 su)
(infantry 149 126 su)

(infantry 249 90 zh)
(infantry 255 91 zh)
(infantry 240 91 zh)
(infantry 255 92 zh)
(infantry 239 92 zh)
(infantry 252 93 zh)
(infantry 249 93 zh)
(infantry 247 95 zh)
(infantry 245 95 zh)
(infantry 247 97 zh)
(infantry 247 98 zh)
(infantry 245 98 zh)
(infantry 242 99 zh)
(infantry 241 99 zh)
(infantry 241 102 zh)
(infantry 239 103 zh)
(infantry 240 104 zh)
(infantry 237 105 zh)

;;; Modify for territorial changes from 1938.

;; British gains in East Africa.

(unit "Adis Abeba" (s uk))
(unit "Asmera" (s uk))
(unit "Mogadishu" (s uk))

;; German gains in Europe.

(unit "Copenhagen" (s de))
(unit "Rotterdam" (s de))
(unit "Warsaw" (s de))
(unit "Brussels" (s de))
(unit "Prague" (s de))
(unit "Vienna" (s de))
(unit "Budapest" (s de))
(unit "Zagreb" (s de))
(unit "Bucharest" (s de))
(unit "Thessaloniki" (s de))
(unit "Athens" (s de))
(unit "Bordeaux" (s de))
(unit "Le Havre" (s de))
(unit "Lyon" (s de))
(unit "Marseilles" (s de))
(unit "Nantes" (s de))
(unit "Nice" (s de))
(unit "Toulouse" (s de))
(unit "Paris" (s de))
(unit "Kiev" (s de))
(unit "Minsk" (s de))
(unit "Odessa" (s de))
(unit "Riga" (s de))
(unit "Vilnius" (s de))

;; Japanese conquests in China and Indochina.

(unit "Amoy" (s jp))
(unit "Canton" (s jp))
(unit "Hankow" (s jp))
(unit "Hanoi" (s jp))
(unit "Kaifeng" (s jp))
(unit "Nanking" (s jp))
(unit "Nanchang" (s jp))
(unit "Ningpo" (s jp))
(unit "Paoting" (s jp))
(unit "Peiping" (s jp))
(unit "Phnom Penh" (s jp))
(unit "Saigon" (s jp))
(unit "Shanghai" (s jp))
(unit "Soochow" (s jp))
(unit "Swatow" (s jp))
(unit "Taiyuan" (s jp))
(unit "Tientsin" (s jp))
(unit "Tsinan" (s jp))
(unit "Tsingtao" (s jp))

;; (add minor countries also?)

;; (set up agreements applying to particular dates)

(scorekeeper (do last-alliance-wins))

(set initial-date "Jan 1942")

(set initial-year-part 12)

;;; Set up January 1942 alliances.

(side uk (trusts (uk 1) (us 1) (su 1) (zh 1)))
(side su (trusts (uk 1) (us 1) (su 1) (zh 1)))
(side zh (trusts (uk 1) (us 1) (su 1) (zh 1)))
; 
(side us (trusts (uk 1) (us 1) (su 1) (zh 1)))

(side de (trusts (de 1) (it 1) (jp 1)))
(side it (trusts (de 1) (it 1) (jp 1)))
(side jp (trusts (de 1) (it 1) (jp 1)))
; Vichy France.
(side fr (controlled-by de))

;;; Show the current control.

(area (control-sides
  "360X"
  "9X4b157X4h19X46h121X"
  "6b5X2b3Xb151X4h17X53h5X13h98X2b"
  "10b5X3b118X3e6a2h20X3h16X57h2X26h47X3d38Xb"
  "10bX2bX5b5Xb109X5e6a6h31X104h25X13d3X4d4b23X2b"
  "14X7b2X5b105X3e10a7h22X116h19X21d23b6X"
  "7b7X14b104X3e11ah21X129h13X22d28b"
  "27b105X2e5a2X6a2h6X5h3X139h9X21d28b"
  "25b72X5a29X2e5a3X6a3hXhX153h9X17d28b"
  "24b73X6a26X4e5a4X6a159h4X22d27b"
  "24b74X5a24X5e5a4X7a147h5X6h5X22d27b"
  "24b16X4b66Xa15X6e5a4X7a134h4X7h11X2h10X21d23b"
  "25b16X6b79X6e6a4X6a128h2X5h4X3h24X26d20b"
  "25b17X7b77X7e5a10Xe125h5X4h31X28d18b"
  "25b18X8b4Xb70X7e5a11X126h5X3h32X11d2X3d9X6d14b"
  "26b17X10b2X3b58X2b9X3e3X5a5XeX9e108h16X5h35X10d16XdX2d12b"
  "28b16X15b56X4b12XeX5a5XeX9e105h18X7h39X2d20Xd2Xd11b"
  "34b8Xb2X16b56X2b12X2e2X3a5X16e98h21X7h37X4d2X2d19X11b"
  "36b8X18b52X2bX4b10X4e8X16e98h2Xh17X7h35X4d28X9b"
  "41b3X21b49Xa2b2X4b9Xe6X3eX16e100h19X6h4XhXh27X2d32X8b"
  "41b3X23b47X3a3X3b6X32e103hXh12X5h31XdXd33Xb2X7b"
  "43b2X23b46X3a2X5b4X34e103hXh12X3h15Xd11XdXdXd38Xb3X4b"
  "44bX22b48Xa4X5b2X35e74h2a27hXh13Xh22XdXdXd48X4b"
  "66b2Xb50X3b5X36e70h6a10g16h2X2h11Xh76X3b"
  "36b3X22b6X4b52XeX38e68h8a17g9h2X2h89X2b"
  "4X31d5X22bXb3X5b50X41e64h12a19g8h2Xh91X"
  "2XdX31d2Xd4X20b5X6b50X11e3a27e63h12a19g7h3Xh9Xh81X"
  "3X38d3X19b3Xc58X3e9a4f18eX2e3X12h3X47h12a19g7h4Xh90X"
  "3X37d2X2d3X9bd9b60X2e6a6fX15e4X3e2X10h5X47h12a19g6h13Xg82X"
  "4X36d2X2d2X10b2d2b2X2b63Xe7a6f2X14e4Xe2X11h5X39h4i15a2i18g6h5X2g4X2g83X"
  "4X36d2X3dX4b3X5d4Xb57X8j7a4X3f2X13e10X9h4X33hX2h8i13a3i19g4h6X5g86X"
  "5X36d2X3d2b2X8d62X11ja5Xc3X3f3X10e12X8h5X29h4X10i11a5i19g10X3g87X"
  "5X37dX3d3X10d62X2a9j10X4f2Xf7e2a3X4a5X8h5X28h3X11i11a4i19g101X"
  "6X51d64X2a8j7X2f4X4fXf4e2X20a3h4X27h18i7a5i10g2X7g11X2g88X"
  "6X50d65X2a7j2Xj6X2f5Xf3Xf2e5X20a2h4X26h31i8g2X2g2X3g12X2g88X"
  "8X48d47Xa17X2a8j16Xf4X3e2X23a5X18h2X5h31i8g7X3g11X2g88X"
  "8X48d49Xa17X7j17Xf4X2e2XfX10a3ca3b5a5X7a12h2X4h31i9gXg5X3g9X3g88X"
  "10X46d69Xb8X9c2X2f7Xe5XaXaX2a2X4c5b17a10h4Xh2a7i5aX18i13g3X3g7X5g87X"
  "11X45d76X11c13X2eXf8X2c9b16a6h2a7X16aiX14i10g6X3g2X10g87X"
  "12X44d70X17c5Xb16X2b2X11b17a4h3a2b6XaX7aX5a15i11g6Xg5X2gX2g90X"
  "15X39d71X19c25X12b23a5b21X19i5g9X2gX2g92X"
  "14Xd2X37d15Xb47Xj7X19c5f5X3f12X3b4a6b21a7b22X18i5g9X2g94X"
  "18XdX33d71X10c7X4c4f5X5f5X2b3X3ba2X3a5b20a10b20X15i2gi5g8Xg95X"
  "18X2a2Xa31d71X7c11X2c16f10b2a4X3a4b18a14b18X14iX7g45Xa58X"
  "19X2a2X9a8d3Xd8X2d63Xj7X6c15X15f10b2a6X3ab2X17a17b16X13i9g103X"
  "21X2a2X8a7d13X2d64XjXj3Xj3c35X5bXbXa8X3a3X12a23b13X14i9g7Xg52Xd41X"
  "21X3a2X10a3d15X2d67X4j38X5b2X2a9X2a4X10a28bX4b4X21i103X"
  "24X2aX11a2d14X3d66X4j39X5b2X2a9X3a7X6a34bX21ig7Xg95X"
  "25Xa2X2aX8ad15X2dXb63X4j41X5b2X2a10XaXb3Xa3X4a36b21i73Xd29X"
  "26X2a2X10a20Xb61X4j42X5b2Xa11X3a2X3a10X32b17igigX2g2Xg16Xg80X"
  "27X2a2X9a22Xb59X3j43X5b3X2a10X9a9X32b13igX2g3X2g100X"
  "33X8a14X5a63X2j43X6b3X2a16X4a9X22bX8b2ic10iXb5Xg79Xd20X"
  "34X8a17X6a58X3c43X6b3X3a16X2a10X2bX15b4X7b3a5ci2Xi91Xd18X"
  "35X8a6X4a10X3a58X3c42X7b4Xa16X2a14X14b5X6b4a5c96Xd16X"
  "29Xa5X9a5X4a14X3a54X4c41X8b4Xa15X2a14X13b7X5b5a3c3X2i94Xd14X"
  "37X9a3X3ab10X2b2X6aX2dXa47X5c39X10b3X2a12X2a16X11b10X4b5a3c3X2i10Xd82X2d13X"
  "40X12ab72X8c24X2c10X11b3X2a11Xa17X10b11X4b6a3c13X3d96X"
  "43X9ab27Xb36Xa7X9c14X6c2X6c6X13b4X12a18X9b12X2bXb6a4c12X2d96X"
  "49X9a67X10c9X23c15b3X7b2a22X6b18Xb6a4c11X2d22Xg73X"
  "51X9a21Xc37Xa5X42c17b2X6b25X6b18X6a5c11X2d95X"
  "53X7a23Xb42X3b18c9b12c17bX3b29X5b18X5a6c13X2d20Xg72X"
  "57X4a14Xa6Xa44X20c10b12c17b33X5b13Xa4X2aXa7c12X2dXd91X"
  "58X3a10XaXa10Xb43X3a16c10b12c18b6X2b3Xb20X5b18X2a3X6c16Xd90X"
  "59X3a7X4aX5a2XaXa46X3c2b7c4b2c9b13c19b2X5b20Xb4X3b14Xa4Xa5X4c11XdXdX2d90X"
  "61Xa4Xa2X4aX10a47X2c3b7c4bc9b13c26b25X3b2Xb16Xa6Xc12X2d5Xd12Xa76X"
  "62X4aX19a47X3ba6c4b2c8b14c25b27Xb2X2b12Xa3X2a17Xd4X4d88X"
  "65Xa2X18ab47X2b2a6c4bc8b14c25b30X2b17X2b21X5d25Xa61X"
  "70X16a3b47X3a6c4bc7b15c24b31X2b18X2b15Xa6X2d32Xa54X"
  "70X17a3b2c45X2a7cb5X5b16c23b48Xa2X3b12X4a94X"
  "71X17a2b2cX3a44Xc12X2b2X13c8X18b48X3aX3b11Xa2X2a93X"
  "71X17a2b7a61X10c12X15b50X3aX3b9X2a2Xa94X"
  "71X27a61X8c14X14b52X2a2X2b8X2a3Xa93X"
  "70X29a60X8c2a12X13b54X3aXb5X4a11Xa48Xa37X"
  "70X30a60X2j5c3a11X12b54XaX3a7Xa7Xa2X5a36Xa48X"
  "58XaXa9X31aXa53Xj3X3j4c3a12X10b58X4a5X2a6Xa2Xa5Xa5X2aXa74X"
  "70X36a54X3j4c2a12Xa2bX7b59X5a5Xa4X2a2X5a3Xa3X3a54Xa20X"
  "71X37a48Xj4X2j4c2a12Xa2b2X5b61X5aXa2X6a3X3a10X2a2X5a6Xa2Xa58X"
  "71X41a51X3c4a11Xa2b2X6b62X4a8X2a3XaXa6X2a3X3a4X3a65X"
  "71X43a5Xa44X2cX3a11Xa3bX6b63X3a13XaX2a13X3a3X3a6Xa56X"
  "72X46a47X5a11Xa10b16Xb48X2a13Xa3Xa14Xa4X3a2X4a3Xa51X"
  "73X45a49X3a11XaX9b68Xa3X2a31Xb3a7XaXa21Xa27X"
  "74X45a48X4a10X2aX9b68X7a20Xa9X4b30Xa27X"
  "75X44a19Xb28X4a10X2a2X9b73X2aXaXa3Xa2X2a4Xa14X4b7Xa48X"
  "16Xa59X43a49X4a9X3abX9b80Xa3Xa23X2b10Xa18Xa25X"
  "77X42a50X3a9X3a5bX6b109Xb11Xa42X"
  "78X41a51X2a7XaX4a5bXb5a56Xb32XbXb9Xb61X"
  "79X39a24Xa27X2a7X6a5bXb5a3Xc4Xc81X6b5X2b60X"
  "81X37a52X2a7X2a10bXb5a7X2c80X6b6X2b43XaX2a12X"
  "82X36a52X3a6X2a8b2a2b5a7X2c76X2bX7b6X3b58X"
  "27Xa55X36a33Xb18X2a6X3a6b4a2b4a6X4c75X11b6X3b21Xa35X"
  "85X34a52X3a6X2a7b4ab4a5X5c73X15b4X3b22Xa34X"
  "9Xa78X31a53X11a8b5a7X5c73X17b2X5b22Xa32X"
  "89X2aX27a53X20b3a8X5c72X25b55X"
  "90X2a2X26a53X20b2a9X5c70X28b22Xa30X"
  "90X2a2X26a54X18b3a9X5c8Xb60X31b16Xc34X"
  "Xa89X3aX25a55X18b3a8X5c7Xb59X34b16Xca32X"
  "91X2aX25a56X18b4a8X4c67X35b49X"
  "91X3aX22a59X18b3a9X3c67X37b47X"
  "91X3aX20a62X17b3a10X2c67X38b46X"
  "92X2a2X19a63X16b2a12Xc68X38b45X"
  "92X2aX19a64X17ba81X39b44X"
  "54Xa38X2aX19a64X17b82X39b43X"
  "93X22a65X16b82X40b42X"
  "94X21a67X14b84X38b14Xa27X"
  "94X20a68X14b84X38b42X"
  "94X20a70X12b85X38b41X"
  "94X19a71X11b87X9b9X19b41X"
  "87Xa7X14aX3a72X10b87X9b11X16b42X"
  "95X15a76X2b95X3b16XbX14b42X"
  "95X16a196X11b23Xb18X"
  "95X16a197X10b24Xb17X"
  "95X16a198X9b25X2b15X"
  "95X12a205XbX2b27X4b13X"
  "96X12a235X4b13X"
  "96X10a239X2b13X"
  "97X10a208X4b24X2bXb13X"
  "97XaX4aX3a209X4b22X4b14X"
  "99X8a210X3b21X4b15X"
  "99X8a211X2b20X4b16X"
  "99XaX5a234X4b16X"
  "98XaX6a235X2b17X"
  "100X7a234Xb18X"
  "99X8a253X"
  "99XaX6a253X"
  "100X6a254X"
  "100XaX5a8XbXb242X"
  "102X5a253X"
  "105Xa2Xa251X"
  "104Xa2X4a249X"
  "106X4a30Xb219X"
  "360X"
  "360X"
  "360X"
  "360X"
  "360X"
  "360X"
  "360X"
  "360X"
  "360X"
  "360X"
))
