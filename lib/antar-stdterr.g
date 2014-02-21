(game-module "antar-stdterr"
  (title "Antarctica Standard Terrain")
  (blurb "Kludge for converting Antarctic terrain types to stdterr.g types")
)

(include "stdterr")

(define ice-edge shallows)
(define ice-shelf desert)
(define icy-lowlands plains)
(define icy-highlands ice)
(define icy-mountains mountains)
(define buried-lake ice)

(add sea image-name "civ-ocean")
(add shallows image-name "atoll")
(add desert image-name "light-cyan")
(add plains image-name "civ-tundra")
(add ice image-name "civ-glacier")
(add mountains image-name "civ-mountains")

(add t* river-chance 0)
