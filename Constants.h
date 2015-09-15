#pragma once

#define SIZE_OF_HEAT_TIMING_BUFFER 1024 // = 2^10
#define SIZE_OF_HR_MATRIX 50
#define NUMBER_OF_SERVERS_IN_ONE_HR_MATRIX_CELL_MAX 60

#define LOWEST_SUPPLY_TEMPERATURE 10.0
#define EMERGENCY_TEMPERATURE 29.999

#define CHASSIxSERVER 500
#define CHASSIS 50
#define SERVERS 10

//#define SWF_BUFFER_MIN_LINES 120000
#define SWF_BUFFER_MIN_LINES 2000000 //número máximo de linhas a serem lidos
#define MAX_NUMBER_OF_ELEMENTS_LOG_INTERVAL 2048  // need to be bigger than (max sim sec)/PERIODIC_LOG_INTERVAL (too bad but dynamic allocation was too slow...)

#define UINT64 unsigned long long
#define FLOATINGPOINT double

typedef struct sortserver {
	     int chassi;
		 int server;
		 double temperature;
		 double temperatureFuture;
		 FLOATINGPOINT utilizationCPU;
		 double ranking;
		 bool assignVM;
  } SORTSERVER;

typedef struct pool {
	     int chassi;
		 int server;
		 double temperature;
  } POOL;

typedef struct structemp {
	     int chassi;
		 int server;
		 FLOATINGPOINT temperature;
		 FLOATINGPOINT utilizationCPU;
  } STRUCTEMP;

