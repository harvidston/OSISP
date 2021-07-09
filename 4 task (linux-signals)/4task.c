//Дерево процессов 8 вариант
//1->(2,3,4,5,6) 6 -> (7,8)
//Последовательность обмена сигналами 4 вариант
//1->(2,3) SIGUSR1 2->(4,5) SIGUSR1 5->6 SIGUSR1 6->(7,8) SIGUSR1 8->1 SIGUSR1
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/time.h>

int getTime() {
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    return ((int)(currentTime.tv_usec))%1000;
}

void writePid(char pid[1]) {
    char filename[20] = "";
    strcat(strcat(filename, pid), ".txt");
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "%d: %s\n", getpid(), strerror(errno));
        exit(1);
    }
    fprintf(file, "%d", getpid());
    fclose(file);
}

int getPid(char *pid) {
    char filename[20] = "";
    strcat(strcat(filename, pid), ".txt");
    while(fopen(filename, "r") == NULL) {}
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "%d: %s\n", getpid(), strerror(errno));
        exit(1);
    }
    int res = -1;
    fscanf(file, "%d", &res);
    fclose(file);
    return res;
}

void handle_pr8(int sig) {
    static int countSent=0;
    if (sig == SIGUSR1) {
        printf("8 %d %d get SIGUSR1 %d\n", getpid(), getppid(), getTime());
        int pid1;
        while ((pid1 = getPid("1")) < 1) {}
        printf("8 %d %d send SIGUSR1 %d\n", getpid(), getppid(), getTime());
        kill(pid1, SIGUSR1);
        countSent++;
    } else if (sig == SIGTERM) {
        printf("%d %d closed after %d SIGUSR1\n", getpid(), getppid(), countSent);
        exit(0);
    }
}

void create8() {
    printf("8 %d %d %d\n", getpid(), getppid(), getTime());
    writePid("8");
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = handle_pr8;
    sigaction(SIGUSR1, &action, 0);
    sigaction(SIGTERM, &action, 0);

    fopen("alldone.txt","w");

    while (1) { pause(); }
}

void handle_pr7(int sig) {
    if (sig == SIGUSR1) {
        printf("7 %d %d get SIGUSR1 %d\n", getpid(), getppid(), getTime());
    } else if (sig == SIGTERM) {
        kill(getPid("8"), SIGTERM);
        wait(0);
        printf("%d %d closed after 0 SIGUSR1\n", getpid(), getppid());
        exit(0);
    }
}

void create7() {
    printf("7 %d %d %d\n", getpid(), getppid(), getTime());
    writePid("7");

    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = handle_pr7;
    sigaction(SIGUSR1, &action, 0);
    sigaction(SIGTERM, &action, 0);

    while (1) { pause(); }
}

void handle_pr6(int sig) {
    static int countSent=0;
    if (sig == SIGUSR1) {
        printf("6 %d %d get SIGUSR1 %d\n", getpid(), getppid(), getTime());
        int pid7;
        int pid8;
        while ((pid7 = getPid("7")) < 1) {}
        while ((pid8 = getPid("8")) < 1) {}
        countSent++;
        killpg(pid7, SIGUSR1);
        printf("6 %d %d send SIGUSR1 %d\n", getpid(), getppid(), getTime());
    } else if (sig == SIGTERM) {
        killpg(getPid("7"), SIGTERM);
        for (int i = 0; i < 2; i++){
            wait(0);
        }
        printf("%d %d closed after %d SIGUSR1\n", getpid(), getppid(), countSent);
        exit(0);
    }
}

void create6() {
    printf("6 %d %d %d\n", getpid(), getppid(), getTime());
    writePid("6");

    int pid7 = fork();
    if (pid7 == -1) {
        fprintf(stderr, "%d: %s\n", getpid(), strerror(errno));
        exit(1);
    } else if (pid7 == 0) {
        create7();
    }
    setpgid(pid7, pid7);

    int pid8 = fork();
    if (pid8 == -1) {
        fprintf(stderr, "%d: %s\n", getpid(), strerror(errno));
        exit(1);
    } else if (pid8 == 0) {
        create8();
    }
    setpgid(pid8, pid7);

    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = handle_pr6;
    sigaction(SIGUSR1, &action, 0);
    sigaction(SIGTERM, &action, 0);

    while (1) { pause(); }
}

void handle_pr5(int sig) {
    static int countSent = 0;
    if (sig == SIGUSR1) {
        printf("5 %d %d get SIGUSR1 %d\n", getpid(), getppid(), getTime());
        int pid6;
        while ((pid6 = getPid("6")) < 1) {}
        kill(pid6, SIGUSR1);
        countSent++;
        printf("5 %d %d send SIGUSR1 %d\n", getpid(), getppid(), getTime());
    } else if (sig == SIGTERM) {
        kill(getPid("6"), SIGTERM);
        printf("%d %d closed after %d SIGUSR1 \n", getpid(), getppid(), countSent);
        exit(0);
    }
}

void create5() {
    printf("5 %d %d %d\n", getpid(), getppid(), getTime());
    writePid("5");

    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = handle_pr5;
    sigaction(SIGUSR1, &action, 0);
    sigaction(SIGTERM, &action, 0);

    while (1) { pause(); }
}

void handle_pr4(int sig) {
    static int countSent=0;
    if (sig == SIGUSR1) {
        printf("4 %d %d get SIGUSR1 %d\n", getpid(), getppid(), getTime());
        
    } else if (sig == SIGTERM) {
        wait(0);
        printf("%d %d closed after %d SIGUSR1\n", getpid(), getppid(), countSent);
        exit(0);
    }
}

void create4() {
    printf("4 %d %d %d\n", getpid(), getppid(), getTime());
    writePid("4");
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = handle_pr4;
    sigaction(SIGUSR1, &action, 0);
    sigaction(SIGTERM, &action, 0);

    while (1) { pause(); }
}

void handle_pr3(int sig) {
    static int countSent=0;
    if (sig == SIGUSR1) {
        printf("3 %d %d get SIGUSR1 %d\n", getpid(), getppid(), getTime());
    } else if (sig == SIGTERM) {
        wait(0);
        printf("%d %d closed after %d SIGUSR1\n", getpid(), getppid(), countSent);
        exit(0);
    }
}

void create3() {
    printf("3 %d %d %d\n", getpid(), getppid(), getTime());
    writePid("3");
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = handle_pr3;
    sigaction(SIGUSR1, &action, 0);
    sigaction(SIGTERM, &action, 0);

    while (1) { pause(); }
}

void handle_pr2(int sig) {
    static int countSent = 0;
    if (sig == SIGUSR1) {
        printf("2 %d %d get SIGUSR1 %d\n", getpid(), getppid(), getTime());
        int pid4;
        while ((pid4 = getPid("4")) < 1) {}
        int pid5;
        while ((pid5 = getPid("5")) < 1) {}
        killpg(pid4, SIGUSR1);
        countSent++;
        printf("2 %d %d send SIGUSR1 %d\n", getpid(), getppid(), getTime());
    } else if (sig == SIGTERM) {
        killpg(getPid("4"), SIGTERM);
        wait(0);
        printf("%d %d closed after %d SIGUSR1\n", getpid(), getppid(), countSent);
        exit(0);
    }
}

void create2() {
    printf("2 %d %d %d\n", getpid(), getppid(), getTime());
    writePid("2");

    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = handle_pr2;
    sigaction(SIGUSR1, &action, 0);
    sigaction(SIGTERM, &action, 0);

    while (1) { pause(); }
}

void handle_pr1(int sig) {
    static int countSent=0;
    if (sig == SIGUSR1) {
        static int countGet = 0;
        countGet++;
        printf("1 %d %d get SIGUSR1 %d\n", getpid(), getppid(), getTime());
        if (countGet == 101) {
            raise(SIGTERM);
        }
        int pid2;
        int pid3;
        while ((pid2 = getPid("2")) < 1) {}
        while((pid3 = getPid("3")) < 1) {}
        killpg(pid2, SIGUSR1);
        countSent++;
        printf("1 %d %d send SIGUSR1 %d\n", getpid(), getppid(), getTime());
    } else if (sig == SIGTERM) {
        killpg(getPid("2"), SIGTERM);
        for(int i = 0; i < 5; i++){
           wait(0);
        }
        printf("%d %d closed after %d SIGUSR1\n", getpid(), getppid(), countSent);
        exit(0);
    }
}

void create1() {
    printf("1 %d %d %d\n", getpid(), getppid(), getTime());
    writePid("1");


    int pid2 = fork();
    if (pid2 == -1) {
        fprintf(stderr, "%d: %s\n", getpid(), strerror(errno));
        exit(1);
    } else if (pid2 == 0) {
        create2();
    }
    setpgid(pid2, pid2);

    int pid3 = fork();
    if (pid3 == -1) {
        fprintf(stderr, "%d: %s\n", getpid(), strerror(errno));
        exit(1);
    } else if (pid3 == 0) {
        create3();
    }
    setpgid(pid3, pid2);

    int pid4 = fork();
    if (pid4 == -1) {
        fprintf(stderr, "%d: %s\n", getpid(), strerror(errno));
        exit(1);
    } else if (pid4 == 0) {
        create4();
    }
    setpgid(pid4, pid4);

    int pid5 = fork();
    if (pid5 == -1) {
        fprintf(stderr, "%d: %s\n", getpid(), strerror(errno));
        exit(1);
    } else if (pid5 == 0) {
        create5();
    }
    setpgid(pid5, pid4);

    int pid6 = fork();
    if (pid6 == -1) {
        fprintf(stderr, "%d: %s\n", getpid(), strerror(errno));
        exit(1);
    } else if (pid6 == 0) {
        create6();
    }
    setpgid(pid6,pid6);

    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = handle_pr1;
    sigaction(SIGUSR1, &action, 0);
    sigaction(SIGTERM, &action, 0);

    while (1) { pause(); }
}

int main(int argc, char *argv[]) {
    remove("0.txt");
    remove("1.txt");
    remove("2.txt");
    remove("3.txt");
    remove("4.txt");
    remove("5.txt");
    remove("6.txt");
    remove("7.txt");
    remove("8.txt");
    remove("alldone.txt");

    int pid1 = fork();
    if (pid1 == -1) {
        fprintf(stderr, "%d: %s\n", getpid(), strerror(errno));
        exit(1);
    } else if (pid1 == 0) {
        create1
    ();
    } else {
        writePid("0");
        int pid;
	while(fopen("alldone.txt", "r") == NULL) {}
	char ps[128];
	sprintf(ps, "pstree -p %d", getPid("0"));
        system(ps);	

        while ((pid = getPid("1")) < 1) {}
        kill(pid, SIGUSR1);
        pid_t pidpid = getPid("1");
        wait(&pidpid);
    }
}
