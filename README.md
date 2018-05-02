# chainsmoker

There are 3 major functions other than main i.e. Smoker, Pusher, Agent.

All of these have their respective threads for multithread programming and also semaphores to impliment process synchronization in order to deadlock

The Pusher plays the role of the middle man and takes and puts the ingredients from the Agent onto the table,and then allows only one of the smoker to access the ingrdients to make and smoke it.
