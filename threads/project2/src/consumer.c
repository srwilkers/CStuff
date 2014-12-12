/* Steve Wilkerson
   Jeff Augustine
   consumer.c
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

void main(int argc, char* argv[])
{
    int counter, handle;
    atoms *atom;
    sem_t semaphore;

    handle = dstrtol(argv[1], NULL, 10);
    semaphore = dstrtol(argv[2], NULL, 10);

    atom = (atoms *) shmat(handle);
    //sem_wait(semaphore);

    while (1)
    {
        sem_wait(semaphore);
        //lock_acquire(atom->lock);
        if (atom->o_atoms >= 2)
        {
            atom->o_mols++;
            atom->o_atoms = atom->o_atoms - 2;
            Printf("O2 molecule formed.\n");
        }
        if ((atom->n_atoms >= 1) && (atom->o_mols >= 1))
        {
            atom->n_atoms--;
            atom->o_mols--;
            atom->n_mols++;
            Printf("NO2 molecule formed.\n");
        }
        if (atom->count >= MAX)
        {
            //lock_release(atom->lock);
            exit();
        }
        sem_signal(semaphore);
        //lock_release(atom->lock);
    }
    //sem_signal(semaphore);
}
