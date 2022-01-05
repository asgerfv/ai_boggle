<pre>

-:  Boggle Solver, July 2017 ::---------------------------------------------

by Asger of Rota Galaxy



---------: Overall ::----------------------------------------------------------

An extremely fast AI for solving a game of Boggle.

I stumpled upon this interesting problem on how to solve a game of Boggle (or
Scrapple).

First a simple approach was implemented which proved clean and easy to grasp.

Afterwards I added an interface in order to try different techniques. One of
those was a "Trie" which proved to be very fast.
To make it faster I added different techniques like pooling and minimizing the
memory footprint of each trie.

Once this was optimized I meassured it against other public solvers on the
inet and this one was by far the fastest. I then took that fastest version
made it run in parallel.
First I just divided the dictionary up so that every word was shuffled evenly
among the threads. This didn't play well with a Trie, and the result was a
doubled memory increase, and as such worse performance than the single threaded
approach.
Instead I divided the work set according to the first letter of the word, so
thread 0 would get all words starting with letter A, thread 1 got all words
with B and so on.

This proved so fast that even a board with 1000x1000 and a dictionary with
almost 300.000 words gets done in less than 2 seconds.

I therefore created a massive 10.000 x 10.000 board with a dictionary of 180K
words. This completed in 12 minutes using the initial trie approach, and ended
up under at 2 minutes using 4 worker threads.


Tested on Windows 7 x64 and OSX El Capitain.


---------: How to run ::-------------------------------------------------------

* At the program root, type:

$ cmake -DCMAKE_BUILD_TYPE=Release .
$ make
$ ./game_boggle

Will execute test in: regression_qu1
Found 3 words for a score of 6
  Total time: 1 ms
  LoadDictionary : 0 ms
  LoadingBoard : 0 ms
  FindWords : 0 ms
Test passed!
Will execute test in: regression_qu2
Found 4 words for a score of 6
  Total time: 2 ms
  LoadDictionary : 0 ms
  LoadingBoard : 0 ms
  FindWords : 1 ms
Test passed!
Will execute test in: regression_ensure-non-duplicates
Found 0 words for a score of 0
  Total time: 0 ms
  LoadDictionary : 0 ms
  LoadingBoard : 0 ms
  FindWords : 0 ms
Test passed!
Will execute test in: performance_huge
Found 44211 words for a score of 229797
  Total time: 157 ms
  LoadDictionary : 75 ms
  LoadingBoard : 0 ms
  FindWords : 81 ms
Test passed!
Will execute test in: performance_monster
Found 43980 words for a score of 177438
  Total time: 211 ms
  LoadDictionary : 101 ms
  LoadingBoard : 2 ms
  FindWords : 107 ms
Test passed!
Will execute test in: performance_titan
Found 131809 words for a score of 905138
  Total time: 1726 ms
  LoadDictionary : 145 ms
  LoadingBoard : 10 ms
  FindWords : 1570 ms
Test passed!
Will execute test in: performance_titans-creator
Found 152181 words for a score of 1283115
  Total time: 133299 ms
  LoadDictionary : 68 ms
  LoadingBoard : 1198 ms
  FindWords : 132032 ms
Test passed!

* The transcript above was from a run on a Macbook Pro with
  Intel i7-5557U CPU @ 3.10GHz


---------: TODO ::-------------------------------------------------------------

Since the potential for further optimizations on this is huge, I've
included these random ideas for future improvements:

* Do a quick clipping check to see if the current position is at an edge of the
  board, and if so, select a different "neighbour check".

* Do a <tolower> on each word while building the dictionary, just to ensure that
  we don't have to do InCase comparisons.

* Compress the board - and word list, to only 'a' -> 'z' == 25 chars (5 bits).

* Compress the Trie's children list.


-------------------------------------------------------------------------------
