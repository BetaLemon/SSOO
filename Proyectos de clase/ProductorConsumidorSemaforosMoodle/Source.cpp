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

#define N 10
#define ITERACIONES 100

/**
 * Este union no está definido en algunas versiones de Linux/UNIX.
 * Nosotros no lo tenemos en este Ubuntu y nos lo tenemos que picar.
 */
union semun
{
    int val; /*Valor que se utiliza con SETVAL*/
    struct semid_ds* buf; /*buffer para IPC_STATS, IPC_SET --> Nosotros no lo utilizamos*/
    unsigned short* array; /*array para GETALL, SETALL*/
};


/**
 *  Datos que se guardan en la memoria compartida
 */
struct MemoriaCompartida
{
    int iLectura=0; /*Indice en el que toca leer*/
    int iEscritura=0; /*Indice en el que toca escribir*/
    int buffer[N]; /*Array en el que se almacenan los items*/
};

/**
 *  Esta función se llama desde el proceso que hace de productor.
 */
void FuncionProductor(struct MemoriaCompartida* _memoriaCompartida, int _semID)
{
    int item, ok;

    struct sembuf semBuf[1]; /*Este array de struct sembuf se utiliza en la función semop para indicar qué operaciones queremos hacer sobre los semáforos.
                                Haremos sólo una operación a la vez sobre cada semáforo. (1) Para ver si hay espacio para escribir y (2) para avisar de que el buffer tiene un item más.*/
    /*Esta es su estructura interna: https://www.screencast.com/t/A2nWvr4bdxAw */

    semBuf[0].sem_flg = SEM_UNDO; /*Con este flag aseguramos que si el proceso muere no se deja bloqueado el semáforo. */
                                  /*Tambien tenemos el flag IPC_NOWAIT (revisar para qué sirve)*/

    /*Se producen ITERACIONES ítems*/
    for (int i = 0; i < ITERACIONES; i++)
    {
        item = i; //El valor del ítem hacemos que coincida con el número de la iteración.

        semBuf[0].sem_num = 0; /*Esta operación se hará sobre el semáforo que hay en la posición 0*/
        semBuf[0].sem_op = -1; /*Estamos haciendo un WAIT*/
        ok = semop(_semID, semBuf, 1); /*El tercer parmetro es el número de operaciones que contiene el array de sembuf. En nuestro caso, 1.*/
        if (ok < 0)
        {
            std::cout << "Error al bloquear el productor al querer escribir." <<std::endl;
            exit(10);
        }

        //EMPIEZA LA REGIÓN CRITICA
        _memoriaCompartida->buffer[_memoriaCompartida->iEscritura] = item; //Almaceno el item en la posición que toca del buffer.
        std::cout << "Meto "<<item<<" en la posicion "<<_memoriaCompartida->iEscritura<<std::endl;
        _memoriaCompartida->iEscritura = (_memoriaCompartida->iEscritura + 1) % N; //Incremento el índice de escritura
        /*Aquí empiezo a preparar la salida de la zona crítica haciendo un signal al segundo semáforo*/
        semBuf[0].sem_num = 1; /*Esta operación se hará sobre el semáforo que hay en la posición 1*/
        semBuf[0].sem_op = 1; /*Estamos haciendo un SIGNAL*/
        ok = semop(_semID, semBuf, 1);
        //ACABA LA REGION CRITICA

        if (ok < 0)
        {
            std::cout << "Error al hacer signal desde el productor." <<std::endl;
            exit(11);
        }
    }
}

/**
 *  Esta función se llama desde el proceso que hace de consumidor.
 */
void FuncionConsumidor(struct MemoriaCompartida* _memoriaCompartida, int _semID)
{
    int item, ok;
    struct sembuf semBuf[1]; /*Este array de struct sembuf se utiliza en la función semop para indicar qué operaciones queremos hacer sobre los semáforos.
                                Haremos sólo una operación a la vez sobre cada semáforo. (1) Para ver si hay algún ítem que leer y (2) para avisar de que ya hay espacio para escribir.*/
    /*Esta es su estructura interna: https://www.screencast.com/t/A2nWvr4bdxAw */
    semBuf[0].sem_flg = SEM_UNDO; /*Con este flag aseguramos que si el proceso muere no se deja bloqueado el semáforo. */
                                  /*Tambien tenemos el flag IPC_NOWAIT (revisar para qué sirve)*/

    /*Se producen ITERACIONES ítems*/
    for (int i = 0; i < ITERACIONES; i++)
    {
        semBuf[0].sem_num = 1; /*Esta operación se hará sobre el semáforo que hay en la posición 1*/
        semBuf[0].sem_op = -1; /*Estamos haciendo un WAIT*/
        ok = semop(_semID, semBuf, 1); /*El tercer parmetro es el número de operaciones que contiene el array de sembuf. En nuestro caso, 1.*/
        if (ok < 0)
        {
            std::cout << "Error al bloquear el consumidor porque no hay nada que leer." <<std::endl;
            exit(20);
        }
        //EMPIEZA LA REGIÓN CRITICA
        item = _memoriaCompartida->buffer[_memoriaCompartida->iLectura]; /*Leemos el ítem que hay en la posición donde toca leer*/
        std::cout << "He leido " << item << " en la posicion " << _memoriaCompartida->iLectura << std::endl;
        _memoriaCompartida->iLectura = (_memoriaCompartida->iLectura + 1) % N; /*Aumentamos el índice de lectura*/
        /*Aquí empiezo a preparar la salida de la zona crítica haciendo un signal al primer semáforo*/
        semBuf[0].sem_num = 0; /*Esta operación se hará sobre el semáforo que hay en la posición 0*/
        semBuf[0].sem_op = 1; /*Estamos haciendo un SIGNAL*/
        ok = semop(_semID, semBuf, 1);
        //ACABA LA REGION CRITICA
        if (ok < 0)
        {
            std::cout << "Error al hacer signal desde consumidor." <<std::endl;
            exit(20);
        }

    }
}

int main()
{
    /**
     * Creación de la zona de memoria compartida
     */
    int shmID = shmget(IPC_PRIVATE, sizeof(struct MemoriaCompartida), IPC_CREAT|0666);
    if (shmID < 0)
    {
        std::cout << "Error al crear zona de memoria compartida\n";
        exit(1);
    }
    /**
     * Creación del puntero de acceso a la zona de memoria compartida (attach)
     */
    struct MemoriaCompartida* shmPTR = (struct MemoriaCompartida*)shmat(shmID, NULL, 0);
    if (shmPTR < 0)
    {
        std::cout<<"Error al crear puntero\n";
        exit(2);
    }


    /**
     * Creación de los semáforos: Un semáforo para controlar las escrituras y otro para las lecturas
     */
    int semID = semget(IPC_PRIVATE, 2, IPC_CREAT|0600);
    if (semID < 0)
    {
        std::cout<<"Error al crear semaforo.\n";
        exit(3);
    }
    /**
     * Este es el semáforo que controlará las escrituras. Como al ppio está vacio, caben N entradas.
     */
    union semun arg;
    arg.val = N;
    int okSem = semctl(semID, 0, SETVAL, arg);
    if (okSem < 0)
    {
        std::cout<<"El semaforo no se ha podido poner a N.\n";
        exit(1);
    }

    /**
     * Este es el semáforo que controlará las lecturas. Como al ppio está vacio, no se puede leer nada y
     * lo ponemos a cero.
     */
    arg.val = 0;
    okSem = semctl(semID, 1, SETVAL, arg);
    if (okSem < 0)
    {
        std::cout<<"El semaforo no se ha podido poner a 0.\n";
        exit(1);
    }


    /**
     * Creamos un hijo productor y un hijo consumidor
     */
    pid_t pidProductor, pidConsumidor;
    pidProductor = fork();
    if (pidProductor == 0)
    {
        FuncionProductor(shmPTR, semID);
        exit(0);
    }
    pidConsumidor = fork();
    if (pidConsumidor == 0)
    {
        FuncionConsumidor(shmPTR, semID);
        exit(0);
    }

    /**
     * Esperamos a que acaben los hijos para liberar la memoria compartida y los semáforos.
     */
    int status;
    wait(&status);
    wait(&status);

    semctl(semID, 0, IPC_RMID, arg);
    shmdt(shmPTR);
    shmctl(shmID, IPC_RMID, NULL);
    exit(0);
}

