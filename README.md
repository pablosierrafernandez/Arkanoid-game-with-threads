# Arkanoid-game-with-threads
🕹A basic Arkanoid or Breakout game in C with threads


**

## Summary

** 
A basic version of the Arkanoid or Breakout game. A rectangular game field is generated with an entrance/exit, a palette that moves with the keyboard to cover the exit, and a ball that bounces off the walls, the palette, and some blocks. There are three types of blocks: blocks that break and disappear (A), blocks that break, disappear, and create a new ball (B), and blocks that do not break and simply bounce the ball. If there are multiple balls, they will bounce off each other. When a ball goes through the exit, it disappears. The program ends when there are no balls on the field, there are no breakable blocks left, or the user presses the RETURN key. The game is won if all the breakable blocks are destroyed, and there is at least one ball on the field.
## Phases

To facilitate the design of the practice, 4 phases are proposed:

0.  Sequential program ('mur0.c'): Sequential version with a single ball and paddle. This version is provided to you in the laboratory 5.
    
1.  Creation of threads ('mur1.c'): Starting from the previous example, modify the functions that control the movement of the paddle and the ball so that they can act as independent execution threads. Thus, the main program will create a thread for the player (paddle) and a thread for the first ball. Then, the appearance of new balls when one of the blocks (B) is destroyed will be added. This phase will surely have deficiencies that will be solved in the next one.
    
2.  Thread synchronization ('mur2.c'): Complete the previous phase so that thread execution is synchronized when accessing shared resources (for example, the drawing environment, racing routines, necessary global variables). This way, the visualization problems presented by the mur1 version must be solved, which become more evident when working on a multiprocessor or server.
    
3.  Creation of processes ('mur3.c' / 'pilota3.c'): The objective of this phase is to learn basic concepts related to programming with multiple processes (multiprocess), so it is proposed to modify the game developed in phase 2 so that now processes are used instead of threads. Specifically, each ball will be controlled by a child process. The code of these child processes must reside in a different executable file than the executable file of the main program (also called the parent program).
    
4.  Process synchronization ('mur4.c' / 'pilota4.c'): In this phase, critical sections must be established to avoid the concurrency problems of the previous phase. It is also necessary to implement the functionality of communication between the various child processes (and perhaps also with the parent process) to practice with the mechanism of communication between processes.
