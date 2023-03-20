#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Flight.h"
#include "fileHelper.h"

void	initFlight(Flight* pFlight, const AirportManager* pManager)
{
	Airport* pPortOr = setAiportToFlight(pManager, "Enter name of origin airport:");
	pFlight->nameSource = _strdup(pPortOr->name);
	int same;
	Airport* pPortDes;
	do {
		pPortDes = setAiportToFlight(pManager, "Enter name of destination airport:");
		same = isSameAirport(pPortOr, pPortDes);
		if (same)
			printf("Same origin and destination airport\n");
	} while (same);
	pFlight->nameDest = _strdup(pPortDes->name);
	initPlane(&pFlight->thePlane);
	getCorrectDate(&pFlight->date);
}

int		isFlightFromSourceName(const Flight* pFlight, const char* nameSource)
{
	if (strcmp(pFlight->nameSource, nameSource) == 0)
		return 1;
		
	return 0;
}


int		isFlightToDestName(const Flight* pFlight, const char* nameDest)
{
	if (strcmp(pFlight->nameDest, nameDest) == 0)
		return 1;

	return 0;


}

int		isPlaneCodeInFlight(const Flight* pFlight, const char*  code)
{
	if (strcmp(pFlight->thePlane.code, code) == 0)
		return 1;
	return 0;
}

int		isPlaneTypeInFlight(const Flight* pFlight, ePlaneType type)
{
	if (pFlight->thePlane.type == type)
		return 1;
	return 0;
}


void	printFlight(const Flight* pFlight)
{
	printf("Flight From %s To %s\t",pFlight->nameSource, pFlight->nameDest);
	printDate(&pFlight->date);
	printPlane(&pFlight->thePlane);
}

void	printFlightV(const void* val)
{
	const Flight* pFlight = *(const Flight**)val;
	printFlight(pFlight);
}


Airport* setAiportToFlight(const AirportManager* pManager, const char* msg)
{
	char name[MAX_STR_LEN];
	Airport* port;
	do
	{
		printf("%s\t", msg);
		myGets(name, MAX_STR_LEN,stdin);
		port = findAirportByName(pManager, name);
		if (port == NULL)
			printf("No airport with this name - try again\n");
	} while(port == NULL);

	return port;
}

void	freeFlight(Flight* pFlight)
{
	free(pFlight->nameSource);
	free(pFlight->nameDest);
	free(pFlight);
}


int saveFlightToFile(const Flight* pF, FILE* fp)
{
	if (!writeStringToFile(pF->nameSource, fp, "Error write flight source name\n"))
		return 0;

	if (!writeStringToFile(pF->nameDest, fp, "Error write flight destination name\n"))
		return 0;

	if (!savePlaneToFile(&pF->thePlane,fp))
		return 0;

	if (!saveDateToFile(&pF->date,fp))
		return 0;

	return 1;
}


int loadFlightFromFile(Flight* pF, const AirportManager* pManager, FILE* fp)
{

	pF->nameSource = readStringFromFile(fp, "Error reading source name\n");
	if (!pF->nameSource)
		return 0;

	pF->nameDest = readStringFromFile(fp, "Error reading destination name\n");
	if (!pF->nameDest)
		return 0;

	if (!loadPlaneFromFile(&pF->thePlane, fp))
		return 0;

	if (!loadDateFromFile(&pF->date, fp))
		return 0;

	return 1;
}

int	compareFlightBySourceName(const void* flight1, const void* flight2)
{
	const Flight* pFlight1 = *(const Flight**)flight1;
	const Flight* pFlight2 = *(const Flight**)flight2;
	return strcmp(pFlight1->nameSource, pFlight2->nameSource);
}

int	compareFlightByDestName(const void* flight1, const void* flight2)
{
	const Flight* pFlight1 = *(const Flight**)flight1;
	const Flight* pFlight2 = *(const Flight**)flight2;
	return strcmp(pFlight1->nameDest, pFlight2->nameDest);
}

int	compareFlightByPlaneCode(const void* flight1, const void* flight2)
{
	const Flight* pFlight1 = *(const Flight**)flight1;
	const Flight* pFlight2 = *(const Flight**)flight2;
	return strcmp(pFlight1->thePlane.code, pFlight2->thePlane.code);
}

int		compareFlightByDate(const void* flight1, const void* flight2)
{
	const Flight* pFlight1 = *(const Flight**)flight1;
	const Flight* pFlight2 = *(const Flight**)flight2;


	return compareDate(&pFlight1->date, &pFlight2->date);
	

	return 0;
}

int saveBinaryFileCompressedFlight(Flight* flight, FILE* f)
{
	BYTE data[2];
	BYTE data1[3];
	BYTE data2;
	int lenSorce = (int)strlen(flight->nameSource);
	int lenDestanation = (int)strlen(flight->nameDest);

	data[0] = lenSorce << 3 | lenDestanation >> 2;
	data[1] = lenDestanation << 6 | flight->thePlane.type << 4 | flight->date.month;

	if (!fwrite(&data1, sizeof(BYTE), 2, f))
		return 0;

	data1[0] = (flight->thePlane.code[0] - 'A') << 3 | (flight->thePlane.code[1] - 'A') >> 2;
	data1[1] = (flight->thePlane.code[1] - 'A') << 6 | (flight->thePlane.code[2] - 'A') << 1 | (flight->thePlane.code[3] - 'A') >> 4;
	data1[2] = (flight->thePlane.code[3] - 'A') << 4 | (flight->date.year - 2021);

	if (!fwrite(&data1, sizeof(BYTE), 3, f))
		return 0;
	data2 = flight->date.day;
	if (!fwrite(&data2, sizeof(BYTE), 1, f))
		return 0;
	if (!fwrite(flight->nameSource, sizeof(char), lenSorce, f))
		return 0;
	if (!fwrite(flight->nameDest, sizeof(char), lenDestanation, f))
		return 0;
	return 1;
}


int readBinaryFileCompressedFlight(Flight* flight, FILE* f)
{
	BYTE data[2];
	BYTE data1[3];
	BYTE data2;
	if (!fread(&data, sizeof(BYTE), 2, f))
		return 0;
	if (!fread(&data1, sizeof(BYTE), 3, f))
		return 0;
	if (!fread(&data2, sizeof(BYTE), 1, f))
		return 0;
	int lenSorce = data[0] >> 3 & 0x1F;
	int lenDestanation = (data1[0] & 0x7) << 2 | data[1] >> 6;
	flight->thePlane.type = data[1] >> 4 & 0x3;
	flight->date.month = data[1] & 0xF;
	flight->thePlane.code[0] = 'A' + (data1[0] >> 3);
	flight->thePlane.code[1] = 'A' + ((data1[0] & 0x7) << 2 | data1[1] >> 6);
	flight->thePlane.code[2] = 'A' + ((data1[1] >> 1) & 0x1F);
	flight->thePlane.code[3] = 'A' + ((data1[1] & 0x1) << 4 | data1[2] >> 4);
	flight->date.year = (data1[2] & 0xF) + 2021;
	flight->date.day = data2;

	flight->nameSource = (char*)calloc(lenSorce + 1, sizeof(char));
	if (!flight->nameSource)
		return 0;
	if (!fread(flight->nameSource, sizeof(char), lenSorce, f))
	{
		free(flight->nameSource);
		return 0;
	}
	flight->nameDest = (char*)calloc(lenDestanation + 1, sizeof(char));
	if (!flight->nameDest)
		return 0;
	if (!fread(flight->nameDest, sizeof(char), lenDestanation, f))
	{
		free(flight->nameDest);
		return 0;
	}
	return 1;
}



