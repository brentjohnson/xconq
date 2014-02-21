;;;  ng-chinese  v. 0.1  10/28/92
;;;  M. Campostrini
;;;
;;; Chinese place name generator.
;;; Uses the "official" Pin Yin transcription scheme.
;;; From 149 town names.
;;; 

(namer chinese-place-names (grammar root 10
  (root (or 130 (syll syll)
	     19 (syll syll syll)
	     )
	)
  (syll (or 3 an bai 2 bao 3 bei bie 2 bo cai cang cao chan 3 chang 9
	    cheng chu chuan 2 ci 3 da dan di 5 dian 3 ding dong fang 2 feng
	    fu gang gao 2 ge gong gou 7 gu 2 guan guang guo hai han hang 9
	    he hei heng her hoe hong hu 3 hua huai huan 4 huang huo 3 ja 4
	    ji jian jiao jie 5 jin jing 2 jiu 2 kai 2 kou le 4 li liao 4
	    lin 2 ling long 3 lu mang 2 ming 4 nan nei 2 nian 2 ning peng 3
	    ping 2 pu 3 qi qiao 2 qing 4 qiu qu quan 2 qui 2 quing rao ren
	    ru san se 3 shan 4 shang she 3 shen sheng 4 shi 3 shou 2 shui
	    shun si sui 3 tai tan 2 tang tao 2 tian tong tuo 2 wan 3
	    wang 2 wei wu 5 xi xia 12 xian 3 xiang 4 xin 3 xing xu xuan ya
	    5 yan 9 yang 2 ye 3 yi 2 ying 2 yong 4 yu 3 yuan yuang 2 ze
	    zhai 4 zhang zhao zhe 2 zhen 4 zheng 4 zhou zhu 3 zhuang zhuo
	    zun)
	)
  )
)


