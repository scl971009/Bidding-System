/*b05902038 林詩芩*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>

int main(int argc, char *argv[])
{
    //deal with input and make FIFO name
    int host_id, random_key;
    char player_index;
    char ff[15], ffe[15];
    host_id = atoi(argv[1]);
    player_index = *argv[2];
    random_key = atoi(argv[3]);
    sprintf(ff, "host%d.FIFO", host_id);
    sprintf(ffe, "host%d_%c.FIFO", host_id, player_index);
    //fprintf(stderr, "open for read %s\n", ffe);

    //competition
    FILE *fp1, *fp2;
    int turn = player_index - 'A', i;
    int money, useless1, useless2, useless3, useless4, fd1, fd2;
    char temp[30];
    //fprintf(stderr, "before competition\n");
    fd1 = open(ffe, O_RDONLY | O_NONBLOCK);
    fp1 = fdopen(fd1, "r");
    assert(fp1 != NULL);
    fd2 = open(ff, O_WRONLY);
    fp2 = fdopen(fd2, "w");
    assert(fp2 != NULL);
    fd_set ready_set, check_set;
    FD_ZERO(&ready_set);
    FD_SET(fd1, &ready_set);
    for(i = 0; i < 10; i++){
        //bet all
        if(i % 4 == turn){
            switch(turn){
            case 0:
		//fprintf(stderr, "is scan all bet A okay?\n");
                check_set = ready_set;
		select(fd1 + 1, &check_set, NULL, NULL, NULL);
		fscanf(fp1, "%d%d%d%d\n", &money, &useless1, &useless2, &useless3);
		//fprintf(stderr, "A of course!\n");
                break;
            case 1:
		//fprintf(stderr, "is scan all bet B okay?\n");
		//read(fd, &money, sizeof(money));
                check_set = ready_set;
		select(fd1 + 1, &check_set, NULL, NULL, NULL);
                fscanf(fp1, "%d %d %d %d\n", &useless1, &money, &useless2, &useless3);
                //fprintf(stderr, "B of course!\n");
		break;
            case 2:
		//fprintf(stderr, "is scan all bet C okay?\n");
		//read(fd, &money, sizeof(money));
                check_set = ready_set;
		select(fd1 + 1, &check_set, NULL, NULL, NULL);
                fscanf(fp1, "%d %d %d %d\n", &useless1, &useless2, &money, &useless3);
                //fprintf(stderr, "C of course!\n");
		break;
            case 3:
	    	//fprintf(stderr, "is scan all bet D okay?\n");
		//read(fd, &money, sizeof(money));
                check_set = ready_set;
		select(fd1 + 1, &check_set, NULL, NULL, NULL);
                fscanf(fp1, "%d %d %d %d\n", &useless1, &useless2, &useless3, &money);
		//fprintf(stderr, "D of course!\n");
		break;
            default:
                fprintf(stderr, "mod error");
                exit(1);
            }

            fprintf(fp2, "%c %d %d\n", player_index, random_key, money);
	    fflush(fp2);
	    fsync(fd2);
	    //fprintf(stderr, "%c bet %d\n", player_index, money);
        }
        //bet 0
        else{
	    //fprintf(stderr, "is %c scan 0 okay?\n", player_index);
            check_set = ready_set;
	    select(fd1 + 1, &check_set, NULL, NULL, NULL);
            fscanf(fp1, "%d %d %d %d\n", &useless1, &useless2, &useless3, &useless4);
            //fprintf(stderr, "%c i think so\n", player_index);
            fprintf(fp2, "%c %d 0\n", player_index, random_key);
            fflush(fp2);
	    fsync(fd2);
	    //fprintf(stderr, "%c bet zero\n", player_index);
        }
    }
    return 0;
}
