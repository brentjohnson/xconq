;;  -*- mode: lisp; tab-width: 1 -*-
;;  Generates place names in Swedish.
;;  Created and maintained by Erik Sigra (sigra@home.se).

(namer
	swedish-place-names
	(grammar
		root
		100
		(root
			(reject
				name
				Åå åå åkraåkra älvälv
				bergberg borgborg by
				daldal
				fältfält forsfors
				hemhem
				hemhus
				hemlösa ;  Means "homeless" (plural)
				hemstad ;  Means "homecity"
				höghög
				höghus  ;  Means "highouse"
				hushem
				hushög  ;  Means "pile of houses"
				hultshult hushus
				karlekarle kullakulla
				laxlax leklek
				markmark markamarka
				sandsand sjösjö slättslätt smjöl stensten
				strömström strömström sundsund
				vivi vikvik
				rrr ;Norrryd
				ttt ;Slätttuna
				)
			)    
		(name (or 16 mainpart 1 (preword " " (capitalize mainpart))))
		(mainpart (or 8 (prefix suffix) (prefix middle suffix) 2 (complete)))
		(prefix
			(or

				;;  Color
				svart blå grön gul röd brun vit
     
				;;  Age
				ny gammel

				;;  Preposition
	
				;;  Capitalized because automatic capitalization
				;;  doesn't work for this letter.
				Över

				neder
		
				;;  Size
				stor lill
		
				;;  Direction
				nord nordan norr
				Öst Östan Öster
				söder syd sydan väst
				västan väster vest ;;  Vestfjärd

				;;  Personal firstname
				adolfs bengts everts fredriks karls oskars tyres
		
				;;  Other
				Å åkers alings Älv ar Ås
				bå back berg björ björne bjurs bjus bor borg brahe
				bräm brunns
				dal djurs
				eke em en
				fågel fält fin finn fogd fors frö furu
				gör göte glimminge gran	gränges	gryt
				halls halm han helsing hem hög horn hults hus
				jön
				karle ki klags kol kors krono kulla kvarn kyrk
				lapp lax lek lem lin ling lule lums lycke
				malm mär marie mark marka mjöl morgon möln mört munk
				myckel
				näck nåden
				ockel öd om on Öre ox oxel
				på pite
				räfs råg rätt ronne ro rönn run
				såg salt sand sibb side sig simris sjö skattung slätt
				sol sollefte sollen ställ sten ström stock sund sve
				svine
				täll tammer tand töv tran troll
				udde ule ulvs ume unn upp
				våm var vår vet vikar vin ving vir
				ytter

				)
			)
		(suffix
			(or
				å åker åkra älv ås
				bäck bäcka backe baden berg berga björka borg bo böle
				bro bruk by byn
				dal dala dalen
				fält fjärd fors fred
				gård gården gärde gärdet gårda gåva ;  Morgongåva
				haga hamn haninge hättan hed heden hem hög hult hus
				hyttan
				kalix köping kulla kulle kvarn kvarna kyrka
				landa lax lösa lycke
				mark marka mo myra
				näs näset
				ö
				ryd
				sala sand sätra sjö skär slätt slott sta stad sten
				stig ström sund
				tjärn torp träsk tuna vattnet
				valla vi vik
				)
			)
		(middle
			(or ;  Fyll på!
				å åkra
				by
				fors
				karle
				lek lycke
				mjöl
				näs
				sjö ström
				)
			)
		(complete
			(or
				alfta Älvho Ånge artsjö
		  bäcka bjus bjuv bomarsund borlänge
		  dals-ed dösjebro
		  falun floda föglö
		  gagnef glimminge gustavs
		  habo hamra haninge hede höör huddinge
		  idre
		  jakobstad järna
		  kalix kalmar kävlinge kökar kvidinge
		  lima los lund
		  malung mora mörkret motala
		  nacka nora
		  ore orsa
		  sälen särna skanör skövde solv svalöv sveg
		  tierp tuna
		  vasa växjö vendel
				)
			)
		(preword
			(or

				;;  Color
				svarta blåa gröna gula röda bruna vita
		 
				;;  Age
				16 nya 16 gamla
		 
				;;  Preposition
				16 Övre 16 nedre

				;;  Size
				16 stora 16 lilla
		 
				;;  Direction
				16 norra 16 Östra 16 södra 16 västra
				)
			)
  )
	)
