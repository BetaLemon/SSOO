#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <wait.h>
#include <stdio.h>
#include <iostream>
#include <SFML/Graphics.hpp>

struct memComp {
    sf::Vector2f trenAzul[4];
    sf::Vector2f trenRojo[6];
}

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
            int shmID = shmget(1100, sizeof(int), IPC_CREAT|0666);
                if(shmID < 0){ std::cout << "Error creando la memoria compartida." };
            struct memComp * shmPTR = (struct memComp*)shmat(shmID,NULL, 0);
                if(shmPTR < 0){ std::cout << "Error al crear el puntero." };

            while(true);
            exit(0);
        }
    }
    return 0;
}
