
#include <cstdio>   
#include <cstdlib>  
#include <cstring>  
#include <fstream>  
#include <string>   
#include <vector>   
#include <cassert>

using std::ifstream;
using std::string;
using std::vector;

class jobInfo {
public:
	
    jobInfo(string _jobID, int _arrival, int _runtime, int _prty) :
        jobID(_jobID), arrival(_arrival), runtime(_runtime), prty(_prty),
        start(0), end(0), turnaround(0), responsetime(0), timeLeft(0){ }

    
    string jobID;
    int arrival;
    int runtime;
    int prty;
    
    
    int start;
    int end;   
    int turnaround;
    int responsetime;
    int timeLeft;
};


typedef vector<jobInfo>::size_type vsize_type;


void print(const char *headerMsg, vector<jobInfo> &jobs) {
    printf("\n%s\n\n", headerMsg);
    printf("%10s %10s %10s %10s %10s %10s %10s %10s\n\n",
            "ProcessID", "Arrival", "CPU Burst", "Priority", "Start", "Completion",
            "Turnaround", "ResponseTime");
    int totalTurnaround = 0;
    int totalResponsetime = 0;
    for (vsize_type i = 0; i < jobs.size(); ++i) {
        printf("%10s %10d %10d %10d %10d %10d %10d %10d\n",
                jobs[i].jobID.c_str(), 
                jobs[i].arrival, jobs[i].runtime, jobs[i].prty, 
                jobs[i].start, jobs[i].end, jobs[i].turnaround, jobs[i].responsetime);
        totalTurnaround += jobs[i].turnaround;
        totalResponsetime += jobs[i].responsetime;
    }
    printf ("%s \n", "Run Stats:");
    printf("\nAverage turnaround time = %12.2f\n", 
            (double) totalTurnaround / (double) jobs.size());
    printf("\nAverage response time = %12.2f\n", 
            (double) totalResponsetime / (double) jobs.size());
            
}


void scheduleFCFS(vector<jobInfo> &jobs);
void scheduleSJF(vector<jobInfo> &jobs);
void scheduleRR(vector<jobInfo> &jobs, const int quantum);

static bool verbose = false;

int main(int argc, char* argv[]) {

    
    if (argc < 2) {
        fprintf(stderr, "usage:  %s inputfile [--verbose]\n", argv[0]);
        return EXIT_FAILURE;
    }
    verbose = (argc > 2) && (strcmp(argv[2], "--verbose") == 0);

    vector<jobInfo> jobs;

  	//get input
    ifstream infile;
    infile.open(argv[1]);
    if (infile == NULL) {
        fprintf(stderr, "cannot open input file %s\n", argv[1]);
        return EXIT_FAILURE;
    }
    string jobID;
    int arrival, runtime, prty;
   
    while (infile >> jobID >> arrival >> runtime >> prty) {
        jobs.push_back(jobInfo(jobID, arrival, runtime, prty));
    }
    if (!infile.eof()) {
        fprintf(stderr, "invalid input\n");
        return EXIT_FAILURE;
    }
    infile.close();

   
	printf("\nNumber of Jobs in new Q = %12.2f\n",  (double)jobs.size());
    scheduleFCFS(jobs);
    print("Terminated job. First come, first served", jobs);

    scheduleSJF(jobs);
    print("Terminated job. Shortest job first", jobs);

     scheduleRR(jobs, 15);
     print("Terminated job. Round robin, quantum = 15", jobs);


    return EXIT_SUCCESS;
}
void initializeTimeLeft(vector<jobInfo> &jobs) {
    for (vsize_type i = 0; i < jobs.size(); ++i) {
        jobs[i].timeLeft = jobs[i].runtime;
    }
}

// FCFS
void scheduleFCFS(vector<jobInfo> &jobs) {
    int time = 0;                  
    vector<vsize_type> readyQueue; 
    vsize_type nextInput = 0;      

    if (verbose) {
        printf("\nStarting FCFS scheduling\n");
    }

   
    while ((nextInput < jobs.size()) || (readyQueue.size() > 0)) {
        // add jobs with arrival times <= current time to ready queue
        while ((nextInput < jobs.size()) && (jobs[nextInput].arrival <= time)) {
            readyQueue.push_back(nextInput++);
        }
       
        if (readyQueue.size() > 0) {
            vsize_type job = readyQueue[0];
            readyQueue.erase(readyQueue.begin());
            if (verbose) {
                printf("At time %d, starting job %s\n",
                        time, jobs[job].jobID.c_str());
            }
            jobs[job].start = time;
            time += jobs[job].runtime;
            jobs[job].end = time;
            jobs[job].turnaround = jobs[job].end - jobs[job].arrival;
            jobs[job].responsetime = jobs[job].start - jobs[job].arrival;
        }
       
       
        else {
            time = jobs[nextInput].arrival;
        }
    }
}

vsize_type getShortest(vector<jobInfo> &jobs, vector<vsize_type> &readyQueue) {
    assert (readyQueue.size() > 0);
    vsize_type shortest = 0;
    for (vsize_type i = 1; i < readyQueue.size(); ++i) {
        if (jobs[readyQueue[i]].runtime < jobs[readyQueue[shortest]].runtime)
            shortest = i;
    }
    vsize_type returnVal = readyQueue[shortest];
    readyQueue.erase(readyQueue.begin()+shortest);
    return returnVal;
}

// SJF
void scheduleSJF(vector<jobInfo> &jobs) {
    int time = 0;                   
    vector<vsize_type> readyQueue;  
    vsize_type nextInput = 0;       

    if (verbose) {
        printf("\nStarting SJF scheduling\n");
    }

    while ((nextInput < jobs.size()) || (readyQueue.size() > 0)) {
    	
        while ((nextInput < jobs.size()) && (jobs[nextInput].arrival <= time)) {
            readyQueue.push_back(nextInput++);
        }
       
        if (readyQueue.size() > 0) {
            vsize_type job = getShortest(jobs, readyQueue);
            if (verbose) {
                printf("At time %d, starting job %s\n",
                        time, jobs[job].jobID.c_str());
            }
            jobs[job].start = time;
            time += jobs[job].runtime;
            jobs[job].end = time;
            jobs[job].turnaround = jobs[job].end - jobs[job].arrival;
            jobs[job].responsetime = jobs[job].start - jobs[job].arrival;
        }
 
        else {
            time = jobs[nextInput].arrival;
        }
    }
}

// Round robin.
void scheduleRR(vector<jobInfo> &jobs, const int quantum) {
    int time = 0;                   
    vector<vsize_type> readyQueue;  
    vsize_type nextInput = 0;       

    if (verbose) {
        printf("\nStarting round-robin scheduling with time quantum = %d\n",
                quantum);
    }

    
    initializeTimeLeft(jobs);

    
    while ((nextInput < jobs.size()) || (readyQueue.size() > 0)) {
       
        while ((nextInput < jobs.size()) && (jobs[nextInput].arrival <= time)) {
            readyQueue.push_back(nextInput++);
        }
        
        if (readyQueue.size() > 0) {
            vsize_type job = readyQueue[0];
            readyQueue.erase(readyQueue.begin());
            if (verbose) {
                printf("At time %d, starting job %s (time left = %d)\n",
                        time, jobs[job].jobID.c_str(), jobs[job].timeLeft);
            }
          
            if (jobs[job].timeLeft == jobs[job].runtime)
                jobs[job].start = time;
          
            if (quantum < jobs[job].timeLeft) {
                
                time += quantum;
                jobs[job].timeLeft -= quantum;
                readyQueue.push_back(job);
            }
            else {
                
                time += jobs[job].timeLeft;
                jobs[job].end = time;
                jobs[job].turnaround = jobs[job].end - jobs[job].arrival;
                jobs[job].responsetime = jobs[job].start - jobs[job].arrival;
                jobs[job].timeLeft = 0;
            }
        }
        
        else {
            time = jobs[nextInput].arrival;
        }
    }
}
