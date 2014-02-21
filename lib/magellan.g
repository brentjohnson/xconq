(game-module "magellan"
  (title "Magellan")
  (blurb "Circumnavigate the world and bring back treasure.")
  (base-module "voyages")
  (variants
   (world-seen false)
   (see-all false)
   (sequential false)
   ("Last Side Wins" last-side-wins false
     (true (scorekeeper (do last-side-wins))))
   ("Wind" wind
    "Include winds and their effects on sailing."
     (true
       (add fleet speed-wind-effect
         ((0 ((0  10) (1 100) (3 120))) ; downwind
          (1 ((0  10) (1 120) (3 150)))
          (2 ((0  10) (1  40) (3  20)))
          (3 ((0  10) (1  10) (3  10))) ; upwind
          ))
       (add t* wind-force-min 0)
       (add t* wind-force-average 1)
       (add t* wind-force-max 3)
       (add land wind-variability 30)
       (add waters wind-variability 10)
       (set wind-mix-range 1)
       (set see-weather-always false)
       ))
   )
  (instructions (
  "Take your fleet and crew and try to sail around the world."
  ""
  "Be careful not to get lost or wreck your ships,"
  "don't run out of food and water, and look for treasure."
  ))
  )

;;; Modify the basic game to increase the amount of what's already known.

(add city initial-seen-radius 10)

(add fleet image-name "caravel-fleet-spain")

;;; Use the big map of the world.

(include "earth-1deg")

;;; One specific side only.

(set sides-max 1)

(side 1 (name "Spain") (noun "Spaniard") (adjective "Spanish")
  (emblem-name "flag-spain-old")
  (names-locked true)
  )

(set advantage-max 1)

;;; Suppress creation of any other units.

(add u* start-with 0)

;;; Magellan starts out near Madrid.

(explorer 125 105 1 (n "Magellan")
  )

(crew 123 103 1)

(fleet 124 102 1)   ; should start in Cadiz perhaps

;;; Set up some European cities of Magellan's time.

(city 124 103 1 (n "Cadiz"))
(city 126 106 1 (n "Madrid"))
(city 122 104 0 (n "Lisbon"))
(city 125 104 1 (n "Cordoba"))
(city 128 107 1 (n "Zaragoza"))
(city 131 107 1 (n "Barcelona"))

;;; (should add some native cities)

(side 1 (terrain-view
  "360a"
  "360a"
  "360a"
  "130a3i227a"
  "120a9i4b2i2a4f219a"
  "102ai16a3i5bi6b8f218a"
  "99a12b4a2ia3i11b2h9f217a"
  "97a20b3i10b4h5f2b5f214a"
  "93a36b6h3f4b5f213a"
  "91a37b7h3f4b6f212a"
  "90a37b8h2f4b8f211a"
  "90a36b2f7h2f4b9f210a"
  "90a36bf8h3f4b9f209a"
  "90a3b2ih30b2f6h4f10b4f208a"
  "76aib12a4b3h29b2fh6g3fc10bfb2f207a"
  "76ah3b10a5bfh19b2f9bf2g3b2g3fc3bcfc9f206a"
  "76a6b9a24b2f2h12bfbfg3fc3bcgc4f2g4f205a"
  "92a25b2h12b2f2b3fc3bc6f2g6f203a"
  "94a20b2fb4f10b4f7bc17f200a"
  "95a19b3f2b4f9bf4cbc3fc11f2g5f199a"
  "96a18b3f3b3f6b17fg15f198a"
  "96a18b3f2b5f4b10f2g23f197a"
  "93a22bf4b5f2b8fg14f2g11f197a"
  "88a31b3f5b4fg18fg13f196a"
  "79a44bfb4fg2fg12f4h17f194a"
  "72a24b3a23b17f2h4fh4fh18fb2f188a"
  "66ab6f19b6a25b11f6hf2h7f2h16fb5f186a"
  "65abf16b14a28b9f6hf2h9f2h5fb2f3c10f185a"
  "64agf10b20a30b2f2g2f2h6fc8f5h2f4b3f2c4fa5fb184a"
  "62a2g5b26a32b3fg2fhfh5fbc2fh11f4bfbc11f2b183a"
  "62af6b25a26b3f2hfgh2fh4f4b3fbc2f2hfhfh5f10b2f2h5fb183a"
  "62a5b26a27b2f3h4f2hf5bf3b3f2bcf2h3f3hf12b2f5hf183a"
  "61a6b22a32bh10f10b4fbcf3h6f3b4f5b2f188a"
  "60a6b23a12ba19b2f3h5f7b2f4b4fbf2h2f2cfb3fhf2h7f188a"
  "62a2b23a12b2a20b9f2bf6b2f5bf2bcfhf5b8f2hfh2f188a"
  "86a12b2a3bf17b5f3h2f16bf3bcfhf2b4f2h2efh2fhfh188a"
  "84a14ba6bf17b7f17bf4bhf2bfb5f3h4fhf188a"
  "83a7b3a3b2a27bf8b9f2b2f7bh5bfbfb2f2b2fe189a"
  "81a9b7a35b4f5h2f13b2fbf8b2fe189a"
  "80a15ba30b2fh5f5h4f5be16b2f2bfe189a"
  "76a49b4f7h2f5ef25bfe189a"
  "76a41bf7b4f4h9eae5f5b3f12b2f189a"
  "74a50b3f4h11e2a4ef5b5f5b2f3b2f9a3f2h175a"
  "39a4fc8f22a18b2a31b3f2h7e11ae7f9e3f4e8ae3f2hg173a"
  "39af3bfcb6cfg6b13a18b4a22bf7b2fh8e13a15e5fh3e8aef2b3hf171a"
  "38a2f12bc2f7b10a18ba28bfbf3b11e27a3ef2ebhb2e8a2e3bh5fe166a"
  "38af15b2f11b5a51bf11e29a3ef2e2b2e9a2e4b7f6aef156a"
  "36a4f14bfgf13b2a51bf3e40aef2e2bf10a3e7b4f5e2f51afb102a"
  "36a4f15bgfbf63b3e43aef2e2be10a2ebe3bf3bf3efe4f2e48af2b101a"
  "34a2e4fc18bcf61b3e44aef2e2be10a4e2b2ef10b4e3f42a3fbf101a"
  "35aeh3f21bcf59b3e43a2ef2e3bf10a7e3f9be6f38a4f3bf101a"
  "35a2h4f14b5f4bcf57b2e43a2ef3e3be12a4ea3ef9b6f12a4fb19a2fbf5b101a"
  "36afh4f17b6f58be45af2e2ae3bf2e16a2e10b2fb2fg9a3f4bf17af7b103a"
  "37ahf2h2f6bg3f10b3f58b45aef2e2ae4bfe14a3e14bgf8a4f5bg16a7b104a"
  "38a2hfh2f5bg2ag14b3f54be45ae2fe2ae4b2e12a4e14bgf7a4f7bfg15af2b107a"
  "40a2fh3f3bg2ag10b2f3b5fb2fcf47be45a2efe2a2e3b3e10a3e16b2f6a3f10bg14afg2b106a"
  "43a3g2f2g2ag24b2c46be46aefe4af3bf2e5a6e17bgf4a4f11bg14a4b106a"
  "49ag2af27bf12b5a19bf7b2f46ae2f4af3b3e4a6e18bfg3a4f12bfgbf10a4b106a"
  "53a5g22b2c11b4a28b2f47a3f4af3bf3e3f2e22bfg2a2f18bg9af2b107a"
  "58a2g21bfc11b2a23bf5b2f48a2f5a2f2bfh4f25bfga3f6ba11b10af2b106a"
  "59ag22bcf42b3f55afb3f14b5a10bgfa2f5b5a2bc5bga2f6af2b106a"
  "60af13bcf6bf17b2a25b3f55af16b8a9b2fa2f4b7a2bg4bgfbfgf3agf2b105a"
  "59agf10bfbfc9bf14b2a27b2f56ae6b2e3bf3b11a6bg4f4b8abc4bgf3bgf3af2b105a"
  "59agfg7b2fhfc5f2bfbf13b4a29bf56afe2b3eae6b12a2bf4bgaf5b8abg4bg5b2g2f2b105a"
  "60abf3b2f2bf2afcha8f14b3a30bfg56a2ef3ae6b13a6b3f2bg2b9ac4bg6bf4b105a"
  "61abg2fg5f2afh9a2f43ba3bg61ae5b16a6bf2b2g2b9abg3bfg9b105a"
  "62a3bf2bg2f15ag42b2a3b2g59ae5b17a8bgf3b9a5bgf6b107a"
  "68a2bgf15a2g40b3a4bg8agf3gfg43ae5b19a7b2f2b10a6bgf2b109a"
  "69abgh17afgfgf34b5a4b5gf4g5bg2a2g38ae5b21a10b10a2bg2bfgf2b108a"
  "94a2g32b8a4bg12b2gfbg36a2e5b26a5b10a2bgagb3g2b107a"
  "96agc29b10a12b3a6bg35ae6b42abcgfgbf2g2b16a5b85a"
  "97agc27b14a3b10a6b34a2f5b44a2bc2gcbfg4b10a9b84a"
  "98af27b31a2bg32afe6b45a2bc3gbf5bf8af5bf3b83a"
  "99afc24b33a2bg30a2f7b45a2bgbgag7bg7af2b4gf4b81a"
  "99agfcg22b33a2bg30af7b47a4bghgf5b2g6af2bg5bg5b78a"
  "100a2cfgfg19b33a2bg29a2f7b48a4b2ga2g5bg5af2b5g3bg3bg77a"
  "101a2g3afg16b35a2b2g27af7b52a2b2g2agbg2bf2g2fgc2b3g10b2g75a"
  "108af3g11b37a3bg27af6b53a3bg2af5b3c2fc2bgbg6b2g3bg74a"
  "112a2f5bf3b38a3bg26af5b55a3b3g13bfb2g11b74a"
  "114a4f5b40a2bgf24af4b58a3bgf13bf3bg10b73a"
  "117af4b42a2bcg23af4b59a5bfb2cgf23b71a"
  "107ag10af2b44a2bf24af4b60a4b7f20bgb70a"
  "118af3b43a2bf25af4b62a7bfgbfbf2bcfbcgf2b2cg2c2b70a"
  "118af3b44a2bf25af5b61a14bfbcbfc2bcbcb73a"
  "118af3b45a2bg25af6ba4b59a20b74a"
  "117a2f2b46a3bf25af11b59a17b76a"
  "117af3b46a3bg25af3bf4bf3b151a"
  "117af3b47a2bf26af7b2f3b150a"
  "117af2b48a2bf26af7b2f5b148a"
  "118af2b47a3b26af6b2fag5b147a"
  "118af2b48a2be25af5bgf2ag6b146a"
  "118af3b47a3b22a2f7bg3ag7b145a"
  "118af4b47a2be21af8bf3ag7b145a"
  "119af4b47a2be20af9bf3ag9b142a"
  "119af5b47a2be19af9bg3ag8bfb141a"
  "118a2f6b47a2be19af8bg3ag7bf2b141a"
  "116a3f9b45a2b2e19af8bg2ag10b141a"
  "114a3f4b2a10b41a2b2e18af9bgaf3b148a"
  "114af6b3a22b29a2be17a2f10b2f2b149a"
  "113a2f4b5a26b26a2be16af12bf2b149a"
  "113af3b8a29b22a2be15a2f15b149a"
  "114af2b9a2b2a3b8a3b2a14b17a2be14a2f10ba4b149a"
  "114af2b33a12b16a2b2e12a2f5b159a"
  "113a2f2b37a12b13a3be12af4b160a"
  "113af3b42a12b9a2bef11af3b161a"
  "112a2f3b44a14b2a2b2a3be9a2f3b161a"
  "108a2f2af3b50a15ba2bf9af3b162a"
  "108afb3f3b54a15bfa8f3b162a"
  "109af5b60a11b2f9b163a"
  "110af4b66a16b163a"
  "186a4b170a"
  "360a"
  "360a"
  "360a"
  "360a"
  "360a"
  "360a"
  "360a"
  "360a"
  "360a"
  "360a"
  "360a"
  "360a"
  "360a"
  "360a"
  "360a"
  "360a"
  "360a"
  "360a"
  "360a"
  "360a"
  "360a"
  "360a"
  "360a"
  "360a"
  "360a"
  "360a"
  "360a"
  "360a"
  "360a"
  ))
