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
    pid_t root = getpid();
    int n_children = 5, parent = 1, child_id=0;
    pid_t children_id[n_children], grand_pid;

    signal(SIGUSR1, signal_handler);

    for(child_id=0; child_id < n_children; child_id++){
        if(!(children_id[child_id] = fork())){
            if(child_id % 2 == 0){
                if(!(grand_pid = fork())){
                    parent = 0;
                }
            }
            break;
        }
    }

    if(root == getpid()){ //padre
        showtree();

        send_to(children_id[n_children-1]);
        pause();

        for(int i=0; i < n_children * n_children; i++) wait(NULL);
    }else{// hijos
        pause();

        if(parent){
            if(child_id % 2 == 0){
                send_to(grand_pid);
                pause();
            }
            child_id == 0 ? send_to(getppid()) : send_to(children_id[child_id-1]);
            /*if(child_id == 0){
                send_to(getppid());
            }else{
                send_to(children_id[child_id-1]);
            }*/
        }else{
            send_to(getppid());
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