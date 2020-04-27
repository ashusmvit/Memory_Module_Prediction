#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
const int C = 10e6;
const int CLow = 100;   //prevent the simulation stops very early
const double delta = 0.02;

//data structure for processors
typedef struct processor{
    int request;
    int accessNum;
    bool prevConnected;
    //int priority;
}processor;

//Arrays for memories, processors and memory distribution
int memories[2048] = {0};
processor processors[64];
int memoryDistribution[64] = {0};

//return a uniform (random) memory distribution
void getUniformMemoryDistribution(int memoryNum, int processorNum, int memoryDistribution[]){
    if(memoryNum == 1)
        memoryDistribution[0] = 0;
    else{
        for(int i = 0; i < processorNum; i++)
        	if(processors[i].prevConnected == true)
            	memoryDistribution[i] = rand() % memoryNum;
    		else
    			continue;
    }
}


//Box muller method-referenced from StackOverflow 
double random_value(double mean, double stddev)
{
    static double value1 = 0.0;
    static int value2 = 0;
    if (!value2)
    {
        double X, Y, r;
        do
        {
            X = 2.0*rand()/RAND_MAX - 1;
            Y = 2.0*rand()/RAND_MAX - 1;

            r = X*X + Y*Y;
        }
        while (r == 0.0 || r > 1.0);
        {
            double d = sqrt(-2.0*log(r)/r);
            double diff = X*d;
            value1 = Y*d;
            double result = diff*stddev + mean;
            value2 = 1;
            return result;
        }
    }
    else
    {
        value2 = 0;
        return value1*stddev + mean;
    }
}


//return a normal memory distribution
void getNormalMemoryDistribution(int memoryNum, int processorNum, int memoryDistribution[]){
double sd = memoryNum/6;
getUniformMemoryDistribution(memoryNum, processorNum, memoryDistribution);
for(int i = 0; i<processorNum; i++){
		if(processors[i].prevConnected == true){
        	double value = random_value(memoryDistribution[i], sd);
        	memoryDistribution[i] = ((int)value + memoryNum) % memoryNum;
        	}
}
}


void initializeProcessors(processor processors[], int n){
    for(int i = 0; i < n; i++){
        processors[i].accessNum = 0;
    	processors[i].prevConnected = true;
}
}

void initializeMemories(int memories[], int n){
    for(int i = 0; i < n; i++)
        memories[i] = 0;
}

void simulation(int processorNum, char memoryMode){
    //memoryMode = u: uniform distribution, mode = n: normal distribution
    if(memoryMode == 'u')
        printf("Uniform Memory Distribution on %d processors\n", processorNum);
    else
        printf("Normal Memory Distribution on %d processors\n", processorNum);
    double averageW = 0;
    int memoryNum;
    FILE *fp;
    fp = fopen("data.txt","w");
    for(memoryNum = 1; memoryNum <= 2048; memoryNum++){
        //initialize processor array before the simulation starts
        initializeProcessors(processors, processorNum);

        int start = 0;
        //calling for C cycles
        for(int c = 1; c <= C; c++)
        {
            //initialize memories array before each simulation turn
            initializeMemories(memories, memoryNum);
            if(memoryMode == 'u')
                getUniformMemoryDistribution(memoryNum, processorNum, memoryDistribution);
            else
                getNormalMemoryDistribution(memoryNum, processorNum, memoryDistribution);
            int nextStart = start;  //store the next start position (the first starving memory)
            int count = 0;
            for(int k = 0; k < processorNum; k++)
            {
                //relabel
                if(start + k == processorNum)
                    start = -k;     //let start + k = 0

                if(memories[memoryDistribution[k]] == 0)
                {
                    //request successfully
                    processors[start + k].request = memoryDistribution[k];
                    processors[start + k].accessNum++;
                    memories[memoryDistribution[k]] = 1;
                    processors[start + k].prevConnected = true;
                }
                else if(count == 0)
                {
                    //first memory that fails to request a memory
                    processors[start+k].prevConnected = false;
                    count++;
                    nextStart = start + k;
                }
            }
            start = nextStart;

            //calculate averageW
            double averageAccessTimeSum = 0;
            int processorId;
            for(processorId = 0; processorId < processorNum; processorId++)
            {
                if(processors[processorId].accessNum == 0)
                    //At least one processor fails every time
                    break;
                else
                    //average access time = memory cycle / access number so far
                    averageAccessTimeSum += ((double)c / processors[processorId].accessNum);
            }
            if(processorId == processorNum)
            {
                //All processors succeed for at least 1 time
                //averageW = sum(avg access time) / num of processor
                double newAverageW = averageAccessTimeSum / processorNum;
                if(c > CLow && fabs(1 - averageW / newAverageW) < delta)
                {
                    //abs(1-Wc-1/Wc) < delta
                    averageW = newAverageW;
                    break;
                }
                else
                    averageW = newAverageW;
            }
        }
    fprintf(fp, "%lf\n", averageW);
	fflush(fp);
    printf("%.4f\n", averageW);
    }
  fclose(fp);
}

int main(int argc, char *argv[]) {
    int processorNum = atoi(argv[1]);
    char mode = argv[2][0];

    simulation(processorNum, mode);

    return 0;
}
