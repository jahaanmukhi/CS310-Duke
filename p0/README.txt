Compsci 310
Assignment p0
Michael Liu
msl40
1/30/18

Resources Used: StackOverflow and Piazza
Poeple Consulted: No one :)
Time Spent: 10 hours
Impressions: At first I tried to do the assignment with an explicit free list and the pointer logic was very difficult. Then someone who has taken the class before suggested using an implicit free list. I tried this and it was much easier. 

Problems: I'm not sure why, but when I change the MAX_HEAP_SIZE to some value like 1024*1024*32 (32 MB), I get a segfault but this doesn't happen for (4MB). This may be why I pass 3 of the 4 stress tests on autograder but fail one.
