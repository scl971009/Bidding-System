/*b05902038 ªL¸ÖÍÁ*/
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

void fork_host(int host_num, int fd[12][2], int filedes[12][2]){
	pid_t pid[20];
	int i;
	for(i = 0; i < host_num; i++){
		if(pid[i] = fork() == 0){
			close(filedes[i][1]);
			close(fd[i][0]);
			//set STDIN
			if(filedes[i][0] != STDIN_FILENO){
				if(dup2(filedes[i][0], STDIN_FILENO) != STDIN_FILENO){
					fprintf(stderr, "dup2 error");
					exit(1);
				}
				close(filedes[i][0]);
			}
			//set STDOUT
			if(fd[i][1] != STDOUT_FILENO){
				if(dup2(fd[i][1], STDOUT_FILENO) != STDOUT_FILENO){
					fprintf(stderr, "dup2 error");
					exit(1);
				}
				close(fd[i][1]);
			}
			//exec
			char host[3];
			sprintf(host, "%d", i + 1);
			if(execl("./host", "host", host, (char *)0) == -1){
				fprintf(stderr, "exec error");
				exit(1);
			}
		}
	}
	return;
}

int c(int n){
	return (n * (n - 1) * (n - 2) * (n - 3) / 24);
}

void fill_in(char arr[5000][20], int num){
	int count = 0;
	for(int a = 1; a <= num - 3; a++)
		for(int b = a + 1; b <= num - 2; b++)
			for(int c = b + 1; c <= num - 1; c++)
				for(int d = c + 1; d <= num; d++){
					sprintf(arr[count], "%d %d %d %d\n", a, b, c, d);
					count++;
				}
	return;
}

int min(int a, int b){
	if(a > b)
		return b;
	return a;
}

int main(int argc, char *argv[])
{
	int host_num, player_num;
	host_num = atoi(argv[1]);
	player_num = atoi(argv[2]);

	//fork host
	int fd[12][2], filedes[12][2];
	FILE *fp[12];
	for(int loop_for_pipe = 0; loop_for_pipe < host_num; loop_for_pipe++){
		pipe(fd[loop_for_pipe]);
		pipe(filedes[loop_for_pipe]);
		fp[loop_for_pipe] = fdopen(fd[loop_for_pipe][0], "r");
	}
	fork_host(host_num, fd, filedes);

	for(int i = 0; i < host_num; i++){
		close(fd[i][1]);
		close(filedes[i][0]);
	}

	//determine player to write to host
	int total_comp = c(player_num);
	int temp = total_comp;
	char player[5000][20];
	int record = 0;
	fill_in(player, player_num);

	//first write
	for(int first = 0; first < min(total_comp, host_num); first++){
		write(filedes[first][1], player[first], strlen(player[first]));
		record++;
		fsync(filedes[first][1]);
		temp--;
	}

	//select ready host
	fd_set ready_set, check_set;
	FD_ZERO(&ready_set);
	for(int i = 0; i < host_num; i++){
		FD_SET(fd[i][0], &ready_set);
	}

	//read result and write player
	int player_id[4], player_rank[4], score[20] = {0};
	while(total_comp > 0){
		check_set = ready_set;
		//fprintf(stderr, "%d left\n", total_comp);
		select(fd[host_num - 1][0] + 1, &check_set, NULL, NULL, NULL);
		for(int i = 0; i < host_num; i++){
			if(FD_ISSET(fd[i][0], &check_set)){
				fscanf(fp[i], "%d%d%d%d%d%d%d%d", &player_id[0], &player_rank[0], &player_id[1], &player_rank[1], &player_id[2], &player_rank[2],
						                     &player_id[3], &player_rank[3]);
				//fprintf(stderr, "%d %d\n%d %d\n%d %d\n%d %d\n", player_id[0], player_rank[0], player_id[1], player_rank[1], player_id[2], player_rank[2],										   player_id[3], player_rank[3]);
				for(int j = 0; j < 4; j++){
					score[player_id[j] - 1] += 4 - player_rank[j];
					//fprintf(stderr, "%d score plus %d is %d\n", player_id[j], player_rank[j], score[player_id[j]]);
				}
				total_comp--;
				if(temp > 0){
					write(filedes[i][1], player[record], strlen(player[record]));
					record++;
					fsync(filedes[i][1]);
					temp--;
				}
		//fprintf(stderr, "host = %d, temp = %d, total_comp = %d, host_num = %d\n", i + 1, temp, total_comp, host_num);
			}

		//fprintf(stderr, "host = %d, temp = %d, total_comp = %d, host_num = %d after if\n", i + 1, temp, total_comp, host_num);
		}
		//fprintf(stderr, "%d left2\n", total_comp);
	}

	//fprintf(stderr, "debug\n");
	//fprintf(stderr, "%d %d %d %d\n", score[0], score[1], score[2], score[3]);
	//fprintf(stderr, "debug\n");

	//printf output
	int order[20], tempo;
	for(int i = 0; i < player_num; i++)
		order[i] = i;

	//sort score
	for(int i = player_num - 1; i >= 0; i--)
		for(int j = 0; j <= i; j++){
			if(score[j] < score[j + 1]){
				tempo = score[j];
				score[j] = score[j + 1];
				score[j + 1] = tempo;
				tempo = order[j];
				order[j] = order[j + 1];
				order[j + 1] = tempo;
			}
		}

	//determine rank
	int rank[20];
	rank[order[0]] = 0;
	for(int i = 1; i < player_num; i++){
		if(score[i] != score[i - 1]){
			rank[order[i]] = i;
		}
		else{
			rank[order[i]] = rank[order[i - 1]];
		}
	}
	for(int i = 0; i < player_num; i++){
		printf("%d %d\n", i + 1, rank[i] + 1);
	}
	//kill host
	for(int i = 0; i < host_num; i++){
		//fprintf(stderr, "finish\n");
		char *finish = "-1 -1 -1 -1\n";
		write(filedes[i][1], finish, strlen(finish));
		fsync(filedes[i][1]);
		//fprintf(stderr, "i = %d, print to host %s\n", i, finish);
	}
	//wait
	int loop_for_wait;
	for(loop_for_wait = 0; loop_for_wait < host_num; loop_for_wait++){
		//fprintf(stderr, "before wait\n");
		wait(NULL);
		//fprintf(stderr, "after wait\n");
	}
	return 0;
}
