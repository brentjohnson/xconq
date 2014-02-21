(game-module "noram"
  (title "Earth 800x320")
  (blurb "Earth's terrain, at 50km/hex")
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

(world 800)

(area 175 135 (latitude 6) (longitude 69) (cell-width 50000))

(area (terrain
  "800h"
  "125h4a301h3a9h54ea28h5a179h3a45h10a5h2a10ha15h"
  "72ha52h6ag251h3e19h10a2h3a8h7a2ha4eh53e2a8h4a15h2a2e3a14ha161ha2h29a4ha12h17a11h2a13h"
  "125h7ag240h6a7e19h11ah5a6h4a2f5a56e31a5e3a12h2a14h7a137ha3h30a17h17a26h"
  "126h7ag238h4a6e4a19h4a5f5af11a6f2a55e13a7e4a7eb5ebe3a6h3a2h3a11h3a5e7a132h32a17h17a26h"
  "26h2a78haha18h8a176ha60ha7e7a17h3a8f4a2f6af3a7f2a51e5ae3a5ea20e2b3eb2ebe2a3h2ah9aha5h2ah2a10e11a118h37a5ha11h17a25h"
  "23h4a13h4a25ha36h4a19h7ag65ha29h2a44ha8h7a6h15a56ha7e9a15h4a9f4afa2fa14f2a47e4a36eb2eb2eb3e2ah8ah9ah2ah2a15e10a88ha2h5a12h7a3c34a4h2a11h16a25h"
  "23h4a13ha3e2a59h5a20h7ag61h5a5h2a8h3a7h9a7h8a12h3a8h30a3e2aea2e10a2ha42h3a6e11a12h5a9f4a19f2a61ef27e5b3eb8a2e3a2e12a14ea7e16a65ha6ha6h6a4h7a11c66a23h"
  "23h3a15ha3e2a58h5a21h6ag37hahaha18h25a5h12a5h9a4h2a5h38a22e7a38h4a7e11a9h6a10f3a19f2a52e19f9ef9eb3e4b3eae3a9ea2e6a2e4f18e21a54h5a6h21a22c60a22h"
  "28e13h2a3ea59h3a2ha21h6a36h7ah2a8h31a4h13a4h16a3h29a32e9a22h8a5h8a6e24a10f4a18fea15e68f8eb34e9f5e8f20e3a16ha14ha19h33a26c4e66a8e"
  "24e4a13h2a3e2a27ha30h4a2ha21h5ag35h12a4h99a12e4f9e3f11e12a5h5a5h30a4e18a10f3a18fea14e71f7eb24e33f20e2a8h28a17h28a4c2e12c18e41a3e25ae"
  "2a19e7a14ha4e2a15h5a19ha17h6a23h5ag34h114a35f10e54a3e17a11f2a19f13e74f7eb17e42f18e18a2e2a11e48a3c7e7c34e25a19e12a"
  "37a5h3a3e5a2h5a4h3ae3a14h3a18h8a21h6ag32h115a37f14e52a4e13a10f2a20f6e82f5eb8eg2e50f16e15a3e5a17e40aec36e9g11e14a34e2a"
  "2e17a3e24a3e6ah2ae2a3h3a3e4a14ha16hah7aha20h5a31h117a3g37f16e10ae8a4e6a5e13a8e10a10f4a106f4eb7e3g53f2eg2eg28e4a22e30a36e7g23e2cb7c21e7f8e"
  "2e20a2e14a2e6a6e6a2e7a6e2a32h8a3h3a16h4ag30h116a6g37f16e9a4e5a3e4a7e6a20ef5a9f2a2f4a103f3eb7e3g49f2g4f2ge2g2e3g32e10g15e21a31e6g30e2c3b9c5e22f4e"
  "18e2a20eaeae2a6e7a2e6a8ea2ha4ha17ha2h2a2h14a16h4ag29h115a2g2f2g39f16e8a5e10a9ea15e13f3a8f2a5f3a101f3eb7egfg50f3g3f15g5e5g14e14g13ea7e17a9f15e4g8e9f5e9f5e4c3b5c14fbfb15f"
  "3f17ea20ea3ea7e6a3e4a9e21a3ha7h16a17h3ag27h116agf3g42f14e9a3e10a20e21fa7f3a3fc4fa101f3eb6egfg44fg6f3g3f12g12e6g2e2g3e6ge14g18e15a14f5e5g13f7c20f6c6b24fb6f"
  "5f16ea33e4a4ea10e25a2ha9h13a16h4a27h116a45f8a7e10a2e11a17e31f4a3f4c2f3a99f2eb6e2g45f2g6f3g3f6g23e11g9e5g3e8g13e18a2c27f11c24f2c4f2b14f9b6f"
  "11f62e28ah3ah19a15h4a28h114a2f2g46f8a2e6a3f4ae10a13e33f6a3f5c104f2eb6e3g44f3g5f2gf7g19e13g27e5g11e8a5c4a26f13c32f4b17f2bfb4f"
  "3fb10f57e31a4h21a13h4ag27hg113a4g19f6a24f12a9f9a6e34f6b6a2cfcfc3f2c103f2eb7e2g4f2g38f3g5f8g2f14e15g22ea16e10a13e20f3b4f5c6f2c35f3b12f10b"
  "10fb4f2e5f46e60a12h5ag26hg112af3g20f7a26f3a2f3a13f4a4e34f4cb2c3f4cf4cf5cf2cfc101f2eb8e5g3f7g14f3g13f3g5f6g4f5e19g15e3g9e5a10e13a12e8f5c2f5b57f5b6f2b4f5b"
  "6fb15fb2f43e62a11h5ag26h112af3g19f6a29f2a59f9c3f2cf7c2f3cf5c100f2eb9e13f4g11f6g10f3g4f5g4f21g16e5g4e13a7e16a10e8fc7b9f2c60f2b8f2b"
  "15fb13f2ef34e64a12h5ag25hg109a2f2gfg19f5a31f2a58f3cf3cfcf3cf2cf6cf4cf4c101f2e2b8e14f5g10f8g7f3g3f5g2f18g20e2g12e13a3e19a8e7fcb4c12f3c70f"
  "2fb17fb4fb9f26e70aha8hah6ag24hg108a4f2g18f6a91f7cfcf4cf17cf3c99f3e3b20f8g7f9g7f3gf3gfgf12g33e7g3e41ae7fb4c13f3g3f4g62f"
  "25fb14f19e73a11h7ag22hg109af3gf2g16f6a92f8cf7cf16c2f2c100f4e5b18f8g8f6g6f7gfgf7g2f32e7g8e23a3f14ae7fb3c12f3g2hg2f6g60f"
  "5fb5fb6f2b21f16e77a7h11a21h2g107a2f2g2fg16f7a52fe39f4cf33c12fc89fe5f2b17f11g6f5g6f4g3f8g5f26e10g4e31a2f9a4e7fb14f2g3h2gf2g2h6g57f"
  "12f2b2f4b23f13e37a5e38ah2a2h12a19h2g108a2f2g2fg15f9a51fe39f8c2f11cf8cf4cfc3f4c2f2c2f3c12fe2fe79fb12fg5f12g6f3g11f7g8f9e3a10e10g5e42ac3ebe7fb14fg3h2g2f4g2hgh11g48f"
  "13f5b11fb15f10e39a8e35a3h15ag15h3g108af3gf2g11feaef9a52fe5fe7fe4f3e4fe2fe2fg7f5c2fc2f11cf6cfce23c15fe78fb11fg7f12g6f2g9f6g6f6a6e2a3f7e10g5e42acb4c2e7fb13f6gfcf5g2hg10h7g42f"
  "9fb2f4b6fb10fb14f7e39a10e53ag12h2g110af3g14f5e9a46fe2fefe4f2efefe3fefe2fef2e3f2e4fg6fcfe13cfcf10cf6cfcf5c2fc2f5c9f2e4fe91f2g8f15g6fg3f7g5f8a4e3a4f3e11g6e44a2c8b4f2b13fghg4fcf7g4hg8hgh7g38f"
  "10f5b10fb26f3e40a11e52ag11h2g36a6g69a3g14f5e9a10f3e6f3a20f3e2fe6f2efefef2ef2efef2e2f2efef3e9f6cfce9c2e6cfe2ce22cf2c7fe96f3g3fg8f12g2fg4f9g3f11a2e3a5f2e9g6e48a7c2f5b14fghg3f2a2f2gf8g4h16g2f4g2fg26f"
  "20fb7fb24f2e40a16e49ag8h2g36a8g68a3g12fafa2ea2e7aea10f4ef3e4a4f2a13fef2efe3f2e4f6ef3efe2f3e2f3e5fg5f2c2ef10ce20cf2cf17c8fe93f4g3f2g11f10g3fgf8g3f12ae3a7f15e52a3cb2c19fg2hgf4af3g2a10g10h17g24f"
  "54f2e39a18e48a2ghg3h2g38a8g67ae2g14f7e3a2e5a4fe8a4e2a4e19fe5f3e3f2e5f3e2f2efe2f2e2f3e8f11ce11cfe10cf7ce14c7f2e89f7g3fg16f8g2f6g6f15a7f2a3e7a2e56a5ce17fg2h4a3f2g3af2a6g18h10g11f7g6f"
  "18fb5fb6fb7fb16f40a17e10ae41agh2g40a7g68ae2g13f8e26aea4e19f2e2fef3efefef3ef2ef2e2fef3ef4efefe6f4ce14cece9c2ef14cf10c7f2e87f2g7f3g7f4g27f14a7f3a2e9ae62ae16fgh4a2fgh3gaf7a3hgh2g2hf6hg9h6g2f3g4f5gfg6f"
  "35fb21f38a14e5f9a2e87a2e2g43a2e24a4f7a4f8e13a26e6fe12fe4fe3fefe3fefef4e4fefefefg3f8cf2cf4ce2cece29ce8c8fe83f5g7f2g8f4g28f11a6fg3f77a6e2f2e7fg5afg2h20aga3ghg4h7gh6g2f2g5fg5fgf2g2f"
  "29fb28f37a9e12f6a4e86a2e2g40aea3e26a2f7a4f7e10a2ea34ea8fe7fe4f2e2f2e2f6ef7e4f2e24ce2ce31c13fe84f4g9f5g6f20a5f8a7f2g3f78a4ea3ea2e2f3b2f4afg31ag3h6g4h3gf2g4fg7f3g2f"
  "3g21fb9fb3fbfb18f36a8e14f4a4f2e129aea2e2g2e32a2e2f7e10a2e2a11e3f15e2b5e2f2e2fe2f2ef3e4f3ef5ef10ef2efg2f3ef6ce16ce32c4fe4fe80f11g8f6g5f21a7f6a8f2g3f3af85ae4f3g39a8g3h4g2fg10f4g2f"
  "gf2g43fb13f35a4e19fa6f2e130a2e3ge33ae2f8e15a9e4f15e2b20e2fef2e2f6ef4ef6e3f6efcf8ce2ce10cf3ce20c2e3c5f2e3fe2fe68f17g9f6g4f36a7f3g4f2af85aefa2fgh3a2f36ag2a6g2h4g2fg10fgf2gf"
  "3f2g26fb6fbfb2fb18f36a2e29f130a2e2ge29a2e3a10e2a2e6a2e3a9e4f44ef9e3f7e3f6e50c4e2fefe2fefefe2fe36f3g3f9g5f4g4f21g8f4g4f38a6f3g5f86a3ea2f2g3a2f39afa7g2h3g4f2g11f"
  "g3f2g28fb2fb25fa3f32a32f129ae3ge28a2e4a8e3a2e6a3e3a66ef8e4f4efef2e2c2e4cec2e4cf2ce2ce23c3e11f2e3fe35f2g2e4f45g4f3g4f40a6f2g6f85a2e3f3g3a3f40afa7g2h3g4fg5f2g4f"
  "3g6f4e26fbfb3fbfb6fb17f28a32f122a3e3a5e27a4e4a5e13a82e4f15ef9e7cec2e16c5e12f4e36f2ga7f45g7f42a5f3g7f82a3e2f3g6af42afa7g2h4g13fg"
  "5g5f6e20fb6fb6fb24f25a34f120a4e2a4e27a8ea3e15a83e4f16e2f5e10c5ef3e8c7e9f7e34f3ga7f10g3f7g2f24g4f44a5f2g8f81a2e2gh2g52af3a5g3h3g11f2g"
  "3g2f2g4f7e11fb9fb3fc3fbfb27f22a37f117a5e3a7e23a12e15a83e2fg2f16ef9e5c4e2f2e2f3e5c8e7f8e10fe23f3ga5f10g20f20gf45a5f2g8f79a8g55a2fa5ghgh3g10f2g"
  "3f5g4f4e2b2e2fe9fb2fbfbc2fcfbfc6fbfb5fbfb18f17a40f115a7e3a6e23a3e2a2e18a27e2f56e4f34e2f2ef20e6fe5f3e13fe20f2a2f8g29f8g11f2af41a4f2g8f8ag69a3g62a2fa7g3h2g10f"
  "3f6g5f12e13fbfcb3fb2fb7fb33f5a43f114a7e5a5e23a4e20a28e2f55efg2f35e2fef21e14fe6fe6f2e18f2a50f2g8f42a4f2g6f11ag67a2g65af2a5g2f3h2g3f2g3fg"
  "7fgf4g2f16e3fb8fcb2fcb3fb3fbc2fbfb29f5a6fb22fbfb15f109a7e6a6e21a4e14a3e4a7ef76e5f54e27fa8f4e11fe3a16f8g23f2g3f6g3f2af3a3f3ae29a3fg4f2e78a2g69af2a3g4f4g3f2g3f"
  "7f4gf2g2f23eb6f6c2f4cb33f6a4f2b23f5b14f108a6e7a6e21a4e3a4e4a16e2f75efg2f56e11fe8f6g7f2g4fe10f4a9fg3f8g27f3g4f4g3faf2a6f2af28a3fg4fe74agagag75a3g5f4g3fg2f"
  "14f2gf27e2fcf2c2bcfc3bc3fb3fb26f6a29fb20f105a6e7a7e19a109e3f52ec5efefe6feg5fe9g2f3g5fgfe3f4e6a8f2g2f4g35f3g2f3g14fa2f28a2fg4f33ag40a2g75a3f2a2g7f7gf"
  "16fgf28e5fc2bc4bc5fb28faf3a51f104a6e5a12e9a116e3f59e5fe2fe2geg2e2f2g5f12g4f5e6a8f2g23fe20f2g3f2g13fa2f29a6f35ag25ag4ag2aga2gag77a3f3a9f6g"
  "g3fe6fe5fgf28e4fb2fcfc2bc2fbfb30f5a22fb27f106a2e8a11e8aea47e2f67e2f61e7f4g2e2ge9fe2fe3fgf2g3f4e3a2e7f4gefef2e18f2e7f2e13fgfg10fc3fa2f28a4f155a3f3a9f5g"
  "2fg8fe4fgfgf29e3f2c2f2c2bc5fb2fb27f3a33fb16f106a2e9a10e7a50e3f66e3f64ef3e4g3e3ge10f3g3f4gfe4a6e3f3g2f4e2f3e15f2e3fefe15fg8fe4fc3fa3f28a3f50ag3ag4ag97a2f4a8fh2fg"
  "3f2ge5fefe5fgf30e3fb5cbc2fb33f2a28fb21f119a8e7a50e2f69ef67e5ge4g4e7f2g4fe2fg3f10e8f6ef4e8f2e9f2e3f2e12fg5f3e7f2a3f28a2f165a10f"
  "4f3g3fef2e8f31e2fb2fcfcbc5fb3fb2fb23f2a11fb4fb5fb25f2af114a13e4a52ef137e5ge2g3e2ge2f3g2f2gefefe3g9f7e6f6e2f4e7fe8f2e3f4efe15f3e7f3a3f28af168a7f"
  "5fgfef2gfe5f2g3f31e3fbfcfbc14fb4fb64f3a2f113a12e4a6e2f18e5f104e3d53e7g8e10g7e2fe6f8e5f8e2f3efe6f2g10f4efefe3fe6fgfe11f2a4f200a3f"
  "a5fgfe3gf2e3f2g4f32e2fbc2fbc2fb2f2b3fb48fb22f4a2f113a3e10a9e2f5e2f10e2f3egfg102e4d52e8g6e2g2f8g24e5fe3f6e10f2g4f2e3f5efe11fg2fe11f3afcf27af167a5f3a"
  "f2a7f2gef2e7fg2f32e2b4e12fb2f2b6fb7fb31fe21a3faf111a2e11a10ef5e2f10e2f6e4g98e4d53e8g2eb8e7g23e5fe3fef7e7f2g3f2e4f5efe6fe4f3e2c10f3aefaf26af169a6f"
  "2fe2a3efg2eg2fefe9f38e2cfb3fb5fbf2bfb31fb9f2e9a2f11a3fa3f119ae2a28e2f10eg2e5g90e6d49e11g16e4g20e12f7e6f2g3f3e3f6efe10f3e3c9f4a2f200a3f"
  "3fea3e2fge2d6e5f2g2f36e3c2b5fb17fb4fb24f2e13a2f9a12f111a2e2a31e2f11e11g19e2b63e8d45e15g16e4g23e8f9e4f2g3f4e3f4e2fe10f3e3c10f3aef25af177a"
  "7ae2fge3de3d2e6f2gf35e2c4efbfb3fb3f5a15fb7fb8f5e2a8f15a3fc4f3cf111a37e5f7e3g7e5g58e8d14e5d49e15geg41e3f12e4f2gfa5e3f4e2fe2fe7f2e4c10f4af203a"
  "3a4fae2fg8d3e4f2gef41e9f7a30f5e2a11f13a4fb8f112a59ef4g37eb18e9d14e5d44e2b5e16g38e4f13e3fg3f6e3f3e2fe9fef3cfc10f4ae203a"
  "4af2gfaefge2de3ded4e2g2fg2f43e5f12a7f2e4fb2fb5f8e2a11f13a12fafaf113a31e8f19e5g34e2b24e3d2e7de2a4e2d2e4d29e2d10eb7e11g13e4g25e2f14e4f8e2f4e2f2e8f5c12f5af23af178a"
  "5afgfae2fge9d2e2g2f3gf44e2f5af8a2f8e7fb3f8ea14f20af3a3f114a23e15gf14e2f2ef5g17eb38e3c3d2e7d4a7e4d29e2d17eb2e10g11e6gd40e2f9e2f8e7f7c10f6a2e201a"
  "6a3fe2fg2e9d3e5g3f42ef4a4f7af13e3f10e3a14faf5a2f8aea2f4a2f115a7e2f11e11gfgfgfgf15e2fe6gf24e3a25e5a2c11d5a7e7d5e8d2e11b2d3g17ege8g7e4g8d38e2f9ef8e2fe5fe6c10f7a2e18af181a"
  "6a3f2e2fge10d3e4g2fef41efa11f4af9eb14e2a17fa3f2a2f134a6e3f8e10g9f19e7g8e3a9e5a24e9ac11de5a8e6d4e10dbe11d3gb23e4g6e4g11d55e2fe7f5c9f8aeae13a2f184a"
  "9afe2fege3de6d3e3g6f40e2f5e4f5eae3a20ea19f4a5f133a7e3f6e4g3e6f3efg2f15ef9g7e9a2e7a24e8a3c10d2e4a8e8d2e10dbe10d6gbg14e17gd2g13d54e7f3e4ce8f22a2f186a"
  "8a2fe4f2e11d2e2g4f2gf48e2fe13a16ea17f2a7f135a8e3f4e3g13eae2fgf14ef8gf6ec7a5e2ab2a25e7a4e10d2e5a8e5ded2e7de2debe8d10g11ege16g3de15d53e6f3e2c2e8f10aef2e7a3f187a"
  "8aefe2fg2fe7de5d2e3f4g2f4eg44e5a2e5ae2a12e3a13f5a6f138a8e2f4e4g12e2aea2fg13efg7f8e9a6eb25e8a3e11d3e4a8e5ded3e6d2e11d10gb12e17g3de15dec51e6f3e2b2e7f11ae3f3ea3f190a"
  "9a2fe4fe14d4e3gf2gf48e5a3e5aea10e3a2efb11f4a6f140a13e4g12e5ae7f8eg14e11a2e9a20e10a4e10d2e2a3e5d2e5ded11e9d5e13g4e8g31dec7ec41e5f6ebe7f12aef2gf3e193a"
  "9a2fe4f2e16ded2g2f2gf2eg44e4a5e4a7e6a3e3fe4fe3f6a5f141a14e4g12e5a2fg5f7e2g13e24a18e11a4e10d6e6d2e4d2ed2g14e3d6e13g2h8g33d2ecec2ec42e6f7e6f11a4e3fe195a"
  "9ae5fgf2e13dg4d6g2f2e3g41e3a5e5a5e7a2e4fe4f2e9a3f127a10e5a19e6a7e5aef2g2f6g2e2g13e25a2e4g11e12a2e9de18d2ed2e2g11eg4e3a6e6g6h6ge10d2g24d46e8f7e5f12a7e195a"
  "10a7f2e3de6d2e2dg3d7gf2e4g40e3a7e2a16e3fe3f2e137a7egeg19e2a3e11a7e4afe2fgfge4ge3g11e28ae7g9e10a2e9de8d2e8de2d3e2g11eg8eg3e5g7h6ge12d2g24dec42e9f6e2f15aeae3ae195a"
  "10a7fge18d7gf4e3g39e4a6ea4e5a13ef4e137a3egeg12e5g5e11ae6a6e5a2e3fege7ge7g2e29a2e14g2e9ae2a11d2e13de2d4e3g10e10g2e5g6h4g3e40d2e2d40e8f4e20ae200a"
  "11a5fef2e17dge3gf2gf3e3g40e3a9e5a20e137a20e7ge11a2e6a6e9a2ege9g7e29a4e13ge10aebae21de2d7e15g3b5g3e6g3h5g3e46d38e8f3e23a2e197a"
  "11a5f2eg13d2e4d8g3f5g38e3a7e6a21eae136a2eg19e5ge11ae7a6e10a2e2ge11g3e8a6e3ge10ae4g3e10ge13a3e2de18d3ed3e20g7e16gde15d2g28d37e7f3e24a3e196a"
  "12a4f2e15de2db2d8g2f5g39e2a18ef17e136a28e20a8e9ae2ge5g9e3a5e2ge8ge7a7geg5e5ge10a2d3e22d2ed4e4g2e13g6edg2d2gdg2dgd2g22dg28d25e3a9e6f2e24a2e2f196a"
  "12ae4fe11dg4dg2dbdgd3gd2ge2f5g57e3f9e2b141a14e2g10e13a2e10a5e9a4e2g4e5a9e2g4eg2e22g8e2ge10ae2d3e21d7e2g5e9g5e67d25e5a9e5f2e24a2e3f195a"
  "13a2f3e10dedg4dgd2bd3gdgdg2df3ghg56e4f12e141a10e2g10e16a3e10a8e5ab2e3g6e2aea2e3aegeg10e11ge3ge2g2e3g7ege10a3d2e22d6eg8e7ge2g2e65d26e6a9e4f2e25ae4f195a"
  "14af3eg8de4dg5dg2d3gegdg2dge3g54e7f9e143a7e5g6eg2e8ae9a2e12a4e2a2e4aeg2e2ge2ae10aeg3eg4e3d3e18gegb2g9e9a4e23d11e4g2h9g2e27d2ge7d6g19d26e7a10e3f2e25ae2f2e195a"
  "15af3eg12d2g4d2gbd2ge2gdged5g53e4fg2f9e144a20e7ae11a2e14a2e3ae4ae6g4ae2a3e3g9e4d2e3ge22g5e11a3e24d2e12g3h9ge27d3g6d2ghgh4g17d25ec6a5e4a2e4f24aeae2f2e195a"
  "16af3egde9d2g4d2g2d5g2dg2d4g52e8f6ebef144a21e3ae14a2e15a2e9a2egege8aeg3eg8e3d2e3geg2e2g2e16g4e11a4e24de12g4h7g2e27d4g4d7gh4g15d25e7a2e7a4e3f21ae3a2ef2e195a"
  "16a5eg8de2dg4d2g3dgd2g5d3ghg50e4fg3f6ebebe144a21e36ae8aea2egege2ae3aea2e2g10e4g3e2gege9ga10g4e11a2d2e2d2e23d11g3h10g15dgde5g2de13gh3gh2g14d25e7ae9a4e3f20ae3a2e2fe195a"
  "17aea3eg16dg6dg6d5g49e8f4ebeb2e104aeae40a18e37ae11a2eg4e6a2egegegegege8g2e2ge4ge4g2a9ge2g2e11a2d2e2d3e22de7g2h16g14d25gh6g11d27e16a6e2f20a4efgfe195a"
  "18aea3e2g5dg5dg6dg10d4g31eb17e4fg3f5ebebe110ae36a18e31a7e15aeaeae3a10eg6e3g7e3d2e9ga6ge2g5e3g3a4ed5e3d2e16d8gh23g2d19gh13gh7g7d30ec16a4e2fe18a2e2fgf3e194a"
  "19a5e2g5dg4d2g5d2g8ded4g32eb14e8f7e2be148a11e3g2e33a4eg14a4e8a8e3g2e4g8e7dg2d5ga5gegeg5e4g6ed5e2d2e16d45gh2gh27g37ec2a2e12a3e3fe16a3e4f3e194a"
  "20a5e2g5dg3d2g5d2g11d2g24e3f20e6fg2f9e149a3e3a9e24a4eae6a4e14ae2g4ae3a10e7g8e11dgd3ga10g13e28d7gh49g4c16g43e8a3efefe15a2e6f4e193a"
  "21a2g3e3g3dg4dg6dg12d2g22e7f4eb12e4fg3f11e155a3e14a16e3g2e9ae16age19a5e5a3e13d2e11geg13e24de9g2h15gh11gh3gh16g8c6ga6g41e10a3ef3e8a7e2f3efg2eae193a"
  "22a2g5eg10d2ge15dgeg19e9f15e10f10e156a2e13a2e19ge38ae18a3e14d3e6ge6g20e7de2de4ge31gh48g40e11a3ef3e7a2e8f2ea3e2ae193a"
  "23a2g4e4ge6d2g12dg4de2g18e3f2g4f4eb10e3f2e5f11e168a3e2g4e15ge36ae19a2e16d3ege11g12e3d4e2d2e7d14g2f38gh11gh3gh14gf38e13a7e4a11ef2e200a"
  "25a6e2geg3dg15d2g2dg2d21e7f3eb11e3f2e5f10ece154a2e8a4e3g3e12d5ge9ae19a6e22ae17d4ege3ge6g10e12d5egd14ge6f52gb11g2f36e14a7e4a5e5aefe201a"
  "26a7e3g18d2g2d2g3d21e5f20efe2f11ece154a11e4g4e18de51a2e4ae18d5e10g9e11ded5e16g5e2f55gh7g3f37e14a3e7ae6a2eaefe201a"
  "31a4e3gdg14d2g3dg4d21e2fe2f3eb26ec4e154a8e5g3e22de57a2e18d5e7geg9e13de6d14g7ef51ghgh9g2f38e22a4e2ae3fea2e201a"
  "27aeae4a4egdg14dg4d2g3d21e5f31e32ae123a9e2g4e25de56a2e18d7e8g2f6e18d2e4d4g5ded7ef62g3f38e20a3e2feaef2ef204a"
  "31ae3aeg3ede19d2g4d20e4f3eb26e157a9eg2e32d53a2e19d6e8g2f6e21de2d7g5d7e2f52gh3gh4g2f39e20a3egfeafeaf204a"
  "36aeg2e13de8de4d20e5f28e138ae18a6e7g34d19a4e28ae20d9e7g2f4e26de4g2de2de2d6e2f4gh41gh13g2f39e19aeaefge2ae206a"
  "36ae2ge12d2e6d2e7d19e4f3eb24e157a6e5g41d13a2e5d26ae20d9ege5g2f5e25degdgdg3d2e2d7e2f4gh56gf39e19aeaefe209a"
  "37adg19d2g11d19e2f27e158ad7e2g44de10ae7d25ae21d11e6g2f4e23ded3geg2dede3d8ef4g2h41ghgh11gf37eb2e19aefe209a"
  "38adgd3a15dg12d23eb22e159a2d6eg46de10a11d21ae22d11e5g2f5e2de22d2gdeg4de2d9e2f5gh37gh9gh5g2f24e4f9e2b19a3e209a"
  "39a3d4a14d2gf9d44ece160a2d3e3g47de10a16d5ae2c2e4ade25d3e2c3eg2e6g6e23dg2d2g3d14e3f5gh40gh9g2f26e3f7eb3e20ae209a"
  "40a2d5a14d2g10d23eb20ece141ae17a2d5g54d5a22d5e33d7eg2e4g7e21de2gd3g3d8ed6e3f5gh30gh4gh11g4f39e230a"
  "41a2d5a9de16d17e6cebe2c6a7e2c2e147ae11a2d2g60d2a24d3e38d4eg2eg11e21d3gdg2d7e4d6e3f6g2h28g2h3gh3ghgh4g2f32e3f3e21ae117ad92a"
  "36ad5a3d4a26d16ec3a3c2ecb8a2e2a4c2e145ae3ae9a90de2da36d2a6eg11e22de2ged6e5d9e2f6gh4g3h12gh4ghgh2gh13g5f29e4f2ea3e228a"
  "43a4d4a11de2g10d13e9a4c16ac3e157a92de2da5da29d3a2egeg13e22dedg2d2e8ded9e3f5gh6gh2g3h2g2h3g2hg2h5gh5gh2gh3g7f26e4f6e228a"
  "45a3d5a11dg9ded11e14a2c15ac3e155a93de3da4da30d4a20e3de15d2g2de11d11e4f4ghghg2hgh16gh2gh3gh9g3fefe2f25e5f5e228a"
  "44ad2a3d4a22d11e32a4e153a94de4dadgda31d5a20e17d2g2de11d15e3f7gh2g2h13ghgh11gh3g2fe4f26e5f4e17ae210a"
  "47a4d4a6de13d12e33a4e149a97de5da2da32d6a20e15degd2e11d18e3f30gh2gh6g2f2e4f24e6f4e228a"
  "46a7d4a3d4e3dg8d11e33a5e148a98de5d6a30d7a19e17de12d21e5f14g2f3gf16g2fe4f25e5f3e229a"
  "51a4d3a3d5e3dg7d10e33a3eb2e147a99de5d5a32d7a17e14ded2e11d26e6f4g3fge3f4g4f2gh9g5fe2f3e2f20e2f4e229a"
  "53a3d4a3d4e2d3g6d10e34ae2ce146a101d2e4d6a32d8a2e6a6e6d2e2ded3ed2e9d32e5f7e2f4g4f12g8f3e2f11e7f2e2f4e17ae146ad64a"
  "55a2d4a3d5e6dg2d10e35a2c2e145a103d2e3d6a33d10ad4a21e10d34e3f8e6fg4f11gfg5f17e8f6e18ae211a"
  "56a2d5a2d6e7d11e35a2ce145a105de3d6a32dadad6ad6a20ede7d36e2f6e13f10g7f2ef14e7f8e229a"
  "56a3d5a2d9e3d11e36a2c145a105de4d6a32d2a2d4a3d23a4e5d49e11f8g4fg3f2e2fef11e2f3ef9e4ae224a"
  "57a3d5a3d13eg7e35ae146a107de4d6a33da2d3a4d23a3e4dec49e11f7g2fb3f4ef28e4a2ef43ae179a"
  "58a4d5ad10e5g7e181a72d2g33de5d6a35d3a5d23a9c48e11fg2f5g3fg2f9ef22e4ae2f6aeae168ae45a"
  "60a3d5ad9e5g7e181a72d3g33de6d5a44d23a8c48e5f2e7fgfg2fg5f2ef27e5ae2f177a2f44a"
  "62a2d6a8e5g8e180a72d5g3dg34d6a49d20a3e2c48e5f3e20f6ef19e6ae2f177a2f44a"
  "63ad7a7e2g3e2g6e33a4e144a72d10g33d6a49d22a2c41eca5e5f4e20f24e8ae2f180a2e40a"
  "72a6e2g3e2g7e30a9e141a72d8g2d3g33d5a48d19a2e2c40e3ca4e5f5e20f16ea3e12aef180a2e40a"
  "73a5e2f3e3ge3g2e29a2e4a2c6e137a74d13g32d5a47d20a8eae2f30e5c3a2e4f5e18f19e16ae185aef35a"
  "74a5ef4e8ge32ae5a7e135a80d6g33d5a46d22a6ea2e2f28e10ae5f5efe16f14e206a2f35a"
  "74a4e2f3ef5g3ege15a8f17a6e133a83d2g35d5a45d24a4e2ae2f27e12ae4f3ef5e15f6e3ae213a3e30a"
  "63ad11a4ef3e3f2gf3e2g2e13ac7f18a7e131a120d7a43d30ae2f26e14ae3f3ef7e14f4e5ae25ae186a4e29a"
  "76a6e9f3e2ge12ac8f21ac4e130a120d8a40d32a3f23e18a2f3e2f7e13f3e220a4e28a"
  "78a5e3fe7f2g2e12a8f20a8e128ae119d9a37d34a3f22e20a2f3e2f8e12f2e5a4ef211a3e28a"
  "80a9e9fe11a8f30a9e118ae110de8d9a35d36ae2f20e23af2e2f2ef6e12fe5ae2f2e22ae101ad117a"
  "81a10efe5f3e9a8f32a9e11ae105ae109de9d9a34d36a2e2f19e24af2e2f11e10fe4ae3fe242a"
  "82a14e4f2e8a8fc20a4f9a9e3a4eae106ae103de10de5d9a32d38a2ef19e25af2efe2fef7e9fe5a4f242a"
  "84a13e5fe3af2c9fc21af2ef4a8e16ae104ae104de16d8a31d39ae2f18e25af3efe2f9e9fe6af24a3f216a"
  "87a12e18fcf36ae104aeae15a101de8ded2e8d8a29d40a2e2f17e26af2efe5f7e9fe30afg2f215a"
  "89a14e17f54aeae101ae4de2de82d2e7de2de3de3de3d2e6d8a26d43a2e2f15e27a6e5f2ef5e8f2e29a3f215a"
  "92a12e16f145ae12ae2de4de2de66de2de12de10de12de4d7ada23d46ae3f13e28aec4ea2e3f11e7fe27afg2f214a"
  "95a12e12f59ae99ae4de2d6e60d2e6de6d3e6d3e8de8d4e2d9a19d50a2e2f12e29a3ce3a2e3f11e8f25afa3f214a"
  "99a5e3a2e18fc52ae82aeae13a13e2de2de2dede53d2e4d3e7d2e9de2d3e3d10edad7a18d50a2e2f12e37a2e3f2ef10e6fe24afef215a"
  "109a2e17f3c148a17e2d2e3d2e22de2d2e28d3e3de5de5d3e6d13e3gd9a16de51a2e2f11e38ae3f2e2f7ef2e6fe24afef54ae159a"
  "111a2e17fc50ae97a17e2d2e3d2e4dede15ded2e27d4e11d3e2d23e2g3d8a13de54a2e2f10e39a4f2ef6efe2fe6fe23af2ef213a"
  "113a3fe14fc146a34e2dede6d3ede2d2ed2e3ded2e22ded3e8d25e3g3d7a9d2e56a2e3f10e38a2e2f3ef4efef4e6f24afaef212a"
  "114a3e7fe6fc50ae97a32e2de3d2e2d6ed12e2d3e2b19d2e6d28e3g3d6a9d59a2e3f9e39ae3f7ef3e2f2e5fe25a3f2af208a"
  "117a4e2f2e7fc148a66e2b14d3e4de3d29e4g5d4a6de17a2e42a2e2f10e39ae2f9ef6e4f2e22af3a4fefaf43ae160a"
  "120a4eae6fc49ae3ae94a67e4b8e2d8e2d31e6g5d2a3d2e64a2e2f9e40aefe2a8efef2e4f2e24a2f3af2af205a"
  "125a2eb5fc148a6e3f58e2b52e7g6da2de66ae2f10e25ae13ae2fea5ef2ef5e3f3e24a2faf2afafaf202a"
  "126a3e4fc26ae21ae99ae5fe5f96ec12ege7g5d69a2ef3e2fe2f2e40aefe6ae2f6e2f4e23afaf3afaf2a2f201a"
  "128a2e5f23a3e123a4f2e4f98ec10e10g4d16ad53a2ef2e2fef3e26ae13ae2f7a2ef5e2f4e35a2f200a"
  "129aeb4f19a2e4f2e3ae117ae4af3e6f93e4c9e10g4d11a6d53aef9e41a2f8aef11e23af5afe3af2af199a"
  "130ae2b3f18a3egfgfea11e12ae103a3e5f91e2c3e3c6e12g4d7a9d53a2ef9e26ae12afa2f8a11e24af5afeafafaf200a"
  "131a3e3f17a3e2fgfea4e4f6e2a4efae105a12f84ecece3c3ec5e6g2e5ge18d54a2ef8e41a2f11a7e25af6afafaf2af199a"
  "132aea2e2f17a3e3fe2a2e5f12e2a2e106a10f85ece8c6e5g3e4g3e17d54a10e41a2fe9ae2a4e26af8afafafaf198a"
  "135a2e3f15a3e2fgfc2ae3f14e2f108ae11f43ef5ef6efef26ece5cec6e11g11e9d55a7e3a2f39aefe12a2e26a2f7a2faf32af169a"
  "137ae3f5ae3f4af5egfecae2f15e3f3c105ae12f20e4f12e3fe2fefefef4e9fef20e10c5e12g11e8d56a6e3a3f38aefe12a2e26af9a2f5af197a"
  "139ae7fe4f2afef4eg2f19efe2f3c105a18f10efe22f2e2fefefef2e11f20ece7cec6e10g10e9d57a4e5a4f37af2e12ae26af15a3f196a"
  "141ae5fe2a3fafe2f3e3f3g17e6fcf103a19f9e26fe4fe2fe16f18e3cec2e2c6e10g10e9d58a2e6ae4f36aef2e51afa5f195a"
  "143ae2fc5a3fe4f2egfg2f18e9f102a57fefe20fef16e5c12e6g10ede7d67ae4f24ae13a2e48a5fgfef195a"
  "142ae2a3f5a7f2efg3f18efe9f101a83f2e5f8ec4ec11e4g10e10d67a2e3f39a2e37af9afafa4fef194a"
  "154a6f2efg3f18e13f100afe78fe3fefe5f6ece2c12e4g9e2de8d68a2e3f25ae13a2e35a2ef13afc2faf70af122a"
  "155a2fe2f2e3g2f4e2f7ef5e13fc100a93f20e3g9e11d69a2ef41a3e32afeg2f8af4a3f195a"
  "155a2fefg2e3g2f3e4f5e3f5e13fc7f94ae24f4e7a57f20e2g8e11d114a3ef3ae26af3gf6af9af194a"
  "155a2fefgf2e2g2f2e13f6e22f94a2e17f5e11a57f31e9d103a2f10ae4f27a2f3g2fc209a"
  "155af2ef2g2eg19f7e21f95a16f2e16a11fe46f30e8d104a5f6a2e5f26af4g4faf205a"
  "156a2ef3geg20f7e21fc95a5f29a8f2e47f7ec3eb18e7d105a2f2g2f5ae6f23a2fef3g3f208a"
  "156a2e2f2gfg22f5e21f2c136ae7f2e39f10eb18e7d108a2fgfce4ae5f23a4f2g3f19af189a"
  "156ae3f2gfg23f4e22fc134af2a6f2e41f10eb17e7d110af2g2e4a2e4f9af12a3f3g3f16af192a"
  "156ae3f2g27fe24fc136a7fe41f10eb16e8d111a2fg3f3a2e3f5af15a4f4g3f208a"
  "155a2e2f4g52fc136a18f2c29f10eb16e6d114afg2fc3a2e3f18a2c3f5g3f99ab108a"
  "155a2e2f4g43f2e8fc135a19f2c27fg11eb14e7d110af4afg2f2c3a2e2f16a2c4f6g3f98ab2ab105a"
  "156aef4g44f3e8f2c134a16fcf5c9b16fb26e5d117a5f2c3a3e11af3a2c4f6g4f21af75ab24ad83a"
  "130a2e3ae20aef4g44f4e8f2c133a16fc2f3c2b8f2b13fgb6eg18e5d118a5f5c2a2e10ae2fec5f5g5f15af5afaf74ab89ad17a"
  "155a2ef4g46f2e9f2c126af5a17fcfc3b11fb12fgc3ec22e3d117af2a2fg4f4c13a9f3g9f12a2f6a2f182a"
  "155a3e4g58fcac131a18fcb14fb11f2g26e2d122a2f2gfc2f4caf9ae7f3g5fc3f5a9f8a2f181a"
  "155a3e4g58fcb125af6a18fcb26f2g5eb8eg11ed125a2fg2fcf3c12ae15fc2f5af16af5a2f175a"
  "155a4e3g58f2bf3c128a15f2c2fb26fbf2ec5b18e123af3a2fg4f3c11ae14f2c2f5af17af8a3f170a"
  "155a4e3g32f2c24fb3f2c2a2fe122a17fc2fbc27f2e2cb2ab18e129af2g3f2caf10ae15f2c6af5af9afaf5a8f70ad96a"
  "155a4e3g58fb4fba5f2e120a17fc2f2b26fg2ecb2ab18e125af4a2fg4fc13a16f6a2fa4f19a7f33af132a"
  "156a3e3g58fb4fb9fe119a18fcb27fg4ebabec16e131a2fg4f2c2af9a14f8a4f3af8a2f9ac4f166a"
  "158ae4g54f9b14f117a16fcb2fb24fb4e2bab16e133a2fg4fc3af9a6f2cfc3fe7afg2f5a2fa3f8af3a4c2f7a2f18ab2fb134a"
  "158a2e4g43f6b4fb2fb4fb18fe113a16fb29f5e2b16e134a2fg4f3ca2f2af5a5f6c2e7afg3f28af3a9f15a3b4afa2f126a"
  "159ae4g43fb2c3f5b3fb4fb6fe7fe3f2e2f111ac12f2b29f16eg6e135a2fgfef2cfc13a8c2e7a3fa2f24a5f2a13f22a2f124a"
  "159aed4g39f4bc6fb10fb4fe7f4e7fe110a2e10fb30fb22e137af2ge2f2c19a2c3e6a3f2a2f9a3fa5f4a25f20af6af116a"
  "159a2d4g41f3c6fb14f3e6f4e8f2e13ae96ae7f3b28f2ebf21e36ae101a2fg2e2fc19ac2e9aefa2fe17a2f5afa3f8g5f2c5f18af6a2f114a"
  "160ad5g61fb3f4e5f3e10f2e109ae27fefe5f3efeb21e139a2fg2e2f10a4b17afe2afe29a6f6g3f5c3f13a2faf6a2f113a"
  "160a2d5g61fb3f2e2fe3f6e8f3e108ae26f3e5f2ef2eb9ef10e2ae139afg3ec31a2e2a2e31a3c5f4g9f13af9a2faf110a"
  "161ad5g67f3e5fe2f2e7f7e105ae3b21fef2efefe3f2efeb11ef7e143a3f2e31a2e5af32a4c4f4g7f8afa3f13a2fa2f105a"
  "161a2d5g67fe7fef2e6f9e105ab5fe16f3ef2ef3ef5e2b19eae142a4f36af23af2af8a6c3f2g10f2a7f18a2faf101a"
  "161a3d5g67fe6f4e5f11e105a10f6e5f9ef6efeb13ef6e211af8a8c6f2g2f2ef3a4f14afa2f5af100a"
  "162a2d6g66fe6f4e5f11e107a8fe2f2e2fef3ef9ef7eb10ef10e146af4e6a3fa3f4af40af9a7c7f3g2f26af2a2faf99a"
  "164ad7g65fe6f5e3f12e106a4fe2f21ef6ef3eb20e145a4f3e4f5e59a7c8f2g2f131a"
  "165ad7g64f2e4f6ef14e107a3f2ef2e2fe2f5ef20eb13ef6e148a12f2e3f38af15a7c10f2gf32a2f96a"
  "166ad7g64f2e3f22e106a4fe2f2e2f8ef2efef4efef4ef3eb20e155afe7fae22aea2e6af3af14a9c11fgf129a"
  "167ad7g63f2e3f22e107a3f3ef6efef5ef6efecf29e161a3fa2eaeaea3ea7eaeae33a6c4f4a3fg2fb4af121a"
  "168ad7g62f3e2f22e46ae60a13ef4e2f2ef5ef8e2f6ef16e167aea3e15a5e30a3c4f7a4f2b7af16ae99a"
  "169ad7g61f3e2f21e108a40ef2efe2f15ec129ae55a4e38a3bab5ae3f2bf121a"
  "35ae134ad7g61f3ef21e109a8ef12ef7ef2ef14eb14e174a2e8a3e40a3bab7a4f2af119a"
  "171ad7g56fe3f25e109a7e2f14e2f3e2fef7e2f7eb9ef3ec175a2e7ae40afa3b11a3f2b118a"
  "172ad7g28f2e25f2e2f19ef4e111a17ef3ef8ef4ef4e3f4eb14e180ae2ae43a3b14a3bab115a"
  "173ad9g25f3efe16f2e5fe3f17e2f4e112ad16ef6efefef8ebcef6eb14e225af4b16a5b112a"
  "173ad11g23f6e15f3e4f2e2f17e3f3e112a2d5e2fef4e2f9ef2ef5ef2e2c3e2f2e2b13e6ae191a3fa2f21a2f3b20abf111a"
  "174ad12g22f7e13f3e4f22e3f3e111a3d4e3f4ef2ef7ef3ef7ef4ef5eb13ec7ae10ae184a4f17a4f2b132a"
  "175ad13g22f6e9fe2f4e3f22e3f3e112a3d11ef5efef3ef6ef6e2f6eb12ec9ae8a2e179af3e9faf11a4fb132a"
  "176ad14g22f5e6f10e2f22e4fae112a4d3ef5ef11ef3ef9ef8eb7ef6e17ae2f178a2f3e11f10a3fef2b54ae75a"
  "177a2d15g20f44e5f113a5d6ef2efef14ef6ef4ef5eb13e15aeae3f177a3f9e3f12ae4f2b94af34a"
  "179ad9gh6g19f15e3f25e5fe111a8d4e2fef16efefc2ef25e17ae2gf169a4e4a2f9e4f12af3e2f3b50af43af32a"
  "180a2d9ghgh4g18f14e5f22e6fe111a8d11ef3ef8ef3ec6ef21e15a3eg2f25ae142a5e4af10e3faf11a2fe5fb49a2f45af29a"
  "182ad13gh4g16f13e6f22e5f2e111a8d7efef8e2f2ef7e2f13eb10e15a3e4f166a8e4f2e3d4e3f13a2f2e2fe2fb49a2f2ae71a"
  "183a2d13gh4g13f14e6f22e5f2e111a10d7e2f6ef39e13a3e5f166a7e5fe7d3ef14af4efe2fb124a"
  "185ad10g2b6g12fe2f11e6f23e4f3e111a11d22e3c5ef22e13a3e4faf165a7e3f3e9d3ef12af7efb51aeae70a"
  "186a4d7g2b2gh3g16f9e6f23e4f3e111a10de3d47e2c13a4e5f166a6d5e12def2ef11af6efeb123a"
  "190a3d15g7fefefc2fe5c3e5f24e3f4e72ae38a14d45ec13a6e2fg3f162a2e8d3e13d3ef3e9af7efeb52ae18a2f49a"
  "193a2d14g4fe2f2e3fcf6c4e3f25e2f4e111a16d42ec14a6e6f162a28d5e2f7af6ef3e122a"
  "20af174a2d13ge3f2efef2e3fe5c31e2f4e112a14de2d39ec15a5e3fg3f162a30d6ef5af7e2fe54ae15af51a"
  "197ad12g2efefe2f5e2f6c37e111a18d38ec15aef3e3fg3f162a2e30d5e2f2a2f10ef69a2f50a"
  "198a2d11g2e3f2ef2e2f2ef5c37e112a15de3d35ec17aef4efe2fe162a2e31d22e42ae5ae5ae65a"
  "199a2d10ge2fe3f2ef2ef3e4c27eg9e113a19d3e2c28e2c18aef8e162a2d2e33d8e6d3efe42a2e5ae69a"
  "199a2d11ge2fe6f5e5c26eg9e114a19d2e3c27ec19aef2efe2fefe160ae2d2e49d2efe43a2e73a"
  "200a2d10g3ef3e2fef2ef2e5c23e2g9e116a19d2e3c27e20aefe3fg2fe159a3de26d2e12d2e10d3efe43a2e71a"
  "200a2d11g2efefe3fef2efe2c26e2g9e118a18d7e5d19e21a2e2f2g2fe20ae134ae58ded3e2f2e113a"
  "201a2d10g4e4f2e2f4e2c26eg9e119a20d2e8d19e20a2e4fgfe17ae134a2e7d2e54d5efe112a"
  "201a3d9g4efefe2fef5ec16ef19e120a31d17ec20ae4fgfe150a2e6de52de6d5e2fe77ae32a"
  "202a2d9g4e4f2ef3ef17e3f19e121a31d17e20ae4fgfe150a2e4de53d2e4d2e3d6e109a"
  "202a3d9g6e3f3ef17e6f16e123a30d18e19ae4fg2fe146aea3e10de31de27d4efe108a"
  "203a2d9g8ef10ec10e6f15e125a30d17e19ae5fgfe146a5e10de29de28d2ed4efe106a"
  "203a2d10g10ef5e3c10e6f15e126a29d18e19a5fgfe147a4e3de12de47dede4de2d3efe105a"
  "203a3d10g16e3c9e6f10e132a29d17e19ae4fgfe147a4e16d2e46d3e4d2e3d3ef104a"
  "203a3d10g16e3c9e6f8e134a29d18e19a4fgfe147a4e43d2e13de18defef103a"
  "204a2d10g17e2c9e5f7e137a28d13ec4e19ae5fe147a5e8de66ded2e2f102a"
  "204a3d9g17e2c9e6f5e138a27d14e3c2e20ae3f2e148a6e63de6dede3d4ef100a"
  "205a2d9g29e5f4e140a26d14e2c24aefe152a6e60d2e6dede5d2efef98a"
  "205a4d4gh2g30e4f4e141a25d15e181a2ed4e66de9d4ef96a"
  "205a4d7g38e142a19de4d15e178aeaea4ed2ed2e71d2efe3f95a"
  "205a3d2e6g39e142a20ded17e178aea6ede8de59d2e4d3ef2ef95a"
  "120ae85a2d4e4g40e141a20d19e179a11e58de7d2e4d4ef3e94a"
  "206a3d3e4g40e142a18d20e181a10e66de4d3e3f2e94a"
  "206a3d2e3g43e142a17d21e181a5e2f3e4de64d5e2f3e93a"
  "207a2d2e3g43e143a16d21e182a10e3d2e4de58d4e4fef93a"
  "207a2d2ef3gf11ec29e144a12de4d10e2g7e183a6efede16de29de21d4e2f2efe92a"
  "207a3de2f2gf9ec2ec28e146a10de2d13e3g5e184a10e14d2e51d2ef2e2fefe32ae59a"
  "208a2d2e2f3g11ec28e147ae5d21e2g4e185a5ef3ede5d3e38de17d6e2f2ef92a"
  "208a2d3ef4g8e2c28e149a27eg3e187a9e34de11de9de7d5efef3ef92a"
  "194ae14ad4ef3g9e2c24eb2e150a25e2g3e188a4ef5e3de33d2e3de2d2e6de6d8efef3e92a"
  "209a2d5e2g9ecbc23eb2e151a23e3g3e189a3e2f5e4d3e27de10de14d6e2fefefe92a"
  "209a2d5e2g9e3c25e153a21e2g4e190a4e2f5e45de14d5ef4efe92a"
  "210ad3e4g10e4c22e155a25e192a3e3f4ede17d7a4d5e2de10de2d2e5d10ef3e92a"
  "210ad3e3gf36e156a23e193a4e3f3e16d17a6e8de3de4dc11efefe92a"
  "210a4e2fg37e156a22e195a9e4de8d21a5eae2de7de3d2c13efe92a"
  "210a4efeg37e157a20e196a12e8d25a3e2ae11de3d10e2f3e92a"
  "211a3e2f2g36e157a18e198a14e6d26a2e2a3e12d2cec6e3f3e92a"
  "211a3e2f2g23ea11e160a7e207a10e37ae3a4e8de5c2ef3e3f3e92a"
  "211a3e2f3g23e3a7e164ae212a7e38ae3aea2ede10d3c6ef2efe92a"
  "211a3e3f2gf23e435aea2e3de3de5d11efe49ae42a"
  "212a2eg3fgf24ef432a2ea6e11d3e4f4e50a2e40a"
  "212a2e2g2f2gf23ef438a6ed9e2fgfg3fe51a2e39a"
  "212a3eg2f2gf23efc432a3e3aec12e2fg6fe53aeae36a"
  "212a4e3fgf24efcf437aec11e2fg5f2e54aeae35a"
  "212a4e3fg2f2ed21e2f97ae340aec6e2f2efg5f2efe54a3e34a"
  "212a3e2g2fg2f2ed23e439a12e3f7e55a4e3ae28a"
  "213a3eg2fg2f2e2d22e444aec3ea6e60a9e27a"
  "213a4e3f2gfe5d18e448ae4a2e63a2e4fe28a"
  "213a4eg2f2gf2e4d15e521a2ebgf2e28a"
  "213a5e3f2gfe4d10e525a3efg2e29a"
  "214a3e2g2f2gf2e4d10e458ae8ae57a3ef2e29a"
  "214a4e2gf2gf2e5d9e527a3e30a"
  "214a4e5g2f2e5d2e2a5e459af4e2a2e53af5a2e30a"
  "215a4f5gf2e5d2e467a4f3efe52a2fafea2e30a"
  "215af2af5g2f2e5d2e467a4f2efe52a3f2e33a"
  "215af2af5g2f2e5d2e469a3fe2fe51af2ge34a"
  "216af2af4g3fe6d5e466a4f2e50aefg2e34a"
  "216af2af4g3f2e5d2e2ae467afbfe51af2ge35a"
  "219af5g3fe5d4e469a2f2e49aefgf2e34a"
  "217afaf5g3fe4d4e523af2g2e35a"
  "218afaf5g2f2e2d6e237a3e280a4gf2e35a"
  "218afaf5g2f2e2d4e240a2e279a5gf2e35a"
  "221af4g2f2e2d2e522a5g2f2e35a"
  "219af2a5gf2e3de521a5g2f2e36a"
  "219afa2f4gf2e3d2e520a3g5e37a"
  "218a3fa5g2f2e2d4e523a2e38a"
  "219a2fgh4g2f2e4d4e520ae40a"
  "221agh4g2f2e5d3e306a2e253a"
  "220afa5g2f2e5d3e306a2e253a"
  "220afaf3g4f2e4d3e17ae2a2e539a"
  "221afaghg4f2e3d3e18ae3a2e538a"
  "223af3g4f2e2d3e562a"
  "221a2faghg4f2e2d3e562a"
  "222afaf2g5f6e562a"
  "223afafg5f6e562a"
  "225af3g4f5e562a"
  "223a2fa2fbg4f4e562a"
  "226af2bg4f5e561a"
  "225af2afgb4f2e563a"
  "226af2agbg2b2f2ae561a"
  "228af2agb2f2a2f2e559a"
  "229a2f2a2faf3afe69agh487a"
  "232ag3agfa2f3e69a2h485a"
  "231afaf2a3f2af4e69ah484a"
  "232af2a7g2f5e551a"
  "237a2g7ae553a"
  "240a3fa2f554a"
  "243af556a"
  "800a"
  "800a"
  "800a"
  "251a2g547a"
  "249a4g547a"
  "249aghg548a"
  "249aghg548a"
  "249aghg548a"
  "249aghg548a"
  "249ag2hg547a"
  "249ag2hg547a"
  "249ag2hg423ah123a"
  "249ag2hg155ah8ahah3ahah9a2h200ah25ahah4ahah3ah35ah87a"
  "240h8ahg2hg2hah5a2h3ahahah3aha4h3a2h3a2h2ah4ah3ah3ahah2ah4ah2ahaha3hahahah2aha2hah2ahaha4ha4ha2h3a4hah2a2ha3h2a6h3a4ha7h4aha5hahaha3ha3ha5h4a2ha5h2a2ha3ha2h5aha2hah6ah4a3hah3ah2a3h6ah6ah3ah4ah7a3hah4ah9a3h8ahah5a2h10ah10ah3a2h6aha2hah2ahaha2h4ah3a3h9ah9ah5ah5a2h4ah2ahah3ah3a3h3aha3h3a2h2ah2a2h3ah5ahah4ah7ah2a2h4a3h2aha4hah3ah3ah3ah8a47h"
  "240hahah2a6h2g2hah2a2hah7ahahah5a5h3a3ha3h3ahaha2ha4h2a6h2ah2ah4a5haha2ha31h5a6ha2ha3h3a9ha48ha8ha9h3a10hah2a5hah2a4h3a2h3ah3a2h2a2h6a3h7a3h4a3h2a2h5ahah3a9h2ah2ah2a3h2a6ha6h2ah2a6h2aha3h6a3h5a2hahah3ah4ah5a2h4ah10aha9ha5h5a2h4a5h6ah7a2h4a3h7ah10ah16a48h"
  "240h2ah5a8ha3hah4ahah2ahah3ahaha5h2a5h2a2ha6ha2hah2a2h6a8h2a171ha35h2a18ha75ha14h3a7ha42h2a18h2a7hah2a5h5ah6ah3a46h"
  "241h2ah2a10ha2h3a2hah4a2h2aha2hah2aha3h3a19h2a8ha2ha2ha415h11aha46h"
  "247ha10ha8h2aha7h2a4h3ah2a452h9a49h"
  "246ha14h2a6h2ah2ah8ah2aha460h3a49h"
  "272h2ah2aha2ha4ha513h"
  "273ha526h"
  "279ha520h"
  "800h"
  "710hg89h"
  "800h"
  "800h"
  "765hg34h"
  "800h"
  ))

;; First 60 should be largest features

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
   ( 167 "desert" "Arabian")
   ( 168 "desert" "Atacama")
   ( 169 "desert" "Chihuahuan")
   ( 170 "desert" "Death Valley")
   ( 171 "desert" "Gibson")
   ( 172 "feature" "172")
   ( 173 "lake" "Great Salt %T")
   ( 174 "river" "Yukon %T")
   ( 175 "river" "Mackenzie %T")
   ( 176 "river" "Amazon %T")
   ( 177 "river" "Congo %T")
   ( 178 "lake" "%T Rudolf")
   ( 179 "lake" "ozero Zajsan")
   ( 180 "sea" "Sulu %T")
   ( 181 "island" "Cebu")
   ( 182 "sea" "Celebes %T")
   ( 183 "sea" "Java %T")
   ( 184 "island" "Flores")
   ( 185 "island" "Sumbawa")
   ( 186 "island" "Sumba")
   ( 187 "island" "Halmahera")
   ( 188 "island" "Ceram")
   ( 189 "sea" "Banda %T")
   ( 190 "sea" "Arafura %T")
   ( 191 "island" "Wake %T")
   ( 192 "island" "Kiritimati")
  )
  "800a"
  "125b5c40*71,66ba160ba43b55g304b"
  "125b6c40*71,267b58g31b2g271b"
  "125b7cb38*71,261b2g5b56g31b5g269b"
  "126b7c39*71,237b5g5bg11b6g2b55g13b7g4b15g31b5g231b"
  "127b8c37*71,235b8g4b2g6bg3b7g2b51g5bg3b5gb30g29b10g225b"
  "128b8c37*71,234b9g4bgb2gb14g2b47g4b46g27b15g125b3h92b"
  "41b3h86b7c36*71,133b3f2bfb2f92b9g4b19g2b98g8b2g3b2g12b14gb7g112b11h89b"
  "42b3h86b6cb36*71,129b22f82b10g3b19g2b101gbg3b9gb2g6b24g107b22h82b"
  "43b3h86b7c35*71,119b32f82b10g4b19gb168g87b30h74b"
  "43b3h87b6c36*71,115b39f79b10g3b19gb170g83b36h41b3h26b"
  "43b4h20b73c35*71,74c40y45f74b204g18b2g2b11g48b51h25b19h12b"
  "45b3h19bh73c33*71,76c39y51f52b4g13b203g15b3g5b17g40b58h14b34h2b"
  "2h17b3h24b3h9bh8b3h71c31*71,78c39y56f10ba8b4a6b5g13b8g10b222g4b22g30b68h175,43h"
  "2h20b2h14b2h6b6h6b2h7b6h68c31*71,79c37y59f9b4a5b3a4b7g6b21g5b256g21b69h3*175,40h"
  "18h2b20hbhbh2b6h7b2h6b8h67c30*71,79c36y61f8b5a10b9gb28g3b255gb7g17b68h3*175,37h"
  "20hb20hb3hb7h6b3h4b9h68c28*71,81c35y61f9b3a10b41gb266g15b70h6*175,24hA6h"
  "21hb33h4b4hb10h70c27*71,81c35y45f8b7f10b2a11b311g18b70h2*175,14h9A6h"
  "73h2t69c28*71,81c33y50f8b2f6b2fg4ba10b313g8b5h4b71h4*175,17h2AhA4h"
  "71h6t67c29*71,80c33y23f6z24f12b4f5g9b291gp16g10p33h3*174,52h3*175,12h10A"
  "68h11t66c28*71,81c31y24f7z26f3b2f3b5f8g4b294g5p10g13p27h5*174,57h5*175,6h2A4h5A"
  "68h15t63c27*71,81c31y23f6z29f2b10f300g13p7g16p19h7*174,71h2A8h2A"
  "66h20t61c27*71,81c28y25f5z31f2b10f303g13p3g19p16h174,89h"
  "61h28t60c26*71,80c28y24f6z43f305g41p8h174,89h"
  "59h32t59c24*71,82c27y23f6z44f306g40p8h174,89h"
  "11hC6h2C37h36t59c23*71,81c26y23f7z44f301g42p11h174,89h"
  "12h2C2h4C36h37t5h55c21*71,83c25y22f9z44f270g3r25g42p13h174,89h"
  "13h5C37h39t8h53c19*71,83c25y22f9z44f257g6r6g2r25g42ph174,13h174,89h"
  "9hC2h4C39h39t10h53c15*71,86c24y23f9z43f255g8r4g3r24g44p2h8*174,4h2*174,89h"
  "10h5C40h40t11h52c14*71,36c6*72,44c25y22f9z19f3H22f252g11r2g3r22g48p9h5*174,20h2e68h"
  "55h40t16h49c11*71,36c8*72,44c24y23f7zaz18f4H21f251g12rg3r22g52p30h4e4h2e61h"
  "56h39t18h48c9*71,38c8*72,43c24y24f3z2a5z5f8z4f2H22f251g15r7g2p3g7p2g56p26h4e5h3eh2e58h"
  "56h40t17h10ch41c4*71,40c7*72,44c24y24f26zfH23f250g14r7g3p2g9pg62p19h4e7heh7e54h"
  "57h38t19h9c2h87c4*72,43c2*70,24y4f7y12f13z38f250g11r10g77p18h5e4h20ehe44h"
  "58h37t21h6c4h86c4*72,40c70,c3*70,26y2f7y11f10z2az40f225g20r5g8r12g78p4hp3hp9h4e2h31e37h"
  "59h36t22h4c6h129c70,c6*70,32y11f10z2a2z39f223g21r7g6r13g89p8h39e34h"
  "61h35t23hc8h130c6*70,33y11f15z38f220g36r14g88p7h3e2*133,36eh2e30h"
  "62h36t31h130c5*70,29y2f3y10f2z2a6z2f3z38f218g38r14g86p8h3e2*133,39ehe29h"
  "63ht3h32t32h129c5*70,28y2f4y8f3z2a6z3f3z37f216g40r14g85p8h3e3*133,40ehe28h"
  "72h28t32h122c3*73,3c5*70,27y4f4y5f13z43f154g@59g42r15g82p8h6e133,42ehe27h"
  "75h25t34h120c4*73,2c4*70,27y8fy3f15z43f154g@57g44r15g81p7h52eh3e24h"
  "77h22t37h117c5*73,3c7*70,23y12f15z44f153g@56g45r15g79p8h55e2he23h"
  "81h17t40h115c7*73,3c6*70,23y3f2z2f18z45f152g2@58g2rg41r14g8ea69p3h62e2he22h"
  "91h5t43h114c7*73,5c5*70,23y4f20z45f152g2@60g42r12g11ea67p2h65eh2e21h"
  "92h5t47h109c7*73,6c6*70,21y4f14z3f4z45f150g3@61g2rg3r3g3r148,29r10g16e62p2*112,69eh2e19h"
  "92h6t48h108c6*73,7c6*70,21y4f3z4f4z54f149g4@62grg2r6g2r148,28r9g22e52p112,e112,e112,75e18h"
  "55h3E35h6t50h105c6*73,7c7*70,19y70f147g6@73gr2*148,28r7g28e5pa40p2*112,75e3h2e17h"
  "55h4E35h5t51h104c6*73,5c12*70,9y78f145g6@74gs2*148,29r6g35ea25p112,4e112,2e112,e2*112,e112,77e3h3e15h"
  "57h2E36h5t50h106c2*73,8c11*70,8yfy77f145g3@78gs2*148,28r4g41e22p92e3h3e14h"
  "58h2E37h3t50h106c2*73,9c10*70,7y80f143g4@79gs3*148,28r3g50e112,3p112,2p2e112,97e2h4e12h"
  "60hE37h2t50h119c8*70,7y81f225g2s3*148,28r2g165e10h"
  "61hE37h2t48h2c76,114c13*70,4y82f225g3s3*148,28rg168e7h"
  "61hE85h3c2*76,113c12*70,4y85f225g2s4*148,28r172e3h"
  "e61hE83h4c2*76,113c3*70,10c88f225g3s3*148,27ra167e5*155,3e"
  "155,2e125h21c3*76,c76,111c2*70,11c88f225g3s2*148,r148,26ra169e6*155,"
  "3*155,2e122h9c2V11c3*76,c3*76,119cf2c89f224g4s2*148,26r174e3*155,"
  "4*155,e120h13c2V9c12*76,111c2f2c91f225g3s2*148,25ra177e"
  "7e75h5:47h15c12*76,111c96f224g4s148,25r178e"
  "3e4he73h7:48h13c13*76,112c94f98g2*179,124g4s148,25r178e"
  "4e4he71h12:43h13c12*76,c76,c76,113c91f44g3<53g179,123g5s148,23ra178e"
  "5e3he70h5:h8:43h21c76,3c3*76,114c91f43g4<175g6s2*148,21r180e"
  "6e72h4:4h7:44hc77,5c2h10c2*76,4c2*76,115c87f3w25g5~13g5<29g11G134g7s2*148,18ra181e"
  "7e71h:11h4:44hc3*77,2c2h134c70f3w9f5w24g9~13g5<28gG143g8s148,r148,13r2a184e"
  "9e85h:h3=40h4c5h133c68f9w2f7w24g8~15g4<28gG143g10s12r2a186e"
  "8e82h5>8=34h2c7h135c31fo36f7w5f5w25g7~16g5<28gG141g10s4*127,7r2a188e"
  "8e82h5>2h5=h2=27h6c6h138c30f2ofo33f9w6fw25g8~17g4<169g11s7*127,r3a190e"
  "9e81h5>3h5=h=25h7c6h140c29f5o31f11w2f9w20g10~16g2<169g12s8*127,193e"
  "9e81h4>5h4=7h6F12h9c5h141c30f5o30f24w18g11~185g11s8*127,195e"
  "9e82h3>5h5=5h7F12h10c3h127c10f5c19f6o7f5o29f25w17g12~183g12s7*127,195e"
  "10e81h3>7h2=25h137c29f2o3f11o7f4o27f28w17g10~180g15s127,s127,3e127,195e"
  "10e81h4>6h=4h5D18h137c28f11o105,6o6f5o25f29w18g9~g2~172g20s127,s199e"
  "11e81h3>9h5D20h137c28f11o2*105,6o6f9o20f29w18g10~g2~170g23s2*128,197e"
  "11e81h3>7h6D21hch136c28f11o105,7o6f10o19f8w10g10w19g13~168g24s3*128,196e"
  "12e24h173,56h2>36h136c28f20o8f9o18f3w17g7w20g10~149g3u17g24s4*128,196e"
  "12e25h173,85h144c26f13o2*110,10o5f9o10f5o5f46g10~147g5u16g24s5*128,195e"
  "13e24h2*173,88h141c22f16o3*110,10o8f6o11f2oao2f3o46g10~145g6u15g25s5*128,195e"
  "14e110h144c21f8o103,9o2*110,12o4f2o2f4o7f2of10o48g9~143g7u15g25s5*128,195e"
  "15e110h144c20f7o103,11o2*110,14o2f3of4o7f4oa2o52g11~142g6u5g4u6g24s128,s5*128,195e"
  "16e106hc2h144c21f3o103,14o2*110,15o2f9o6f8o50g11~141g7u2g7u7g21s128,3s5*128,195e"
  "16e105hchch144c21f36of8o104,o6f2oa3oao50g11~140g7ug9u7g20s128,3s5*128,195e"
  "17e102hchc2h104cXcX40c18f37of11o7f6o50g11~141g16u8g20s8*128,195e"
  "18e102hchch110cX36c18f31o7f15ofofoa3o59g3~144g16u7g18s9*128,194e"
  "19e102h2ch148c16f33o5f14o4f8o205g2u2g12u7g16s10*128,194e"
  "20e103h149c3f3c9f24o4joj6o4f14o3f4oa3o213g8u7g15s12*128,193e"
  "21e103h155c3f14o21j9of16o2f19o5g5o190g10u7g8s16*128,e128,193e"
  "22e102h156c2f13o22j38o109,18o189g11u7g7s12*128,e3*128,2e128,193e"
  "23e102h155c13o25j36oa19o188g13u7g4s14*128,200e"
  "25e101h154c2j8o28j9o108,19o6*106,22o186g14u7g4s5*128,5e3*128,201e"
  "26e100h154c38j51o2*107,4o187g14u3g3u4s130,6e2*132,e3*128,201e"
  "31e94h154c39j57o188g20u2s4*130,2e5*132,e2*128,201e"
  "27eheh4e88h156c41j56o189g20u6*130,e5*132,204e"
  "31eh3e86h157c44j53o190g20u6*130,e2*132,e132,204e"
  "36e84h138c74,18c47j19o4j28o191g19u130,u4*130,2e132,206e"
  "36e84h157c52j13o7j26o192g19u130,u3*130,209e"
  "37e82h158c55j10o8j25o191gu2g19u3*130,209e"
  "38e3h3}74h159c56j10o11j21o192g21u3*130,209e"
  "39e3h4}72h160c56j10o16j5o5j4o196g19ue130,209e"
  "40e2h5}72h141c64,17c61j5o27j199g19u211e"
  "41e2h5}54h6q11h147c64,11c64j2o28j196g21ua210e"
  "36ea5e3h4}43h3q6h9q2h2q6h145c64,3c64,9c93jx36g2|157gu3g17u211e"
  "43e4h4}37h9q4h16q4h157c95jx5gx29g3|160g17u211e"
  "45e3h5}34h14q2h15q4h155c97jx4gx30g4|158g17u211e"
  "44eh2e3h4}33h32q4h153c99jx3gx31g5|156g17ua210e"
  "47e4h4}32h33q4h149c103jx2gx32g6|154g17u211e"
  "46e7h4}30h33q5h148c104j6x30g7|151g18u211e"
  "51e4h3}29h33q6h147c105j5x32g7|149g18u211e"
  "53e3h4}28h34q4h146c107j6x32g8|2g2|4d138g17u131,211e"
  "55e2h4}27h35q4h145c108j6x33g10|g4d136g18u131,211e"
  "56e2h5}26h35q3h145c109j6x32g|a|g6|g6d135g16u213e"
  "56e3h5}25h36q2h145c110j6x32g2|2g4|3g23d117gm3u152,11u213e"
  "57e3h5}24h35qhq145c112j6x33g|2g3|4g23d115g4m3*152,e9u33e129,179e"
  "58e4h5}23h37q144c113j6x35g3|5g23d114g4m3*152,4e2uaua214e"
  "60e3h5}22h37q144c115j5x44g23d111g5m3*152,223e"
  "62e2h6}21h37q143c115j6x49g20d107g6m3*152,223e"
  "63eh7e20h33q4*84,144c115j6x49g22d44gd58g8m3*152,180e2*126,40e"
  "72e20h30q9*84,141c118j5x48g19d47gd50gm3g12m2*152,180e2*126,40e"
  "73e19h29q2*84,4n8*84,137c119j5x47g20d8gd33gd4g3d48g16m152,222e"
  "74e19h26q6n84,5n7*84,135c119j5x46g22d6gd32g10d43g21m222e"
  "74e20h15q8h17n5*84,134c120j5x45g24d4g2d30g12d35g3mg25m188e3*125,30e"
  "63ea11e21h13q8h18n7*84,131c120j7x43g30d29g14d33g5mg25ma186e4*125,29e"
  "76e21h12q9h21n5*84,130c120j8x40g32d26g18d30g32m188e4*125,28e"
  "78e20h12q8h20n8*84,128c120j9x37g34d25g20d29g5m5g23m188e3*125,28e"
  "80e19h11q8h30n9*88,118c120j9x35g36d23g23d27g5m5g22ma101e191,117e"
  "81e20h9q8h32n9*88,11ca105c120j9x34g36d23g24d27g4m5g24m218e"
  "82e20h8q9h20n4*89,9n9*88,3n4*91,na2n104c121j9x32g38d22g25d26g5m4g24m218e"
  "84e19h3q13h21n4*89,4n8*88,16na104c122j8x31g39d21g25d27g6mg24m3*141,216e"
  "87e32h36n88,18n86c66,c66,15c122j8x29g40d21g26d27g30m4*141,215e"
  "89e31h54nan82,101c123j8x26g43d19g27d29g29m3*141,215e"
  "92e28h58n87c66,12c123j10x22g46d17g28d6gd24g27m4*141,214e"
  "95e24h59n87,99c124j9x19g50d16g29d4g3d24g25m141,m3*141,214e"
  "99e5h3e21h52n86,82c66,c66,13c124j9x18g50d16g37d25g24m3*141,215e"
  "109e21h51n98c125j9x17g51d15g38d25g24m3*141,54ea159e"
  "111e20h50n90,97c127j8x14g54d14g39d25g23m4*141,213e"
  "113e19h50n96c130j7x11g56d15g38d25g24m141,a2*141,212e"
  "114e19h49na97c66j2I61j6x9g59d14g39d25g24ma3*141,2e141,208e"
  "117e16h50n98c66j2I63j4x7g17d2a42d14g39d25g22m4a6*141,ea43e124,160e"
  "120e4he8h49na3c83,94c67j4I62j2x5g64d13g40d3g2m19g24m2*143,3a141,2a141,205e"
  "125e9h48n100c67j2I65jx3g66d13g25da13d4gm20g24m2*143,6a141,e147,202e"
  "126e8h26ni21na99c135j69d13g40d3g6m15g23ma180,143,8a2*147,201e"
  "128e7h23n3i23n100c133j16dj53d12g26da13d3g7m14g23m5*180,7a2*147,200e"
  "129e6h19n8i3ni19n98ca4c130j11d6j53d11g41d2g8m13g23m145,5*180,2*146,3a181,2a147,199e"
  "130e6h18n8iJ11i12na103c129j7d9j53d12g26da12dad2g8m11g24m145,5*180,2*146,a144,a181,3a199e"
  "131e6h17n8iJ14i2n5ina105c144j54d11g41d2g11m7g25m145,6*180,146,a144,a181,3a199e"
  "132ehe4h17n7i2J19i2n2a106c143j54d10g41d3g9ma2m4g26m145,7*180,a144,a181,4a198e"
  "135e5h15n8i2J20i2n106c143j55d7g3d2*101,39d3g12m2g26m2*145,7*180,2*144,a181,4a28e115,169e"
  "137e4h5n4h4n10iJ24i105c142j56d6g3d3*101,38d3g12m2g26m145,9*180,2*144,5a142,197e"
  "139e13h2n36i105c141j57d4g5d4*101,37d3g12mg26m145,11*180,4a3*142,196e"
  "141e7h2e3hn38i103c141j58d2g6d5*101,36d4g39m12*180,142,a5*142,195e"
  "143e4h5e2h40i102c140j67d5*101,24da13d2g38m10*180,9*142,195e"
  "145e3h5eh42i101c139j67d5*101,39d2g37m158,9*180,142,182,142,182,6*142,194e"
  "154e44i100c138j68d5*101,25da13d2g35m3*158,9*180,4*182,4*142,e142,70e115,122e"
  "155e45i100c136j69d3*101,41d3g32m5*158,8*180,a4*182,3*142,195e"
  "155e53i94c29j7c98j114d4g3ma26m5*158,6*180,a9*182,142,194e"
  "155e55i94c24j11c97j103d2*165,10d5g27m8*158,4*180,11*182,194e"
  "155e56i95c18j16c96j104d5*165,6d7g26m9*158,182,a11*182,194e"
  "156e57i95c5j29c94j105d6*165,5d7g23m10*158,15*182,193e"
  "156e58i136c59j178,25j108d6*165,4d6g23m9*158,16*182,3ea189e"
  "156e58i134ca2c59j178,24j110d5*165,4d6g9ma12m9*158,16*182,a192e"
  "156e59i136c59j178,24j111d6*165,3d5g5ma15m11*158,16*182,192e"
  "155e61i136c59j178,22j114d5*165,2da5g18m13*158,16*182,192e"
  "155e62i135c60j178,21j110da4d6*165,3a4g16m15*158,15*182,192e"
  "156e63i134c23j9*177,48j117d7*165,3a3g11m158,3m16*158,15*182,6e187,100ea83e"
  "130e2*122,3e122,20e64i133c22j2*177,8j2*177,45j118d10*165,2a2g10m20*158,15*182,159,5e187,e187,164e192,17e"
  "155e66i126ca5c20j3*177,11j177,43j117da2d11*165,a12m21*158,2*183,10*182,2*159,6e2*187,182e"
  "155e66ica131c19j177,14j177,41j122d12*165,ma9m20*158,5*183,9*159,5a3e2*187,181e"
  "155e66i176,125ca6c19j177,33j;21j125d10*165,12m19*158,5*183,159,16a187,a4e2a175e"
  "155e65i2*176,4i128c19j177,31j5;18j123da3d10*165,11m19*158,5*183,159,17a187,5a3e3*138,170e"
  "155e65i176,5i2c3i122c20j177,32j4;18j129d8*165,ma10m18*158,6*183,159,5a159,11a187,5a8*138,70e136,96e"
  "155e65i176,4i2*176,7i120c20j177,31j4;18j125da4d8*165,12m183,16*158,6*183,2*159,a4*159,19a7*138,166e"
  "156e64i176,4i176,10i119c19j177,32j3;18j131d9*165,2ma7m2*183,14*158,8*183,4*159,3*189,20a5*138,166e"
  "158e59i9*176,14i117c17j177,32j4;16j133d8*165,3ma6m3*183,14*158,7*183,4*159,5*189,18a6*138,7e2*138,156e"
  "158e49i6*176,4i176,27i113c16j177,34j2;16j134d10*165,m2a2ma5*183,13*158,7*183,5*159,10*189,18a138,3e9*138,24e2*164,126e"
  "159e48i176,5i5*176,31i111c13j2*177,52j135d10*165,13*183,10*158,7*183,3*159,189,2*159,9*189,15a5*138,2e13*138,22e2*164,124e"
  "159e45i4*176,43i110c12j177,30j?22j137d8*165,19*183,5*158,6*183,3*159,2*189,2*159,9*189,4a5*188,4a25*138,20e164,6e113,116e"
  "159e94i13c69,96c8j3*177,30j?22j36da101d8*165,19*183,3*158,9*183,2*159,189,3*159,17*189,2*188,5*189,138,189,23*138,18e164,6e2*113,114e"
  "160e95i109c41j?21j139d7*165,31*183,2*159,2*189,2*159,27*189,2*190,23*138,13e2*163,e164,6e2*113,113e"
  "160e97i108c40j?20j142d6*165,31*183,2*159,2*189,2*159,26*189,5*190,21*138,13e163,9e2*113,112e"
  "161e100i105cj3*177,36j?19j143d5*165,31*183,2*159,5*189,a25*189,7*190,19*138,8e163,e3*163,123e"
  "161e101i106c38j2?19jd102,142d4*165,32*183,4*189,a23*189,a2*189,a8*190,21*138,2e7*163,123e"
  "161e102i105c39j?20j147d33*183,31*189,a8*190,21*138,3e4*163,124e"
  "162e101i107c38j?21j146d5*160,6*183,3*160,183,3*161,4*183,a9*183,31*189,a9*190,19*138,29e2*123,101e"
  "164e100i106c39j?20j145d16*160,18*183,29*189,12*190,19*138,131e"
  "165e99i107c39j?20j148d17*160,13*183,25*189,a15*190,20*138,130e"
  "166e99i106c40j?20j155d9*160,183,157,12*183,10*189,a189,2a6*189,a3*189,a14*190,22*138,129e"
  "167e98i107c61j161d3*160,d2*157,183,a183,185,183,3*185,183,7*184,189,a189,a9*189,24*190,10*138,4e6*138,127e"
  "168e97i46cW60c62j167dad3*185,15d5*166,30*190,7*138,7e4*138,126e"
  "169e95i108c63j129da55d4*166,36*190,12e4*138,124e"
  "35e134,134e94i109c47jB14j174d2*186,8d3*166,2d37*190,13e4*138,122e"
  "171e93i109c48jB14j175d2*186,7d166,4d36*190,16e3*138,120e"
  "172e91i111c47jB14j180da2da5d37*190,138e"
  "173e90i112c47jB14j189d36*190,k137e"
  "173e90i112c47j2B13j6da182d9*190,3k190,2k21*190,2k136e"
  "174e90i111c48jB14j7da10d96,170d14*190,4k17*190,4k134e"
  "175e89i112c48jB13j9da8d2*96,170d9*190,13k190,k11*190,4k133e"
  "176e86ici112c48jB14j17d3*96,169d9*190,16k10*190,5k56e156,75e"
  "177e86i113c49jB13j15dad4*96,169d8*190,15k12*190,5k96e149,34e"
  "179e85i111c64j17d4*96,169d4k4*190,15k12*190,6k53e156,43e149,32e"
  "180e84i111c65j15d6*96,25da142d5k4*190,14k190,a11*190,8k50e2*156,45e149,29e"
  "182e83i111c64j15d7*96,166d24k13*190,9k50e2*156,2e156,71e"
  "183e82i111c65j13d8*96,166d24k14*190,9k125e"
  "185e11i2N68i111c64j13d7*96,d96,165d26k12*190,9k52e156,e156,70e"
  "186e11i2N67i111c63j13d9*96,166d28k11*190,9k124e"
  "190e77i72c78,38c60j13d12*96,162d33k9*190,10k53e156,18e2*120,49e"
  "193e74i111c59j14d12*96,162d35k7*190,11k122e"
  "20e151,174e72i112c57j15d12*96,162d37k5*190,11k54e156,15e120,51e"
  "197e71i111c57j15d12*96,162d39k2*190,13k69e2*120,50e"
  "198e70i112c55j17d11*96,162d55k42e137,11e156,65e"
  "199e69i113c54j18d10*96,162d56k42e2*137,75e"
  "199e69i114c52j19d11*96,160d58k43e2*137,73e"
  "200e67i116c51j20d10*96,159d61k43e2*137,71e"
  "200e67i118c49j21d9*96,20d97,134d68k113e"
  "201e66i119c49j20d9*96,17d99,134d72k112e"
  "201e66i120c49j20d8*96,150d76k77e154,32e"
  "202e65i121c48j20d8*96,150d77k109e"
  "202e64i123c48j19d9*96,146dkd79k108e"
  "203e62i125c47j19d9*96,146d83k106e"
  "203e62i126c47j19d8*96,147d83k105e"
  "203e57i132c46j19d8*96,147d84k104e"
  "203e55i134c47j19d7*96,147d85k103e"
  "204e52i137c46j19d7*96,147d86k102e"
  "204e51i138c46j20d6*96,148d87k100e"
  "205e49i140c42j24d3*96,152d87k98e"
  "205e49i141c40j181d88k96e"
  "205e49i142c39j178dkdkd88k95e"
  "205e50i142c39j178dkd89k95e"
  "120e119,85e50i141c39j179d91k94e"
  "206e50i142c38j181d89k94e"
  "206e51i142c38j181d89k93e"
  "207e50i143c37j182d88k93e"
  "207e50i144c36j183d88k92e"
  "207e50i146c34j184d87k92e"
  "208e49i147c33j185d86k92e"
  "208e48i149c31j187d85k92e"
  "209e47i150c30j188d84k92e"
  "209e47i151c29j189d83k92e"
  "209e46i153c27j190d83k92e"
  "210e44i155c25j192d29k7d46k92e"
  "210e44i156c23j193d26k17d39k92e"
  "210e44i156c22j195d22k21d5kd32k92e"
  "210e44i157c20j196d20k25d3k2d31k92e"
  "211e43i157c18j198d20k26d2k2d31k92e"
  "211e30ic11i160c7j2c205d10k37dk3d30k92e"
  "211e31i3c7i164cj5c207d7k38dk3dkd28k92e"
  "211e32i180c255dkd28k49e139,42e"
  "212e33i178c254d2kd28k50e2*139,40e"
  "212e33i179c259d25k51e2*139,39e"
  "212e34i179c253d3k3d24k53e139,e139,36e"
  "212e36i177c260d23k54e139,e139,35e"
  "212e36i97c81,80c260d23k54e3*139,34e"
  "212e36i178c261d22k55e4*139,3e139,28e"
  "213e35i179c265d5ke6k60e9*139,27e"
  "213e34i180c268dk4e2k63e7*139,28e"
  "213e31i184c269d68e7*139,28e"
  "213e26i189c269d67e7*139,29e"
  "214e26i189c269da8ea57e6*139,29e"
  "214e26i189c270d69e2*139,30e"
  "214e20i2c5i189c269de5*153,2e2*153,53e139,5e2*139,30e"
  "215e19i196c270de9*153,52e2*139,e2*139,e2*139,30e"
  "215e20i196c269d2e8*153,52e5*139,33e"
  "215e20i197c269d3e7*153,51e4*139,34e"
  "216e23i193c270d3e6*153,50e5*139,34e"
  "216e20i2ci194c269d4e4*153,51e4*139,35e"
  "217e21i195c269d5e4*153,49e6*139,34e"
  "217e20i197c269d57e5*139,35e"
  "218e20i196c41d3a226d54e7*139,35e"
  "218e18i199c41d2a226d53e8*139,35e"
  "219e15i202c269d51e9*139,35e"
  "219e15i202c270d49e9*139,36e"
  "219e16i202c269d49e8*139,37e"
  "218e19i200c270d53e2*139,38e"
  "219e20i199c269d52e139,40e"
  "220ea18i199c107d2a161d92e"
  "220eia17i200c106d2a161d92e"
  "220eia17i17c68,2c2*68,179c269d91e"
  "221eia15i18c68,3c2*68,178c269d91e"
  "222ea15i203c269d90e"
  "221e2ia14i203c269d90e"
  "222eia14i203c270d89e"
  "223eia13i204c270d88e"
  "224ea13i204c270d88e"
  "223e2ia2ia9i205c270d87e"
  "225eai2a10i205c269d87e"
  "225ei2a2ia6ia207c269d86e"
  "226ei2aiai2a2i2a80,207c268d86e"
  "228ei2aia2i2a4*80,205c269d85e"
  "229e2i2a2ia80,3a2*80,69c2*79,134c269d84e"
  "232ei3a2*80,a5*80,69c2*79,132c269d84e"
  "231eiei2a3*80,2a5*80,69c79,131c270d83e"
  "232ei2e14*80,199c269d83e"
  "237e2*80,7a80,201c270d82e"
  "240e3ia2i203c269d82e"
  "243ei2e203c270d81e"
  "247e202c270d81e"
  "249e201c270d80e"
  "250e200c270d80e"
  "251e2l198c270d79e"
  "249e4l199c269d79e"
  "249e3l200c270d78e"
  "249e3l201c269d78e"
  "249e3l202c268d78e"
  "249e3l202c269d77e"
  "249e4l202c268d77e"
  "249e4l202c268d77e"
  "249e4l203c267d77e"
  "249e4l203c268d76e"
  "248e6la202c267d76e"
  "249e5l2a201c268d75e"
  "250e6a2c2a198c267d75e"
  "250e6ac3a198c268d74e"
  "250e11a198c267d74e"
  "250e11a198c268d73e"
  "251e11a198c267d73e"
  "252e11a197c268d72e"
  "253e11a197c267d72e"
  "253e11a197c268d71e"
  "253e12a197c267d71e"
  "254e12a196c267da70e"
  "254e13a196c267d70e"
  "254e14a195ca266da69e"
  "800a"
  ))
