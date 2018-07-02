#ifndef UTILS_TIME_H_
#define UTILS_TIME_H_

#define StartTimer(i) start_timer((i))
#define StopTimer(i) stop_timer((i))
#define ResetTimer(i) reset_timer((i))
#define PrintTimer(str, i)              \
    if (get_worker_id() == MASTER_RANK) \
        printf("%s : %f seconds\n", (str), get_timer((i)));

enum TIMERS {
	WORKER_TIMER = 0,
	SERIALIZATION_TIMER = 1,
	TRANSFER_TIMER = 2,
	COMMUNICATION_TIMER = 3,
	EVALUATION_TIMER = 4,
	ASSEMBLE_TIMER  = 5,
	INDEX_TIMER  = 6
};

double get_current_time();
void init_timers();
void start_timer(int);
void reset_timer(int);
void stop_timer(int);
double get_timer(int);


#endif /* UTILS_TIME_H_ */
