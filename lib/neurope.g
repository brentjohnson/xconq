(game-module "neurope"
  (title "Neurope")
  (blurb "Classic Xconq set in Europe, but with random sides.")
  (version "0.1")
  (base-module "classic")
  (default-base-module "classic")
  (variants
    (world-seen true)
   (see-all true)
   (sequential false)
   (one-city
    (true
	 (add T start-with 0)
	 (add @ start-with 1)
	 (add u* independent-near-start 0)
	 (table independent-density (u* t* 0))
     ))
   (bare
    (true
	 (add u* start-with 0)
	 (add u* independent-near-start 0)
	 (table independent-density (u* t* 0))
	 ))
   ("Alternate economy" model1
    "Use a different algorithm for moving materials among units."
    (true (set backdrop-model 1)))
   ("AltEcon uses doctrine" model1doctrine
    "When using alternate economy, don't drain units below doctrine levels."
    (true (set backdrop-ignore-doctrine 0))
    (false (set backdrop-ignore-doctrine 1)))
   )
  )

(world 240)

(area 240 200)

(area (terrain
  "13a2ea22f2e5a3e93fi98a"
  "11a2e2ae22fe6a3e93fi65ab32a"
  "9a6f2e19f2e7ab3e93fi97a"
  "9a27fe7a4e94fi97a"
  "7a14fg13fef8a4e14f2b17f2b59fi96a"
  "7a13fg13f3e7a5e6fb9fb14fb2f2b58fi96a"
  "5a7fg22f2e8a5e6fb10fb13f2b2fb58fi95a"
  "5a5f4g22fe8a5e5f2b6fb3fb14f4b58fi95a"
  "4ab5f2g2h3g19fe10af3e4fbfb5f2bf2b16f4b57fi94a"
  "3a6f2gh2ghg19f2e10af2e12fb5fb3f3a9f5b56fi94a"
  "3a6f2g2hgf3g17f3e10af2e15f3b4f6a9f2b56fi93a"
  "3a5f3g2hg2f2g17f2e11a3e15fb6f7a66fi93a"
  "4a5fgfg2h2g20f2e12a2e14fb7f7a66fi92a"
  "4a7fg3hg21fb12a2e22fe6abe64fi92a"
  "5a8fgh3g20fb12a2e22fe6a2e64fi91a"
  "5a7f2hg23fb12a4e19f3e5a2e64fi91a"
  "7a2e2f2g2hg24f11abe2f3e9f12e2a6e62fi90a"
  "6a4ef3gh25f11a3efef10e5a7ea8e38fe22fi90a"
  "7a4e2fghg26f10a14e11a12e38f2e21fi89a"
  "7a4e3ghg26f2e12a8e11a14e19f2b5fe2f2e2fe3f2efe20fi89a"
  "9a2e2g3h2g4f3e17f3e11a7e11a8efefef3e18f2e2be3fe3fe4f3e23fi88a"
  "8af2ef2gh2g4f5e14f5e10aba5e11a10e4f2e19fb2ebf2efe2fe4f3e3f2e3fe15fi88a"
  "9a4fgh3g4f5e14f7e21aebab14e11fe8fe2fbe3b2fefefefe2fb5fe3fe15fi71ae15a"
  "9a3fe3g6f5e13f8e18a8e2fef3efefe13fe6f5e5b7eb2eb6e2fefe15fi65a2e20a"
  "10a4f2g7f2eae12f9eb16a11e17fe5fe5f7e3b9e3b9e8fe8fi59ae4a7e15a"
  "11a3fg8f2ea2e11f5ea2be16a9e2b30f21eb6efefe6fe9fi64a6e16a"
  "12a11f3ea2e7f3bf6eb3e14a5ef4e3b3f2be22f27e2fe9f2e8fi62a7e16a"
  "12a9f4e2ae8f3b10e12aeab5e2f3e3b2ef3b8f2e11f30e10f2e8fi61a7e17a"
  "13a7f3e5ae4f2bf4b7e15ab2a5e2f4e2b3fefb7fe13f17ef13efe2fe5fe8fi61a6e17a"
  "13a7f2e7a4f2bf4b6eb16aba4e3f4e2b8e19f12ef4ef18e4fe7fefi61a10e13a"
  "15a6fe8af3e4b8eb14a3eab3e6f3e2be2f4e10f2e6f5ef9ef5ef2ef2ef10e2fe5fefe2fi59a14e10a"
  "16a5fe8af3e3b2e2b5e15a3e3abe3fe3f3eb3f4e9f2e6fef3e2f5e2f24ef2e4fe2f2e2fi60a13e10a"
  "31af8e2b5e22a4f2e2f10e17f3efef7efef5efef6efef9ef10ei59a14e9a"
  "31ab7e2b2f4e22a16e16f6e2f2e2fe2f2e2f3efef4ef11ef14ei59a13e10a"
  "33a6e2b4f3e17af5a14e8f2e7f8e2fe3fef6efef17ef8ef4ei59a11e11a"
  "33a6eb5f3e16a3f4af13e9f2e6f6efef5e4f4e2f2ef3ef13e2f10ei60a9eb11a"
  "28abe4a4e2f3e4f2e5ae9abe3f3aef13e10fe5f6e2f8efef3e3f4e2fef23ei59a10e11a"
  "28a2e5a3ef4e4f2e5ae9a2e3f2ab3f11e10f3e3f8ef8e2fefe4f4e5f5ef4ef11ei61a7e12a"
  "28a2e6a2e2f4e4feb14a2e5f2e3f12e8f3e3fef7ef9ef2efefef4efe2f2ef20ei61a8e10a"
  "24a2ea3e7ae2f4e4fe15a4f2ef3e3f10e2fef2ef3e4fe2f6e3f4efef2ef2efef5e4f6e3fef3e2f3ef3ei49a2ea2e8a8e9a"
  "25a7eb6aef4e3fe15abef5e7f10ef2e2f3e4fefe3f7ef10efef2ef5ef7efe3f12ei48a8e5a9e8a"
  "25a6e8a5e2f3e2b13a14ef24e3fef6ef13ef11ef3e4fef11ei48a9e3a11e7a"
  "26a8e6a4e3f3e15ab41ef6ef13e2f7ef7e4fe2f10ei43a2e3a9e3a11e6a"
  "25a9e6a4e2f4e16a40e2f2ef26ef8e6f3ef6ei43a14e3a11e6a"
  "27a6e6aea7eaeb16a25ef7ef2ef3e2f8ef11ef6e3fef4ef2e5f2ef7ei42a15e4a10e5a"
  "26a7e4ab2ea5e21aef26ef11ef4efef13e2f6e3f6efe10f6ei42a15e7a7e5a"
  "27a6e3ab4ea5e21a27ef9ef9e2f21e4f6efefe4fe2f5ei42a14e7a9e3a"
  "27a7e2a4eba5e22a26ef12ef2e2f5ef7ef2ef7e4f3ef7e5fef5ei42a13e5ae2a9e3a"
  "28ab8ea3e2a4e23a26e2f3ef3ef8ef4ef10ef8e4fef8e7f5ei43a12e9a9ea"
  "28a2b7ea3e29a25efef4e2f5ef10ef3ef4ef8e5f3efef6e4f6ei42a13e10a8ea"
  "e30a4eab2ab2e8ae17ae2b33e2f14ef3efef2e2f8efe2f8e2fef2e2f7ei42a13e9ab8e"
  "e28aba4e3a3e26a32ef18e3fe2f2ef12e2fe2f6e3fef11ei41a13e10a9e"
  "2e28a2b4e5abab14a5e6a19ef16ef13e6f17efef5e5f12ei39a15e5aba11e"
  "2eb28ab5e20a7e3a50e3f22e2f7efefe2f10ei39a15e5a7ef5e"
  "3e30a7e4ab4e8a2efe2f2e2b5e3f25ef3ef12e3f2e2f25efe4fef11ei38a15e6a12e"
  "3eb29a7e2a5ebab4ab9ef2ef6e2f32ef8e2f4ef29ef9ef4ei38a14e7a12e"
  "4e28aeab25ef29ef10ef2e2fef7efe3f4ef44ei39a5ea2e12a11e"
  "4e24a3eb3eb14eba23ef3ef4ef19e2f10e2fef3e2f18ef10ef14ei59a11e"
  "9e14ab3e2a3ea42ef8ef2ef7ef30ef43ei57a12e"
  "10e13ab4eb3eb45e3f10ef11ef14ef5ef20ef3e2f5ef13ei56a13e"
  "11e12a14e3f38e3f7ef82ei55ab12e"
  "11e8a2ebea14ef2ef15efe2f2ef24e4f2ef13ef3ef58ei58a10e"
  "11e9a2e2bea15e2f13e2f3e3f20ef5e7f2e3f7ef62ei59a8e"
  "11e8a3e2b16e2f15ef33e13f16efef49ei63a4e"
  "10eb9a36ef26ef7e12f17ef52ei56a10e"
  "9e10a37e2f34e12f17ef51ei55a11e"
  "10e10a40ef35e11f13e2f3ef12ef32ef2ei54a11e"
  "11e9a22ef13efefef36e10f13ef17ef35ei53a12e"
  "11eb6abea22e2f6e3f6e2f34e11f13ef18ef35ei56a3e3abe"
  "2ae3aeab4a32ef4ef45e12f30e2f35ei64a"
  "13a22ef56ef5e2f2e3fe3f30ef36ei63a"
  "13a21e2f2ef38ef15ef47ef34ei63a"
  "14a22e4f102e2f33ei62a"
  "13ab12ef9e4f42e3f11e2f46ef32ei62a"
  "2a2e8a13e2f9e4f18e5fef19ef26ef66ei61a"
  "2a3e5a13e3f11e2f19ef27ef2ef35e2f20e2f8ef5ef14ei61a"
  "3a5e2a13e4f12ef3e3f3e2f6e2f22ef4efe2fef19ef14efe2f2ef32ef13ei60a"
  "3a19e2f3e3f13e2f5ef4ef16ef2ef9ef2ef2ef3ef12e2f23e2f30e2f12ei60a"
  "4a34ef4e2f10ef18efef13e2f2efef6ef3ef27ef31ef12ei52a7e"
  "2e2a33e2f3e3f3ef6ef16e8f8e2f3ef14ef3ef23eb30ef12ei52ab6e"
  "38e2f4e2f2e2f6e2f19e2fef11ef2efef7e2f3ef26eb43ei53a5e"
  "37e2f4e3f3ef7ef17e3fefef11ef2ef41eb42ei53a5e"
  "29e2f6e3f4e2f12ef19e4fef2g9e2f2ef40eb12e2a27ei54ab2e"
  "29ef2ef4e2f3ef36efe3fgf4efe7f2ef38eb10e4a27ei56ae"
  "b27e3f2ef3e2f3e3f6e4f6e3f13e2f2e5fg2e3fefe3fe2gf39eb9eb2a29ei56a"
  "2a25e2f3ef3e2f4e2f7e2f10ef20e12fefef2gf37eb10eba30ei56a"
  "3ab27e2f5ef4ef7e2f14ef16e6f2efef2e3fef2gf4ef29e2b9eab4a28ei55a"
  "4a18ef6e2f26e2f9ef17ef13efe2fg2f3ef27eb9ea2b5ab27ei55a"
  "5a18ef6ef8ef4efef10e2f3ef38ef2ef3g2f28eb8ebea2b6ab26ei54a"
  "5a17ef6ef13e2f9e5f2e2f42e3gfefef24eb8e2a3b6a28ei54a"
  "6a17ef32e7f8e3f22ef3e2f2efe4gfe2f3ef19eb8eba5b5a28ei53a"
  "7a45e4fefg4fef3g2f4g3eb19efef2ef2efef3g2fe2f16e2ba2b7eab2a7b3a28ei53a"
  "9a32ef8efe8gf14g24efefef4ef3gfefef15e4a5e2b3eb6a2e2a22e3f5ei52a"
  "9ab29e2f8eg2f6ge12g4f23ef10efgfgfef2ef13e5a3b3a5eb4a2eab19ef6g4ei52a"
  "11a27e3f4eg2f23gfg33ef4egf2g2fef14e9a9eb4e3a15e2f10g2ei51a"
  "11a27e2g2efef11gh16g11eb16efgf3ef3efgfg2fe2f12e10a13e6a7e2f11g4h2gei51a"
  "11a28egebe27g3fg10e2b17e2gfe2f4e3fgef2ef10eb10a10e14a2e2f9g6h2gei50a"
  "12a26ef2egf26g14e2b16efe3f9efefe3f9e14a8e16a2ef6g4hg4hgei50a"
  "12aea10efe2f13ef15gh2gf5gf3g2f2g32ef11egef2ef8eb15a3e21a2e15gei49a"
  "12afa12efg3ef8ef4g3h2gf15g2efgf2ef44e2ge2f9e16a2e24a2e2ge10gei49a"
  "13afa5e2f5efg2e2f8e6gh2g4f6ge2gf37ef6e2f3e3fe3fe2f10e16a2e29a5e2g2fgei32ae15a"
  "12a2fa3e3f5ef2g2e2f8ef3gh3g2e6f2ge2gf44ef3gf7gef2ef9e49a9ei31a4e13a"
  "13a3f4ef6ef2gfe4f4ef4gh3g6ef4e2f41ef2e3f3g5fg4e2f8eb51a6efei27a11e9a"
  "13a3f12e2f2g2fg4e2f4ghgf20e3a3eg28e2fe3gf2g3e2f4ef10e53a3ef2gei27a14e6a"
  "13a4f12ef5gf3ef3gh3gf20e4a4ef9efef16efegf7efe3f12e54ae4gei27a9ef6e3a"
  "13a5f11efgf3g4ef3gh3g20e5a5ef8e3fef12e2f14efe2f10e54ae4gei27a8e4gf5ea"
  "4e9a7f10e3fgf5ef7g20e5a13e3f3ef14ef11ef13e55ae4gei27a6ef10gf"
  "g4eb7a3e4f10e4f2ef2e3f6g7e2f12e5a12e2f6ef10e2f11ef14e55a5gei26a7e2fef8g"
  "2gf7ea22ef3ef2e3fe5g7efgf9e9aea3efefe3fef2e3f9ef2efef23e54ae5gei26a5efg2f3g3f3g"
  "g3f33efe4fef6gf3ea4ef8e10a3e3fgfe2f5ef10efef25e53ae6gei26a3efef5g6fg"
  "6f9ef15ef9e3fef3gf2g2e5a3e2g6e11a4ef2gfegfef2efef12ef23e53a5ghgei25ab2efgfef3g2f2e3f"
  "6f8ef4gf22ef2ef2g5e7a3ef7e10a5e4g2fe4f10efef24e53aegh2gh2gei26a7e4f4e2f"
  "2e4fgf8ef6g13e3a11e12a12e9a4ef5gfeg4f35e50a2e2gh3gh2gei25a11ef5e"
  "fe3f4gf7e8g2f7e8a7e14a6ef5e9ab4ef2g2fgegf2ef33e28a3e16a4ef6gh3gei25a16ef"
  "2e4f6g5e3f10gf3e10a4e16a13e9a5efgf3g5f2g3f6egf21e23a4e2f6e10aef9gh4gei25a16e"
  "2e5f5g7efge9g3e13ae16a14e9a7ef6gf3geg7e2gf19e21ae2f2g2f17e16gei25a11ef4e"
  "3e6f5g10ef8g2e30a14e10a7ef8g4f4e2f2e2f2e3fgf10e21a2e2g4f4e2f4ef2ef19gei24a11e4fe"
  "3e2f3g2f4gf9efe6g3e31a9ef3e11a6ef10g6eg2f4efgf2efe2f4e2f2e19a3f2g2f3g2fg3f2ef23gei23a12e4fe"
  "5f7g3f14ef6e25ae6a8e2g2e13ae2a3egf5gf3efefe2g2fg6efe10fe18ae4f4gf2e2g2egf3e22gei23a5eg5e5f"
  "2g2f8g2f22e23a3e6a8e2gf2e15a3e4f3g7ef2g2f2gf4e13fe16ae2f2e6gfe2f2e2f25gei22a8e8fg"
  "7g4f4g3f18e23aeg2e7a7e3g2e17a5e3g3ef3ef2g2e4g4e3fe2fe2fe2fe15ae4f7gfgfgfefef24gei21a4ef9ef2g"
  "3gfg2f2e2f5gfgf16e24aegfe8a6ef2gfeb18a3e2f3e2gef2e2gfe5gfg2e4fef2e4fe12a2e3f8gfe7f25gei21a2e3f9e3g"
  "10e3f7g14e27aefe9a5ef2g4e18a4e2f2gf6e3f7g16e3a2e2f3efe9gf2ef3g2f27gei20a17e"
  "10e3f8gf8e31aefe10a4efgfgf4e17a3ef4gf4ef2ge7g5f12ea6e2fef8gf3ef32gei19a18e"
  "12e2f9g2f3e34a2e13a3e2f3g8e14a2e6g5ef2ef2e2fg7fef4e6a6e2f10gfg2f32gei19a17e"
  "f10e3f9gf3e35a2e14a5e2f7e15a2egfg2f9ef5e2fefe4f2e8a6ef3gfgfgf3g2fe2f19gf9g2fei19a7ef9e"
  "4e3f5e3f9g3e53a6ef6e14a2ef4gfe2fg17efef2e7a8efg3e2fef2gfgfg2f16gf7gfef4ei19a3e5f8e"
  "5e3f4e3f7gf3e37a3e14a14e14a3egf2gefg12e3aeb6e2ae2a11egefefefe2gfgf19g2f5g2f6ei20a3e4f8e"
  "7e2f5e9f3e37aef2e18a13e11a3eg2fg3f9e2ae8a2e2a7efe2fefg3e2fef4gf21gf5g3f6ei20a3e2f9e"
  "15e6f4e36a6e19a13e10a3e5gef8e11ae3a8e5g7fef3g3f25g2f2gf2e2fei20a3ef10e"
  "25e16ae20a6e21a3e2f7eb7a3ef5gefg7e12a11e2f5g4fg3f3g2fg2f21g2ef3gf2gfei19a14e"
  "12e4f9e12ae24a6e22a3ef9e7a2ef3gfegef2e2a3e10a14ef6g7f2g3f22gf4efgfef3ei19a2ef11e"
  "12e3fgf2e2f5e11a4e22a3e2fe22a2e3f3e3a3e6aefef3g4eg2e14a12e2f9g4f3g4f19gf14ei19a2ef10e"
  "10e5f2g4f5e5ae4a4ef22a3ef2e23a2e2f3e5a2e5a3e2f2gf4efe14a10ef15gf3g5f17gf15ei18a4ef9e"
  "15e2gf9e4ae7a2f23a6e24a3ef2e6ae7a3e3g7e16a8e2f17g5f13gefg2f15ei18a4ef8e"
  "14efg2f6ef3e36a6e26aefe15a2e2f2gf7e12a12efgf26gh2gf3gf2e2f15ei20a11e"
  "14e5g9e37a6e26a3e15a3e2f2gf6e13a2ea11ef27g2fefegefgf15ei26a4e"
  "10efg2e5g6eb39a5e28a4e14a3e2f2g5e18a9e3f13gf11g4f2efe2f16ei27a3e"
  "10ef2gfe2f2gf6e40a5e28ae2g2e14a4eg2f5eae14a9ef2e2f13g4f6g3f4eg19ei28ae"
  "10e2f2g2f3g6e41a3e30aefg2e16a3efg6ea2e9aea7e3fe5f21g2f6ef19ei28ab"
  "a8e2fef5gf6e76a3e17a3ef3g4e2a2e8aea5e2fefefe2fgf21gf5e2a20ei28a"
  "5ef7e5gf2e80a2e18a5e3f8e11a4e8f5gf15g2f6ea21ei28a"
  "a20e80a2ef20a5ea5e16a2fe4f7g3f14gfe7a21ei27a"
  "2a4e10a4e81aef22ae6a2f4e11aeae2fe3f7g2fe4f6gef2ge8a20edi27a"
  "4ae96aef22abaef2geafa4e13a2fefegf7gf7e5gf4e9a19edi26a"
  "101a2e22a3e3fe2f3a2e13a2e5f6g2fe5a2e5gf2e10a18e2di26a"
  "3a3e82a8efg2e27a3egf2ef8ae10a2f6e4gf2e8af2g3e11a17e3di25a"
  "2a4e82a10ef29a3ef2eae18af3ab2a3e2g3e9a3e13a9e2fgfde5di25a"
  "3a5e82a10e30a3efe11ae17ae2gf2e26a7e4f2e6di24a"
  "2a4efe32a2e3a9e38a6ef31a6e21ae6aeg2e28a6e2f5e6di24a"
  "3a4ef2e24a22e2a10e4a7e15a7e30aeaea2e20a2e28ae9a14e5di23a"
  "2a7ef2e20a13ef12ef20e3ae14a4e31aeae22ae27a2e11a4ef11e2di23a"
  "3ae2f4e4g5e8a13e5fe5f2e4fgf25e16ae36ae47a2e12aege3fe2f6e3di22a"
  "a3e3f4ef3g6e6a13e2f2gfefg4fdg3f3g2f2ef18e101a4e11a6gfg2f8di22a"
  "12ef3gf23e4f4ef4e2d3g12f4e3d5e102ae2fe11ae5g5f8di21a"
  "14e2f22e6f9e3d4g10f2e6d4e103a2e12ae4g2f2e10di17a4e"
  "25e2f3g2f2e5f4g2f4e2f3e6d3g7f3e6d4e117ae4g4e10di13a7e"
  "6e4f2efgf6e12gf3gf5g2fe7fe6d5g6f2e7d3e16ae44a2e2a5e47ae3g5e10di11a9e"
  "2efefgf5gf3gf2e3g2f20g10fe7d10gfg6d6e59a10e46ae2g6e10di9a10e"
  "e2fef12ge6gf19g5f6g7d12g7d5e64a3e48ae2g5e11di8a11e"
  "3e3f11gef19g2f15g10d10g7d5e115a2ef3e2f11di8a10e"
  "5e2f9g2f16gf2gf17g14d2gd2g8d5e115a6e2gf10di6a12e"
  "6ef9g2f34gd2g28d5e114a7ef2g10di5a12e"
  "5e11gf33g14d2b17d5e114a7ef2g10di4a13e"
  "3e2f43g17d4b15d4e115a4ef3ef11di3a13e"
  "ef45g18d3bdb13d4e116a8e12di3a2ef10e"
  "7g3h36g20d3bdb3d2b8de119a3ebef3e11di3aef2g2f4e2f"
  "5g7h31g41de119a6ef2e11di2a2ef10g"
  "4g2h33g46d2e118a7efe11di2ae8g2hg"
  "16gd3g4d12g49d2e2a2e114a7e2f11di2a3ef8g"
  "15g4d2g6d8g52d3e116a4eb2e2f11di2a7ef3g"
  "15g3d4g6d3g56d5e113ad4ebe2f12di2a5ef5g"
  "12g7d3g67d4e113a2d3ebe2f12dia4ef6g"
  "10g10d2g68d3e2b110a3d5egf12di3ef8g"
  "4gd3g2dg11d2g68d6eb45a7e53a6d4e2g12dief9g"
  "5d3g86d6e3a3e35a11e40a2e7a10d3e2g12dif10g"
  "95d15e31a12e39a5e2ae12d2ef2g6dg5difd3g5d"
  "97d14e29a8e2d4e2a2e32a9ea12d2ef2g5d2g5di10d"
  "101d14eb24a6e7d11e24a13e13def2g5d3g4di3de5d"
  "102d3e3d8e23a4e11d10e5ab17a3e3d6e3db11d2efg6d2g4di9d"
  "112d5e23a3e12d28e4a3e6d2e17dedg7d3g3di8d"
  "113d4e23a2e14d2e3de2d26e6d2e5db13d2g6d2g4di8d"
  "116d3e23a2e22d24e8de19d2g12di7d"
  "117d2e23a2e23de4ded15e10de4dea10dad3g11di7d"
  "119d4e20a2e31d6e4b12d3e5da10dad4g10di6d"
  "119d10e14ae34de5b14d3e6da4d2g3dad3g11di6d"
  "121d13e10ae34d3b18de7da5dg3dad4g10di5d"
  "121d14e9ae33d4b18de8da3d3g2d2af3g6d3gdi5d"
  "125d2e2d9e6a2e32d3b19de9dba3d2gfd2af2d3gd6gdi4d"
  "131d8e4a2e38db16de10d2a2d3gd2a4d2g2d5gdi4d"
  "136d10e55de11d2a6d3a7d4gdi3d"
  "137d8e57de10d3a4d6a6d3gdi3d"
  "180d3b21de10d3ad8a10di2d"
  "181d2b22de11d11a9di2d"
  "206de12d11a8did"
  "207de11d13a6did"
  "208de12d13a5di"
  "208de10dfed13a4di"
  "i207de11dgfd15a2d"
  "i220d3e14a2d"
))

(town 230 167 0 (n "Aberdeen"))
(town 61 76 0 (n "Ajaccio"))
(town 196 58 0 (n "Aleppo"))
(town 27 63 0 (n "Alicante"))
(town 19 56 0 (n "Almeria"))
(town 210 31 0 (n "Amman"))
(town 21 136 0 (n "Amsterdam"))
(town 76 85 0 (n "Ancona"))
(town 22 131 0 (n "Antwerp"))
(town 135 55 0 (n "Athens"))
(town 96 71 0 (n "Bari"))
(town 42 108 0 (n "Basel"))
(town 61 80 0 (n "Bastia"))
(town 14 91 0 (n "Bayonne"))
(town 200 42 0 (n "Beirut"))
(town 220 155 0 (n "Belfast"))
(town 100 92 0 (n "Belgrade"))
(town 140 21 0 (n "Benghazi"))
(town 7 181 0 (n "Bergen"))
(town 43 104 0 (n "Berne"))
(town 9 91 0 (n "Bilbao"))
(town 65 90 0 (n "Bologna"))
(town 34 126 0 (n "Bonn"))
(town 16 97 0 (n "Bordeaux"))
(town 116 147 0 (n "Bransk"))
(town 78 111 0 (n "Bratislavia"))
(town 33 140 0 (n "Bremen"))
(town 234 121 0 (n "Brest"))
(town 236 135 0 (n "Bristol"))
(town 72 117 0 (n "Brno"))
(town 24 127 0 (n "Brussels"))
(town 149 72 0 (n "Bursa"))
(town 0 58 0 (n "Cadiz"))
(town 68 62 0 (n "Cagliari"))
(town 13 129 0 (n "Calais"))
(town 233 137 0 (n "Cardiff"))
(town 100 49 0 (n "Catania"))
(town 4 124 0 (n "Cherbourg"))
(town 32 126 0 (n "Cologne"))
(town 63 47 0 (n "Constantine"))
(town 40 153 0 (n "Copenhagen"))
(town 8 63 0 (n "Cordoba"))
(town 213 143 0 (n "Cork"))
(town 33 108 0 (n "Dijon"))
(town 57 127 0 (n "Dresden"))
(town 221 149 0 (n "Dublin"))
(town 228 163 0 (n "Dundee"))
(town 30 130 0 (n "Dusseldorf"))
(town 228 160 0 (n "Edinburgh"))
(town 32 130 0 (n "Essen"))
(town 67 85 0 (n "Florence"))
(town 40 123 0 (n "Frankfurt"))
(town 63 146 0 (n "Gdansk"))
(town 40 102 0 (n "Geneva"))
(town 54 91 0 (n "Genoa"))
(town 4 56 0 (n "Gibraltar"))
(town 225 160 0 (n "Glasgow"))
(town 33 164 0 (n "Goteborg"))
(town 203 36 0 (n "Haifa"))
(town 39 135 0 (n "Hannover"))
(town 40 156 0 (n "Helsingborg"))
(town 62 180 0 (n "Helsinki"))
(town 58 106 0 (n "Innsbruck"))
(town 151 42 0 (n "Iraklion"))
(town 115 174 0 (n "Ivanovo"))
(town 148 60 0 (n "Izmir"))
(town 208 30 0 (n "Jerusalem"))
(town 69 147 0 (n "Kaliningrad"))
(town 78 123 0 (n "Katowice"))
(town 79 149 0 (n "Kaunas"))
(town 36 146 0 (n "Kiel"))
(town 122 109 0 (n "Kisinov"))
(town 80 122 0 (n "Krakow"))
(town 164 108 0 (n "Krasnodar"))
(town 223 98 0 (n "La Coruna"))
(town 10 106 0 (n "La Rochelle"))
(town 42 103 0 (n "Lausanne"))
(town 10 123 0 (n "Le Havre"))
(town 237 149 0 (n "Leeds"))
(town 51 128 0 (n "Leipzig"))
(town 28 126 0 (n "Liege"))
(town 20 127 0 (n "Lille"))
(town 134 148 0 (n "Lipeck"))
(town 224 73 0 (n "Lisbon"))
(town 231 148 0 (n "Liverpool"))
(town 75 130 0 (n "Lodz"))
(town 87 129 0 (n "Lublin"))
(town 33 120 0 (n "Luxembourg"))
(town 98 121 0 (n "Lvov"))
(town 35 99 0 (n "Lyon"))
(town 47 133 0 (n "Magdeburg"))
(town 10 57 0 (n "Malaga"))
(town 235 146 0 (n "Manchester"))
(town 42 119 0 (n "Mannheim"))
(town 237 33 0 (n "Marrakech"))
(town 42 86 0 (n "Marseille"))
(town 99 53 0 (n "Messina"))
(town 7 112 0 (n "Nantes"))
(town 236 153 0 (n "Newcastle"))
(town 48 86 0 (n "Nice"))
(town 185 48 0 (n "Nicosia"))
(town 0 142 0 (n "Nottingham"))
(town 52 118 0 (n "Nurnberg"))
(town 35 151 0 (n "Odense"))
(town 31 47 0 (n "Oran"))
(town 24 176 0 (n "Oslo"))
(town 91 53 0 (n "Palermo"))
(town 37 66 0 (n "Palma"))
(town 122 95 0 (n "Ploesti"))
(town 124 80 0 (n "Plovdiv"))
(town 232 131 0 (n "Plymouth"))
(town 199 23 0 (n "Port*Said"))
(town 224 87 0 (n "Porto"))
(town 63 134 0 (n "Poznan"))
(town 1 46 0 (n "Rabat"))
(town 25 120 0 (n "Reims"))
(town 72 161 0 (n "Riga"))
(town 22 133 0 (n "Rotterdam"))
(town 95 86 0 (n "Sarajevo"))
(town 62 70 0 (n "Sassari"))
(town 2 62 0 (n "Seville"))
(town 239 145 0 (n "Sheffield"))
(town 113 76 0 (n "Skopje"))
(town 47 174 0 (n "Stockholm"))
(town 40 114 0 (n "Strasbourg"))
(town 46 114 0 (n "Stuttgart"))
(town 204 16 0 (n "Suez"))
(town 65 175 0 (n "Tallinn"))
(town 3 53 0 (n "Tangier"))
(town 100 68 0 (n "Taranto"))
(town 204 32 0 (n "Tel Aviv"))
(town 20 135 0 (n "The Hague"))
(town 123 69 0 (n "Thessaloniki"))
(town 108 73 0 (n "Tirane"))
(town 45 85 0 (n "Toulon"))
(town 25 89 0 (n "Toulouse"))
(town 15 111 0 (n "Tours"))
(town 71 96 0 (n "Trieste"))
(town 103 24 0 (n "Tripoli"))
(town 15 197 0 (n "Trondheim"))
(town 121 155 0 (n "Tula"))
(town 80 47 0 (n "Tunis"))
(town 24 68 0 (n "Valencia"))
(town 67 95 0 (n "Venice"))
(town 222 92 0 (n "Vigo"))
(town 165 133 0 (n "Volgograd"))
(town 137 142 0 (n "Voronez"))
(town 38 123 0 (n "Wiesbaden"))
(town 68 128 0 (n "Wroclaw"))
(town 79 96 0 (n "Zagreb"))
(town 20 80 0 (n "Zaragoza"))
(town 47 107 0 (n "Zurich"))
(city 190 22 0 (n "Alexandria"))
(city 46 50 0 (n "Algiers"))
(city 164 72 0 (n "Ankara"))
(city 33 77 0 (n "Barcelona"))
(city 52 135 0 (n "Berlin"))
(city 239 140 0 (n "Birmingham"))
(city 123 93 0 (n "Bucharest"))
(city 87 107 0 (n "Budapest"))
(city 198 17 0 (n "Cairo"))
(city 237 44 0 (n "Casablanca"))
(city 205 41 0 (n "Damascus"))
(city 137 121 0 (n "Dnepropetrovsk"))
(city 147 123 0 (n "Donetsk"))
(city 128 173 0 (n "Gorki"))
(city 37 141 0 (n "Hamburg"))
(city 144 76 0 (n "Istanbul"))
(city 134 132 0 (n "Kharkov"))
(city 115 128 0 (n "Kiev"))
(city 77 181 0 (n "Leningrad"))
(city 6 134 0 (n "London"))
(city 9 76 0 (n "Madrid"))
(city 54 96 0 (n "Milan"))
(city 94 145 0 (n "Minsk"))
(city 114 162 0 (n "Moscow"))
(city 56 111 0 (n "Munich"))
(city 86 70 0 (n "Naples"))
(city 131 107 0 (n "Odessa"))
(city 20 117 0 (n "Paris"))
(city 61 122 0 (n "Prague"))
(city 77 75 0 (n "Rome"))
(city 119 81 0 (n "Sofia"))
(city 48 95 0 (n "Torino"))
(city 74 111 0 (n "Vienna"))
(city 79 134 0 (n "Warsaw"))
