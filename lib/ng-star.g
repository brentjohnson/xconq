; Star names generator
; based on catalogue of fixed stars from Swiss Ephemeris

(namer star-names (grammar root 20
   (root (or 5 (greek-letter " " c-a) 2 a-a))
   (greek-letter (or Alpha Beta Gamma Delta Epsilon
                     Zeta Eta Theta Iota Kappa Lambda Mu Nu Xi
                     Pi Rho Sigma Tau Upsilon Phi Chi Psi Omega))
   (c-a (or ("Capriu" c-ap) ("Monocu" c-as) ("Scutum" c-af)
       5 "Cygnus" ("Sagiu" c-ap) ("Herc" c-z) "Librado"
       2 ("Hydra" c-ai) ("Micron" c-l) ("Pisco" c-c) ("Choce" c-az)
       "Ara" "Lupus" ("Microl" c-aq) ("Crati" c-e) ("Serpe" c-au)
       ("Plat" c-ba) ("Cancer" c-ax) "Leo" "Libra" ("Aquari" c-d)
       ("Pisca" c-m) ("Norma Beren" c-p) ("Gemin" c-i)
       ("Crate" c-bc) 2 ("Trian" c-av) ("Circ" c-z) 2 ("Draco" c-c)
       ("Capris" c-x) "Apus" ("Centar" c-s) ("Auris" c-x)
       ("Pisce" c-az) ("Scul" c-f) ("Scorn" c-ak) ("Orion" c-l)
       ("Ephem" c-w) "Lepus" "Corvus" ("Caprin" c-i) ("Micros" c-ah)
       2 ("Dracu" c-as) ("Carig" c-bd) "Grus" ("Coma Beren" c-p)
       2 "Tria" "Pyxis" "Scor" ("Spec" c-an) "Teleus"
       ("Eridalis" c-x) ("Carin" c-i) ("Puppis" c-x) ("Chamer" c-ax)
       ("Vola" c-y) 2 ("Octa" c-v) ("Phoeni" c-at) ("Aries" c-o)
       ("Reti" c-e) ("Musc" c-al) ("Antil" c-ac) ("Tucan" c-av)
       ("Bunnic" c-aj) ("Boote" c-bc) ("Cariu" c-ap) ("Incul" c-f)
       "Coma" "Aquila" "Lynx" ("Aquilan" c-av) 2 ("Coro" c-b)
       ("Andra" c-ai) 3 ("Cani" c-at) ("Horom" c-j) 2 "Ursa"
       ("Horol" c-aq) "Colum" ("Andro" c-b) ("Camel" c-be)
       ("Horon" c-l) ("Caelu" c-ab) ("Laces" c-o) ("Pyxis Vena" c-g)
       "Tuca" ("Cephe" c-r) "Pega" ("Caprig" c-bd) ("Antic" c-aj)
       "Crux" "Succubus" "Epheus" "Lyrado" ("Teleo" c-k)
       ("Aurin" c-i) "Bunni" ("Centan" c-av) 2 "Indus" "Virgo"
       "Norma" ("Cephi" c-ad) ("Vela" c-y) ("Auric" c-aj)
       ("Equul" c-f) "Columba" ("Teles" c-o) ("Delpt" c-ae)
       ("Aquard" c-n) "Bunnix" ("Exco" c-c) ("Puppiu" c-ap)
       ("Cances" c-o) "Cetus" ("Caris" c-x) "Centa" ("Ophi" c-ad)
       "Incubus" ("Microm" c-j) ("Camed" c-q) "Cratypus"
       ("Chocu" c-as) ("Vulpt" c-ae) ("Bunnis" c-x) ("Forn" c-ak)
       ("Chamed" c-q) ("Succul" c-f) ("Horos" c-ah) 2 ("Octo" c-ar)
       ("Picto" c-ar) ("Eridan" c-av) ("Vulpe" c-au) ("Monoce" c-az)
       "Perseus" ("Cetum" c-af) ("Aurig" c-bd) ("Cassiop" c-ay)
       ("Auriu" c-ap) ("Vulph" c-h) ("Camer" c-ax) "Scutus"
       2 ("Sagit" c-aa) ("Delph" c-h) ("Caric" c-aj) 2 "Hydrus"
       "Lyra" "Phoens" "Centaurus" ("Delpe" c-au) "Pegasus" "Andrus"
       ("Scorp" c-ao) ("Serpi" c-bb) "Erida" ("Lacer" c-ax)
       ("Sexta" c-v) 2 ("Chamel" c-be) "Pavo" ("Capric" c-aj)
       ("Cassion" c-l) "Taurus" 2 ("Hydro" c-b) ("Oriop" c-ay)
       ("Caele" c-t) "Dorado" "Dora" ("Caelo" c-am)))
   (c-b (or ("l" c-aq) 2 ("n" c-l) ("m" c-j) 2 ("s" c-ah)))
   (c-c (or "" ("n" c-l) 2 ("p" c-ay) ("r" c-aw)))
   (c-d (or ("n" c-i) ("c" c-aj) 2 ("u" c-ap) ("g" c-bd) ("s" c-x)))
   (c-e (or ("l" c-ac) 2 ("c" c-aj)))
   (c-f (or ("pt" c-ae) 2 ("e" c-t) 4 ("u" c-ab) 4 ("a" c-y)
       ("ph" c-h) ("pe" c-au)))
   (c-g (or 4 "" "x" ("t" c-ba)))
   (c-h (or ("e" c-r) ("i" c-ad)))
   (c-i (or 2 "us" ("a" c-g) ("i" c-at)))
   (c-j (or ("er" c-ax) ("ed" c-q) ("el" c-be)))
   (c-k (or ("nquer" c-ax) "n" ("na" c-g)))
   (c-l (or 2 "" 2 ("a" c-g) ("quer" c-ax)))
   (c-m (or "" ("n" c-av)))
   (c-n (or ("alis" c-x) "a" ("an" c-av)))
   (c-o (or 5 "" ("c" c-al)))
   (c-p (or "s" 2 ("i" c-at)))
   (c-q (or ("alis" c-x) "a" ("an" c-av)))
   (c-r (or "us" ("m" c-w)))
   (c-s (or ("d" c-n) 2 ("i" c-d)))
   (c-t (or "us" 2 ("s" c-o) ("o" c-k)))
   (c-u (or "" ("n" c-i)))
   (c-v (or 2 "" "urus" 2 ("n" c-av) ("r" c-s)))
   (c-w (or ("er" c-ax) ("ed" c-q) ("el" c-be)))
   (c-x (or 6 "" (" Vena" c-g)))
   (c-y (or 7 "" ("n" c-av)))
   (c-z (or ("u" c-as) ("i" c-u)))
   (c-aa (or 2 ("tar" c-s) 2 "ta" 2 ("tan" c-av) 2 "taurus"))
   (c-ab (or 6 "mba" 6 "m"))
   (c-ac (or 2 "a" 2 ("an" c-av)))
   (c-ad (or ("u" c-ap) ("n" c-i)))
   (c-ae (or ("or" c-aw) ("op" c-ay)))
   (c-af (or 9 "" "ba"))
   (c-ag (or ("it" c-aa) ("iu" c-ap)))
   (c-ah (or "" ("c" c-al)))
   (c-ai (or "do" 3 ""))
   (c-aj (or ("e" c-az) 2 ("u" c-as) ("i" c-u) ("o" c-c)))
   (c-ak (or "us" ("a" c-g)))
   (c-al (or ("e" c-az) ("a" c-m) 2 ("o" c-c)))
   (c-am (or ("p" c-ay) ("g" c-ag)))
   (c-an (or 2 ("ul" c-f) 2 "ubus"))
   (c-ao (or ("e" c-au) ("i" c-bb)))
   (c-ap (or 3 "s" 3 ("m" c-af) "chus"))
   (c-aq (or ("og" c-ag) ("op" c-ay)))
   (c-ar (or ("p" c-ay) 2 ("r" c-aw)))
   (c-as (or 7 ("l" c-f) 2 "bus"))
   (c-at (or "" 2 ("c" c-aj) "x" 3 ("s" c-x)))
   (c-au (or ("c" c-an) ("n" c-p) ("ian" c-av) "ia"))
   (c-av (or 3 "s" 2 ("gul" c-f) "us" ("a" c-g)))
   (c-aw (or 2 "" ("p" c-ao) ("n" c-ak)))
   (c-ax (or ("tar" c-s) 2 "" "ta" "us" ("tan" c-av) ("o" c-b)
       ("i" c-d) "taurus"))
   (c-ay (or ("e" c-au) ("ard" c-n) "us" ("ari" c-d) 2 ("i" c-bb)))
   (c-az (or 3 ("r" c-ax) 2 ("s" c-o)))
   (c-ba (or ("e" c-bc) "ypus" ("i" c-e)))
   (c-bb (or 3 ("u" c-ap) ("s" c-x)))
   (c-bc (or ("r" c-ax) ("s" c-o)))
   (c-bd (or "asus" "a"))
   (c-be (or ("e" c-t) ("u" c-ab) ("o" c-am)))
   (a-a (or ("Bram" a-aq) ("Ankan" a-ci) ("Mimost" a-bb) "Alpherati"
       "Adhil" "Zaurus" 3 "Alshain" ("Alpherata" a-by) "Nekkah"
       "Tara" 2 ("Antak" a-db) ("Syrmak" a-db) "Regorealis"
       ("Talitha Bors" a-cs) "Mizar" "Ara" ("Scept" a-dj)
       ("Alaraz" a-dg) "Armus" "Phact" ("Dorn" a-ce) ("Matak" a-db)
       ("Lesats" a-dh) ("Lesata" a-by) ("Al Jab " a-l) ("Izam" a-aq)
       ("Mintau" a-cx) "Alzirr" ("Has" a-cr) ("Daba" a-bq) "Adares"
       2 ("Arkar" a-ay) 3 "Regulus" ("Adarg" a-ag) ("Prin" a-bs)
       ("Taraf" a-f) 3 ("Jabb" a-av) ("Dabh" a-r) "Angeta" "Ascella"
       ("Talitha Austra" a-cd) ("Minkalinan" a-ci) ("Sherato" a-ch)
       "Merops" ("Thubano" a-al) ("Tarab" a-cq) "Lesat"
       ("Alula Bora" a-cd) 2 ("For" a-af) ("Ke Kwan" a-ci)
       2 "Suhail" 2 ("Arkaa" a-bh) "Naos" ("Matar" a-ay)
       ("Al Kalb al Ra" a-bg) ("Almak" a-db) "Nusakis"
       2 ("Antar" a-ay) "Acrux" ("Akram" a-aq) ("Alphekkat" a-bn)
       ("Gemmal" a-bx) "Alpherath" ("Dziba" a-ck) "Pollux" "Agena"
       ("Krab" a-cq) 3 ("Steri" a-dd) 2 "Ukdah" "Alphard"
       ("Aldafa" a-g) ("Prijipata" a-by) "Nunki" "Asmidiske"
       4 "Rucha" "Alcyoni" ("Mintar" a-ay) "Seat" ("Dora" a-cd)
       "Manubrium" "Atria" ("Matam" a-aq) ("Alula Bors" a-cs)
       ("Almaa" a-bh) ("Alarap" a-aa) "Rukbah" ("Marka" a-aj)
       ("Minkaa" a-bh) "Char" "Arrai" ("Nusakr" a-bm) "Akra"
       2 "Altais" ("Skab" a-cq) ("Gach" a-v) ("Misam" a-aq) "Minkah"
       ("Alnair " a-bv) ("Alkalurope" a-bi) 2 "Arka" "Kitalpha"
       ("Prijipaten Kai" a-ap) ("Chertam" a-aq) ("Mirfat" a-bn)
       ("Aludrap" a-aa) 5 ("Zuben " a-cp) ("Etar" a-ay)
       ("Alpherats" a-dh) ("Aladfa" a-g) "Alfirk" ("Alphekkab" a-cq)
       "Aldera" ("Akrap" a-aa) ("Miaplac" a-di) 3 ("Jabi" a-de)
       "Alkalurophrah" ("Nekkar" a-ay) ("Bos" a-cc) 2 "Nodus"
       ("Phecdat" a-bn) 2 "Seginus" ("Bahama" a-br) "Muliphein"
       ("Cestak" a-db) "Alsuhail" ("Brab" a-cq) "Nekka"
       ("Bahamb" a-cy) ("Chertan" a-ci) "Achird" ("Al Kalba" a-df)
       ("Phecdar" a-ay) ("Zosa" a-bu) ("Cestar" a-ay) "Schedares"
       3 ("Jab " a-l) ("Kaus Bor " a-m) ("Tarae" a-j) "Mirfaljidhma"
       ("Talitha Born" a-ce) ("Acama" a-br) ("Adaro" a-e) "Alphar"
       2 ("Polaris" a-t) "Caleano" "Theemin" ("Fom" a-i)
       ("Alaraf" a-f) "Shedir" "Alya" "Sualocin" ("Krae" a-j)
       ("Mirzam" a-aq) "Gacrux" "Anser" "Masym" "Kaus Austrix"
       ("Acra" a-cd) ("Har" a-ay) 2 "Al Thalimaim" ("Schedari" a-ct)
       ("Elnat" a-bn) ("Skan" a-ci) ("Kraz" a-dg) ("Hat" a-bn)
       "Al Khab" ("Dorg" a-ag) "Thuban" "Alkes" ("Schedarg" a-ag)
       "Alrischa" "Alarai" 2 "Gienah" ("Dabb" a-av) 3 ("Jaba" a-bq)
       ("Mizaro" a-e) 2 "Kuma" ("Talitha Bor " a-m) ("Kurdal" a-cz)
       ("Al Khabdhilinan" a-ci) ("Al Jabr" a-bp) "Metallah"
       2 ("Ukdal" a-cz) ("Kajam" a-aq) "Alkaluropus" "Alcyone"
       "Asellus Bor" "Merakis" 2 ("Gianf" a-ab) "Acam"
       ("Benetnab" a-cq) "Alrai" "Zavijava" ("Adari" a-ct)
       ("Schedaro" a-e) 3 ("Pro" a-e) "Kerb" ("Mirfar" a-ay)
       "Talitha Borealis" ("Minkab" a-cq) 2 "Haedi" "Phacturus"
       ("Alula Borr" a-ba) 2 ("Antab" a-cq) "Sheliak" ("Alnas" a-cr)
       "Saiph" ("Maaz" a-dg) ("Cestau" a-cx) ("Akras" a-cr) "Merga"
       ("Mintak" a-db) "Al Pherg" "Tyl" ("Krac" a-au) "Shemali"
       ("Arrac" a-au) "Lab" 2 ("Cap" a-aa) ("Alphekkaa" a-bh)
       ("Mirfak" a-db) "Arct" "Alara" "Heze" "Acubens"
       ("Asellus Prim" a-k) 2 "Alchita" "Alaralis" ("Rukban" a-ci)
       ("Bota" a-by) ("Alsafi" a-bf) ("Krak" a-db) ("Era" a-cd)
       2 ("Yed Pr" a-ca) ("Gom" a-i) 3 "Jab" "Alula Borealis"
       "Cegin" ("Etan" a-ci) "Kissin" 2 "Scheat" ("Khamb" a-cy)
       ("Por" a-af) "Tolimaim" "Peacock" "Dab" 2 "Hya"
       ("Tayge" a-cv) ("Talitha Austru" a-ae) ("Syrmam" a-aq)
       ("Agenar" a-ay) ("Atlase" a-bk) ("Skad" a-ah) ("Difda" a-u)
       2 ("Almac" a-au) ("Etab" a-cq) ("Kras" a-cr) ("Taran" a-ci)
       ("Schedara" a-cd) "Alchiba" ("Haedus " a-bc) "Bellatrix"
       ("Arraz" a-dg) ("Kurda " a-bt) ("Al Jabh" a-r)
       ("Asellus Bor " a-m) "Kham" ("Sham" a-z) "Rukba"
       ("Sulafat" a-bn) 2 ("Antam" a-aq) "Phecda" "Sirr" 2 "Segin"
       ("Salme" a-cf) "Situla" "Merak" ("Brak" a-db) ("Mizarg" a-ag)
       "Thubanev" ("Al Hecka" a-aj) "Ankah" 2 ("Sar" a-ay)
       ("Dabi" a-de) "Alkalurops" "Acherna" ("Sirra" a-cd) "Sherati"
       ("Diademiatr" a-b) ("Peacocy" a-cn) ("Prijipato" a-ch)
       ("Kaus Bora" a-cd) ("Alula Aust" a-cu) ("Deneb Kai" a-ap)
       ("Alarak" a-db) ("Alula Bor " a-m) ("Zaura" a-bw) "Adar"
       "Arrah" ("Al Jabb" a-av) ("Gemmam" a-aq) "Sherat"
       ("Electru" a-ae) ("Arrae" a-j) ("Tarac" a-au) "Lesati"
       ("Nekkaa" a-bh) "Rukbalia" "Arneb" ("Minkat" a-bn)
       ("Alarac" a-au) "Enif" ("Alphekkar" a-ay) ("Hom" a-i)
       ("Alaras" a-cr) ("Aludraz" a-dg) "Denebola" ("Manubra" a-cd)
       "Al Tarf" 8 ("Ras" a-cr) "Tegmin" ("Lab " a-l) ("Arrak" a-db)
       "Dor" ("Aldaff" a-dc) "Apex"
       ("Al Minliar al Asad Minor" a-af) ("Dschubba" a-bq)
       ("Azelfafi" a-bf) ("Kitak" a-db) "Akrai" ("Aludraf" a-f)
       "Gac" "Mekbuda" 2 "Algol" ("Braf" a-f) ("Acami" a-y)
       ("Andra" a-cd) ("Al Jabi" a-de) ("Marfa" a-g) ("Ziba" a-ck)
       ("Alphekkalinan" a-ci) 2 ("Asteri" a-dd) ("Alcors" a-cs)
       ("Kurdar" a-ay) 3 ("Gor" a-af) 3 ("Jabh" a-r) "Lesath"
       ("Azelfafa" a-g) ("Muscida" a-cg) ("Ran" a-ci)
       ("Rigel Kenta" a-by) "Maia" ("Rota" a-by) ("Almeisan" a-ci)
       ("Matan" a-ci) ("Minkad" a-ah) ("Bran" a-ci)
       ("Bellatru" a-ae) ("Mirphat" a-d) ("Gienah Cyge" a-cv)
       ("Thubani" a-az) ("Matau" a-cx) ("Benetnar" a-ay)
       ("Asellus Borg" a-ag) "Azha" ("Benetnac" a-au)
       ("Lesaten Kai" a-ap) 2 "Tegmen" ("Toliman" a-ci)
       ("Taram" a-aq) ("Ankar" a-ay) ("Lesato" a-ch)
       2 "Suhail al Muhlif" 2 ("Gians" a-bo) "Salm" "Giedi Primus"
       ("Minkan" a-ci) 2 "Arkah" "Tarai" ("Achernas" a-cr)
       ("Naosm" a-ax) ("Syrmal" a-bx) "Kaus Bor" "Zauri"
       ("Subra" a-cd) "Krah" 5 "Alphirk" ("Alrischab" a-bl)
       ("Arras" a-cr) ("Naost" a-bb) ("Koc" a-bz) "Yildun" "Atlas"
       ("Al Jaba" a-bq) ("Nair " a-bv) ("Arraf" a-f) 5 "Menkib"
       ("Waz" a-dg) 2 "Ukda" ("Aludras" a-cr) "Oculus"
       ("Vindemiatr" a-b) 3 ("Jabr" a-bp) ("Alpherato" a-ch)
       ("Aludrak" a-db) "Krai" ("Dabr" a-bp) ("Nekkat" a-bn)
       ("Alcor " a-m) "Aludra" ("Peacoch" a-p) ("Azelfaff" a-dc)
       "Alterf" ("Aludrae" a-j) 2 ("Arkab" a-cq) "Minka"
       5 ("Asellus Aust" a-cu) ("Meissa" a-bu) "Rigel Kent"
       ("Unukal" a-cw) "Alpheratz" "Prijipatz" ("Saida" a-cg) "Keid"
       ("Mars" a-cs) ("Denebolaris" a-t) ("El Kop" a-dk)
       ("Alfecca Meridia" a-at) "Pola" ("Caleanop" a-dk)
       ("Regorg" a-ag) ("Tejat" a-bn) "Akralis" "Taralis"
       2 "Asellus Prior" ("Alpheraten Kai" a-ap) ("Had" a-ah)
       ("Labr" a-bp) ("Brae" a-j) ("Eda" a-u) 2 "Albaldah"
       ("Akraz" a-dg) ("Asellus Borr" a-ba) ("Arrap" a-aa) "Kurdah"
       ("Kaf" a-f) ("Cursa" a-bu) "Tegmine" ("Miram" a-aq) "Isis"
       ("Mebsuth" a-bd) 2 ("Hyad" a-ah) ("Etau" a-cx) "Bungula"
       ("Sulafar" a-ay) ("Benetnas" a-cr) "Kra" "Schedar"
       ("Aludrac" a-au) ("Achernab" a-cq) ("Al Tarfa" a-g)
       ("Izar" a-ay) 3 "Rigel" ("Al Minliar al Asada" a-u)
       "Muscidus" "Grumium" ("Mimosm" a-ax) ("Regorr" a-ba) "Haedus"
       9 ("Sad" a-ah) ("Dor " a-m) ("Regora" a-cd) "Sulafage"
       2 "Spica" "Baham" ("Al Dhana" a-h) ("Dorr" a-ba)
       2 ("Aldebar" a-ay) "Almeisa" ("Sulaphat" a-d) "Vega" "Kurhah"
       ("Brac" a-au) "Menkent" ("Giedi Prima" a-n) ("Sherata" a-by)
       ("Kor" a-af) ("Alphekkan" a-ci) ("Mizari" a-ct)
       ("Acuben " a-cp) ("Labb" a-av) "Heka" "Rijl al Awwa"
       2 ("Arkad" a-ah) ("Kraf" a-f) ("Krap" a-aa)
       ("Asellus Bora" a-cd) "Al Jab" ("Achernar" a-ay)
       ("Prijipats" a-dh) "Dheneb" 2 ("Alathfar" a-ay)
       ("Chertar" a-ay) ("Maas" a-cr) ("Alsciaukal" a-cw)
       2 ("Nodus " a-aw) 2 ("Antan" a-ci) ("Skaa" a-bh)
       ("Al Thaliman" a-ci) "Alpherat" "Kaus Borealis"
       ("Kaus Bors" a-cs) ("Alnitak" a-db) ("Braz" a-dg) "Dubhe"
       "Aspidiske" 2 ("Giedi Secund" a-ak) "Botein"
       ("Talitha Borr" a-ba) "Alnitalpha" "Murzim" "Alnair"
       ("Talitha Bora" a-cd) "Muphrid" ("Cujam" a-aq) ("Khama" a-br)
       2 ("Yed Po" a-ac) ("Prim" a-k) ("Mabsuth" a-bd) ("Err" a-ba)
       ("Adart" a-ai) 2 ("Arkan" a-ci) "Said" ("Alphekkad" a-ah)
       "Gacis" ("Merakr" a-bm) ("Fum Alsam" a-aq) ("Chara" a-cd)
       "Al Haud" ("Zost" a-bb) "Auva" ("Sinistra" a-cd) "Alkaid"
       "Mebsuta" ("Bras" a-cr) "Anka" ("Alderam" a-aq) "Alhena"
       ("Athaf" a-o) ("Arrab" a-cq) "Al Minliar al Shuja" "Al Anz"
       ("Electra" a-cd) "Polis" 2 "Altair" 2 ("Cas" a-cr)
       ("Arran" a-ci) "Rotein" "Alchibah" ("Meraka" a-aj) "Avior"
       ("Etam" a-aq) ("Aludrab" a-cq) "Deneb Algedi" "Arcturus"
       ("Alniyat" a-bn) ("Anchab" a-bl) "Alioth"
       ("Talitha Borg" a-ag) ("Pulcherrima" a-n) ("Akrab" a-cq)
       ("Merope" a-bi) ("Mirzar" a-ay) "Al Minliar al Asadir"
       "Vindem" 8 "Deneb" "Aludrai" 3 ("Stero" a-cj) ("Alnat" a-bn)
       ("Adhaf" a-o) 3 "Ain" ("Unukat" a-bn) ("Menkenta" a-by)
       ("Akrac" a-au) "Chard" "Prijipati" 2 ("Pherka" a-aj)
       "Tse Tseng" "Mirphak" ("Asellus Bors" a-cs) ("Alcora" a-cd)
       ("Alcorg" a-ag) ("Cestab" a-cq) ("Laba" a-bq) ("Labi" a-de)
       "Grum" ("Eltan" a-ci) "Brai" "Bralis" ("Alula Born" a-ce)
       "Regor" 2 "Hoedus" ("Mizart" a-ai) "Muhlifain" ("Skar" a-ay)
       "Bra" "Alnilam" "Andr" "Merophrah" 2 "Atik" "Scutulum"
       "Electrix" ("Ankad" a-ah) ("Regor " a-m) ("Sab" a-cq)
       2 "Ancha" ("Alcyona" a-h) ("Algor" a-af) 2 ("Hoedus " a-bc)
       "Talitha Austrix" 2 "Beteigeuse" "Nihal" ("Gemmar" a-ay)
       2 "Ceginus" ("Sheraten Kai" a-ap) 4 "Ruchbah"
       2 ("Ukdat" a-bn) 3 ("Alshar" a-w)
       ("Al Minliar al Asado" a-bj) ("Ankalinan" a-ci)
       ("Nekkad" a-ah) 5 ("Menka" a-aj) ("Aldafi" a-bf)
       2 ("Ukda " a-bt) "Sceps" "Sherath" "Mirfage" ("Tarak" a-db)
       "Al Minliar al Saif" ("Alphecca Meridia" a-at)
       ("Deneb Oka" a-aj) ("Mintam" a-aq) ("Nusaka" a-aj)
       "Asellus Tertius" ("Adara" a-cd) ("Benetna " a-ad)
       "Asellus Borealis" ("Chertab" a-cq) ("Schedart" a-ai)
       ("Ankat" a-bn) ("Chor" a-c) "Sulaphak" "Talitha Bor"
       ("Kaus Borr" a-ba) ("Mirac" a-au) 2 "Marfik" ("Alarab" a-cq)
       ("Cox" a-am) ("Alula Borg" a-ag) ("Achernac" a-au)
       "Deneb Algenubi" 2 "Algedi" ("Naosa" a-bu) ("Tarap" a-aa)
       2 "Betelgeuse" "Aludralis" "Alarah" ("Sulafak" a-db) "Thabit"
       "Alcor" "Terebellium" ("Kaus Austru" a-ae) ("Sinistru" a-ae)
       "Tureis" 2 ("Ukdar" a-ay) ("Alarae" a-j) "Rigel Kentis"
       "Mufrid" "Cih" "Dhur" ("Akran" a-ci) "Diphda" 2 ("Gra" a-cd)
       "Kaus Medis" 2 "Albali" "Marf" ("Phecdal" a-cz)
       ("Regors" a-cs) "Mizares" 2 ("Nas" a-cr) 2 ("Arkalinan" a-ci)
       ("Alphara" a-cd) "Skah" "Acamen" ("Skalinan" a-ci) "Prijipat"
       2 ("Tania" a-ao) 5 "Zubens" ("Arram" a-aq) ("Phecdas" a-cr)
       2 ("Arkat" a-bn) ("Mintan" a-ci) "Gienah Cygni"
       ("Mirak" a-db) ("Alsafa" a-g) "Markeb" ("Al Athfar" a-ay)
       ("Regorn" a-ce) "Tarah" 2 "Sirius" ("Cestam" a-aq) "Beid"
       2 ("Ukdas" a-cr) "Aldhiba" ("Mizara" a-cd) ("Aludram" a-aq)
       "Subrium" ("Rigil Kent" a-cl) ("Masya" a-x)
       "Al Minliar al Asad" ("Nekkalinan" a-ci) ("Ham" a-aq)
       "Kelb Alrai" "Algenib" ("Bellatra" a-cd) "Benetna"
       ("Dors" a-cs) ("Kurdat" a-bn) "Ed Asich" ("Taras" a-cr)
       ("Acherna " a-ad) "Nair" "Arra" ("Kaus Born" a-ce) "Kurda"
       ("Minkar" a-ay) "Mimos" "Mabsuta" ("Sherats" a-dh)
       "Deneb Dulphim" ("Alminhar" a-w) "Wezen" ("Alaran" a-ci)
       "Alphekka" "Ska" "Brah" ("Andro" a-e) ("El Nat" a-bn)
       2 "Ain al Rai" "Alcorealis" "Megrez" "Taygni"
       ("Alkaida" a-cg) "Chow" "Zos" ("Akrak" a-db) "Alphecca"
       ("Mesar" a-ay) "Albireo" ("Ankaa" a-bh) ("Kram" a-aq)
       "Phecdah" ("Can" a-ci) "Lesatz" "Sheratz" ("Han" a-ci) "Zava"
       "Tseen Ke" ("Phecda " a-bt) ("Zan" a-ci) "Pulcherrimus"
       "Gemma" "Prijipath" "Akrah" ("Brap" a-aa) ("Angete" a-an)
       "Sulafaljidhma" 3 ("Cor" a-af) ("Alzirra" a-cd) "Alkurhah"
       ("Akrae" a-j) ("Rijl al Awwan" a-ci) "Celbalrai" ("Dab " a-l)
       ("Etak" a-db) ("Alaram" a-aq) "Al Tarfik" 2 ("Giana" a-h)
       ("Alcorr" a-ba) ("Alsaff" a-dc) "Syrma" "Kralis"
       ("Adhar" a-w) ("Kran" a-ci) ("Was" a-cr) ("Mergas" a-cr)
       ("Al Minliar al Asadu" a-da) ("Pra" a-cd) ("Alcorn" a-ce)
       "Dorealis" "Menkentis" "Tsih" ("Rukbat" a-bn) "Furud"
       ("Mintab" a-cq) "Nusak" ("Altair " a-bv) ("Taraz" a-dg)
       "Alcyon" "Alphekkah" 2 ("Ain al Ram" a-aq) ("Mimosa" a-bu)
       ("Akraf" a-f) ("Gemmak" a-db) ("Secund" a-ak) ("Nekkan" a-ci)
       2 "Tabit" "Sinistrix" "Alula Bor" "Aludrah" 2 ("Astero" a-cj)
       ("Skat" a-bn) "Biham" ("Aludran" a-ci) ("Ankab" a-cq)
       ("Chertau" a-cx) "Ke Kwa" ("Cestan" a-ci) "Arralis"
       5 ("Asellus Secu" a-s) ("Bat" a-bn) ("Labh" a-r)
       ("Alsciaukat" a-bn) 2 "Algieba" ("Kurdas" a-cr) "Shaula"
       "Diadem" ("Kaus Borg" a-ag) 3 ("Alshat" a-d) ("Syrmar" a-ay)
       "Tsze" ("Thubana" a-h) ("Asellus Born" a-ce)
       ("Kaus Austra" a-cd) "Alfecca" "Asellus Tertia" "Meropus"
       ("Chertak" a-db) "Minchir" ("Zosm" a-ax) 2 ("Antau" a-cx)
       ("Nekkab" a-cq) 2 "Mira" ("Al Minliar al Asadr" a-as)
       "Aldhibah" ("Matab" a-cq) "Ril Alauva"))
   (a-b (or ("u" a-ae) 11 ("a" a-cd) 2 "ix"))
   (a-c (or ("t" a-ai) ("o" a-e)))
   (a-d (or 3 "" 2 (" al Akr" a-q)))
   (a-e (or "li" ("p" a-dk) ("c" a-bz) ("x" a-am) ("m" a-i)
       ("s" a-cc)))
   (a-f (or ("a" a-g) 2 ("f" a-dc) 3 ("i" a-bf)))
   (a-g (or "ljidhma" 2 ("r" a-ay) 2 ("k" a-db) "ge" ("t" a-bn)))
   (a-h (or 2 "" ("c" a-au) 3 (" " a-ad) ("r" a-ay) ("b" a-cq)
       ("s" a-cr)))
   (a-i (or "eisa" ("eisan" a-ci) 2 ("a" a-br) "eda"))
   (a-j (or "" "cipua"))
   (a-k (or "us" 4 ("a" a-n)))
   (a-l (or 2 ("Pr" a-ca) 2 ("Po" a-ac)))
   (a-m (or ("Car" a-ay) ("Hyd" a-cm) ("Hya" a-x) ("Serpent" a-cl)))
   (a-n (or 3 "" 2 (" " a-ar)))
   (a-o (or "i" ("eram" a-aq) "era"))
   (a-p (or ("ab" a-bl) "a"))
   (a-q (or 2 "ah" 2 ("af" a-f) 2 "a" 2 ("ac" a-au) 2 "ai" 2 "alis"
       2 ("ab" a-cq) 2 ("ak" a-db) 2 ("as" a-cr) 2 ("az" a-dg)
       2 ("am" a-aq) 2 ("ae" a-j) 2 ("ap" a-aa) 2 ("an" a-ci)))
   (a-r (or 3 "ah" 3 ("at" a-d)))
   (a-s (or 2 ("nda" a-u) 2 ("ndu" a-dm)))
   (a-t (or 2 "" (" Austru" a-ae) " Austrix" (" Austra" a-cd)))
   (a-u (or 3 "h" 4 "" 5 ("l" a-cz) (" " a-bt) ("r" a-ay) ("s" a-cr)
       ("t" a-bn)))
   (a-v (or 2 "" "ium"))
   (a-w (or "" "d" 3 ("a" a-cd)))
   (a-x (or "" 4 ("d" a-ah)))
   (a-y (or "" "ne" 2 "n"))
   (a-z (or 2 "" ("a" a-br) ("b" a-cy)))
   (a-aa (or 2 "h" "ella"))
   (a-ab (or "aljidhma" ("ak" a-db) "age" ("at" a-bn) ("ar" a-ay)))
   (a-ac (or 2 "s" 2 ("sa" a-bu) 2 ("st" a-bb) 2 ("sm" a-ax)))
   (a-ad (or ("Quatru" a-ae) 2 "Tertia" ("Quatra" a-cd) "Quatrix"
       2 "Tertius" ("Secu" a-s)))
   (a-ae (or "m" "mium"))
   (a-af (or 5 "" 2 ("n" a-ce) ("r" a-ba) 3 (" " a-m) 2 ("a" a-cd)
       ("s" a-cs) 3 ("g" a-ag) 6 "ealis"))
   (a-ag (or ("as" a-cr) 3 "oni" "a" 3 ("ona" a-h) 3 "one" 3 "on"))
   (a-ah (or "ir" 2 "" 4 ("u" a-da) ("r" a-as) 7 ("a" a-u)
       (" Minor" a-af) ("o" a-bj)))
   (a-ai (or "" "him"))
   (a-aj (or 3 "" ("n" a-ci) ("a" a-bh) 4 ("r" a-ay) ("linan" a-ci)
       2 ("d" a-ah) "h" 4 ("b" a-cq) ("t" a-bn)))
   (a-ak (or ("u" a-dm) 3 ("a" a-u)))
   (a-al (or "" ("p" a-dk)))
   (a-am (or "ima" "a" ("ima " a-ar)))
   (a-an (or "na" ("nar" a-ay)))
   (a-ao (or " Bor" (" Bora" a-cd) (" Bor " a-m) (" Born" a-ce)
       (" Aust" a-cu) (" Bors" a-cs) "h" " Borealis" (" Borr" a-ba)
       (" Borg" a-ag)))
   (a-ap (or 2 ("tosa" a-bu) 2 ("tosm" a-ax) 2 ("tost" a-bb) 2 "tos"
      ))
   (a-aq (or 6 "" 3 ("a" a-br) "en" 3 ("i" a-y)))
   (a-ar (or 2 ("Hyd" a-cm) "Cent" ("Centa" a-by) 2 ("Hya" a-x)
       "Centis"))
   (a-as (or "" ("a" a-cd) ("o" a-e)))
   (a-at (or ("ns" a-bo) ("nf" a-ab) ("na" a-h)))
   (a-au (or "is" "" 3 ("h" a-v) "rux"))
   (a-av (or "ah" "alia" "a" ("at" a-bn) ("an" a-ci)))
   (a-aw (or 2 "II" 2 "I"))
   (a-ax (or ("ak" a-db) ("am" a-aq) "a" ("al" a-bx) ("ar" a-ay)))
   (a-ay (or 18 "" 3 ("a" a-cd) ("g" a-ag) 2 "es" ("o" a-e)
       3 ("i" a-ct) ("t" a-ai)))
   (a-az (or ("a" a-ao) "n"))
   (a-ba (or ("a" a-cd) ("ima" a-n) "imus"))
   (a-bb (or ("a" a-by) ("r" a-b) 2 ("ero" a-cj) 2 ("eri" a-dd)
       ("o" a-ch)))
   (a-bc (or 2 "II" 2 "I"))
   (a-bd (or "at" ("at al Akr" a-q)))
   (a-be (or 2 "" 2 ("e" a-bk)))
   (a-bf (or "" 2 "as" "rah"))
   (a-bg (or "i" ("m" a-aq)))
   (a-bh (or "" ("k" a-db)))
   (a-bi (or 2 " II" 2 "" 2 " I"))
   (a-bj (or ("rs" a-cs) ("rn" a-ce) ("rr" a-ba) ("rg" a-ag)
       "realis" "r" ("ra" a-cd) ("r " a-m)))
   (a-bk (or ("d Bor " a-m) ("d Borr" a-ba) "d Borealis"
       ("d Bora" a-cd) ("d Aust" a-cu) ("d Bors" a-cs)
       ("d Born" a-ce) "d Bor" ("d Borg" a-ag)))
   (a-bl (or "" ("dhilinan" a-ci)))
   (a-bm (or 2 ("a" a-cd) "ibi"))
   (a-bn (or ("en Kai" a-ap) "i" 7 "" ("a" a-by) ("s" a-dh) 4 "h"
       "z" ("o" a-ch)))
   (a-bo (or "a" ("at" a-bn) ("al" a-co) ("ar" a-ay)))
   (a-bp (or "umium" "um"))
   (a-bq (or "h" "" ("n" a-ci) "lia" ("t" a-bn)))
   (a-br (or 3 "" 3 ("l" a-bx) ("k" a-db) ("r" a-ay) ("m" a-aq)))
   (a-bs (or "" ("cept" a-dj) "ceps"))
   (a-bt (or ("Hyd" a-cm) ("Hya" a-x)))
   (a-bu (or 3 ("l" a-co) 3 "" ("r" a-ay) ("t" a-bn)))
   (a-bv (or "al Saif" "al Shuja"))
   (a-bw (or "kis" "k" ("ka" a-aj) ("kr" a-bm)))
   (a-bx (or "haut" "hague" "i" "" "hai"))
   (a-by (or 2 ("u" a-cx) 4 ("n" a-ci) ("k" a-db) 3 ("r" a-ay)
       ("b" a-cq) ("m" a-aq)))
   (a-bz (or ("y" a-cn) "k" ("h" a-p)))
   (a-ca (or 3 "ior" 3 ("im" a-k)))
   (a-cb (or ("c" a-di) ("s" a-be)))
   (a-cc (or 5 "" ("a" a-bu) ("m" a-ax) 2 ("t" a-bb)))
   (a-cd (or "i" 2 ("e" a-j) 9 "" 2 ("n" a-ci) 7 "lis" ("m" a-aq)
       ("s" a-cr) "h" ("p" a-aa) ("c" a-au) 2 ("k" a-db)
       7 ("b" a-cq) 2 ("f" a-f) 2 ("z" a-dg)))
   (a-ce (or ("ephor" a-c) ("a" a-h)))
   (a-cf (or 2 "lik" 2 "lek"))
   (a-cg (or "" ("k" a-db)))
   (a-ch (or "ni" ("r" a-af) "ne" "n" 3 ("na" a-h)))
   (a-ci (or 15 "" "ev" ("a" a-h) 2 ("i" a-az) ("o" a-al)))
   (a-cj (or 3 "phrah" 3 "ps" 3 ("pe" a-bi) 3 "pus"))
   (a-ck (or "l" ("n" a-ci)))
   (a-cl (or "is" 2 "" 2 ("a" a-by)))
   (a-cm (or ("ro" a-e) "r" ("ra" a-cd)))
   (a-cn (or "oni" ("ona" a-h) "one" "on"))
   (a-co (or "haut" "hague" ("a" a-cb) "hai" "gethi"))
   (a-cp (or ("Haka" a-aj) ("Esham" a-z) "Hak" "Elgenubi"
       2 ("Elas" a-be) ("Hakr" a-bm) 2 ("Elak" a-dl) "Hakis"))
   (a-cq (or 9 "" 2 (" " a-l) 3 ("h" a-r) ("a" a-bq) ("r" a-bp)
       ("b" a-av) 3 ("i" a-de)))
   (a-cr (or "l" "" 2 "ym" "seleh" 4 ("a" a-bu) ("ya" a-x)
       2 (" Elak" a-dl) " Algethi" "if" 2 "h" " Mutallah" "ich"
       "hira" 2 (" Elas" a-be) 3 ("t" a-bb)))
   (a-cs (or "ik" "um"))
   (a-ct (or "" ("n" a-bs) ("s" a-t)))
   (a-cu (or 7 "rix" 7 ("ra" a-cd) 7 ("ru" a-ae)))
   (a-cv (or "ta" ("te" a-an)))
   (a-cw (or "haut" "hague" "hai"))
   (a-cx (or 2 "ri" 2 ("ra" a-bw) 2 "rus"))
   (a-cy (or "ah" "alia" "a" ("at" a-bn) ("an" a-ci)))
   (a-cz (or ("b al Ra" a-bg) "suud" ("ba" a-df) ("a" a-cb)
       2 ("me" a-cf) "m"))
   (a-da (or 2 "m II" 2 "m I" 4 "m"))
   (a-db (or 2 "is" 9 "" ("r" a-bm) 3 ("a" a-aj)))
   (a-dc (or "ias" ("a" a-g)))
   (a-dd (or 3 "on" 3 "or"))
   (a-de (or "h" "" "k"))
   (a-df (or ("rg" a-ag) ("ro" a-e) "r" ("ra" a-cd) "res"
       ("rt" a-ai) ("ri" a-ct)))
   (a-dg (or 2 "" "ed" "n"))
   (a-dh (or "ym" ("ya" a-x)))
   (a-di (or "idus" ("ida" a-cg) "hbia"))
   (a-dj (or "rix" ("ra" a-cd) ("ru" a-ae)))
   (a-dk (or 4 ("e" a-bi) "s" 2 "us" "hrah"))
   (a-dl (or 2 "ribi" 2 ("ra" a-cd)))
   (a-dm (or "s" ("s " a-aw)))
))
