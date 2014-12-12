/* Steve Wilkerson
 * Jeff Augustine
 * martianrun.c
 */

#include "lab2-api.h"


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
    int handle;
    //int producers, consumers;
    sem_t semaphore;
    atoms *atom;
    char handle_str[10], semaphore_str[10];

    /*switch(argc)
    {
        case 3:
            producers = dstrtol(argv[1], NULL, 10);
            consumers = dstrtol(argv[2], NULL, 10);
            break;
        default:
            printf("Invalid arguments!");
            exit();
    }*/
    
    handle = shmget();
    atom = (atoms *)shmat(handle);

    if (atom == NULL)
    {
        printf("Error mapping the shared memory page, bailing out now!");
        exit();
    }

    semaphore = sem_create(2);
    atom->lock = lock_create();
    atom->count = 0;
    atom->n_atoms = 0;
    atom->o_atoms = 0;
    atom->n_mols = 0;
    atom->o_mols = 0;
    
    ditoa(handle, handle_str);
    ditoa(semaphore, semaphore_str);

    process_create("producer.dlx.obj", handle_str, semaphore_str, NULL);
    process_create("consumer.dlx.obj", handle_str, semaphore_str, NULL);

    sem_wait(semaphore);
    
}
