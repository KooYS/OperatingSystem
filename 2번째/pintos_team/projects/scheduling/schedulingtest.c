#include <stdio.h>
#include <string.h>

#include "threads/thread.h"
#include "threads/synch.h"
#include "devices/timer.h"
#include "projects/scheduling/schedulingtest.h"


#define MAX_THREAD_CNT 5

struct thread_info 
{
	int id;
	int priority;
	int64_t start_time;
	int tick_count;
	struct semaphore sema_join;
};

static void load_thread (void *__ti) 
{
	struct thread_info *ti = (struct thread_info *) __ti;
	int64_t sleep_time = 3 * TIMER_FREQ; 	// 300
	int64_t spin_time = sleep_time + 3 * TIMER_FREQ; // 600
	int64_t last_time = 0;
	// 3초 sleep
	timer_sleep (sleep_time - timer_elapsed (ti->start_time));
	while (timer_elapsed (ti->start_time) < spin_time) {//3~6초까지
		int64_t cur_time = timer_ticks ();
		if (cur_time != last_time) {
			printf("%d_Thread %d got tick. \n",ti->priority, ti->id);
			ti->tick_count++;
		}
		last_time = cur_time;
	}

	sema_up(&ti->sema_join);
}


void run_scheduling_test(char **argv UNUSED)
{
	int i;
	struct thread_info info0[MAX_THREAD_CNT];
	struct thread_info info1[MAX_THREAD_CNT];
	struct thread_info info2[MAX_THREAD_CNT];
	struct thread_info info3[MAX_THREAD_CNT];
	struct thread_info info4[MAX_THREAD_CNT];
	int64_t start_time;

	start_time = timer_ticks();
	for (i=0; i<MAX_THREAD_CNT; i++) {
		struct thread_info *ti_0 = &info0[i];
		struct thread_info *ti_1 = &info1[i];
		struct thread_info *ti_2 = &info2[i];
		struct thread_info *ti_3 = &info3[i];
		struct thread_info *ti_4 = &info4[i];
		char name[30];

		ti_0->id = i;
		ti_0->priority = 0;
		ti_0->start_time = start_time;
		ti_0->tick_count = 0;
		sema_init(&ti_0->sema_join, 0);

		ti_1->id = i;
		ti_1->priority = 1;
		ti_1->start_time = start_time;
		ti_1->tick_count = 0;
		sema_init(&ti_1->sema_join, 0);

		ti_2->id = i;
		ti_2->priority = 2;
		ti_2->start_time = start_time;
		ti_2->tick_count = 0;
		sema_init(&ti_2->sema_join, 0);

		ti_3->id = i;
		ti_3->priority = 3;
		ti_3->start_time = start_time;
		ti_3->tick_count = 0;
		sema_init(&ti_3->sema_join, 0);

		ti_4->id = i;
		ti_4->priority = 4;
		ti_4->start_time = start_time;
		ti_4->tick_count = 0;
		sema_init(&ti_4->sema_join, 0);

		snprintf(name, sizeof name, "queue 0의 %d번" , i);
		thread_create(name, 0, load_thread, ti_0);
		snprintf(name, sizeof name, "queue 1의 %d번" , i);
		thread_create(name, 1, load_thread, ti_1);
		snprintf(name, sizeof name, "queue 2의 %d번" , i);
		thread_create(name, 2, load_thread, ti_2);
		snprintf(name, sizeof name, "queue 3의 %d번" , i);
		thread_create(name, 3, load_thread, ti_3);
		snprintf(name, sizeof name, "queue 4의 %d번" , i);
		thread_create(name, 4, load_thread, ti_4);
	}

	printf("Starting threads took %lld ticks.\n", timer_elapsed (start_time));
	printf("Sleeping until threads join, please wait ... \n");

	for (i=0; i<MAX_THREAD_CNT; i++) {
		sema_down(&info0[i].sema_join);
		sema_down(&info1[i].sema_join);
		sema_down(&info2[i].sema_join);
		sema_down(&info3[i].sema_join);
		sema_down(&info4[i].sema_join);
		printf("0번 큐 Thread %d received %d ticks.\n", i, info0[i].tick_count);
		printf("1번 큐 Thread %d received %d ticks.\n", i, info1[i].tick_count);
		printf("2번 큐 Thread %d received %d ticks.\n", i, info2[i].tick_count);
		printf("3번 큐 Thread %d received %d ticks.\n", i, info3[i].tick_count);
		printf("4번 큐 Thread %d received %d ticks.\n", i, info4[i].tick_count);
	}
}