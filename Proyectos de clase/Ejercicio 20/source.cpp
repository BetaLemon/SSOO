#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <wait.h>
#include <stdio.h>
#include <iostream>

int main(){
    int shmID = shmget(IPC_PRIVATE, sizeog(int), IPC CREAT|0666);
    int * vida = (int*)shmat(shmID, NULL, 0);
    if(fork() == 0){
        // Destructor

        exit(0);
    }

    // Constructor

    wait(&status);
    shmdt(vida);
    shmctl(shmID, IPC_RMID, NULL);
}

/*
int main(){


    pid_t pid;
    pid = fork();
    if(pid == 0){ // Hijo 1 - Destructor
        key_t shmKey = ftok("/home", 'a');
        int shmID = shmget(shmKey, sizeof(int), 0666);
        int *shmPTR = (int*)shmat(shmID, NULL, 0);

        while(*vida < 10000){
            *vida = *vida - 20;
            sleep(1);
        }
        shmdt(vida);
    }
    else{
        pid = fork();
        if(pid == 0){ // Hijo 2 - Constructor
            key_t shmKey = ftok("/home", 'a');
            int shmID = shmget(shmKey, sizeof(int), IPC CREAT|0666);
            int *shmPTR = (int*)shmat(shmID, NULL, 0);

            while(*vida < 10000){
                *vida = *vida + 20;
                sleep(1);
            }
            shmdt(vida);
            shmctl(shmID, IPC RMID, NULL);
        }
        else{

        }
    }
}*/

/*struct SharedMem {int status, int buffer};

int main(){
    key_t shmKey = ftok("/home", 'a');
    int shmID = shmget(shmKey, sizeof(int), IPC CREAT|0666);
    struct SharedMem *shmPTR = (struct SharedMem*)shmat(shmID, NULL, 0);
    shmPTR->buffer = -1;
    pid_t pid;
    pid = fork();
    if(pid == 0){ // Hijo 1

    }
    else{
        pid = fork();
        if(pid == 0){ // Hijo 2

        }
        else{
            pid = fork();
            if(pid == 0){   // Hijo 3

            }
            else{
                pid = fork();
                if(pid == 0){   // Hijo 4

                }
                else{ // PADRE
                    int vida = 10000;
                    if(shmPTR->buffer == -1){

                    }
                }
            }
        }
    }
}
*/
