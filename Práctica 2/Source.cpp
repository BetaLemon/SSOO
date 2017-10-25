#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <wait.h>
#include <stdio.h>
#include <iostream>
#include <SFML/Graphics.hpp>



int main(){

    pid_t pid = fork();
    if(pid == 0){
        /// Tren Azul
        exit(0);
    }
    else{
        pid = fork();
        if(pid == 0){
            /// Tren Rojo
            exit(0);
        }
        else{
            /// Padre (dibuja el tablero)
            shmID = shmget(1100, sizeof(int), IPC_CREAT|0666);
            sf::Vector2f *posTren[2] = (sf::Vector2f[2]*)shmat(shmID, NULL, 0);
            while(true);
            exit(0);
        }
    }
    return 0;
}
