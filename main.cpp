#include <iostream>
#include <stdio.h>  /* defines FILENAME_MAX */
#ifdef WIN32
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <iterator>
#include <algorithm>
#include <cstdlib>
#include <conio.h>
#include <math.h> 
#include "Constants.h"
#include "DataCenter.h"
#include "Server.h"
#include "Users.h"
#include "JobQueue.h"
#include "PoolServers.h"
#include "Topology.h"

using namespace std;

int NUMBER_OF_CHASSIS = 50; // how many blade server chassis in the data center?
int NUMBER_OF_SERVERS_IN_ONE_CHASSIS = 10;
int NUMBER_OF_CORES_IN_ONE_SERVER = 10;
long int TOTAL_OF_MEMORY_IN_ONE_SERVER = 16000000;   // in kbytes 
int PERIODIC_LOG_INTERVAL = 86400; // in seconds. shorter interval requires more memory space
int SKIP_x_SECONDS = 0; // skip first ? seconds
int FINISHES_AT_DAY = 0; // stop simulator at day FINISHES_AT_DAY
string SCHEDULING_ALGORITHM = "2D_POOL";
int SUPPLY_TEMPERATURE_OFFSET_ALPHA = 0; // T_trigger = T_emergency + alpha
int CRAC_SUPPLY_TEMP_IS_CONSTANT_AT = 0; // 0 = false, use dynamic crac control
bool INSTANT_COOL_AIR_TRAVEL_TIME = false; // true = cool air from CRAC arrives instantly to the servers
bool CONSTANT_FAN_POWER = false; // true = fan rpm is constant at max
bool INSTANT_CHANGE_CRAC_SUPPLY_AIR = false; // true = CRAC changes discharge air temperature instantly (e.g., from 10C to 15C or 
int CRAC_DISCHARGE_CHANGE_RATE_0_00x = 10;
bool FAN_RPM_NO_LIMIT = false; // false = Fan's max rpm to set at 3000, use true for SUPPLY_TEMPERATURE_OFFSET_ALPHA >= 0


bool SIMULATES_MIGRATION_VMS = true;
bool SIMULATES_NETWORK = true;
bool SIMULATES_POOL_SERVER = true;

int TRANSFER_TIME_VMS = 60; // transfer time between VMs
int POWER_ON = 120; // time to power on a server

// parameters for future work
int COOL_AIR_TRAVEL_TIME_MULTIPLIER = 1;
bool TEMPERATURE_SENSING_PERFORMANCE_CAPPING = false;
int TEMPERATURE_SENSING_PERFORMANCE_CAPPING_AGGRESSIVENESS = 0;
bool REASSIGN_VMS = false;

//prediciton
bool PREDICITIONSHEDULER = true;

string PREDICTION_ALGORITHM = "POLYNOM";

//parameters prediction algorithm RBF and POLYNOM
bool PRINTSCREEN = false;			//
int SIZEWINDOWNPREDICTION = 600;	//	Size Windown Prediction
int MONITORINGTIME = 120;			//  Monitoring Time in Seconds

// parameters prediction algorithm RBF 
int DIM = 2;						//-m embedding dimension [default: 2]
int DELAY = 1;						//-d delay [default: 1]
int CENTER = 10;					    //-p number of centers [default: 10]
int STEP = 1;						//-s steps to forecast [default: 1]
unsigned int COLUMN = 1;			//-c column to read [default: 1]
unsigned long LENGTH = ULONG_MAX;	//-l # of data to use [default: all from vector]
long CLENGTH = 5;					//-L steps to cast [default: none]
char MAKECAST = 1;					// not change

//parameters prediction algorithm POLYNOM
int N = 2;							// -p order of the polynomial [default: 2]\n
char CAST = 1;						// not change

//prediction of parameters to calculate the total servers that must remain connected

int SUMVMARRIVALTIME = 120; // INTERVAL TIME MONITORING FOR THE ARRIVAL OF VMS
int SIZEWINDOWNPREDICTIONPOOLSERVER = 360; // Size Windown Prediction for Pool Servers

// Parameter Network topology three-tier
	
int NUMBER_CHASSI_RACK = 4;							// Number of Chassis on a rack. Size Chassi is 10U

int LEVEL_OF_REDUNDANCY_OF_LINKS_N0N1 = 2;			//level of redundancy of links n0n1 ( server -> access switch)	
int LEVEL_OF_REDUNDANCY_OF_LINKS_N1N2 = 2;			//level of redundancy of links n1n2 ( access switch -> aggregation switch)		
int LEVEL_OF_REDUNDANCY_OF_LINKS_N2N3 = 2;			//level of redundancy of links n1n2 ( aggregation switch -> core switch)

int NUMBER_OF_PORTS_SWITCH_ACCESS = 48;				// Number of Ports of Access Switch
int NUMBER_OF_PORTS_SWITCH_AGGREGATION = 24;		// Number of Ports of Access Aggregation
int NUMBER_OF_PORTS_SWITCH_CORE = 24;				// Number of Ports of Access Core


int SWITCH_ACCESS_SPEED = 1000;				// 10 100 1000 10000 mbit/s
double SWITCH_ACCESS_DELAY = 0.1;			// delay (microseconds)

int SWITCH_AGGREGATION_SPEED = 10000;		// 10 100 1000 10000 mbit/s
double SWITCH_AGGREGATION_DELAY = 0.01;		// delay (microseconds)

int SWITCH_CORE_SPEED = 10000;				// 10 100 1000 10000 mbit/s
double SWITCH_CORE_DELAY = 0.001;			// delay (microseconds)

double EACCE_CHASSIS = 100;					// power consumption of the chassis
double EACCE_LINECARD	= 20;				// power consumption of the line card	
double EACCE_PORT = 1;						//power consumption of the port

double EAGGR_CHASSIS = 200;					// power consumption of the chassis
double EAGGR_LINECARD	= 40;				// power consumption of the line card
double EAGGR_PORT = 2;						//power consumption of the port

double ECORE_CHASSIS = 300;					// power consumption of the chassis 
double ECORE_LINECARD = 60;					// power consumption of the line card
double ECORE_PORT = 3;						//power consumption of the port


int SWITCH_ACCESS =  (ceil((double) ((NUMBER_OF_SERVERS_IN_ONE_CHASSIS * NUMBER_CHASSI_RACK)+LEVEL_OF_REDUNDANCY_OF_LINKS_N0N1) / (double) NUMBER_OF_PORTS_SWITCH_ACCESS) * ceil( (double) NUMBER_OF_CHASSIS / (double) NUMBER_CHASSI_RACK)) * LEVEL_OF_REDUNDANCY_OF_LINKS_N0N1;	// Number switches in ACCESS network
int SWITCH_AGGREGATION = ceil( (double) (SWITCH_ACCESS + LEVEL_OF_REDUNDANCY_OF_LINKS_N1N2)  / (double) NUMBER_OF_PORTS_SWITCH_AGGREGATION ) * LEVEL_OF_REDUNDANCY_OF_LINKS_N1N2;		// Number of Switches in AGGREGATION network 	
int SWITCH_CORE	=  ceil( (double) SWITCH_AGGREGATION / (double) NUMBER_OF_PORTS_SWITCH_CORE ) *  LEVEL_OF_REDUNDANCY_OF_LINKS_N2N3;				// Number of L3 Switches in the CORE network


void PrintUsage()
{
	cout << "Usage: SimWare.exe <SWF file name>" << endl;
	cout << "\t[-NUMBER_OF_CHASSIS #(1-50)] (default: "<< NUMBER_OF_CHASSIS <<")" << endl;
	cout << "\t[-NUMBER_OF_SERVERS_IN_ONE_CHASSIS #(1-10)] (default: "<<NUMBER_OF_SERVERS_IN_ONE_CHASSIS<<")" << endl;
	cout << "\t[-NUMBER_OF_CORES_IN_ONE_SERVER #(1-)] (default: "<<NUMBER_OF_CORES_IN_ONE_SERVER<<")" << endl;
	cout << "\t[-TOTAL_OF_MEMORY_IN_ONE_SERVER #(1-)] (default: "<<TOTAL_OF_MEMORY_IN_ONE_SERVER<<")" << endl;
	cout << "\t[-PERIODIC_LOG_INTERVAL #(500- in seconds)] (default: "<< PERIODIC_LOG_INTERVAL <<")" << endl;
	cout << "\t[-SKIP_x_SECONDS #(any, in seconds)] (default: "<<SKIP_x_SECONDS<<")" << endl;
	cout << "\t[-FINISHES_AT_DAY #(1-)] (default: "<<FINISHES_AT_DAY<<", until all jobs are done)" << endl;

	cout << "\t[-SCHEDULING_ALGORITHM best_performance | uniform_task | low_temp_first | min_hr | random | center_rack_first ] (default: "<< SCHEDULING_ALGORITHM <<")" << endl;
	cout << "\t[-SUPPLY_TEMPERATURE_OFFSET_ALPHA #(any integer)] (default: " << SUPPLY_TEMPERATURE_OFFSET_ALPHA << ")" << endl;
	cout << "\t[-CRAC_SUPPLY_TEMP_IS_CONSTANT_AT #(1-)] (default: " << CRAC_SUPPLY_TEMP_IS_CONSTANT_AT << ")" << endl;
	cout << "\t[-INSTANT_COOL_AIR_TRAVEL_TIME] (default: "<<INSTANT_COOL_AIR_TRAVEL_TIME<<")" << endl;
	cout << "\t[-CONSTANT_FAN_POWER] (default: "<<CONSTANT_FAN_POWER<<")" << endl;
	cout << "\t[-INSTANT_CHANGE_CRAC_SUPPLY_AIR] (default: "<<INSTANT_CHANGE_CRAC_SUPPLY_AIR<<")" << endl;
	cout << "\t[-CRAC_DISCHARGE_CHANGE_RATE_0_00x #(5,10,20)] (default: " << CRAC_DISCHARGE_CHANGE_RATE_0_00x << ")" << endl;
	cout << "\t[-FAN_RPM_NO_LIMIT] (default: " << FAN_RPM_NO_LIMIT << ")" << endl;
	
	cout << "\t[-PREDICITIONSHEDULER] (default: " << PREDICITIONSHEDULER << ")" << endl;
	cout << "\t[-PREDICTION_ALGORITHM] (default: " << PREDICTION_ALGORITHM << ")" << endl;

	cout << "\t[-TRANSFER_TIME_VMS] (default: " << TRANSFER_TIME_VMS << ")" << endl;
	cout << "\t[-POWER_ON] (default: " << POWER_ON << ")" << endl;

	cout << "\t[-SIZEWINDOWNPREDICTION] (default: " << SIZEWINDOWNPREDICTION << ")" << endl;
	cout << "\t[-MONITORINGTIME] (default: " << MONITORINGTIME << ")" << endl;


	cout << "\t[-DIM] (default: " << DIM  << ")" << endl;
	cout << "\t[-DELAY] (default: " << DELAY << ")" << endl;
	cout << "\t[-STEP] (default: " << STEP << ")" << endl;
	cout << "\t[-CENTER] (default: " << CENTER << ")" << endl;
	cout << "\t[-STEP] (default: " << STEP << ")" << endl;
	cout << "\t[-COLUMN] (default: " << STEP << ")" << endl;
	cout << "\t[-LENGTH] (default: " << LENGTH << ")" << endl;
	cout << "\t[-CLENGTH] (default: " << CLENGTH << ")" << endl;
	cout << "\t[-MAKECAST] (default: " << MAKECAST << ")" << endl;

	cout << "\t[-N] (default: " << N << ")" << endl;
	cout << "\t[-CAST] (default: " << CAST << ")" << endl;


#ifdef _DEBUG
	cout << "\t[-COOL_AIR_TRAVEL_TIME_MULTIPLIER #1-] (default: "<< COOL_AIR_TRAVEL_TIME_MULTIPLIER <<")" << endl;
	cout << "\t[-TEMPERATURE_SENSING_PERFORMANCE_CAPPING] (default: "<<TEMPERATURE_SENSING_PERFORMANCE_CAPPING<<")" << endl;
	cout << "\t[-TEMPERATURE_SENSING_PERFORMANCE_CAPPING_AGGRESSIVENESS #(0-9)] (default: "<<TEMPERATURE_SENSING_PERFORMANCE_CAPPING_AGGRESSIVENESS<<")" << endl;
	cout << "\t[-REASSIGN_VMS] (default: "<< REASSIGN_VMS << ", reassign all vms every 4.5 hours)" << endl;
#endif

}

bool ParsingArguments(int argc, char* argv[])
{
	if (argc < 2) {
		PrintUsage();
		return false;
	}
	int optind=2;
	while ((optind < argc) && (argv[optind][0]=='-')) {
		string sw = argv[optind];
		if (sw=="-TEMPERATURE_SENSING_PERFORMANCE_CAPPING") {
			TEMPERATURE_SENSING_PERFORMANCE_CAPPING = true;
		}
		else if (sw=="-TEMPERATURE_SENSING_PERFORMANCE_CAPPING_AGGRESSIVENESS") {
			optind++;
			TEMPERATURE_SENSING_PERFORMANCE_CAPPING_AGGRESSIVENESS = atoi(argv[optind]);
			if (TEMPERATURE_SENSING_PERFORMANCE_CAPPING_AGGRESSIVENESS > 20) {
				cout << "Error: -TEMPERATURE_SENSING_PERFORMANCE_CAPPING_AGGRESSIVENESS has to be less than or equal to 9" << endl;
				return false;
			} else if (TEMPERATURE_SENSING_PERFORMANCE_CAPPING_AGGRESSIVENESS < 0) {
				cout << "Error: -TEMPERATURE_SENSING_PERFORMANCE_CAPPING_AGGRESSIVENESS has to be larger than or equal to 0" << endl;
				return false;
			}
			if (TEMPERATURE_SENSING_PERFORMANCE_CAPPING==false) {
				cout << "Error: You have to enable -TEMPERATURE_SENSING_PERFORMANCE_CAPPING before setting -TEMPERATURE_SENSING_PERFORMANCE_CAPPING_AGGRESSIVENESS" << endl;
				return false;
			}
		}
		else if (sw=="-NUMBER_OF_SERVERS_IN_ONE_CHASSIS") {
			optind++;
			NUMBER_OF_SERVERS_IN_ONE_CHASSIS = atoi(argv[optind]);
			if (NUMBER_OF_SERVERS_IN_ONE_CHASSIS > NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX) {
				cout << "Error: -NUMBER_OF_SERVERS_IN_ONE_CHASSIS too big" << endl;
				return false;
			} else if (NUMBER_OF_SERVERS_IN_ONE_CHASSIS < 1) {
				cout << "Error: -NUMBER_OF_SERVERS_IN_ONE_CHASSIS too small" << endl;
				return false;
			}
		}
		else if (sw=="-NUMBER_OF_CORES_IN_ONE_SERVER") {
			optind++;
			NUMBER_OF_CORES_IN_ONE_SERVER = atoi(argv[optind]);
			if (NUMBER_OF_CORES_IN_ONE_SERVER < 1) {
				cout << "Error: -NUMBER_OF_CORES_IN_ONE_SERVER too small" << endl;
				return false;
			}
		}
		else if (sw=="-SCHEDULING_ALGORITHM") {
			optind++;
			SCHEDULING_ALGORITHM = argv[optind];
		}
		else if (sw=="-PERIODIC_LOG_INTERVAL") {
			optind++;
			PERIODIC_LOG_INTERVAL = atoi(argv[optind]);
			if (PERIODIC_LOG_INTERVAL < 3600)
				cout << "Warning: -PERIODIC_LOG_INTERVAL too small" << endl;
		}
		else if (sw=="-SKIP_x_SECONDS") {
			optind++;
			SKIP_x_SECONDS = atoi(argv[optind]);
			if (SKIP_x_SECONDS < 0)
				cout << "Warning: -SKIP_x_SECONDS too small" << endl;
		}
		else if (sw=="-NUMBER_OF_CHASSIS") {
			optind++;
			NUMBER_OF_CHASSIS = atoi(argv[optind]);
			if (NUMBER_OF_CHASSIS < 1 || NUMBER_OF_CHASSIS > 50) {
				cout << "Error: -NUMBER_OF_CHASSIS too small or too big. Use 1-50: " << NUMBER_OF_CHASSIS << endl;
				return false;
			}
		}
		else if (sw=="-COOL_AIR_TRAVEL_TIME_MULTIPLIER") {
			optind++;
			COOL_AIR_TRAVEL_TIME_MULTIPLIER = atoi(argv[optind]);
			if (COOL_AIR_TRAVEL_TIME_MULTIPLIER < 0 || COOL_AIR_TRAVEL_TIME_MULTIPLIER > 10) {
				cout << "Error: -COOL_AIR_TRAVEL_TIME_MULTIPLIER too small or too big. " << COOL_AIR_TRAVEL_TIME_MULTIPLIER << endl;
				return false;
			}
		}
		else if (sw=="-SUPPLY_TEMPERATURE_OFFSET_ALPHA") {
			optind++;
			SUPPLY_TEMPERATURE_OFFSET_ALPHA = atoi(argv[optind]);
			if (SUPPLY_TEMPERATURE_OFFSET_ALPHA < -10 || SUPPLY_TEMPERATURE_OFFSET_ALPHA > 20)
				cout << "Warning: -SUPPLY_TEMPERATURE_OFFSET_ALPHA too small or too big: " << SUPPLY_TEMPERATURE_OFFSET_ALPHA << endl;
		}
		else if (sw=="-CRAC_SUPPLY_TEMP_IS_CONSTANT_AT") {
			optind++;
			CRAC_SUPPLY_TEMP_IS_CONSTANT_AT = atoi(argv[optind]);
			if (CRAC_SUPPLY_TEMP_IS_CONSTANT_AT < LOWEST_SUPPLY_TEMPERATURE 
				|| CRAC_SUPPLY_TEMP_IS_CONSTANT_AT > EMERGENCY_TEMPERATURE) {
				cout << "Error: -CRAC_SUPPLY_TEMP_IS_CONSTANT_AT too small or too big. " << CRAC_SUPPLY_TEMP_IS_CONSTANT_AT << endl;
				return false;
			}
		}
		else if (sw=="-CRAC_DISCHARGE_CHANGE_RATE_0_00x") {
			optind++;
			CRAC_DISCHARGE_CHANGE_RATE_0_00x = atoi(argv[optind]);
			if (CRAC_DISCHARGE_CHANGE_RATE_0_00x < 1 
				|| CRAC_DISCHARGE_CHANGE_RATE_0_00x > 50) {
				cout << "Error: -CRAC_DISCHARGE_CHANGE_RATE_0_00x too small or too big. " << CRAC_DISCHARGE_CHANGE_RATE_0_00x << endl;
				return false;
			}
		}
		else if (sw=="-FINISHES_AT_DAY") {
			optind++;
			FINISHES_AT_DAY = atoi(argv[optind]);
			if (FINISHES_AT_DAY < -1) {
				cout << "Error: -FINISHES_AT_DAY too small :" << FINISHES_AT_DAY << endl;
				return false;
			}
		}
		else if (sw=="-REASSIGN_VMS") {
			REASSIGN_VMS = true;
		}
		else if (sw=="-FAN_RPM_NO_LIMIT") {
			FAN_RPM_NO_LIMIT = true;
		}
		else if (sw=="-INSTANT_COOL_AIR_TRAVEL_TIME") {
			INSTANT_COOL_AIR_TRAVEL_TIME = true;
		}
		else if (sw=="-CONSTANT_FAN_POWER") {
			CONSTANT_FAN_POWER = true;
		}
		else if (sw=="-INSTANT_CHANGE_CRAC_SUPPLY_AIR") {
			INSTANT_CHANGE_CRAC_SUPPLY_AIR = true;
		}
		else {
			cout << "Error: Unknown switch: " << argv[optind] << endl;
			PrintUsage();
			return false;
		}
		optind++;
	}

	cout << endl;
	
    cout << "-SIMULATES_MIGRATION_VMS : " << SIMULATES_MIGRATION_VMS << endl;
	cout << "-SIMULATES_NETWORK : " << SIMULATES_NETWORK << endl;
	cout << "-SIMULATES_POOL_SERVER : " << SIMULATES_POOL_SERVER << endl;
	cout << "-NUMBER_de_CHASSIS : " << NUMBER_OF_CHASSIS << endl;
	cout << "-NUMBER_OF_SERVERS_IN_ONE_CHASSIS : " << NUMBER_OF_SERVERS_IN_ONE_CHASSIS << endl;
	cout << "-NUMBER_OF_CORES_IN_ONE_SERVER : " << NUMBER_OF_CORES_IN_ONE_SERVER << endl;
	cout << "-PERIODIC_LOG_INTERVAL : " << PERIODIC_LOG_INTERVAL << endl;
	cout << "-SKIP_x_SECONDS : " << SKIP_x_SECONDS << endl;
	cout << "-FINISHES_AT_DAY : " << FINISHES_AT_DAY << endl;

	cout << "-SCHEDULING_ALGORITHM : " << SCHEDULING_ALGORITHM << endl;
	cout << "-SUPPLY_TEMPERATURE_OFFSET_ALPHA : " << SUPPLY_TEMPERATURE_OFFSET_ALPHA << endl;
	cout << "-CRAC_SUPPLY_TEMP_IS_CONSTANT_AT : " << CRAC_SUPPLY_TEMP_IS_CONSTANT_AT << endl;
	cout << "-INSTANT_COOL_AIR_TRAVEL_TIME : " << INSTANT_COOL_AIR_TRAVEL_TIME << endl;
	cout << "-CONSTANT_FAN_POWER : " << CONSTANT_FAN_POWER << endl;
	cout << "-INSTANT_CHANGE_CRAC_SUPPLY_AIR : " << INSTANT_CHANGE_CRAC_SUPPLY_AIR << endl;
	cout << "-CRAC_DISCHARGE_CHANGE_RATE_0_00x : " << CRAC_DISCHARGE_CHANGE_RATE_0_00x << " (rate=" << (0.001*CRAC_DISCHARGE_CHANGE_RATE_0_00x) << ")" << endl;
	cout << "-FAN_RPM_NO_LIMIT : " << FAN_RPM_NO_LIMIT << endl;


	cout << "-PREDICITIONSHEDULER : " << PREDICITIONSHEDULER << endl;
	cout << "-PREDICTION_ALGORITHM : " << PREDICTION_ALGORITHM << endl;

	cout << "-TRANSFER_TIME_VM : " << TRANSFER_TIME_VMS << endl;
	cout << "-POWER_ON : " << POWER_ON << endl;

	cout << "-SIZEWINDOWNPREDICTION : " << SIZEWINDOWNPREDICTION << endl;
	cout << "-MONITORINGTIME : " << MONITORINGTIME << endl;


	cout << "-DIM : " << DIM  << endl;
	cout << "-DELAY : " << DELAY << endl;
	cout << "-STEP : " << STEP << endl;
	cout << "-CENTER : " << CENTER << endl;
	cout << "-COLUMN : " << STEP << endl;
	//cout << "-LENGTH : " << LENGTH << endl;
	cout << "-CLENGTH : " << CLENGTH << endl;
	//cout << "-MAKECAST : " << MAKECAST << endl;

	cout << "-N : " << N << endl;
	//cout << "-CAST : " << CAST << endl;


#ifdef _DEBUG
	cout << "-COOL_AIR_TRAVEL_TIME_MULTIPLIER : " << COOL_AIR_TRAVEL_TIME_MULTIPLIER << endl;
	cout << "-TEMPERATURE_SENSING_PERFORMANCE_CAPPING : " << TEMPERATURE_SENSING_PERFORMANCE_CAPPING << endl;
	cout << "-TEMPERATURE_SENSING_PERFORMANCE_CAPPING_AGGRESSIVENESS : " << TEMPERATURE_SENSING_PERFORMANCE_CAPPING_AGGRESSIVENESS << endl;
	cout << "-REASSIGN_VMS : " << REASSIGN_VMS << endl;
#endif

	cout << "-SWITCH ACCESS " << SWITCH_ACCESS << " SWITCH AGGREGATION " << SWITCH_AGGREGATION << " SWITCH_CORE " << SWITCH_CORE << endl;


	cout << endl;
	cout << "+ Total cores = " << (NUMBER_OF_CHASSIS*NUMBER_OF_CORES_IN_ONE_SERVER*NUMBER_OF_SERVERS_IN_ONE_CHASSIS) << endl;


	return true;
}

bool ReadMoreLines(ifstream* pIfs, vector<SWFLine*>* pvSWF)
{
	// read swf file into pJobQueueFile
	bool isReadingFinished = false;
	int numberOfLines = 0;
	
	vector<string> pJobQueueFile;
	string aLine;

READ_MORE_LINES:
	if (getline(*pIfs, aLine)) {
		vector<string> vOneLine;
		boost::split(vOneLine, aLine, boost::is_any_of(";"));
		if (vOneLine[0].size()==0) {
			goto READ_MORE_LINES; // skip comments
		} else {
			pJobQueueFile.push_back(aLine);
			numberOfLines++;
			if (numberOfLines < SWF_BUFFER_MIN_LINES)
				goto READ_MORE_LINES;
		}
	} else {
		isReadingFinished = true;
	}
	
	// parse pJobQueueFile into SWFLine (struct)
	for (unsigned int i=0; i<pJobQueueFile.size(); ++i) {
		vector<string> vOneLine;
		boost::split(vOneLine, pJobQueueFile[i], boost::is_any_of("\t "));
REMOVE_EMPTY_ELEMENT:
		for (unsigned int j=0; j<vOneLine.size(); ++j) {
			if(vOneLine[j].empty()) {
				vOneLine.erase(vOneLine.begin()+j);
				goto REMOVE_EMPTY_ELEMENT;
			}
		}
		if (vOneLine.size() == 0)
			continue; // could not read anything
		if (vOneLine.size() != 18) {
			cout << "Error: a SWF line has more than or less than 18 cols" << endl;
			continue;
		}

		// parsing SWF file
		SWFLine* pstALine = new SWFLine();
		memset(pstALine, 0x00, sizeof(SWFLine));
		pstALine->jobNumber	= boost::lexical_cast<UINT64>(vOneLine[0].c_str());
		//cout << "jobNumber " << pstALine->jobNumber << endl;
	
		pstALine->submitTimeSec = atoi(vOneLine[1].c_str());
		// pstALine->submitTimeSec = pstALine->submitTimeSec;
		pstALine->submitTimeSec = pstALine->submitTimeSec - 31526918;
		//cout << "SubmitTimeSec " << pstALine->submitTimeSec << endl;
		
		pstALine->waitTimeSec = atoi(vOneLine[2].c_str());
		//cout << "WaitTimeSec " << pstALine->waitTimeSec << endl;
		
		pstALine->runTimeSec = atoi(vOneLine[3].c_str());
		//cout << "RunTimeSec " << pstALine->runTimeSec << endl;

		pstALine->numCPUs = atoi(vOneLine[4].c_str());
		//cout << "NumCPUs " << pstALine->numCPUs << endl;
		
		pstALine->avgCPUTimeSec = atoi(vOneLine[5].c_str());
		//cout << "AvgCPUTimeSec " << pstALine->avgCPUTimeSec << endl;

		pstALine->usedMemKB = atol(vOneLine[6].c_str());
		//cout << "UsedMemKB  " <<pstALine->usedMemKB << endl;

		pstALine->numCPUsRequested = atoi(vOneLine[7].c_str());
		//cout << "numCPUsRequested  " <<pstALine->numCPUsRequested << endl;


		pstALine->runTimeSecRequested = atoi(vOneLine[8].c_str());
		// cout << "runTimeSecRequested  " <<pstALine->runTimeSecRequested << endl;

		pstALine->usedMemKBRequested = atoi(vOneLine[9].c_str());
		// cout << "usedMemKBRequested  " <<pstALine->usedMemKBRequested << endl;

		pstALine->status = atoi(vOneLine[10].c_str());
		// cout << "status   " <<pstALine->status  << endl;

		pstALine->userID = atoi(vOneLine[11].c_str());
		// cout << "userID   " <<pstALine->userID  << endl;

		pstALine->groupID = atoi(vOneLine[12].c_str());
		// cout << "groupID   " <<pstALine->groupID  << endl;
		
		pstALine->exefileID = atoi(vOneLine[13].c_str());
		// cout << "exefileID  " <<pstALine->exefileID  << endl;
		
		pstALine->queueNum = atoi(vOneLine[14].c_str());
		// cout << "queueNum   " <<pstALine->queueNum   << endl;
		
		pstALine->partitionNum = atoi(vOneLine[15].c_str());
		// cout << "partitionNum   " <<pstALine->partitionNum   << endl;

		if (boost::lexical_cast<long long>(vOneLine[16].c_str()) == -1)
			pstALine->dependencyJobNum = 0;
		else
			pstALine->dependencyJobNum = boost::lexical_cast<UINT64>(vOneLine[16].c_str());
		
		pstALine->thinkTimeAfterDependencySec = atoi(vOneLine[17].c_str());
		// cout << "thinkTimeAfterDependencySec   " <<pstALine->thinkTimeAfterDependencySec   << endl;

		// validate memory

		if (pstALine->usedMemKB < 0 || pstALine->usedMemKB == 0) {
			pstALine->usedMemKB = 1000000;
		}
		else
		{
			pstALine->usedMemKB = pstALine->usedMemKB + 1000000;
		}

		// validate
		if (pstALine->runTimeSec > 0 && pstALine->numCPUs > 0) {
			if (pstALine->avgCPUTimeSec == -1)
				pstALine->avgCPUTimeSec = pstALine->runTimeSec; // no info about cpu utilization. set 100%

			if (pstALine->avgCPUTimeSec == 0)
				pstALine->avgCPUTimeSec = 1; // min value is 1

			if (pstALine->avgCPUTimeSec <= 0) {
				cout << "Warning: avgCPUTimeSec <=0 for " << pstALine->jobNumber << " (skipped)" << endl;
				continue;
			}
			if (pstALine->avgCPUTimeSec > pstALine->runTimeSec)
				pstALine->avgCPUTimeSec = pstALine->runTimeSec;

			pstALine->submitTimeSec -= SKIP_x_SECONDS;
			if (pstALine->submitTimeSec < 0)
				continue;
			if (FINISHES_AT_DAY > 0 && pstALine->submitTimeSec > (FINISHES_AT_DAY*3600*24))
				continue;
			pvSWF->push_back(pstALine);
		}
     
	}
	pJobQueueFile.clear();

	return isReadingFinished;
}

int main(int argc, char* argv[])
{
	// arguments
	cout << "SimWare v2.1" << endl;

	for(int i = 0; i < argc; i++)
		cout << argv[i] << " ";
	cout << endl;
	
	if(ParsingArguments(argc, argv)==false)
		return 1;

#ifdef SINGLE_PRECISION
	cout << "+ Single Precision: only for debugging" << endl;
#else
	cout << "+ Double Precision" << endl;
#endif
#ifdef NO_DEP_CHECK
	cout << "+ Not checking job dependency" << endl;
#endif
	// current working directory
	char cCurrentPath[FILENAME_MAX];
	if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath))); // yes. intended.
	   cout << "+ Working Directory: " << cCurrentPath << endl;

	// Reading File
	bool isReadingFinished = false;
	ifstream ifs(argv[1]);
	if(!ifs.is_open()) {
		cout << "Cannot open a file :" << argv[1] << endl;
		return 1;
	}
	vector<SWFLine*> vSWF;
	isReadingFinished = ReadMoreLines(&ifs, &vSWF);
	if (isReadingFinished)
		ifs.clear();
		
	// Create & run
	cout << endl; 
	
	JobQueue myJobQueue;
	
	Users myUsers(&myJobQueue, &vSWF);
	
	ServersPOOL myPoolServers;

	Topology myTopology;
	
	DataCenter myDataCenter(&myJobQueue, &myPoolServers, &myTopology);

	int myClock = 0;
	int wallClock = 0;
	int lastTime = myUsers.LastJob();
	
	while (!(myJobQueue.IsFinished() && myDataCenter.IsFinished() && myUsers.IsFinished())) {
		myUsers.EveryASecond();
		myDataCenter.EveryASecond();
		myClock++;
		wallClock++;
		if (wallClock < lastTime) {
			printf("Simulating ---> %.2f %% ", ((double) wallClock / (double) lastTime) * 100);
			cout << " clock " << wallClock << " myJobQueue.IsFinished() " << myJobQueue.IsFinished() << " myDataCenter.IsFinished() " << myDataCenter.IsFinished() << " myUsers.IsFinished() " << myUsers.IsFinished() << endl;
		}
		else {
			if (wallClock == lastTime) {
				printf("Simulating ---> %.2f %% ", ((double) wallClock / (double) lastTime) * 100);
				cout << " clock " << wallClock << " myJobQueue.IsFinished() " << myJobQueue.IsFinished() << " myDataCenter.IsFinished() " << myDataCenter.IsFinished() << " myUsers.IsFinished() " << myUsers.IsFinished() << endl;
			}
			else {
				printf("Ending Simulation ");
				cout << " clock " << wallClock << " myJobQueue.IsFinished() " << myJobQueue.IsFinished() << " myDataCenter.IsFinished() " << myDataCenter.IsFinished() << " myUsers.IsFinished() " << myUsers.IsFinished() << endl;
			}
		} 
		fflush(stdout);
		if (myClock>(PERIODIC_LOG_INTERVAL<<2)) {
			myClock = 0;
			//cout << "."; // showing progress
			fflush(stdout);
		}
		if (isReadingFinished == false && vSWF.size() < SWF_BUFFER_MIN_LINES)
			isReadingFinished = ReadMoreLines(&ifs, &vSWF);
	}

	cout << "finished" << endl << endl;
	//myPoolServers.print();
	//myDataCenter.PrintVector();

	myDataCenter.PrintResults(&myPoolServers); // Temperature, Power draw
	myJobQueue.PrintResults(wallClock); // Latency

	return 0;
} 

/* Standard Workload Format (SWF)
#  0 - Job Number
#  1 - Submit Time
#  2 - Wait Time
#  3 - Run Time
#  4 - Number of Processors
#  5 - Average CPU Time Used
#  6 - Used Memory
#  7 - Requested Number of Processors
#  8 - Requested Time
#  9 - Requested Memory
# 10 - status (1=completed, 0=killed)
# 11 - User ID
# 12 - Group ID
# 13 - Executable (Application) Number
# 14 - Queue Number
# 15 - Partition Number
# 16 - Preceding Job Number
# 17 - Think Time from Preceding Job
*/
