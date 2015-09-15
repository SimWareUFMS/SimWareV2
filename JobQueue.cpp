#include "JobQueue.h"

JobQueue::JobQueue(void)
{
	mapSingleJob.clear();
	mapRunningSingleJob.clear();
	vFinishedJobID.clear();
	averageLatencyNumberOfSamples = 0;
	averageLatencySum = 0.0;
	for (int i=0; i<MAX_NUMBER_OF_ELEMENTS_LOG_INTERVAL; ++i) {
		averageLatencyPerIntervalNumberOfSamples[i] = 0;
		averageLatencyPerIntervalSum[i] = 0.0;
		averageLatencyComparedToSWFInPercentage[i] = 0.0;
	}
}

JobQueue::~JobQueue(void)
{
  mapSingleJob.clear();
  mapRunningSingleJob.clear();
  vFinishedJobID.clear();

}

bool JobQueue::IsFinished()
{
	return mapSingleJob.empty();
}

SingleJob* JobQueue::TakeFirst()
{
	if (IsFinished())
		return NULL;
	SingleJob* retval = mapSingleJob.begin()->second;
	mapRunningSingleJob.insert(mapPair(mapSingleJob.begin()->first, mapSingleJob.begin()->second));
	mapSingleJob.erase(mapSingleJob.begin());
	return retval;
}

bool JobQueue::IsTheSingleJobFinished(UINT64 jobNum)
{
	if (jobNum == 0) 
		return true;
	
#ifdef NO_DEP_CHECK
	cout << "Error: dependency check is intentionally disabled. JobNumber: " << jobNum << endl;
	return true;
#endif

//TODO: the following is very slow
	cout << jobNum;
	if (find(vFinishedJobID.begin(), vFinishedJobID.end(), jobNum)!=vFinishedJobID.end())
		return true;
	return false;
}

void JobQueue::OneVMFinishedOnASingleJob(UINT64 jobNum, int clock, float nodeNum)
{
	try {
		SingleJob *sj = mapRunningSingleJob.find(jobNum)->second;
		sj->finishedVMs++;
#ifdef _DEBUG
		sj->physicalNodeNumber.push(nodeNum);
#endif
		if (sj->finishedVMs == sj->numCPUs) { // finished.
			// log
#ifndef NO_DEP_CHECK
			vFinishedJobID.push_back(sj->jobNumber);
#endif
			averageLatencyNumberOfSamples++;
			averageLatencySum += (clock - sj->queuedClock);
			int index = (sj->queuedClock) / PERIODIC_LOG_INTERVAL;
			if (index < MAX_NUMBER_OF_ELEMENTS_LOG_INTERVAL) {
				averageLatencyPerIntervalNumberOfSamples[index]++;
				FLOATINGPOINT elapsedTime = (FLOATINGPOINT)(clock - sj->queuedClock);
				averageLatencyPerIntervalSum[index] += elapsedTime;
				averageLatencyComparedToSWFInPercentage[index] += elapsedTime/(sj->runTimeSecFromSWF)*100.0;
			}

#ifdef _DEBUG
			sj->finishedClock = clock;
#else
			// erase from mapRunningSingleJob & delete SingleJob instant
			mapRunningSingleJob.erase(jobNum);
			delete sj;
#endif
		}
	} catch (exception) {
		cout << "Error: One VM Finished a job that has not been launched" << endl;
	}
}

void JobQueue::InsertASingleJob(UINT64 jobNum, int numCPU, int runTime, int avgCPUTime, int qClock)
{
	SingleJob* newSingleJob = new SingleJob(jobNum, numCPU, runTime, avgCPUTime, qClock);
	mapSingleJob.insert(mapPair(jobNum, newSingleJob));
}

void JobQueue::InsertASingleJob(UINT64 jobNum, int numCPU, int runTime, int avgCPUTime, int qClock, long int usedMem)
{
	SingleJob* newSingleJob = new SingleJob(jobNum, numCPU, runTime, avgCPUTime, qClock, usedMem);
	mapSingleJob.insert(mapPair(jobNum, newSingleJob));
}

void JobQueue::PrintResults(int endedClock)
{
	// Average latency
	cout << "Average Latency: " << (averageLatencySum/(FLOATINGPOINT)averageLatencyNumberOfSamples) << endl << endl;

	// Average latency (every)
	int max = endedClock / PERIODIC_LOG_INTERVAL;
	cout << "Average Latency (every " << PERIODIC_LOG_INTERVAL << " secs):" << endl;
	for (int i=0; i<=max; ++i) {
		if (averageLatencyPerIntervalNumberOfSamples[i]==0)
			cout << "\t";
		else
			cout << (averageLatencyPerIntervalSum[i]/(FLOATINGPOINT)averageLatencyPerIntervalNumberOfSamples[i]) << "\t";
	}
	cout << endl << endl;

	// Average latency in % (every)
	cout << "Average Latency in % (every " << PERIODIC_LOG_INTERVAL << " secs):" << endl;
	for (int i=0; i<=max; ++i) {
		if (averageLatencyPerIntervalNumberOfSamples[i]==0)
			cout << "\t";
		else
			cout << (averageLatencyComparedToSWFInPercentage[i]/(FLOATINGPOINT)averageLatencyPerIntervalNumberOfSamples[i]) << "\t";
	}
	cout << endl << endl;

	// Number of Jobs (every)
	cout << "Number of Jobs (every " << PERIODIC_LOG_INTERVAL << " secs)" << endl;
	for (int i=0; i<=max; ++i) {
		if (averageLatencyPerIntervalNumberOfSamples[i]==0)
			cout << "0\t";
		else
			cout << averageLatencyPerIntervalNumberOfSamples[i] << "\t";
	}
	cout << endl << endl;

#ifdef _DEBUG
	// Latencies
	cout << "--- Latency for every job ---" << endl << "Job Number\tLatency\tnumCPUs\tPhysical Node Number" << endl;
	for (mapIterator it = mapRunningSingleJob.begin(); it != mapRunningSingleJob.end(); ++it) {
		cout << (it->second->jobNumber)  << "\t" << (it->second->finishedClock - it->second->queuedClock) << "\t" << it->second->numCPUs << "\t";
		while(!it->second->physicalNodeNumber.empty()) {
			cout << it->second->physicalNodeNumber.front() << " ";
			it->second->physicalNodeNumber.pop();
		}
		cout << endl;
	}
	cout << endl;
#endif

}
