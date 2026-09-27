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
        printf("Send N\n");
        return EXIT_FAILURE;
    }

    pid_t root = getpid();
    int n = atoi(argv[1]), n_children = 2, parent = 1, child_id=0, grand_id=0;
    pid_t children_id[n_children], grandson_id[n_children];

    signal(SIGUSR1, signal_handler);

    for(child_id=0; child_id < n_children; child_id++){
        if(!(children_id[child_id] = fork())){
            for(grand_id=0; grand_id < n_children; grand_id++){
                if(!(grandson_id[grand_id] = fork())){
                    parent = 0;
                    break;
                }
            }
            break;
        }
    }

    if(root == getpid()){ //padre
        showtree();

        for(int i=0; i < n; i++){
            printf("\n[ROUND %d] I'm parent %d and sending to: %d\n", i, getpid(), children_id[n_children-1]);
            send_to(children_id[n_children-1]);
            pause();
        }

        for(int i=0; i < n_children * n_children; i++) wait(NULL);
    }else{// hijos
        for(int i=0; i < n; i++){
            pause();

            if(parent){
                if(child_id == 1){ //H2
                    send_to(grandson_id[n_children-1]); //H22
                    pause();
                    send_to(grandson_id[n_children-2]); //H21
                    pause();
                    send_to(children_id[child_id-1]); //H1
                }else{ //H1
                    send_to(grandson_id[n_children-1]); //H12
                    pause();
                    send_to(grandson_id[n_children-2]); //H11
                    pause();
                    send_to(getppid()); //PADRE
                }

            }else{
                send_to(getppid());
            }
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