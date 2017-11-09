// ConsoleApplication1.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

/*
#ifdef WIN32
#include <windows.h> // for Sleep
#else
#include <unistd.h>  // for usleep
#endif
*/
int main(int argc, char** argv)
{

	/* for 5DT */
	char    *szPort = NULL;
	char	szPortToOpen[6];
	fdGlove *pGlove = NULL;
	bool     showraw = true;
	int      glovetype = FD_GLOVENONE;
	int      i, j;

	/* for NI*/
	int32       error = 0;
	TaskHandle  taskHandle = 0;
	int32       read;
	float64     data[MAX_SAMPLE];
	char        errBuff[2048] = { '\0' };

	/* for save data*/
	FILE *sampled_data;
	errno_t err;

	if (argc<2)
	{
		printf("Usage: testglove <devicename> -r\n");
		printf("-r\tShow raw values instead of scaled\n");
	}
	else
	{
		// handle command line arguments
		for (i = 1; i<argc; i++)
		{
			if (!strcmp(argv[i], "-r"))
				showraw = true;
			else
				szPort = argv[i];
		}
	}

	if (!szPort)
	{
#ifdef WIN32
		szPort = "USB";
#else
		szPort = "/dev/fglove";
#endif
	}
	strcpy_s(szPortToOpen, szPort);
	if (strcmp(szPort, "USB") == 0)
	{
		unsigned short aPID[5];
		int nNumFound = 5;
		int nChosen = 0;
		fdScanUSB(aPID, nNumFound);
		if (nNumFound == 0) {
			fprintf_s(stderr, "No Gloves found\nPress Enter to quit");
			getchar();
			return -1;
		}
		else {
			for (int c = 0; c < nNumFound; c++)
			{
				printf_s("Available USB Gloves:\n");
				printf_s("%i - ", c);
				switch (aPID[c])
				{
				case DG14U_R:
					printf_s("Data Glove 14 Ultra Right\n");
					break;
				case DG14U_L:
					printf_s("Data Glove 14 Ultra Left\n");
					break;
				case DG5U_R:
					printf_s("Data Glove 5 Ultra Right\n");
					break;
				case DG5U_L:
					printf_s("Data Glove 5 Ultra Left\n");
					break;
				default:
					printf_s("Unknown\n");
				}
			}
			printf_s("Please enter USB No.:");
			scanf_s("%i", &nChosen);
			sprintf_s(szPortToOpen, "USB%i", nChosen);
			fdOpen(szPortToOpen);
		}
	}

	// Initialize glove
	printf_s("Attempting to open glove on %s .. ", szPortToOpen);
	if (NULL == (pGlove = fdOpen(szPortToOpen)))
	{
		printf_s("failed.\n");
		return -1;
	}
	printf_s("succeeded.\n");

	char *szType = "?";
	glovetype = fdGetGloveType(pGlove);
	switch (glovetype)
	{
	case FD_GLOVENONE: szType = "None"; break;
	case FD_GLOVE5U: szType = "DG5 Ultra serial"; break;
	case FD_GLOVE5UW: szType = "DG5 Ultra serial, wireless"; break;
	case FD_GLOVE5U_USB: szType = "DG5 Ultra USB"; break;
	case FD_GLOVE14U: szType = "DG14 Ultra serial"; break;
	case FD_GLOVE14UW: szType = "DG14 Ultra serial, wireless"; break;
	case FD_GLOVE14U_USB: szType = "DG14 Ultra USB"; break;
	}
	printf_s("glove type: %s\n", szType);
	printf_s("glove handedness: %s\n", fdGetGloveHand(pGlove) == FD_HAND_RIGHT ? "Right" : "Left");
	int iNumSensors = fdGetNumSensors(pGlove);
	printf_s("glove num sensors: %d\n", iNumSensors);
	// Display glove info
	unsigned char buf[64];

	printf_s("glove info: %s\n", (char*)buf);

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk(DAQmxCreateTask("", &taskHandle));
	DAQmxErrChk(DAQmxCreateAIVoltageChan(taskHandle, "Dev1/ai0", "", DAQmx_Val_Cfg_Default, -10.0, 10.0, DAQmx_Val_Volts, NULL));
	DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandle, "", 10000.0, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, MAX_SAMPLE));

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk(DAQmxStartTask(taskHandle));

	/****************************************************************/
	/****************************************************************/
	/****************************************************************/

	printf_s("Press any key to start measurement\n");
	getchar();

	/* Save file open*/
	err = fopen_s(&sampled_data, "sampled_data.csv", "w");
	if (err != 0) {
		fprintf_s(stderr, "Save file cannot be opened. Enter to quit.\n");
		getchar();
		return -1;
	}
	else {
		fprintf_s(sampled_data, "Time [ms], Thumb, Index, Middle, Ring, Little, force\n");
	}
	/* Glove sampling*/

	//	int raw_thumb, raw_index, raw_middle, raw_ring, raw_little;
	unsigned short raw_glove[5];
	//	double scaled_thumb, scaled_index, scaled_middle, scaled_ring, scaled_little;
	float scaled_glove[5];

	LARGE_INTEGER start, end, freq;
	if (!QueryPerformanceFrequency(&freq)) {
		fprintf_s(stderr, "Cannot execute QueryPerformanceFrequency\n");
		return -1;
	}
	if (!QueryPerformanceCounter(&start)) {
		return -1;
	}

	for (i = 0; i<MAX_SAMPLE; i++)
	{
		if (!QueryPerformanceCounter(&end))
			return -1;
		double elapsed_time = (double)(end.QuadPart - start.QuadPart) / freq.QuadPart * 1000;

		printf_s("%f\n", elapsed_time);
		fprintf_s(sampled_data, "%f, ", elapsed_time);
		if (glovetype == FD_GLOVE5U_USB)
		{
			if (showraw)
			{
	//			fdGetSensorRawAll(pGlove, raw_glove);
				
				raw_glove[0] = (int)fdGetSensorRaw(pGlove, FD_THUMBNEAR);
				raw_glove[1] = (int)fdGetSensorRaw(pGlove, FD_INDEXNEAR);
				raw_glove[2] = (int)fdGetSensorRaw(pGlove, FD_MIDDLENEAR);
				raw_glove[3] = (int)fdGetSensorRaw(pGlove, FD_RINGNEAR);
				raw_glove[4] = (int)fdGetSensorRaw(pGlove, FD_LITTLENEAR);
				
				/*
				printf_s("%4d ", (int)fdGetSensorRaw(pGlove, FD_THUMBNEAR));
				printf_s("%4d ", (int)fdGetSensorRaw(pGlove, FD_INDEXNEAR));
				printf_s("%4d ", (int)fdGetSensorRaw(pGlove, FD_MIDDLENEAR));
				printf_s("%4d ", (int)fdGetSensorRaw(pGlove, FD_RINGNEAR));
				printf_s("%4d ", (int)fdGetSensorRaw(pGlove, FD_LITTLENEAR));
				*/
				for (j = 0; j < 5; j++) {
					fprintf_s(sampled_data, "%d, ", raw_glove[j]);
				}
			}
			else // show scaled values
			{
//				fdGetSensorScaledAll(pGlove, scaled_glove);

				scaled_glove[0] = fdGetSensorScaled(pGlove, FD_THUMBNEAR);
				scaled_glove[1] = fdGetSensorScaled(pGlove, FD_INDEXNEAR);
				scaled_glove[2] = fdGetSensorScaled(pGlove, FD_MIDDLENEAR);
				scaled_glove[3] = fdGetSensorScaled(pGlove, FD_RINGNEAR);
				scaled_glove[4] = fdGetSensorScaled(pGlove, FD_LITTLENEAR);
/*		
				printf_s("%.1f ", fdGetSensorScaled(pGlove, FD_THUMBNEAR));
				printf_s("%.1f ", fdGetSensorScaled(pGlove, FD_INDEXNEAR));
				printf_s("%.1f ", fdGetSensorScaled(pGlove, FD_MIDDLENEAR));
				printf_s("%.1f ", fdGetSensorScaled(pGlove, FD_RINGNEAR));
				printf_s("%.1f ", fdGetSensorScaled(pGlove, FD_LITTLENEAR));
	*/			
				for (j = 0; j < 5; j++) {
					fprintf_s(sampled_data, "%f, ", scaled_glove[j]);
				}

			}
		}
		else {
			fprintf_s(stderr, "Not Glove 5U\n");
		}
		/*********************************************/
		// DAQmx Read Code
		/*********************************************/
		DAQmxErrChk(DAQmxReadAnalogF64(taskHandle, 1, 10.0, DAQmx_Val_GroupByChannel, data, 1, &read, NULL));
		fprintf_s(sampled_data, "%f, \n", data[0]);

#ifdef WIN32
		//		Sleep(15);
#else
		usleep(15000); // fixme
#endif
	}

	printf_s("Measured data saved.\n");
	// Close glove
	printf_s("closing glove\n");
	fdClose(pGlove);
	printf_s("glove closed, goodbye\n");

	printf_s("Acquired %d points\n", i);

	//	printf("Acquired %d points\n", (int)read);
	/*	fprintf_s(sampled_data, "NIDAQ\n");
	for (i = 0; i < MAX_SAMPLE; i++) {
	fprintf_s(sampled_data, "%d, %lf\n", i, data[i]);
	}
	*/
	err = fclose(sampled_data);
	if (err != 0) {
		fprintf_s(stderr, "Save file cannot be closed. Enter to quit.\n");
		getchar();
		return -1;
	}


Error:
	if (DAQmxFailed(error))
		DAQmxGetExtendedErrorInfo(errBuff, 2048);
	if (taskHandle != 0) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(taskHandle);
		DAQmxClearTask(taskHandle);
	}
	if (DAQmxFailed(error))
		printf_s("DAQmx Error: %s\n", errBuff);
	printf_s("End of program, press Enter key to quit\n");
	getchar();

	return 0;
}

