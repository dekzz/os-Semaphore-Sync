#include <pthread.h>
#include <iostream>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <stdlib.h>
using namespace std;

int PrvaPonuda, DrugaPonuda; 

sem_t Pusaci[3], trg;

int Posjed[3] = {2, 0, 1}; 
                             
void Ispisivanje(int IDPredmet)
{
    if (IDPredmet == 0)
       cout << "rizle";
    else if (IDPredmet == 1)
       cout << "upaljac";
    else if (IDPredmet == 2)
       cout << "duhan";
}

void *Prodavac(void *x)
{
    while(1) 
    {
        PrvaPonuda = rand() % 3;
        do 
        {
            DrugaPonuda = rand() % 3;
        }while (DrugaPonuda == PrvaPonuda);
        cout << "Prodavac nudi sljedece: ";
        Ispisivanje(PrvaPonuda);
        cout << " i ";
        Ispisivanje(DrugaPonuda);
        cout << "." << endl;
        sem_post(&Pusaci[0]); sem_post(&Pusaci[1]); sem_post(&Pusaci[2]);
        sem_wait(&trg);
        sleep(1);
    }
}

void *Pusac(void *ID) 
{
    int IDPusaca = *(int*)ID;
    while(1) 
    {
        sem_wait(&Pusaci[IDPusaca]);
        if (PrvaPonuda != Posjed[IDPusaca] && DrugaPonuda != Posjed[IDPusaca]) 
        {
            cout << "Pusac " << IDPusaca << " uzima ";
            Ispisivanje(PrvaPonuda);
            cout << " i ";
            Ispisivanje(DrugaPonuda);
            cout << "." << endl;
        }
        sem_post(&trg);

    }
}

int main() {
    srand(time(NULL));
    sem_init(&Pusaci[0], 0, 0);    
    sem_init(&Pusaci[1], 0, 0);
    sem_init(&Pusaci[2], 0, 0);
    sem_init(&trg, 0, 0);
    int SviID[4] = {0, 0, 1, 2};
    pthread_t dretva[4]; 
    pthread_create(&dretva[0], NULL, Prodavac, NULL);
    for (int i = 1; i <= 3; i++)
        pthread_create(&dretva[i], NULL, Pusac, &SviID[i]);
    for (int i = 0; i < 4; i++) 
        pthread_join(dretva[i], NULL);

    return 0;
}

