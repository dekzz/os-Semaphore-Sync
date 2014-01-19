#include <semaphore.h>
#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <fstream>
using namespace std;

string MS; 
string ImeDat;
fstream Dat;
sem_t s0, s1, s2; 
int Kraj = 0, n = 0, Suma = 0;

string Ekstenzija()
{
    string Eks;
    int Tocka = ImeDat.find_last_of('.');
    for (int i = Tocka+1; i < ImeDat.length(); i++)
        Eks.append(1, ImeDat[i]);
    return Eks;    
}

int BrojiRijeciTagove()
{
    int Broj = 0, Tocka;
    string Eks = Ekstenzija();
    if (Eks == "html" && Kraj != 1)
    {
        for (int i = 0; i < MS.length(); i++)
        {
            if (MS[i] == '<' && MS[i+1] != '/')
                Broj++;
        }
    }
    else if (Eks == "txt" && Kraj != 1)
    {
        for (int i = 0; i < MS.length(); i++) {
            if (MS[i] == ' ')
                Broj++;
        }
        if (MS.length() > 0)
            Broj++;
    }
    return Broj;
}

void* Citaj(void *x)
{ 
    string Red;
    while (getline(Dat, Red))
    {
        MS = Red;
        sem_post(&s1); 
        sem_post(&s2);
        for (int i = 0; i < 2; i++)
            sem_wait(&s0);
    }
    Kraj = 1;
    sem_post(&s1);
    sem_post(&s2);
}

void* Izbroji(void *x)
{
    do
    {
        sem_wait(&s1);
        n += BrojiRijeciTagove();    
        sem_post(&s0);
    }while (Kraj == 0);
}

void* Zbroji(void *x)
{
    do
    {
        sem_wait(&s2);
        for (int i = 0; i < MS.length(); i++)
            Suma = Suma^MS[i];
        sem_post(&s0);
    }while (Kraj == 0);
}

int main(int argc, char *argv[])
{
    pthread_t dretva[3];
    int BrojDatoteka = argc;
    sem_init(&s0, 0, 0);
    sem_init(&s1, 0, 0);
    sem_init(&s2, 0, 0);
    while (--BrojDatoteka)
    {
        switch(fork())
        {
            case -1:
                cout << "Greska!" << endl;
                return 0;
            case 0:
                ImeDat = argv[BrojDatoteka];
                if (Ekstenzija() != "html" && Ekstenzija() != "txt")
                {
                    cout << "Analiza je zaustavljena za datotekeku " << ImeDat <<  endl;
                    return 0;
                }
                Dat.open(argv[BrojDatoteka]);
                pthread_create(&dretva[0], NULL, Citaj, NULL);
                pthread_create(&dretva[1], NULL, Izbroji, NULL);
                pthread_create(&dretva[2], NULL, Zbroji, NULL); 
                for (int i = 0; i < 3; i++)
                     pthread_join(dretva[i], NULL);
                cout << "Datoteka " << argv[BrojDatoteka] << ": Ima " << n << " rijeci, zastitna suma je: " << Suma << endl;
                return 0;
        }
    }
    int BrojProcesa = argc -1;
    while (BrojProcesa--) wait(NULL);

    return 0;
}


