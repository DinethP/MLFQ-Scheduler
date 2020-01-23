#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lab5_queue.h"

void outprint(int time_x, int time_y, int pid, int arrival_time, int remaining_time);

int run_time;

void sort_by_pid(Process *proc, int proc_num)
{
    int i, j;
    Process dummy;
    for (i = 0; i < proc_num; i++)
    {
        for (j = i; j < proc_num; j++)
        {
            if (proc[i].process_id > proc[j].process_id)
            {
                dummy = proc[j];
                proc[j] = proc[i];
                proc[i] = dummy;
            }
        }
    }
}

void queue_sort(LinkedQueue *queue, int proc_num)
{
    int var = 0;
    Process arr[proc_num];
    for (int i = 0; i < proc_num; i++)
        arr[i].process_id = -2;
    while (queue->next != NULL)
    {
        arr[var] = DeQueue(queue);
        var++;
    }
    sort_by_pid(arr, proc_num);
    for (var = 0; var < proc_num; var++)
    {
        if (arr[var].process_id >= 0)
            EnQueue(queue, arr[var]);
    }
}

void arrival_sort(Process *proc, int proc_num)
{
    int i, j;
    Process dummy;
    for (i = 0; i < proc_num; i++)
    {
        for (j = i; j < proc_num; j++)
        {
            if (proc[i].arrival_time > proc[j].arrival_time)
            {
                dummy = proc[j];
                proc[j] = proc[i];
                proc[i] = dummy;
            }
        }
    }
}

void terminate(Process proc, int start)
{
    run_time += proc.completion_time;
    proc.completion_time = 0;
    outprint(start, run_time, proc.process_id, proc.arrival_time, proc.completion_time);
}

void execute(LinkedQueue **ProcessQueue, int index, int proc_num, int period, int queue_num)
{
    LinkedQueue *queue = ProcessQueue[index];
    Process dummy = DeQueue(queue);
    int start = run_time;
    int condition_slice = ((run_time % period) + queue->time_slice);
    int condition_proc_time = ((run_time % period) + dummy.completion_time);
    if (condition_slice > period)
    {
        if (condition_proc_time > period)
        {
            int difference = (period - (run_time % period));
            dummy.completion_time -= difference;
            run_time += difference;
            dummy.waiting_time = 0;
            outprint(start, run_time, dummy.process_id, dummy.arrival_time, dummy.completion_time);
            EnQueue(ProcessQueue[queue_num - 1], dummy);
        }
        else
            terminate(dummy, start);
    }
    else
    {
        if (dummy.completion_time <= queue->time_slice)
            terminate(dummy, start);
        else
        {
            dummy.completion_time -= queue->time_slice;
            run_time += queue->time_slice;
            dummy.waiting_time += queue->time_slice;
            outprint(start, run_time, dummy.process_id, dummy.arrival_time, dummy.completion_time);
            if (dummy.waiting_time >= queue->allotment_time && index > 0)
            {
                dummy.waiting_time = 0;
                EnQueue(ProcessQueue[index - 1], dummy);
                queue_sort(ProcessQueue[index - 1], proc_num);
            }
            else
                EnQueue(queue, dummy);
        }
    }
}

void scheduler(Process *proc, LinkedQueue **ProcessQueue, int proc_num, int queue_num, int period)
{
    int i = 0, j, proc_indicator = 0;
    int exec_time = 0;
    int pre_period_check = 0;
    int period_check = 0;
    for (j = 0; j < proc_num; j++)
        exec_time += proc[j].execution_time;
    arrival_sort(proc, proc_num);
    run_time = proc[0].arrival_time;
    exec_time = run_time + exec_time;
    while (run_time < exec_time)
    {
        pre_period_check = period_check;
        period_check = run_time / period;
        for (i = proc_indicator; i < proc_num; i++)
        {
            if (run_time >= proc[i].arrival_time)
            {
                proc[i].completion_time = proc[i].execution_time;
                proc[i].waiting_time = 0;
                EnQueue(ProcessQueue[queue_num - 1], proc[i]);
                queue_sort(ProcessQueue[queue_num - 1], proc_num);
                proc_indicator++;
            }
        }
        if (period_check > pre_period_check)
        {
            Process popped;
            for (j = 0; j < queue_num - 1; j++)
            {
                LinkedQueue *pointer = ProcessQueue[j];
                while (pointer->next != NULL)
                {
                    popped = DeQueue(pointer);
                    popped.waiting_time = 0;
                    EnQueue(ProcessQueue[queue_num - 1], popped);
                }
                queue_sort(ProcessQueue[queue_num - 1], proc_num);
            }
        }
        for (j = queue_num - 1; j > -1; j--)
        {
            if (!IsEmptyQueue(ProcessQueue[j]))
            {
                execute(ProcessQueue, j, proc_num, period, queue_num);
                break;
            }
        }
    }
}