#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define LOG_FILE "/var/log/volume-control-panel.log"

int open_log_file(char *log_file_path);
int open_tty(char *path);

int main(int argc, char **argv){
	if (open_log_file(LOG_FILE) < 0){
		fprintf(stderr,"could not open log file.\n");
		exit(EXIT_FAILURE);
	}
	if (argc != 2){
		fprintf(stderr,"invalaid args provided.\n");
		return 1;
	}
	printf("started, tty=%s\n",argv[1]);

	//======= get a file descriptor for the panel =====
	int panel_fd = open_tty(argv[1]);

	return 0;
}
int open_tty(char *path){
}
int open_log_file(char *log_file_path){
	//check we can open it and create if it doesnt exist
	FILE *log_file = fopen(log_file_path,"a");
	if (log_file == NULL){
		perror("fopen");
		return -1;
	}
	//figure out the date and time
	time_t time_now = time(NULL);
	struct tm *tm = localtime(&time_now);
	char time_string[100];
	int time_string_length = strftime(time_string,sizeof(time_string),"%d-%m-%Y %H:%M",tm);

	//write the date and time
	int result = fwrite(time_string,1,time_string_length,log_file);
	if (result < 0){
		perror("fwrite");
		fclose(log_file);
		return -1;
	}
	result = fprintf(log_file,"\n");
	if (result < 0){
		perror("fwrite");
		fclose(log_file);
		return -1;
	}

	fclose(log_file);
	//close

	//re open stdout and stderr to the log file
	freopen(log_file_path,"a",stderr);
	freopen(log_file_path,"a",stdout);
	return 0; 
}
