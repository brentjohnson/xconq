;;; American names tend to be a hodgepodge of Indian, Spanish, English, etc,
;;; sometimes even mixed together, and usually distorted to boot.

;; (should finish this one)
(namer american-place-names-random (grammar root 9
  (root (or amerindian-name
	    english-name
	    spanish-name
	))
  (amerindian-name ("?"))
  (english-name ("?"))
  (spanish-name (or saint-name spanish-word ("los " spanish-word)))
  (saint-name ("San " (or jose francisco lucas joao juan luis miguel)))
))

(namer american-place-names (random
  ; AL  
  "Boaz" "Center Point" "Creola" "Hackleburg" "New Hope" "Stotesville"
  ; AK  
  "Barrow" "Coldfoot" "Fort Yukon" "Kaktovik" "Naknek" "Sitka"
  ; AZ  
  "Benson" "Bisbee" "Gila Bend" "Tempe" "Tombstone" "Turkey Flat"
  "Tuba City" "Wide Ruins"
  ; AR  
  "Fayetteville" "Hackett" "Metalton" "Oil Trough" "Pine Bluff" "Texarkana"
  ; CA  
  "Anaheim" "Arcata" "Barstow" "Burnt Ranch" "Calexico" "Chula Vista"
  "Death Valley Junction" "Eel Rock" "Fresno"
  "Fort Bragg" "Geyserville" "Gilroy" "Glendale" "Granada Hills"
  "Hemet" "Leucadia" "Lompoc" "Los Altos"
  "Madera" "Milpitas" "Mormon Bar" "Morro Bay" "Oroville"
  "Pescadero" "Petaluma" "Pinole" "Placerville" "Pumpkin Center" 
  "Scotts Valley" "Toms Place" "Ukiah" "Whiskeytown" "Yreka"
  ; CO  
  "Estes Park" "Greeley" "Las Animas" "Nederland" "Silver Plume"
  ; CT  
  "Danbury" "East Norwalk" "Moosup" "Old Lyme" "Upper Stepney" "Wallingford"
  ; DE
  "Argos Corner" "Christiana" "Kitts Hummock" "Lowes Crossroads" "Pepperbox"
  ; FL
  "Big Pine Key" "Daytona Beach" "Estiffanulga"
  "Key Biscayne" "Kissimmee" "Leonia"
  "Ocala" "Pahokee" "Panacea" "Sink Creek" "Wewahitchka" "Yeehaw Junction"
  ; GA  
  "Americus" "Dacula" "Dixieion" "Fowlstown" "Hinesville" "Statesboro"
  ; HW  
  "Hilo" "Hoolehua" "Kaneohe" "Laupahoehoe" "Lihue" "Pahoa" "Waimea"
  ; ID  
  "Kootenai" "Malad City" "Mountain Home"
  "Pocatello" "Smelterville" "Soda Springs"
  ; IL  
  "Aroma Park" "Farmer City" "Goreville" "Illiopolis" "Kankakee"
  "Mascoutah" "Metamora" "Metropolis" "New Boston" "Peoria" "Pontoon Beach"
  "Romeoville" "Skokie" "Teutopolis" "Urbana" "Waukegan"
  ; IN  
  "Bloomington" "Etan Green" "Fort Wayne" "French Lick" "Kokomo"
  "Loogootee" "Muncie" "Needmore" "Ogden Dunes" "Oolitic" "Star City" "Wabash"
  ; IA  
  "Coon Rapids" "Correctionville" "Council Bluffs" "Grinnell" "Grundy Center"
  "Lost Nation" "Oskaloosa" "Ossian" "Sac City" "Storm Lake"
  ; KA  
  "Coffeyville" "Countryside" "Greeley" "Grouse Creek" "Half Mound"
  "Leavenworth" "Mankato" "Overland Park" "Pretty Prairie" "Topeka"
  ; KY  
  "Big Clifty" "Cloverport" "Dog Walk" "Druid Hills" "Fancy Farm" 
  "Hardburly" "Hardshell" "Horse Cave" "New Hope" "Owensboro"
  "Pine Knot" "Pleasureville" "Science Hill" "Sublimity City" "Watergap" 
  ; LA  
  "Arnaudville" "Bayou Goula" "Cut Off" "Frogmore" "Hackberry" "Lutcher" 
  "Moss Bluff" "Natchitoches" "Ponchatoula" "Waggaman"
  ; ME  
  "Veazie" "Madawaska" 
  ; MD  
  "Bestgate" "College Park" "Frostburg" "Pocomoke City" 
  "Port Deposit" "Pumphrey" "Tammany Manor"
  "Weems Creek" "Whiskey Bottom" "Hack Point"
  ; MA  
  "Assinippi" "Buzzards Bay" "Dorothy Pond" "Hopkinton" 
  "Housatonic" "Pigeon Cove" "Swampscott" "Gloucester"
  "Hyannis Port" "Ipswich" "Boxford"
  ; MI  
  "Bad Axe" "Brown City" "Cassopolis" "New Buffalo" 
  "Petoskey" "Ishpeming" "Ypsilanti" "Saugatuck" 
  ; Michigan UP (from Sandra Loosemore)  
  "Skanee" "Bruce Crossing" "Baraga" "Germfask" 
  "Assinins" "Tapiola" "Gaastra" "Bete Grise" 
  ; MN  
  "Ada" "Blue Earth" "Brainerd" "Eden Valley"  
  "Lino Lakes" "New Prague" "Sleepy Eye" "Waconia"  
  ; MS  
  "Bogue Chitto" "Buckatunna" "Guntown" "Picayune" 
  "Red Lick" "Senatobia" "Tie Plant" "Yazoo City"  
  ; MO  
  "Bourbon" "Doe Run" "Hayti" "Humansville" 
  "Lutesville" "Moberly" "New Madrid" "Peculiar" 
  "Sappington" "Vandalia"  
  ; MT  
  "Big Sandy" "Hungry Horse" 
  "Kalispell"  "East Missoula"
  ; NE
  "Hershey" "Loup City" 
  "Minatare" "Wahoo"  "Grainfield"
  ; NV  
  "Winnemucca" "Tonopah" "Jackpot"  
  ; NH  
  "Littleton" "Winnisquam"  
  ; NJ  
  "Cheesequake" "Freewood Acres"
  "Forked River" "Hoboken" "Succasunna"  
  "Maple Shade" "New Egypt" "Parsippany" "Ship Bottom"  
  ; NM  
  "Adobe Acres" "Cloudcroft" "Ruidoso" "Toadlena"  
  "Los Padillos" "Ojo Caliente" 
  ; NY  
  "Angola on the Lake" "Podunk" "Chili Center"
  "Aquebogue" "Muttontown" "Hicksville" 
  "Hoosick Falls" "Nyack"
  "Painted Post" "Peekskill" "Portville"  
  "Ronkonkoma" "Wappingers Falls" 
  "Sparrow Bush" "Swan Lake"
  ; NC  
  "Altamahaw"
  "Biltmore Forest" "Boger City" "Granite Quarry"  
  "High Shoals" "Lake Toxaway"
  "Scotland Neck" "Hiddenite" 
  "Mocksville" "Yadkinville" "Nags Head" 
  "Kill Devil Hills" "Rural Hall"  
  ; ND  
  "Cannon Ball" "Hoople" "Zap"  
  ; OH  
  "Academia" "Arcanum" "Blacklick Estates" "Blue Ball"  
  "Crooksville" "Dry Run" "Flushing" "Gratis"  
  "Lithopolis" "Mingo Junction" "Newton Falls"
  "New Straitsville" "Painesville" "Pepper Pike" 
  "Possum Woods" "Sahara Sands"  
  ; OK  
  "Bowlegs" "Broken Arrow" "Fort Supply" "Drumright" 
  "Dill City" "Okay" "Hooker"  
  ; OR  
  "Condon" "Happy Valley" "Drain" "Junction City" 
  "Molalla" "Philomath" "Tillamook" "Wankers Corner"
  ; PA  
  "Atlasburg" "Beaver Meadows" "Birdsboro" "Daisytown" 
  "Fairless Hills" "Fairchance" "Kutztown" "Erdenheim" 
  "Hyndman" "Pringle" "Scalp Level" "Slickville" 
  "Zelienople" "Sugar Notch" "Toughkenamon" "Throop" 
  "Tire Hill" "Wormleysburg" "Oleopolis"
  ; RI  
  "Woonsocket" "Pawtucket"
  ; SC  
  "Due West" "Ninety Six" 
  "Travelers Rest" "Ware Shoals"  
  ; SD  
  "Deadwood" "Lower Brule" 
  "New Underwood" "Pickstown" 
  "Plankinton" "Tea" "Yankton"  
  ; TN  
  "Berry's Chapel" "Bulls Gap" "Cornersville" "Counce" 
  "Gilt Edge" "Grimsley" "Malesus" "Soddy-Daisy"  
  ; TX  
  "Bastrop" "New Braunfels" "Harlingen" "Dimock" 
  "Devils Elbow" "North Zulch" "Llano" "Fort Recovery" 
  "Arp" "Bovina" "Cut and Shoot" "College Station" 
  "Grurer" "Iraan" "Leming" "Harlingen" 
  "Muleshoe" "Munday" "Kermit" "La Grange" 
  "Ropesville" "Wink" "Yoakum" "Sourlake"  
  ; UT  
  "Delta" "Moab" "Nephi" "Loa" 
  "Moroni" "Orem" "Tooele" "Sigurd" 
  ; VT  
  "Bellows Falls" "Chester Depot" "Winooski"  
  ; VA  
  "Accotink" "Ben Hur" "Ferry Farms" "Disputanta" 
  "Dooms" "Sleepy Hollow" "Max Meadows" "Goochland" 
  "Rural Retreat" "Sandston" "Stanleytown"
  "Willis Wharf" "Stuarts Draft" 
  ; WA  
  "Black Diamond" "Carnation" "Cle Elum" "Cosmopolis" 
  "Darrington" "Enumclaw" "Forks" "Goose Prairie" 
  "Navy Yard City" "La Push" "Soap Lake" "Walla Walla" 
  "Sedro Woolley" "Pe Ell" "Ruston"  
  ; WV  
  "Barrackville" "Pocatalico" "Fort Gay" "Big Chimney" 
  "Nutter Fort" "Hometown" "Nitro" "Triadelphia" 
  "Star City"  
  ; WI  
  "Combined Lock" "Coon Valley" "Black Earth"
  "New Holstein" "Little Chute" "Wisconsin Dells"
  "Random Lake" "Sheboygan" "Nauwatosa"  
  ; WY  
  "East Thermopolis" "Fort Washakie" "Paradise Valley" 
  ))

(namer argentinian-place-names (random
  "Bahia Blanca" "Buenos Aires" "Chivilcoy" "Corrientes" "General Roca"
  "La Plata" "Mendoza" "Rio Gallegos" "Salta" "Viedma"
  ))

(namer bolivian-place-names (random
  "Atocha" "Cochabamba" "Corocoro" "Huarina" "La Lava" "La Paz"
  "Potosi" "San Borja" "Sucre" "Tarija" "Tupiza" "Yata" "Yotausito"
  ))

(namer brazilian-place-names (random
  "Aracaju" "Ariquemes" "Belem" "Brasilia"
  "Campo Grande" "Cuiari" "Curitiba" "Curvelo" "Erexim" "Estancia"
  "Fortaleza" "Goiania" "Gurupi" "Itaquari" "Jatai"
  "Manaus" "Recife" "Rio Branco" "Rio de Janeiro"
  "Sao Luis" "Sao Paulo" "Uberaba" "Varginha" "Vila Velha"
  ))

(namer canadian-place-names (random
  "Calgary" "Codroy Pond" "Corner Brook" "Duck Bay" "Edmonton"
  "Halifax" "Inuvik" "Kamloops" "Kitchener" "Medicine Hat" "Mississauga"
  "Moncton" "Montreal" "Moose Jaw" "Ottawa" "Powell River" "Quebec"
  "Red Deer" "Regina" "Saskatoon" "Sudbury"
  "Thunder Bay" "Toronto" "Trois-Rivieres" "Vancouver"
  "Whitehorse" "Winnipeg" "Yellowknife"
  ))

(namer chilean-place-names (random
  "Angol" "Antofagasta" "Arica" "Concepcion"
  "Iquique" "Los Muermos"
  "Osorno" "Ovalle" "Puerto Montt" "Punta Arenas" "Rancagua"
  "Santiago" "Tocopilla" "Valdivia" "Vallenar" "Valparaiso"
  ))

(namer colombian-place-names (random
  "Barranquilla" "Bogota" "Cali" "Cucuta" "El Banco"
  "Ibague" "Loreto Macagua"
  "Manizales" "Medellin" "Neiva" "Pasto" "Quibdo" "Riohacha"
  "Santa Marta" "Tumaco"
  ))

(namer costa-rican-place-names (random
  "Alajuela" "Boruca"
  "Golfito" "Guapiles" "Heredia" "Los Chiles"
  "Palmar Sur" "Puerto Limon"
  "San Jose" "Sardinal" "Zarcero"
  ))

(namer cuban-place-names (random
  "Bayamo" "Camaguey" "Cienfuegos"
  "Guantanamo" "Holguin" "La Habana" "Las Tunas"
  "Matanzas" "Moron" "Pinar del Rio" "Placetas"
  "Santiago de Cuba" "Tunas de Zaza" "Vertientes" "Zulueta"
  ))

(namer ecuadorean-place-names (random
  "Ambato" "Azoques" "Babahoyo" "Cuenca"
  "Guayaquil" "Ibarra"
  "Machala" "Manta" "Quevedo" "Quito" "Riobamba"
  "Sangolqui" "Tulcan" "Yaupi"
  ))

(namer guatemalan-place-names (random
  "Chiquimula" "Chuntuqui" "Escuintla"
  "Guatemala" "Jutiapa" "Los Amates"
  "Poptun" "Puerto Barrios"
  "Santa Eulalia" "Tacana" "Zacapa"
  ))

(namer honduran-place-names (random
  "Azacualpa" "Choluteca" "Comayagua" "El Progreso"
  "Jesus de Otoro"
  "Minas de Oro"
  "San Pedro Sula" "Santa Ana" "Tegucigalpa" "Tela" "Yoro"
  ))

(namer haitian-place-names (random
  "Cap-Haitien" "Gonaives" "Hinche" "Jeremie" "Le Limbe" "Les Cayes"
  "Petionville" "Port-au-Prince" "Saint-Marc" "Trou-du-Nord"
  ))

(namer jamaican-place-names (random
  "Alligator Pond" "Annotto Bay" "Duncans"
  "Kingston" "Mandeville" "May Pen" "Montego Bay" "Morant Bay"
  "Negril" "Ocho Rios" "Portmore" "Spanish Town"
  ))

(namer mexican-place-names (random
  "Acapulco" "Cancun" "Chihuahua" "Cozumel" "Durango" "El Sueco"
  "Guadalajara" "Hermosillo" "Irapuato" "Leon"
  "Matamoros" "Mazatlan" "Merida" "Monterrey" "Nogales" "Oaxaca" "Puebla"
  "Rio Bravo" "Saltillo" "Tampico" "Tijuana" "Torreon" "Veracruz" "Xalapa"
  ))

(namer nicaraguan-place-names (random
  "Bluefields" "Boaco" "Chinandega" "Corinto" "Esteli"
  "Granada" "Juigalpa" "Leon"
  "Managua" "Masaya" "Matagalpa" "Ocotal" "Puerto Sandino" "Rivas"
  "San Miguelito" "Tipitapa"
  ))

(namer panamanian-place-names (random
  "Balboa" "Bocas del Toro" "Chitre" "Colon" "David"
  "Guabito" "La Chorera"
  "Nombre de Dios" "Panama City" "Paraiso"
  "Taboga" "Tonosi" "Yaviza"
  ))

(namer paraguayan-place-names (random
  "Asuncion" "Caaguazu" "Capiata" "Ciudad del Este" "Encarnacion"
  "Fortin Florida" "Horqueta" "Lambare"
  "Nueve Germania" "Pedro Juan Caballero" "Pozo Colorado" "Puerto Mihanovich"
  "Villarica" "Ype Jhu" "Ybycui"
  ))

(namer peruvian-place-names (random
  "Arequipa" "Ayacucho" "Callao" "Chimbote" "Cusco" "El Tambo"
  "Ica" "Iquitos" "Huanuco" "Lima"
  "Machu Picchu" "Moyobamba" "Nazca" "Piura" "Pucalipa"
  "Sitabamba" "Sullana" "Tacna" "Trujillo" "Tumbes" "Vitarte"
  ))

(namer salvadoran-place-names (random
  "Acajutla" "Chalchuapua" "Cojutepeque" "Delgado" "El Congo"
  "Intipuca" "Izalco" "Lolotique"
  "Mejicanos" "Metapan"
  "San Salvador" "Santa Ana" "Sonsonate" "Soyapango" "Usulutan"
  ))

(namer uruguayan-place-names (random
  "Algorta" "Artigas" "Canelones" "Durazno" "Fray Marcos"
  "Greco" "Ismael Cortinas" "Las Piedras"
  "Melo" "Mercedes" "Minas" "Montevideo" "Paysandu" "Piedra Sola" "Rivera"
  "Salto" "Tres Arboles" "Valle Eden"
  ))

(namer venezuelan-place-names (random
  "Barquisimeto" "Baruta" "Calabozo" "Caracas" "Ciudad Guayana" "Coro"
  "Guarenas" "La Tiama" "La Urbana" "Limoncito" "Los Teques"
  "Maracaibo" "Maracay" "Merida" "Petare" "Pozuelos" "Punto Fijo"
  "Tucupita" "Valle de la Pascua"
  ))
