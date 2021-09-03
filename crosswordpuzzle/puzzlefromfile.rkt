#lang typed/racket
(require "../include/cs151-core.rkt")
(require typed/test-engine/racket-tests)

;; ====== data structures/types
(define-type (Optional A)
  (U (Some A) 'None))

(define-struct (Some A)
  ([value : A]))

(define-type Direction
  (U 'down 'across))

(define-struct LetterCell
  ([letter : Char]))

(define-struct NumberedCell
  ([letter : Char]
   [number : Integer]))

(define-type Cell
  (Optional (U LetterCell NumberedCell)))

(define-struct Clue
  ([number : Integer]
   [direction : Direction]
   [text : String]))

(define-struct CrosswordPuzzle
  ([num-cells-across : Integer]
   [num-cells-down   : Integer]
   [cells : (Listof Cell)]
   [clues : (Listof Clue)]
   [source : (Optional String)]))

;; ====== helper functions from lab4-include.rkt file
(: first-split (-> (Listof Char) Char (Listof Char)))
;; Produces the list of characters up until the first given Char
(define (first-split str c)
  (match str
    ['() '()]
    [(cons head tail)
     (if (char=? head c)
         '()
         (cons head (first-split tail c)))]))

(check-expect (first-split (string->list "aa") #\,) (string->list "aa"))
(check-expect (first-split (string->list "aaa,bbbbb,b,b") #\,)
              (string->list "aaa"))
(check-expect (first-split (string->list "aaa/bbbbb,b,b") #\/)
              (string->list "aaa"))

(: rest-split (-> (Listof Char) Char (Listof Char)))
;; Produces the list of characters after the first given Char
(define (rest-split str c)
  (match str
    ['() '()]
    [(cons head tail)
     (if (char=? head c)
         tail
         (rest-split tail c))]))

(check-expect (rest-split (string->list "aa") #\,) (string->list ""))
(check-expect (rest-split (string->list "aaa/bbbbb,b") #\/)
              (string->list "bbbbb,b"))

(: list-split (-> (Listof Char) Char (Listof (Listof Char))))
;; Creates a list of list of Chars, separated based on the given Char
(define (list-split str c)
  (match str
    ['() '()]
    [_ (cons (first-split str c)
             (list-split (rest-split str c) c))]))

(check-expect (list-split (string->list "aa") #\,) (list (string->list "aa")))
(check-expect (list-split (string->list "aaa,bbbbb,b") #\,) 
              (list (string->list "aaa") 
                    (string->list "bbbbb") 
                    (string->list "b")))
(check-expect (list-split (string->list "aaa/bbbbb,b") #\/) 
              (list (string->list "aaa") (string->list "bbbbb,b")))

(: split-string (-> String Char (Listof String)))
;; Takes a string and splits it based on the given Char using list-split 
(define (split-string str c)
  (map list->string (list-split (string->list str) c)))

(check-expect (split-string "aa" #\,) (list "aa"))
(check-expect (split-string "aaa,bbbbb,b" #\,) 
              (list "aaa" "bbbbb" "b"))
(check-expect (split-string "aaa/bbbbb,b" #\/) 
              (list "aaa" "bbbbb,b"))

;; ====== helper functions for puzzle-from-file function
(: get-source : (Listof String) -> (Optional String))
;; gets the source from the file text if it exists
(define (get-source lst)
  (local
    {(define zero-line (list-ref lst 0))
     (define six (substring zero-line 0 6))}
    (if (string=? six "source")
        (Some (substring zero-line 7))
        'None)))

(check-expect (get-source '("source:New York Times Mini, Nov 27, 2020"
                            "-,1.F,2.O,3.I,4.L"
                            "-,5.L,U,K,E"))
              (Some "New York Times Mini, Nov 27, 2020"))
(check-expect (get-source
               '("source:New York Times, Monday Jan 25, 2016 by Ian Livengood"
                 "1.L,2.E,3.F,4.T,-,5.A,6.L,7.S,8.O,-,-,9.A,10.B,11.C,12.S"
                 "13.I,R,I,S,-,14.N,O,K,I,15.A,-,16.T,O,O,L"))
              (Some "New York Times, Monday Jan 25, 2016 by Ian Livengood"))
(check-expect (get-source '("7.O,R,C,A,-" "8.B,E,E,S,-"))
              'None)

(: get-across : (Listof String) -> Integer)
;; finds how many cells are in each row
(define (get-across lst)
  (local
    {(define new-lst (split-string (list-ref lst 0) #\,))}
    (length new-lst)))

(check-expect (get-across '("-,1.F,2.O,3.I,4.L"
                            "-,5.L,U,K,E")) 5)
(check-expect (get-across '("1,4,2,18,1,3")) 6)

(: get-down : (Listof String) -> Integer)
;; finds how many cells are in each column
(define (get-down lst)
  (length lst))

(check-expect (get-down '("-,1.F,2.O,3.I,4.L"
                          "-,5.L,U,K,E"
                          "6.J,A,N,E,T"
                          "7.O,R,C,A,-"
                          "8.B,E,E,S,-")) 5)
(check-expect (get-down '("1,3,4" "1,2,4"
                          "1,3,99" "10,0,0")) 4)
(check-expect (get-down '()) 0)

(: string->cell : String -> Cell)
;; in the (Listof String), it turns any given cell string, isolates
;; one of the elements separated by commas, and turns that into a Cell
;; operates on the assumption that the cell number is at most two digits
(define (string->cell str)
  (match str
    ["-" 'None]
    [s
     (if (number? (string->number (substring s 0 1)))
         (cond
           [(number? (string->number (substring s 1 2)))
            (Some (NumberedCell (string-ref str 3)
                                (cast (string->number (substring s 0 2))
                                      Integer)))]
           [else
            (Some (NumberedCell (string-ref str 2)
                                (cast (string->number (substring s 0 1))
                                      Integer)))])
         (Some (LetterCell (string-ref str 0))))]))

(check-expect (string->cell "15.L") (Some (NumberedCell #\L 15)))
(check-expect (string->cell "E") (Some (LetterCell #\E)))
(check-expect (string->cell "-") 'None)

(: get-cells : (Listof String) -> (Listof Cell))
;; creates a list of cells from the Listof String
(define (get-cells lst)
  (local
    {(define big-str (foldr (lambda ([l : String] [s : String])
           (string-append l "," s)) "" lst))
     (define str-lst (split-string big-str #\,))}
    (map string->cell str-lst)))

(check-expect (get-cells '("-,1.F,2.O,3.I,4.L"
                           "-,5.L,U,K,E"))
              (list
               'None (Some (NumberedCell #\F 1)) (Some (NumberedCell #\O 2))
               (Some (NumberedCell #\I 3)) (Some (NumberedCell #\L 4)) 'None
               (Some (NumberedCell #\L 5)) (Some (LetterCell #\U))
               (Some (LetterCell #\K)) (Some (LetterCell #\E))))

(: string->clue : String -> Clue)
;; in the (Listof String), it turns any given clue string and turns it
;; into a Clue 
;; operates on the assumption that the clue number is at most two digits
(define (string->clue str)
  (local
    {(define cc (list-ref (split-string str #\:) 1))}
    (cond
      [(number? (string->number (substring str 0 2)))
       (Clue
        (cast (string->number (substring str 0 2)) Integer)
        (if (char=? #\a (string-ref str 2))
            'across
            'down)
        cc)]
      [else 
       (Clue
        (cast (string->number (substring str 0 1)) Integer)
        (if (char=? #\a (string-ref str 1))
            'across
            'down)
        cc)])))

(check-expect (string->clue "5across:Eldest of Hollywood's Hemsworth brothers")
              (Clue 5 'across "Eldest of Hollywood's Hemsworth brothers"))
(check-expect (string->clue "24down:1/8 cup")
              (Clue 24 'down "1/8 cup"))

(: get-clues : (Listof String) -> (Listof Clue))
;; creates a list of clues from the List
(define (get-clues lst)
  (map string->clue lst))

(check-expect (get-clues '("31down:Insects in colonies"
                           "9across:\"Where does that guy get off?!\""
                           "34down:\"____ comes trouble!\""
                           "3across:*Inaptly named part of the elbow"
                           "36down:Where a cake is baked"
                           "37down:\"Piece of cake\""))
              (list (Clue 31 'down "Insects in colonies")
                    (Clue 9 'across "\"Where does that guy get off?!\"")
                    (Clue 34 'down "\"____ comes trouble!\"")
                    (Clue 3 'across "*Inaptly named part of the elbow")
                    (Clue 36 'down "Where a cake is baked")
                    (Clue 37 'down "\"Piece of cake\"")))

(: source? : (Listof String) -> Boolean)
;; returns true/false based on if there is a source
(define (source? lst)
  (match (get-source lst)
    ['None #f]
    [_ #t]))

(check-expect (source? '("source:New York Times Mini, Nov 27, 2020"
                         "-,1.F,2.O,3.I,4.L"
                         "-,5.L,U,K,E")) #t)
(check-expect (source?
               '("source:New York Times, Monday Jan 25, 2016 by Ian Livengood"
                 "1.L,2.E,3.F,4.T,-,5.A,6.L,7.S,8.O,-,-,9.A,10.B,11.C,12.S"
                 "13.I,R,I,S,-,14.N,O,K,I,15.A,-,16.T,O,O,L")) #t)
(check-expect (source? '("7.O,R,C,A,-" "8.B,E,E,S,-")) #f)

(: first-cell-index : (Listof String) -> Integer)
;; returns the index of the first string in the (Listof String) which contains
;; a Cell
;; returns either 0 or 1, depending on whether or not there is a source
(define (first-cell-index lst)
  (if (source? lst)
      1
      0))

(check-expect (first-cell-index '("source:New York Times Mini, Nov 27, 2020"
                                  "-,1.F,2.O,3.I,4.L"
                                  "-,5.L,U,K,E")) 1)
(check-expect (first-cell-index '("7.O,R,C,A,-" "8.B,E,E,S,-")) 0)

(: first-clue-index : (Listof String) -> Integer)
;; returns the index of the first string in the (Listof String) that contains
;; a clue
(define (first-clue-index lst)
  (local
    {(define counter 0)}
    (match lst
      ['() (error "no file was provided")]
      [(cons head tail)
       (if (or (string=? (substring head 1 7) "across")
               (string=? (substring head 1 5) "down"))
           counter
           (+ 1 counter (first-clue-index tail)))])))

(check-expect (first-clue-index (file->lines test_file1)) 6)
(check-expect (first-clue-index (file->lines test_file2)) 16)

;; ====== main puzzle-from-file functions
(: sublist : (Listof String) Integer (Optional Integer) -> (Listof String))
;; creates a sublist of a (Listof String) based on the indices
;; if the last index is 'None, then it returns the tail-end of the List
(define (sublist lst x oi)
  (local
    {(define new-list '())}
    (match oi
      ['None
       (if (< x (length lst))
           (append new-list (list (list-ref lst x)) (sublist lst (+ x 1) oi))
           new-list)]
      [(Some y)
       (if (< x y)
           (append new-list (list (list-ref lst x)) (sublist lst (+ x 1) oi))
           new-list)])))

(check-expect (sublist '("1" "2" "3" "4") 2 'None) '("3" "4"))
(check-expect (sublist '("1" "2" "#" "$" "%" "^" "&" "*") 3 (Some 6))
              '("$" "%" "^"))

(: puzzle-from-file : String -> CrosswordPuzzle)
;; creates the PuzzleWorld from the xwp text file
(define (puzzle-from-file text)
  (local
    {(define file (file->lines text))
     (define source (get-source file))
     (define first-cell (first-cell-index file))
     (define first-clue (first-clue-index file))
     (define cells (sublist file first-cell (Some first-clue)))
     (define clues (sublist file first-clue 'None))}
    (CrosswordPuzzle
     (get-across cells)
     (get-down cells)
     (get-cells cells)
     (get-clues clues)
     source)))

(test)