(game-module "anc-near-east"
  (title "Ancient Near East")
  (blurb "The Ancient Near East in 1500 BC. Nine emerging civilizations struggle for power.")
  (base-module "advances")
  (version "1.0")
  (variants
    (world-seen true)
    (see-all true)
    (sequential true)
  )
  (instructions (
    "Research scientific advances in order to produce more powerful units."
    "Make colonizers that can build new cites and colonize all available land. "
    "Do not forget to make military units, or suffer the consequences!"
  ))
)

(set indepside-has-ai true)
(set indepside-can-research false)

(include "t-near-east")

(table independent-density
	(city cell-t* 0)
)

(add colonizers start-with 0)


;;;	SIDES

;	This is to override independent-names in order to avoid name duplication (some
;	names in independent names are already used for existing cities below). 

(add tribe namer "short-generic-names")
(add village namer "short-generic-names")
(add city namer "short-generic-names")

(set sides-min 9)

(side 1
    (name "Sumer")
    (plural-noun "Sumerians")	
    (adjective "Sumerian")	
#|    (unit-namers (tribe "sumerian-names")
    			(village "sumerian-names")
    			(city "sumerian-names"))
|#
)

(side 2
    (name "Akkad") 
    (plural-noun "Akkadians")	
    (adjective "Akkadian")	
#|    (unit-namers (tribe "akkadian-names")
    			(village "akkadian-names")
    			(city "akkadian-names"))
|#
)
    			
(side 3
    (name "Assyria")
    (plural-noun "Assyrians")	
    (adjective "Assyrian")	
#|    (unit-namers (tribe "assyrian-names")
    			(village "assyrian-names")
    			(city "assyrian-names"))
|#
)

(side 4
    (name "Elam")
    (plural-noun "Elamites")	
    (adjective "Elamite")	
#|    (unit-namers (tribe "elamite-names")
    			(village "elamite-names")
    			(city "elamite-names"))
|#
)

(side 5
    (name "Mitanni")
    (plural-noun "Mitannis")	
    (adjective "Mitanni")	
#|    (unit-namers (tribe "mitanni-names")
    			(village "mitanni-names")
    			(city "mitanni-names"))
|#
)

(side 6
    (name "Hatti")
    (plural-noun "Hittites")	
    (adjective "Hittite")	
#|    (unit-namers (tribe "hittite-names")
    			(village "hittite-names")
    			(city "hittite-names"))
|#
)

(side 7
  (name "Lower Egypt")
    (plural-noun "Lower Egyptians")		
    (adjective "Lower Egyptian")		
#|    (unit-namers (tribe "lower-egyptian-names")
    			(village "lower-egyptian-names")
    			(city "lower-egyptian-names"))
|#
)

(side 8
    (name "Upper Egypt")
    (plural-noun "Upper Egyptians")		
    (adjective "Upper Egyptian")		
#|    (unit-namers (tribe "upper-egyptian-names")
    			(village "upper-egyptian-names")
    			(city "upper-egyptian-names")) 
|#
)

(side 9
    (name "Achaea")
    (plural-noun "Achaeans")		
    (adjective "Achaean")		
#|    (unit-namers (tribe "greek-names")
    			(village "greek-names")
    			(city "greek-names"))
|#
)

 ;;;	 CITIES

;	Independent cities

(city 201 244 0 (n "Alalakh"))
(city 207 238 0 (n "Ebla"))
(city 208 242 0 (n "Haleb"))
(city 209 230 0 (n "Hama"))
(city 290 247 0 (n "Hasanlu"))
(city 276 47 0 (n "Mero‘"))
(city 246 60 0 (n "Napata"))
(city 219 220 0 (n "Nashala"))
(city 210 223 0 (n "Qadesh"))
(city 213 226 0 (n "Qatna"))
(city 226 225 0 (n "Tadmor"))
(city 200 235 0 (n "Ugarit"))
(city 205 250 0 (n "Urshu"))

(city 257 222 0 (n "Hindanu"))
(city 251 226 0 (n "Mari"))
(city 245 231 0 (n "Terqa"))

(city 24 279 0 (n "Dodona"))
(city 83 279 0 (n "Ilion"))
(city 82 234 0 (n "Kydonia"))
(city 94 233 0 (n "Knossos"))
(city 91 230 0 (n "Phaistos"))
(city 105 230 0 (n "Praisos"))
(city 72 287 0 (n "Samothrake"))
(city 93 242 0 (n "Thera"))

;	Sumerian cities

(city 316 200 1 (n "Adab"))
(city 322 195 1 (n "Bad-Tibara"))
(city 326 186 1 (n "Eridu"))
(city 323 197 1 (n "Girsu"))
(city 312 196 1 (n "Isin"))
(city 317 205 1 (n "Kesh"))
(city 325 192 1 (n "Kutalla"))
(city 327 195 1 (n "Lagash"))
(city 321 193 1 (n "Larsa"))
(city 311 206 1 (n "Malgium"))
(city 311 201 1 (n "Nippur"))
(city 315 197 1 (n "Shuruppak"))
(city 330 194 1 (n "Sirara"))
(city 320 198 1 (n "Umma"))
(city 326 189 1 (n "Ur"))
(city 317 193 1 (n "Uruk"))

;	Akkadian cities

(city 269 225 2 (n "Ana"))
(city 300 221 2 (n "Awal"))
(city 300 206 2 (n "Babil"))
(city 300 203 2 (n "Borsippa"))
(city 303 201 2 (n "Dilbat"))
(city 307 200 2 (n "Eresh"))
(city 300 217 2 (n "Eshnunna"))
(city 281 214 2 (n "Hit"))
(city 304 204 2 (n "Kish"))
(city 301 209 2 (n "Kutu"))
(city 306 197 2 (n "Marad"))
(city 307 205 2 (n "Mashkan-Shapir"))
(city 298 223 2 (n "Me-Turan"))
(city 303 211 2 (n "Neribtum"))
(city 285 214 2 (n "Rapiku"))
(city 297 214 2 (n "Shaduppum"))
(city 296 210 2 (n "Sippar"))
(city 300 214 2 (n "Tutub"))
(city 275 218 2 (n "Yabliya"))

;	Assyrian cities

(city 260 247 3 (n "Apku"))
(city 277 243 3 (n "Arbil"))
(city 275 234 3 (n "Assur"))
(city 275 237 3 (n "Ekallatum"))
(city 274 241 3 (n "Kalhu"))
(city 256 241 3 (n "Karana"))
(city 270 243 3 (n "Nineveh"))
(city 281 238 3 (n "Qabra"))
(city 260 242 3 (n "Qatara"))
(city 251 253 3 (n "Shubat-Enlil"))
(city 268 248 3 (n "Talmusa"))

;	Elamite cities

(city 305 224 4 (n "Alman"))
(city 312 212 4 (n "Der"))
(city 341 201 4 (n "Susa"))
(city 334 203 4 (n "Urua"))

;	Mitanni cities

(city 224 237 5 (n "Abattum"))
(city 240 250 5 (n "Ashnakkum"))
(city 221 246 5 (n "Badna"))
(city 222 255 5 (n "Elahut"))
(city 219 241 5 (n "Emar"))
(city 216 257 5 (n "Hahhum"))
(city 215 253 5 (n "Halpi"))
(city 225 247 5 (n "Harran"))
(city 247 247 5 (n "Kahat"))
(city 217 248 5 (n "Kargamish"))
(city 211 254 5 (n "Kistan"))
(city 220 260 5 (n "Nihriya"))
(city 245 238 5 (n "Qattuna"))
(city 251 240 5 (n "Razama"))
(city 244 235 5 (n "Saggaratum"))
(city 244 242 5 (n "Tabate"))
(city 228 238 5 (n "Tuttul"))
(city 240 253 5 (n "Urkish"))
(city 236 246 5 (n "Washukanni"))
(city 226 243 5 (n "Zalpha"))

;	Hittite cities

(city 172 272 6 (n "Ankuwa"))
(city 188 249 6 (n "Ataniya"))
(city 168 288 6 (n "Durhumit"))
(city 162 278 6 (n "Hattusas"))
(city 172 253 6 (n "Hubishna"))
(city 199 264 6 (n "Hurama"))
(city 178 266 6 (n "Kanesh"))
(city 185 274 6 (n "Karahna"))
(city 183 283 6 (n "Komana"))
(city 190 261 6 (n "Kummanni"))
(city 196 258 6 (n "Mahama"))
(city 201 258 6 (n "Markash"))
(city 210 265 6 (n "Melid"))
(city 161 264 6 (n "Purushkanda"))
(city 207 270 6 (n "Shamuha"))
(city 160 282 6 (n "Tawiniya"))
(city 198 271 6 (n "Tilgarim"))
(city 172 290 6 (n "Tuhpiya"))
(city 176 255 6 (n "Tuwana"))
(city 164 266 6 (n "Ulama"))
(city 171 264 6 (n "Washhaniya"))
(city 179 259 6 (n "Washushana"))
(city 171 295 6 (n "Zalpa"))

;	Lower Egyptian cities

(city 178 191 7 (n "Bah"))
(city 180 185 7 (n "Bast"))
(city 167 190 7 (n "Behdet"))
(city 176 188 7 (n "Busiris"))
(city 182 191 7 (n "Djannet"))
(city 173 194 7 (n "Hebyt"))
(city 182 172 7 (n "Henen-Nesut"))
(city 170 187 7 (n "Imu"))
(city 181 181 7 (n "Iunu"))
(city 179 180 7 (n "Khem"))
(city 175 184 7 (n "Mefket"))
(city 183 177 7 (n "Mennufer"))
(city 181 167 7 (n "Per-Medjed"))
(city 184 184 7 (n "Per-Sopdu"))
(city 170 193 7 (n "Per-Wadjit"))
(city 182 188 7 (n "Pi-Riamsese"))
(city 162 191 7 (n "Rakote"))
(city 188 188 7 (n "Sile"))
(city 184 168 7 (n "Teudjoi"))
(city 187 185 7 (n "Tjeku"))
(city 172 189 7 (n "Zau"))

;	Upper Egyptian cities

(city 207 141 8 (n "Abedju"))
(city 190 157 8 (n "Akhetaten"))
(city 221 95 8 (n "Buhen"))
(city 226 122 8 (n "Djeba"))
(city 197 150 8 (n "Djew-Qa"))
(city 219 139 8 (n "Gebtu"))
(city 212 140 8 (n "Hut-Sekhem"))
(city 201 148 8 (n "Ipu"))
(city 186 159 8 (n "Khmun"))
(city 183 162 8 (n "Menat-Khufu"))
(city 221 105 8 (n "Miam"))
(city 225 126 8 (n "Nekheb"))
(city 224 125 8 (n "Nekhen"))
(city 218 137 8 (n "Nubt"))
(city 228 119 8 (n "Nubt"))
(city 216 132 8 (n "Per-Hathor"))
(city 227 112 8 (n "Pselqet"))
(city 189 154 8 (n "Qis"))
(city 229 115 8 (n "Swenet"))
(city 220 129 8 (n "Ta-Senet"))
(city 215 141 8 (n "Tantere"))
(city 215 136 8 (n "Tjamet"))
(city 204 145 8 (n "Tjenu"))
(city 218 134 8 (n "Waset"))
(city 193 151 8 (n "Zawty"))

;	Achaean cities

(city 30 274 9 (n "Ambrakia"))
(city 57 252 9 (n "Amyklai"))
(city 68 259 9 (n "Athenai"))
(city 65 265 9 (n "Chalkis"))
(city 89 263 9 (n "Chios"))
(city 51 267 9 (n "Delphi"))
(city 42 258 9 (n "Elis"))
(city 104 259 9 (n "Ephesos"))
(city 109 249 9 (n "Halikarnassos"))
(city 48 256 9 (n "Heraia"))
(city 31 267 9 (n "Ithaka"))
(city 40 267 9 (n "Kalydon"))
(city 68 241 9 (n "Kythera"))
(city 44 277 9 (n "Larisa"))
(city 28 271 9 (n "Leukas"))
(city 63 260 9 (n "Megara"))
(city 79 245 9 (n "Melos"))
(city 106 254 9 (n "Miletos"))
(city 59 256 9 (n "Mykenai"))
(city 89 270 9 (n "Mytilene"))
(city 90 249 9 (n "Naxos"))
(city 56 266 9 (n "Orchomenos"))
(city 55 271 9 (n "Orcos"))
(city 44 263 9 (n "Patrai"))
(city 51 248 9 (n "Pylos"))
(city 101 256 9 (n "Samos"))
(city 98 264 9 (n "Smyrna"))
(city 61 264 9 (n "Thebe"))

;	Unused Achaean cities

#|	
(city 67 256 9 (n "Aigina"))
(city 48 263 9 (n "Aigion"))
(city 75 260 9 (n "Karystos"))
(city 114 245 9 (n "Knidos"))
(city 108 247 9 (n "Kos"))
(city 59 258 9 (n "Korinthos"))
(city 99 231 9 (n "Lyktos"))
(city 43 261 9 (n "Pharai"))
(city 48 276 9 (n "Pherai"))
(city 121 242 9 (n "Rhodos"))
(city 35 270 9 (n "Stratos"))
(city 88 233 9 (n "Sybrita"))
(city 62 256 9 (n "Tiryns"))
|#

