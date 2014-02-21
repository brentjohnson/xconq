(game-module "t-roman"
  (title "the Roman world")
  (blurb "Map and towns of the Roman world, ca 50 AD")
  (default-base-module "roman")
  (notes 
   ("Map and towns of the Roman world, ca 50 AD."
    ""
    "Massimo Campostrini"
    ))
  )

;  The feature area layer is used for provinces on this map.
;  All borders are therefore drawn correctly.
(set feature-boundaries true)

(world 2000)

(area 200 110)
(area (terrain
  (by-name
    (sea 0) (shallows 1) (swamp 2) (desert 3) (plains 4)
    (forest 5) (mountains 6) (ice 7) (road 8) (river 9))
  "146h54a"
  "h43af6g16f11a68fh53a"
  "h43a2f5g2fa10f13a70fh52a"
  "h42a5f2g3fafa12f9a71fh51a"
  "h22af3a3f16a8f2a3f2b7f11a70fh50a"
  "h21a2f2a4f15a7f4a2f3b6f9a2f2a70fh49a"
  "h25a4f16a7f4a2fb7f10af3a70fh48a"
  "h22afa3f19a5f6a9f14a65f6eh47a"
  "h25a7f17a2f7a9f3a2f5a2f2a59f12eh46a"
  "h24a8f26a9f3a2f4a58f19eh45a"
  "h24afa6f23af4a8f3a2f4a56f21eh44a"
  "h24afa4fe22a3f4a8f3a2f4a55f23eh43a"
  "h26afa3e22a4f4a7f10a51f27eh42a"
  "h28a4e20a5f5a6f10a48f31eh41a"
  "h21a4f3a6f19a4faf4a4f12a44f35eh40a"
  "h21a4f3a6f19a4faf4a3f13a41f39eh39a"
  "h18a2fa6fa4f3e19ab3f2afafa3f14a38f41eh38a"
  "h19a6f2ea2f2a3e20a3f2a3faf16a37f43eh37a"
  "h19a6f2e5a2f3e19a2faf2a2f16a37f45eh36a"
  "h20a5fe3af2a3f3e18a3f2af13a2f2a38f46eh35a"
  "h21a3f3e6a2f3e19a3f2a2f9a5f2b36f43e2b3eh34a"
  "h19a5f3e6a2f4e19a2f12a7fb36f37e6a3b2edh33a"
  "h18a7f3e6a4efe18ab3f3a4f3b43f38e7a3be2dh32a"
  "h17a6f4e4a4fef2efe17a5fb51f38e8a4b2dh31a"
  "h18a5f5e4a4f2ef3e14a2fb55f39e9a4b2dh30a"
  "h19a3f2e9a3f2e3fe13a3fb55f38e10a5b2dh29a"
  "h33a4fe3fea2e8a60f39e10ab2e2b3dh28a"
  "h32ae3f2e4f3e7afb59f39e10ab4e4dh27a"
  "h32a8efef2e7a2f2b3e53f41e9a6e4dh26a"
  "h37aef6e7ab5ef2e51f40e9a8e4dh25a"
  "h35a8e9a5ef3e51f41e8a8e5dh24a"
  "h34a2fe2fe2f3e6a9efe49f42e8a9e5dh23a"
  "h33a2f11e4a13e40fe7f43e11a6e5dh22a"
  "h32a4e5a2e2a2ea4e2f10e39f2e5f44e12a6e5dh21a"
  "h48a2e5f10e38f53e15a3e4dh20a"
  "h47a3e6f3ef7e36f29ea2b22e15a2e3b2dh19a"
  "h41a2e5a2e2fe2f12e35f30e2a25e14a2e4bdh18a"
  "h41a2e4a20e32f32e4a25e13a2e5bdh17a"
  "h35a4e3a26e23f5g2f32e6a26e14a5bdh16a"
  "h37a14ef16e24fe2g14eg19eb6a28e13a2b4eh15a"
  "h36a2e2f11e2f8e2f3ef2e23f18eg18e2b6a29e12a6eh14a"
  "h36a6e2f2e2f9ef4ef3ef2e23f18efg16e2c3b3a32e11a6eh13a"
  "h40a3e6f8e2f8ef2e7f2efe12f18egf9eb3e2bc4eb2ea4e2g6e8ge7g6e13a2eh12a"
  "h42a3e4f11ef6e2f2e7f4e11f19egf8eb4ab4eg2e4a4e7g3hg3h2g3h2gh5g4e11a2eh11a"
  "h43a3e3f18e2f4e4f5e11f6e2f7e2f2efg7e6a4e2ge9a4e22g6e11a2eh10a"
  "h43a5e2f17e2f5e4f3e2f3g7f7ef6e6fg6e10aege13a6e5g17e13a2eh9a"
  "h45a3e2f23e9f2g3f2e3f15ef2g2f2g7e10a2e19a23e13a2eh8a"
  "h45a10e2f16ef9g2f22efgf11ec31a6e8g6e2c12a4eh7a"
  "h47a8e4f2e3f7e2f9g4f21e2f13e32a5e9g6e2c12a4eh6a"
  "h47a9e2f4e2f6e5g3f6g3f20e2f9ef3e33a6e8g7e12a5eh5a"
  "h48a2ef4e2f6e2f5efghgf4eg3efg7f15e3f8e2f2e33ae2g4e8g7e13a4eh4a"
  "h49a3f3e3f12ef3g11eg6f16e2f12e33ae2g4e10g5e13a5eh3a"
  "h49a3f2e3f13ef2g14e8f13e2f13e33a4e15g4e11a6eh2a"
  "h49a3f2e4f12e3g10e2a5e3f2g3f10e3f12e29a8e18ge9a2g7eha"
  "h31a2e16a4f2e3f6ef5ef3gf8e3a5e4f2g3f9e3f11e29a8e24g3a3g8eh"
  "ah28a2e2f6e10a4f9e4f5e4g2f3g3e4ae2a2e5f3g4f6e2f11e14a3eae2ae4a9e32g8eh"
  "2ah28aef5g2f3e7a3f9e6f4e3f3ef3g3e8a2e4f6gf6e4f4e2f3e13a6e2a10e12ge22g8eh"
  "3ah27a2e12g5a3f9e2f12e2a3e2gf2e7a8e6g6e3f4g2f3e12a18e13g2e13g2e2f4g2e6dh"
  "4ah27a3g3f8g3e3g2f22e5ae3g3e7a8e5g5e3f10e11a3e2g6e20g3e7g7e3f2g2fe6dh"
  "5ah26a3g9e3g4e5g17e8a2e3g3e7a8e6g17e9a4e2g3e24g3e7g6e3f2g2f7dh"
  "6ah26a13e2g5e5g6e5a4e9a3e4g2e9a6e6g16e8a11e3ge19g5e5g5e7f7dh"
  "7ah25a14e2g6e5g3e19a4e3g2e10a6e6g17e3a24e5g2e6g6e7ge7fe7dh"
  "8ah24a9ef6eg7e5g2e16ae3a4ef2g2e10a6e3ge3g4eg3f7ea6e3g16e4g4e5g7e7g6f2e7dh"
  "9ah23a5ef3e2f15e4ge16age3a5e2g2e11a5e3ge7gf8e4a4e3g15e3g7e4g10e5g5f2e7dh"
  "10ah23a2e5f2e2f17e2g2e15age4a5e2g3e12a2e3g2e5g2ea4e4a24e3g7e4g11e6g3f2e7dh"
  "11ah22a2e5f5g8ef10e16age5a5e3g4e9a3e3g8e3ae2a9eg16e4g7e5g11e6g3fe7dh"
  "12ah21a3e7g10e3f7e18ae7a5e3f3e9a2e4g5e8ae2g13e2d7e5g4eg3e4g13e5g3f7dh"
  "13ah19a3e7g12e2f5e29a6e3fe9a3e5g3e8a11eg5e2d6e5g3e3g3e3g14e5g3f6dh"
  "14ah18a2e4g21ec22ae12a3e2f5e5a2e4g7e4aea4e2g13eg4e2g2e2g3e2g21e4g4f5dh"
  "15ah17a20ef6e23a2e12a3e3f5e3a5e2g2e2a2e6a7eg5e2g4eg2e5g2eg27e4g3f5dh"
  "16ah17a16ef2e3f4e22ae2fe14ae3fe2a3e3a2e5ge12a6e3g2eg7e4g4eg26e5g3f4dh"
  "17ah17a14e2f2e3f4e23ae2f14a3efe4ae3a3e5ge8aea7e5g7e3g4eaeg27e4g4e3dh"
  "18ah17a13ef5ef4e7a3eae12ae2f16aef10a3e3ge11a4e3g3e3g3e4g4e3a29e5g2e3dh"
  "19ah16a24e7a2e15ae2f17aefe10a2eg2e12a11e8g2e4a31e5ge3dh"
  "20ah16a21ef3e3ae19a3e17aefe10a3eg4e8ae2a9e6g3e5a31e6g2dh"
  "21ah14a21e2f3e23a3e18a2e11a9e10a15e7a32e6gdh"
  "22ah14a13e3g4ef3e25ae19a2e17aea2e9a5eg4e4a2e3ae4a31e6gh"
  "23ah19a7e4g2f4e47ae13a5e5ae7a2e2ae2ge9a2e4a31e3g3ah"
  "24ah20a4ef5gf4e47ae14ae2geae8ae7a3e7a4e6a29e6ah"
  "25ah20a6e2fg3e45a3eae15a2ege6ae7ae12ae2g2e5a12e5d13e5ah"
  "26ah20a11e40a8e19a4e3ae2ae2ae3ae12a3e6aeg10e11d8e4ah"
  "27ah19a3e48a2e4f2g19aea2e35aeg6e22de4ah"
  "28ah71a2e2f2g26a4eae27aeg5e25d2ah"
  "29ah18ae25a4e3a2e7ae11a4e28a4e27aeg5e26dah"
  "30ah16a3e16a28eae8a3e59aeg5e26dh"
  "31ah14a6e13a11e6g3e2g9e70aeg5e26dh"
  "32ah14a2eg7e5a10e14g4eg5e71a6e26dh"
  "33ah12a2e4g17e21g6e71a6e26dh"
  "34ah11a5e41g5e11ae59a5e26dh"
  "35ah7a5e43g7e70a5e26dh"
  "36ah5a4e33gd4e4g11e69a5e26dh"
  "37ah4a2e30g8d2e2c2ece2c7e69a5e26dh"
  "38ah3a2e20g21dcd3e2c7e61a2e5a3e29dh"
  "39ah2ade13g31d10e33a4e25a9e30dh"
  "40ah2a2d7g37d9e33a6e21a5e36dh"
  "41aha50d5e32a12e15a6e36dh"
  "42ah52d7e28a14ea13ed5e35dh"
  "43ah56d2e28a3e8d5e14d4e35dh"
  "44ah57d3e26a2e28d4e3da6da23dh"
  "45ah58d6e22ae29d3e4da2d2gda23dh"
  "46ah63d6e17ae28db3e4d2a2dgda22dh"
  "47ah66d3e16ae30d3e4d2a3da22dh"
  "48ah66d4e15a2e29d3e5d3ad3a19dh"
  "49ah66d4e14a4e28d2e6d7a18dh"
  "50ah45de21d8e9a2e30d2e7d7a16dh"
  "51ah67d10e5a4e30d2e7d7a15dh"
  "52ah72d12e14de17d2e7d8a13dh"
  "53ah107de8d2e7d8a12dh"
  "54ah116d2e7d9a10dh"
  "55a145h"
))
(area (aux-terrain road
  "200a"
  "200a"
  "200a"
  "200a"
  "200a"
  "200a"
  "200a"
  "200a"
  "200a"
  "200a"
  "200a"
  "200a"
  "200a"
  "200a"
  "200a"
  "200a"
  "200a"
  "33acwq164a"
  "35a@164a"
  "36a@163a"
  "37aD162a"
  "37af162a"
  "38a@161a"
  "39a@160a"
  "40a@159a"
  "41a@158a"
  "42a@157a"
  "43aD156a"
  "37ae5aj156a"
  "38a@4aj156a"
  "39a>3sv10ae145a"
  "44a>u9a>su142a"
  "46a@4ak2su3a>u140a"
  "47a<aksr3aB4sN}138a"
  "49af6a@4af@137a"
  "50a@6aD4aD@136a"
  "51a@5af4afa@135a"
  "52a>su3a@4aDa@134a"
  "55a>u2aF3sx2sNu132a"
  "57aFwr4aD3aD131a"
  "55aksra>u3af3aj131a"
  "55aj5a@3aD2aj131a"
  "54akr6a>uaf2af12ak2su115a"
  "53aktu8a@aD2aD10akr3a>10sy103a"
  "53aj2a@8a>~2af10aj15an103a"
  "52akr3a>su6aj@2a@4ak3s{r14akrD102a"
  "52aj7a>u4aja>uaF3sr3aj15ajaf102a"
  "52aj9a@3aj3a>v7aj14akr2a@20aksu78a"
  "51akr10a>uaj5a>u5af13akr4a>u18aj2aD77a"
  "51aj13aJr7a@5aD9ak2stu6a>u16aj2aj77a"
  "51aj10ak2sr@8aD4aj8akr4a>3su3a>su13af2aj77a"
  "51an8aksr4aFu6af4aj6aksr10a>2su2aB{u11aFsv37ak9su29a"
  "51aj@7aj6afa@5akVw2stsw3swr16a>2sQa@10aj2aD36aj10a>u27a"
  "51aja@6aj7a@a@ak2srfa@4aD3a>u19a@a@7aksr2aj29ak2su3aj12a@26a"
  "51aj2a>u4aj8aDaFr4aDa>u2aj5a>u18a@a>4sw{r4aj29aj3a@ksr13a@25a"
  "51aj4a@2akr8ajkr5aj3a@af7a>u17aD6a=5af14aksu9ak2sr4aE16a@24a"
  "51aj5a>wr9alr4aksv4a>u@8a@16af13aD13aj2a>su2ak3sr7af17a@23a"
  "50akr7a@9aj5aj2a>su3a>P8a@16a@12af12akr5a>sr12a@17a@22a"
  "50aj9a>u7aj2ak2sr5aD4aFu7a@16a@12a@11aj22aD17a@21a"
  "35ak5su8aj11a@k5s|2sr8af4afa@7a>2su13a@12a@9akr22af18a>su18a"
  "33aksr6a>u6aj11akM5ab12a@4aDa@10a@11aksN7su4a@8aj24a@20a>u16a"
  "33aj10a>u4aj11aj20a@3aj2aH10a>u9aj10a>2sua>u3ak2sr25a@21a>u14a"
  "33aj12a>u2aj11af21a@2af2aj>u10a>u5aksv14a>s{Lcsv8ak10s{2su6aD22a>u12a"
  "33af14aFst3su8aD21a>uaDaj2a@11a>2sukr2a@8aku3aksr4a@6akr9akr3a>u4aj24a>u10a"
  "34aD12akr6a>su5aj23a@faj2akNsu12aE4a@7aja>2sr7aF5sr9akr6a>3sv26aH9a"
  "34aj11akr10a@3akr24a>Rt2sr3a>u10af5a>u3aksr10aksv14akr12a@23aksr@8a"
  "34aj10akr12a@ksr27a>u7a@10aFu5a@2aj11akr2a@12akr14a@20aksr3a>u6a"
  "33akr10aj13akM31a>2su4a@9afa@5a@aj9aksr4a@10akr16aD16ak2sr7a@5a"
  "33aj11af13aj36a>su2a>su7a@a@3aksVr9af7a@9aj17aj12ak3sv11a>u3a"
  "33adu11aD12aj39aB4sV2sq4a@a>2sraf11a>wsu3akN2su4akr17af12aj4a>u11a@2a"
  "35a>u9aj12aj40a@3aj8a@6a@12aDaF2sr4a>3st3su5aku8a@11aj6a@11a>q"
  "37a@8aj12aj41a>uaj9a@6aD11afaj16a@3akra@7akNu4aksu2aj7a>11sq"
  "38a>u6aj12aj43a@j10a>u4aj12a>v17a>{sr3a@4aksr2a>{2sr2a>sr20a"
  "40a>u4af9ak2sz44aA12a@3af14a@17aj6aF3sr5aj28a"
  "42a>4sV8sr2aj45a@12a@3a@14a@14aksr6aj9aj28a"
  "47aj11aj46aD12a>3sNwsu11aF4su3ak4sr8af9aj28a"
  "47aj11aj46af18aDa<10ab5a>2sr14a@8aj28a"
  "47aj11aj47aD17aj37a@7aj28a"
  "47aj11ab47aj17af38aD6aj28a"
  "46acr55aksyab18aD37aj6aj28a"
  "99ak3sraj20ab37aj6aj28a"
  "99af5aj58af6aj28a"
  "100a>u3aj59aD5aj28a"
  "74ak2su3aku19a>2sr59af5aj28a"
  "47aksu16ak7sr3a>2sra>9su72aF4sz28a"
  "47aj2a>u13akr27a@71af4aj28a"
  "47aj4a>3su5ak2sr29aD71aD3aj28a"
  "46akv9a>4sr32af71af3aj28a"
  "46aja>sq45a@71aH2aj28a"
  "44acsr50a@70aj@aj28a"
  "98aD69ajaFv28a"
  "98aj69alsra@27a"
  "97akr68akr4a@26a"
  "97aj33ak2su27ak4sr6aD25a"
  "97aj33aj3a>u24akz11af25a"
  "97an32akr5a>3su15akw2srf12aD24a"
  "97aj>su29aj11a>uak11sra@3aD11aj24a"
  "97aj3a@28af13a>r14a@2aj11ab24a"
  "97aj4a>su26a@29a>uf36a"
  "97aj7a>3su22a@30a2@35a"
  "96akr12a>4su17aD30a2@34a"
  "96aj19a@16af31a2D33a"
  "96aj20a>u15aD30afn33a"
  "96aj22a@14af31aA@32a"
  "96ab23a>4su9aD31a2@31a"
  "126a>su5akr32a2@30a"
  "129a>4sr34a2@29a"
  "170a2@28a"
  "171a@<27a"
  "172a<27a"
))
(area (aux-terrain river
  "200a"
  "133acy65a"
  "134ad}i63a"
  "135ab?y62a"
  "136aepq61a"
  "135aeoM<61a"
  "135agM<62a"
  "135agL63a"
  "135agL63a"
  "135agL63a"
  "135agL63a"
  "135agL63a"
  "135agL63a"
  "135acT63a"
  "136ady62a"
  "137ady61a"
  "138ahq60a"
  "130ai7acT60a"
  "130ad}i6ahq59a"
  "131ab?}i4acT59a"
  "133ab?}mi2ahq58a"
  "135ab=?yacT58a"
  "60ai19agq56adyad}i56a"
  "60ad}i9acy6agL57ahqab?}mi53a"
  "61ab?y9ahq5acT57acT3ab=?}i51a"
  "63ad}i7acT6ad}2mi54ahq5ab?}i49a"
  "64ab?}mi5ady6ab3=54agL7ab?}i47a"
  "66ab=?}mi3adq62aeoL9ab=47a"
  "69ab=?}mi64agM<58a"
  "54ai17ab=?}mi60aeoL59a"
  "54ad}mi17ab=?y59agM<59a"
  "55ab=?}i18ady57aeoL60a"
  "58ab?}i17ady55aeoM<60a"
  "60ab?y17ahq53aeoM<61a"
  "62ady16acT53agM<62a"
  "63ady16ady53a<63a"
  "64ady16ady116a"
  "65ad}i15adq115a"
  "66ab?y131a"
  "68ahq130a"
  "68agL9ami119a"
  "68agL9ab?}2mi115a"
  "68acT11ab2=?}10mi103a"
  "52ae2mi13ahq14ab10=Cq102a"
  "51aeoM=?y12acT3aemi19acT102a"
  "45ae2miaeoM<2ady12ad}2moM=20ahq101a"
  "46a2=?}oM<4ab13ab3=<21acT19ae3m78a"
  "49ab=<46ad}i17agM=?T77a"
  "99ab?}i15agL2ab77a"
  "65aemi33ab?}mi12agL80a"
  "65agM?}mi32ab=?}mi9acT80a"
  "65acTab2=35ab=?y9ahq79a"
  "66ahq41ady6aemoL79a"
  "66acT10a5mi26ad}5moM=<79a"
  "67ahq9ab5=27ab6=<81a"
  "67agL131a"
  "67acT131a"
  "68ahq130a"
  "68agL130a"
  "68agL130a"
  "33a6mi29a<130a"
  "33ab6=160a"
  "163ae8ai27a"
  "52ac}i108agL7ady26a"
  "53ab?y107acT8ady25a"
  "55ad}i106ady8ady24a"
  "46aem8ab?}i105ady8ahq23a"
  "43ae2moM<9ab?y105ahq7acT23a"
  "39ae3moM2=<12ab105agL8ady22a"
  "33ae5moM3=<121acT9ady21a"
  "34a6=<126ad}2mi6ad}6mi13a"
  "168ab2=?}3mi2ab6=?y12a"
  "172ab3=?}i8ady11a"
  "177ab?}2mi5ady10a"
  "179ab2=?}i4ahq9a"
  "183ab?}i2acT9a"
  "185ab?}2mpy8a"
  "187ab3=?y7a"
  "192ab7a"
  "200a"
  "200a"
  "200a"
  "200a"
  "200a"
  "200a"
  "200a"
  "200a"
  "200a"
  "200a"
  "200a"
  "200a"
  "200a"
  "160agq38a"
  "160acT38a"
  "158aicyhq37a"
  "158adyd{T37a"
  "159adyd;q36a"
  "160ad}pL36a"
  "161ab?T36a"
  "163ady35a"
  "164ady34a"
  "165ahq33a"
  "165acT33a"
  "166ady32a"
  "167ady31a"
  "168ady30a"
  "169ady29a"
  "170ady28a"
  "171ady27a"
  "172ab27a"
))

(area (features (
   (1 "provincia" "Britannia")
   (2 "provincia" "Hispania Lusitania")
   (3 "provincia" "Hispania Baetica")
   (4 "provincia" "Hispania Tarraconensis")
   (5 "provincia" "Gallia Aquitania")
   (6 "provincia" "Gallia Narbonensis")
   (7 "provincia" "Gallia Lugudunensis")
   (8 "provincia" "Gallia Belgica")
   (9 "provincia" "Gallia Cisalpina")
   (10 "provincia" "Germania Superior")
   (11 "provincia" "Germania Inferior")
   (12 "provincia" "Alpes")
   (13 "provincia" "Raetia")
   (14 "provincia" "Noricum")
   (15 "provincia" "Sardinia")
   (16 "provincia" "Corsica")
   (17 "provincia" "Sicilia")
   (18 "provincia" "Dalmatia")
   (19 "provincia" "Pannonia Superior")
   (20 "provincia" "Pannonia Inferior")
   (21 "provincia" "Moesia Superior")
   (22 "provincia" "Moesia Inferior")
   (23 "provincia" "Thracia")
   (24 "provincia" "Macedonia")
   (25 "provincia" "Epirus")
   (26 "provincia" "Achaia")
   (27 "provincia" "Creta")
   (28 "provincia" "Bithynia")
   (29 "provincia" "Pontus")
   (30 "provincia" "Phrygia Minor")
   (31 "provincia" "Mysia")
   (32 "provincia" "Lydia")
   (33 "provincia" "Caria")
   (34 "provincia" "Phrygia")
   (35 "provincia" "Pisidia")
   (36 "provincia" "Lycia")
   (37 "provincia" "Pamphylia")
   (38 "provincia" "Paphlagonia")
   (39 "provincia" "Galatia")
   (40 "provincia" "Lycaonia")
   (41 "provincia" "Galaticus")
   (42 "provincia" "Cilicia")
   (43 "provincia" "Cappadocia")
   (44 "provincia" "Armenia Minor")
   (45 "provincia" "Commagene")
   (46 "provincia" "Syria")
   (47 "provincia" "Palmyrene")
   (48 "provincia" "Iudaea")
   (49 "provincia" "Aegyptus")
   (50 "provincia" "Libya")
   (51 "provincia" "Marmarica")
   (52 "provincia" "Cyrenaica")
   (53 "provincia" "Africa Proconsolaris")
   (54 "provincia" "Numidia")
   (55 "provincia" "Mauretania Caesarensis")
   (56 "provincia" "Mauretania Tingitana")
   (57 "provincia" "Cyprus")
   (58 "provincia" "Polemon")
   (59 "provincia" "Cappadocicus")
   (60 "provincia" "Italia")

   (61 "region" "Phazania")
   (62 "region" "Armenia")
   (63 "region" "Assyria")
   (64 "region" "Mesopotamia")
   (65 "region" "Media")
   (66 "region" "Atropatene")
   (67 "region" "Parthia")
   (68 "region" "Arabia")
   (69 "region" "Caledonia")
   (70 "region" "Hibernia")
  )

  "200a"
  "200a"
  "200a"
  "200a"
  "23a69,3a3*69,170a"
  "22a2*69,2a4*69,170a"
  "26a4*69,170a"
  "23a69,a3*69,172a"
  "26a7*69,167a"
  "25a8*69,167a"
  "25a69,a6*69,167a"
  "25a69,a5*69,168a"
  "27a69,a3*69,168a"
  "29a4*69,167a"
  "22a4*70,3a6*69,165a"
  "22a4*70,3a6*69,165a"
  "19a2*70,a6*70,a7*69,164a"
  "20a8*70,a2*69,2a3b164a"
  "20a8*70,5a5b162a"
  "21a6*70,6a6b161a"
  "22a6*70,6a5b161a"
  "20a8*70,6a6b160a"
  "19a10*70,6a6b159a"
  "18a10*70,4a10b158a"
  "19a10*70,4a10b157a"
  "20a5*70,9a9b157a"
  "34a9ba2b154a"
  "33a13b7al146a"
  "33a13b7a2l145a"
  "38a8b8a2l144a"
  "36a8b9a4l143a"
  "35a11b6a2i4l142a"
  "34a13b4a3i6l140a"
  "33a4b5a2b2a2ba6i7l138a"
  "49a6i8l137a"
  "48a8i8l136a"
  "42a2h5a9i6lk135a"
  "42a2h4a4h12i2k134a"
  "36a4h3a12h10i3k132a"
  "38a19h8i4k131a"
  "37a21h8i3k131a"
  "37a22h7i3k6a4n2o119a"
  "41a18h8i2k6a4n6o115a"
  "43a16h3k5i3k6a3n6o9t3u74a4Z25a"
  "44a15h5k3i3k6a3n6o9t3u73a6Z24a"
  "44a8h5f3h11k4a5n6o8t4u60a17Z25a"
  "46a3f2h7f5h12k5n6o9t3u60a18Z24a"
  "46a13f7h9k6n5o9t4u20a4w35a15Z65,2Z24a"
  "48a12f7h7k7n5o10t5u18a4w35a15Z4*65,23a"
  "48a13f7h3k4mn9j11t7u16a4w33a2W16Z3*65,23a"
  "49a13f4h2gh2k4m11j11t9u13a4w33a3W15Z4*65,13a4*67,5a"
  "50a13f3h5g4m12j11t11u11a3w33a4W14Z4*65,13a5*67,4a"
  "50a14f7g3m14j16s6u10a4w33a3W15Z5*65,11a6*67,3a"
  "50a14f7g3m10j2a2j21suv7a6w29a7W15Z5*65,9a9*67,2a"
  "32a2e16a12f10g3m9j3a2j21s5v9w29a8W7a7Z6*65,4*67,3a11*67,a"
  "30a10e10a8f14g3m9j4aj2a19s5v5x4w14a3~a~2a~4a4V5W2H7a2[4Z7*65,18*67,a"
  "31a12e7a8f15g2m10j8a17s6v9x13a6~2a~3E6VW4H8a6[6*65,66,17*67,a"
  "31a14e5a8f17gj2a8j7a18s5v10x11a10~4E6V4H8a7[5*65,2*66,16*67,a"
  "32a19e8f17g5a2j5X7a16s5v10x11a3~4B3~5E6V4H8a6[5*65,3*66,15*67,a"
  "32a21e6f14g8a8X7a15s5v11x9a4~6B7E5V4H8a7[3*65,4*66,15*67,a"
  "33a23e3f5g5a4g9a9X9a11s6v11x8a}2~7B7E6V4H8a7[2*65,4*66,15*67,a"
  "33a6c19e2f3g19a9X10a10s6v2y11x3a5}~8C6E6V5H7a9[4*66,15*67,a"
  "33a7c20e4g16aq3a9X10a7s10y11xa7}9C3E7GV5H8a9[3*66,15*67,a"
  "33a8c21e2g16a2q3a9X11a4s13y8x4a5}3>6C16G9a9[2*66,15*67,a"
  "34a8c24e15a2q4a10X12a14ya4x4a7}4>5C16G10a10[15*67,a"
  "34a9c22e16a2q5a12X9a14y3ax2a4:5}6>3C18G10a10[14*67,a"
  "34a9c21e18aq7a11X9a11y8a7:9>3D12G6I6a4*64,10[13*67,a"
  "33a11c18e29a10X9a11y8a7;9>4D6G3F9I9*64,10[13*67,a"
  "33a11c17e22ap12a10X5a13y4a;a6;9>5D5G4F8I10*64,9[13*67,a"
  "33a11c16e23a2p12a11X3a3z6y2a2y6a7;7>6D5G5F4I3J11*64,8[13*67,a"
  "34a10cd15e22a4p14a5X2a3X3a4z4y13a4<6>7D2G7F8J11*64,7[13*67,a"
  "35a8c3d14e23a3p14a5X4aX3a5z4y8a<a6<5>3?5D7FaF9J3K15*64,12*67,a"
  "36a6c5d13e7a3eae12a3p16a2X10a4z3y11a6<4>4?4D6F3a9J6K2*68,11*64,11*67,a"
  "36a5c7d12e7a2e15a3p17a3X10a3z2y12a6<3>6?D5F4a10J6K4*68,10*64,10*67,a"
  "37a4c8d13e3ae19a3p17a3X10a3z5{8a<2a4=2>4?4A4F5a9J6K8*68,7*64,9*67,a"
  "36a5c9d12e23a3p18a2X11a2z7{10a3=4@7AF7a9J6K10*68,5*64,9*67,a"
  "37a4c10d10e25ap19a2X17a{a2{9a4=4@2A4aAF3aU4a7J6K12*68,4*64,8*67,a"
  "43a9d8e47aX13a5{5a{7a2=2a4@9a2U4a7J6K13*68,4*64,4*67,4a"
  "45a8d7e47aX14a4{a{8a{7a3@7a4U6a5J6K14*68,4*64,7a"
  "46a8d4e45a3raX15a4{6a{7a{12a5U5a6J5K16*68,3*64,6a"
  "47a8d3e40a8r19a4{3a{2a{2a{3a{12a3U6a7J4K17*68,3*64,5a"
  "47a3d48a8r19a{a2{35a11J19*68,64,5a"
  "100a6r26a4|a|27a10J22*68,3a"
  "48aT25a4S3a2Q7aQ11a4r28a4|27a11J22*68,2a"
  "47a3T16a12S4R12QaQ8a3r59a10J23*68,a"
  "46a6T13a13S5R13Q70a10J23*68,a"
  "47a10T5a16S6R12Q71a9J23*68,a"
  "46a12T20S7R11Q71a3L6J23*68,a"
  "46a12T18S10R11Q11ar59a4L4J23*68,a"
  "43a15T14S15R11Q70a5L26*68,a"
  "43a15T12S18R11Q69a5L26*68,a"
  "44a14T12S19R10Q69a5L26*68,a"
  "45a13T13S19R9Q61a2M5a6L26*68,a"
  "49a10T12S19R8Q33a4P25a8M5L26*68,a"
  "72a18R8Q33a6P21a11M3L27*68,a"
  "73a18R7Q32a10P2O15a13M29*68,a"
  "80a11R11Q28a10P4Oa3O8N14M29*68,a"
  "84a7R11Q28a11P7O8N14M29*68,a"
  "86a6R13Q26a10P7O8N12MaM5*68,a23*68,a"
  "87a5R18Q22a9P7O9N12Ma5*68,a23*68,a"
  "88a4R24Q17a8P7O9N13M2a4*68,a22*68,a"
  "89a3R25Q16a9P6O9N14M2a3*68,a22*68,a"
  "91a2R26Q15a8P6O10N15M3a68,3a19*68,a"
  "93a27Q14a8P6O10N16M7a18*68,a"
  "94a4Qa3Q4Y20Q9a7P7O10N17M7a16*68,a"
  "95a5Q2a9Y18Q5a9P6O10N18M7a15*68,a"
  "96a4Q2a14Y17Q10P6O11N18M8a13*68,a"
  "102a17Y14Q10P7O10N19M8a12*68,a"
  "103a20Y11Q9P7O11N19M9a10*68,a"
  "200a"
))

(urbs 89 44 0 (n "Roma"))
(oppidum 96 5 0 (n "Cidamus"))
(oppidum 57 45 0 (n "Ilerda"))
(oppidum 169 38 0 (n "Thapsacus"))
(civitas 170 58 0 (n "Artaxata"))
(oppidum 164 58 0 (n "Carana"))
(oppidum 58 53 0 (n "Tolosa"))
(oppidum 163 45 0 (n "Melitene"))
(tribe 183 69 0 (n "Massagetae"))
(oppidum 53 66 0 (n "Cenabum"))
(oppidum 156 38 0 (n "Tarsus"))
(oppidum 60 58 0 (n "Gergovia"))
(civitas 172 1 0 (n "Thebae"))
(oppidum 60 64 0 (n "Alesia"))
(oppidum 169 3 0 (n "Ptolemais Hermiu"))
(oppidum 146 33 0 (n "Attalea"))
(civitas 63 49 0 (n "Narbo Martius"))
(oppidum 146 13 0 (n "Catabathmus Maior"))
(oppidum 98 19 0 (n "Thapsus"))
(oppidum 56 76 0 (n "Aduatuca"))
(civitas 150 43 0 (n "Caesarea"))
(tribe 61 82 0 (n "Chauci"))
(oppidum 146 39 0 (n "Iconium"))
(oppidum 80 65 0 (n "Noreia"))
(oppidum 100 29 0 (n "Panormus"))
(tribe 66 80 0 (n "Langobardi"))
(civitas 192 38 0 (n "Susa"))
(oppidum 107 40 0 (n "Brundisium"))
(urbs 190 45 0 (n "Ecbatana"))
(tribe 72 77 0 (n "Suebi"))
(oppidum 194 29 0 (n "Teredon"))
(oppidum 126 29 0 (n "Sparta"))
(urbs 179 38 0 (n "Seleucia"))
(oppidum 122 34 0 (n "Delphi"))
(civitas 175 14 0 (n "Petra"))
(oppidum 120 49 0 (n "Philippopolis"))
(oppidum 175 12 0 (n "Aelana"))
(oppidum 133 38 0 (n "Smyrna"))
(civitas 178 51 0 (n "Gazaca"))
(oppidum 138 33 0 (n "Halicarnassus"))
(oppidum 171 25 0 (n "Damascus"))
(oppidum 137 50 0 (n "Heraclea"))
(civitas 170 19 0 (n "Ierusalem"))
(oppidum 126 45 0 (n "Lysimachia"))
(oppidum 44 20 0 (n "Sala"))
(tribe 90 75 0 (n "Bastarnae"))
(oppidum 50 21 0 (n "Volubilis"))
(tribe 77 89 0 (n "Rugii"))
(oppidum 46 22 0 (n "Lix"))
(tribe 87 84 0 (n "Venedae"))
(civitas 47 25 0 (n "Tingis"))
(tribe 100 83 0 (n "Amadoci"))
(oppidum 56 23 0 (n "Rusaddir"))
(tribe 82 90 0 (n "Aestii"))
(oppidum 46 30 0 (n "Gades"))
(tribe 76 82 0 (n "Burgunti"))
(oppidum 47 32 0 (n "Hispalis"))
(tribe 81 70 0 (n "Marcomanni"))
(oppidum 33 40 0 (n "Olisipo"))
(oppidum 113 50 0 (n "Naissus"))
(civitas 59 31 0 (n "Nova Carthago"))
(tribe 101 66 0 (n "Iazyges"))
(oppidum 45 42 0 (n "Toletum"))
(oppidum 122 58 0 (n "Tomis"))
(civitas 50 46 0 (n "Numantia"))
(tribe 114 68 0 (n "Getae"))
(oppidum 33 49 0 (n "Baracara Augusta"))
(oppidum 117 54 0 (n "Nicopolis"))
(oppidum 43 49 0 (n "Lancia"))
(civitas 60 42 0 (n "Tarraco"))
(oppidum 82 49 0 (n "Pisae"))
(civitas 171 31 0 (n "Palmyra"))
(civitas 105 26 0 (n "Syracusae"))
(oppidum 80 52 0 (n "Luni"))
(oppidum 168 18 0 (n "Gaza"))
(oppidum 142 47 0 (n "Ancyra"))
(civitas 167 23 0 (n "Tyrus"))
(civitas 94 42 0 (n "Capua"))
(oppidum 166 6 0 (n "Hermopolis Magna"))
(oppidum 107 30 0 (n "Rhegium"))
(oppidum 84 25 0 (n "Hippo Regius"))
(oppidum 96 66 0 (n "Aquincum"))
(civitas 165 26 0 (n "Berytus"))
(tribe 113 60 0 (n "Daci"))
(oppidum 165 16 0 (n "Pelusium"))
(tribe 56 95 0 (n "Cimbri"))
(oppidum 97 14 0 (n "Tacapae"))
(civitas 130 41 0 (n "Pergamum"))
(oppidum 163 11 0 (n "Heliopolis"))
(tribe 65 99 0 (n "Gothi"))
(civitas 163 10 0 (n "Memphis"))
(oppidum 89 49 0 (n "Ancona"))
(urbs 161 36 0 (n "Antiochia"))
(oppidum 82 57 0 (n "Patavium"))
(civitas 161 14 0 (n "Sais"))
(civitas 104 40 0 (n "Tarentum"))
(urbs 158 14 0 (n "Alexandria"))
(oppidum 115 37 0 (n "Nicopolis"))
(civitas 125 32 0 (n "Corinthus"))
(civitas 69 49 0 (n "Massilia"))
(oppidum 107 57 0 (n "Sirmium"))
(civitas 157 31 0 (n "Salamis"))
(oppidum 135 26 0 (n "Gortyna"))
(civitas 93 25 0 (n "Utica"))
(civitas 94 24 0 (n "Chartago"))
(civitas 156 56 0 (n "Trapezus"))
(oppidum 86 34 0 (n "Caralis"))
(oppidum 112 9 0 (n "Sabrata"))
(oppidum 55 69 0 (n "Lutecia"))
(oppidum 151 64 0 (n "Dioscurias"))
(oppidum 151 3 0 (n "Ammonium"))
(tribe 149 72 0 (n "Siraces"))
(oppidum 82 45 0 (n "Aleria"))
(civitas 58 70 0 (n "Durcotorum"))
(civitas 65 60 0 (n "Lugudunum"))
(civitas 123 5 0 (n "Leptis Magna"))
(oppidum 43 79 0 (n "Londinium"))
(oppidum 135 4 0 (n "Boreum"))
(oppidum 145 52 0 (n "Amisus"))
(oppidum 37 81 0 (n "Glevum"))
(tribe 147 83 0 (n "Alani"))
(civitas 77 57 0 (n "Mediolanum"))
(civitas 141 30 0 (n "Rhodos"))
(civitas 105 30 0 (n "Messana"))
(oppidum 61 76 0 (n "Colonia Agrippina"))
(civitas 66 71 0 (n "Magontiacum"))
(civitas 83 54 0 (n "Ravenna"))
(civitas 37 88 0 (n "Eburacum"))
(tribe 25 89 0 (n "Hiberni"))
(tribe 57 82 0 (n "Frisii"))
(tribe 59 79 0 (n "Batavi"))
(tribe 64 76 0 (n "Sugambri"))
(tribe 69 73 0 (n "Chatti"))
(civitas 85 57 0 (n "Aquileia"))
(civitas 98 50 0 (n "Salonae"))
(tribe 29 99 0 (n "Picti"))
(civitas 133 16 0 (n "Cyrene"))
(civitas 109 44 0 (n "Dyrrachium"))
(civitas 127 33 0 (n "Athenae"))
(civitas 120 41 0 (n "Thessalonica"))
(civitas 136 36 0 (n "Ephesus"))
(civitas 126 50 0 (n "Apollonia"))
(civitas 130 47 0 (n "Bysanthium"))
; (urbs 130 47 0 (n "Constantinopolis"))
(civitas 135 44 0 (n "Nicaea"))
(civitas 134 46 0 (n "Nicomedia"))
(oppidum 123 68 0 (n "Olbia"))
(civitas 140 54 0 (n "Sinope"))
(tribe 118 77 0 (n "Roxolani"))
(tribe 131 66 0 (n "Tauri"))
(oppidum 136 67 0 (n "Panticapaeum"))
(oppidum 135 75 0 (n "Tanais"))
(tribe 125 86 0 (n "Sarmati"))
