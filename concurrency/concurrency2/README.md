#Concurrency 2: The Dining Philosophers Problem

The purpose of these concurrent programming exercises is to hone your skills in thinking in parallel. This is a very important skill, to the point where many companies view it as being as fundamental as basic algebra or being able to write code. You will be writing a solution to the problem in class, without the use of computers. This will be submitted prior to leaving the class today. Make sure you keep a copy though, as you will implement your solution over the course of the next week.

For this exercise, you will be implementing a solution to the dining philosopher's problem. Image courtesy of Benjamin D. Esham / Wikimedia Commons.

![alt text](https://upload.wikimedia.org/wikipedia/commons/7/7b/An_illustration_of_the_dining_philosophers_problem.png "Dining philosophers around table")


The Dining Philosophers Problem was proposed by Dijkstra in 1965, when dinosaurs ruled the earth. It appears in a number of variations, but the standard features are a table with five plates, five forks (or chopsticks), and a big bowl of spaghetti. Five philosophers, who represent interacting threads, come to the table and execute the following loop: