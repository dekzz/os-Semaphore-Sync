#include <iostream>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
using namespace std;

union UnijaSem {
    int Vrijednost;
    struct IDsem_ds *buf;
    unsigned short *array;
    struct SemInfo *__buf;
};

char *M; 
int *ULAZ, PISI, PUN, PRAZAN, NIZ;

bool *Zavrseno, *Upisali; 
sembuf s_wait;
sembuf s_post;

void Proizvodac(long int ID)
{
    if (ID > 0)
        ID = 1;
    string NizUlaza;

    semop(NIZ, &s_wait, 1);
    cout << "Unesi znakove za proizvodjaca " << ID << ": ";
    getline(cin, NizUlaza);
    Upisali[ID] = true;
    semop(NIZ, &s_post, 1); 
    while(Upisali[!ID] == false)
        usleep(100000); 

    for (int i = 0; i < NizUlaza.length(); i++)
    {
        semop(PUN, &s_wait, 1);
        semop(PISI, &s_wait, 1); 
        cout << "Proizvodjac " << ID << " -> " << NizUlaza[i] << endl;
        M[*ULAZ] = NizUlaza[i]; 
        cout << "Stanje buffera M: " << M << endl;
        *ULAZ = (*ULAZ+1) % 5;
        semop(PISI, &s_post, 1);
        semop(PRAZAN, &s_post, 1);
    }
    Zavrseno[ID] = true;
}

void Potrosac()
{
    string NizIzlaza;
    int IZLAZ = 0; 
    
    while ((Zavrseno[1] && Zavrseno[0]) == false)
    { 
        semop(PRAZAN, &s_wait, 1);
        NizIzlaza.append(1, M[IZLAZ]);
        cout << "Potrosac <- " << M[IZLAZ] << endl;
        cout << "Stanje izlaznog niza: " << NizIzlaza << endl << endl;
        IZLAZ = (IZLAZ +1) % 5;
        sleep(1);
        semop(PUN, &s_post, 1);
    }
    cout << "\nKombinirani niz: " << NizIzlaza << endl;
}


int main() 
{
    int id_M = shmget(IPC_PRIVATE, sizeof(char)*5, 0600);
    int id_ULAZ = shmget(IPC_PRIVATE, sizeof(int*), 0600);
    int id_zavrsio = shmget(IPC_PRIVATE, sizeof(bool)*2, 0600);
    int id_oba_upisala = shmget(IPC_PRIVATE, sizeof(bool)*2, 0600);
    Zavrseno = (bool*)shmat(id_zavrsio, NULL, 0);
    M = (char*)shmat(id_M, NULL, 0);
    ULAZ = (int*)shmat(id_ULAZ, NULL, 0);
    Upisali = (bool*)shmat(id_oba_upisala, NULL, 0);
    PISI = semget(IPC_PRIVATE, 1, 0600);
    PUN = semget(IPC_PRIVATE, 1, 0600);
    PRAZAN = semget(IPC_PRIVATE, 1, 0600);
    NIZ = semget(IPC_PRIVATE, 1, 0600);
    UnijaSem Vr;
    Vr.Vrijednost = 0;  
    semctl(PRAZAN, 0, SETVAL, Vr);
    Vr.Vrijednost = 1;
    semctl(PUN, 0, SETVAL, Vr);
    semctl(PISI, 0, SETVAL, Vr);
    semctl(NIZ, 0, SETVAL, Vr);

    s_wait.sem_num = 0;
    s_wait.sem_flg = 0;
    s_wait.sem_op = -1; 
    
    s_post.sem_num = 0;
    s_post.sem_flg = 0;
    s_post.sem_op = 1;  

    Upisali[0] = false;
    Upisali[1] = false;

    switch(fork())
    { 
        case 0: 
            Proizvodac(fork());
            wait(NULL); 
            return 0;
        case -1:
            cout << "Greska!" << endl;
            return 0;
        default:
            if (!fork())
            {
                Potrosac();
                return 0;
            }
    }
    
    int br = 2;
    while(br--)
        wait(NULL);

    return 0;
}

