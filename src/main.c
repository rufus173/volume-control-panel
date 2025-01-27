#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>

#define LOG_FILE "/var/log/volume-control-panel.log"
#define POT_MARGIN 5

int nearest_5(int val){
	int modval = val % 5;
	if (modval >= 3) return val + 5 - modval;
	else return val - modval;
}

int open_log_file(char *log_file_path);
int open_tty(char *path);
char *readline(int fd);
int set_output_volume(int volume);
int set_input_volume(int volume);

int main(int argc, char **argv){
	int exit_status = EXIT_SUCCESS;

	int result = 0;
	result = open_log_file(LOG_FILE);
	if (result < 0){
		fprintf(stderr,"could not open log file.\n");
		exit(EXIT_FAILURE);
	}
	if (argc != 2){
		fprintf(stderr,"invalaid args provided.\n");
		exit(EXIT_FAILURE);
	}
	printf("started, tty=%s\n",argv[1]);

	//======= get a file descriptor for the panel =====
	printf("connecting board...\n");

	int panel_fd = open_tty(argv[1]);
	if (panel_fd < 0){fprintf(stderr,"could not open serial connection.\n"); exit(EXIT_FAILURE);}
	
	printf("board connected.\n");

	int old_pot_1_percent = 0;
	int old_pot_2_percent = 0;
	//======= read data from the board ======
	for (;;){
		char *line = readline(panel_fd);
		if (line == NULL){
			fprintf(stderr,"could not read line.\n");
			exit_status = EXIT_FAILURE;
			goto cleanup;
		}

		//discard empty lines
		if (strlen(line) < 1){
			free(line);
			continue;
		}

		char *endptr;
		int pot_1_percent = strtol(line,&endptr,10);
		endptr++;
		int pot_2_percent = strtol(endptr,NULL,10);
		//printf("%s - %s\n",line,endptr);
		//printf("%d\n",pot_2_percent);
		if ((pot_1_percent < (old_pot_1_percent - POT_MARGIN)) || (pot_1_percent > (old_pot_1_percent + POT_MARGIN))){
			old_pot_1_percent = pot_1_percent;
			set_output_volume(nearest_5(pot_1_percent));
		}
		if ((pot_2_percent < (old_pot_2_percent - POT_MARGIN)) || (pot_2_percent > (old_pot_2_percent + POT_MARGIN))){
			old_pot_2_percent = pot_2_percent;
			set_input_volume(nearest_5(pot_2_percent));
		}
		free(line);
	}

	//====== cleanup ======
	cleanup:
	close(panel_fd);
	printf("board disconnected.\n");
	fflush(stdout);
	fflush(stderr);
	return exit_status;
}
//stolen from c-code/source-code/foot-pedal-driver.c
int open_tty(char *path){
	//open device
	int board_fd = open(path,O_RDWR);
	if (board_fd < 0){
		perror("open");
		return -1;
	}

	//set baud rate
	struct termios options;
	int result = tcgetattr(board_fd, &options); //fill in the current options
	if (result < 0){perror("tcgetattr");return -1;}
	
	result = cfsetispeed(&options,B9600);
	if (result < 0){perror("cfsetispeed");return -1;}

	result = cfsetospeed(&options,B9600);
	if (result < 0){perror("cfsetospeed");return -1;}
	
	//set other important attributes
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;
	// no flow control
	options.c_cflag &= ~CRTSCTS;
	options.c_cflag |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines
	options.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // make raw
	options.c_oflag &= ~OPOST; // make raw

	//timings for bytes in control sequences
	options.c_cc[VMIN]  = 0;
	options.c_cc[VTIME] = 20;

	result = tcsetattr(board_fd,TCSANOW,&options); //make changes immediately
	if (result < 0){perror("tcsetattr");return -1;}

	tcgetattr(board_fd, &options);

	result = tcflush(board_fd,TCIOFLUSH); //flush any data
	if (result < 0){perror("tcflush");return -1;}

	return board_fd;
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
	FILE *file_result = freopen(log_file_path,"a",stderr);
	if (file_result == NULL){perror("freopen");return -1;}
	//stderr = file_result;

	file_result = freopen(log_file_path,"a",stdout);
	if (file_result == NULL){perror("freopen");return -1;}
	//stdout = file_result;

	return 0; 
}
char *readline(int fd){
	char *line = malloc(1);
	size_t line_size = 1;
	line[0] = '\0';
	for (;;){
		char buffer[1] = {'\0'};
		int result = read(fd,buffer,sizeof(buffer));
		if (result < 0){
			perror("read");
			free(line);
			return NULL;
		}
		if (buffer[0] == '\n'){ //line ends in crlf
			if (line[line_size-2] == '\r'){
				line[line_size-2] = '\0'; //remove the '\r'
			}
			return line;
		}
		line_size++;
		line = realloc(line,line_size);
		if (line == NULL){
			perror("realloc");
			return NULL;
		}
		line[line_size-2] = buffer[0];
		line[line_size-1] = '\0';
	}
}
int set_output_volume(int volume){
	//printf("%d\n",volume);
	char command_buffer[200];
	snprintf(command_buffer,sizeof(command_buffer),"sudo -u \"#1000\" amixer -q -D pulse sset Master %d%%",volume);
	system(command_buffer);
}
int set_input_volume(int volume){
	char command_buffer[200];
	//printf("%d\n",volume);
	//snprintf(command_buffer,sizeof(command_buffer),"sudo -u \"#1000\" pactl set-source-volume @DEFAULT_SOURCE@ %d%%",volume);
	snprintf(command_buffer,sizeof(command_buffer),"sudo -u \"#1000\" pactl set-source-volume @DEFAULT_SOURCE@ %d%%",volume);
	system(command_buffer);
}
