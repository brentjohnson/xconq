(game-module "red-october"
  (title "October Revolution")
  (blurb "Russian Revolution of October 1917.")
  (version "1.0")
  (base-module "russian-rev")
  (variants
    (world-seen true)
    (see-all false)
    (sequential false)
  )
)

; (set terrain-seen true)

(include "u-rus-1910")

(set synthesis-methods '(make-initial-materials))

(set calendar '(usual day 2)) ; ?

(set initial-date "8 Nov 1917") ; but October, old style

(set sides-min 4)
(set sides-max 4)

(side 1 (plural-noun "Government") (adjective "Governmental") ; Kerenskij
  (color "blue") (emblem-name "flag-russia")) ; which emblem ?
(side 2 (noun "Bolshevik")(adjective "Bolshevik")
  (color "red") (emblem-name "hammer-and-sickle"))
(side 3 (noun "German") (adjective "German") 
  (color "gray,black,white") (emblem-name "german-cross"))
(side 4 (noun "Ukrainian") (adjective "Ukrainian")
  (color "yellow,blue") (emblem-name "flag-ukraine"))

(unit "Riga" (s 1))
(unit "Minsk" (s 1))
(unit "Odessa" (s 1))
(unit "Tallin" (s 1))
(unit "Sevastopol'" (s 1))
(fleet 25 11 1 (n "Black Sea fleet"))
(infantry 24 13 1)
(infantry 13 28 1)
(infantry 10 35 1) 
(infantry  8 33 1)

(unit "Petrograd" (s 2))
(unit "Helsingfors" (s 2))
(militia 10 40 2 (n "1st Red Guard"))
(militia 10 40 2 (n "2nd Red Guard"))
(militia 10 40 2 (n "3rd Red Guard"))
(fleet 9 40 2 (n "Baltic fleet") (sym the-fleet))
(militia 9 40 2 (n "Baltic fleet sailors") (in the-fleet))

(unit "Warszawa" (s 3))
(unit "Danzig" (s 3))
(unit "Konigsberg" (s 3))
(unit "Warszawa" (s 3))
(unit "Krakow" (s 3))
(unit "Lodz" (s 3))
(unit "Lvov" (s 3))
(unit "Katowice" (s 3))
(infantry 21 13 3)
(infantry 16 18 3)
(infantry 16 21 3)
(infantry 13 24 3)
(infantry 10 29 3)
(infantry  9 31 3)
(infantry  7 32 3)
(train 8 24 3)
(train 6 29 3)
(fleet 5 29 3)

(unit "Kiev" (s 4))
(unit "Rostov" (s 4))
(militia 19 22 4)
(militia 19 22 4)
(cavalry 35 14 4 (n "1st Cossack"))
(cavalry 36 14 4 (n "2nd Cossack"))

(train 22 32 0)
(train 128 7 0)

; 12 Apr 2001: Massimo Campostrini writes:
;   I am not able to work on xconq any longer (not even to play xconq),
;   but I still lurk on the mailing list.
;   . . .
;   If anyone feels as taking over any other xconq game I left orphan,
;   please do so.

(game-module (design-notes (
  "This scenario was designed by"
  "Massimo Campostrini <Massimo.Campostrini@df.unipi.it>."
  )))

(game-module (notes (
  "The Governatives should try to crush the Revolution"
  "and contain the Germans at the same time."
  )))
