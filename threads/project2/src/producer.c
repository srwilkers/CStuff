/* Steve Wilkerson
   Jeff Augustine
   producer.c
*/

#include "lab2-api.h"

#define MAX 100

typedef struct atoms
{

    int count;
    int n_atoms;
    int o_atoms;
    int n_mols;
    int o_mols;
    lock_t lock;

} atoms;

void main(int argc, char*argv[])
{
    int handle;
    atoms *atom;
    sem_t semaphore;

    handle = dstrtol(argv[1], NULL, 10);
    semaphore = dstrtol(argv[2], NULL, 10);

    atom = (atoms *)shmat(handle);
    //sem_wait(semaphore);

    while (1)
    {
        sem_wait(semaphore);
        if (atom->count == MAX)
            exit();
        lock_acquire(atom->lock);
        atom->n_atoms++;
        Printf("N atom created.\n");
        atom->o_atoms++;
        Printf("O atom created.\n");
        atom->count++;
        lock_release(atom->lock);
        sem_signal(semaphore);
    }
    //sem_signal(semaphore);
}
