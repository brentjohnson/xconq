(game-module "coral-sea-th"
  (title "Coral Sea")
  (version "1.0")
  (blurb "Battle of the Coral Sea on 4-8 May 1942.")
  (instructions "The Japanese attempt to capture Port Moresby.
Type '?' for help.
(I especially recommend reading the notes in the \"modules\"
section and at the end of each unit description.)")
  (base-module "tailhook")
  (variants
    (world-seen true)
    (see-all false)
    (sequential false)
  )
)

(set initial-date "6:00 4 May 1942")

(side 1 (name "USA") (noun "American") (class "american")
  (emblem-name "flag-usa") (names-locked true))
(side 2 (name "Japan") (noun "Japanese") (class "japanese") 
  (emblem-name "flag-japan") (names-locked true))

(area 120 80 (restrict 800 320 630 100))
(include "earth-50km")
(include "t-e50-river")

(add u* start-with 0)
(add places point-value 0)

;; Could expand the map and add towns in Japan, China, Borneo, etc.

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; American forces ;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(unit-defaults (s 1))

;; Australian cities
(port 626  83 (n "Perth"))
(port 637 126 (n "Darwin"))
;;(town 676 117 (n "Cooktown"))  ;; 100 miles north of Cairns
(town 676 117 (n "Cairns"))
(port 680  76 (n "Adelaide"))
(port 697  70 (n "Melbourne"))
(town 705  75 (n "Canberra"))
(town 705  93 (n "Brisbane"))
(port 707  78 (n "Sydney"))
(port 709  59 (n "Hobart"))
(port 762  72 (n "Auckland"))
(port 765  57 (n "Christchurch"))

(port          672 133 (n "Port Moresby") (point-value 200))
(town          673 134 (n "Buna"))
(town          727 110 (n "Tontouta"))
(seaplane-base 728 110 (n "Noumea"))
(airstrip      661 131 (n "Thursday"))
(airfield      674 106 (n "Cloncurry"))
(airstrip      682 108 (n "Charters Towers"))
(airstrip      668 123 (n "Coen"))
(airfield      683 110 (n "Townsville"))
;(seaplane-base 686 129 (n "Deboyne")) ; temporary Japanese base

;; 19 B-25, 19 A-24, 14 A-20 at Charters Towers (3rd Light Bombardment
;;   Group)
(B-25 (in "Charters Towers"))
(B-25 (in "Charters Towers"))
(B-25 (in "Charters Towers"))
(B-25 (in "Charters Towers"))
(B-25 (in "Charters Towers"))
(B-25 (in "Charters Towers"))
(A-24 (in "Charters Towers"))
(A-24 (in "Charters Towers"))
(A-24 (in "Charters Towers"))
(A-24 (in "Charters Towers"))
(A-24 (in "Charters Towers"))
(A-24 (in "Charters Towers"))
(A-20 (in "Charters Towers"))
(A-20 (in "Charters Towers"))
(A-20 (in "Charters Towers"))
(A-20 (in "Charters Towers"))
(A-20 (in "Charters Towers"))

;; 12 B-25, 80 B-26, 50 P-39 at Townsville (22nd Medium Bombardment
;;   Group, 8th Fighter Group)
(B-25 (in "Townsville"))
(B-25 (in "Townsville"))
(B-25 (in "Townsville"))
(B-25 (in "Townsville"))
(B-26 (in "Townsville"))
(B-26 (in "Townsville"))
(B-26 (in "Townsville"))
(B-26 (in "Townsville"))
(B-26 (in "Townsville"))
(B-26 (in "Townsville"))
(B-26 (in "Townsville"))
(B-26 (in "Townsville"))
(B-26 (in "Townsville"))
(B-26 (in "Townsville"))
(B-26 (in "Townsville"))
(B-26 (in "Townsville"))
(B-26 (in "Townsville"))
(B-26 (in "Townsville"))
(B-26 (in "Townsville"))
(B-26 (in "Townsville"))
(B-26 (in "Townsville"))
(B-26 (in "Townsville"))
(B-26 (in "Townsville"))
(B-26 (in "Townsville"))
(B-26 (in "Townsville"))
(B-26 (in "Townsville"))
(B-26 (in "Townsville"))
(B-26 (in "Townsville"))
(B-26 (in "Townsville"))
(B-26 (in "Townsville"))
(B-26 (in "Townsville"))
(P-39 (in "Townsville"))
(P-39 (in "Townsville"))
(P-39 (in "Townsville"))
(P-39 (in "Townsville"))
(P-39 (in "Townsville"))
(P-39 (in "Townsville"))
(P-39 (in "Townsville"))
(P-39 (in "Townsville"))
(P-39 (in "Townsville"))
(P-39 (in "Townsville"))
(P-39 (in "Townsville"))
(P-39 (in "Townsville"))
(P-39 (in "Townsville"))
(P-39 (in "Townsville"))
(P-39 (in "Townsville"))
(P-39 (in "Townsville"))
(P-39 (in "Townsville"))

;; 50 P-39 at Port Moresby (8th Fighter Group)
(P-39 (in "Port Moresby"))
(P-39 (in "Port Moresby"))
(P-39 (in "Port Moresby"))
(P-39 (in "Port Moresby"))
(P-39 (in "Port Moresby"))
(P-39 (in "Port Moresby"))
(P-39 (in "Port Moresby"))
(P-39 (in "Port Moresby"))
(P-39 (in "Port Moresby"))
(P-39 (in "Port Moresby"))
(P-39 (in "Port Moresby"))
(P-39 (in "Port Moresby"))
(P-39 (in "Port Moresby"))
(P-39 (in "Port Moresby"))
(P-39 (in "Port Moresby"))
(P-39 (in "Port Moresby"))
(P-39 (in "Port Moresby"))

;; 48 B-17 at Cloncurry (19th Heavy Bombardment Group)
(B-17 (in "Cloncurry"))
(B-17 (in "Cloncurry"))
(B-17 (in "Cloncurry"))
(B-17 (in "Cloncurry"))
(B-17 (in "Cloncurry"))
(B-17 (in "Cloncurry"))
(B-17 (in "Cloncurry"))
(B-17 (in "Cloncurry"))
(B-17 (in "Cloncurry"))
(B-17 (in "Cloncurry"))
(B-17 (in "Cloncurry"))
(B-17 (in "Cloncurry"))
(B-17 (in "Cloncurry"))
(B-17 (in "Cloncurry"))
(B-17 (in "Cloncurry"))
(B-17 (in "Cloncurry"))

;; 90 P-40 at Darwin (49th Fighter Group)
(P-40E (in "Darwin"))
(P-40E (in "Darwin"))
(P-40E (in "Darwin"))
(P-40E (in "Darwin"))
(P-40E (in "Darwin"))
(P-40E (in "Darwin"))
(P-40E (in "Darwin"))
(P-40E (in "Darwin"))
(P-40E (in "Darwin"))
(P-40E (in "Darwin"))
(P-40E (in "Darwin"))
(P-40E (in "Darwin"))
(P-40E (in "Darwin"))
(P-40E (in "Darwin"))
(P-40E (in "Darwin"))
(P-40F (in "Darwin"))
(P-40F (in "Darwin"))
(P-40F (in "Darwin"))
(P-40F (in "Darwin"))
(P-40F (in "Darwin"))
(P-40F (in "Darwin"))
(P-40F (in "Darwin"))
(P-40F (in "Darwin"))
(P-40F (in "Darwin"))
(P-40F (in "Darwin"))
(P-40F (in "Darwin"))
(P-40F (in "Darwin"))
(P-40F (in "Darwin"))
(P-40F (in "Darwin"))
(P-40F (in "Darwin"))

;; 100 P-39 at Sydney (35th Fighter Group)
(P-39 (in "Sydney"))
(P-39 (in "Sydney"))
(P-39 (in "Sydney"))
(P-39 (in "Sydney"))
(P-39 (in "Sydney"))
(P-39 (in "Sydney"))
(P-39 (in "Sydney"))
(P-39 (in "Sydney"))
(P-39 (in "Sydney"))
(P-39 (in "Sydney"))
(P-39 (in "Sydney"))
(P-39 (in "Sydney"))
(P-39 (in "Sydney"))
(P-39 (in "Sydney"))
(P-39 (in "Sydney"))
(P-39 (in "Sydney"))
(P-39 (in "Sydney"))
(P-39 (in "Sydney"))
(P-39 (in "Sydney"))
(P-39 (in "Sydney"))
(P-39 (in "Sydney"))
(P-39 (in "Sydney"))
(P-39 (in "Sydney"))
(P-39 (in "Sydney"))
(P-39 (in "Sydney"))
(P-39 (in "Sydney"))
(P-39 (in "Sydney"))
(P-39 (in "Sydney"))
(P-39 (in "Sydney"))
(P-39 (in "Sydney"))
(P-39 (in "Sydney"))
(P-39 (in "Sydney"))
(P-39 (in "Sydney"))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Task force 17 ;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Adm. Fletcher

;; TG 17.2 attack group (Adm. Kinkaid)
(CA 698 135 (n "Minneapolis"))
(CA 698 135 (n "New Orleans"))
(CA 698 135 (n "Astoria"))
(CA 698 135 (n "Chester"))
(CA 698 135 (n "Portland"))
(DD 698 135 (n "Phelps"))
(DD 698 135 (n "Dewey"))
(DD 698 135 (n "Farragut"))
(DD 698 135 (n "Aylwin"))
(DD 698 135 (n "Monaghan"))

;; TG 17.3 support group (Adm. Crace)
(CA 697 122 (n "HMAS Australia"))
(CA 697 122 (n "HMAS Hobart"))
(DD 697 122 (n "Walke"))
(CA 698 135 (n "Chicago"))
(DD 698 135 (n "Perkins"))

;; TG 17.5 carrier group (Adm. Fitch)
;
;; Adm. Fitch
(CV 700 125 (n "Yorktown"))
(DD 700 125 (n "Hammann"))
(DD 700 125 (n "Russell"))
;
;; Adm. Fletcher
(CV 698 135 (n "Lexington"))
(DD 698 135 (n "Morris"))
(DD 698 135 (n "Anderson"))

;; TG 17.6 fueling group (Cpt. Phillips)
(oiler 700 125 (n "Neosho"))
(oiler 700 125 (n "Tippecanoe"))
(DD    700 125 (n "Sims"))
(DD    700 125 (n "Worden"))

;; VF-42 (20+1 F4F-3), VB-5 (19 SBD/3), VS-5 (19 SBD/3), 
;; VT-5 (13 TBD) on Yorktown
(F4F-3 (n "VF-42A") (in "Yorktown")) ;; 42-F-1, -2, and -3
(F4F-3 (n "VF-42B") (in "Yorktown"))
(F4F-3 (n "VF-42C") (in "Yorktown"))
(F4F-3 (n "VF-42D") (in "Yorktown"))
(F4F-3 (n "VF-42E") (in "Yorktown"))
(F4F-3 (n "VF-42F") (in "Yorktown"))
(F4F-3 (n "VF-42G") (in "Yorktown"))
(SBD (n "VB-5A") (in "Yorktown")) ;; 5-B-1, -2, and -3
(SBD (n "VB-5B") (in "Yorktown")) ;; 5-B-4, -5, and -6
(SBD (n "VB-5C") (in "Yorktown"))
(SBD (n "VB-5D") (in "Yorktown"))
(SBD (n "VB-5E") (in "Yorktown"))
(SBD (n "VB-5F") (in "Yorktown"))
(SBD (n "VS-5A") (in "Yorktown")) ;; 5-S-1, -2, and -3
(SBD (n "VS-5B") (in "Yorktown"))
(SBD (n "VS-5C") (in "Yorktown"))
(SBD (n "VS-5D") (in "Yorktown"))
(SBD (n "VS-5E") (in "Yorktown"))
(SBD (n "VS-5F") (in "Yorktown"))
(TBD (n "VT-5A") (in "Yorktown"))
(TBD (n "VT-5B") (in "Yorktown"))
(TBD (n "VT-5C") (in "Yorktown"))
(TBD (n "VT-5D") (in "Yorktown"))

;; VF-2 (22+1 F4F-3), VB-2 (18 SBD/3), VS-2 (18 SBD/3), 
;; VT-2 (12 TBD) on Lexington
(F4F-3 (n "VF-2A") (in "Lexington"))
(F4F-3 (n "VF-2B") (in "Lexington"))
(F4F-3 (n "VF-2C") (in "Lexington"))
(F4F-3 (n "VF-2D") (in "Lexington"))
(F4F-3 (n "VF-2E") (in "Lexington"))
(F4F-3 (n "VF-2F") (in "Lexington"))
(F4F-3 (n "VF-2G") (in "Lexington"))
(F4F-3 (n "VF-2H") (in "Lexington"))
(SBD (n "VB-2A") (in "Lexington"))
(SBD (n "VB-2B") (in "Lexington"))
(SBD (n "VB-2C") (in "Lexington"))
(SBD (n "VB-2D") (in "Lexington"))
(SBD (n "VB-2E") (in "Lexington"))
(SBD (n "VB-2F") (in "Lexington"))
(SBD (n "VS-2A") (in "Lexington"))
(SBD (n "VS-2B") (in "Lexington"))
(SBD (n "VS-2C") (in "Lexington"))
(SBD (n "VS-2D") (in "Lexington"))
(SBD (n "VS-2E") (in "Lexington"))
(SBD (n "VS-2F") (in "Lexington"))
(TBD (n "VT-2A") (in "Lexington"))
(TBD (n "VT-2B") (in "Lexington"))
(TBD (n "VT-2C") (in "Lexington"))
(TBD (n "VT-2D") (in "Lexington"))

;; TG 17.9 search group (Cdr. DeBaun)
(tender 728 109 (n "Tangier"))
(PBY-5 (in "Noumea"))
(PBY-5 (in "Noumea"))
(PBY-5 (in "Noumea"))
(PBY-5 (in "Noumea"))
(PBY-5 (in "Noumea"))
(PBY-5 (in "Noumea"))
(PBY-5 748 110 (m 30)) ; 30 fuel
(PBY-5 748 110 (m 30))
(PBY-5 748 110 (m 30))
(PBY-5 748 110 (m 30))
(PBY-5 748 110 (m 30))
(PBY-5 748 110 (m 30))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; TF 42 submarine group ;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Adm. Rockwell

;; 11 S-class submarines based at Brisbane.  Patrolling coastal waters
;; of Papua, and the Louisiade and Bismarck Archipelagos.  Only four
;; were operational at the time (one of which was S-42).

(tender 645 129 (n "Griffin"))
(sub 682 128 (n "S-38"))
(sub 686 132 (n "S-40"))
(sub 681 134 (n "S-42"))
(sub 668 147 (n "S-45"))


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Japanese forces ;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(unit-defaults (s 2))

;; Adm. Inouye

(port          663 174 (n "Truk"))      ;; way N of New Ireland
(airstrip      672 148 (n "Kavieng"))   ;; NW New Ireland
(port          676 144 (n "Rabaul"))    ;; NE New Britain
(airstrip      674 140 (n "Gasmata"))   ;; S New Britain
(airstrip      686 144 (n "Kieta"))     ;; SE Bougainville
(seaplane-base 687 143 (n "Shortland")) ;; SE Bougainville
(airstrip      666 140 (n "Lae"))       ;; E New Guinea
(seaplane-base 667 140 (n "Salamaua"))  ;; E New Guinea
(seaplane-base 696 141 (n "Tulagi"))    ;; N of Guadalcanal

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Land-based air force ;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Adm. Yamada

;; 3 "seaplanes" at Shortland
(H6K4 (in "Shortland"))
(H6K4 (in "Shortland"))
(H6K4 (in "Shortland"))

;; 6 "seaplanes" at Tulagi
(H6K4 (in "Tulagi"))
(H6K4 (in "Tulagi"))
(H6K4 (in "Tulagi"))
(H6K4 (in "Tulagi"))
(H6K4 (in "Tulagi"))
(H6K4 (in "Tulagi"))

;; 12 VF, 41 VB, 3 "seaplanes" at Rabaul
(A6M2 (in "Rabaul"))
(A6M2 (in "Rabaul"))
(A6M2 (in "Rabaul"))
(A6M2 (in "Rabaul"))
(D3A1 (in "Rabaul"))
(D3A1 (in "Rabaul"))
(D3A1 (in "Rabaul"))
(D3A1 (in "Rabaul"))
(D3A1 (in "Rabaul"))
(D3A1 (in "Rabaul"))
(D3A1 (in "Rabaul"))
(D3A1 (in "Rabaul"))
(D3A1 (in "Rabaul"))
(D3A1 (in "Rabaul"))
(D3A1 (in "Rabaul"))
(D3A1 (in "Rabaul"))
(D3A1 (in "Rabaul"))
(D3A1 (in "Rabaul"))
(H6K4 (in "Rabaul"))
(H6K4 (in "Rabaul"))
(H6K4 (in "Rabaul"))

;; 6 VF at Lae
(A6M2 (in "Lae"))
(A6M2 (in "Lae"))

;; Tainan and Genzan Air Groups: 45 VF, 45 VB at Truk
(A6M2 (in "Truk"))
(A6M2 (in "Truk"))
(A6M2 (in "Truk"))
(A6M2 (in "Truk"))
(A6M2 (in "Truk"))
(A6M2 (in "Truk"))
(A6M2 (in "Truk"))
(A6M2 (in "Truk"))
(A6M2 (in "Truk"))
(A6M2 (in "Truk"))
(A6M2 (in "Truk"))
(A6M2 (in "Truk"))
(A6M2 (in "Truk"))
(A6M2 (in "Truk"))
(A6M2 (in "Truk"))
(D3A1 (in "Truk"))
(D3A1 (in "Truk"))
(D3A1 (in "Truk"))
(D3A1 (in "Truk"))
(D3A1 (in "Truk"))
(D3A1 (in "Truk"))
(D3A1 (in "Truk"))
(D3A1 (in "Truk"))
(D3A1 (in "Truk"))
(D3A1 (in "Truk"))
(D3A1 (in "Truk"))
(D3A1 (in "Truk"))
(D3A1 (in "Truk"))
(D3A1 (in "Truk"))
(D3A1 (in "Truk"))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Carrier striking force ;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Adm. Takagi

(CA    673 155 (n "Myoko"))
(CA    673 155 (n "Haguro"))
(CV    673 155 (n "Zuikaku"))
(CV    673 155 (n "Shokaku"))
(DD    673 155 (n "Ariake"))
(DD    673 155 (n "Yugure"))
(DD    673 155 (n "Shiratsuyu"))
(DD    673 155 (n "Shigure"))
(DD    673 155 (n "Ushio"))
(DD    673 155 (n "Akebono"))
(oiler 673 155 (n "Toho Maru"))
(F1M (in "Myoko"))
(F1M (in "Myoko"))
(F1M (in "Haguro"))
(F1M (in "Haguro"))

;; 21 VF, 21 VB, 21 VT in Zuikaku
(A6M2 (in "Zuikaku"))
(A6M2 (in "Zuikaku"))
(A6M2 (in "Zuikaku"))
(A6M2 (in "Zuikaku"))
(A6M2 (in "Zuikaku"))
(A6M2 (in "Zuikaku"))
(A6M2 (in "Zuikaku"))
(B5N  (in "Zuikaku"))
(B5N  (in "Zuikaku"))
(B5N  (in "Zuikaku"))
(B5N  (in "Zuikaku"))
(B5N  (in "Zuikaku"))
(B5N  (in "Zuikaku"))
(B5N  (in "Zuikaku"))
(D3A1 (in "Zuikaku"))
(D3A1 (in "Zuikaku"))
(D3A1 (in "Zuikaku"))
(D3A1 (in "Zuikaku"))
(D3A1 (in "Zuikaku"))
(D3A1 (in "Zuikaku"))
(D3A1 (in "Zuikaku"))

;; 21 VF, 20 VB, 21 VT in Shokaku
(A6M2 (in "Shokaku"))
(A6M2 (in "Shokaku"))
(A6M2 (in "Shokaku"))
(A6M2 (in "Shokaku"))
(A6M2 (in "Shokaku"))
(A6M2 (in "Shokaku"))
(A6M2 (in "Shokaku"))
(B5N  (in "Shokaku"))
(B5N  (in "Shokaku"))
(B5N  (in "Shokaku"))
(B5N  (in "Shokaku"))
(B5N  (in "Shokaku"))
(B5N  (in "Shokaku"))
(B5N  (in "Shokaku"))
(D3A1 (in "Shokaku"))
(D3A1 (in "Shokaku"))
(D3A1 (in "Shokaku"))
(D3A1 (in "Shokaku"))
(D3A1 (in "Shokaku"))
(D3A1 (in "Shokaku"))
(D3A1 (in "Shokaku"))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Invasion force ;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; Tulagi invasion group (Adm. Shima)
(transport 696 140 (n "Azumasan Maru"))
(DD        696 140 (n "Kikuzuki"))
(DD        696 140 (n "Yuzuki"))

;; Port Moresby invasion group (Adm. Kajioka)
(transport (in "Rabaul"))
(transport (in "Rabaul"))
(transport (in "Rabaul"))
(transport (in "Rabaul"))
(transport (in "Rabaul"))
(transport (in "Rabaul"))
(transport (in "Rabaul"))
(transport (in "Rabaul"))
(transport (in "Rabaul"))
(transport (in "Rabaul"))
(transport (in "Rabaul"))
(DD (n "Oite") (in "Rabaul"))
(DD (n "Asanagi") (in "Rabaul"))
(DD (n "Uzuki") (in "Rabaul"))
(DD (n "Mutsuki") (in "Rabaul"))
(DD (n "Mochizuki") (in "Rabaul"))
(DD (n "Yayoi") (in "Rabaul"))
(CL (n "Yubari") (in "Rabaul"))
(oiler (n "Goyo Maru") (in "Rabaul"))
(oiler (n "Hoyo Maru") (in "Rabaul"))

;; Support group (Adm. Marushige)
(CL     688 140 (n "Tenryu"))
(CL     688 140 (n "Tatsuta"))
(tender 688 140 (n "Kamikawa Maru"))

;; Covering group (Adm. Goto)
(CA  680 153 (n "Aoba"))
(CA  680 153 (n "Kako"))
(CA  680 153 (n "Kinugasa"))
(CA  680 153 (n "Furutaka"))
(CVE 680 153 (n "Shoho"))
(DD  680 153 (n "Sazanami"))
(F1M (in "Aoba"))
(F1M (in "Aoba"))
(E7K (in "Kako"))
(E7K (in "Kako"))
(E13A (in "Kinugasa"))
(E13A (in "Kinugasa"))
(E7K (in "Furutaka"))
(E7K (in "Furutaka"))

;; 12 VF, 9 VT in Shoho
(A6M2 (in "Shoho"))
(A6M2 (in "Shoho"))
(A6M2 (in "Shoho"))
(A6M2 (in "Shoho"))
(B5N (in "Shoho"))
(B5N (in "Shoho"))
(B5N (in "Shoho"))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Submarines
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Cpt. Ishizaki

;; Based at Rabaul

;; Recce Australia and South Sea islands
(sub 720 120 (n "I-21"))

;; Deploy 250 miles SW of Guadalcanal
(sub 690 128 (n "I-22"))
(sub 692 129 (n "I-24"))
(sub 694 130 (n "I-28"))
(sub 696 131 (n "I-29"))

;; Blockade Port Moresby and guide transports
(sub 672 130 (n "RO-33"))
(sub 675 129 (n "RO-34"))
