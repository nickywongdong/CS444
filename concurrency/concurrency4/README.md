# Problem 1:
A barbershop consists of a waiting room with n chairs, and the barber room containing the barber chair. If there are no customers to be served, the barber goes to sleep. If a customer enters the barbershop and all chairs are occupied, then the customer leaves the shop. If the barber is busy, but chairs are available, then the customer sits in one of the free chairs. If the barber is asleep, the customer wakes up the barber. Write a program to coordinate the barber and the customers.

Some useful constraints:

* Customers invoke get_hair_cut when sitting in the barber chair.
* If the shop is full, a customer exits the shop.
* The barber thread invokes cut_hair.
* cut_hair and get_hair_cut should always be executing concurrently, for the same duration.
* Your solution should be valid for any number of chairs.

Write concurrent code for customers and barbers that implements a solution to the above constraints.

# Problem 2:
Implement a solution to the cigarette smokers problem, as described in The Little Book of Semaphores, section 4.5.

From The Little Book of Semaphores section 4.5:
```
The cigarette smokers problem problem was originally presented by Suhas Patil
[8], who claimed that it cannot be solved with semaphores. That claim comes
with some qualifications, but in any case the problem is interesting and challenging.
Four threads are involved: an agent and three smokers. The smokers loop
forever, first waiting for ingredients, then making and smoking cigarettes. The
ingredients are tobacco, paper, and matches.
We assume that the agent has an infinite supply of all three ingredients, and
each smoker has an infinite supply of one of the ingredients; that is, one smoker
has matches, another has paper, and the third has tobacco.
The agent repeatedly chooses two different ingredients at random and makes
them available to the smokers. Depending on which ingredients are chosen, the
smoker with the complementary ingredient should pick up both resources and
proceed.
For example, if the agent puts out tobacco and paper, the smoker with the
matches should pick up both ingredients, make a cigarette, and then signal the
agent.
To explain the premise, the agent represents an operating system that allocates
resources, and the smokers represent applications that need resources. The
problem is to make sure that if resources are available that would allow one more
applications to proceed, those applications should be woken up. Conversely, we
want to avoid waking an application if it cannot proceed.
```