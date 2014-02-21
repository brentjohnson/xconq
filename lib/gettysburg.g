(game-module "gettysburg"
  (title "Gettysburg")
  (version "1.0")
  (blurb "The Battle of Gettysburg, 1-3 July 1863.")
  (instructions (
   "Most of the brigades are off-board, and will come in a few at a time; "
   "the Confederates from the north and west, the Federals from the south."
   ""
   "You have to try to hold your positions, while at the same time "
   "charging the enemy."
   ""
   "Your artillery can be captured and used against you, so be sure to "
   "guard it."
   ))
  (variants
    (world-seen true)
    (see-all true)
    (sequential false)
    ("Day/Night" day-night true (true (world (day-length 24))))
    )
  )

(unit-type infantry (image-name "soldier-acw")
  (description-format (name "'s brigade (" side-name ")"))
  (help "a single brigade"))
(unit-type cavalry
  (description-format (name "'s cavalry (" side-name ")"))
  (help "a single brigade, fast but weak"))
(unit-type artillery (image-name "cannon")
  (description-format (name "'s artillery (" side-name ")"))
  (help "a demi-brigade of cannons, about 20 in all"))
(unit-type supply-train (image-name "wagon")
  (help "the `center' of the army"))

(define inf infantry)
(define cav cavalry)
(define arty artillery)

(terrain-type clear (image-name "plains") (char "+"))
;; (should become rough-woods and rough-light-woods)
(terrain-type rough (image-name "mountains") (char "^"))
(terrain-type light-woods (char "%"))
(terrain-type orchard)
(terrain-type woods (image-name "forest") (char "%"))
(terrain-type town (image-name "urban") (char "G"))
(terrain-type stream (image-name "river")
  (subtype border))
(terrain-type ravine (image-name "cliffs")
  (subtype border))
(terrain-type trail (image-name "tan")
  (subtype connection))
(terrain-type road (image-name "dirt-road")
  (subtype connection))

(table drawable-terrain
  (t* t* true)
  ((trail road) town false)
  )

;; This covers the gamut of elevations, from the bottom of Rock Creek to
;; the top of Big Round Top.  Values are in feet.

(add t* elevation-min 250)
(add t* elevation-max 700)

(add (light-woods orchard woods town) thickness (10 10 20 20))

(set grid-color "gray")

;;; Static relationships.

(add t* capacity 3)

(table unit-size-in-terrain
  (u* t* 1)
  (inf road 10)
  (supply-train t* 0)
  )

;(add u* possible-sides (not "independent"))

;;; Vision.

(set terrain-seen true)

(add u* vision-range 5)

(table vision-night-effect
  ;; Can only see adjacent units at unit.
  (u* t* 20)
  )

(add u* can-see-behind false)

(table visibility
  (u* t* 100)
  (u* (orchard light-woods town) 50)
  (u* woods 25)
  )

(table see-chance
  (u* u* 100)
  )

(table see-chance-adjacent
  ;; This is high so that multiplication with visibility yields
  ;; at least 100%, so we can always see adjacent units.
  (u* u* 400)
  )

(table eye-height
  ;; Infantry can stand on a fence, cavalry is on horse,
  ;; artillerists can stand on their cannons to see better.
  (u* t* 9)
  )

;;; Actions.

(add u* acp-per-turn (8 16 8 8))

;; Night shuts things down considerably.

(table night-multiplies-acp
  ;; 1/4 of normal abilities.
  (u* t* 25)
  ;; Infantry in open terrain can still move around somewhat.
  (infantry clear 50)
  ;; (should allow movement along roads at normal rate?)
  )
  
;;; Movement.

;; Most units pay a cost to get both in and out of a piece of terrain,
;; defaulting to 1 + 1 = 2.

(table mp-to-enter-terrain
  (u* t* 1)
  ;; Only infantry can get into rough terrain.
  (u* rough 99)
  (inf rough 2)
  ((cav arty) woods 4)
  ;; It costs artillery time to cross a stream, but not inf or cav.
  ((inf cav) stream 0)
  ;; Ravines are especially difficult, though not impassable.
  ((cav arty) ravine 2)
  ((cavalry arty) road 0)
  )

(table mp-to-leave-terrain
  (u* t* 1)
  ;; Only infantry can get out of rough terrain.
  (u* rough 99)
  (inf rough 2)
  ((cav arty) woods 4)
  ((cavalry arty) road 0)
  )

(table mp-to-traverse
  ;; Roads and trails help the movement of mounted and wheeled units.
  ((arty cavalry) road (2 1))
  (supply-train (trail road) (2 1))
  )

;; Everybody has to move carefully when in shooting range of the other
;; side.

(table zoc-range
  (inf inf 2)
  )

(table mp-to-enter-zoc
  (inf inf 4)
  )

(table mp-to-traverse-zoc
  (inf inf 4)
  )

;;; Repair.

(table can-repair
  (supply-train u* true)
  )

(table acp-to-repair
  (supply-train u* 4)
  )

(table hp-per-repair
  (supply-train u* 2.00)
  )

;;; Combat.

(add u* hp-max 8)

(table acp-to-attack
  (u* u* 4)
  ;; Artillery is numerically too weak to engage in assaults.
  (arty u* 0)
  (supply-train u* 0)
  )

(table acp-to-defend
  (u* u* 4))

(table hit-chance
  (u* u* 50)
  (inf u* (50 40 80 90))
  (cav u* (40 30 50 90))
  (supply-train u* 0)
  )

(table damage
  (u* u* 1d3)
  (supply-train u* 0)
  )

(table withdraw-chance-per-attack
  ;; Lots of running away in this era.
  (inf u* (30 50 10 10))
  (cav u* (40 20 10 10))
  )

(add arty range 5)  ;  (5 or 3 - need two types?)

(add arty acp-to-fire 8)

(table weapon-height
  (arty t* 4)
  )

(table capture-chance
  ;; Infantry can capture the artillery.
  (infantry arty 30)
  )

(table stack-protection
  ;; Artillery stacked with infantry can't be attacked or captured.
  ((infantry cavalry) arty 0)
  )

;; Everybody bounces back over time.

(add u* hp-recovery 0.25)
 
;;; The world.

(world 125000 (axial-tilt 22))

(area 50 36 (cell-width 1000))

(area (terrain
  "50a"
  "aea2e5ae5a2e3ae9ae18a"
  "ae8a2e5aea2e2ae7ae18a"
  "9aeae5ae2ae2ae7ae18a"
  "10ae39a"
  "4ae23ae2ae4ae13a"
  "13ad6ae8a3e4ae13a"
  "ae4a2e5aed5ae3ae25a"
  "ae12ae6ae3ad6a2e3ad12a"
  "ae2ae2ae2ae3ae12a2e4ae16a"
  "5aeae2aea2ead11a2e21a"
  "10aead2ae2a3f6a3e8ae3ae7a"
  "3ae3ae2a2e4ae2a2fe5a3e8a2e3ae6a"
  "aeae3ae3ae4ae10a2e6ae2a3e3ae5a"
  "aea2e2a2e2aead3ad6ae2a2ead4ae7a3e4a"
  "2ae6a2e2ad3ae6a2ea3e15ae4a"
  "2a2e4ad4ae3ae6a3ea3e19a"
  "14ae2ae3ac4aea3eae10ae6a"
  "15a3e5aea7ea2e6ae2a3ed2a"
  "13ad2aeae5ae2a2ea2e2ae3a4e8a"
  "12a2e3ae13ae10ae7a"
  "11ae2ae2ae2ad3aea2eae3ae16a"
  "7a2e2aea3e2ae10ae3ae5ae10a"
  "7a2e2ae7a2d3a2e3ae3ae4a3e7aea"
  "8a2e14a3ea3e10aead3a2ea"
  "9a2e3aeaea2ead2e5a3eae14aea"
  "10ae3aeae3ae2ae3c6ae8a3e3aea"
  "14ae6ae2ae2c7aeae5ac3e4a"
  "21ae2a4e6ae9a2c4a"
  "21ad6e22a"
  "18a3e2a5e9ae2a3e2ad4a"
  "19ae4a2e2aead10a3e2ad3a"
  "18a3e7a2e20a"
  "19a2e3a3ea2e20a"
  "25a2e2ae3ae3ae3a3e6a"
  "50a"
))

(area (aux-terrain stream
  "50a"
  "6agq7acy33a"
  "6agL8ahq32a"
  "6acT8agL32a"
  "7ahq7acXi31a"
  "7acXi7ab?y30a"
  "8ab?y8ahq29a"
  "10ahq7acT29a"
  "10acT8ap}4aem22a"
  "11ady7adU<3agM<21a"
  "12ahq7ady3agL22a"
  "12agL8ahq2agL22a"
  "12agL8acT2agL22a"
  "ai10agL9adyagL22a"
  "ady9agL10adygL22a"
  "2ad}mi6agL11ad:L22a"
  "3ab=?y5acT12ahL22a"
  "6ady5ahq11agL22a"
  "7ahq4agL11acT22a"
  "6aeoL4agL12ady21a"
  "6acU<4acTgq11ady20a"
  "7ady5ahwL12ady19a"
  "8ad}mi2acZL13ady18a"
  "9ab=?y2ahL14ahq17a"
  "12ad}igL14acT17a"
  "13abCwL15ahqae14a"
  "14agNT15acXicT13a"
  "14agLhq14aenKqhq12a"
  "14acTgL15a?YDgL12a"
  "15ad:T16abC:L12a"
  "16ab?y12ac}2moM<12a"
  "18ahq12ab=CYH2m11a"
  "18agL14acU3=<10a"
  "18agL15ady13a"
  "18acL16adq12a"
  "50a"
  ))

(area (aux-terrain ravine
  "50a"
  "7agq41a"
  "7agL14ai26a"
  "7acT14ad}3agq20a"
  "8ahq14ab<3a<20a"
  "9a<40a"
  "50a"
  "50a"
  "50a"
  "35ai14a"
  "35ady13a"
  "34agqdy12a"
  "18ae15agLab12a"
  "18agL14agL14a"
  "18agL14acL14a"
  "18agL30a"
  "19a<14agq14a"
  "34agL4ae9a"
  "34agL4acL8a"
  "34acT4ae9a"
  "35ahqeiaoL8a"
  "35agLgMab<8a"
  "36a<a<11a"
  "50a"
  "50a"
  "50a"
  "50a"
  "50a"
  "50a"
  "50a"
  "50a"
  "50a"
  "50a"
  "50a"
  "50a"
  "50a"
))

(area (aux-terrain trail
  "50a"
  "19ae30a"
  "20a@29a"
  "21a>u27a"
  "23a@3ac}21a"
  "24a@3aj>u19a"
  "25a>uaj2a>u17a"
  "27a@j4a@4ai11a"
  "28aE5a@3aj11a"
  "27akr6a@2aj11a"
  "27aj8a@aj11a"
  "27aj9a@j11a"
  "18ai8af10aE11a"
  "18aj9aD9aj11a"
  "18aj9aj9aj11a"
  "18ab9aj9aj11a"
  "28af9aj11a"
  "7ae21aD8af11a"
  "8aD20aj9aD10a"
  "8af20aj9aj10a"
  "9aD18akr9aj10a"
  "9af18aj10aj10a"
  "10a>u12ak3sv10aj10a"
  "12a@7acu2aj4aD9af10a"
  "13a>s2{su3a>uj4an10aD9a"
  "15a2j2a>q3a?suakr@6ai2aj9a"
  "15a2f10a>r2a@5aj2aj9a"
  "16a2D9acsu2a@4aj2ab9a"
  "16a2j12a>ua@3aj12a"
  "16ajn14a>sP2aj12a"
  "14acstr@2ae13a>wr12a"
  "19a@2a@14a@12a"
  "20a@2a@7aksu4aD11a"
  "21a<2a>6sr2a>3sr11a"
  "50a"
  "50a"
))

(area (aux-terrain road
  "50a"
  "7ae4ae2ai3ai10aksq17a"
  "8a>u3a@aj3aj8aksr19a"
  "acu7a@3a@f3aj7akr21a"
  "3a>u6a@3a@D2aj5aksr22a"
  "5a>u5a>u2aE2aj4akr24a"
  "7a>u5a@af2aj4aj25a"
  "9a>u4a@aDaj3akr25a"
  "11a>u3a@faj2akr26a"
  "13a>u2a@Djakr27a"
  "15a>uaAnkr28a"
  "17a>q=?2su26a"
  "15ak3syi3a>su23a"
  "13aksr3a2f6a>su20a"
  "12akr6aDH8a>3su15a"
  "10aksr7apr@12a>3su10a"
  "8aksr9aj@a@16a>2su6a"
  "6aksr11ajaDa@19a>3sqa"
  "4aksr13ajaf2a@24a"
  "3acr15aj2a@2a@23a"
  "20aj3a@2a@22a"
  "20aj4aD2a>su19a"
  "20aj4af5a@18a"
  "20aj5aD5a>u16a"
  "20aj5af7a>u14a"
  "20af6aD8a>u12a"
  "21aD5aj10a>u10a"
  "21aj5af12a>u8a"
  "21aj6a@13a>u6a"
  "21aj7aD14a>u4a"
  "21aj7af16a>u2a"
  "21aj8aD17a<a"
  "21aj8af19a"
  "21aj9aD18a"
  "21ab9ab18a"
  "50a"
))

(area (features (
   (17 "ridge" "Seminary %T")
   (16 "town" "Gettysburg")
   (15 "hill" "Barlow's %T")
   (14 "hill" "Granite %T")
   (13 "hill" "Oak %T")
   (12 "hill" "Benner's %T")
   (11 "hill" "Wolf %T")
   (10 "hill" "Culp's %T")
   (9 "hill" "Power's %T")
   (8 "hill" "Cemetery %T")
   (7 "hill" "Little Round Top")
   (6 "hill" "Big Round Top")
   (5 "ridge" "Cress %T")
   (4 "ridge" "Warfield %T")
   (3 "ridge" "Cemetery %T")
   (2 "ridge" "McPherson's %T")
   (1 "ridge" "Herr %T")
  )
  "50a"
  "50a"
  "50a"
  "33ao16a"
  "50a"
  "50a"
  "8ab41a"
  "8a2b40a"
  "7a3b40a"
  "7a3b3ac36a"
  "7a3b3ac36a"
  "7a2b4a2c3a3q3am25a"
  "8ab5ac4a2q29a"
  "8ab5ac35a"
  "8ab12ad28a"
  "8ab12aidak25a"
  "9ab11a2d6al20a"
  "21a2d27a"
  "21a2d27a"
  "22ad27a"
  "23ad26a"
  "24ad2ad22a"
  "25ad24a"
  "26ad23a"
  "25ab24a"
  "50a"
  "26ah23a"
  "26ah23a"
  "50a"
  "26ag23a"
  "50a"
  "50a"
  "50a"
  "50a"
  "50a"
  "50a"
))

(area (elevations (xform 1 370)
  "5*90,7a3:3k3:2a4*70,a5*130,17a"
  "6*90,:4*90,a4:2k3:2a2*70,2*110,2*70,5*130,16a"
  "6*90,:2*90,a2*90,a4:2k2:3a2*70,3*110,6*130,15a"
  "3*90,2:90,:2*90,a2*90,a4:3k4a3*70,2*110,4*130,260,2*130,14a"
  "3*90,5:2*90,a2*90,a3:3k2aXa3*90,3*110,5*130,2*120,13a"
  "2*90,2N4:2*90,2a2*90,5:k5X3*90,3*110,7*120,12a"
  "4N4:2*90,2a2*90,a4:a2X2*70,110,2*90,110,90,5*110,5*120,11a"
  "4N4:2*90,3aXa5:aX3*70,110,X110,3*90,2*110,4*100,3*120,10a"
  "4N3:3*90,3a2Xa4:2aX3*70,X11*90,3*120,9a"
  "3N4:3*90,a2:2Xa5:a:4X4*90,5*80,4*90,2N8a"
  "7:3*90,2a2:aXa4:2a:80,2X:4*90,8*80,N80,N7a"
  "7:2*90,D70,a2:aXa5:2a80,5:90,10*70,NaN6a"
  "a7:90,2D70,2:XaX4:D:a80,:2a:3N10X3N5a"
  "2a6:90,2D2*70,:XDX70,2:2*70,N:a:2a:17N4a"
  "3a5:90,2Da2:X2DX2:70,130,N:4a4:2N10:3N3a"
  "6a2:90,a3D:3DX3:2*130,:138,3a:a16:2N2a"
  "7a2:90,7DX3:3*70,:4a170,a18:a"
  "8a:8Da3:2*70,N:4a21:"
  "9a:7Da2:3*70,N2:3a100,20:"
  "9a:7D2a5*70,2:3a80,a2:a16:"
  "10a4:D90,3a2*70,2N70,2ak3a100,2a17:"
  "10a4:4a90,70,90,3N100,2a80,2a-30,2a17:"
  "12a2:3a:a2*70,N:2N70,4a2*-30,2a16:"
  "13a2:2a2:a90,N2:N100,70,4a2*-30,2a15:"
  "17a4:N3:70,5a2*-30,a7:a8:"
  "18a9:5a2*-30,a6:5a4:"
  "9a3:6a8:180,5a2*-30,3a5:8a"
  "10a3:5a6:70,:180,5a3*-30,a-30,k6:6a"
  "11a3:4a7:X80,6a5*-30,2a3k2:5a"
  "12a3:4a6:X290,:5a2*-30,-70,2*-30,a7k4a"
  "13a3:3a5:X3:4a2*-30,3*-70,2*-30,a7k3a"
  "14a3:3a5:3k:4a-30,3*-70,2*-30,2a7k2a"
  "15a3:2a5:ka-10,2:4a3*-70,3*-30,2a7ka"
  "16a2:2a5:kaka:5a2*-70,2*-30,5a4k2a"
  "20a5:ka-10,k:5a-30,2*-70,8a3ka"
  "20a5:ka2*-10,21a"
  ))
#|(area (elevations (xform 20 250)
  "50g"
  "g5sg3su12g6s21g"
  "g5sg2sgsu12g5s21g"
  "g2s2gsg2sgsu12g5s21g"
  "g2s5g2sgsu11g2s24g"
  "gs6g2s2g2s10g2s24g"
  "8gs3g2s10g2s24g"
  "8g2s3gs10g2s24g"
  "7g3s3g2s9g2s24g"
  "7g3s3g2s9g2s24g"
  "7g3s3gsgs8g2s24g"
  "7g2s4gsgs8gs25g"
  "8gs5gsgs7gs25g"
  "8gs5gsgs33g"
  "8gs5gs2gs3gu28g"
  "8gs8gs3g2u27g"
  "9gs7gs3g2u27g"
  "21g2u27g"
  "21g2s27g"
  "22g2s26g"
  "23gs26g"
  "27gs22g"
  "25gs24g"
  "26gs23g"
  "25gs24g"
  "50g"
  "26gu23g"
  "26gu23g"
  "50g"
  "26gu23g"
  "50g"
  "50g"
  "50g"
  "50g"
  "50g"
  "50g"
))|#

(side 1 (name "US") (adjective "Federal")
  (color "blue,white") (emblem-name "flag-usa")
  (names-locked true))

(side 2 (name "CSA") (adjective "Confederate")
  (color "gray,black") (emblem-name "flag-csa")
  (names-locked true))

(set sides-min 2)
(set sides-max 2)

(inf -21 -1 1 (n "Meredith") (cp -1) (appear 3))
(inf -21 -1 1 (n "Cutler") (cp -1) (appear 3))
(inf -21 -1 1 (n "Paul") (cp -1) (appear 4))
(inf -21 -1 1 (n "Baxter") (cp -1) (appear 4))
(inf -3 -16 1 (n "Rowley") (cp -1) (appear 6))
(inf -3 -16 1 (n "Stone") (cp -1) (appear 6))
(arty -21 -1 1 (n "1st Wainwright") (cp -1) (appear 5))
(arty -21 -1 1 (n "2nd Wainwright") (cp -1) (appear 5))
(inf -31 -1 1 (n "Cross") (cp -1) (appear 18))
(inf -31 -1 1 (n "Kelly") (cp -1) (appear 18))
(inf -31 -1 1 (n "Zook") (cp -1) (appear 18))
(inf -31 -1 1 (n "Brooke") (cp -1) (appear 19))
(inf -31 -1 1 (n "Harrow") (cp -1) (appear 19))
(inf -31 -1 1 (n "Webb") (cp -1) (appear 19))
(inf -31 -1 1 (n "Hall") (cp -1) (appear 20))
(inf -31 -1 1 (n "Carroll") (cp -1) (appear 20))
(inf -31 -1 1 (n "Smyth") (cp -1) (appear 20))
(inf -31 -1 1 (n "Willard") (cp -1) (appear 21))
(arty -31 -1 1 (n "1st Hazard") (cp -1) (appear 21))
(arty -31 -1 1 (n "2nd Hazard") (cp -1) (appear 21))
(inf -21 -1 1 (n "Graham") (cp -1) (appear 12))
(inf -21 -1 1 (n "Ward") (cp -1) (appear 12))
(inf -21 -1 1 (n "de Trobriand") (cp -1) (appear 27))
(inf -21 -1 1 (n "Carr") (cp -1) (appear 13))
(inf -21 -1 1 (n "Brewster") (cp -1) (appear 13))
(inf -21 -1 1 (n "Burling") (cp -1) (appear 27))
(arty -21 -1 1 (n "1st Rand") (cp -1) (appear 12))
(arty -21 -1 1 (n "2nd Rand") (cp -1) (appear 14))
(inf -43 -19 1 (n "Tilton") (cp -1) (appear 27))
(inf -43 -19 1 (n "Schweitzer") (cp -1) (appear 27))
(inf -43 -19 1 (n "Vincent") (cp -1) (appear 27))
(inf -43 -19 1 (n "Day") (cp -1) (appear 28))
(inf -43 -19 1 (n "Burbank") (cp -1) (appear 28))
(inf -43 -19 1 (n "Weed") (cp -1) (appear 28))
(inf -43 -19 1 (n "McCandless") (cp -1) (appear 31))
(inf -43 -19 1 (n "Fisher") (cp -1) (appear 32))
(arty -43 -19 1 (n "1st Martin") (cp -1) (appear 29))
(arty -43 -19 1 (n "2nd Martin") (cp -1) (appear 29))
(inf -48 -4 1 (n "Torbert") (cp -1) (appear 34))
(inf -48 -4 1 (n "Bartlett") (cp -1) (appear 34))
(inf -48 -4 1 (n "Russell") (cp -1) (appear 34))
(inf -48 -4 1 (n "Grant") (cp -1) (appear 35))
(inf -48 -4 1 (n "Neill") (cp -1) (appear 35))
(inf -48 -4 1 (n "Shaler") (cp -1) (appear 35))
(inf -48 -4 1 (n "Eustis") (cp -1) (appear 36))
(inf -48 -4 1 (n "Wheaton") (cp -1) (appear 36))
(arty -48 -4 1 (n "1st Tompkins") (cp -1) (appear 37))
(arty -48 -4 1 (n "2nd Tompkins") (cp -1) (appear 37))
(arty -48 -4 1 (n "3rd Tompkins") (cp -1) (appear 37))
(inf -21 -1 1 (n "von Gilsa") (cp -1) (appear 6))
(inf -21 -1 1 (n "Ames") (cp -1) (appear 6))
(inf -31 -1 1 (n "Coster") (cp -1) (appear 8))
(inf -31 -1 1 (n "Smith") (cp -1) (appear 8))
(inf -31 -1 1 (n "Schimmelfenning") (cp -1) (appear 6))
(inf -31 -1 1 (n "Krzyzanowski") (cp -1) (appear 6))
(arty -31 -1 1 (n "1st Osborn") (cp -1) (appear 7))
(arty -31 -1 1 (n "2nd Osborn") (cp -1) (appear 7))
(inf -48 -4 1 (n "McDougall") (cp -1) (appear 10))
(inf -48 -4 1 (n "Ruger") (cp -1) (appear 10))
(inf -48 -4 1 (n "Candy") (cp -1) (appear 10))
(inf -48 -4 1 (n "Kane") (cp -1) (appear 11))
(inf -48 -4 1 (n "Greene") (cp -1) (appear 11))
(arty -48 -4 1 (n "1st Muhlenburg") (cp -1) (appear 12))
(arty -48 -4 1 (n "2nd Muhlenburg") (cp -1) (appear 12))
(inf -21 -1 1 (n "Stannard") (cp -1) (appear 11))
(inf -48 -4 1 (n "Lockwood") (cp -1) (appear 25))
(arty -31 -1 1 (n "Ransom") (cp -1) (appear 24))
(arty -31 -1 1 (n "McGilvey") (cp -1) (appear 28))
(arty -31 -1 1 (n "Taft") (cp -1) (appear 24))
(arty -31 -1 1 (n "Huntington") (cp -1) (appear 25))
(arty -31 -1 1 (n "Fitzhugh") (cp -1) (appear 25))
(cav 20 24 1 (n "Gamble"))
(cav 20 23 1 (n "Devin"))
(cav -43 -19 1 (n "McIntosh") (cp -1) (appear 36))
(cav -43 -19 1 (n "Gregg") (cp -1) (appear 36))
(cav -43 -19 1 (n "Farnsworth") (cp -1) (appear 57))
(cav -43 -19 1 (n "Custer") (cp -1) (appear 57))
(supply-train -48 -4 1 (cp -1) (appear 16))

(inf -1 -32 2 (n "Anderson") (cp -1) (appear 18))
(inf -1 -32 2 (n " Benning") (cp -1) (appear 18))
(inf -1 -32 2 (n "Law") (cp -1) (appear 31))
(inf -1 -32 2 (n "Robertson") (cp -1) (appear 18))
(arty -1 -32 2 (n "Henry") (cp -1) (appear 19))
(inf -1 -32 2 (n "Barksdale") (cp -1) (appear 19))
(inf -1 -32 2 (n "Kershaw") (cp -1) (appear 19))
(inf -1 -32 2 (n "Semmes") (cp -1) (appear 20))
(inf -1 -32 2 (n "Wofford") (cp -1) (appear 20))
(arty -1 -32 2 (n "Cabell") (cp -1) (appear 20))
(inf -1 -32 2 (n "Armisted") (cp -1) (appear 36))
(inf -1 -32 2 (n "Garnett") (cp -1) (appear 36))
(inf -1 -32 2 (n "Kemper") (cp -1) (appear 36))
(arty -1 -32 2 (n "Dearing") (cp -1) (appear 37))
(inf -1 -32 2 (n "Alexander") (cp -1) (appear 21))
(inf -1 -32 2 (n "Eshleman") (cp -1) (appear 21))
(inf -19 -34 2 (n "Gordon") (cp -1) (appear 9))
(inf -19 -34 2 (n "Hays") (cp -1) (appear 9))
(inf -19 -34 2 (n "Hoke") (cp -1) (appear 9))
(inf -19 -34 2 (n "Smith") (cp -1) (appear 10))
(arty -19 -34 2 (n "H. Jones") (cp -1) (appear 10))
(inf -1 -32 2 (n "Jones") (cp -1) (appear 13))
(inf -1 -32 2 (n "Nicholls") (cp -1) (appear 13))
(inf -1 -32 2 (n "Steuart") (cp -1) (appear 13))
(inf -1 -32 2 (n "Walker") (cp -1) (appear 14))
(arty -1 -32 2 (n "Latimer") (cp -1) (appear 14))
(inf -7 -34 2  (n "Daniel") (cp -1) (appear 7))
(inf -7 -34 2 (n "Doles") (cp -1) (appear 7))
(inf -7 -34 2 (n "Iverson") (cp -1) (appear 7))
(inf -7 -34 2 (n "O'Neal") (cp -1) (appear 8))
(inf -7 -34 2  (n "Ramseur") (cp -1) (appear 8))
(arty -1 -32 2 (n "Carter") (cp -1) (appear 8))
(arty -1 -32 2 (n "Dance") (cp -1) (appear 15))
(arty -1 -32 2 (n "Nelson") (cp -1) (appear 15))
(inf -1 -32 2 (n "Mahone") (cp -1) (appear 12))
(inf -1 -32 2 (n "Perry") (cp -1) (appear 12))
(inf -1 -32 2 (n "Posey") (cp -1) (appear 12))
(inf -1 -32 2 (n "Wilcox") (cp -1) (appear 13))
(inf -1 -32 2 (n "Wright") (cp -1) (appear 13))
(arty -1 -32 2 (n "J. Lane") (cp -1) (appear 13))
(inf 5 30 2 (n "Archer"))
(inf -1 -32 2 (n "Brockenbrough") (cp -1) (appear 4))
(inf 5 30 2 (n "Davis"))
(inf -1 -32 2 (n "Pettigrew") (cp -1) (appear 4))
(inf -1 -32 2 (n "J. Garnet") (cp -1) (appear 4))
(inf -1 -32 2 (n "Lane") (cp -1) (appear 7))
(inf -1 -32 2 (n "Perrin") (cp -1) (appear 7))
(inf -1 -32 2 (n "Scales") (cp -1) (appear 7))
(inf -1 -32 2 (n "Thomas") (cp -1) (appear 8))
(arty -1 -32 2 (n "Poague") (cp -1) (appear 8))
(arty -1 -32 2 (n "McIntosh") (cp -1) (appear 5))
(arty -1 -32 2 (n "Pegram") (cp -1) (appear 3))
(cav -19 -34 2 (n "Hampton") (cp -1) (appear 33))
(cav -19 -34 2 (n "Jenkins") (cp -1) (appear 10))
(cav -19 -34 2 (n "F. Lee") (cp -1) (appear 33))
(cav -19 -34 2 (n "W. Lee") (cp -1) (appear 33))
(supply-train -1 -32 2 (cp -1) (appear 16))

;;; Should be more elaborate.

(scorekeeper (do last-side-wins))

;;; We get at least three days.

(set last-turn 72)

;;; ... and possibly another few hours.

(set extra-turn-chance 10)

(set calendar '(usual hour))

(set initial-date "6:00 1 Jul 1863")

;; The game starts at six hours before noon.

(set initial-day-part 16.00)

(add u* already-seen 100)

(set feature-boundaries false)

(game-module (design-notes (
  "This is a brigade-level simulation."
  ""
  "The scale is 5 hexes/mile, 1 hour/turn."
  )))

