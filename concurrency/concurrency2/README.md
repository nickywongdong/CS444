#Concurrency 2: The Dining Philosophers Problem

The purpose of these concurrent programming exercises is to hone your skills in thinking in parallel. This is a very important skill, to the point where many companies view it as being as fundamental as basic algebra or being able to write code. You will be writing a solution to the problem in class, without the use of computers. This will be submitted prior to leaving the class today. Make sure you keep a copy though, as you will implement your solution over the course of the next week.

For this exercise, you will be implementing a solution to the dining philosopher's problem. Image courtesy of Benjamin D. Esham / Wikimedia Commons.

![alt text](https://upload.wikimedia.org/wikipedia/commons/7/7b/An_illustration_of_the_dining_philosophers_problem.png "Dining philosophers around table")


The Dining Philosophers Problem was proposed by Dijkstra in 1965, when dinosaurs ruled the earth. It appears in a number of variations, but the standard features are a table with five plates, five forks (or chopsticks), and a big bowl of spaghetti. Five philosophers, who represent interacting threads, come to the table and execute the following loop:

```
while True:
	think()
	get_forks()
	eat()
	put_forks()
```

Assuming that the philosophers know how to think() and eat(), write a solution to this problem

There are several synchronization constraints that we need to enforce to make this system work correctly:
* A single fork can only be held by a single philosopher
* It must be impossible for deadlock to occur
* Starvation is disallowed
* More than a single philosopher must be capable of eating simultaneously
* All must be alive at program termination. This means no implementing the solution where philosophers murder their neighbors.
* Only adjacent forks are accessible to a given philosopher.
* Both adjacent forks are required to eat.

Write concurrent code which implements a solution which satisfies the above description. A few important details:

* Each bowl of noodles is infinite.
* Eating takes a random amount of time in the range of 2-9 seconds.
* Thinking takes a random amount of time in the range of 1-20 seconds.
* The program should run indefinitely until the user kills it.
* You must have output that makes sense, and shows the correctness of your solution.
   * This includes the status of all forks at all times.
   * The status of each philosopher, as well.
* You are welcome to name your philosophers whatever you want, so long as they are named after real people.
* Use whatever synchronization construct you feel is appropriate.
* While you are not constrained to any given language, it is required to run on os2. There is an (older) installation of mono on os2 in /scratch/bin (--prefix=/scratch, for reference). In other words, C#, C++, C, python, ruby, perl, etc. are all usable. That said, your program *must* truly run concurrently!
* Parallelism is also up to you. pthreads, C++threads, Boost.Thread, OpenMP, Intel TBB, whatever. It's all up to you. Again, just make sure it runs on os2.