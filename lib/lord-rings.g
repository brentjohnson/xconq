(game-module "lord-rings"
  (title "Lord of the Rings")
  (blurb "Harlan Thompson's Lord of the Rings scenario for Civ2 ported to Xconq.")
  (instructions "Ash nazg durbatuluk, ash nazg gimbatul, ash nazg thrakatuluk agh burzum-ishi krimpatul.")
  (base-module "3rd-age")
  (version "1.0")
  (variants 
    (see-all true)
    (world-seen true)
    (sequential true)
  )
)

(include "t-midearth")

(set synthesis-methods nil)
(set indepside-has-ai true)			;	Let the monsters move.
(set self-required true)			;	Require self-units.

(side 1 (name "Gondor") (plural-noun "Gondorians") (adjective "Gondor's") (class "gondor"))        
(side 2 (name "Mordor") (plural-noun "Sauron's slaves") (adjective "Mordor's") (class "mordor"))
(side 3 (name "Isengard") (plural-noun "Saruman's slaves") (adjective "Isengard's") (class "isengard"))
(side 4 (name "Rohan") (plural-noun "Rohirrim") (adjective "Rohan's") (class "rohan"))    
(side 5 (name "Elves") (plural-noun "Elves") (adjective "Elven") (class "elves"))
(side 6 (name "Harad") (plural-noun "Haradrim") (adjective "Harad's") (class "harad"))    
(side 7 (name "Rhun") (plural-noun "Easterlings") (adjective "Rhun's") (class "rhun"))    

;;	Add free folks without their own sides to Gondor

(add hobbits possible-sides "gondor")
(add dunedain possible-sides "gondor")
(add eagle possible-sides "gondor")
(add dwarves possible-sides "gondor")
(add rangers possible-sides "gondor")
(add ents possible-sides "gondor")

;;	Don't allow construction of colonizers

(add colonizers cp 0)

(side 1 (trusts (4 1)(5 1)))
(side 2 (trusts (6 1)(7 1)))
(side 4 (trusts (1 1)(5 1)))
(side 5 (trusts (1 1)(4 1)))
(side 6 (trusts (2 1)(7 1)))
(side 7 (trusts (2 1)(6 1)))

(define M mines)
(define V village)
(define G gondor-city)
(define @ town)
(define r ruins)
(define m mordor-city)
(define T tree-city)
(define E elven-city)

;; Gondor

(G (n "Minas Tirith")	   (@ 67 24))
(G (n "Dol Amroth")	   (@ 57 20))
(G (n "Pelargir")	   (@ 68 19))
(G (n "Calembel")	   (@ 58 23))
(G (n "Lamedon")	   (@ 44 19))
(G (n "Langstrand")	   (@ 52 21))
(G (n "Linhir")		   (@ 63 19))
(G (n "Morthond")	   (@ 54 25))

;; Rohan

(G (n "Edoras")		   (@ 54 30))
(V (n "Dunharrow")	   (@ 55 27))
(V (n "Helm's Deep")	   (@ 51 31))

;; Mordor

(m (n "Barad-Dur")      	(@ 74 28))
(m (n "Minas Morgul")   	(@ 70 24))
(m (n "Nurnen")	   	(@ 83 21))
(m (n "Carn Dum")	   	(@ 31 66))
(m (n "Dol Guldur")	   	(@ 55 43))
(m (n "Durthang")	   	(@ 69 28))
(M (n "Orodruin")	   	(@ 72 27))
(M (n "Ered Mithrin Mines")	(@ 49 61))
(M (n "Moria East")	   	(@ 46 44))
(M (n "Moria West")        	(@ 45 45))

;; Rhun

(@ (n "Rhun")		   (@ 76 46))

;; Harad

(@ (n "Harad")		   (@ 89  2))
(@ (n "Umbar")		   (@ 69  2))
(@ (n "Imloth Melui")   (@ 67 12))
(@ (n "Khand")		   (@ 91 14))

;; Isengard

(m (n "Isengard")	   (@ 49 34))
(@ (n "Arnach")		   (@ 40 29))
(@ (n "Grimslade")	   (@ 41 37))

;; Shire

(V (n "Michel Delving")(@ 25 51))
(V (n "Bree")		   (@ 33 52))
(V (n "Brandy Hall")	   (@ 30 52))
(V (n "Hobbiton")	   (@ 27 52))
(V (n "Staddle")	   	   (@ 33 54))
(V (n "Tuckborough")	   (@ 27 50))
(V (n "Undertowers")	   (@ 24 53))

;; Dale

(@ (n "Esgaroth")	   	(@ 55 57))
(@ (n "Dale")		   	(@ 54 59))
(V (n "Rhosgobel")	   	(@ 57 49))
(V (n "Woodmen's Village") (@ 51 52))
(M (n "Erebor")		   	(@ 53 59))
(M (n "Iron Hills mines")  (@ 65 58))
	
;; Elves

(T (n "Caras Galadhon")    (@ 50 42))
(E (n "Mithlond")	       (@ 20 53))
(E (n "Rivendell")	       (@ 43 53))
(T (n "Egladil")	   	       (@ 49 44))
(E (n "Elvenking's Halls") (@ 52 58))
(M (n "Ered Luin Mines")   (@ 21 50))

;; Ruins

(r (n "Fornost")	   	   (@ 30 57))
(r (n "Tharbad")	   (@ 37 45))
(r (n "Lond Daer")	   (@ 36 36))
(r (n "Osgiliath")	   (@ 68 24))
(r (n "Annuminas")	   (@ 28 55))

(unit-defaults (s 0))

(gollum 			  (@ 50 48))
(dragon  (n "Smaug")    (@ 51 66))

(unit-defaults (s 1))

(unit "Minas Tirith")
(unit "Dol Amroth")
(unit "Pelargir")
(unit "Calembel")
(unit "Lamedon")
(unit "Langstrand")
(unit "Linhir")
(unit "Morthond")

(unit "Michel Delving")
(unit "Bree")
(unit "Brandy Hall")
(unit "Hobbiton")
(unit "Staddle")
(unit "Tuckborough")
(unit "Undertowers")

(unit "Esgaroth")
(unit "Dale")
(unit "Rhosgobel")
(unit "Woodmen's Village")
(unit "Erebor")
(unit "Iron Hills mines")

(frodo (in "Hobbiton")(sym frodo1))
(ring (in frodo1))
(sam (in "Hobbiton"))
(pippin (in "Brandy Hall"))
(merry (in "Brandy Hall"))
(gandalf (in "Bree"))
(aragorn (in "Bree"))
(gimli (in "Bree"))
(faramir (in "Minas Tirith"))
(radagast (in "Rhosgobel"))
(boromir (in "Minas Tirith"))
(knights (in "Minas Tirith"))
(knights (in "Minas Tirith"))
(pikemen (in "Minas Tirith"))
(pikemen (in "Minas Tirith"))
(pikemen (in "Minas Tirith"))
(pikemen (in "Minas Tirith"))
(knights (in "Pelargir"))
(knights (in "Pelargir"))
(pikemen (in "Minas Tirith"))
(pikemen (in "Minas Tirith"))
(pikemen (in "Minas Tirith"))
(pikemen (in "Minas Tirith"))

(unit-defaults (s 2))

(unit "Barad-Dur")
(unit "Minas Morgul")
(unit "Nurnen")
(unit "Carn Dum")
(unit "Dol Guldur")
(unit "Durthang")
(unit "Orodruin")
(unit "Ered Mithrin Mines")
(unit "Moria East")
(unit "Moria West")

(sauron (in "Barad-Dur"))
(add sauron can-be-self true)
(balrog (in "Moria East"))
(witch-king (in "Minas Morgul"))
(nazgul (in "Minas Morgul"))
(nazgul (in "Minas Morgul"))
(nazgul (in "Minas Morgul"))
(nazgul (in "Minas Morgul"))
(nazgul (in "Minas Morgul"))
(nazgul (in "Minas Morgul"))
(nazgul (in "Minas Morgul"))
(nazgul (in "Minas Morgul"))
(shelob (@ 72 22))

(unit-defaults (s 3))

(unit "Isengard")
(unit "Arnach")
(unit "Grimslade")

(saruman (in "Isengard"))
(add saruman can-be-self true)
(wormtongue (in "Isengard"))

(unit-defaults (s 4))

(unit "Edoras")
(unit "Dunharrow")
(unit "Helm's Deep")

(theoden (in "Edoras"))
(eomer (in "Edoras"))
(eowyn (in "Edoras"))

(unit-defaults (s 5))

(unit "Caras Galadhon")
(unit "Mithlond")
(unit "Rivendell")
(unit "Egladil")
(unit "Elvenking's Halls")
(unit "Ered Luin Mines")

(legolas (in"Rivendell"))
(elrond (in"Rivendell"))
(galadriel (in"Caras Galadhon"))

(unit-defaults (s 6))

(unit "Harad")
(unit "Umbar")
(unit "Imloth Melui")
(unit "Khand")

(unit-defaults (s 7))

(unit "Rhun")
