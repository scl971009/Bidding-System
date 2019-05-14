/*b05902038 林詩芩*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <sys/wait.h>
#define DEBUG
int main(int argc, char *argv[])
{
    //make FIFO
    int host_id = atoi(argv[1]);
    char ff[15], ffa[15], ffb[15], ffc[15], ffd[15];
    sprintf(ff, "host%d.FIFO", host_id);
    sprintf(ffa, "host%d_A.FIFO", host_id);
    sprintf(ffb, "host%d_B.FIFO", host_id);
    sprintf(ffc, "host%d_C.FIFO", host_id);
    sprintf(ffd, "host%d_D.FIFO", host_id);
    mkfifo(ff, 0777);
    mkfifo(ffa, 0777);
    mkfifo(ffb, 0777);
    mkfifo(ffc, 0777);
    mkfifo(ffd, 0777);

    //make random_key
    int random_key[4];
    srand(time(NULL));
    random_key[0] = rand();
    while(random_key[0] < 0 || random_key[0] > 65535)
        random_key[0] = rand();
    random_key[1] = rand();
    while(random_key[1] < 0 || random_key[1] > 65535 || random_key[1] == random_key[0])
        random_key[1] = rand();
    random_key[2] = rand();
    while(random_key[2] < 0 || random_key[2] > 65535 || random_key[2] == random_key[0] || random_key[2] == random_key[1])
        random_key[2] = rand();
    random_key[3] = rand();
    while(random_key[3] < 0 || random_key[3] > 65535 || random_key[3] == random_key[0] || random_key[3] == random_key[1] || random_key[3] == random_key[2])
        random_key[3] = rand();

    while(1){
        //scan player_id
        int player_id[4];
        scanf("%d%d%d%d", &player_id[0], &player_id[1], &player_id[2], &player_id[3]);
#ifdef DEBUG
	//fprintf(stderr, "player = %d %d %d %d\n", player_id[0], player_id[1], player_id[2], player_id[3]);
#endif
	if(player_id[0] == -1 && player_id[1] == -1 && player_id[2] == -1 && player_id[3] == -1){
            //clear FIFO
            unlink(ff);
            unlink(ffa);
            unlink(ffb);
            unlink(ffc);
            unlink(ffd);
            exit(0);
        }
        //fork
        pid_t pid1, pid2, pid3, pid4;
        char random[4][6], ID[3];
        sprintf(ID, "%d", host_id);
        //A
        if((pid1 = fork()) == 0){
            sprintf(random[0], "%d", random_key[0]);
            if(execl("./player", "player", ID, "A", random[0], (char *)0) == -1){
                fprintf(stderr, "exec error");
                exit(1);
            }
        }
        //B
        if((pid2 = fork()) == 0){
            sprintf(random[1], "%d", random_key[1]);
            if(execl("./player", "player", ID, "B", random[1], (char *)0) == -1){
                fprintf(stderr, "exec error");
                exit(1);
            }
        }
        //C
        if((pid3 = fork()) == 0){
            sprintf(random[2], "%d", random_key[2]);
            if(execl("./player", "player", ID, "C", random[2], (char *)0) == -1){
                fprintf(stderr, "exec error");
                exit(1);
            }
        }
        //D
        if((pid4 = fork() == 0)){
            sprintf(random[3], "%d", random_key[3]);
            if(execl("./player", "player", ID, "D", random[3], (char *)0) == -1){
                fprintf(stderr, "exec error");
                exit(1);
            }
        }

        //competition
        int i, j, fd1, fd2, fd3, fd4, fd5;
        FILE *fp1, *fp2, *fp3, *fp4, *fp5;
        int money[4] = {1000, 1000, 1000, 1000};
        int bet[4], win[4] = {0};
        char player_id_temp;
        int random_key_temp, bet_temp;

        //open FIFO
        fd5 = open(ff, O_RDONLY | O_NONBLOCK);
        fp5 = fdopen(fd5, "r");
        assert(fp5 != NULL);
        fd1 = open(ffa, O_WRONLY);
        fp1 = fdopen(fd1, "w");
	//fprintf(stderr, "fd1 = %d\n", fd1);
        assert(fp1 != NULL);
        fd2 = open(ffb, O_WRONLY);
        fp2 = fdopen(fd2, "w");
	//fprintf(stderr, "fd2 = %d\n", fd2);
        assert(fp2 != NULL);
        fd3 = open(ffc, O_WRONLY);
        fp3 = fdopen(fd3, "w");
	//fprintf(stderr, "fd3 = %d\n", fd3);
        assert(fp3 != NULL);
        fd4 = open(ffd, O_WRONLY);
        fp4 = fdopen(fd4, "w");
	//fprintf(stderr, "fd4 = %d\n", fd4);
        assert(fp4 != NULL);

	//select
	fd_set ready_set, check_set;
	FD_ZERO(&ready_set);
	FD_SET(fd5, &ready_set);

	//fprintf(stderr, "before competition\n");

        for(i = 0; i < 10; i++){
            //tell money

            fprintf(fp1, "%d %d %d %d\n", money[0], money[1], money[2], money[3]);
	    fflush(fp1);
	    fsync(fd1);
            fprintf(fp2, "%d %d %d %d\n", money[0], money[1], money[2], money[3]);
	    fflush(fp2);
	    fsync(fd2);
            fprintf(fp3, "%d %d %d %d\n", money[0], money[1], money[2], money[3]);
	    fflush(fp3);
	    fsync(fd3);
            fprintf(fp4, "%d %d %d %d\n", money[0], money[1], money[2], money[3]);
	    fflush(fp4);
	    fsync(fd4);

	    //fprintf(stderr, "after telling money\n");

            //read bet
	    char buffer[70];
	    for(j = 0; j < 4; j++)
		    bet[j] = -1;
            while(bet[0] == -1 || bet[1] == -1 || bet[2] == -1 || bet[3] == -1){
		check_set = ready_set;

		//bug here!!!
#ifdef DEBUG
		//fprintf(stderr, "before select\n");
#endif
		select(fd5 + 1, &check_set, NULL, NULL, NULL);
#ifdef DEBUG
		//fprintf(stderr, "after select\n");
#endif
		int length = read(fd5, buffer, sizeof(buffer));
		j = 0;

		//deal with buffer
		while(j < length){
			if(buffer[j] == 'A' || buffer[j] == 'B' || buffer[j] == 'C' || buffer[j] == 'D'){
				sscanf(&buffer[j], "%c %d %d\n", &player_id_temp, &random_key_temp, &bet_temp);
				if(player_id_temp == 'A' || player_id_temp == 'B' || player_id_temp == 'C' || player_id_temp == 'D'){
                			if(random_key_temp == random_key[0] && player_id_temp == 'A')
                    			bet[0] = bet_temp;
                			else if(random_key_temp == random_key[1] && player_id_temp == 'B')
                    			bet[1] = bet_temp;
                			else if(random_key_temp == random_key[2] && player_id_temp == 'C')
               		     		bet[2] = bet_temp;
       		         		else if(random_key_temp == random_key[3] && player_id_temp == 'D')
	               	   		bet[3] = bet_temp;
        	       			else{
   	               	 			fprintf(stderr, "random_key or player_id error\n random_key is %d, and player_id is %c", random_key_temp, player_id_temp);
               			    		exit(1);
                			}
				}
				while(buffer[j] != '\n')
					j++;
			}
			else
				j++;
            	}
	    }

	    //fprintf(stderr, "after reading bets\n");
	    //fprintf(stderr, "A bets %d, B bets %d, C bets %d, D bets %d\n", bet[0], bet[1], bet[2], bet[3]);
            //determine winner

	    int order[4];
	    order[0] = 0;
	    order[1] = 1;
	    order[2] = 2;
	    order[3] = 3;
            int index, k, tempo;
            for(j = 3; j > 0; j--){
		    for(k = 0; k < j; k++){
			    if(bet[k] < bet[k + 1]){
				    tempo = bet[k];
				    bet[k] = bet[k + 1];
				    bet[k + 1] = tempo;
				    tempo = order[k];
				    order[k] = order[k + 1];
				    order[k + 1] = tempo;
			    }
		    }
	    }
	    tempo = 0;
	    int a = 0;
	    for(j = 0; j < 4; j++){
                if(bet[j] == bet[j + 1]){
			tempo = bet[j];
			continue;
		}
		if(bet[j] != tempo){
			index = order[j];
			a = j;
			break;
		}
            }
            win[index]++;
	    //fprintf(stderr, "A's money is %d, B's money is %d, C's money is %d, D's money is %d\n", money[0], money[1], money[2], money[3]);
            //determine next round money
            for(j = 0; j < 4; j++){
                if(j == index)
                    money[j] -= bet[a];
                money[j] += 1000;
            }
	    //fprintf(stderr, "winner is %d, he bets %d\n", index, bet[a]);
	    //fprintf(stderr, "A's money is %d, B's money is %d, C's money is %d, D's money is %d\n", money[0], money[1], money[2], money[3]);
	}

	//fprintf(stderr, "after competition\n");

        //print result
        int arr[4], temp;
        arr[0] = 0;
        arr[1] = 1;
        arr[2] = 2;
        arr[3] = 3;
        for(i = 3; i > 0; i--){
            for(j = 0; j < i; j++){
                if(win[j] < win[j + 1]){
                    temp = win[j];
                    win[j] = win[j + 1];
                    win[j + 1] = temp;
                    temp = arr[j];
                    arr[j] = arr[j + 1];
                    arr[j + 1] = temp;
                }
            }
        }
        temp = 1;
        printf("%d 1\n", player_id[arr[0]]);
        for(i = 1; i < 4; i++){
            if(win[i] == win[i - 1])
                printf("%d %d\n", player_id[arr[i]], temp);
            else{
                printf("%d %d\n", player_id[arr[i]], i + 1);
                temp = i + 1;
            }
        }
	fflush(stdout);
	fsync(1);
	//fprintf(stderr, "before wait\n");
	//wait
	int status;
	pid_t pid;
	pid = wait(&status);
	pid = wait(&status);
	pid = wait(&status);
	pid = wait(&status);
	//fprintf(stderr, "after wait\n");
	fclose(fp1);
	fclose(fp2);
	fclose(fp3);
	fclose(fp4);
	fclose(fp5);
	close(fd1);
	close(fd2);
	close(fd3);
	close(fd4);
	close(fd5);
    }
    return 0;
}
