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

#define MAX 100
#define SIZE_TABLERO 64
#define SIZE_FILA_TABLERO 8
#define LADO_CASILLA 64
#define RADIO_AVATAR 25.f
#define OFFSET_AVATAR 5

#define SIZE_TABLERO 64
#define LADO_CASILLA 64
#define RADIO_AVATAR 25.f
#define OFFSET_AVATAR 5

/// Un simple enum que utilizo para indicar que tren mover en la funcion 'moverTren()'.
enum Tren{ AZUL, ROJO};

union semun
{
    // Este union no está definido en algunas versiones de Linux/UNIX.
    // Nosotros no lo tenemos en este Ubuntu y nos lo tenemos que picar.
    int val; /*Valor que se utiliza con SETVAL*/
    struct semid_ds* buf; /*buffer para IPC_STATS, IPC_SET --> Nosotros no lo utilizamos*/
    unsigned short* array; /*array para GETALL, SETALL*/
};

struct memComp {
    sf::Vector2i trenAzul[4];
    sf::Vector2i trenRojo[6];
    bool acabado[2]; //[0] = Azul, [1] = Rojo
};

/// Es una función que usamos para evitar repetir este trozo de código.
/// Es por comodidad y mejor comprensión; simplemente hace las instrucciones del Wait.
void WAIT(int semID){
    // WAIT:
    struct sembuf sb[1];
    sb[0].sem_num = 0;
    sb[0].sem_op = -1;
    sb[0].sem_flg = SEM_UNDO;
    semop(semID, sb, 1);
}

/// Es una función que usamos para evitar repetir este trozo de código.
/// Es por comodidad y mejor comprensión; simplemente hace las instrucciones del Signal.
void SIGNAL(int semID){
    struct sembuf sb[1];
    sb[0].sem_num = 0;
    sb[0].sem_op = 1;
    sb[0].sem_flg = SEM_UNDO;
    semop(semID, sb, 1);
}

/// Esta función mueve al tren que se le indique con 'queTren'. Recorre las posiciones del array del Vec2i de ese tren y desplaza en -1.
void moverTren(struct memComp *ptr, Tren queTren){
    if(queTren == Tren::AZUL){
        int tmp;
        for(int i = 0; i < 4; i++){
            tmp = ptr->trenAzul[i].x;
            if(tmp == 0){   // Para reiniciar la posicion si llega al borde de la pantalla.
                tmp = 7;
            }
            else{
                tmp--;
            }
            ptr->trenAzul[i].x = tmp;
        }
    }
    else if(queTren == Tren::ROJO){
        int tmp;
        for(int i = 0; i < 6; i++){
            tmp = ptr->trenRojo[i].y;
            if(tmp == 0){   // Para reiniciar la posicion si llega al borde de la pantalla.
                tmp = 7;
            }
            else{
                tmp--;
            }
            ptr->trenRojo[i].y = tmp;
        }
    }
    else{
        std::cout << "Tren no válido." << std::endl;
    }
}

sf::Vector2i TransformaCoordenadaACasilla(int _x, int _y)
{
    int xCasilla = _x/LADO_CASILLA;
    int yCasilla = _y/LADO_CASILLA;
    sf::Vector2i casilla(xCasilla, yCasilla);
    return casilla;
}

sf::Vector2i BoardToWindows(sf::Vector2i _position)
{
    return sf::Vector2i(_position.x*LADO_CASILLA+OFFSET_AVATAR, _position.y*LADO_CASILLA+OFFSET_AVATAR);
}

void DibujaSFML(struct memComp * shmPTR)
{
    sf::Vector2f casillaOrigen, casillaDestino;

    sf::RenderWindow window(sf::VideoMode(512,512), "El Gato y el Raton");
    while(window.isOpen() && (!(shmPTR->acabado[0]) || !(shmPTR->acabado[1])))
    {
        sf::Event event;

        //Este primer WHILE es para controlar los eventos del mouse
        while(window.pollEvent(event))
        {
            switch(event.type)
            {
                case sf::Event::Closed:
                    window.close();
                    break;
                case sf::Event::MouseButtonPressed:
                    if (event.mouseButton.button == sf::Mouse::Left)
                    {
                        // no lo usamos.
                    }
                    break;
                default:
                    break;

            }
        }

        window.clear();

        //A partir de aquí es para pintar por pantalla
        //Este FOR es para el tablero
        for (int i =0; i<8; i++)
        {
            for(int j = 0; j<8; j++)
            {
                sf::RectangleShape rectBlanco(sf::Vector2f(LADO_CASILLA,LADO_CASILLA));
                rectBlanco.setFillColor(sf::Color::White);
                if(i%2 == 0)
                {
                    //Empieza por el blanco
                    if (j%2 == 0)
                    {
                        rectBlanco.setPosition(sf::Vector2f(i*LADO_CASILLA, j*LADO_CASILLA));
                        window.draw(rectBlanco);
                    }
                }
                else
                {
                    //Empieza por el negro
                    if (j%2 == 1)
                    {
                        rectBlanco.setPosition(sf::Vector2f(i*LADO_CASILLA, j*LADO_CASILLA));
                        window.draw(rectBlanco);
                    }
                }
            }
        }

        // TREN AZUL:

        sf::CircleShape shapeTrenAzul(RADIO_AVATAR);
        shapeTrenAzul.setFillColor(sf::Color::Blue);
        sf::Vector2i posTrenAzul;
        for(int i = 0 ; i<4 ; i++){ // Para cada uno de los vagones del tren.
            posTrenAzul = shmPTR->trenAzul[i];
            posTrenAzul = BoardToWindows(posTrenAzul);
            shapeTrenAzul.setPosition((sf::Vector2f)posTrenAzul);
            window.draw(shapeTrenAzul);
        }

        // TREN ROJO:

        sf::CircleShape shapeTrenRojo(RADIO_AVATAR);
        shapeTrenRojo.setFillColor(sf::Color::Red);
        sf::Vector2i posTrenRojo;
        for(int i = 0 ; i<6 ; i++){ // Para cada uno de los vagones del tren.
            posTrenRojo = shmPTR->trenRojo[i];
            posTrenRojo = BoardToWindows(posTrenRojo);
            shapeTrenRojo.setPosition((sf::Vector2f)posTrenRojo);
            window.draw(shapeTrenRojo);
        }

        window.display();
    }

}

int main(){
    // Inicializamos la memoria compartida:
    int shmID = shmget(1100, sizeof(int), IPC_CREAT|0666);
    if(shmID < 0){ std::cout << "Error creando la memoria compartida."; };
    struct memComp * shmPTR = (struct memComp*)shmat(shmID,NULL, 0);
    if(shmPTR < 0){ std::cout << "Error al crear el puntero."; };

    // Inicializamos los semáforos:
    int semID = semget(IPC_PRIVATE, 1, IPC_CREAT|0600);
    semctl(semID, 0, SETVAL, 1);

    int itCount = 0; // El contador del iterador, para que mueva solo 20 veces.

    shmPTR->acabado[0] = false;
    shmPTR->acabado[1] = false;

    pid_t pid = fork();
    if(pid == 0){
        /// Tren Azul
        Tren soy = Tren::AZUL;
        // Inicialización de las posiciones:
        shmPTR->trenAzul[0].x = 3;
        shmPTR->trenAzul[0].y = 2;
        for(int i = 1 ; i < 4; i++){
            shmPTR->trenAzul[i].x = (3+i)%8;
            shmPTR->trenAzul[i].y = 2;
        }
        // Loop del movimiento del tren azul:
        while(itCount < 20){
            // Comprueba si esta apunto de entrar en la RC y hace el Wait.
            if(shmPTR->trenAzul[0].x == 3 && shmPTR->trenAzul[0].y == 2){
                WAIT(semID);
            }
            // Mueve el tren, y augmenta el contador:
            moverTren(shmPTR, soy);
            itCount++;
            // Comprobamos si ha salido de la RC, y hacemos el Signal en caso afirmativo.
            if(shmPTR->trenAzul[3].x == 1 && shmPTR->trenAzul[3].y == 2){
                SIGNAL(semID);
            }
            sleep(1);   // Para que se mueva cada segundo.
        }
        std::cout << "El tren azul ha hecho los 20 movimientos." << std::endl;
        shmPTR->acabado[0] = true;
        exit(0);
    }
    else{
        pid = fork();
        if(pid == 0){
            /// Tren Rojo
            Tren soy = Tren::ROJO;
            // Inicialización de las posiciones:
            shmPTR->trenRojo[0].x = 2;
            shmPTR->trenRojo[0].y = 3;
            for(int i = 1 ; i < 6; i++){
                shmPTR->trenRojo[i].x = 2;
                shmPTR->trenRojo[i].y = (3+i)%8;
            }

            // Loop del movimiento del tren rojo:
            while(itCount < 20){
                // Comprueba si esta apunto de entrar en la RC y hace el Wait.
                if(shmPTR->trenRojo[0].x == 2 && shmPTR->trenRojo[0].y == 3){
                    WAIT(semID);
                }
                // Mueve el tren, y augmenta el contador:
                moverTren(shmPTR,soy);
                itCount++;
                // Comprobamos si ha salido de la RC, y hacemos el Signal en caso afirmativo.
                if(shmPTR->trenRojo[5].x == 2 && shmPTR->trenRojo[5].y == 1){
                    SIGNAL(semID);
                }
                sleep(2);   // Para que se mueva cada 2 segundos.
            }
            std::cout << "El tren rojo ha hecho los 20 movimientos." << std::endl;
            shmPTR->acabado[1] = true;
            exit(0);
        }
        else{
            /// Padre (dibuja el tablero)

            // Dibujamos el tablero:
            DibujaSFML(shmPTR);

            // Recibimos el estado de los hijos, para evitar zombies:
            int status;
            wait(&status);
            wait(&status);

            // Destruimos los semforos:
            semctl(semID, 0, IPC_RMID, NULL);
            // Hacemos el detach de la memoria compartida:
            shmdt(shmPTR);
            // Destruimos la memoria compartida:
            shmctl(shmID, IPC_RMID, NULL);
        }
    }
    return 0;
}
