;;; Japanese place name generator.

;;;  ng-japanese  v. 0.3  10/27/92
;;;  M. Campostrini
;;;
;;; Japanese place name generator.
;;;
;;; From 160 town names
;;; Some spelling conversions: pp->h, kk->k, tt->t.
;;; Tried to single out common endings with a definite meaning
;;;    (e.g. shima = island, hama = beach).  Needs confirmation!
;;; Short/long vowel distinction missing.
;;; A few losers still find their way in.
;;;

(namer japanese-place-names (grammar root 10
  (root (or 41 (starter ender)
	    77 (starter middle ender)
	    34 (starter middle middle ender)
	     2 (starter middle middle middle ender)
	     )
	)
  (starter (or 12 a ban be 2 chi cho chu da 7 fu 8 ga ge gi
	       5 ha 5 hi 6 i jo 7 ka 2 ki 7 ko 7 ku kyo
	       3 ma 5 mi mo mu myo 5 na 3 ni no nu 11 o ri 
	       4 sa sen 7 shi sho su 6 ta 9 to 2 u 4 wa 4 ya 6 yo zu)
	   )
  (middle (or 3 ba be bi 2 bu 3 chi da do e 3 go gu 2 ha 3 hi ho 7 i 2 ji
	      10 ka ke 2 ki 6 ko 8 ku 9 ma me 3 mi 6 mo 3 na ne 5 no nu
	      2 o 7 ra re 3 ri 3 ro 3 ru 4 sa 2 se 3 su 5 shi ta te 3 tsu 
	      4 wa 2 ya 2 yo 2 zu)
	  )
  (ender (or ba be bo bu 5 chi chu da 2 dai 2 do 2 fu 2 ga gi go 
	     2 hama he hu 2 ji 2 jima 2 ka kan 3 kawa keo 2 ki 3 ko kui kyo 
	     2 ma 2 mai 4 matsu me 3 mi nai nan nawa 2 ne 3 no 3 oka 
	     4 ra ran re 5 ri 4 ro saki sawa 2 seki sen 4 shi 10 shima shin
	     2 so 7 ta te 6 to 3 tsu tsue wa wara 2 ya 11 yama zawa 6 zu)
	 )
  )
)

#|
;;; Weights on consonants come from simple-minded (I looked in the atlas)
;;; histogramming of names.

(namer japanese-place-names (grammar root 9
  (root (or 1 (maybe-vowel last-syllable)
	    20 (maybe-vowel syllable last-syllable)
	    50 (maybe-vowel syllable syllable last-syllable)
	    50 (syllable syllable syllable last-syllable)))
  (maybe-vowel (or 90 "" 10 vowel))
  (syllable (consonant vowel))
  (last-syllable (or 95 (consonant vowel)
		5 (consonant vowel "n")))
  (vowel (or 5 a 2 e 3 i 3 o 1 u))
  (consonant (or 3 b 3 ch 1 d 3 f 11 g 6 h 2 j 25 k 16 m
	     10 n 7 r 14 s 2 sh 17 t 2 ts 6 w 5 y 3 z))
))

;;; Actually, should use the actual syllables - although this list
;;; is weighted to favor real ones, some losers find their way in.
|#
