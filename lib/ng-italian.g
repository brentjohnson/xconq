;;;  ng-italian  v. 1.0  10/26/92
;;;  M. Campostrini
;;;
;;;  Italian place name generator.
;;;  
;;;  PROBLEM: "finale" (ending) is capitalized by xconq; 
;;;     it shouldn't!  All the needed capitalizations
;;;     are included in the strings themselves.
;;;

(namer italian-place-names (grammar root 30
  (root (or 20 nome (nome " Terme")))
	(nome (or (santo) 2 (santo " " finale) 
		  (altro) 2 (altro " " finale) 2 (inizio " " finale)))
	(santo ("S. " nome-santo)) 
	(nome-santo (or Adele Adriano 2 Agata 2 Agnese 2 Agostino Albino 
			3 Ambrogio Anastasio 5 Andrea 5 Angelo 5 Anna 
			5 Annunziata Antimo 5 Antonio Apollinare 3 Arcangelo
			3 Barbara 5 Benedetto 3 Bernardo 2 Biagio Bianca
			Brigida 4 Carlo 2 Cassiano Cataldo 5 Caterina 
			5 Cecilia Cesareo 4 Chiara 2 Cipriano Clemente
			Colomba 3 Colombano 4 Cristina 4 Cristoforo 4 Croce
			Damaso Damiano Donato Donnino Egidio Elena Elia 
			2 Eufemia Eusebio Fabiano Faustino 3 Felice 5
			Filippo Fiora Firmina 3 Floriano Franca 5 Francesco
			Frediano 3 Gabriele Gaetano Galgano Gaudenzio 3
			Gennaro Gervasio 3 Giacomo 2 Gimignano 4 Giorgio 
			5 Giovanni 2 "Giovanni Battista" Giulia Giuliana 
			2 Giuliano 5 Giuseppe 3 Giusto 2 Guido Iacopo Ilario
			Ippolito Lazzaro 2 Leo 2 Leonardo Liberata Liberato
			4 Lorenzo 4 Luca 5 Lucia Luciano 2 Marcello 4 Marco
			3 Margherita 5 Maria 2 Marino Mario 3 Marta 
			5 Martino 2 Maurizio 2 Mauro 5 Michele Miniato 
			4 Nicola Pancrazio 5 Paolo 2 Patrizio 3 Pellegrino 
			2 Piero 5 Pietro Polo Prospero Quirico Regolo
			Reparata 4 Rocco Romano Ruffillo 3 Salvatore Savino
			Secondo 3 Severa Severo 3 Silvestro 2 Siro Sisto 
			3 Sofia 5 Stefano 5 Tommaso Ubaldo 3 Valentino
			Venanzio 4 Vincenzo 2 Vitale 2 Vito Vittore
			Vittoria 2 Zeno))
	(inizio (or 2 Badia 2 Bagno Baracca Bassano 3 Borgo Bosco 
		    2 Borghetto 2 Campo Capanne 1 Casale Case 3 "Ca'"
		    Casino 5 Castel 3 Castelletto 3 Castelnuovo 
		    3 Castiglione 5 Certosa 2 Chiesa "Citta`" 3 Cittadella
		    2 Colle Corte Costa 2 Croce Crocetta 2 Dogana Dosso
		    Fabbriche 3 Fonte 5 Forno 2 Forte Fossa Grotte 6 Lido
		    Lugo 15 Madonna 6 Marina 2 Maso Massa 2 Mercato 
		    4 Molino Monastero Montebello Montecchio 3 Ospitale
		    Ospitaletto 3 Osteria Palazzi Penna 7 Pian 9 Pieve
		    Poggio 2 Ponte Porto Pozzo 7 Rocca 2 Selva Serra 
		    2 Serravalle Sasso Tavernelle Tombolo 4 Torre Vico
		    12 Villa 4 Villafranca))
	(altro (or Badia Bassano Borghetto Canonica Cantone Capraia Caprile
		   Carraia Casale Casanova Caselle Castagneto Castelletto
		   Castellina Castelnuovo Castelvecchio Castiglione Castro
		   Cerreto Chiesanuova Cittadella Civitella Colognola
		   Farneta Felino Fiano Frassineto Fratta Gabella Galliera
		   Gazzo Giglio Gorgo "Le Grazie" Grazzano Groppo Gualdo
		   Isola Lama Lavino Leccio Legnaro Lucignano Lugo Magliano
		   Marano Marciana Massa Meleto Mercatale Mezzano Miano
		   Migliarino Montagnana Montalto Montebello Montecchio
		   Montenero Montorio Oliveto Ospitaletto Palazzolo
		   Panicale Panzano Piazza Prato Querceta Rivalta Ronchi
		   "La Rotta" Rovereto Sala Serravalle Stagno Strada
		   Tavernelle Vaiano Vignola Villafranca Villanova))
	(finale (or "a Caiano" "a Egola" "a Elsa" "a Ema" "a Grado"
		    "a Greve" "a Moriano" "a Pilli" "a Po" "a Signa"
		    "al Lanzo" "al Mare" "al Serchio" "al Toppo" "al Vento"
		    "alla Lastra" "alla Palma" "alla Vena" "Baganza"
		    "Bovarino" "d'Adda" "d'Adige" "d'Albero" "d'Alma"
		    "d'Arbia" "d'Arceno" "d'Argine" "d'Asso" "d'Elsa"
		    "d'Enza" "d'Ombrone" "d'Orcia" "de'Fabbri"
		    "de'Lombardi" "degli Arduini" "dei Conti"
		    "dei Marsi" "del Benaco" "del Colle" "del Guidice"
		    "del Lago" "del Lario" "del Pero" "del Rio"
		    "del Vescovo" "del Voglio" "dell'Abate" "dell'Alpe"
		    "dell'Arsa" "della Chiana" "della Pescaia"
		    "della Valle" "delle Corti" "delle Olle" "di Baggio"
		    "di Brancoli" "di Brenta" "di Castro" "di Compito"
		    "di Corsano" "di Diamantina" "di Garfagnana" "di Magra"
		    "di Marte" "di Monte" "di Novello" "di Piave" "di Reno"
		    "di Setta" "di Sopra" "di Sotto" "di Vallico" "di Vico"
		    "Finalese" "in Bosco" "in Chianti" "in Collina"
		    "in Gualdo" "in Persiceto" "in Salcio" "in Teverina"
		    "in Tuscia" "Maggiore" "nel Frignano" "Pelago"
		    "Roncole" "Secchia" "sul Metauro" "sul Panaro"
		    "sul Trasimeno" "sull'Oglio" "Trebbio" "Val di Pesa"
		    "Valdarno" "Vallese" "Vara"))
	)
  )



