#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_LEN 64
#define MAX_RESULTS 128

typedef struct {
    char region[MAX_LEN];
    int minutes;
} Result;

typedef struct {
    char turno;
    char vm[MAX_LEN];
    char region[MAX_LEN];
    char fail [MAX_LEN];
    int minutes;
    char cause[MAX_LEN];
} FileData;

typedef struct {
    Result results[MAX_RESULTS];
    int count;
    int is_busy;
} SharedResults;

void showtree();

void read_file(const char *file, char ***vec, int *size);
void createSHMv(int *x_shm, void **X, size_t size);

int main(int argc, char **argv){
    if(argc < 3){
        printf("Send file and N\n");
        return EXIT_FAILURE;
    }
    size_t total_char = 0;
    char **lines;
    int n, pos = 0, chunk;
    int n_children = atoi(argv[2]), child_id;
    pid_t root = getpid();

    read_file(argv[1], &lines, &n);

    for(int i=0; i < n; i++){
        total_char += strlen(lines[i]) + 1;
    }

    chunk = n / n_children;

    int text_shm_id, index_shm_id, results_shm_id;
    char *shared_text;
    int *offset;
    SharedResults *shared;

    createSHMv(&results_shm_id, (void**)&shared, sizeof(SharedResults));
    createSHMv(&index_shm_id, (void**)&offset, n * sizeof(int));
    createSHMv(&text_shm_id, (void**)&shared_text, total_char * sizeof(char));

    shared->count = 0;

    for(int i=0; i < n; i++){
        offset[i] = pos;
        strcpy(shared_text + pos, lines[i]);
        pos += strlen(lines[i]) + 1;
    }

    for(int i=0; i < n; i++)free(lines[i]);
    free(lines);

    for(child_id=0; child_id < n_children; child_id++){
        if(!fork()) break;
    }

    if(root == getpid()){
        showtree();

        for(int i=0; i < n_children; i++) wait(NULL);

        int max = -1;
        char reg[MAX_LEN];

        for(int i=0; i < shared->count; i++){
            printf("Region: %s (%d minutes)\n", shared->results[i].region, shared->results[i].minutes);
            if(shared->results[i].minutes > max){
                max = shared->results[i].minutes;
                strcpy(reg, shared->results[i].region);
            }
        }

        printf("Region with longest time, %s (%d minutes)\n", reg, max);

        shmdt(offset);
        shmdt(shared);
        shmdt(shared_text);
        shmctl(text_shm_id, IPC_RMID, NULL);
        shmctl(index_shm_id, IPC_RMID, NULL);
        shmctl(results_shm_id, IPC_RMID, NULL);
    }else{

        //FALTA HACER ALGO (CALCULAR LOS INDICES SI LOS HIJOS SON MENORES QUE LA CANTIDAD DE ARCHIVOS)

        /*sleep(1);
        printf("Child: %d\n", getpid());
        for(int i=0; i < n; i++){
            printf("%d, %s\n", i, shared_text + offset[i]);
        }*/

        FILE *fl = fopen(shared_text + offset[child_id], "r");
        if(!fl) exit(-1);

        int n_register, total_inc = 0;
        FileData fl_data;
        Result result;

        fscanf(fl, "%d", &n_register);

        for(int i =0; i < n_register; i++){
            fscanf(fl, " %c;%[^;];%[^;];%[^;];%d;%s", &fl_data.turno,
                fl_data.vm, fl_data.region, fl_data.fail,
                &fl_data.minutes, fl_data.cause
            );

            total_inc += fl_data.minutes;
        }

        while(shared->is_busy) usleep(5000);
        shared->is_busy = 1;

        strcpy(shared->results[shared->count].region, fl_data.region);
        shared->results[shared->count].minutes = total_inc;
        shared->count++;

        shared->is_busy = 0;

        shmdt(offset);
        shmdt(shared_text);
        shmdt(shared);
        fclose(fl);
    }

    return EXIT_SUCCESS;
}

void createSHMv(int *x_shm, void **X, size_t size){
    *x_shm = shmget(IPC_PRIVATE, size, 0666 | IPC_CREAT);

    *X = shmat(*x_shm, 0, 0);
}

void read_file(const char *file, char ***vec, int *size){
    FILE *fl = fopen(file, "r");
    if(!fl) exit(-1);

    char buff[MAX_LEN];

    fscanf(fl, "%d", size);
    fgetc(fl);
    printf("Size: %d\n", *size);

    *vec = (char **) malloc(*size * sizeof(char*));
    if(!*vec) exit(-1);

    for(int i=0; i < *size; i++){
        fgets(buff, sizeof(buff), fl);
        buff[strcspn(buff, "\n")] = '\0';
        (*vec)[i] = (char*) malloc((strlen(buff)+1) * sizeof(char));
        strcpy((*vec)[i], buff);
    }


    for(int i=0; i < *size; i++){
        printf("%s\n", (*vec)[i]);
    }

    fclose(fl);
}

void showtree(){
    char cmd[20] = {""};
    sprintf(cmd, "pstree -cAlp %d", getpid());
    system(cmd);	
}