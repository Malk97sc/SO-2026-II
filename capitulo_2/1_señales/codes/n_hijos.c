#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

void showtree();

void signal_handler(int sig);
void send_to(pid_t pid);

int main(int argc, char **argv){
    if(argc < 2){
        return EXIT_FAILURE;
    }

    pid_t root = getpid();
    int n = atoi(argv[1]), i;
    pid_t pid_h[n];

    signal(SIGUSR1, signal_handler);

    for(i=0; i < n; i++){
        if(!(pid_h[i] = fork())){
            break;
        }
    }

    if(root == getpid()){ //padre
        showtree();

        send_to(pid_h[n-1]);
        pause();

        printf("Termine\n");
        for(int i=0; i < n; i++) wait(NULL);
    }else{// hijos
        pause();

        if(i == 0){
            send_to(getppid());
        }else{
            send_to(pid_h[i-1]);
        }
    }

    return 0;
}

void send_to(pid_t pid){
    printf("[PID %d] sending to: %d\n", getpid(), pid);
    usleep(5000);
    kill(pid, SIGUSR1);
}

void signal_handler(int sig){

}

void showtree(){
    char cmd[20] = {""};
    sprintf(cmd, "pstree -cAlp %d", getpid());
    system(cmd);
}