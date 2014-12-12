
#include <stdio.h>
#include <stdlib.h>
#include "lab2-api.h"

typedef struct db
{

    int buffer = 0;
    int n_atoms = 0;
    int o_atoms = 0;
    int n_mols = 0;
    int o_mols = 0;
    lock_t lock;

} db;

void nReady()
{
    printf("An atom of nitrogen was created.\n");
    
}

void oReady()
{
    printf("An atom of oxygen was created.\n");
}

void makeOxygen()
{
    printf("An oxygen molecule was created.\n");
}

void makeNO2()
{
    printf("A nitrogen molecule was created.\n");
}

int main()
{


}
