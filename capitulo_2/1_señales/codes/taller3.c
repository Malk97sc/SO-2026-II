#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

int count=0, end_flag=1;

void showtree();

void signal_handler(int sig);
void send_to(pid_t pid, int sig);
void send_count(pid_t pid, int count);

void read_file(const char *file, int ***mtx, int *rows, int *cols);

int main(int argc, char **argv){
    if(argc < 2){
        printf("Send file\n");
        return EXIT_FAILURE;
    }
    pid_t root = getpid(), child;
    int **mtx, rows, cols;

    signal(SIGUSR1, signal_handler);
    signal(SIGUSR2, signal_handler);

    child = fork();

    if(root == getpid()){ //padre
        showtree();
        int value;

        read_file(argv[1], &mtx, &rows, &cols);

        send_count(child, rows);
        send_count(child, cols);

        for(int i=0; i < rows; i++){
            for(int j=0; j < cols; j++){
                value = mtx[i][j];
                //printf("Sending Value: %d\n", value);
                for(int k=0; k < value; k++){
                    send_to(child, SIGUSR1);
                }
                send_to(child, SIGUSR2);
                pause();
            }
        }
        send_to(child, SIGUSR2);
    
        wait(NULL);
        printf("Ending\n");
    }else{// hijos
        int value=0;

        pause();
        while(end_flag != 0){
            pause();
            value++;
        }
        rows = value;
        value = 0;    

        send_to(getppid(), SIGUSR2);
        pause();

        while(end_flag != 0){
            pause();
            value++;
        }
        cols = value;
        value = 0;

        printf("Rows: %d, Cols: %d\n", rows, cols);

        mtx = (int **) malloc(rows * sizeof(int*));
        if(!mtx) exit(1);
        for(int i=0; i < rows; i++){
            mtx[i] = (int *) malloc(cols * sizeof(int));
            if(!mtx[i]) exit(1);
        }

        send_to(getppid(), SIGUSR2);
        pause();

        for(int i=0; i < rows; i++){
            for(int j=0; j < cols; j++){
                value = 0;
                while(end_flag != 0){
                    pause();
                    value++;
                    mtx[i][j] = value;
                }
                //printf("Value: %d\n", value);
                send_to(getppid(), SIGUSR2);
                pause();
            }
        }

        printf("New\n");

        for(int i=0; i < rows; i++){
            for(int j=0; j < cols; j++){
                printf("%d ", mtx[i][j]);
            }
            printf("\n");
        }

    }   

    for(int i=0; i < rows; i++){
        free(mtx[i]);
    }
    free(mtx);
    return 0;
}

void read_file(const char *file, int ***mtx, int *rows, int *cols){
    FILE *fl = fopen(file, "r");
    if(!fl) exit(1);

    fscanf(fl, "%d", rows);
    fscanf(fl, "%d", cols);
    printf("Row: %d, Cols: %d\n", *rows, *cols);

    *mtx = (int **) malloc(*rows * sizeof(int*));
    if(!(*mtx)) exit(1);
    for(int i=0; i < *rows; i++){
        (*mtx)[i] = (int *) malloc(*cols * sizeof(int));
        if(!(*mtx)[i]) exit(1);
    }

    for(int i=0; i < *rows; i++){
        for(int j=0; j < *cols; j++){
            fscanf(fl, "%d", &(*mtx)[i][j]);
        }
    }

    for(int i=0; i < *rows; i++){
        for(int j=0; j < *cols; j++){
            printf("%d ", (*mtx)[i][j]);
        }
        printf("\n");
    }

    

    fclose(fl);
}

void send_count(pid_t pid, int count){
    for(int i=0; i < count; i++) send_to(pid, SIGUSR1);
    send_to(pid, SIGUSR2);
    pause();
}

void send_to(pid_t pid, int sig){
    //printf("[PID %d] sending to: %d\n", getpid(), pid);
    usleep(5000);
    kill(pid, sig);
}

void signal_handler(int sig){
    switch(sig){
        case SIGUSR1:{
            end_flag = 1;
            break;
        }
        case SIGUSR2:{
            end_flag = 0;
            break;
        }
    }
}

void showtree(){
    char cmd[20] = {""};
    sprintf(cmd, "pstree -cAlp %d", getpid());
    system(cmd);
}