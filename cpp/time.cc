#include "time.h"
#include <sys/time.h>

double get_current_time() {
	timeval t;
	gettimeofday(&t, 0);//取得当前时间(保存在结构体timeval中)
	return (double) t.tv_sec + (double) t.tv_usec / 1000000;//"sec" - 自 Unix 纪元起的秒数"usec" - 微秒数
}

const int N_Timers = 7; //currently, 6 timers are available  time.h  enum TIMERS 只有0-6
static double _timers[N_Timers]; // timers  current time
static double _acc_time[N_Timers]; // accumulated time累计时间

void init_timers() {
	for (int i = 0; i < N_Timers; i++) {
		_acc_time[i] = 0;
	}
}


//currently, only 6 timers are used, others can be defined by users

void start_timer(int i) {
	_timers[i] = get_current_time();
}

void reset_timer(int i) {
	_timers[i] = get_current_time();
	_acc_time[i] = 0;
}

void stop_timer(int i) {
	double t = get_current_time();
	_acc_time[i] += t - _timers[i];
}

double get_timer(int i) {
	return _acc_time[i];
}
