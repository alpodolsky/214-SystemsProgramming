# Projects from Rutgers CS214
- projects in this class were mostly group projects(2 people per group)

# project1:
- Project1 had us simulate and recreate malloc, where we only start with a char array with a default size of 4096 and return a void pointer to the  user for each malloc call
- Besides malloc, we also had to create a slew of test cases that would check the correctness of the malloc'ing under specific circumstances.

# asst2
- asst2 introduced us to multithreading and multiprocessing and asked us to find a tradeoff of when threads are more efficient than processes and vice versa.
- the project required each executable to search for a specific value in the array(given by the driver) and return the result
- a driver was needed for this project as well but needed to be a little more thorough to ensure uniform testing for both multiprocessing and multithreading since they are both loaded as different executables. 
- the driver needed to first fill in an array of a specific size with values from 1 to n(where n is the size of the array) and then jumble at least 3/4ths of them. this was accomplished by using the rand() function on its own modded by the max array size. we used the regular rand() function because it kept the randomizer the same for both executables where seeding it first would lead to very different arrays across both executables.
- in the end we found threads work best for larger arrays(in the high thousands) and processes work best for arrays of smaller sizes.

# asst3
- asst3 required the most work out of the previous assignments and required us to create both a client and a server, specifically a client and server capable of handling a simplified version of a email service
- the client was in charge of making sure the user sent valid commands to the server as well as sending correct commands to the server.
- the server needed to be able to create message boxes with names, delete them, open and close them, and put messages into the boxes.
- since this was a simplified client/server project, boxes could not be created within each other and any messages put into the boxes were stored in a queue and deleted upon being sent from the server to the client.
