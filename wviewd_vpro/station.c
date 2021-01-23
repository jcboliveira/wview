/*---------------------------------------------------------------------------

  FILENAME:
		station.c

  PURPOSE:
		Provide the station abstraction utility.

  REVISION HISTORY:
		Date            Engineer        Revision        Remarks
		12/31/2005      M.S. Teel       0               Original

  NOTES:

  LICENSE:
		Copyright (c) 2005, Mark S. Teel (mark@teel.ws)

		This source code is released for free distribution under the terms
		of the GNU General Public License.

----------------------------------------------------------------------------*/

/*  ... System include files
*/

/*  ... Library include files
*/
#include <radprocutils.h>

/*  ... Local include files
*/
#include <station.h>

/*  ... global memory declarations
*/

static int newProcessEntryPoint(void* pargs)
{
	char*               binFile = (char*)pargs;

	return (system(binFile));
}


// send shutdown notification
int stationSendShutdown(WVIEWD_WORK* work)
{
	WVIEW_MSG_SHUTDOWN          msg;

	if (radMsgRouterMessageSend(WVIEW_MSG_TYPE_SHUTDOWN, &msg, sizeof(msg))
		== ERROR)
	{
		// can't send!
		MsgLog(PRI_HIGH, "radMsgRouterMessageSend failed: shutdown");
		return ERROR;
	}

	return OK;
}

int stationSendArchiveNotifications(WVIEWD_WORK* work, float sampleRain)
{
	WVIEW_MSG_ARCHIVE_NOTIFY    notify;
	int                         retVal;
	HISTORY_DATA                store;

	notify.dateTime = work->archiveDateTime;
	notify.intemp = (int)floorf(work->loopPkt.inTemp * 10);
	notify.inhumidity = work->loopPkt.inHumidity;
	notify.temp = (int)floorf(work->loopPkt.outTemp * 10);
	notify.humidity = work->loopPkt.outHumidity;
	notify.barom = (int)floorf(work->loopPkt.barometer * 1000);
	notify.stationPressure = (int)floorf(work->loopPkt.stationPressure * 1000);
	notify.altimeter = (int)floorf(work->loopPkt.altimeter * 1000);
	notify.winddir = work->loopPkt.windDir;
	notify.wspeedF = work->loopPkt.windSpeedF;
	notify.dewpoint = (int)floorf(work->loopPkt.dewpoint * 10);
	notify.hiwspeedF = work->loopPkt.windGustF;
	notify.rxPercent = work->loopPkt.rxCheckPercent;
	notify.sampleRain = sampleRain;
	notify.UV = work->loopPkt.UV;
	notify.radiation = work->loopPkt.radiation;

	// Grab last 60 minutes and last 24 hours from database:
	retVal = dbsqliteArchiveGetAverages(FALSE,
		work->archiveInterval,
		&store,
		time(NULL) - WV_SECONDS_IN_HOUR,
		WV_SECONDS_IN_HOUR / SECONDS_IN_INTERVAL(work->archiveInterval));
	if (retVal <= 0)
	{
		notify.rainHour = ARCHIVE_VALUE_NULL;
	}
	else
	{
		notify.rainHour = store.values[DATA_INDEX_rain];
	}

	retVal = dbsqliteArchiveGetAverages(FALSE,
		work->archiveInterval,
		&store,
		time(NULL) - WV_SECONDS_IN_DAY,
		WV_SECONDS_IN_DAY / SECONDS_IN_INTERVAL(work->archiveInterval));
	if (retVal <= 0)
	{
		notify.rainDay = ARCHIVE_VALUE_NULL;
	}
	else
	{
		notify.rainDay = store.values[DATA_INDEX_rain];
	}

	notify.rainToday = sensorGetCumulative(&work->sensors.sensor[STF_DAY][SENSOR_RAIN]);

	if (radMsgRouterMessageSend(WVIEW_MSG_TYPE_ARCHIVE_NOTIFY, &notify, sizeof(notify))
		== ERROR)
	{
		// can't send!
		MsgLog(PRI_HIGH, "radMsgRouterMessageSend failed: notify");
		return ERROR;
	}

	return OK;
}

int stationProcessInfoResponses(WVIEWD_WORK* work)
{
	WVIEW_MSG_STATION_INFO  apath;

	if (!work->archiveRqstPending)
	{
		return OK;
	}

	apath.lastArcTime = work->archiveDateTime;
	apath.archiveInterval = work->archiveInterval;
	apath.latitude = work->latitude;
	apath.longitude = work->longitude;
	apath.elevation = work->elevation;
	if (!work->showStationIF)
	{
		sprintf(apath.stationType, "%s", work->stationType);
	}
	else
	{
		if (work->medium.type == MEDIUM_TYPE_USBHID)
		{
			sprintf(apath.stationType, "%s (USB)", work->stationType);
		}
		else if (work->medium.type == MEDIUM_TYPE_NONE)
		{
			sprintf(apath.stationType, "%s", work->stationType);
		}
		else if (!strcmp(work->stationInterface, "ethernet"))
		{
			sprintf(apath.stationType, "%s (%s:%d)",
				work->stationType, work->stationHost, work->stationPort);
		}
		else
		{
			sprintf(apath.stationType, "%s (%s)",
				work->stationType, work->stationDevice);
		}
	}

	if (radMsgRouterMessageSend(WVIEW_MSG_TYPE_STATION_INFO, &apath, sizeof(apath))
		== ERROR)
	{
		MsgLog(PRI_HIGH, "radMsgRouterMessageSend failed Archive Path");
		return ERROR;
	}

	work->archiveRqstPending = FALSE;
	return OK;
}

void processRealTimeData(LOOP_PKT loopData, WV_SENSOR sensor[STF_MAX][SENSOR_MAX]);
void processRealTimeData(LOOP_PKT loopData, WV_SENSOR sensor[STF_MAX][SENSOR_MAX])
{
	FILE *ficheiro3;
	float high;
	float low;
	int windSpeedF=FALSE;
	int windGustF=FALSE;
	int tenMinuteWindGust=FALSE;

	ficheiro3 = fopen("/var/lib/wview/img/ramdisk/json/realtimeparameterlist.txt", "w");
	if (ficheiro3 != NULL) {


		if (loopData.outTemp != ARCHIVE_VALUE_NULL) {			
			fprintf(ficheiro3, "{\n");
			fprintf(ficheiro3, "\"outsideTemp\":%.1f", wvutilsConvertFToC(loopData.outTemp));
			high = wvutilsConvertFToC(sensorGetDailyHigh(sensor, SENSOR_OUTTEMP));
			low = wvutilsConvertFToC(sensorGetDailyLow(sensor, SENSOR_OUTTEMP));
			if (high < wvutilsConvertFToC(loopData.outTemp)) {
				high = wvutilsConvertFToC(loopData.outTemp);
				sensorUpdate(&sensor[STF_DAY][SENSOR_OUTTEMP], loopData.outTemp);
			}
			if (low > wvutilsConvertFToC(loopData.outTemp)) {
				low = wvutilsConvertFToC(loopData.outTemp);
				sensorUpdate(&sensor[STF_DAY][SENSOR_OUTTEMP], loopData.outTemp);
			}
			fprintf(ficheiro3, ",\n\"hiOutsideTemp\":%.1f", high);
			fprintf(ficheiro3, ",\n\"lowOutsideTemp\":%.1f", low);
		} else {
			fprintf(ficheiro3, "{\n\"dummy\":0");
		}
		

		if  (loopData.dewpoint != ARCHIVE_VALUE_NULL) {
			fprintf(ficheiro3, ",\n\"outsideDewPt\":%.1f", wvutilsConvertFToC(loopData.dewpoint));
			high = wvutilsConvertFToC(sensorGetDailyHigh(sensor, SENSOR_DEWPOINT));
			low = wvutilsConvertFToC(sensorGetDailyLow(sensor, SENSOR_DEWPOINT));
			if (high < wvutilsConvertFToC(loopData.dewpoint)) {
				high = wvutilsConvertFToC(loopData.dewpoint);
				sensorUpdate(&sensor[STF_DAY][SENSOR_DEWPOINT], loopData.dewpoint);
			}
			if (low > wvutilsConvertFToC(loopData.dewpoint)) {
				low = wvutilsConvertFToC(loopData.dewpoint);
				sensorUpdate(&sensor[STF_DAY][SENSOR_DEWPOINT], loopData.dewpoint);
			}
			fprintf(ficheiro3, ",\n\"hiDewpoint\":%.1f", high);
			fprintf(ficheiro3, ",\n\"lowDewpoint\":%.1f", low);
		}

		if  (loopData.extraTemp[0] != ARCHIVE_VALUE_NULL) {
			fprintf(ficheiro3, ",\n\"extraTemp1\":%.1f", wvutilsConvertFToC(loopData.extraTemp[0]));
			high = wvutilsConvertFToC(sensorGetDailyHigh(sensor, SENSOR_EXTRATEMP1));
			low = wvutilsConvertFToC(sensorGetDailyLow(sensor, SENSOR_EXTRATEMP1));
			if (high < wvutilsConvertFToC(loopData.extraTemp[0])) {
				high = wvutilsConvertFToC(loopData.extraTemp[0]);
				sensorUpdate(&sensor[STF_DAY][SENSOR_EXTRATEMP1], loopData.extraTemp[0]);
			}
			if (low > wvutilsConvertFToC(loopData.extraTemp[0])) {
				low = wvutilsConvertFToC(loopData.extraTemp[0]);
				sensorUpdate(&sensor[STF_DAY][SENSOR_EXTRATEMP1], loopData.extraTemp[0]);
			}
			fprintf(ficheiro3, ",\n\"hiOutsideATemp\":%.1f", high);
			fprintf(ficheiro3, ",\n\"lowOutsideATemp\":%.1f", low);
		}

		fprintf(ficheiro3, ",\n\"dailyRain\":%.1f", wvutilsConvertRainINToMetric(loopData.dayRain));
		fprintf(ficheiro3, ",\n\"rainRate\":%.1f", wvutilsConvertRainINToMetric(loopData.rainRate));
		high = wvutilsConvertRainINToMetric(sensorGetDailyHigh(sensor, SENSOR_RAINRATE));
		if (high < wvutilsConvertRainINToMetric(loopData.rainRate)) {
			high = wvutilsConvertRainINToMetric(loopData.rainRate);
			sensorUpdate(&sensor[STF_DAY][SENSOR_RAINRATE], loopData.rainRate);
		}
		fprintf(ficheiro3, ",\n\"hiRainRate\":%.1f", high);
		fprintf(ficheiro3, ",\n\"stormRain\":%.1f", wvutilsConvertRainINToMetric(loopData.stormRain));
		fprintf(ficheiro3, ",\n\"stormStart\":%d", (int32_t)loopData.stormStart);

		if  (loopData.outHumidity != 101) {
			fprintf(ficheiro3, ",\n\"outsideHumidity\":%d", (uint16_t)loopData.outHumidity);
			high = sensorGetDailyHigh(sensor, SENSOR_OUTHUMID);
			low = sensorGetDailyLow(sensor, SENSOR_OUTHUMID);
			if (high < (float)loopData.outHumidity) {
				high = (float)loopData.outHumidity;
				sensorUpdate(&sensor[STF_DAY][SENSOR_OUTHUMID], loopData.outHumidity);
			}
			if (low > (float)loopData.outHumidity) {
				low = (float)loopData.outHumidity;
				sensorUpdate(&sensor[STF_DAY][SENSOR_OUTHUMID], loopData.outHumidity);
			}
			fprintf(ficheiro3, ",\n\"hiHumidity\":%d", (uint16_t)high);
			fprintf(ficheiro3, ",\n\"lowHumidity\":%d", (uint16_t)low);
		}

		if  (loopData.barometer != ARCHIVE_VALUE_NULL) {
			fprintf(ficheiro3, ",\n\"barometer\":%.1f", wvutilsConvertINHGToHPA(loopData.barometer));
	
			high = wvutilsConvertINHGToHPA(sensorGetDailyHigh(sensor, SENSOR_BP));
			low = wvutilsConvertINHGToHPA(sensorGetDailyLow(sensor, SENSOR_BP));
			if (high < wvutilsConvertINHGToHPA(loopData.barometer)) {
				high = wvutilsConvertINHGToHPA(loopData.barometer);
				sensorUpdate(&sensor[STF_DAY][SENSOR_BP], loopData.barometer);
			}
			if (low > wvutilsConvertINHGToHPA(loopData.barometer)) {
				low = wvutilsConvertINHGToHPA(loopData.barometer);
				sensorUpdate(&sensor[STF_DAY][SENSOR_BP], loopData.barometer);
			}
			
			fprintf(ficheiro3, ",\n\"hiBarometer\":%.1f", high);
			fprintf(ficheiro3, ",\n\"lowBarometer\":%.1f", low);
		}

		if  (loopData.windSpeedF != ARCHIVE_VALUE_NULL) {
			fprintf(ficheiro3, ",\n\"windSpeed\":%.1f", wvutilsConvertMPHToKPH(loopData.windSpeedF));
			windSpeedF=TRUE;
		}

		if (loopData.windGustF != ARCHIVE_VALUE_NULL) {
			fprintf(ficheiro3, ",\n\"windGustSpeed\":%.1f", wvutilsConvertMPHToKPH(loopData.windGustF));
			windGustF=TRUE;
		}

		if  (loopData.tenMinuteWindGust != ARCHIVE_VALUE_NULL) {
			fprintf(ficheiro3, ",\n\"tenMinuteWindGust\":%.1f", wvutilsConvertMPHToKPH(loopData.tenMinuteWindGust));
			tenMinuteWindGust=TRUE;
		}

		
		high = wvutilsConvertMPHToKPH(sensorGetDailyHigh(sensor, SENSOR_WGUST));
		if (tenMinuteWindGust==TRUE&&wvutilsConvertMPHToKPH(loopData.tenMinuteWindGust)>high) {
			high = wvutilsConvertMPHToKPH(loopData.tenMinuteWindGust);
			sensorUpdateWhen(&sensor[STF_DAY][SENSOR_WGUST],loopData.tenMinuteWindGust,(float)loopData.WinddirtenMinuteWindGust);
		}

		if (windGustF==TRUE&&wvutilsConvertMPHToKPH(loopData.windGustF)>high) {
			high = wvutilsConvertMPHToKPH(loopData.windGustF);
			sensorUpdateWhen(&sensor[STF_DAY][SENSOR_WGUST],loopData.windGustF,(float)loopData.windGustDir);
		}
		if (windSpeedF==TRUE&&wvutilsConvertMPHToKPH(loopData.windSpeedF)>high) {
			high = wvutilsConvertMPHToKPH(loopData.windSpeedF);
			sensorUpdateWhen(&sensor[STF_DAY][SENSOR_WGUST],loopData.windSpeedF,(float)loopData.windDir);
		}
		if (high<200)
			fprintf(ficheiro3, ",\n\"hiWindSpeed\":%.1f", high);
		
		
		fprintf(ficheiro3, ",\n\"windDirectionDegrees\":%d", (uint16_t)loopData.windDir);
		fprintf(ficheiro3, ",\n\"windGustDirectionDegrees\":%d", (uint16_t)loopData.windGustDir);
		if  (loopData.tenMinuteWindGust != ARCHIVE_VALUE_NULL) {
			fprintf(ficheiro3, ",\n\"WinddirtenMinuteWindGust\":%d", (uint16_t)loopData.WinddirtenMinuteWindGust);
		}
		fprintf(ficheiro3, ",\n\"ET\":%.1f", wvutilsConvertRainINToMetric(loopData.dayET));

		if  (loopData.UV != ARCHIVE_VALUE_NULL) {
			fprintf(ficheiro3, ",\n\"UV\":%.1f", loopData.UV);

			high = sensorGetDailyHigh(sensor, SENSOR_UV);
			if (high < loopData.UV) {
				high = loopData.UV;
				sensorUpdate(&sensor[STF_DAY][SENSOR_UV], loopData.UV);
			}
			fprintf(ficheiro3, ",\n\"hiUV\":%.1f", high);
		}

		if  (loopData.radiation != 4000) {
			fprintf(ficheiro3, ",\n\"solarRad\":%d", (uint16_t)loopData.radiation);
		
			high = sensorGetDailyHigh(sensor, SENSOR_SOLRAD);
			if (high < loopData.radiation) {
				high = loopData.radiation;
				sensorUpdate(&sensor[STF_DAY][SENSOR_SOLRAD], loopData.radiation);
			}
			fprintf(ficheiro3, ",\n\"hiRadiation\":%.1f", high);
		}

		if  (loopData.tenMinuteAvgWindSpeed != ARCHIVE_VALUE_NULL) 
			fprintf(ficheiro3, ",\n\"tenMinuteAvgWindSpeed\":%.1f", wvutilsConvertMPHToKPH(loopData.tenMinuteAvgWindSpeed));
		
		if  (loopData.twoMinuteAvgWindSpeed != ARCHIVE_VALUE_NULL) 
			fprintf(ficheiro3, ",\n\"twoMinuteAvgWindSpeed\":%.1f", wvutilsConvertMPHToKPH(loopData.twoMinuteAvgWindSpeed));

		fprintf(ficheiro3, "\n}");

		fclose(ficheiro3);
	}
}

WV_SENSOR           sensor[STF_MAX][SENSOR_MAX];

int stationProcessIPM(WVIEWD_WORK* work, char* srcQueueName, int msgType, void* msg)
{
	WVIEW_MSG_REQUEST*           msgRqst;
	WVIEW_MSG_LOOP_DATA         loop;
	WVIEW_MSG_HILOW_DATA        hilow;
	WVIEW_MSG_ALERT*            alert;
	int                         retVal, i;

	switch (msgType)
	{
	case WVIEW_MSG_TYPE_REQUEST:
		msgRqst = (WVIEW_MSG_REQUEST*)msg;

		switch (msgRqst->requestType)
		{
		case WVIEW_RQST_TYPE_STATION_INFO:
			// flag that a request has been tendered
			work->archiveRqstPending = TRUE;

			// are we currently in a serial cycle?
			if (!work->runningFlag)
			{
				// yes, just bail out for now
				return OK;
			}
			else
			{
				// no, send it now
				return (stationProcessInfoResponses(work));
			}

		case WVIEW_RQST_TYPE_LOOP_DATA:
			loop.loopData = work->loopPkt;
			if (loop.loopData.sampleET == ARCHIVE_VALUE_NULL)
				loop.loopData.sampleET = 0;
			if (loop.loopData.radiation == 0xFFFF)
				loop.loopData.radiation = 0;
			if (loop.loopData.UV < 0)
				loop.loopData.UV = 0;
			if (loop.loopData.rxCheckPercent == 0xFFFF)
				loop.loopData.rxCheckPercent = 0;

			if (radMsgRouterMessageSend(WVIEW_MSG_TYPE_LOOP_DATA,
				&loop,
				sizeof(loop))
				== ERROR)
			{
				MsgLog(PRI_HIGH, "radMsgRouterMessageSend failed LOOP");
				return ERROR;
			}

			return OK;

		case WVIEW_RQST_TYPE_HILOW_DATA:

			hilow.hilowData = work->sensors;

			if (radMsgRouterMessageSend(WVIEW_MSG_TYPE_HILOW_DATA,
				&hilow,
				sizeof(hilow))
				== ERROR)
			{
				MsgLog(PRI_HIGH, "radMsgRouterMessageSend failed HILOW %d", sizeof(WVIEW_MSG_HILOW_DATA));
				return ERROR;
			}

			return OK;
		}
		break;

	case WVIEW_MSG_TYPE_ALERT:
		alert = (WVIEW_MSG_ALERT*)msg;
		break;

	default:
		// Pass it through to the station-specific function:
		stationMessageIndicate(work, msgType, msg);
		break;
	}

	return OK;
}

int stationPushDataToClients(WVIEWD_WORK* work)
{
	WVIEW_MSG_LOOP_DATA loop;

	if (!work->runningFlag)
	{
		return OK;
	}

	memcpy(&loop.loopData, &work->loopPkt, sizeof(loop.loopData));

	if (radMsgRouterMessageSend(WVIEW_MSG_TYPE_LOOP_DATA_SVC, &loop, sizeof(loop))
		== ERROR)
	{
		MsgLog(PRI_HIGH, "radMsgRouterMessageSend failed for loop transmit!");
		return ERROR;
	}

	radProcessTimerStart(work->pushTimer, work->pushInterval);
	return OK;
}

int stationPushArchiveToClients(WVIEWD_WORK* work, ARCHIVE_PKT* pktToSend)
{
	WVIEW_MSG_ARCHIVE_DATA  arc;

	if (!work->runningFlag)
	{
		return OK;
	}

	memcpy(&arc.archiveData, pktToSend, sizeof(*pktToSend));

	if (radMsgRouterMessageSend(WVIEW_MSG_TYPE_ARCHIVE_DATA, &arc, sizeof(arc))
		== ERROR)
	{
		MsgLog(PRI_HIGH, "radMsgRouterMessageSend failed for archive transmit!");
		return ERROR;
	}

	return OK;
}

void stationStartArchiveTimerUniform(WVIEWD_WORK* work)
{
	time_t              ntime;
	struct tm           currTime;
	int                 intSECS;

	// get the current time
	ntime = time(NULL);
	localtime_r(&ntime, &currTime);

	// get the next archive interval delta in seconds
	intSECS = currTime.tm_min / work->archiveInterval;
	intSECS *= work->archiveInterval;
	intSECS += work->archiveInterval;
	intSECS -= currTime.tm_min;                     // delta in minutes
	intSECS *= 60;

	// we try to land 4 seconds after top-of-minute
	intSECS += (4 - currTime.tm_sec);

	// now we have the delta in seconds
	work->nextArchiveTime = ntime + intSECS;
	radTimerStart(work->archiveTimer, intSECS * 1000);
}

void stationStartCDataTimerUniform(WVIEWD_WORK* work)
{
	time_t              ntime;
	struct tm           locTime;
	int                 moduloVal, cdataintSECS;

	ntime = time(NULL);
	localtime_r(&ntime, &locTime);
	cdataintSECS = work->cdataInterval / 1000;

	// try to hit the start of next cdataInterval
	moduloVal = locTime.tm_sec % cdataintSECS;
	moduloVal = cdataintSECS - moduloVal;

	radTimerStart(work->cdataTimer, moduloVal * 1000);
}

int stationStartSyncTimerUniform(WVIEWD_WORK* work, int firstTime)
{
	time_t              ntime;
	struct tm           locTime;
	int                 tempVal;

	ntime = time(NULL);
	localtime_r(&ntime, &locTime);
	locTime.tm_sec %= 60;

	if (firstTime)
	{
		// Make sure the sync timer does not track with the archive interval
		// by doing the sync at 2:30 of any 5 minute interval:
		tempVal = locTime.tm_min % 5;
		tempVal = 2 - tempVal;
		if (tempVal < 0)
		{
			tempVal += 5;
		}
		tempVal *= 60;                      // Make it seconds

		if (locTime.tm_sec <= 30)
		{
			tempVal += 30 - locTime.tm_sec;
		}
		else
		{
			if (tempVal == 0)
			{
				tempVal = 5 * 60;           // Add 5 minutes
			}
			tempVal -= (locTime.tm_sec - 30);
		}
		if (tempVal == 0)
		{
			tempVal = 1;
		}

		radTimerStart(work->syncTimer, tempVal * 1000);
		return FALSE;
	}

	if (locTime.tm_sec < 10)
	{
		// Correct:
		radTimerStart(work->syncTimer, ((30 - locTime.tm_sec) * 1000));
	}
	else if (locTime.tm_sec > 45)
	{
		radTimerStart(work->syncTimer, ((90 - locTime.tm_sec) * 1000));
	}
	else
	{
		// try to hit 30 secs into the minute
		tempVal = 30 - locTime.tm_sec;
		radTimerStart(work->syncTimer, WVD_TIME_SYNC_INTERVAL + (tempVal * 1000));
		return TRUE;
	}

	return FALSE;
}

int stationVerifyArchiveInterval(WVIEWD_WORK* work)
{
	ARCHIVE_PKT     tempRec;

	// sanity check the archive interval against the most recent record
	memset(&tempRec, 0, sizeof(tempRec));
	if ((int)dbsqliteArchiveGetNewestTime(&tempRec) == ERROR)
	{
		// there must be no archive records - just return OK
		return OK;
	}

	if ((int)tempRec.interval != (int)work->archiveInterval)
	{
		MsgLog(PRI_HIGH,
			"verifyArchiveInterval: station value of %d does NOT match archive value of %d",
			work->archiveInterval, (int)tempRec.interval);
		return ERROR;
	}
	else
	{
		return OK;
	}
}

int stationGetConfigValueInt(WVIEWD_WORK* work, char* configName, int* store)
{
	int             value;

	wvconfigInit(FALSE);
	value = wvconfigGetINTValue(configName);
	wvconfigExit();
	*store = value;
	return OK;
}

int stationGetConfigValueBoolean(WVIEWD_WORK* work, char* configName, int* store)
{
	int             value;

	wvconfigInit(FALSE);
	value = wvconfigGetBooleanValue(configName);
	wvconfigExit();
	*store = value;
	return OK;
}

int stationGetConfigValueFloat(WVIEWD_WORK* work, char* configName, float* store)
{
	float           tempfloat;

	wvconfigInit(FALSE);
	tempfloat = wvconfigGetDOUBLEValue(configName);
	wvconfigExit();

	tempfloat *= 100;
	if (tempfloat < 0.0)
		tempfloat -= 0.5;
	else
		tempfloat += 0.5;
	tempfloat /= 100;
	*store = tempfloat;
	return OK;
}

void stationClearLoopData(WVIEWD_WORK* work)
{
	work->loopPkt.rxCheckPercent = 0xFFFF;
	work->loopPkt.outTemp = ARCHIVE_VALUE_NULL;
	work->loopPkt.inTemp = ARCHIVE_VALUE_NULL;
	work->loopPkt.inHumidity = 101;
	work->loopPkt.outHumidity = 101;
	work->loopPkt.barometer = ARCHIVE_VALUE_NULL;
	work->loopPkt.windSpeedF = ARCHIVE_VALUE_NULL;
	work->loopPkt.dewpoint = ARCHIVE_VALUE_NULL;
	work->loopPkt.windchill = ARCHIVE_VALUE_NULL;
	work->loopPkt.heatindex = ARCHIVE_VALUE_NULL;
	work->loopPkt.sampleET = ARCHIVE_VALUE_NULL;
	work->loopPkt.UV = ARCHIVE_VALUE_NULL;
	work->loopPkt.radiation = 4000;
	work->loopPkt.extraTemp[0] = ARCHIVE_VALUE_NULL;
	work->loopPkt.extraTemp[1] = ARCHIVE_VALUE_NULL;
	work->loopPkt.extraTemp[2] = ARCHIVE_VALUE_NULL;
	work->loopPkt.soilTemp1 = ARCHIVE_VALUE_NULL;
	work->loopPkt.soilMoist1 = 254;
	work->loopPkt.leafWet1 = 254;
	work->loopPkt.tenMinuteAvgWindSpeed = ARCHIVE_VALUE_NULL;
	work->loopPkt.tenMinuteWindGust = ARCHIVE_VALUE_NULL;
	work->loopPkt.twoMinuteAvgWindSpeed = ARCHIVE_VALUE_NULL;


	return;
}
