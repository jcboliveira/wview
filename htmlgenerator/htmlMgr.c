/*---------------------------------------------------------------------------

  FILENAME:
		htmlMgr.c

  PURPOSE:
		Provide the wview html generator utilities.

  REVISION HISTORY:
		Date            Engineer        Revision        Remarks
		08/30/03        M.S. Teel       0               Original

  NOTES:

  LICENSE:
		Copyright (c) 2004, Mark S. Teel (mark@teel.ws)

		This source code is released for free distribution under the terms
		of the GNU General Public License.

----------------------------------------------------------------------------*/

/*  ... System include files
*/
#include <termios.h>

/*  ... Library include files
*/
#include <radtimeUtils.h>

/*  ... Local include files
*/
#include <services.h>
#include <dbsqlite.h>
#include <html.h>
#include <htmlMgr.h>

/*  ... global memory declarations
*/
char sampleLabels[MAX_DAILY_NUM_VALUES][8];
char sampleHourLabels[MAX_DAILY_NUM_VALUES][8];

/*  ... global memory referenced
*/

/*  ... static (local) memory declarations
*/
#define DEBUG_GENERATION            FALSE

static HTML_MGR     mgrWork;
static uint64_t     GenerateTime;

//  ... local utilities

static int readHtmlTemplateFile(HTML_MGR* mgr, char* filename)
{
	HTML_TMPL*       html;
	FILE*            file;
	char*            token;
	char            temp[HTML_MAX_LINE_LENGTH];

	file = fopen(filename, "r");
	if (file == NULL)
	{
		MsgLog(PRI_HIGH, "htmlmgrInit: %s does not exist!",
			filename);
		return ERROR_ABORT;
	}

	while (fgets(temp, HTML_MAX_LINE_LENGTH, file) != NULL)
	{
		if (temp[0] == ' ' || temp[0] == '\n' || temp[0] == '#')
		{
			// comment or whitespace
			continue;
		}

		html = (HTML_TMPL*)malloc(sizeof(*html));
		if (html == NULL)
		{
			for (html = (HTML_TMPL*)radListRemoveFirst(&mgr->templateList);
				html != NULL;
				html = (HTML_TMPL*)radListRemoveFirst(&mgr->templateList))
			{
				free(html);
			}

			fclose(file);
			return ERROR;
		}

		// do the template file name
		token = strtok(temp, " \t\n");
		if (token == NULL)
		{
			free(html);
			continue;
		}
		wvstrncpy(html->fname, token, sizeof(html->fname));

		radListAddToEnd(&mgr->templateList, (NODE_PTR)html);
	}

	fclose(file);
	return OK;
}

static void cleanupForecastRules(HTML_MGR* mgr)
{
	int             i;

	for (i = 0; i <= HTML_MAX_FCAST_RULE; i++)
	{
		if (mgr->ForecastRuleText[i] != NULL)
		{
			free(mgr->ForecastRuleText[i]);
			mgr->ForecastRuleText[i] = NULL;
		}
	}

	for (i = 1; i <= VP_FCAST_ICON_MAX; i++)
	{
		if (mgr->ForecastIconFile[i] != NULL)
		{
			free(mgr->ForecastIconFile[i]);
			mgr->ForecastIconFile[i] = NULL;
		}
	}

	return;
}

static int readForecastRuleConfigFile(HTML_MGR* mgr, char* filename)
{
	FILE*            file;
	char*            token;
	char            temp[HTML_MAX_LINE_LENGTH];
	int             iconNo = 1, ruleNo = 0;

	file = fopen(filename, "r");
	if (file == NULL)
	{
		return ERROR_ABORT;
	}

	//  read each line starting with "ICON", assigning to the corresponding
	//  forecast icon
	while (fgets(temp, HTML_MAX_LINE_LENGTH - 1, file) != NULL)
	{
		// does the line begin with "ICON"?
		if (strncmp(temp, "ICON", 4))
		{
			// nope
			continue;
		}

		// get the memory to store the string
		mgr->ForecastIconFile[iconNo] = (char*)malloc(FORECAST_ICON_FN_MAX);
		if (mgr->ForecastIconFile[iconNo] == NULL)
		{
			MsgLog(PRI_HIGH,
				"readForecastRuleConfigFile: cannot allocate memory for forecast icon #%d!",
				iconNo);
			cleanupForecastRules(mgr);
			fclose(file);
			return ERROR;
		}

		// skip the ICON keyword
		token = strtok(temp, " \t");
		if (token == NULL)
		{
			free(mgr->ForecastIconFile[iconNo]);
			mgr->ForecastIconFile[iconNo] = NULL;
			continue;
		}
		token = strtok(NULL, " \t\n");
		if (token == NULL)
		{
			free(mgr->ForecastIconFile[iconNo]);
			mgr->ForecastIconFile[iconNo] = NULL;
			continue;
		}

		wvstrncpy(mgr->ForecastIconFile[iconNo],
			token,
			FORECAST_ICON_FN_MAX);
		if (mgr->ForecastIconFile[iconNo][strlen(mgr->ForecastIconFile[iconNo]) - 1] == '\n')
			mgr->ForecastIconFile[iconNo][strlen(mgr->ForecastIconFile[iconNo]) - 1] = 0;

		if (++iconNo == (VP_FCAST_ICON_MAX + 1))
		{
			// we're done!
			break;
		}
	}

	// reset to the beginning of the file
	fseek(file, 0, SEEK_SET);

	// throw away all lines until we hit the keyword "RULES"
	while (fgets(temp, HTML_MAX_LINE_LENGTH - 1, file) != NULL)
	{
		if (!strncmp(temp, "<RULES>", 7))
		{
			// begin the rule dance
			break;
		}
	}

	//  read each line, assigning to the corresponding forecast rule
	while (fgets(temp, HTML_MAX_LINE_LENGTH - 1, file) != NULL)
	{
		// get the memory to store the string
		mgr->ForecastRuleText[ruleNo] = (char*)malloc(strlen(temp) + 1);
		if (mgr->ForecastRuleText[ruleNo] == NULL)
		{
			MsgLog(PRI_HIGH, "readForecastRuleConfigFile: cannot allocate memory for forecast rule #%d!",
				ruleNo);
			cleanupForecastRules(mgr);
			fclose(file);
			return ERROR;
		}

		// copy the string (we know temp is bounded and we allocated for its length):
		strcpy(mgr->ForecastRuleText[ruleNo], temp);
		if (mgr->ForecastRuleText[ruleNo][strlen(temp) - 1] == '\n')
			mgr->ForecastRuleText[ruleNo][strlen(temp) - 1] = 0;

		if (++ruleNo == (HTML_MAX_FCAST_RULE + 1))
		{
			// we're done!
			break;
		}
	}

	// did we find them all?
	if (ruleNo != (HTML_MAX_FCAST_RULE + 1))
	{
		// nope, let's fuss about it
		MsgLog(PRI_MEDIUM, "readForecastRuleConfigFile: NOT all forecast rules are assigned in %s: "
			"the last %d are missing and will be empty ...",
			filename,
			(HTML_MAX_FCAST_RULE + 1) - ruleNo);
	}
	if (iconNo != (VP_FCAST_ICON_MAX + 1))
	{
		// nope, let's fuss about it
		MsgLog(PRI_MEDIUM, "readForecastRuleConfigFile: NOT all forecast icons are assigned in %s: "
			"the last %d are missing and will be empty ...",
			filename,
			(VP_FCAST_ICON_MAX + 1) - iconNo);
	}

	MsgLog(PRI_STATUS, "%d icon definitions, %d forecast rules found ...",
		iconNo - 1,
		ruleNo);

	fclose(file);
	return OK;
}

static void emptyWorkLists
(
	HTML_MGR_ID     id
)
{
	NODE_PTR        nptr;

	for (nptr = radListRemoveFirst(&id->templateList);
		nptr != NULL;
		nptr = radListRemoveFirst(&id->templateList))
	{
		free(nptr);
	}

	return;
}

//  ... API methods

HTML_MGR_ID htmlmgrInit
(
	char*            installPath,
	int             isMetricUnits,
	char*            imagePath,
	char*            htmlPath,
	int             arcInterval,
	int             isExtendedData,
	char*            name,
	char*            city,
	char*            state,
	int16_t         elevation,
	int16_t         latitude,
	int16_t         longitude,
	char*            mphaseIncrease,
	char*            mphaseDecrease,
	char*            mphaseFull,
	char*            radarURL,
	char*            forecastURL,
	char*            dateFormat,
	int             isDualUnits
)
{
	HTML_MGR_ID     newId;
	int             i, numNodes, numImages, numTemplates;
	char            confFilePath[256];

	newId = &mgrWork;
	memset(newId, 0, sizeof(mgrWork));

	radListReset(&newId->templateList);

	newId->isMetricUnits = isMetricUnits;
	newId->archiveInterval = arcInterval;
	newId->isExtendedData = isExtendedData;
	newId->imagePath = imagePath;
	newId->htmlPath = htmlPath;
	newId->stationElevation = elevation;
	newId->stationLatitude = latitude;
	newId->stationLongitude = longitude;
	wvstrncpy(newId->stationName, name, sizeof(newId->stationName));
	wvstrncpy(newId->stationCity, city, sizeof(newId->stationCity));
	wvstrncpy(newId->stationState, state, sizeof(newId->stationState));
	wvstrncpy(newId->mphaseIncrease, mphaseIncrease, sizeof(newId->mphaseIncrease));
	wvstrncpy(newId->mphaseDecrease, mphaseDecrease, sizeof(newId->mphaseDecrease));
	wvstrncpy(newId->mphaseFull, mphaseFull, sizeof(newId->mphaseFull));
	wvstrncpy(newId->radarURL, radarURL, sizeof(newId->radarURL));
	wvstrncpy(newId->forecastURL, forecastURL, sizeof(newId->forecastURL));
	wvstrncpy(newId->dateFormat, dateFormat, sizeof(newId->dateFormat));
	newId->isDualUnits = isDualUnits;

	//  ... initialize the newArchiveMask
	newId->newArchiveMask = NEW_ARCHIVE_ALL;

	//  ... now initialize our html template list
	sprintf(confFilePath, "%s/html-templates.conf", installPath);
	if (readHtmlTemplateFile(newId, confFilePath) != OK)
	{
		return NULL;
	}
	else
	{
		numTemplates = radListGetNumberOfNodes(&newId->templateList);
		MsgLog(PRI_STATUS, "htmlmgrInit: %d templates added", numTemplates);
	}

	//  ... now initialize our forecast rule text list
	sprintf(confFilePath, "%s/forecast.conf", installPath);
	if (readForecastRuleConfigFile(newId, confFilePath) != OK)
	{
		MsgLog(PRI_STATUS, "htmlmgrInit: forecast html tags are disabled - %s not found...",
			confFilePath);
	}

	//  ... initialize the sample label array
	htmlmgrSetSampleLabels(newId);

	statusUpdateStat(HTML_STATS_TEMPLATES_DEFINED, numTemplates);

	return newId;
}

void htmlmgrExit
(
	HTML_MGR_ID     id
)
{
	cleanupForecastRules(id);

	emptyWorkLists(id);

	return;
}

void htmlmgrSetSampleLabels(HTML_MGR_ID id)
{
	int         i;
	time_t      startTime, ntime = time(NULL);
	struct tm   tmtime;
	int         daySamples;

	// initialize the sample label array:
	localtime_r(&ntime, &tmtime);
	daySamples = id->dayStart;

	startTime = ntime - WV_SECONDS_IN_DAY;
	localtime_r(&startTime, &tmtime);
	tmtime.tm_sec = 0;
	tmtime.tm_hour = (daySamples * id->archiveInterval) / 60;
	tmtime.tm_min = (daySamples * id->archiveInterval) % 60;
	startTime = mktime(&tmtime);

	for (i = 0; i < DAILY_NUM_VALUES(id) - 1; i++)
	{
		if (daySamples >= DAILY_NUM_VALUES(id) - 1)
			daySamples = 0;

		localtime_r(&startTime, &tmtime);
		sprintf(sampleLabels[daySamples], "%d:%2.2d", tmtime.tm_hour, tmtime.tm_min);
		sprintf(sampleHourLabels[daySamples], "%d:00", tmtime.tm_hour);

		startTime += (60 * id->archiveInterval);
		daySamples++;
	}

	// Do the current time (always store it in the last slot):
	localtime_r(&startTime, &tmtime);
	sprintf(sampleLabels[DAILY_NUM_VALUES(id) - 1], "%d:%2.2d", tmtime.tm_hour, tmtime.tm_min);
	startTime += WV_SECONDS_IN_HOUR;
	localtime_r(&startTime, &tmtime);
	sprintf(sampleHourLabels[DAILY_NUM_VALUES(id) - 1], "%d:00", tmtime.tm_hour);
}

int htmlmgrBPTrendInit
(
	HTML_MGR_ID         id,
	int                 timerIntervalMINs
)
{
	int                 i;

	//  ... initialize the barometric pressure trend array
	for (i = 0; i < BP_MAX_VALUES; i++)
	{
		id->baromTrendValues[i] = 0;
	}

	id->baromTrendNumValues = 0;
	id->baromTrendIndex = 0;
	id->baromTrendIndexMax = 60 / timerIntervalMINs;    // samples per hour
	id->baromTrendIndexMax *= 4;                        // 4 hours
	id->baromTrendIndicator = 1;                        // steady

	return OK;
}

static int computeBPTrend
(
	HTML_MGR_ID         id
)
{
	register int        i;
	register float      avg, sum = 0;
	float               currVal = id->loopStore.barometer;

	//  ... compute the barometric pressure trend
	//  ... we just use a circular queue since the order of the values
	//  ... doesn't really matter, and it is more efficient

	//  ... save the new value
	id->baromTrendValues[id->baromTrendIndex] = currVal;

	//  ... do we need to wrap the index?
	if (++id->baromTrendIndex >= id->baromTrendIndexMax)
		id->baromTrendIndex = 0;

	//  ... manage the number of values
	if (id->baromTrendNumValues < id->baromTrendIndexMax)
		id->baromTrendNumValues++;

	//  ... compute the average over all values
	for (i = 0; i < id->baromTrendNumValues; i++)
	{
		sum += id->baromTrendValues[i];
	}
	avg = sum / id->baromTrendNumValues;

	if (currVal < avg)
		id->baromTrendIndicator = 0;
	else if (currVal > avg)
		id->baromTrendIndicator = 2;
	else
		id->baromTrendIndicator = 1;

	return OK;
}

static int newProcessEntryPoint(void* pargs)
{
	char*               binFile = (char*)pargs;

	return (system(binFile));
}

int htmlgenOutputFiles(HTML_MGR_ID , uint64_t );

int htmlmgrGenerate
(
	HTML_MGR_ID         id
)
{
	int                 retVal, htmls = 0;
	char                temp[256];
	struct stat         fileData;

	GenerateTime = radTimeGetMSSinceEpoch();

#if __DEBUG_BUFFERS
	MsgLog(PRI_STATUS, "DBG BFRS: HTML BEGIN: %u of %u available",
		buffersGetAvailable(),
		buffersGetTotal());
#endif

	//  ... compute the Barometric Pressure trend
	computeBPTrend(id);

#if DEBUG_GENERATION
	MsgLog(PRI_MEDIUM, "GENERATE: images");
#endif

	//  ... clear the archiveAvailable flag (must be after generator loop)
	id->newArchiveMask = 0;

#if DEBUG_GENERATION
	MsgLog(PRI_MEDIUM, "GENERATE: pre-generate script");
#endif

	// If the wview pre-generation script exists, run it now
	sprintf(temp, "%s/%s", WVIEW_CONFIG_DIR, HTML_PRE_GEN_SCRIPT);
	if (stat(temp, &fileData) == 0)
	{
		// File exists, run it
		radStartProcess(newProcessEntryPoint, temp);
	}

#if DEBUG_GENERATION
	MsgLog(PRI_MEDIUM, "GENERATE: templates");
#endif

	//  ... now generate the HTML
	if ((htmls = htmlgenOutputFiles(id, GenerateTime)) == ERROR)
	{
		return ERROR;
	}

	wvutilsLogEvent(PRI_STATUS, "Generated: %u ms:%d template files",
		(uint32_t)(radTimeGetMSSinceEpoch() - GenerateTime), htmls);

	id->templatesGenerated += htmls;
	statusUpdateStat(HTML_STATS_IMAGES_GENERATED, 0);
	statusUpdateStat(HTML_STATS_TEMPLATES_GENERATED, id->templatesGenerated);

#if __DEBUG_BUFFERS
	MsgLog(PRI_STATUS, "DBG BFRS: HTML END: %u of %u available",
		buffersGetAvailable(),
		buffersGetTotal());
#endif

#if DEBUG_GENERATION
	MsgLog(PRI_MEDIUM, "GENERATE: post-generate script");
#endif

	// Finally, if the wview post-generation script exists, run it now
	sprintf(temp, "%s/%s", WVIEW_CONFIG_DIR, HTML_POST_GEN_SCRIPT);
	if (stat(temp, &fileData) == 0)
	{
		// File exists, run it
		radStartProcess(newProcessEntryPoint, temp);
	}

#if DEBUG_GENERATION
	MsgLog(PRI_MEDIUM, "GENERATE: DONE");
#endif

	return OK;
}

// read archive database to initialize our historical arrays:
int htmlmgrHistoryInit(HTML_MGR_ID id)
{
	HISTORY_DATA    data;
	time_t          ntime, baseTime, arcTime;
	struct tm       locTime;
	int             i, retVal, saveHour;

	// Compute when last archive record should have been:
	arcTime = time(NULL);
	localtime_r(&arcTime, &locTime);
	locTime.tm_min = ((locTime.tm_min / id->archiveInterval) * id->archiveInterval);
	locTime.tm_sec = 0;
	arcTime = mktime(&locTime);

	// first, figure out our start label indexes and populate the history arrays
	// for each of day, week, month and year:

	// do the samples in the last day:
	id->dayStart = wvutilsGetDayStartTime(id->archiveInterval);

	// update the sample label array:
	htmlmgrSetSampleLabels(id);

	baseTime = arcTime;
	saveHour = -1;

	for (i = 0; i < DAILY_NUM_VALUES(id); i++)
	{
		ntime = baseTime;
		ntime -= (WV_SECONDS_IN_DAY - (i * SECONDS_IN_INTERVAL(id->archiveInterval)));

		retVal = dbsqliteArchiveGetAverages(id->isMetricUnits,
			id->archiveInterval,
			&data,
			ntime,
			1);

		if (retVal <= 0)
		{
			id->windDayValues[i] = ARCHIVE_VALUE_NULL;
		}
		else
		{
			if (data.values[DATA_INDEX_windDir] <= ARCHIVE_VALUE_NULL)
			{
				id->windDayValues[i] = ARCHIVE_VALUE_NULL;
			}
			else
			{
				id->windDayValues[i] = data.values[DATA_INDEX_windDir] / data.samples[DATA_INDEX_windDir];
			}
		}

		if (((i % 50) == 0) || ((i + 1) == DAILY_NUM_VALUES(id)))
		{
			MsgLog(PRI_STATUS, "Wind : DAY: samples=%d", i);
		}
	}

	dbsqliteHistoryPragmaSet("synchronous", "normal");
	return OK;
}

int htmlmgrAddSampleValue(HTML_MGR_ID id, HISTORY_DATA* data, int numIntervals)
{
	register int   j;

	// check for data gap
	while (numIntervals > 1)
	{
		// we have apparently missed some archive records - add empty data in
		// the "gap"

		// do the normal new record stuff first
		for (j = 0; j < DAILY_NUM_VALUES(id) - 1; j++)
		{
			id->windDayValues[j] = id->windDayValues[j + 1];
		}

		// populate history data with ARCHIVE_VALUE_NULL
		id->windDayValues[DAILY_NUM_VALUES(id) - 1] = ARCHIVE_VALUE_NULL;

		// decrement interval count
		numIntervals--;
	}

	// now add the new record data
	for (j = 0; j < DAILY_NUM_VALUES(id) - 1; j++)
	{
		id->windDayValues[j] = id->windDayValues[j + 1];
	}

	id->windDayValues[DAILY_NUM_VALUES(id) - 1] = data->values[DATA_INDEX_windDir];

	// Compute the new day start interval:
	id->dayStart = wvutilsGetDayStartTime(id->archiveInterval);

	// update the sample label array:
	htmlmgrSetSampleLabels(id);

	return OK;
}