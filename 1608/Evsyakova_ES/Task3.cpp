#include <stdlib.h>
#include <cstdio>
#include <mpi.h>
#include <time.h>

#define ROOT 0 
const enum Condition{ Thinking, Eating, Hungry };

int think(){ return Hungry; }
int doneEating(){ return Thinking; }
int eat(){return Eating;}
void print_spoons(const int* spoons,const int &n){
    printf("Spoons:");
    fflush(stdout);
    for (int i = 0; i < n; i++){
        printf("%d", spoons[i]);
    }
    fflush(stdout);
    printf("\n");
    fflush(stdout);
}

int main(int argc, char* argv[]){
    int procRank, procSize;
    int condition;
    int *message;
    const int n = 8; //number of repeat
    MPI_Status stat;
    double worktime;

    message = (int*)malloc(sizeof(int));

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &procSize);
    MPI_Comm_rank(MPI_COMM_WORLD, &procRank);
    printf("Process %d reporting start \n", procRank);
    fflush(stdout);
    worktime = MPI_Wtime();
    
    //oficiant start
    if (procRank == ROOT){
        int *spoons;
        int repeat = n*(procSize-1);
        int k=0;
        spoons = (int*)malloc(sizeof(int)*(procSize-1));
        for (int i = 0; i<(procSize - 1); ++i){
            spoons[i] = 1;
        }
#ifdef _DEBUG
        print_spoons(spoons, procSize-1);
        printf("Oficiant receiving orders \n", procRank);
        fflush(stdout);
#endif
        while (repeat > 0){
            MPI_Recv(message, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
            if (stat.MPI_TAG == 0){
                if ( (spoons[(stat.MPI_SOURCE-1)] == 1) && (spoons[(stat.MPI_SOURCE) % (procSize-1)] == 1) ){
                    spoons[(stat.MPI_SOURCE - 1)] = 0;
                    spoons[stat.MPI_SOURCE % (procSize - 1)] = 0;
                    MPI_Send(message, 1, MPI_INT, stat.MPI_SOURCE, 1, MPI_COMM_WORLD);
#ifdef _DEBUG
                    printf("Oficiant gave spoons to philosoph %d\n", stat.MPI_SOURCE);
                    fflush(stdout);
                    print_spoons(spoons, procSize - 1);
#endif
                }else{
                    MPI_Send(message, 1, MPI_INT, stat.MPI_SOURCE, 0, MPI_COMM_WORLD);
#ifdef _DEBUG
                    printf("Oficiant couldn't give spoons to philosoph %d\n", stat.MPI_SOURCE);
                    fflush(stdout);
                    print_spoons(spoons, procSize-1);
#endif
                }
            }else if (stat.MPI_TAG == 1){
                //spoons recieved
                spoons[stat.MPI_SOURCE - 1] = 1;
                spoons[(stat.MPI_SOURCE) % (procSize - 1)] = 1;
                MPI_Send(message, 1, MPI_INT, stat.MPI_SOURCE, 2, MPI_COMM_WORLD);
                repeat--;
#ifdef _DEBUG
                printf("Spoons returned from philosoph %d\n", stat.MPI_SOURCE);
                fflush(stdout);
                print_spoons(spoons, procSize-1);
#endif
            }
        }
#ifdef _DEBUG
        printf("%d finished\n", procRank);
        fflush(stdout);
#endif
        printf("Worktime = %f\n", worktime);
        fflush(stdout);
        MPI_Finalize();
        return 0;
    }
    //oficiant end
    for (int i = 0; i < n; i++){
#ifdef _DEBUG
        printf("Philosoph %d thinking/hungry \n", procRank);
        fflush(stdout);
#endif
        condition = think();
        //lock
#ifdef _DEBUG
        printf("Philosoph %d trying get spoons\n", procRank);
        fflush(stdout);
#endif
        do{
          MPI_Send(message, 1, MPI_INT, ROOT, 0, MPI_COMM_WORLD);
          MPI_Recv(message, 1, MPI_INT, ROOT, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
          for (int i = 0; i < 10000; i++);
        } while (stat.MPI_TAG == 0); // while not get spoons
#ifdef _DEBUG
        printf("Philosoph %d got spoons\n", procRank);
        fflush(stdout);
#endif
        condition = eat();
#ifdef _DEBUG
        printf("Philosoph %d eating, trying to return spoons\n", procRank);
        fflush(stdout);
#endif
        MPI_Send(message, 1, MPI_INT, ROOT, 1, MPI_COMM_WORLD);
#ifdef _DEBUG
        printf("Philosoph %d returned spoons\n", procRank);
        fflush(stdout);
#endif
        MPI_Recv(message, 1, MPI_INT, ROOT, 2, MPI_COMM_WORLD, &stat);
#ifdef _DEBUG
        printf("Philosoph %d got approvment\n", procRank);
        fflush(stdout);
#endif
        condition = doneEating();
#ifdef _DEBUG
        printf("Philosoph %d done eating\n", procRank);
        fflush(stdout);
#endif
    }
#ifdef _DEBUG
    printf("%d finished\n", procRank);
    fflush(stdout);
#endif
    fprintf(stdout,"Worktime = %f\n", worktime);
    fflush(stdout);
    MPI_Finalize();
    return 0;
}

