## Cache coherence experiment of CS4223 NUS
### Introduction
This simulator has four modules: processor, cache, bus, memory, supporting MESI and dragon protocol. The pipe links each module together, and modules communicate with each other by sending messages through pipe. This md first introduces pipe and definition of message, then it discusses each module's workflow in each cycle (supposed cycle N) seperately.
### pipe.h
The pipe is a circular linked list of elements which are sorted by the earlest cycle each element's message needed to be handled, ascending order. <br>
The element contain three variants: message and pointers pointing to next and pre elements.
### message.h
This file defines the message variant. The message contains five variants.<br>
>    cycle: the earlest cycle the message needed to be handled<br>
>    operation <br>
>    addr: address of memory block <br>
>    dest: destination of the message <br>
>    src: the message is sent by whom

Besides, there is priority in bus operation:
>    Flush > BusUpd > REPLY = BUSRD = BUSRDX
### simulator.c & simulator.h
Initializing simulator, parsing input and running simulator based on the configuration specified by input.
### processor.h
1.	Check the processor status. If status is busy, Goto 5. If status is waiting, Goto 
2.	Read new instruction.
3.	If it is LW/ST, sending message  (needed to be handled in cycle N+1) to cache with operation equalling to LOAD or STORE. Changing processor status to waiting. Return.
4.	If it is other instruction, Changing processor status to busy and setting processor's cycle equalling to the cycle that processor be free again. Return.
5.	If processor's clock equals to N, changing status to free, otherwise do nothing. Return.
6.	Check the pipe from cache, if there is message's operation equalling to reply, changing status to free, otherwise do nothing. Return.
### Cache.h & MESI.h & Dragon.h
1.	
### Bus.h
1.	Scan those pipes from each cache and memory. 
2.	If there is one message with message's cycle equals to or less than N, the bus sends it to dest.
3.	If there is multiple messages satisfying messages' cycle equal to or less than N, the bus chooses one based on priority of bus operation and sends it to dest.
### Memory.h
We supposed memory has infinite ports. <br>
The memory has a linked list records which blocks are in caches. <br>

1.	Scan the list. If there is a block which status equalling to writing back and the cycle finishing writing back equalling to N, we drop this node from list.
2.	For the message from bus, checking the operation.
3.	If it equals to FLUSH (write mem), we find the node with node's addr equalling to message's addr, and change its status to writing back and set its cycle to N+100
4.	If it equals to BUSRD or BUSRDX (read mem), we find the node with node's addr equalling to message's addr. If the block's status is writing back, we send a message with cycle setted to block's cycle+100 to bus. Otherwise, we send a message with cycle setted to N+100 to bus.  
### list.h
Providing some operations on linked list.
### Set up

### Command line options