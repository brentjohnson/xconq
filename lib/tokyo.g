(game-module "tokyo"
  (title "Tokyo 1962")
  (version "1.0")
  (blurb "Save Tokyo from the fire-breathing monsters!")
  (base-module "monster")
  (variants (world-seen true)(see-all true)(sequential false))
)

; (set terrain-seen true)

(area 50 20)

(area (terrain
  "ec5ec3ec3ec7ec5e2c10a9e"
  "2ec5ec2ec4e2c5e7c12a8e"
  "3ec5e3c4ecd6c21a5c"
  "4ec4ec2d5cdc3dcdc20ac2dc1d"
  "c4ec3ec2dc5d2c2dc2d5c15ac3d1c"
  "ec4e8c6dcdc2dc3d2c15a4c"
  "2ec3ecdcdefe6c2d5c3dcfc15ace1f"
  "3ec2ecdcdf2ecdc2dc2dcdc2e4cefc15ac1e"
  "3ec2e3c2d3c2dc2d3c2dcec2dcefec15a1c"
  "7c2d3cdc2dc2d2cdc2d2c2dc2efec15a"
  "7ecdcdc2d7c2d3cdcdce2fec15a"
  "8e2cdc3dc2dc2ec2dc3d2cf3ec15a"
  "3e3c2e8c2dcefe3c3dc2f4c15a"
  "6e3cec3d5cfe4f7c18a"
  "6ec4e2cd3c2dcfe3fc5b19a"
  "6ec6e2c2dcd2c4ec24a"
  "14ec2dcdcd6c23a"
  "15ecd3c2dc4ac22a"
  "15e3c2b3c27a"
  "10e8c32a"
))

(set sides-max 2)	; No units for other sides.

(side 1 (noun "Japanese") (emblem-name "none")
  (class "human")
)

(side 2 (noun "monster") (emblem-name "none")
  (class "monster")
)

(add u* point-value (100 1 1 2 1 1 0 0))

(scorekeeper (do last-side-wins))

;; Suppress default unit creation.

(add u* start-with 0)

;; Our hero (?) starts out in the water.

(monster 34 16 2 (n "Godzilla"))

;; Everything else

(mob 29 12 1)
(mob 20 17 1)
(mob 28 8 1)
(mob 24 9 1)
(mob 20 11 1)
(mob 23 7 1)
(mob 16 13 1)
(mob 19 9 1)
(mob 16 10 1)
(mob 14 4 1)
(mob 11 7 1)
(|fire department| 18 10 1 (in "Firehouse Central"))
(|fire department| 23 3 1)
(|fire department| 12 14 1)
(|fire department| 15 6 1)
(|fire department| 11 9 1)
(|national guard| 22 1 1)
(|national guard| 21 1 1)
(|national guard| 20 1 1)
(|national guard| 19 2 1)
(|national guard| 18 2 1)
(building 27 7 1 (n "Apple Japan HQ"))
(building 26 5 1 (n "Beach Hotel"))
(building 13 8 1 (n "Capitalists Trust"))
(building 16 2 1 (n "City Hall"))
(building 15 4 1 (n "Courthouse"))
(building 20 15 1 (n "Cygnus Support Tokyo"))
(building 27 10 1 (n "Fawlty Towers"))
(building 18 10 1 (n "Firehouse Central"))
(building 26 13 1 (n "Firehouse East"))
(building 17 15 1 (n "Firehouse North"))
(building 18 3 1 (n "Firehouse South"))
(building 8 10 1 (n "Firehouse West"))
(building 11 16 1 (n "Fry's"))
(building 14 12 1 (n "Hi-Rez Apts"))
(building 11 15 1 (n "Hi-Rise Apts"))
(building 28 9 1 (n "Hilton Tokyo"))
(building 13 15 1 (n "IBM"))
(building 10 9 1 (n "Japan Tobacco HQ"))
(building 22 12 1 (n "K9-Mart"))
(building 17 11 1 (n "Last National Bank"))
(building 16 4 1 (n "Main Barracks"))
(building 28 10 1 (n "McDonald's"))
(building 15 10 1 (n "Mormon Temple"))
(building 18 5 1 (n "Nordstrom"))
(building 17 17 1 (n "North Barracks"))
(building 14 8 1 (n "Price Club"))
(building 20 3 1 (n "Sears"))
(building 21 10 1 (n "Smiths"))
(building 27 11 1 (n "Target"))
(building 11 6 1 (n "Weird Stuff"))
(building 22 11 1)
(building 17 14 1)
(building 16 15 1)
(building 16 14 1)
(building 15 15 1)
(building 15 14 1)
(building 14 14 1)
(building 14 15 1)
(building 17 16 1)
(building 9 13 1)
(building 9 12 1)
(building 9 11 1)
(building 10 11 1)
(building 7 10 1)
(building 8 9 1)
(building 7 12 1)
(building 7 13 1)
(building 20 7 1)
(building 24 12 1)
(building 13 3 1)
(building 10 5 1)
(building 7 8 1)
(building 5 12 1)
(building 7 15 1)
(building 5 14 1)
(building 8 15 1)
(building 17 10 1)
(building 19 16 1)
(building 28 14 1)
(building 28 13 1)
(building 27 14 1)
(building 27 13 1)
(building 26 14 1)
(building 28 11 1)
(building 24 15 1)
(building 23 16 1)
(building 24 14 1)
(building 23 15 1)
(building 23 14 1)
(building 21 16 1)
(building 21 15 1)
(building 20 16 1)
(building 27 8 1)
(building 26 9 1)
(building 21 14 1)
(building 26 8 1)
(building 24 10 1)
(building 23 11 1)
(building 26 7 1)
(building 25 8 1)
(building 23 10 1)
(building 20 13 1)
(building 19 14 1)
(building 25 7 1)
(building 20 12 1)
(building 19 13 1)
(building 18 14 1)
(building 23 8 1)
(building 22 9 1)
(building 19 12 1)
(building 22 8 1)
(building 21 9 1)
(building 18 11 1)
(building 17 12 1)
(building 16 12 1)
(building 15 11 1)
(building 10 16 1)
(building 19 6 1)
(building 17 8 1)
(building 14 11 1)
(building 10 15 1)
(building 17 7 1)
(building 16 8 1)
(building 14 10 1)
(building 11 13 1)
(building 21 2 1)
(building 16 7 1)
(building 20 2 1)
(building 18 4 1)
(building 17 5 1)
(building 13 9 1)
(building 12 10 1)
(building 10 12 1)
(building 12 9 1)
(building 12 8 1)
(building 16 3 1)
(building 13 6 1)
(building 15 3 1)
(building 13 5 1)
(building 12 6 1)
(building 10 8 1)
