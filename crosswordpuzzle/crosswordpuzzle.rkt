#lang typed/racket

(require "../include/cs151-core.rkt")
(require "../include/cs151-image.rkt")
(require "../include/cs151-universe.rkt")
(require "./puzzlefromfile.rkt")

(require typed/test-engine/racket-tests)

;; editor's note: a lot of function/data structure definitions are provided 
;; in the puzzlefromfile.rkt program and not here- I import those definitions
;; into this file at the top 

;; ====== data structures/types
;; the other ones are provided in the puzzlefromfile.rkt file
(define-struct Click
  ([x : Integer]
   [y : Integer]))

(define-struct CrossWorld
  ([cell-size : Integer]
   [puzzle : CrosswordPuzzle]
   [last-click : (Optional Click)]
   [current-direction : (Optional Direction)]
   [current-cell-index : (Optional Integer)]
   [candidate-solution : (Listof (Optional Char))]
   [display-solution : Boolean]
   [check : Boolean]))

;; ====== example crossword puzzles
;; nyt-mini-nov-10-2020-clues
(define nyt-mini-nov-10-2020-clues
  (list
   (Clue 1 'across "Piece starting on the second of seventh rank, in chess")
   (Clue 5 'across "The \"O\" of B&O Railroad")
   (Clue 6 'across "Where a river meets the sea")
   (Clue 8 'across "LSD, by another name")
   (Clue 9 'across "What Pac-Man eats")
   (Clue 1 'down "Group of close friends and relatives, in 2020-speak")
   (Clue 2 'down "In front")
   (Clue 3 'down "Alt-rock band fronted by Jeff Tweedy")
   (Clue 4 'down "Cry in a game of tag")
   (Clue 7 'down "Fast-forwarded parts of podcasts")))

;; nyt-mini-nov-10-2020-cells
(define nyt-mini-nov-10-2020-cells
  (list
    ;; row 1
    (Some (NumberedCell #\P 1))
    (Some (NumberedCell #\A 2))
    (Some (NumberedCell #\W 3))
    (Some (NumberedCell #\N 4))
    'None
    ;; row 2
    (Some (NumberedCell #\O 5))
    (Some (LetterCell #\H))
    (Some (LetterCell #\I))
    (Some (LetterCell #\O))
    'None
    ;; row 3
    (Some (NumberedCell #\D 6))
    (Some (LetterCell #\E))
    (Some (LetterCell #\L))
    (Some (LetterCell #\T))
    (Some (NumberedCell #\A 7))
    ;; row 4
    'None
    (Some (NumberedCell #\A 8))
    (Some (LetterCell #\C))
    (Some (LetterCell #\I))
    (Some (LetterCell #\D))
    ;; row 5
    'None
    (Some (NumberedCell #\D 9))
    (Some (LetterCell #\O))
    (Some (LetterCell #\T))
    (Some (LetterCell #\S))))

;; nyt-mini-nov-10-2020
(define nyt-mini-nov-10-2020
  (CrosswordPuzzle
   5
   5
   nyt-mini-nov-10-2020-cells
   nyt-mini-nov-10-2020-clues
   (Some "https://www.nytimes.com/crosswords/game/mini, Nov 10 2020")))

;; p2-test0
(define p2-test0
  (CrosswordPuzzle
   5
   6
   (list
    ;; row 1
    (Some (NumberedCell #\A 1))
    (Some (LetterCell #\P))
    (Some (NumberedCell #\E 2))
    'None
    'None
    ;; row 2
    'None
    'None
    (Some (NumberedCell #\A 3))
    (Some (LetterCell #\C))
    (Some (LetterCell #\E))
    ;; row 3
    (Some (NumberedCell #\C 4))
    'None
    (Some (LetterCell #\S))
    'None
    'None
    ;; row 4
    (Some (NumberedCell #\H 5))
    (Some (LetterCell #\O))
    (Some (LetterCell #\T))
    'None
    'None
    ;; row 5
    (Some (LetterCell #\I))
    'None
    'None
    'None
    'None
    ;; row 6
    (Some (LetterCell #\P))
    'None
    'None
    'None
    'None)
   (list (Clue 1 'across "Mimic")
         (Clue 3 'across "High card")
         (Clue 5 'across "Cold counterpart")
         (Clue 2 'down "West opposite")
         (Clue 4 'down "English fry"))
   'None))

;; lat-mini-nov-15-2020-clues
(define lat-mini-nov-15-2020-clues
  (list
   (Clue 1 'across "Submit a paperless return")
   (Clue 6 'across "\"Stop stressing out!\"")
   (Clue 7 'across "Power washer's target")
   (Clue 8 'across "Final non-A.D. year")
   (Clue 9 'across "'60s antiwar gp.")
   (Clue 1 'down "\"Cogito ____ sum\"")
   (Clue 2 'down "Terrarium plants")
   (Clue 3 'down "Perjurer's confession")
   (Clue 4 'down "Ewe's babies")
   (Clue 5 'down "Corp. jet passenger")))

;; lat-mini-nov-15-2020
(define lat-mini-nov-15-2020
  (CrosswordPuzzle
   5
   5
   (list
    ;; row 1
    (Some (NumberedCell #\E 1))
    (Some (NumberedCell #\F 2))
    (Some (NumberedCell #\I 3))
    (Some (NumberedCell #\L 4))
    (Some (NumberedCell #\E 5))
    ;; row 2
    (Some (NumberedCell #\R 6))
    (Some (LetterCell #\E))
    (Some (LetterCell #\L))
    (Some (LetterCell #\A))
    (Some (LetterCell #\X))
    ;; row 3
    (Some (NumberedCell #\G 7))
    (Some (LetterCell #\R))
    (Some (LetterCell #\I))
    (Some (LetterCell #\M))
    (Some (LetterCell #\E))
    ;; row 4
    (Some (NumberedCell #\O 8))
    (Some (LetterCell #\N))
    (Some (LetterCell #\E))
    (Some (LetterCell #\B))
    (Some (LetterCell #\C))
    ;; row 4
    'None
    (Some (NumberedCell #\S 9))
    (Some (LetterCell #\D))
    (Some (LetterCell #\S))
    'None)
   lat-mini-nov-15-2020-clues
   (Some "https://www.latimes.com/games/mini-crossword, Nov 15 2020")))

;;------------------------------------------------------------------------------
;; ======= drawing the grid helpers

(: cell-draw : Cell (Optional Char) Integer (Optional Direction)
   (Optional Integer) -> Image)
;; draws any given Cell
(define (cell-draw cell char size dir index)
  (match cell
    ['None (frame (square size 'solid 'black))]
    [(Some (LetterCell l))
     (match index
       ['None
        (match char
          ['None
           (frame (square size 'solid 'white))]
          [(Some c)
           (overlay
            (text (list->string (list c)) (cast (exact-floor
                                                 (* 5 (/ size 6))) Byte) 'blue)
            (frame (square size 'solid 'white)))])]
       [(Some 0)
        (match char
          ['None (overlay
            (highlight size dir)
            (frame (square size 'solid 'white)))]
          [(Some c)
           (overlay
            (highlight size dir)
            (text (list->string (list c)) (cast (exact-floor
                                                 (* 5 (/ size 6))) Byte) 'blue)
            (frame (square size 'solid 'white)))])])]
    [(Some (NumberedCell l n))
     (match index
       ['None
        (match char
          ['None
           (overlay/align/offset "left" "top"
                                 (text (number->string n) (cast (exact-ceiling
                                                       (/ size 7)) Byte) 'black)
                                 (* -1 (exact-floor (/ size 12))) (* -1
                                                      (exact-floor (/ size 12)))
                                 (frame (square size 'solid 'white)))]
          [(Some c)
           (overlay
            (text (list->string (list c)) (cast (exact-floor
                                                 (* 5 (/ size 6))) Byte) 'blue)
            (overlay/align/offset "left" "top"
                                 (text (number->string n) (cast (exact-ceiling
                                                       (/ size 7)) Byte) 'black)
                                 (* -1 (exact-floor (/ size 12))) (* -1
                                                      (exact-floor (/ size 12)))
                                 (frame (square size 'solid 'white))))])]
       [(Some 0)
        (match char
          ['None (overlay
            (highlight size dir)
            (overlay/align/offset "left" "top"
                                  (text (number->string n) (cast (exact-ceiling
                                                       (/ size 7)) Byte) 'black)
                                  (* -1 (exact-floor (/ size 12))) (* -1
                                                      (exact-floor (/ size 12)))
                                  (frame (square size 'solid 'white))))]
          [(Some c)
           (overlay
            (highlight size dir)
            (text (list->string (list c)) (cast (exact-floor
                                                (* 5 (/ size 6))) Byte) 'blue)
            (overlay/align/offset "left" "top"
                                  (text (number->string n) (cast (exact-ceiling
                                                       (/ size 7)) Byte) 'black)
                                  (* -1 (exact-floor (/ size 12))) (* -1
                                                      (exact-floor (/ size 12)))
                                  (frame (square size 'solid 'white))))])])]))

;(cell-draw (Some (NumberedCell #\P 1)) 70)
;(cell-draw (Some (NumberedCell #\A 2)) 70)
;(cell-draw (Some (NumberedCell #\W 3)) 70)

(: check-cell : Cell (Optional Char) Integer Boolean -> Image)
;; checks a cell, and see if the character matches the corresponding letter
;; if it does, then this function will overlay a green square; if not, then a
;; red square. if it's a 'None cell, then nothing gets drawn
(define (check-cell cell char size check)
  (match* (cell check)
    [('None _) empty-image]
    [((Some (LetterCell a)) #t)
     (match char
       ['None empty-image]
       [(Some b)
        (if (char=? a b)
            (square size 'solid (color 0 255 0 50))
            (square size 'solid (color 255 0 0 50)))])]
    [((Some (NumberedCell a _)) #t)
     (match char
       ['None empty-image]
       [(Some b)
        (if (char=? a b)
            (square size 'solid (color 0 255 0 50))
            (square size 'solid (color 255 0 0 50)))])]
    [(_ _) empty-image]))

;(check-cell (Some (NumberedCell #\P 1)) 'None 70 #f)
;(check-cell (Some (NumberedCell #\A 2)) (Some #\B) 70 #t)
;(check-cell (Some (LetterCell #\W)) (Some #\D) 70 #t)

(: cell-list-draw : (Listof Cell) (Listof (Optional Char)) Integer
   (Optional Direction) (Optional Integer) Boolean -> Image)
;; this brings together the cell-draw and check-cell functions; it takes a list
;; of both cells and chars, and overlays them, creating a singular grid together
(define (cell-list-draw lst cs len dir cell-index check)
  (match* (lst cs)
    [('() _) empty-image]
    [((cons h1 t1) (cons h2 t2))
     (match cell-index
       ['None
        (beside
         (overlay
          (check-cell h1 h2 len check)
          (cell-draw h1 h2 len dir 'None))
         (cell-list-draw t1 t2 len dir 'None check))]
       [(Some int)
        (if (= int 0)
            (beside
             (overlay
              (check-cell h1 h2 len check)
              (cell-draw h1 h2 len dir (Some 0)))
             (cell-list-draw t1 t2 len dir 'None check))
            (beside
             (overlay
              (check-cell h1 h2 len check)
              (cell-draw h1 h2 len dir 'None))
             (cell-list-draw t1 t2 len dir (Some (- int 1)) check)))])]))

;(cell-list-draw nyt-mini-nov-10-2020-cells
;                (make-list (* (CrosswordPuzzle-num-cells-across puzzle)
;                              (CrosswordPuzzle-num-cells-down puzzle))
;                           'None) 70 (Some 'across) (Some 5) #f)

(: grid-drawer : Integer Integer (Listof Cell) Integer (Listof (Optional Char))
   (Optional Direction) (Optional Integer) Boolean -> Image)
;; draws any given list of cells arranged into rows and columns
;; uses cell-list-draw as the basis for each row by overlaying the (Listof Cell)
;; and (Listof (Optional Char))
(define (grid-drawer width height cells size cs dir cell-index check)
  (match cells
    ['() empty-image]
    [_
     (match cell-index
       ['None 
        (above
         (cell-list-draw (take width cells) (take width cs) size dir
                         'None check)
         (grid-drawer width height (drop width cells) size (drop width cs)
                      dir 'None check))]
       [(Some int)
        (if (>= int width)
            (above
             (cell-list-draw (take width cells) (take width cs) size dir
                             'None check)
             (grid-drawer width height (drop width cells) size (drop width cs)
                          dir (Some (- int width)) check))
            (above
             (cell-list-draw (take width cells) (take width cs) size dir
                             (Some int) check)
             (grid-drawer width height (drop width cells) size (drop width cs)
                          dir 'None check)))])]))

;(grid-drawer 5 5 nyt-mini-nov-10-2020-cells 70
;             (make-list
;              (* (CrosswordPuzzle-num-cells-across puzzle)
;                 (CrosswordPuzzle-num-cells-down puzzle))
;              'None) (Some 'down) (Some 9) #t)

(: answer-cell-draw : Cell Integer (Optional Direction)
   (Optional Integer) -> Image)
;; draws the cell with the letter displayed if it has one
(define (answer-cell-draw cell size dir index)
  (match cell
    ['None (frame (square size 'solid 'black))]
    [(Some (LetterCell l))
     (match index
       ['None
        (overlay
         (text (make-string 1 l) (cast (exact-floor
                                        (* 5 (/ size 6))) Byte) 'black)
         (frame (square size 'solid 'white)))]
       [(Some 0)
        (overlay
         (highlight size dir)
         (overlay
         (text (make-string 1 l) (cast (exact-floor
                                        (* 5 (/ size 6))) Byte) 'black)
         (frame (square size 'solid 'white))))])]
    [(Some (NumberedCell l n))
     (match index
       ['None
        (overlay
         (text (make-string 1 l) (cast (exact-floor (* 5 (/ size 6))) Byte)
               'black)
         (overlay/align/offset "left" "top"
                               (text (number->string n) (cast (exact-ceiling
                                                      (/ size 7)) Byte) 'black)
                               (* -1 (exact-floor (/ size 12))) (* -1
                                                      (exact-floor (/ size 12)))
                               (frame (square size 'solid 'white))))]
       [(Some 0)
        (overlay
         (highlight size dir)
         (overlay
         (text (make-string 1 l) (cast (exact-floor (* 5 (/ size 6))) Byte)
               'black)
         (overlay/align/offset "left" "top"
                               (text (number->string n) (cast (exact-ceiling
                                                      (/ size 7)) Byte) 'black)
                               (* -1 (exact-floor (/ size 12))) (* -1
                                                      (exact-floor (/ size 12)))
                               (frame (square size 'solid 'white)))))])]))

;(answer-cell-draw (Some (NumberedCell #\P 1)) 70)
;(answer-cell-draw (Some (NumberedCell #\A 2)) 70)
;(answer-cell-draw (Some (NumberedCell #\W 3)) 70)
  
(: answer-cell-list-draw : (Listof Cell) Integer (Optional Direction)
   (Optional Integer) -> Image)
;; draws a list of cells with their letters displayed if they have one
(define (answer-cell-list-draw lst len dir cell-index)
  (match lst
    ['() empty-image]
    [(cons head tail)
     (match cell-index
       ['None
        (beside
         (answer-cell-draw head len dir 'None)
         (answer-cell-list-draw tail len dir 'None))]
       [(Some int)
        (if (= int 0)
            (beside
             (answer-cell-draw head len dir (Some 0))
             (answer-cell-list-draw tail len dir 'None))
            (beside
             (answer-cell-draw head len dir 'None)
             (answer-cell-list-draw tail len dir (Some (- int 1)))))])]))

;(cell-list-draw nyt-mini-nov-10-2020-cells 70)

(: answer-grid-drawer : Integer Integer (Listof Cell) Integer
   (Optional Direction) (Optional Integer) -> Image)
;; draws a grid of cells with their correct letters displayed if they have one
(define (answer-grid-drawer width height cells size dir cell-index)
  (match cells
    ['() empty-image]
    [_
     (match cell-index
       ['None
        (above
         (answer-cell-list-draw (take width cells) size dir 'None)
         (answer-grid-drawer width height (drop width cells) size dir 'None))]
       [(Some int)
        (if (>= int width)
            (above
             (answer-cell-list-draw (take width cells) size dir 'None)
             (answer-grid-drawer width height (drop width cells) size dir
                                 (Some (- int width))))
            (above
             (answer-cell-list-draw (take width cells) size dir (Some int))
             (answer-grid-drawer width height (drop width cells) size dir 'None)
             ))])]))

;(answer-grid-drawer 5 5 nyt-mini-nov-10-2020-cells 70)

;;------------------------------------------------------------------------------
;; ======= drawing the clues helpers

(: clue-draw : Clue Integer (Optional Integer) -> Image)
;; this function will draw a clue, and will be highlighted if it matches the
;; direction of the currently selected cell
(define (clue-draw clue size clue-index)
  (match clue
    [(Clue n _ str)
     (match clue-index
       ['None (text (string-append (number->string n) ". " str)
                    (cast (exact-ceiling (/ size 7)) Byte) 'black)]
       [(Some int)
        (if (= n int)
            (overlay/align "left" "middle"
             (text (string-append (number->string n) ". " str)
                   (cast (exact-ceiling (/ size 7)) Byte) 'black)
             (rectangle (image-width
                         (text (string-append (number->string n) ". " str)
                               (cast (exact-ceiling (/ size 7)) Byte) 'black))
                        (image-height
                         (text (string-append (number->string n) ". " str)
                               (cast (exact-ceiling (/ size 7)) Byte) 'black))
                        'solid (color 156 14 182 50)))
            (text (string-append (number->string n) ". " str)
                  (cast (exact-ceiling (/ size 7)) Byte) 'black))])]))

;(clue-draw (Clue 1 'across "Piece starting on the second of seventh rank,
;in chess") 70)
;(clue-draw (Clue 5 'across "The \"O\" of B&O Railroad") 70)
;(clue-draw (Clue 6 'across "Where a river meets the sea") 70)

(: take : All (A) Integer (Listof A) -> (Listof A))
;; take the first n items from the front of the list
(define (take integer list)
  (match* (integer list)
    [(_ '()) '()]
    [(0 _) '()]
    [(_ (cons head tail)) (cons head (take (- integer 1) tail))]))

(: drop : All (A) Integer (Listof A) -> (Listof A))
;; drop the first n items from the front of the list 
(define (drop integer list)
  (match* (integer (reverse list))
    [(_ '()) '()]
    [(0 (cons f r)) (reverse (cons f r))]
    [(_ (cons f r)) (cond
                            [(< integer (length list))
                             (reverse (take (- (length list) integer)
                                            (cons f r)))]
                            [else '()])]))

(: merge (-> (Listof Clue) (Listof Clue) (Listof Clue)))
;; merges two sorted Clue lists into one sorted list
(define (merge lst1 lst2)
  (match* (lst1 lst2)
    [('() '()) '()]
    [(_ '()) lst1]
    [('() _) lst2]
    [((cons h1 t1) (cons h2 t2))
     (match* (h1 h2)
       [((Clue n1 _ _) (Clue n2 _ _))
        (if (< n1 n2)
            (cons h1 (merge t1 lst2))
            (cons h2 (merge lst1 t2)))])]))

(: merge-sort (-> (Listof Clue) (Listof Clue)))
;; takes in a list of Clues and returns a sorted list from small to larger
(define (merge-sort lst)
  (match lst
    ['() '()]
    [(cons head '()) lst]
    [_
     (local
       {(define l (take (quotient (length lst) 2) lst))
        (define r (drop (quotient (length lst) 2) lst))}
       (merge (merge-sort l) (merge-sort r)))]))

(: sort-clues-across : (Listof Clue) -> (Listof Clue))
;; this function will return a list of Clues that are in the 'across direction
(define (sort-clues-across lst)
  (match lst
    ['() '()]
    [(cons head tail)
     (if (symbol=? (Clue-direction head) 'across)
         (append (list head) (sort-clues-across tail))
         (sort-clues-across tail))]))
;; check-expect
(check-expect (sort-clues-across nyt-mini-nov-10-2020-clues) (list
      (Clue 1 'across "Piece starting on the second of seventh rank, in chess")
      (Clue 5 'across "The \"O\" of B&O Railroad")
      (Clue 6 'across "Where a river meets the sea")
      (Clue 8 'across "LSD, by another name")
      (Clue 9 'across "What Pac-Man eats")))

(: sort-clues-down : (Listof Clue) -> (Listof Clue))
;; this function will return a list of Clues that are in the 'down direction
(define (sort-clues-down lst)
  (match lst
    ['() '()]
    [(cons head tail)
     (if (symbol=? (Clue-direction head) 'down)
         (append (list head) (sort-clues-down tail))
         (sort-clues-down tail))]))
;; check-expect 
(check-expect (sort-clues-down nyt-mini-nov-10-2020-clues) (list
      (Clue 1 'down "Group of close friends and relatives, in 2020-speak")
      (Clue 2 'down "In front")
      (Clue 3 'down "Alt-rock band fronted by Jeff Tweedy")
      (Clue 4 'down "Cry in a game of tag")
      (Clue 7 'down "Fast-forwarded parts of podcasts")))

(: draw-clue-list : (Listof Clue) Integer (Optional Integer) -> Image)
;; this function will draw a given list of clues
(define (draw-clue-list lst size clue-index)
  (match lst
    ['() empty-image]
    [(cons head tail)
     (above/align "left"
                  (clue-draw head size clue-index)
                  (draw-clue-list tail size clue-index))]))

;(draw-clue-list nyt-mini-nov-10-2020-clues 70)

(: clues-drawer : (Listof Clue) Integer (Optional Direction)
   (Optional Integer) -> Image)
;; this function will draw all the clues of a given list, by first sorting them
;; into the corresponding directions. if a given clue is highlighted, it will
;; send that information to the helper functions which will do the highlight :)
(define (clues-drawer lst size dir clue-index)
  (if (<= (length lst) 50)
      (match* (dir clue-index)
        [((Some 'across) (Some int))
         (above/align "left"
               (text "ACROSS" (cast (exact-floor (/ size 3)) Byte) 'black)
               (square (cast (exact-floor (/ size 6)) Byte) 'solid 'white)
               (draw-clue-list (merge-sort (sort-clues-across lst))
                               size (Some int))
               (square (cast (exact-floor (/ size 5)) Byte) 'solid 'white)
               (text "DOWN" (cast (exact-floor (/ size 3)) Byte) 'black)
               (square (cast (exact-floor (/ size 6)) Byte) 'solid 'white)
               (draw-clue-list (merge-sort (sort-clues-down lst)) size 'None))]
        [((Some 'down) (Some int))
         (above/align "left"
               (text "ACROSS" (cast (exact-floor (/ size 3)) Byte) 'black)
               (square (cast (exact-floor (/ size 6)) Byte) 'solid 'white)
               (draw-clue-list (merge-sort (sort-clues-across lst)) size 'None)
               (square (cast (exact-floor (/ size 5)) Byte) 'solid 'white)
               (text "DOWN" (cast (exact-floor (/ size 3)) Byte) 'black)
               (square (cast (exact-floor (/ size 6)) Byte) 'solid 'white)
               (draw-clue-list (merge-sort (sort-clues-down lst))
                               size (Some int)))]
        [(_ _)
         (above/align "left"
               (text "ACROSS" (cast (exact-floor (/ size 3)) Byte) 'black)
               (square (cast (exact-floor (/ size 6)) Byte) 'solid 'white)
               (draw-clue-list (merge-sort (sort-clues-across lst)) size 'None)
               (square (cast (exact-floor (/ size 5)) Byte) 'solid 'white)
               (text "DOWN" (cast (exact-floor (/ size 3)) Byte) 'black)
               (square (cast (exact-floor (/ size 6)) Byte) 'solid 'white)
               (draw-clue-list (merge-sort (sort-clues-down lst))
                               size 'None))])
      (match* (dir clue-index)
        [((Some 'across) (Some int))
         (beside/align "top"
         (above/align "left"
               (text "ACROSS" (cast (exact-floor (/ size 3)) Byte) 'black)
               (square (cast (exact-floor (/ size 6)) Byte) 'solid 'white)
               (draw-clue-list (merge-sort (sort-clues-across lst))
                               size (Some int)))
               (square (cast (exact-floor (/ size 5)) Byte) 'solid 'white)
         (above/align "left"
               (text "DOWN" (cast (exact-floor (/ size 3)) Byte) 'black)
               (square (cast (exact-floor (/ size 6)) Byte) 'solid 'white)
               (draw-clue-list (merge-sort (sort-clues-down lst)) size 'None)))]
        [((Some 'down) (Some int))
         (beside/align "top"
         (above/align "left"
               (text "ACROSS" (cast (exact-floor (/ size 3)) Byte) 'black)
               (square (cast (exact-floor (/ size 6)) Byte) 'solid 'white)
               (draw-clue-list (merge-sort (sort-clues-across lst)) size 'None))
               (square (cast (exact-floor (/ size 5)) Byte) 'solid 'white)
         (above/align "left"
               (text "DOWN" (cast (exact-floor (/ size 3)) Byte) 'black)
               (square (cast (exact-floor (/ size 6)) Byte) 'solid 'white)
               (draw-clue-list (merge-sort (sort-clues-down lst))
                               size (Some int))))]
        [(_ _)
         (beside/align "top"
         (above/align "left"
               (text "ACROSS" (cast (exact-floor (/ size 3)) Byte) 'black)
               (square (cast (exact-floor (/ size 6)) Byte) 'solid 'white)
               (draw-clue-list (merge-sort (sort-clues-across lst)) size 'None))
               (square (cast (exact-floor (/ size 5)) Byte) 'solid 'white)
         (above/align "left"
               (text "DOWN" (cast (exact-floor (/ size 3)) Byte) 'black)
               (square (cast (exact-floor (/ size 6)) Byte) 'solid 'white)
               (draw-clue-list (merge-sort (sort-clues-down lst))
                               size 'None)))])
  ))

;(clues-drawer nyt-mini-nov-10-2020-clues 70)

;;------------------------------------------------------------------------------
;; ======= drawing the source/check helpers

(: source-drawer : (Optional String) Integer -> Image)
;; this function will draw the source in 
(define (source-drawer source size)
  (match source
    ['None empty-image]
    [(Some message)
     (text message (cast (exact-floor (/ size 7)) Byte) 'black)]))

(check-expect (source-drawer 'None 70) empty-image)
;(source-drawer (Some "https://www.nytimes.com/crosswords/game/mini,
;Nov 10 2020") 70)

(: check-drawer : Boolean Integer -> Image)
;; this function will display whether "check" is on or off
(define (check-drawer bool size)
  (match bool
    [#t (text "checking mode: on" (cast (exact-floor (/ size 7)) Byte) 'black)]
    [#f (text "checking mode: off" (cast (exact-floor (/ size 7)) Byte) 'black)]
    ))

;(check-drawer #t 70)
;(check-drawer #f 70)

;;------------------------------------------------------------------------------
;; ======= draw helpers

(: highlight : Integer (Optional Direction) -> Image)
;; highlight will create a highlighted, transparent arrow in the given direction
;; that will be overlayed on top of a white square when clicked on
(define (highlight size dir)
  (match dir
    ['None empty-image]
    [(Some 'across)
     (beside
      (rectangle (cast (exact-floor (* 4 (/ size 7))) Byte)
                 (cast (exact-floor (/ size 7)) Byte)
                 'solid (color 13 14 255 80))
      (rotate 30 (triangle (cast (exact-floor (* 3 (/ size 7))) Byte)
                           'solid (color 13 14 255 80))))]
    [(Some 'down)
     (above
      (rectangle (cast (exact-floor (/ size 7)) Byte)
                 (cast (exact-floor (* 4 (/ size 7))) Byte)
                 'solid (color 13 14 255 80))
      (rotate 180 (triangle (cast (exact-floor (* 3 (/ size 7))) Byte)
                           'solid (color 13 14 255 80))))]))

;(highlight 70 'down)
;(highlight 70 'across)

(: viz : CrosswordPuzzle Integer (Optional Integer) (Optional Integer)
   (Listof (Optional Char)) Boolean (Optional Direction) (Optional Integer)
   Boolean -> Image)
;; this is the main draw-helper function- it combines all the different
;; components together
(define (viz puzzle size x y cs bool dir clue-index check)
  (match puzzle
    [(CrosswordPuzzle width height cells clues source)
     (match cells
       ['() empty-image]
       [_
        (match* (x y)
          [('None 'None)
           (if (boolean=? bool #f)
               (above/align "middle"
                            (beside/align
                             "top" (grid-drawer width height cells size cs dir
                                                'None check)
                             (square (cast (exact-floor (/ size 6)) Byte)
                                     'solid 'white)
                             (clues-drawer clues size 'None 'None))
                            (square (cast (exact-floor (/ size 6)) Byte)
                                    'solid 'white)
                            (beside
                             (check-drawer check size)
                             (square (cast (exact-floor (/ size 6)) Byte)
                                     'solid 'white)
                             (source-drawer source size)))
               (above/align "middle"
                            (beside/align
                             "top" (answer-grid-drawer width height cells size
                                                       dir 'None)
                             (square (cast (exact-floor (/ size 6)) Byte)
                                     'solid 'white)
                             (clues-drawer clues size 'None 'None))
                            (square (cast (exact-floor (/ size 6)) Byte)
                                    'solid 'white)
                            (beside
                             (check-drawer check size)
                             (square (cast (exact-floor (/ size 6)) Byte)
                                     'solid 'white)
                             (source-drawer source size))))]
          [((Some a) (Some b))
           (if (boolean=? bool #f)
               (above/align "middle"
                            (beside/align
                             "top" (grid-drawer width height cells size cs dir
                                                (clicked-upon a b size puzzle)
                                                check)
                             (square (cast (exact-floor (/ size 6)) Byte)
                                     'solid 'white)
                             (clues-drawer clues size dir clue-index))
                            (square (cast (exact-floor (/ size 6)) Byte)
                                    'solid 'white)
                            (beside
                             (check-drawer check size)
                             (square (cast (exact-floor (/ size 6)) Byte)
                                     'solid 'white)
                             (source-drawer source size)))
               (above/align "middle"
                            (beside/align
                             "top" (answer-grid-drawer width height cells size
                                             dir (clicked-upon a b size puzzle))
                             (square (cast (exact-floor (/ size 6))
                                           Byte) 'solid 'white)
                             (clues-drawer clues size dir clue-index))
                            (square (cast (exact-floor (/ size 6)) Byte)
                                    'solid 'white)
                            (beside
                             (check-drawer check size)
                             (square (cast (exact-floor (/ size 6)) Byte)
                                     'solid 'white)
                             (source-drawer source size))))])])]))

;;------------------------------------------------------------------------------
;; ====== building the world

(: clicked-upon : Integer Integer Integer CrosswordPuzzle -> (Optional Integer))
;; given some click, clicked-upon will return the index of the cell it is
;; located in, if the click was within the grid; otherwise, it'll return 'None
(define (clicked-upon x y size puzzle)
  (match puzzle
    [(CrosswordPuzzle width height cells clues source)
     (if (and (< x (* width size)) (< y (* height size)))
         (local
           {(define x-mod (modulo x size))
            (define x-result (- x x-mod))
            (define x-num (inexact->exact (/ x-result size)))
            (define x-pos (cast x-num Integer))
            (define y-mod (modulo y size))
            (define y-result (- y y-mod))
            (define y-num (inexact->exact (/ y-result size)))
            (define y-pos (cast y-num Integer))
            (define final-pos (+ x-pos (* y-pos width)))
            (define val (list-ref cells final-pos))}
           (match val
             ['None 'None]
             [_ (Some final-pos)]))
         'None)]))

(check-expect (clicked-upon 900 400 70 nyt-mini-nov-10-2020) 'None)
(check-expect (clicked-upon 400 900 70 nyt-mini-nov-10-2020) 'None)
(check-expect (clicked-upon 40 50 70 nyt-mini-nov-10-2020) (Some 0))
(check-expect (clicked-upon 340 340 70 nyt-mini-nov-10-2020) (Some 24))
(check-expect (clicked-upon 250 180 70 nyt-mini-nov-10-2020) (Some 13))

(: replace-at : Integer (Optional Char) (Listof (Optional Char)) ->
   (Listof (Optional Char)))
;; I use this function in my react-to-keyboard function in order to replace
;; replace the (Optional Char) at the given position in the list with
;; whatever character the user presses
(define (replace-at i x xs)
  (local
    {(define len (length xs))}
    (cond
      [(>= i len) (error "todo: replace-at")]
      [else (match* (i xs)
              [(_ '()) '()]
              [(0 (cons head tail)) (cons x tail)]
              [(a (cons head tail)) (cons head (replace-at (- a 1) x tail))])])
    )
  )

(check-expect (replace-at 0 'None (list (Some #\t) 'None)) (list 'None 'None))
(check-expect (replace-at 4 (Some #\g) (list 'None (Some #\t)
                                             'None 'None 'None))
              (list 'None (Some #\t) 'None 'None (Some #\g)))
(check-error (replace-at 9 (Some #\f) (make-list 5 'None)) "todo: replace-at")

;; ====== clue-index-helpers
;; the purpose of the following 6 functions is to help transform the Click
;; into the index of the NumberedCell containing the current clue that
;; is being worked on, in the given direction

(: clue-index-helper : CrosswordPuzzle (Listof Integer) -> (Listof Cell))
;; given a puzzle and a list of indices, this function will return the cells
;; at each index within the cells component of the puzzle
(define (clue-index-helper puzzle indices)
  (match puzzle
    [(CrosswordPuzzle width height cells clues source)
     (match indices
       ['() '()]
       [(cons head tail)
        (local
          {(define cells-list (append (list (list-ref cells head))
                                    (clue-index-helper puzzle tail)))}
          cells-list)])]))

(check-expect (clue-index-helper lat-mini-nov-15-2020 (list 2 3 5 1))
                 (list (Some (NumberedCell #\I 3)) (Some (NumberedCell #\L 4))
                       (Some (NumberedCell #\R 6)) (Some (NumberedCell #\F 2))))
(check-expect (clue-index-helper nyt-mini-nov-10-2020 (list 2 4 5 24))
                 (list (Some (NumberedCell #\W 3)) 'None
                       (Some (NumberedCell #\O 5)) (Some (LetterCell #\S))))

(: clue-index-helper2 : CrosswordPuzzle Cell Direction -> Boolean)
;; this function will check to see if a given NumberedCell's direction
;; matches that of the user's current direction (because some NumberedCells
;; have two clues)
;; this is used as a helper function to the clue-index-helper3
(define (clue-index-helper2 puzzle cell dir)
  (match puzzle
    [(CrosswordPuzzle width height cells clues source)
     (match clues
       ['() #f]
       [(cons head tail)
        (match cell
          [(Some (NumberedCell _ n))
           (if (and (= (Clue-number head) n) (symbol=? (Clue-direction head)
                                                       dir))
               #t
               (clue-index-helper2 (CrosswordPuzzle width height cells
                                                tail source) cell dir))])])]))

(check-expect (clue-index-helper2 nyt-mini-nov-10-2020
                                  (Some (NumberedCell #\T 1)) 'across) #t)
(check-expect (clue-index-helper2 nyt-mini-nov-10-2020
                                  (Some (NumberedCell #\T 4)) 'across) #f)
(check-expect (clue-index-helper2 nyt-mini-nov-10-2020
                                  (Some (NumberedCell #\T 8)) 'across) #t)

(: clue-index-helper3 : CrosswordPuzzle (Listof Cell) Direction ->
   (Listof (Optional Integer)))
;; this function will, given the list of cells from clue-index-helper, return a
;; list of optional integers that correspond to the fact whether each cell is a
;; NumberedCell with the clue direction matching the current direction
(define (clue-index-helper3 puzzle lst dir)
  (match puzzle
    [(CrosswordPuzzle width height cells clues source)
     (match lst
       ['() '()]
       [(cons head tail)
        (match head
          ['None (append (list 'None) (clue-index-helper3 puzzle tail dir))]
          [(Some (LetterCell _)) (append (list 'None) (clue-index-helper3 puzzle
                                                                tail dir))]
          [(Some (NumberedCell _ n))
           (cond
             [(clue-index-helper2 puzzle head dir)
              (append (list (Some n)) (clue-index-helper3 puzzle tail dir))]
             [else
              (append (list 'None) (clue-index-helper3 puzzle tail dir))])
           ])])]))

(check-expect (clue-index-helper3 nyt-mini-nov-10-2020
                                  (list (Some (NumberedCell #\D 6))
                                        (Some (LetterCell #\E))
                                        (Some (LetterCell #\L))
                                        (Some (LetterCell #\T))) 'across)
              (list (Some 6) 'None 'None 'None))
(check-expect (clue-index-helper3 nyt-mini-nov-10-2020
                                  (list (Some (NumberedCell #\W 3))
                                        (Some (LetterCell #\I))
                                        (Some (LetterCell #\C))) 'down)
              (list (Some 3) 'None 'None))

(: clue-index-helper4 : (Listof (Optional Integer)) -> (Optional Integer))
;; given the list of optional integers from clue-index-helper3, this function
;; will return the optional integer that matches (Some n) which corresponds
;; to the clue-index. if it doesn't exist, then it returns 'None
(define (clue-index-helper4 lst)
  (match lst
    ['() 'None]
    [(cons head tail)
     (match head
       ['None (clue-index-helper4 tail)]
       [(Some n) (Some n)])]))

(check-expect (clue-index-helper4 (list 'None 'None 'None (Some 3) 'None))
              (Some 3))
(check-expect (clue-index-helper4 (list 'None 'None)) 'None)
(check-expect (clue-index-helper4 (list (Some 4) 'None 'None)) (Some 4))

(: vert-index-finder : Integer Integer -> Integer)
;; if the current direction is 'down, this function will return the indices
;; of all the cells that are above the current one, including the current one
(define (vert-index-finder n across)
  (local
    {(define counter 0)}
    (if (>= (- n across) 0)
        (+ counter 1 (vert-index-finder (- n across) across))
        counter)))

(check-expect (vert-index-finder 13 5) 2)
(check-expect (vert-index-finder 18 5) 3)
(check-expect (vert-index-finder 28 7) 4)

(: clue-index-finder : CrosswordPuzzle (Optional Integer) (Optional Direction)
   -> (Optional Integer))
;; this function will return the index of the clue under the given direction
;; that the current clicked-cell is in. for example, if the direction is 'across
;; then clue-index-finder will return the index of the NumberedCell to the left
;; of the current cell that contains the clue
(define (clue-index-finder puzzle index dir)
  (match index
    ['None 'None]
    [(Some n)
     (match (list-ref (CrosswordPuzzle-cells puzzle) n)
       ['None 'None]
       [_
        (match dir
          ['None 'None]
          [(Some 'across)
           (local
             {(define line-index (modulo n
                                 (CrosswordPuzzle-num-cells-across puzzle)))
              (define line-list (build-list (+ line-index 1) (lambda
                                                      ([x : Integer]) (- n x))))
              (define help-list (clue-index-helper puzzle line-list))
              (define final-list (clue-index-helper3 puzzle help-list 'across))
              (define across-clue (clue-index-helper4 final-list))}
             across-clue)]
          [(Some 'down)
           (local
             {(define across (CrosswordPuzzle-num-cells-across puzzle))
              (define line-index (vert-index-finder n across))
              (define line-list (build-list (+ line-index 1) (lambda
                                           ([x : Integer]) (- n (* x across)))))
              (define help-list (clue-index-helper puzzle line-list))
              (define final-list (clue-index-helper3 puzzle help-list 'down))
              (define down-clue (clue-index-helper4 final-list))}
             down-clue)])])]))

(check-expect (clue-index-finder nyt-mini-nov-10-2020 (Some 3) (Some 'across))
              (Some 1))
(check-expect (clue-index-finder nyt-mini-nov-10-2020 'None (Some 'across))
              'None)

(check-expect (clue-index-finder p2-test0 (Some 10) (Some 'across))
              'None)
(check-expect (clue-index-finder p2-test0 (Some 16) (Some 'across))
              (Some 5))

(check-expect (clue-index-finder nyt-mini-nov-10-2020 (Some 16) (Some 'down))
              (Some 2))
(check-expect (clue-index-finder nyt-mini-nov-10-2020 (Some 3) (Some 'down))
              (Some 4))
(check-expect (clue-index-finder nyt-mini-nov-10-2020 'None (Some 'down))
              'None)

(check-expect (clue-index-finder p2-test0 (Some 2) (Some 'down))
              (Some 2))
(check-expect (clue-index-finder p2-test0 (Some 7) (Some 'down))
              (Some 2))
(check-expect (clue-index-finder p2-test0 (Some 9) (Some 'down))
              'None)
(check-expect (clue-index-finder lat-mini-nov-15-2020 (Some 5) (Some 'down))
              (Some 1))

;;------------------------------------------------------------------------------
;; ======= run functions

(: draw : CrossWorld -> Image)
(define (draw world)
  (match world
    [(CrossWorld size puzzle click cd cci cs ds check)
     (match click
       ['None (viz puzzle size 'None 'None cs ds cd 'None check)]
       [(Some c)
        (match c
          [(Click x y)
           (local
             {(define clue-index (clue-index-finder
                                  puzzle (clicked-upon x y size puzzle) cd))}
             (viz puzzle size (Some x) (Some y) cs ds cd clue-index check))]
          )])]))

;; ======= letter-changes
;; the purpose of the following two functions are such: 
;; if the click lands on a LetteredCell and the direction is such that the next
;; cell in the corresponding direction is also a LetteredCell, then the click
;; and cci will change to the next cell to allow for more ease when entering
;; letters into the puzzle 
(: click-change : (Optional Direction) Integer CrosswordPuzzle
   (Optional Click) -> (Optional Click))
(define (click-change cd size puzzle click1)
  (match click1
    ['None 'None]
    [(Some (Click x y))
     (match (clicked-upon x y size puzzle)
       ['None 'None]
       [(Some int)
        (match puzzle
          [(CrosswordPuzzle across down _ _ _)
           (match cd
             ['None 'None]
             [(Some 'across)
              (if (= 0 (modulo (+ 1 int) across))
                  'None
                  (Some (Click (+ x size) y)))]
             [(Some 'down)
              (if (> (* across down) (+ 1 across int))
                  (Some (Click x (+ y size)))
                  'None)])])])]))

(: cci-change : (Optional Integer) (Optional Direction) CrosswordPuzzle
   Integer (Optional Click) -> (Optional Integer))
(define (cci-change cci cd puzzle size click1)
  (match click1
    ['None 'None]
    [(Some (Click x y))
     (match cci
       ['None 'None]
       [(Some int)
        (match* ((clicked-upon x y size puzzle) puzzle)
          [('None _) 'None]
          [((Some num) (CrosswordPuzzle across down _ _ _))
           (match cd
             ['None 'None]
             [(Some 'across)
              (if (= 0 (modulo (+ 1 num) across))
                  'None
                  (Some (+ 1 int)))]
             [(Some 'down)
              (if (> (* across down) (+ 1 across num))
                  (Some (+ across int))
                  'None)])])])]))

(: react-to-keyboard : CrossWorld String -> CrossWorld)
(define (react-to-keyboard world key)
  (match world
    [(CrossWorld size puzzle click cd cci cs ds check)
     (match key
       ["escape" (if (boolean=? ds #f)
                (CrossWorld size puzzle click cd cci cs #t check)
                (CrossWorld size puzzle click cd cci cs #f check))]
       ["`" (CrossWorld size puzzle 'None (Some 'across) 'None (make-list
                        (* (CrosswordPuzzle-num-cells-across puzzle)
                           (CrosswordPuzzle-num-cells-down puzzle))
                        'None) #f #f)]
       ["\t" (match cd
               [(Some 'across) (CrossWorld size puzzle click (Some 'down) cci
                                           cs ds check)]
               [(Some 'down) (CrossWorld size puzzle click (Some 'across) cci
                                         cs ds check)])]
       ["right" (CrossWorld size puzzle click (Some 'across) cci cs ds check)]
       ["down" (CrossWorld size puzzle click (Some 'down) cci cs ds check)]
       ["\r" (match check
               [#t (CrossWorld size puzzle click cd cci cs ds #f)]
               [#f (CrossWorld size puzzle click cd cci cs ds #t)])]
       [key (if (and (= (string-length key) 1)
                     (char-alphabetic? (list-ref (string->list key) 0)))
                (match cci
                  ['None world]
                  [(Some int)
                   (CrossWorld size puzzle (click-change cd size puzzle click)
                               cd (cci-change cci cd puzzle size click)
                          (replace-at int (Some (char-upcase (list-ref
                                                         (string->list key) 0)))
                                           cs) #f check)])
                world)])]))

(check-expect (react-to-keyboard (CrossWorld 70 nyt-mini-nov-10-2020 'None
                                             'None 'None '() #f #f) "escape")
              (CrossWorld 70 nyt-mini-nov-10-2020 'None
                                             'None 'None '() #t #f))
(check-expect (react-to-keyboard (CrossWorld 70 nyt-mini-nov-10-2020 'None
                                             'None 'None '() #t #f) "escape")
              (CrossWorld 70 nyt-mini-nov-10-2020 'None
                                             'None 'None '() #f #f))

(: react-to-mouse : CrossWorld Integer Integer Mouse-Event -> CrossWorld)
(define (react-to-mouse world x y e)
  (match e
    ["button-down"
     (match world
       [(CrossWorld size puzzle click cd cci cs ds check)
        (match (clicked-upon x y size puzzle)
          ['None (CrossWorld size puzzle 'None cd
                             (clicked-upon x y size puzzle) cs ds check)]
          [(Some int) (CrossWorld size puzzle (Some (Click x y)) cd
                                  (clicked-upon x y size puzzle) cs ds check)]
          )])]
    [_ world]))

(check-expect (react-to-mouse (CrossWorld 70 nyt-mini-nov-10-2020 'None
                                             'None 'None '() #t #f) 900 400
                                                                 "button-down")
              (CrossWorld 70 nyt-mini-nov-10-2020 'None
                                             'None 'None '() #t #f))
(check-expect (react-to-mouse (CrossWorld 70 nyt-mini-nov-10-2020 'None
                                             'None 'None '() #t #f) 900 400
                                                                 "move")
              (CrossWorld 70 nyt-mini-nov-10-2020 'None
                                             'None 'None '() #t #f))

(: run : Integer CrosswordPuzzle -> CrossWorld)
(define (run size puzzle)
  (big-bang (CrossWorld size puzzle 'None (Some 'across) 'None (make-list
                        (* (CrosswordPuzzle-num-cells-across puzzle)
                           (CrosswordPuzzle-num-cells-down puzzle))
                        'None) #f #f) : CrossWorld
    [to-draw draw]
    [on-key react-to-keyboard]
    [on-mouse react-to-mouse]))

;; ====== test run functions
;(run 70 nyt-mini-nov-10-2020)
;(run 70 p2-test0)
;(run 70 lat-mini-nov-15-2020)
;(run 70 (puzzle-from-file test_file1))
;(run 70 (puzzle-from-file test_file2))
;(run 70 (puzzle-from-file joshua_test))

(test)
