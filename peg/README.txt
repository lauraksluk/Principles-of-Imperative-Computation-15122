Peg Solitaire:

Files you won't modify:
   lib/peg-util.c1 - utilities for reading and printing boards
   lib/hdict.c1    - Hash dictionaries
   lib/stacks.c1   - Stacks
   peg-main.c1     - main function, to read in boards and call solver

Code to fill in:
   peg-moves.c1    - Peg solitaire, representation of moves
   peg1.c1         - Peg solitaire, simple solver
   peg2.c1         - Peg solitaire, memoizing solver

Data:
   german.txt      - A trivial-to-solve board
   english.txt     - A board that you may be able to solve with peg2.c1 if
                     you pick a very good move selection strategy (see
                     Appendix A)
   french1.txt     - A difficult-to-solve board (but one with a solution)
   french2.txt     - A difficult-to-solve board (but one with a solution)
   french3.txt     - A difficult-to-solve-board (but one with a solution)

==========================================================

Compiling and running on the german board with -d:
   % cc0 -d -w -o peg1 lib/peg-util.c1 lib/stacks.c1 peg-moves.c1 peg1.c1 peg-main.c1
   % ./peg1 german.txt

   % cc0 -d -w -o peg2 lib/*.c1 peg-moves.c1 peg2.c1 peg-main.c1
   % ./peg2 german.txt

Compiling and running on the english board. These direction give you
the fastest possible code, but use at your own risk!
   % cc0 -w -o peg1 -r unsafe -c-O2 lib/peg-util.c1 lib/stacks.c1 peg-moves.c1 peg1.c1 peg-main.c1
   % ./peg1 english.txt

   % cc0 -w -o peg2 -r unsafe -c-O2 lib/*.c1 peg-moves.c1 peg2.c1 peg-main.c1
   % ./peg2 english.txt

==========================================================

Submitting with Andrew handin script:
   % handin peg peg-moves.c1 peg1.c1 peg2.c1

Creating a tarball to submit with autolab.andrew.cmu.edu web interface:
   % tar -czvf handin.tgz peg-moves.c1 peg1.c1 peg2.c1
