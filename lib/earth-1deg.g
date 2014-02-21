(game-module "earth-1deg"
  (title "Earth 360x140")
  (blurb "Earth's terrain, at 1deg/hex")
  (default-base-module "standard")
  (variants
   (see-all false)
   ;; The following is useful for hacking on the terrain
   (bare false
    (true
     (set synthesis-methods nil)
     (add u* start-with 0)
     (add city start-with 1)
     (add u* independent-near-start 0)
     (table independent-density (u* t* 0))
     ))
   )
  )

;  The feature area layer is just used to position feature names on this map.
;  Borders will therefore appear strange if drawn.
(set feature-boundaries false)

(world 360)

(area 360 140 (latitude 4) (longitude 32))

(area (terrain
  "360h"
  "9a4e48a29h80a4f19a5fa40f35a2hah2a2ha2haha2hahaha2haha3ha17haha2h2a5ha5h2a4ha8ha2h3a"
  "6e5a2e3ae4a20g20ag27hg78a4f14af3a9fa42f5a13f18ahaha2ha6h2a3hahah4ah2a2hahaha3h4a3haha2h2ahah2a3ha2haha2hah2ahahah2a2e"
  "10e5a3e7a8e3g4h4g18ag25h2g46ag10f20a3f16a2f2a53f2a26f16aha4haha2hah2ahaha2hahah6a3f3a2h9a2ha3ha2h2a2ha2haha3h2ae"
  "10ea2ea5e5ae8a2e3g6h2g16ag21h2ghg46a3g15f30a5fcfa2fc93f25a13f3a3f2g3f23a2e"
  "14a7e2a5e10a6g2h2g2h3g10a18h3g49a2g18f22a14f3a2f2c5f2c88f19a6f3gf3g6fg2fgf2g8f10e6a"
  "7e7a14e12a8g5h2g9ag12hgh3g50a3g12f21a19f3acfac5fc97f13a5f3gfg2f6gf3gfg3f2g12f9e"
  "27e17a4eg2h6g8ag12h54a2g15f6a5f3a16fg9fc2fcacf3c4f2c97f9a17f2g3fgf3g22f"
  "16f9e17a7e4g3a2g8ag10hg19a3h2g29a4g4f3b8fafa25fg6f4cfcfc2f2cf2c106f9a16f2g2fg2f2g20f"
  "19f5e15a10egh4g12a2g8hg19ahg3hg26a6g3fba2b3fb34f2c2fc2f2cfcf3c2f2cfc106f4a22f2g2fgf2g19f"
  "24f22a3e2g2h6g11a2g5hg20a2e3g24a8g2f2bab7fb29fg2fc2f6cfcfcf2c2fc2fc92f5a6f5a20f6g3fgfg17f"
  "24f16a4g5a2e5g15a2g3hg33ag15a9gf3bab4fbfb31fg2fc5fcf4cfc2fc2fc3fc76f4a7f11a2f10a2fgfg4f3g3fgfghgh4gfg2fg14f"
  "25f16a2f4g7a4g15aghg50a8g3f2b2ab40f2cfcfc2fcf6cfcfc2f2cfc68f2a5f4a3f24a5f7g3fg4f2ghghgh7g12f"
  "25f17a5f2g26ag50a3g10faeb4e2b6e26fg4f2cf15c2fcfc66f5a4f26af4a6f4g7fgfgfg2fg2fghgf2gfg11f"
  "10f2a13f18a6f2g4ag70a3gfe6f2e2a9bebe27fg3fc3f6c3f5cf6c66f5a3f32a6fg4fbae2g9agf3hgh3gfg2f2g4f"
  "15fafa3fa4f17a7f3g2a3g58a2e9ae2f2bfb3feb2a2ba15eb27f2cf10cf8c3f4e46f16a5f35a8fgf16agafhgh2g2fg3fgf"
  "g18fa3fa4f16a12f3g56a2e2g12aebe3febea2bfb3eb2f16e14fg6fcf12cf5c2f5e44f18a7f39a2g20ag2aghgf2gfgfg2f"
  "fg32f8af2a3fa9f3g56a2g12a2e2b3eb2a2b6e2f37ececf3cf7c3fe2f2ef4e15f2g23fe21a7f37a3f3agf19afhg2f2gfg2f"
  "g3f6e9fa6fa2fa6f8a6fa11f52a2ea4e10a4e8b28eb9efg7ec10e2c11e5fefe8fgae23fe2af17a7f35a3gf27agh2g5fg"
  "f3g2f10e25f3a11fa2fa6f49a3e2a4e9ae6b3eb11e2f15eb10egf22e2fe4f5ef4e8fae26f19a6f4agag27age32ag2hgfgef"
  "3f3gf13e2fa2f2a2fa11f3a9fa3fa5fa3f47a3e3a3e6a17ef24eb35e2f2e8f2g2f3e4f2a32fbf12a5f31agag33af2a3g3fe"
  "3e3f2gf13efaf2a16f2a17fa5f46a3e2a5e4a10e2f8e2c32e2f23e15fg2f4e3a2e3f2e2fe2fef2e17fbf12a3f15ag11agagag38af4afhf"
  "3f3ef2g13e2fa2fa16fa22f48ae4a5e2a8ef13ec2f30ef25e14f3g3f2a5ef4e5fefef3e13fbf13af22agagag48af2ag"
  "a2g2f2efgfg13e2fafa22fa7fe4fef2af50a3e5a4ef18ef58e5fg5fgfg2f2g3f16e3f5e7fe3fba2f11af76a2fa"
  "ea2eg3egfg18efa5f3a14f4e10ae2fe52aea4ef2ef12e4gf62e3g2e3f4g8f13efgf7e5fe4fba2f89a2f"
  "4aeg3e2f2g20efe5ae12f3e2ae2feaf3a2e2fe50a17e2g4eg4eg18eb18ec25e2g3e5f2g3ef15efgf8e2f4e4f2af91a"
  "2afbeg3e3fg19ef2e2af4a2e8f2e3ae2fe6a6e50a11e6ge2g7e2g16eb10ec33e2g30efg14e4f3af9af81a"
  "3afegf4egf2g21e5f3a2e5f2e2a2e4f2e3ae58a8e6ge3g9e2g5eae4b12e3bc4e2a14e4ad7e4g8e2g2d16e2f13e5f4af90a"
  "3a2eg6e2gf20e2f3e2a2e3b5ea3e4feae2fe60a2e2f2e2g6eaeg6e5g2e4a3e2b10e4ab2c3e3a2ed3e3d4ea4d2g3e5g7e2g5d6ed8e2f9e2fe5f13af82a"
  "4afeg2ef3eg2fg3eg19e2a2e2a6ea3ef3e2a2f63a3ef2eg6e2ab3g3eg7e4aeab11e5a2c4e2a6d2e9d5g2e11gde9de2d8ef9e3f2e4f5a2f4a2f83a"
  "4afe2f7e2gfg21e2a3ea4e3aef3e4ae57a6e2f2ef4e3a4e2ab7g5e10a5g4e4a6e2c8de2g3d5e6gh4g20d6e2f3ef5e3f3e2f6ae4f86a"
  "5a3f2e3degde2gfe2g18e2a5e2a8e62a2e2g4e3ge5ae2aeg2e2ab5ge3ge12a2e5ge4ab2e2dec8d2ef4g2e2a2e2g4h2ge19d5e3f3e2f5e3f2e10a2ef87a"
  "5a2feg8db2f2eg19ea3e3a10e62ag10e6ae2a5e2ab7g2e3a4e5ageg3e2g4abe3d2e7d3e2f7ge2g3h2ge19d5e2f5ef5e4fe101a"
  "6af2eg4de3de2gf3g32e64a2e3g5e7a2e4a4eae2g2e2bea3ege2g4e7g4e3a3d2e9d7e3g3e7ge17d15e2a4e2fe11aef88a"
  "6af3eg2dgdg3dg2eghg25ef5e65a9e3ae5ae6ae3af2g5a8e2ge5ge3g2e4a12d2e9ge26d15e2a2e2a2ef12a2f88a"
  "8a3eg2dg2dgdegde3g23efgf2ebe47ae17a5e3g2e15ae4abege2a4e2g2deg2e4gb2gb3g5a5e6d4e3g2h3ge12d2g3d3g5d15e7a2ef11afe88a"
  "8a3eg2dg2dg2dgdegh2g13eb7efgf3eae49ae17a7e14a3e5a2g2aea5e3g4e11g4a6e8d3e2g2h5g6d7gd9g16ebe5ae2f9aefe88a"
  "10a3eg2dg2dg3de3g13eb7efgf4e70ae8a8e12ag5aeaea2e2a2eded2e2d3ge4g2e4d4e4d6g3h15gh9g22e3aefe7ae3fe87a"
  "11a3egdg2dg4d17eb5e2fg6e76a4e5g2e3ad9a2eae8a2e2de2d3e3g5e7d3e4d3g2hg4h17gh7g18eb4a3e2a5ef4e87a"
  "12a4e2dg5dgdeg8efgf2eb5efe2f5e70a2eg5e5g4e22a2ea2e3de2d3e4g2e9d3e5ge8ghgh7gh14g2f15e2b4ae8a2e90a"
  "15ae4de3dgdgd10ef2eb12e71a4e7g2e5de25ae4d2ed4e2g3e9de7g2ef3g4hgh9gh12g3fg13eb8a2ea2e92a"
  "14ae2a2e3dedg2dg2d9e2fb12e15ae47ae7a4e4g11d5e5a3e12a2e6d5e3g2e9d2eg2eged4ef5g4hgh4gh2gh9g4f14eb8a2e94a"
  "18adb3de3de2d9e2feb10e71a3e4g17de5a5e5a2e3a2e8d4e3g2e8de2deded7e5ghg3hg2h15gf15ebe8ae95a"
  "18a2dab4dg5d11eb10e71a3e2g19d7e9d3e13d4e3g2e9ded4ede2d3ef6g8h4gh4gfgf9ea7eb44ad58a"
  "19a2dba4dg4d8e3aeba6bef63ae7a2eg36d5eg12de2ae4ge9d3e6d6ef5ghg2hg3h8g2f12ea5e103a"
  "16ad4a2dbae3de4d6e12ab2e64aeae3a41de2daga12d3a4ge11d2e6d7ef17gfg2f18e7ae52ad41a"
  "21a3dba2dg4d6e15a2e67ae43de2d2a13d4a3e11d2e6d9e2f15g2f2ef15e103a"
  "24a2dbe2dg3d6e14aefe66ae44de2d2ae13d7a9d2e6d13e3f2gfef5g2f2ef15e7ae95a"
  "25ad2b3dg3d5e15afeae63a47de2d2a13dad3ae3a5d5e3d20ef2g2f4gf3efef12e73ad29a"
  "26a2dabe2dg2d4eb18abe61a48de2d2a8de6d2a3d10a4d20ef2eg2f3g4f15ea2e2ae16ae80a"
  "27a2dabdeg2dg3e21abe59a48de2d3ae17dg2e9ad24efgfe2fg5fef10e3aef100a"
  "33aed2g4e14a5e63a31dg15de3d3a7dg10d2gde9a20e2cafgf2e2fg5f2ef5eae5ae79af20a"
  "34aegeg4e17a6e2b56a32d3g11de5d3ae5dg12dgd10a2ea15e4aegf3e5f5e2be91ae18a"
  "35afege2g2e6af3e10a3eab56a35d2g9de5d4ae3dg14d14af13e5a2f3e7f3e96af16a"
  "29ad5afe3geg2e5a4f14a3e54a47d2e4d4a18d14aef11e7aef3e7feb2afe94ae14a"
  "37afef2eg3e3a4f10a2e3a5ea2ebe47a48de5d3a16d16a11e10af3e7feb2a2f10ae82a2e13a"
  "40a6f2e5f24a2b46a48de3dede3ae13d17aef8e11af4e7fe13a2ef96a"
  "43a9fe27ae36ae7a2e44de2d2ed4e3a13d18aef7e12aeca2e7f2e12aef96a"
  "49aefg6f22a2b43a4ede4de2c28d3eded10e3ae3d3e2d22aef4e18a2fefe5fe11a2e22ae73a"
  "51a2e2ge4f21aeb36ae5a6ed2edec2e3d2e16ded2ed3ed12eg2e2aeg4e25aef4e18af4e5fe11a2eb94a"
  "53a3e4f22abe42a24ed2eded2b2ed2ed16e3g2ea3e29aef3e12ab5af5e4fe13aef20ae72a"
  "57a2efe13abe6ae44a4ef29e2d18e4ge33a5e13af4afeaefe4fe12aefaf91a"
  "58aefe10aeaeb9ae43a53e3g2ed6a2d3ae20a5e13ab4afe3afe3fe16af90a"
  "59afef7a2egeb5e2aeae46aefef43e2c3e3g2e2d2a5d20ae3a4e14af4af5a2f2e11afbfae91a"
  "61ae3a2e2a2egebg9e47ae2fefef2ef2ef2ef29e3c2e5g3e7d25a3e2af11ab4af6ae12a2fab3af12ae76a"
  "62af2ef5efgeg11e47a4f2efef2efefef3e2f23e3ce3ge2g4e5d27ae2a2f12af3aef17afb3a4f88a"
  "65ae2af2efg9e5f47a4fef4e2fefe5fef3ef2e2f13e3c2e5g5e4d30aef17afe21a4fe7ae17aeba2b57a"
  "70afef2g9eg4f47a10f4e19f9ecec2e4g6e3d31aef18afe15af4abaef26ab5aeb53a"
  "70afgfgf9efg3fefefe43a5fe4f5a19fef3ef8e2g6e3d48af2ae2f12ae3f38ab55a"
  "71afg2e3f7egfg4fb3f44af12a2fef2efe13f2e3f5ed6e5d48a3fa3f11ae4f93a"
  "71ae2g5fe2f5e10fb60a22f3edbe2d3e4d49abfefae2f9a2fg2f94a"
  "71af2ge7f6e10fb60a8f4b7fg2f4eb2ed2e3de51ab2fbaef8a3fgfe82a2b9a"
  "70a2fg11f2e12fe60a8fb3f3b4fgb3eg5ede3d53ab3fae5afe3fg4f5af36a2b10ae28adb7a"
  "70a2fge25feb59a7fb12f11e2d54afa3f7a4f2g2fe2a5f34a2be2b37a2b7a"
  "58aeae9afeg14fe6fe5febf53ae3a7fb11fgec2b3eg3ed58afgfe5a4fg4f2af5af5a2faf24a2b25a3b20a"
  "70aefge20f3e3f2befef54a7fb12f2e3b7e59a2fe2f5a6fe2a5f3af3a3f54ae20a"
  "70abe2g18fe3b2f3b5fef48ae4a6fb11fg3eb7e61a2fg2faf2a4fefb2a3f10a2f2a4fe6ae61a"
  "71ae2ge3fe12fe2bef3b9fe3f51a4fb11fg9eg2e62a3fe5a3bfeb2afaf6a2f3a3f4g2fe6af58a"
  "71ae2g16fe21f2e5ae44a3fe12fbf10e63a3f13afa2f13a3f2g4f6af56a"
  "72ad2ge13f2e13fe3f3e3f4e47afe14fb10e16ae48afe13af3af12a2b2c3g3f2b3f4af51a"
  "73ad2g13fe13f2e3f2e4f4e48ab14f2b3ef5eae66aea2bfe23a3b3c4gfe7afbf49a"
  "74ad2ge17fe7f4e3fe4f4e48ae14febef2ef4e68a3e2f2e20af2ab4c4fgf9ab48a"
  "75a2dg17fe5f6e2f2e3f5e19ae28a3e12f2ebefe3f3e73a2faeae3a2ea2e4af2b4a2b2cf3b3gf7af2b46a"
  "16ad59a2d2ge14fe2f2e2f6e2f2e3f4e49a2e12fb7efefef80ae3ae14a3b4abegf10afb43a"
  "77ad2g15f2e2f2ef6ef3e2f5e50a2e12fe4feb4e2f101a2b6af11af42a"
  "78a2d2ge12f4ef2e2f4ef3e2f5e51ae12fefc3feb2e4f56ae32afaf8abf2b18ab40a"
  "79aed3g6f3e3f4ef2e2f7e2f4e52a2e3fe2fe9f2ebe5f3ae4ae81ae5fb3ab2f2b58a"
  "81ad4g2e3f5e3f13e2f4e52a3e2f4efe6fef2ebe5f7a2e80a3e3fb4ab2f3b40aeae13a"
  "82ad6ge2f5e4f11e2f2efe52a4ef3e4f5e3f2e6f7a2e76a2faef3efeb2a3bfef3b55a"
  "27af55a2d3ga2g3f19ef2e2fe33ae18a7e4fe3fe2f4e5f6a3ef75a4f6ef6b2ef3b18af10af24a"
  "85ade2gege3f18ef2e2fe52ad7e3f2efe6f2e4f5afegef73ae3f11e4b2ef3b19af34a"
  "9af78ad5g19ef3efe53ad15ef2e5f7afegef73a10e2d5e2b3e2f3b19ae7af24a"
  "89adgh2g23efe53a2d18e2fc8a4ef72ae4d3e6d11e3b52a"
  "90adg2hg25e53a2d6e2c9e3f9a2egef70a2e15d2ed2e2d4e3b19ae30a"
  "90adg2hg25e54a2d3ed12e3f9afg2ef8ae60a2ede16de2de3d4e3b13ae34a"
  "ae89ad2ghg20eg3e55a2de9d4e5f8a2fgef7ae59a5ede21de2d3e3b13ae33a"
  "91adgh2g19eg3e56a3de9d4e5f8afgef67a2e3de16dede4d2ed3e3b46a"
  "91aedghg21e59a2d2e7d6e4f9af2e67a3de23de2deded3e3b44a"
  "91a2dghg19e62a2d2e6d6e4f10a2e67ae4de14de11d3ed2e2b44a"
  "92adg2hg18e63a2de7d6e2f12ae68ade5de14de6de2ded4ea2b42a"
  "92adgh2g13e2f2e64a2d2e6d6e2f81a4de21de7ded3eb43a"
  "54ae38adghg13e2f3e64a2de6d7ef82ade26de3ded5e43a"
  "93ae3g18e65a2de3d6e2g2e82ae2de2de17de8d4ef2e42a"
  "94ae2g18e67ade3d7ege84a2e25de3de2d2egf14ae27a"
  "94a2g18e68ade3d6eg2e84af2ede2de16de8d2efge42a"
  "94ae2g17e70a12e85af2e20d2e4de3defgfe41a"
  "94a2eg16e71a11e87a3e6d9a3d3e3deded2ef3e41a"
  "87ae7a2eg11ea3e72a10e87af4e4d11a2ea2e2ded4ef2e42a"
  "95ae2g12e76a2e95af2e16aea4ed5efefe42a"
  "95a2eg13e196aed5efg2e23ae18a"
  "95aefg13e197aed4e2gfe24ae17a"
  "95aef2gf11e198a9e25a2e15a"
  "95aegfgf7e205aea2e27a2efe13a"
  "96aefg2f7e235a3ef13a"
  "96af2gf6e239afe13a"
  "97aefgf3e2de208aef2e24a2eae13a"
  "97agag2fe2d2e209afg2e22a2g2e14a"
  "99a2gf3ede210aefe21a2g2e15a"
  "99a2gf2ed2e211a2e20a2g2e16a"
  "99aghgfe2d234a2g2e16a"
  "98afbgf4e235a2e17a"
  "99ab2gf4e107ae126af18a"
  "99a3gf4e253a"
  "99afbgf4e253a"
  "100a2g2f2e139ae114a"
  "100afb2g2fe8aeae242a"
  "102a2g2fe253a"
  "104abg2bg251a"
  "104af2b2g2f249a"
  "106ag3f30ae219a"
  "360a"
  "360a"
  "360a"
  "360a"
  "360a"
  "360a"
  "119a6hah233a"
  "116a6h2g2h234a"
  "109aha10h2g3ha4h229a"
  "360h"
))

(area (features (
   ( 1 "ocean" "Arctic %T")
   ( 2 "ocean" "Atlantic %T")
   ( 3 "ocean" "Indian %T")
   ( 4 "ocean" "Pacific %T")
   ( 5 "continent" "Europe")
   ( 6 "continent" "Asia")
   ( 7 "continent" "North America")
   ( 8 "continent" "South America")
   ( 9 "continent" "Africa")
   ( 10 "continent" "Australia")
   ( 11 "continent" "Antarctica")
   ( 12 "sea" "South China %T")
   ( 13 "sea" "Caribbean %T")
   ( 14 "sea" "Mediterranean %T")
   ( 15 "sea" "Bering %T")
   ( 16 "gulf" "%T of Mexico")
   ( 17 "sea" "%T of Okhotsk")
   ( 18 "sea" "%T of Japan")
   ( 19 "bay" "Hudson %T")
   ( 20 "sea" "East China %T")
   ( 21 "sea" "Andaman %T")
   ( 22 "sea" "Black %T")
   ( 23 "sea" "Red %T")
   ( 24 "sea" "North %T")
   ( 25 "sea" "Baltic %T")
   ( 26 "sea" "Yellow %T")
   ( 27 "gulf" "Persian %T")
   ( 28 "gulf" "%T of California")
   ( 29 "sea" "Caspian %T")
   ( 30 "lake" "%T Superior")
   ( 31 "lake" "%T Victoria")
   ( 32 "sea" "Aral %T")
   ( 33 "lake" "%T Huron")
   ( 34 "lake" "%T Michigan")
   ( 35 "lake" "%T Tanganyika")
   ( 36 "lake" "%T Baykal")
   ( 37 "lake" "Great Bear %T")
   ( 38 "lake" "%T Nyasa")
   ( 39 "lake" "Great Slave %T")
   ( 40 "lake" "%T Erie")
   ( 41 "lake" "%T Winnipeg")
   ( 42 "lake" "%T Ontario")
   ( 43 "lake" "%T Balkhash")
   ( 44 "lake" "%T Ladoga")
   ( 45 "lake" "%T Chad")
   ( 46 "lake" "%T Maracaibo")
   ( 47 "lake" "%T Onega")
   ( 48 "lake" "%T Eyre")
   ( 49 "lake" "%T Volta")
   ( 50 "lake" "%T Titicaca")
   ( 51 "lake" "%T Nicaragua")
   ( 52 "lake" "%T Athabasca")
   ( 53 "lake" "Reindeer %T")
   ( 54 "lake" "Issyk Kul %T")
   ( 55 "lake" "Torrens %T")
   ( 56 "lake" "Vanern %T")
   ( 57 "lake" "Nettilling %T")
   ( 58 "island" "Anticosti %T")
   ( 59 "island" "Ascension")
   ( 60 "island" "Azores")
   ( 61 "island" "Bahamas")
   ( 62 "island" "Bermuda")
   ( 63 "island" "Bioko")
   ( 64 "island" "Canary %Ts")
   ( 65 "island" "Cape Breton %T")
   ( 66 "island" "Cape Verde %Ts")
   ( 67 "island" "Faeroe %Ts")
   ( 68 "island" "Falkland %Ts")
   ( 69 "island" "Fernando de Noronha")
   ( 70 "island" "Great Britain")
   ( 71 "island" "Greenland")
   ( 72 "island" "Iceland")
   ( 73 "island" "Ireland")
   ( 74 "island" "Madeira")
   ( 75 "island" "Marajo")
   ( 76 "island" "Newfoundland")
   ( 77 "island" "Prince Edward %T")
   ( 78 "island" "St. Helena")
   ( 79 "island" "South Georgia")
   ( 80 "island" "Tierra del Fuego")
   ( 81 "island" "Tristan da Cunha")
   ( 82 "island" "Antigua")
   ( 83 "island" "Barbados")
   ( 84 "island" "Cuba")
   ( 85 "island" "Curacao")
   ( 86 "island" "Dominica")
   ( 87 "island" "Guadeloupe")
   ( 88 "island" "Hispaniola")
   ( 89 "island" "Jamaica")
   ( 90 "island" "Martinique")
   ( 91 "island" "Puerto Rico")
   ( 92 "island" "Tobago")
   ( 93 "island" "Trinidad")
   ( 94 "island" "Virgin %Ts")
   ( 95 "island" "Andaman")
   ( 96 "island" "Madagascar")
   ( 97 "island" "Mauritius")
   ( 98 "island" "Pemba")
   ( 99 "island" "Reunion")
   ( 100 "island" "Seychelles")
   ( 101 "island" "Sri Lanka")
   ( 102 "island" "Zanzibar")
   ( 103 "island" "Balearic %Ts")
   ( 104 "island" "Corfu")
   ( 105 "island" "Corsica")
   ( 106 "island" "Crete")
   ( 107 "island" "Cyprus")
   ( 108 "island" "Malta")
   ( 109 "island" "Rhodes")
   ( 110 "island" "Sardinia")
   ( 111 "island" "Sicily")
   ( 112 "island" "Aleutian %Ts")
   ( 113 "island" "Bougainville")
   ( 114 "island" "Canton %T")
   ( 115 "island" "Caroline %Ts")
   ( 116 "island" "Choiseul")
   ( 117 "island" "Christmas")
   ( 118 "island" "Clipperton")
   ( 119 "island" "Easter %T")
   ( 120 "island" "Fiji")
   ( 121 "island" "Funafuti")
   ( 122 "island" "Galapagos %Ts")
   ( 123 "island" "Guadalcanal")
   ( 124 "island" "Guam")
   ( 125 "island" "Hawaii")
   ( 126 "island" "Oahu")
   ( 127 "island" "Hokkaido")
   ( 128 "island" "Honshu")
   ( 129 "island" "Iwo Jima")
   ( 130 "island" "Kyushu")
   ( 131 "island" "Okinawa")
   ( 132 "island" "Shokaku")
   ( 133 "island" "Kodiak %T")
   ( 134 "island" "Marquesas %Ts")
   ( 135 "island" "Marshall %Ts")
   ( 136 "island" "Nauru")
   ( 137 "island" "New Caledonia")
   ( 138 "island" "New Guinea")
   ( 139 "island" "New Zealand")
   ( 140 "island" "Leyte")
   ( 141 "island" "Luzon")
   ( 142 "island" "Mindanao")
   ( 143 "island" "Mindoro")
   ( 144 "island" "Negros")
   ( 145 "island" "Palawan")
   ( 146 "island" "Panay")
   ( 147 "island" "Samar")
   ( 148 "island" "Sakhalin %T")
   ( 149 "island" "Samoa")
   ( 150 "island" "Santa Catalina")
   ( 151 "island" "Tahiti")
   ( 152 "island" "Taiwan")
   ( 153 "island" "Tasmania")
   ( 154 "island" "Tonga")
   ( 155 "island" "Vancouver %T")
   ( 156 "island" "Vanuatu")
   ( 157 "island" "Bali")
   ( 158 "island" "Borneo")
   ( 159 "island" "Celebes")
   ( 160 "island" "Java")
   ( 161 "island" "Madura")
   ( 162 "island" "Moluccas")
   ( 163 "island" "New Britain")
   ( 164 "island" "New Ireland")
   ( 165 "island" "Sumatra")
   ( 166 "island" "Timor")
   ( 167 "desert" "Arabian %T")
   ( 168 "desert" "Atacama %T")
   ( 169 "desert" "Chihuahuan %T")
   ( 170 "desert" "Death Valley")
   ( 171 "desert" "Gibson %T")
   ( 172 "feature" "172")
   ( 173 "sea" "Java %T")
   ( 174 "sea" "Celebes %T")
   ( 175 "sea" "Arafura %T")
   ( 176 "island" "Baffin %T")
   ( 177 "desert" "Gobi %T")
  )
  "360a"
  "9b4h48b29*71,52c51b46g121b"
  "6h5b2h3bh4b20*176,20b29*71,53c46b53g5b13g98b2h"
  "10h5b3h5b2t19*176,18b28*71,46c11f39b57g2b26g47b3h38bh"
  "10hb2hb5h5bh8t13*176,16b26*71,46c18f30b104g25b13h3b8h23b2h"
  "14b7h2b5h10t15*176,10b21*71,49c20f22b5f111g19b44h6b"
  "7h7b14h12t15*176,9b18*71,50c15f21b12f117g13b50h"
  "27h17t13*176,8b13*71,54c17f6b5f3b17f122g9b49h"
  "25h17t11*176,3c2*176,8c12*71,19c5*72,29c8f3z8fbfb26f127g9b45h"
  "24h15t16*176,12c11*71,19c6*72,26c9f4z37f128g4p49h"
  "24h16t6c13*176,11c8*71,20c5*72,24c10f4z38f116g5p6g5p49h"
  "24h16t4h5c7*176,15c6*71,33c67,15c10f5z39f102g4p7g11p2g10p44h"
  "25h16t6h7c4*176,15c3*71,50c11f5z38f96g2r5g4p3g24p46h"
  "25h17t7h26c71,50c13fzfz4f2z33f93g5r4g31p46h"
  "25h18t8h4ch70c13f11zfH29f94g5r3g32p11h2e3h9e20h"
  "26h17t10h2c3h58c2*70,9y3fyz6f6z37f81g16r5g35p10h16ehe14h"
  "28h16t15h56c4*70,12yfz5fzf3zfz37f77g18r7g39p2h20eh2e12h"
  "34h11t16h56c2*70,12y2f2z3f5z39f75g21r7g37p3h3e2*133,19e11h"
  "36h8t18h52c2*73,c4*70,10y4f8z40f48g@25g20r7ge34p4h27e10h"
  "41h3t21h49c3*73,2c4*70,9yf6z3fz40f48g@27g19r6g4eaea27p2h32e8h"
  "25h2E14h3t23h47c3*73,3c3*70,6y54f47g2@32gr148,12r5g9e22p112,p112,p32eh2e7h"
  "25h2E16h2t23h46c3*73,2c5*70,4y54f47g3@33gs148,12r3g15e112,11p112,p112,p112,38eh3e4h"
  "27hE16ht22h48c73,4c5*70,2y53f48g2@35gs148,13rg22e112,p112,p112,48e4h"
  "28hE37h2c76,50c3*70,3c2y52f86g2s2*148,11rg76e3h"
  "36h3:22h6c4*76,52cfc53f87g2s2*148,11r78e2h"
  "2e33h5:22hcV3c5*76,50c56f88g2s148,11r80e"
  "2e33h2:h4:19h6c6*76,50c55f87g3s148,9ra81e"
  "3e38h>2=19h3ch58c34fwf4w12f3~5g2<14g4G60g4s148,8r82e"
  "3e37h2>2h3=14hc4h60c14fo15f4w3f2w10f5~5g3<13gG62g6s7ra82e"
  "4e36h2>2h2=14h2c2h63c13f3o14f4wf2w11f5~6g2<76g5s2*127,4r2a83e"
  "4e36h2>3h=12h4ch57c15f3o4f3o12f10w9f4~83g6s5*127,86e"
  "5e36h2>5h2F8h62c12f5o105,2o4f3o10f12w8f5~78g10s3*127,87e"
  "5e37h>3h3D10h62c11f6o110,2o5f3o9f3w4g5w8f5~76g12s89e"
  "6e51h64c10f7o2*110,4o4fo5f2ofo13g9f3~67g2{7g11s2*128,88e"
  "6e50h65c9f3o103,5o110,6of3o3f5o15g7f4~65g2{2g2{3g12s2*128,88e"
  "8e46hch47cX17c10f15of5o3f2o23g5~64g7{3g11s2*128,88e"
  "8e46hch49cX17c7f14o3*111,5o2f2ofo23g4~65g{g5{3g9s3*128,88e"
  "10e45h70cf8o8j12of5ogogo2g2o87g3{3g7s5*128,87e"
  "11e45h70c6o11j3o108,9o2*106,o109,8o85g5{3g2s10*128,87e"
  "12e4h170,39h70c17j22o2*107,o85g6{g8s2*128,90e"
  "15e2h170,36h71c19j25o85g9u2*130,s2*132,92e"
  "14eh2e37h15cZ47c74,7c24j5o3j12o86g9u2*130,94e"
  "18eh}33h71c25j5o5j5o2j3o2g8*167,77g8u130,95e"
  "18e2h2}32h71c43j2g11*167,75g10u35ea58e"
  "19e2h2}4h3*169,10h3qh8q2h63c64,7c43j2g12*167,g2|72g9u94e"
  "16eh4e2h2}2h5*169,8h13q2h64c64,c64,3c44jxgx12*167,3|71g7ua94e"
  "21e3h2}h6*169,6h15q2h67c47j2x13*167,4|68g9u94e"
  "24e2h}h5*169,7h14q3h66c48j2xg12*167,g5|2d63g7u131,95e"
  "25eh2}h4*169,7h15q2hnY63c50j2x13*167,|167,3|g3d61g7u96e"
  "26e2h2}10h15q5nY61c51j2x15*167,2|2*167,g10d52gm2*152,2ua16e129,80e"
  "27e2h2}9h15q7nY59c51j3xg17*167,3g9d49g3m2*152,100e"
  "33e8h14q5*84,4n59c51j3x21*167,g9d22gd21gmg5m152,100e"
  "34e8h12q5n6*84,58c52j3xg20*167,10d2gd15g4d16g2mg10m81e126,18e"
  "35e8h6q4h10n3*84,58c52j4xg18*167,14d14g5d15g14m99e"
  "29ea5e9h5q4h14n3*88,54c53j4x18*167,14d13g7d13g3m2g11m83e125,14e"
  "37e9h3q4h10n2*89,3n5*88,n2*91,c94,47c54j3x16*167,16d11g10d12g3m2g10m141,82e2*125,13e"
  "40e13h26n46c55j3xg13*167,17d10g11d13g13m3*141,96e"
  "43e10h27n82,36c66,7c56j3x13*167,18d9g12d2gd11g12m2*141,96e"
  "49e9h24n43c57j3xg3*167,3g2*167,22d6g18d11g11m2*141,22ea73e"
  "51e9h21n87,n36c66,5c59j2x6g25d6g18d11g11m2*141,95e"
  "53e7h23n83,42c30j2I27jx3g29d5g18d11g13m2*141,20e124,72e"
  "57e4h14n85,6n90,n43c59j33d5g13d95,4d2gm8g12m2*141,ma91e"
  "58e3h10nini10n93,43c59j6d2j3da20d5g18d2g3m6g16ma90e"
  "59e3h7n4iJ5i2ninin45c59j2d5j20da3d4g14d95,4dg5m4g11mamamam90e"
  "61eh3n2h2n4iJ10in46c65j25d3g2d101,16dg6mg12m2a5m142,12ea76e"
  "62e6h18i47c63j27dg2d2*101,12d95,3d2g17ma4m4*142,88e"
  "65eh2e19i47c62j30d2*101,17d2g21m5*142,7ea17ea61e"
  "70e19i47c60j31d2*101,18d2g15m158,6*174,2*142,32ea54e"
  "70e24i43c10j5c44j48d165,2d3g12m4*158,6*174,88e"
  "71e25i44cj12c43j48d3*165,d3g11m5*158,5*174,88e"
  "71e26i61c37j50d3*165,d3g9m5*158,7*174,87e"
  "71e27i61c36j52d2*165,2d2g8m6*158,6*174,87e"
  "70e29i60c35j54d3*165,dg5m10*158,5*174,159,48ea28ea8e"
  "70e30i60c33j54dad3*165,7*173,9*158,2*173,5*159,36ea48e"
  "58e122,e122,9e33i53ca3c22j2;8j58d4*165,5*173,9*158,2*173,159,5*175,162,5e2*138,ea74e"
  "70e36i54c22j3;7j59d5*165,5*173,7*158,2*173,5*159,3*175,162,3*175,3*138,54ea20e"
  "71e37i48ca4c22j;7j61d5*165,173,a2*173,6*158,3*173,3*159,10*175,2*138,2e5*138,6ea61e"
  "71e41i51c29j62d4*165,8*173,2*158,3*173,159,175,159,6*175,2*162,3*175,10*138,6e164,58e"
  "71e43i5ca44c16j?11j63d3*165,13*173,159,175,2*159,13*175,9*138,6e163,56e"
  "72e46i47c16j?10j16d100,48d2*165,13*173,159,3*175,162,14*175,8*138,2e3*163,4e113,51e"
  "73e45i49c14j2?9jd102,66d160,3*173,2*160,7*173,19*175,9*138,7e113,e116,49e"
  "74e45i48c16j?9j68d7*160,6*173,14*175,a3*175,10*138,58e"
  "75e44i19ca28c17j?9j73d2*160,173,157,173,a173,2*175,2a175,2*166,4*175,a8*175,3*138,3e4*138,7ea48e"
  "16e134,59e43i49c27j80da3*175,166,17*175,5e3*138,10e123,44e"
  "77e42i50c20jB6j87d15*175,7e138,11ea42e"
  "78e41i51c20jB6j56da32dk175,k9*175,k61e"
  "79e39i52c20jB6j3da4d96,81d6k5*175,2k60e"
  "81e37i52c21jB6j7d2*96,80d6k6*175,2k43e149,e149,13e"
  "82e36i52c28j7d2*96,76d2kd7k6*175,3k58e"
  "27ea55e5iN30i33ca18c27j6d4*96,75d11k6*175,3k21ea10e120,24e"
  "85e34i52c27j5d5*96,73d15k4*175,3k22ea34e"
  "9e151,78e168,30i53c24j7d5*96,73d17k2*175,5k22ea7e120,24e"
  "89e168,29i53c23j8d5*96,72d25k55e"
  "90e168,29i53c22j9d5*96,70d28k22ea30e"
  "90e168,29i54c21j9d5*96,8d97,60d31k16e137,34e"
  "ea89e168,28i55c21j8d5*96,7d99,59d34k16e137,33e"
  "91e168,27i56c22j8d4*96,67d35k49e"
  "91ei168,24i59c21j9d3*96,67d37k47e"
  "91e2*168,22i62c20j10d2*96,67d38k46e"
  "92e168,22i63c18j12d96,68d38k45e"
  "92e168,21i64c18j81d39k44e"
  "54e119,38e168,21i64c17j82d39k43e"
  "93e22i65c16j82d40k42e"
  "94e21i67c14j84d38k14ea27e"
  "94e20i68c14j84d38k42e"
  "94e20i70c12j85d38k41e"
  "94e19i71c11j87d9k7d2e19k41e"
  "87ei7e14ic3i72c10j87d9k8d3e16k42e"
  "95e15i76c2jc94d3k13d3eke14k42e"
  "95e16i79c110d7e11k23e139,18e"
  "95e16i79c110d8e10k24e139,17e"
  "95e16i80c110d8e9k25e2*139,15e"
  "95e12i84c110d11eke2k27e4*139,13e"
  "96e12i84c110d41e4*139,13e"
  "96e10i86c110d43e2*139,13e"
  "97e10i86c110d12e4*153,24e2*139,e139,13e"
  "97eie8i86c110d13e4*153,22e4*139,14e"
  "99e8i87c110d13e3*153,21e4*139,15e"
  "99e8i87c110d14e2*153,20e4*139,16e"
  "99e7i89c110d35e4*139,16e"
  "98eie6i89c110d36e2*139,17e"
  "100e7i89c18da91d35e139,18e"
  "99e8i89c110d54e"
  "99eie6i90c110d53e"
  "100e6i91c48da61d53e"
  "100eie5i8c68,c68,80c110d52e"
  "102e5i91c110d52e"
  "105ei2c80,90c110d51e"
  "104e80,2e4*80,88c110d51e"
  "106e4*80,30ca59c110d50e"
  "107e93c110d50e"
  "108e93c110d49e"
  "108e93c110d49e"
  "109e93c110d48e"
  "109e93c110d48e"
  "110e93c110d47e"
  "110e93c110d47e"
  "111e11c2l80c110d46e"
  "111e10c2l81c110d46e"
  "360a"
  ))
