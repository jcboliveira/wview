/*---------------------------------------------------------------------------

  FILENAME:
		htmlGenerate.c

  PURPOSE:
		Provide the wview html generator utilities.

  REVISION HISTORY:
		Date            Engineer        Revision        Remarks
		08/30/03        M.S. Teel       0               Original
		06/26/2005      S. Pacenka      1               Add metric support to
														the temp dial
		02/16/2008      M.B. Clark      2               Added ability to customize
														color/size of plots using
														graphics.conf settings
		03/24/2008      W. Krenn        3               metric adaptations
		12/01/2009      M. Hornsby      4               Add Moon Rise and Set

  NOTES:
		This is by far the ugliest code in the wview source. Shortcuts are taken
		to optimize image and HTML generation.

  LICENSE:
		Copyright (c) 2004, Mark S. Teel (mark@teel.ws)

		This source code is released for free distribution under the terms
		of the GNU General Public License.

----------------------------------------------------------------------------*/

/*  ... System include files
*/
#include <termios.h>


/*  ... Local include files
*/
#include <services.h>
#include <html.h>
#include <radtextsearch.h>

/*  ... global memory declarations
*/

/*  ... global memory referenced
*/

/*  ... static (local) memory declarations
*/
#define _DEBUG_GENERATION                   FALSE

static TEXT_SEARCH_ID   tagSearchEngine;

// Note: sample width cannot be less than 10 degrees!
#define WR_SAMPLE_WIDTH_DAY             20
#define WR_SAMPLE_WIDTH_WEEK            30
#define WR_SAMPLE_WIDTH_MONTH           45
#define WR_SAMPLE_WIDTH_YEAR            45
#define WR_MAX_COUNTERS                 36
#define WR_WEDGE_SPACE                  4

//  ... define the HTML data tags
//  ... "computeTag" below depends on the order of these strings!
static char* dataTags[] =
{
	"<!--tempUnit-->",
	"<!--humUnit-->",
	"<!--windUnit-->",
	"<!--barUnit-->",
	"<!--rateUnit-->",
	"<!--rainUnit-->",
	"<!--stationCity-->",
	"<!--stationState-->",
	"<!--stationDate-->",
	"<!--stationTime-->",
	"<!--sunriseTime-->",                   // 10
	"<!--sunsetTime-->",
	"<!--outsideTemp-->",
	"<!--windChill-->",
	"<!--outsideHumidity-->",
	"<!--outsideHeatIndex-->",
	"<!--windDirection-->",
	"<!--windSpeed-->",
	"<!--outsideDewPt-->",
	"<!--barometer-->",
	"<!--rainRate-->",                      // 20
	"<!--dailyRain-->",
	"<!--monthlyRain-->",
	"<!--stormRain-->",
	"<!--totalRain-->",
	"<!--hiOutsideTemp-->",
	"<!--hiOutsideTempTime-->",
	"<!--lowOutsideTemp-->",
	"<!--lowOutsideTempTime-->",
	"<!--hiHumidity-->",
	"<!--hiHumTime-->",                     // 30
	"<!--lowHumidity-->",
	"<!--lowHumTime-->",
	"<!--hiDewpoint-->",
	"<!--hiDewpointTime-->",
	"<!--lowDewpoint-->",
	"<!--lowDewpointTime-->",
	"<!--hiWindSpeed-->",
	"<!--hiWindSpeedTime-->",
	"<!--hiBarometer-->",
	"<!--hiBarometerTime-->",               // 40
	"<!--lowBarometer-->",
	"<!--lowBarometerTime-->",
	"<!--hiRainRate-->",
	"<!--hiRainRateTime-->",
	"<!--lowWindchill-->",
	"<!--lowWindchillTime-->",
	"<!--hiHeatindex-->",
	"<!--hiHeatindexTime-->",
	"<!--hiMonthlyOutsideTemp-->",
	"<!--lowMonthlyOutsideTemp-->",         // 50
	"<!--hiMonthlyHumidity-->",
	"<!--lowMonthlyHumidity-->",
	"<!--hiMonthlyDewpoint-->",
	"<!--lowMonthlyDewpoint-->",
	"<!--hiMonthlyWindSpeed-->",
	"<!--hiMonthlyBarometer-->",
	"<!--lowMonthlyBarometer-->",
	"<!--lowMonthlyWindchill-->",
	"<!--hiMonthlyHeatindex-->",
	"<!--hiMonthlyRainRate-->",             // 60
	"<!--hiYearlyOutsideTemp-->",
	"<!--lowYearlyOutsideTemp-->",
	"<!--hiYearlyHumidity-->",
	"<!--lowYearlyHumidity-->",
	"<!--hiYearlyDewpoint-->",
	"<!--lowYearlyDewpoint-->",
	"<!--hiYearlyWindSpeed-->",
	"<!--hiYearlyBarometer-->",
	"<!--lowYearlyBarometer-->",
	"<!--lowYearlyWindchill-->",            // 70
	"<!--hiYearlyHeatindex-->",
	"<!--hiYearlyRainRate-->",
	"<!--wviewVersion-->",
	"<!--wviewUpTime-->",
	"<!--UV-->",
	"<!--ET-->",
	"<!--solarRad-->",
	"<!--extraTemp1-->",
	"<!--extraTemp2-->",
	"<!--extraTemp3-->",                    // 80
	"<!--soilTemp1-->",
	"<!--soilTemp2-->",
	"<!--soilTemp3-->",
	"<!--soilTemp4-->",
	"<!--leafTemp1-->",
	"<!--leafTemp2-->",
	"<!--extraHumid1-->",
	"<!--extraHumid2-->",
	"<!--forecastRule-->",
	"<!--forecastIcon-->",                  // 90
	"<!--stationElevation-->",
	"<!--stationLatitude-->",
	"<!--stationLongitude-->",
	"<!--insideTemp-->",
	"<!--insideHumidity-->",

	"<!--hourrain-->",
	"<!--hourwindrun-->",
	"<!--houravgtemp-->",
	"<!--houravgwind-->",
	"<!--hourdomwinddir-->",                // 100
	"<!--houravghumid-->",
	"<!--houravgdewpt-->",
	"<!--houravgbarom-->",
	"<!--hourchangetemp-->",
	"<!--hourchangewind-->",
	"<!--hourchangewinddir-->",
	"<!--hourchangehumid-->",
	"<!--hourchangedewpt-->",
	"<!--hourchangebarom-->",
	"<!--daywindrun-->",                    // 110
	"<!--dayavgtemp-->",
	"<!--dayavgwind-->",
	"<!--daydomwinddir-->",
	"<!--dayavghumid-->",
	"<!--dayavgdewpt-->",
	"<!--dayavgbarom-->",
	"<!--daychangetemp-->",
	"<!--daychangewind-->",
	"<!--daychangewinddir-->",
	"<!--daychangehumid-->",                // 120
	"<!--daychangedewpt-->",
	"<!--daychangebarom-->",
	"<!--weekwindrun-->",
	"<!--weekavgtemp-->",
	"<!--weekavgwind-->",
	"<!--weekdomwinddir-->",
	"<!--weekavghumid-->",
	"<!--weekavgdewpt-->",
	"<!--weekavgbarom-->",
	"<!--weekchangetemp-->",                // 130
	"<!--weekchangewind-->",
	"<!--weekchangewinddir-->",
	"<!--weekchangehumid-->",
	"<!--weekchangedewpt-->",
	"<!--weekchangebarom-->",
	"<!--monthtodatewindrun-->",
	"<!--monthtodateavgtemp-->",
	"<!--monthtodateavgwind-->",
	"<!--monthtodatedomwinddir-->",
	"<!--monthtodateavghumid-->",           // 140
	"<!--monthtodateavgdewpt-->",
	"<!--monthtodateavgbarom-->",
	"<!--monthtodatemaxtempdate-->",
	"<!--monthtodatemintempdate-->",
	"<!--monthtodateminchilldate-->",
	"<!--monthtodatemaxheatdate-->",
	"<!--monthtodatemaxhumiddate-->",
	"<!--monthtodateminhumiddate-->",
	"<!--monthtodatemaxdewptdate-->",
	"<!--monthtodatemindewptdate-->",       // 150
	"<!--monthtodatemaxbaromdate-->",
	"<!--monthtodateminbaromdate-->",
	"<!--monthtodatemaxwinddate-->",
	"<!--monthtodatemaxgustdate-->",
	"<!--monthtodatemaxrainratedate-->",
	"<!--yeartodatewindrun-->",
	"<!--yeartodateavgtemp-->",
	"<!--yeartodateavgwind-->",
	"<!--yeartodatedomwinddir-->",
	"<!--yeartodateavghumid-->",            // 160
	"<!--yeartodateavgdewpt-->",
	"<!--yeartodateavgbarom-->",
	"<!--yeartodatemaxtempdate-->",
	"<!--yeartodatemintempdate-->",
	"<!--yeartodateminchilldate-->",
	"<!--yeartodatemaxheatdate-->",
	"<!--yeartodatemaxhumiddate-->",
	"<!--yeartodateminhumiddate-->",
	"<!--yeartodatemaxdewptdate-->",
	"<!--yeartodatemindewptdate-->",        // 170
	"<!--yeartodatemaxbaromdate-->",
	"<!--yeartodateminbaromdate-->",
	"<!--yeartodatemaxwinddate-->",
	"<!--yeartodatemaxgustdate-->",
	"<!--yeartodatemaxrainratedate-->",
	"<!--windDirectionDegrees-->",
	"<!--PLACEHOLDER1-->",
	"<!--PLACEHOLDER2-->",
	"<!--PLACEHOLDER3-->",
	"<!--PLACEHOLDER4-->",                  // 180
	"<!--hiRadiation-->",
	"<!--hiRadiationTime-->",
	"<!--hiMonthlyRadiation-->",
	"<!--hiYearlyRadiation-->",
	"<!--hiUV-->",
	"<!--hiUVTime-->",
	"<!--hiMonthlyUV-->",
	"<!--hiYearlyUV-->",
	"<!--moonPhase-->",
	"<!--airDensityUnit-->",                // 190
	"<!--airDensity-->",
	"<!--cumulusBaseUnit-->",
	"<!--cumulusBase-->",
	"<!--soilMoist1-->",
	"<!--soilMoist2-->",
	"<!--leafWet1-->",
	"<!--leafWet2-->",
	"<!--dayhighwinddir-->",
	"<!--baromtrend-->",
	"<!--dailyRainMM-->",                   // 200
	"<!--stationDateMetric-->",
	"<!--middayTime-->",
	"<!--dayLength-->",
	"<!--civilriseTime-->",
	"<!--civilsetTime-->",
	"<!--astroriseTime-->",
	"<!--astrosetTime-->",
	"<!--stormStart-->",
	"<!--forecastIconFile-->",
	"<!--rainSeasonStart-->",               // 210
	"<!--intervalAvgWindChill-->",
	"<!--intervalAvgWindSpeed-->",
	"<!--stationPressure-->",
	"<!--altimeter-->",
	"<!--localRadarURL-->",
	"<!--localForecastURL-->",
	"<!--windGustSpeed-->",
	"<!--windGustDirectionDegrees-->",
	"<!--windBeaufortScale-->",
	"<!--intervalAvgBeaufortScale-->",      // 220
	"<!--stationType-->",

	"<!--wxt510Hail-->",
	"<!--wxt510Hailrate-->",
	"<!--wxt510HeatingTemp-->",
	"<!--wxt510HeatingVoltage-->",
	"<!--wxt510SupplyVoltage-->",
	"<!--wxt510ReferenceVoltage-->",
	"<!--wxt510RainDuration-->",
	"<!--wxt510RainPeakRate-->",
	"<!--wxt510HailDuration-->",            // 230
	"<!--wxt510HailPeakRate-->",
	"<!--wxt510Rain-->",

	"<!--rxCheckPercent-->",
	"<!--tenMinuteAvgWindSpeed-->",
	"<!--PLACEHOLDER1-->",
	"<!--dayWindRoseList-->",
	"<!--txBatteryStatus-->",
	"<!--consBatteryVoltage-->",

	"<!--stationTimeNoSecs-->",
	"<!--wxt510RainDurationMin-->",         // 240
	"<!--wxt510HailDurationMin-->",

	"<!--wmr918WindBatteryStatus-->",
	"<!--wmr918RainBatteryStatus-->",
	"<!--wmr918OutTempBatteryStatus-->",
	"<!--wmr918InTempBatteryStatus-->",

	"<!--windSpeed_ms-->",
	"<!--windGustSpeed_ms-->",
	"<!--intervalAvgWindSpeed_ms-->",
	"<!--tenMinuteAvgWindSpeed_ms-->",
	"<!--hiWindSpeed_ms-->",                // 250
	"<!--hiMonthlyWindSpeed_ms-->",
	"<!--hiYearlyWindSpeed_ms-->",
	"<!--houravgwind_ms-->",
	"<!--dayavgwind_ms-->",
	"<!--weekavgwind_ms-->",
	"<!--monthtodateavgwind_ms-->",
	"<!--yeartodateavgwind_ms-->",
	"<!--hourchangewind_ms-->",
	"<!--daychangewind_ms-->",
	"<!--weekchangewind_ms-->",             // 260

	"<!--windSpeed_kts-->",
	"<!--windGustSpeed_kts-->",
	"<!--intervalAvgWindSpeed_kts-->",
	"<!--tenMinuteAvgWindSpeed_kts-->",
	"<!--hiWindSpeed_kts-->",
	"<!--hiMonthlyWindSpeed_kts-->",
	"<!--hiYearlyWindSpeed_kts-->",
	"<!--houravgwind_kts-->",
	"<!--dayavgwind_kts-->",
	"<!--weekavgwind_kts-->",               // 270
	"<!--monthtodateavgwind_kts-->",
	"<!--yeartodateavgwind_kts-->",
	"<!--hourchangewind_kts-->",
	"<!--daychangewind_kts-->",
	"<!--weekchangewind_kts-->",

	"<!--wmr918Humid3-->",
	"<!--wmr918Pool-->",
	"<!--wmr918poolTempBatteryStatus-->",
	"<!--wmr918extra1BatteryStatus-->",
	"<!--wmr918extra2BatteryStatus-->",     // 280
	"<!--wmr918extra3BatteryStatus-->",

	"<!--hiAllTimeOutsideTemp-->",
	"<!--lowAllTimeOutsideTemp-->",
	"<!--hiAllTimeHumidity-->",
	"<!--lowAllTimeHumidity-->",
	"<!--hiAllTimeDewpoint-->",
	"<!--lowAllTimeDewpoint-->",
	"<!--hiAllTimeWindSpeed-->",
	"<!--hiAllTimeBarometer-->",
	"<!--lowAllTimeBarometer-->",           // 290
	"<!--lowAllTimeWindchill-->",
	"<!--hiAllTimeHeatindex-->",
	"<!--hiAllTimeRainRate-->",
	"<!--hiAllTimeRadiation-->",
	"<!--hiAllTimeUV-->",

	"<!--alltimeavgtemp-->",
	"<!--alltimeavgwind-->",
	"<!--alltimedomwinddir-->",
	"<!--alltimeavghumid-->",
	"<!--alltimeavgdewpt-->",               // 300
	"<!--alltimeavgbarom-->",

	"<!--alltimemaxtempdate-->",
	"<!--alltimemintempdate-->",
	"<!--alltimeminchilldate-->",
	"<!--alltimemaxheatdate-->",
	"<!--alltimemaxhumiddate-->",
	"<!--alltimeminhumiddate-->",
	"<!--alltimemaxdewptdate-->",
	"<!--alltimemindewptdate-->",
	"<!--alltimemaxbaromdate-->",           // 310
	"<!--alltimeminbaromdate-->",
	"<!--alltimemaxwinddate-->",
	"<!--alltimemaxgustdate-->",
	"<!--alltimemaxrainratedate-->",
	"<!--wmr918Tendency-->",
	"<!--stationName-->",
	"<!--moonriseTime-->",
	"<!--moonsetTime-->",
	"<!--apparentTemp-->",

	"<!--genExtraTemp1-->",                 // 320
	"<!--genExtraTemp2-->",
	"<!--genExtraTemp3-->",
	"<!--genExtraTemp4-->",
	"<!--genExtraTemp5-->",
	"<!--genExtraTemp6-->",
	"<!--genExtraTemp7-->",
	"<!--genExtraTemp8-->",
	"<!--genExtraTemp9-->",
	"<!--genExtraTemp10-->",
	"<!--genExtraTemp11-->",                // 330
	"<!--genExtraTemp12-->",
	"<!--genExtraTemp13-->",
	"<!--genExtraTemp14-->",
	"<!--genExtraTemp15-->",
	"<!--genExtraTemp16-->",
	"<!--genExtraHumidity1-->",
	"<!--genExtraHumidity2-->",
	"<!--genExtraHumidity3-->",
	"<!--genExtraHumidity4-->",
	"<!--genExtraHumidity5-->",             // 340
	"<!--genExtraHumidity6-->",
	"<!--genExtraHumidity7-->",
	"<!--genExtraHumidity8-->",
	"<!--genExtraHumidity9-->",
	"<!--genExtraHumidity10-->",
	"<!--genExtraHumidity11-->",
	"<!--genExtraHumidity12-->",
	"<!--genExtraHumidity13-->",
	"<!--genExtraHumidity14-->",
	"<!--genExtraHumidity15-->",            // 350
	"<!--genExtraHumidity16-->",
	"<!--genExtraTempBatteryStatus1-->",
	"<!--genExtraTempBatteryStatus2-->",
	"<!--genExtraTempBatteryStatus3-->",
	"<!--genExtraTempBatteryStatus4-->",
	"<!--genExtraTempBatteryStatus5-->",
	"<!--genExtraTempBatteryStatus6-->",
	"<!--genExtraTempBatteryStatus7-->",
	"<!--genExtraTempBatteryStatus8-->",
	"<!--genExtraTempBatteryStatus9-->",    // 360
	"<!--genExtraTempBatteryStatus10-->",
	"<!--genExtraTempBatteryStatus11-->",
	"<!--genExtraTempBatteryStatus12-->",
	"<!--genExtraTempBatteryStatus13-->",
	"<!--genExtraTempBatteryStatus14-->",
	"<!--genExtraTempBatteryStatus15-->",
	"<!--genExtraTempBatteryStatus16-->",
	"<!--genExtraWindBatteryStatus-->",
	"<!--genExtraOutTempBatteryStatus-->",
	"<!--genExtraConsoleBatteryStatus-->",  // 370
	"<!--genExtraUVBatteryStatus-->",
	"<!--genExtraSolarBatteryStatus-->",
	"<!--genExtraRainBatteryStatus-->",
	"<!--WetBulbTemp-->",
	"<!--hiOutsideATemp-->",
	"<!--hiOutsideATempTime-->",
	"<!--lowOutsideATemp-->",
	"<!--lowOutsideATempTime-->",
	"<!--houravgatemp-->",
	"<!--hourchangeatemp-->",
	"<!--weekavgatemp-->",
	"<!--weekchangeatemp-->",
	"<!--dayavgatemp-->",
	"<!--daychangeatemp-->",
	"<!--monthtodateavgatemp-->",
	"<!--monthtodatemaxatempdate-->",
	"<!--monthtodateminatempdate-->",
	"<!--yeartodateavgatemp-->",
	"<!--yeartodatemaxatempdate-->",
	"<!--yeartodateminatempdate-->",
	"<!--alltimeavgatemp-->",
	"<!--alltimemaxatempdate-->",
	"<!--alltimeminatempdate-->",
	"<!--hiMonthlyaTemp-->",
	"<!--lowMonthlyaTemp-->",
	"<!--hiYearlyaTemp-->",
	"<!--lowYearlyaTemp-->",
	"<!--hiAllTimeaTemp-->",
	"<!--lowAllTimeaTemp-->",

	"<!--hisoilTemp1-->",
	"<!--hisoilTemp1Time-->",
	"<!--lowsoilTemp1-->",
	"<!--lowsoilTemp1Time-->",
	"<!--hisoilMoist1-->",
	"<!--hisoilMoist1Time-->",
	"<!--lowsoilMoist1-->",
	"<!--lowsoilMoist1Time-->",
	"<!--hileafWet1-->",
	"<!--hileafWet1Time-->",
	"<!--lowleafWet1-->",
	"<!--lowleafWet1Time-->",

	"<!--houravgsoilTemp1-->",
	"<!--hourchangesoilTemp1-->",
	"<!--weekavgsoilTemp1-->",
	"<!--weekchangesoilTemp1-->",
	"<!--dayavgsoilTemp1-->",
	"<!--daychangesoilTemp1-->",
	"<!--monthtodateavgsoilTemp1-->",
	"<!--monthtodatemaxsoilTemp1date-->",
	"<!--monthtodateminsoilTemp1date-->",
	"<!--yeartodateavgsoilTemp1-->",
	"<!--yeartodatemaxsoilTemp1date-->",
	"<!--yeartodateminsoilTemp1date-->",
	"<!--alltimeavgsoilTemp1-->",
	"<!--alltimemaxsoilTemp1date-->",
	"<!--alltimeminsoilTemp1date-->",
	"<!--hiMonthlysoilTemp1-->",
	"<!--lowMonthlysoilTemp1-->",
	"<!--hiYearlysoilTemp1-->",
	"<!--lowYearlysoilTemp1-->",
	"<!--hiAllTimesoilTemp1-->",
	"<!--lowAllTimesoilTemp1-->",

	"<!--houravgsoilMoist1-->",
	"<!--hourchangesoilMoist1-->",
	"<!--weekavgsoilMoist1-->",
	"<!--weekchangesoilMoist1-->",
	"<!--dayavgsoilMoist1-->",
	"<!--daychangesoilMoist1-->",
	"<!--monthtodateavgsoilMoist1-->",
	"<!--monthtodatemaxsoilMoist1date-->",
	"<!--monthtodateminsoilMoist1date-->",
	"<!--yeartodateavgsoilMoist1-->",
	"<!--yeartodatemaxsoilMoist1date-->",
	"<!--yeartodateminsoilMoist1date-->",
	"<!--alltimeavgsoilMoist1-->",
	"<!--alltimemaxsoilMoist1date-->",
	"<!--alltimeminsoilMoist1date-->",
	"<!--hiMonthlysoilMoist1-->",
	"<!--lowMonthlysoilMoist1-->",
	"<!--hiYearlysoilMoist1-->",
	"<!--lowYearlysoilMoist1-->",
	"<!--hiAllTimesoilMoist1-->",
	"<!--lowAllTimesoilMoist1-->",

	"<!--houravgleafWet1-->",
	"<!--hourchangeleafWet1-->",
	"<!--weekavgleafWet1-->",
	"<!--weekchangeleafWet1-->",
	"<!--dayavgleafWet1-->",
	"<!--daychangeleafWet1-->",
	"<!--monthtodateavgleafWet1-->",
	"<!--monthtodatemaxleafWet1date-->",
	"<!--monthtodateminleafWet1date-->",
	"<!--yeartodateavgleafWet1-->",
	"<!--yeartodatemaxleafWet1date-->",
	"<!--yeartodateminleafWet1date-->",
	"<!--alltimeavgleafWet1-->",
	"<!--alltimemaxleafWet1date-->",
	"<!--alltimeminleafWet1date-->",
	"<!--hiMonthlyleafWet1-->",
	"<!--lowMonthlyleafWet1-->",
	"<!--hiYearlyleafWet1-->",
	"<!--lowYearlyleafWet1-->",
	"<!--hiAllTimeleafWet1-->",
	"<!--lowAllTimeleafWet1-->",

	"<!--hourET-->",
	"<!--weekET-->",
	"<!--monthET-->",
	"<!--yearET-->",
	"<!--weekrain-->",

	"<!--hiOutsideATemp1-->",
	"<!--hiOutsideATempTime1-->",
	"<!--lowOutsideATemp1-->",
	"<!--lowOutsideATempTime1-->",
	"<!--houravgatemp1-->",
	"<!--hourchangeatemp1-->",
	"<!--weekavgatemp1-->",
	"<!--weekchangeatemp1-->",
	"<!--dayavgatemp1-->",
	"<!--daychangeatemp1-->",
	"<!--monthtodateavgatemp1-->",
	"<!--monthtodatemaxatempdate1-->",
	"<!--monthtodateminatempdate1-->",
	"<!--yeartodateavgatemp1-->",
	"<!--yeartodatemaxatempdate1-->",
	"<!--yeartodateminatempdate1-->",
	"<!--alltimeavgatemp1-->",
	"<!--alltimemaxatempdate1-->",
	"<!--alltimeminatempdate1-->",
	"<!--hiMonthlyaTemp1-->",
	"<!--lowMonthlyaTemp1-->",
	"<!--hiYearlyaTemp1-->",
	"<!--lowYearlyaTemp1-->",
	"<!--hiAllTimeaTemp1-->",
	"<!--lowAllTimeaTemp1-->",
	"<!--todaydomwinddir-->",
	"<!--twoMinuteAvgWindSpeed-->",
	"<!--tenMinuteWindGust-->",
	"<!--WinddirtenMinuteWindGustDegrees-->",
	NULL
};

static char BPTrendLabels[4] =
{
	'-',
	'~',
	'+',
	' '
};

static char* monthLabel[13] =
{
	"UNK",
	"JAN",
	"FEB",
	"MAR",
	"APR",
	"MAY",
	"JUN",
	"JUL",
	"AUG",
	"SEP",
	"OCT",
	"NOV",
	"DEC"
};

static char                 winddir[16][4] =
{
	"N",
	"NNE",
	"NE",
	"ENE",
	"E",
	"ESE",
	"SE",
	"SSE",
	"S",
	"SSW",
	"SW",
	"WSW",
	"W",
	"WNW",
	"NW",
	"NNW"
};

static char                 tendency[5][14] =
{
	"Clear",
	"Partly Cloudy",
	"Cloudy",
	"Rain",
	"Unknown"
};

static char* buildTimeTag(int16_t timeval)
{
	static char     ret[16];

	if (timeval < 0)
	{
		sprintf(ret, "--:--");
	}
	else
	{
		sprintf(ret, "%2.2d:%2.2d",
			EXTRACT_PACKED_HOUR(timeval),
			EXTRACT_PACKED_MINUTE(timeval));
	}
	return ret;
}

char* buildWindDirString(int dir)
{
	if (dir < 0 || dir > 15)
	{
		return "---";
	}
	else
	{
		return winddir[dir];
	}
}

char* buildTendencyString(int tend)
{
	int text;

	switch (tend)
	{
	case 0x0c: // Clear
		text = 0;
		break;
	case 0x06: // Partly Cloudy
		text = 1;
		break;
	case 0x02: // Cloudy
		text = 2;
		break;
	case 0x03: // Rain
		text = 3;
		break;
	default: // Unknown
		text = 4;
	}

	return tendency[text];
}

static char* makeduration(float x)
{
	int         hour, min, sec;
	static char duration[32];

	hour = (int)(x / 3600);
	x -= (hour * 3600);
	min = (int)(x / 60);
	sec = x - (60 * min);
	sprintf(duration, "%d:%2.2d:%2.2d", hour, min, sec);

	return duration;
}

static char* getBattStatus(uint8_t status)
{
	static char batteryStatus[16];

	if (status == 0)
		sprintf(batteryStatus, "LOW");
	else if (status == 1)
		sprintf(batteryStatus, "OK");
	else
		sprintf(batteryStatus, "UNKNOWN");
	return batteryStatus;
}

static void computeTag(HTML_MGR_ID id, char* tag, int len, char* store)
{
	time_t          ntime;
	struct tm       loctime;
	char            temp[SEARCH_TEXT_MAX];
	int             tagIndex, tempInt, tempInt1;
	float           tempfloat;
	SENSOR_STORE*    sensors = &id->hilowStore;

	// First, find the index:
	strncpy(temp, tag, len);
	temp[len] = 0;

	if (radtextsearchFind(tagSearchEngine, temp, &tagIndex) == ERROR)
	{
		store[0] = 0;
		return;
	}

	//  now "tagIndex" is the index for the matched tag:
	switch (tagIndex)
	{
	case 0:
		strcpy(store, " C");
		break;
	case 1:
		strcpy(store, " %");
		break;
	case 2:
		sprintf(store, " %s", wvutilsGetWindUnitLabel());
		break;
	case 3:
		strcpy(store, " mb");
		break;
	case 4:
		if (wvutilsGetRainIsMM())
			strcpy(store, " mm/h");
		else
			strcpy(store, " cm/h");
		break;
	case 5:
		if (wvutilsGetRainIsMM())
			strcpy(store, " mm");
		else
			strcpy(store, " cm");
		break;
	case 6:
		wvstrncpy(store, id->stationCity, HTML_MAX_LINE_LENGTH);
		break;
	case 7:
		wvstrncpy(store, id->stationState, HTML_MAX_LINE_LENGTH);
		break;
	case 8:
		ntime = time(NULL);
		localtime_r(&ntime, &loctime);
		strftime(store, 256, id->dateFormat, &loctime);
		break;
	case 9:
		ntime = time(NULL);
		localtime_r(&ntime, &loctime);
		sprintf(store, "%2.2d:%2.2d:%2.2d", loctime.tm_hour, loctime.tm_min, loctime.tm_sec);
		break;
	case 10:
		wvstrncpy(store, buildTimeTag(id->sunrise), HTML_MAX_LINE_LENGTH);
		break;
	case 11:
		wvstrncpy(store, buildTimeTag(id->sunset), HTML_MAX_LINE_LENGTH);
		break;
	case 12:
		sprintf(store, "%.1f", wvutilsConvertFToC(id->loopStore.outTemp));
		break;
	case 13:
		sprintf(store, "%.1f", wvutilsConvertFToC(id->loopStore.windchill));
		break;
	case 14:
		sprintf(store, "%d", id->loopStore.outHumidity);
		break;
	case 15:
		sprintf(store, "%.1f", wvutilsConvertFToC(id->loopStore.heatindex));
		break;
	case 16:
		tempfloat = (float)id->loopStore.windDir + 11.24;
		tempfloat /= 22.5;
		tempInt = (int)tempfloat;
		tempInt %= 16;
		sprintf(store, "%s", buildWindDirString(tempInt));
		break;
	case 17:
		sprintf(store, "%.1f", wvutilsGetWindSpeed(id->loopStore.windSpeedF));
		break;
	case 18:
		sprintf(store, "%.1f", wvutilsConvertFToC(id->loopStore.dewpoint));
		break;
	case 19:
		sprintf(store, "%.1f", wvutilsConvertINHGToHPA(id->loopStore.barometer));
		break;
	case 20:
		sprintf(store, "%.2f", wvutilsConvertRainINToMetric(id->loopStore.rainRate));
		break;
	case 21:
		strcpy(store, wvutilsPrintFloat(wvutilsConvertRainINToMetric(id->loopStore.dayRain), 2));
		break;
	case 22:
		strcpy(store, wvutilsPrintFloat(wvutilsConvertRainINToMetric(id->loopStore.monthRain), 2));
		break;
	case 23:
		strcpy(store, wvutilsPrintFloat(wvutilsConvertRainINToMetric(id->loopStore.stormRain), 2));
		break;
	case 24:
		strcpy(store, wvutilsPrintFloat(wvutilsConvertRainINToMetric(id->loopStore.yearRain), 2));
		break;
	case 25:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetDailyHigh(sensors->sensor, SENSOR_OUTTEMP)));
		break;
	case 26:
		strcpy(store, sensorGetDailyHighTime(sensors->sensor, SENSOR_OUTTEMP, temp));
		break;
	case 27:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetDailyLow(sensors->sensor, SENSOR_OUTTEMP)));
		break;
	case 28:
		strcpy(store, sensorGetDailyLowTime(sensors->sensor, SENSOR_OUTTEMP, temp));
		break;
	case 29:
		sprintf(store, "%d", (int)sensorGetDailyHigh(sensors->sensor, SENSOR_OUTHUMID));
		break;
	case 30:
		strcpy(store, sensorGetDailyHighTime(sensors->sensor, SENSOR_OUTHUMID, temp));
		break;
	case 31:
		sprintf(store, "%d", (int)sensorGetDailyLow(sensors->sensor, SENSOR_OUTHUMID));
		break;
	case 32:
		strcpy(store, sensorGetDailyLowTime(sensors->sensor, SENSOR_OUTHUMID, temp));
		break;
	case 33:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetDailyHigh(sensors->sensor, SENSOR_DEWPOINT)));
		break;
	case 34:
		strcpy(store, sensorGetDailyHighTime(sensors->sensor, SENSOR_DEWPOINT, temp));
		break;
	case 35:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetDailyLow(sensors->sensor, SENSOR_DEWPOINT)));
		break;
	case 36:
		strcpy(store, sensorGetDailyLowTime(sensors->sensor, SENSOR_DEWPOINT, temp));
		break;
	case 37:
		sprintf(store, "%.1f", wvutilsGetWindSpeed(sensorGetDailyHigh(sensors->sensor, SENSOR_WGUST)));
		break;
	case 38:
		strcpy(store, sensorGetDailyHighTime(sensors->sensor, SENSOR_WGUST, temp));
		break;
	case 39:
		sprintf(store, "%.1f", wvutilsConvertINHGToHPA(sensorGetDailyHigh(sensors->sensor, SENSOR_BP)));
		break;
	case 40:
		strcpy(store, sensorGetDailyHighTime(sensors->sensor, SENSOR_BP, temp));
		break;
	case 41:
		sprintf(store, "%.1f", wvutilsConvertINHGToHPA(sensorGetDailyLow(sensors->sensor, SENSOR_BP)));
		break;
	case 42:
		strcpy(store, sensorGetDailyLowTime(sensors->sensor, SENSOR_BP, temp));
		break;
	case 43:
		sprintf(store, "%.2f", wvutilsConvertRainINToMetric(sensorGetDailyHigh(sensors->sensor, SENSOR_RAINRATE)));
		break;
	case 44:
		if (sensorGetDailyHigh(sensors->sensor, SENSOR_RAINRATE) < 0.01)
			strcpy(store, "-----");
		else
			strcpy(store, sensorGetDailyHighTime(sensors->sensor, SENSOR_RAINRATE, temp));
		break;
	case 45:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetDailyLow(sensors->sensor, SENSOR_WCHILL)));
		break;
	case 46:
		strcpy(store, sensorGetDailyLowTime(sensors->sensor, SENSOR_WCHILL, temp));
		break;
	case 47:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetDailyHigh(sensors->sensor, SENSOR_HINDEX)));
		break;
	case 48:
		strcpy(store, sensorGetDailyHighTime(sensors->sensor, SENSOR_HINDEX, temp));
		break;
	case 49:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetHigh(&sensors->sensor[STF_MONTH][SENSOR_OUTTEMP])));
		break;
	case 50:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetLow(&sensors->sensor[STF_MONTH][SENSOR_OUTTEMP])));
		break;
	case 51:
		sprintf(store, "%d", (int)sensorGetHigh(&sensors->sensor[STF_MONTH][SENSOR_OUTHUMID]));
		break;
	case 52:
		sprintf(store, "%d", (int)sensorGetLow(&sensors->sensor[STF_MONTH][SENSOR_OUTHUMID]));
		break;
	case 53:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetHigh(&sensors->sensor[STF_MONTH][SENSOR_DEWPOINT])));
		break;
	case 54:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetLow(&sensors->sensor[STF_MONTH][SENSOR_DEWPOINT])));
		break;
	case 55:
		sprintf(store, "%.1f", wvutilsGetWindSpeed(sensorGetHigh(&sensors->sensor[STF_MONTH][SENSOR_WGUST])));
		break;
	case 56:
		sprintf(store, "%.1f", wvutilsConvertINHGToHPA(sensorGetHigh(&sensors->sensor[STF_MONTH][SENSOR_BP])));
		break;
	case 57:
		sprintf(store, "%.1f", wvutilsConvertINHGToHPA(sensorGetLow(&sensors->sensor[STF_MONTH][SENSOR_BP])));
		break;
	case 58:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetLow(&sensors->sensor[STF_MONTH][SENSOR_WCHILL])));
		break;
	case 59:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetHigh(&sensors->sensor[STF_MONTH][SENSOR_HINDEX])));
		break;
	case 60:
		sprintf(store, "%.2f", wvutilsConvertRainINToMetric(sensorGetHigh(&sensors->sensor[STF_MONTH][SENSOR_RAINRATE])));
		break;
	case 61:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetHigh(&sensors->sensor[STF_YEAR][SENSOR_OUTTEMP])));
		break;
	case 62:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetLow(&sensors->sensor[STF_YEAR][SENSOR_OUTTEMP])));
		break;
	case 63:
		sprintf(store, "%d", (int)sensorGetHigh(&sensors->sensor[STF_YEAR][SENSOR_OUTHUMID]));
		break;
	case 64:
		sprintf(store, "%d", (int)sensorGetLow(&sensors->sensor[STF_YEAR][SENSOR_OUTHUMID]));
		break;
	case 65:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetHigh(&sensors->sensor[STF_YEAR][SENSOR_DEWPOINT])));
		break;
	case 66:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetLow(&sensors->sensor[STF_YEAR][SENSOR_DEWPOINT])));
		break;
	case 67:
		sprintf(store, "%.1f", wvutilsGetWindSpeed(sensorGetHigh(&sensors->sensor[STF_YEAR][SENSOR_WGUST])));
		break;
	case 68:
		sprintf(store, "%.1f", wvutilsConvertINHGToHPA(sensorGetHigh(&sensors->sensor[STF_YEAR][SENSOR_BP])));
		break;
	case 69:
		sprintf(store, "%.1f", wvutilsConvertINHGToHPA(sensorGetLow(&sensors->sensor[STF_YEAR][SENSOR_BP])));
		break;
	case 70:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetLow(&sensors->sensor[STF_YEAR][SENSOR_WCHILL])));
		break;
	case 71:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetHigh(&sensors->sensor[STF_YEAR][SENSOR_HINDEX])));
		break;
	case 72:
		sprintf(store, "%.2f", wvutilsConvertRainINToMetric(sensorGetHigh(&sensors->sensor[STF_YEAR][SENSOR_RAINRATE])));
		break;
	case 73:
		sprintf(store, "%s", globalWviewVersionStr);
		break;
	case 74:
		sprintf(store, "%s", radSystemGetUpTimeSTR(WVIEW_SYSTEM_ID));
		break;
	case 75:
		sprintf(store, "%.1f", (float)id->loopStore.UV);
		break;
	case 76:
		strcpy(store, wvutilsPrintFloat(wvutilsConvertRainINToMetric(id->loopStore.dayET), 2));
		break;
	case 77:
		sprintf(store, "%.0f", (float)id->loopStore.radiation);
		break;
	case 78:
		sprintf(store, "%.1f", wvutilsConvertFToC(id->loopStore.extraTemp[0]));
		break;
	case 79:
		sprintf(store, "%.1f", wvutilsConvertFToC(id->loopStore.extraTemp[1]));
		break;
	case 80:
		sprintf(store, "%.1f", wvutilsConvertFToC(id->loopStore.extraTemp[2]));
		break;
	case 81:
		sprintf(store, "%.1f", wvutilsConvertFToC(id->loopStore.soilTemp1));
		break;
	case 82:
		sprintf(store, "%.1f", wvutilsConvertFToC(id->loopStore.soilTemp2));
		break;
	case 83:
		sprintf(store, "%.1f", wvutilsConvertFToC(id->loopStore.soilTemp3));
		break;
	case 84:
		sprintf(store, "%.1f", wvutilsConvertFToC(id->loopStore.soilTemp4));
		break;
	case 85:
		sprintf(store, "%.1f", wvutilsConvertFToC(id->loopStore.leafTemp1));
		break;
	case 86:
		sprintf(store, "%.1f", wvutilsConvertFToC(id->loopStore.leafTemp2));
		break;
	case 87:
		sprintf(store, "%d", (int)id->loopStore.extraHumidity[0]);
		break;
	case 88:
		sprintf(store, "%d", (int)id->loopStore.extraHumidity[1]);
		break;
	case 89:
		if (id->loopStore.forecastRule <= HTML_MAX_FCAST_RULE && id->ForecastRuleText[id->loopStore.forecastRule] != NULL)
			wvstrncpy(store, id->ForecastRuleText[id->loopStore.forecastRule], HTML_MAX_LINE_LENGTH);
		else
			store[0] = 0;
		break;
	case 90:
		// copy html for icon image
		tempInt = id->loopStore.forecastIcon;
		if (tempInt > 0 && tempInt <= VP_FCAST_ICON_MAX && id->ForecastIconFile[tempInt] != NULL)
		{
			sprintf(store, "<img src=\"%s\">", id->ForecastIconFile[tempInt]);
		}
		else
			store[0] = 0;
		break;
	case 91:
		sprintf(store, "%d m", (int)wvutilsConvertFeetToMeters((int)id->stationElevation));
		break;
	case 92:
		sprintf(store, "%3.1f %c", ((float)abs((int)id->stationLatitude)) / 10.0, ((id->stationLatitude < 0) ? 'S' : 'N'));
		break;
	case 93:
		sprintf(store, "%3.1f %c", ((float)abs((int)id->stationLongitude)) / 10.0, ((id->stationLongitude < 0) ? 'W' : 'E'));
		break;
	case 94:
		sprintf(store, "%.1f", wvutilsConvertFToC(id->loopStore.inTemp));
		break;
	case 95:
		sprintf(store, "%d", id->loopStore.inHumidity);
		break;
	case 96:
		sprintf(store, "%.2f", wvutilsConvertRainINToMetric(sensorGetCumulative(&sensors->sensor[STF_HOUR][SENSOR_RAIN])));
		break;
	case 97:
		sprintf(store, "%.1f km", wvutilsConvertMilesToKilometers(sensorGetWindRun(STF_HOUR, &sensors->sensor[STF_HOUR][SENSOR_WSPEED])));
		break;
	case 98:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_HOUR][SENSOR_OUTTEMP])));
		break;
	case 99:
		sprintf(store, "%.1f", wvutilsGetWindSpeed(sensorGetAvg(&sensors->sensor[STF_HOUR][SENSOR_WSPEED])));
		break;
	case 100:
		sprintf(store, "%d", windAverageCompute(&sensors->wind[STF_HOUR]));
		break;
	case 101:
		sprintf(store, "%.0f", sensorGetAvg(&sensors->sensor[STF_HOUR][SENSOR_OUTHUMID]));
		break;
	case 102:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_HOUR][SENSOR_DEWPOINT])));
		break;
	case 103:
		sprintf(store, "%.1f", wvutilsConvertINHGToHPA(sensorGetAvg(&sensors->sensor[STF_HOUR][SENSOR_BP])));
		break;
	case 104:
		sprintf(store, "%.1f", wvutilsConvertDeltaFToC(id->hilowStore.hourchangetemp));
		break;
	case 105:
		sprintf(store, "%.1f", wvutilsGetWindSpeed(id->hilowStore.hourchangewind));
		break;
	case 106:
		sprintf(store, "%d", id->hilowStore.hourchangewinddir);
		break;
	case 107:
		sprintf(store, "%d", id->hilowStore.hourchangehumid);
		break;
	case 108:
		sprintf(store, "%.1f", wvutilsConvertDeltaFToC(id->hilowStore.hourchangedewpt));
		break;
	case 109:
		sprintf(store, "%.1f", wvutilsConvertINHGToHPA(id->hilowStore.hourchangebarom));
		break;
	case 110:
		sprintf(store, "%.1f km", wvutilsConvertMilesToKilometers(sensorGetWindRun(STF_DAY, &sensors->sensor[STF_DAY][SENSOR_WSPEED])));
		break;
	case 111:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_DAY][SENSOR_OUTTEMP])));
		break;
	case 112:
		sprintf(store, "%.1f", wvutilsGetWindSpeed(sensorGetAvg(&sensors->sensor[STF_DAY][SENSOR_WSPEED])));
		break;
	case 113:
		sprintf(store, "%d", windAverageCompute(&sensors->wind[STF_DAY]));
		break;
	case 114:
		sprintf(store, "%.0f", sensorGetAvg(&sensors->sensor[STF_DAY][SENSOR_OUTHUMID]));
		break;
	case 115:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_DAY][SENSOR_DEWPOINT])));
		break;
	case 116:
		sprintf(store, "%.1f", wvutilsConvertINHGToHPA(sensorGetAvg(&sensors->sensor[STF_DAY][SENSOR_BP])));
		break;
	case 117:
		sprintf(store, "%.1f", wvutilsConvertDeltaFToC(id->hilowStore.daychangetemp));
		break;
	case 118:
		sprintf(store, "%.1f", wvutilsGetWindSpeed(id->hilowStore.daychangewind));
		break;
	case 119:
		sprintf(store, "%d", id->hilowStore.daychangewinddir);
		break;
	case 120:
		sprintf(store, "%d", id->hilowStore.daychangehumid);
		break;
	case 121:
		sprintf(store, "%.1f", wvutilsConvertDeltaFToC(id->hilowStore.daychangedewpt));
		break;
	case 122:
		sprintf(store, "%.1f", wvutilsConvertINHGToHPA(id->hilowStore.daychangebarom));
		break;
	case 123:
		sprintf(store, "%.1f km", wvutilsConvertMilesToKilometers(sensorGetWindRun(STF_WEEK, &sensors->sensor[STF_WEEK][SENSOR_WSPEED])));
		break;
	case 124:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_WEEK][SENSOR_OUTTEMP])));
		break;
	case 125:
		sprintf(store, "%.1f", wvutilsGetWindSpeed(sensorGetAvg(&sensors->sensor[STF_WEEK][SENSOR_WSPEED])));
		break;
	case 126:
		sprintf(store, "%d", windAverageCompute(&sensors->wind[STF_WEEK]));
		break;
	case 127:
		sprintf(store, "%.0f", sensorGetAvg(&sensors->sensor[STF_WEEK][SENSOR_OUTHUMID]));
		break;
	case 128:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_WEEK][SENSOR_DEWPOINT])));
		break;
	case 129:
		sprintf(store, "%.1f", wvutilsConvertINHGToHPA(sensorGetAvg(&sensors->sensor[STF_WEEK][SENSOR_BP])));
		break;
	case 130:
		sprintf(store, "%.1f", wvutilsConvertDeltaFToC(id->hilowStore.weekchangetemp));
		break;
	case 131:
		sprintf(store, "%.1f", wvutilsGetWindSpeed(id->hilowStore.weekchangewind));
		break;
	case 132:
		sprintf(store, "%d", id->hilowStore.weekchangewinddir);
		break;
	case 133:
		sprintf(store, "%d", id->hilowStore.weekchangehumid);
		break;
	case 134:
		sprintf(store, "%.1f", wvutilsConvertDeltaFToC(id->hilowStore.weekchangedewpt));
		break;
	case 135:
		sprintf(store, "%.1f", wvutilsConvertINHGToHPA(id->hilowStore.weekchangebarom));
		break;
	case 136:
		sprintf(store, "%.1f km", wvutilsConvertMilesToKilometers(sensorGetWindRun(STF_MONTH, &sensors->sensor[STF_MONTH][SENSOR_WSPEED])));
		break;
	case 137:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_MONTH][SENSOR_OUTTEMP])));
		break;
	case 138:
		sprintf(store, "%.1f", wvutilsGetWindSpeed(sensorGetAvg(&sensors->sensor[STF_MONTH][SENSOR_WSPEED])));
		break;
	case 139:
		sprintf(store, "%d", windAverageCompute(&sensors->wind[STF_MONTH]));
		break;
	case 140:
		sprintf(store, "%.0f", sensorGetAvg(&sensors->sensor[STF_MONTH][SENSOR_OUTHUMID]));
		break;
	case 141:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_MONTH][SENSOR_DEWPOINT])));
		break;
	case 142:
		sprintf(store, "%.1f", wvutilsConvertINHGToHPA(sensorGetAvg(&sensors->sensor[STF_MONTH][SENSOR_BP])));
		break;
	case 143:
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_MONTH][SENSOR_OUTTEMP], temp, id->dateFormat));
		break;
	case 144:
		strcpy(store, sensorGetLowDate(&sensors->sensor[STF_MONTH][SENSOR_OUTTEMP], temp, id->dateFormat));
		break;
	case 145:
		strcpy(store, sensorGetLowDate(&sensors->sensor[STF_MONTH][SENSOR_WCHILL], temp, id->dateFormat));
		break;
	case 146:
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_MONTH][SENSOR_HINDEX], temp, id->dateFormat));
		break;
	case 147:
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_MONTH][SENSOR_OUTHUMID], temp, id->dateFormat));
		break;
	case 148:
		strcpy(store, sensorGetLowDate(&sensors->sensor[STF_MONTH][SENSOR_OUTHUMID], temp, id->dateFormat));
		break;
	case 149:
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_MONTH][SENSOR_DEWPOINT], temp, id->dateFormat));
		break;
	case 150:
		strcpy(store, sensorGetLowDate(&sensors->sensor[STF_MONTH][SENSOR_DEWPOINT], temp, id->dateFormat));
		break;
	case 151:
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_MONTH][SENSOR_BP], temp, id->dateFormat));
		break;
	case 152:
		strcpy(store, sensorGetLowDate(&sensors->sensor[STF_MONTH][SENSOR_BP], temp, id->dateFormat));
		break;
	case 153:
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_MONTH][SENSOR_WSPEED], temp, id->dateFormat));
		break;
	case 154:
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_MONTH][SENSOR_WGUST], temp, id->dateFormat));
		break;
	case 155:
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_MONTH][SENSOR_RAINRATE], temp, id->dateFormat));
		break;
	case 156:
		sprintf(store, "%.1f km", wvutilsConvertMilesToKilometers(sensorGetWindRun(STF_YEAR, &sensors->sensor[STF_YEAR][SENSOR_WSPEED])));
		break;
	case 157:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_YEAR][SENSOR_OUTTEMP])));
		break;
	case 158:
		sprintf(store, "%.1f", wvutilsGetWindSpeed(sensorGetAvg(&sensors->sensor[STF_YEAR][SENSOR_WSPEED])));
		break;
	case 159:
		sprintf(store, "%d", windAverageCompute(&sensors->wind[STF_YEAR]));
		break;
	case 160:
		sprintf(store, "%.0f", sensorGetAvg(&sensors->sensor[STF_YEAR][SENSOR_OUTHUMID]));
		break;
	case 161:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_YEAR][SENSOR_DEWPOINT])));
		break;
	case 162:
		sprintf(store, "%.1f", wvutilsConvertINHGToHPA(sensorGetAvg(&sensors->sensor[STF_YEAR][SENSOR_BP])));
		break;
	case 163:
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_YEAR][SENSOR_OUTTEMP], temp, id->dateFormat));
		break;
	case 164:
		strcpy(store, sensorGetLowDate(&sensors->sensor[STF_YEAR][SENSOR_OUTTEMP], temp, id->dateFormat));
		break;
	case 165:
		strcpy(store, sensorGetLowDate(&sensors->sensor[STF_YEAR][SENSOR_WCHILL], temp, id->dateFormat));
		break;
	case 166:
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_YEAR][SENSOR_HINDEX], temp, id->dateFormat));
		break;
	case 167:
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_YEAR][SENSOR_OUTHUMID], temp, id->dateFormat));
		break;
	case 168:
		strcpy(store, sensorGetLowDate(&sensors->sensor[STF_YEAR][SENSOR_OUTHUMID], temp, id->dateFormat));
		break;
	case 169:
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_YEAR][SENSOR_DEWPOINT], temp, id->dateFormat));
		break;
	case 170:
		strcpy(store, sensorGetLowDate(&sensors->sensor[STF_YEAR][SENSOR_DEWPOINT], temp, id->dateFormat));
		break;
	case 171:
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_YEAR][SENSOR_BP], temp, id->dateFormat));
		break;
	case 172:
		strcpy(store, sensorGetLowDate(&sensors->sensor[STF_YEAR][SENSOR_BP], temp, id->dateFormat));
		break;
	case 173:
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_YEAR][SENSOR_WSPEED], temp, id->dateFormat));
		break;
	case 174:
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_YEAR][SENSOR_WGUST], temp, id->dateFormat));
		break;
	case 175:
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_YEAR][SENSOR_RAINRATE], temp, id->dateFormat));
		break;
	case 176:
		sprintf(store, "%d", id->loopStore.windDir);
		break;
	case 181:
		sprintf(store, "%.0f", sensorGetDailyHigh(sensors->sensor, SENSOR_SOLRAD));
		break;
	case 182:
		strcpy(store, sensorGetDailyHighTime(sensors->sensor, SENSOR_SOLRAD, temp));
		break;
	case 183:
		sprintf(store, "%.0f", sensorGetHigh(&sensors->sensor[STF_MONTH][SENSOR_SOLRAD]));
		break;
	case 184:
		sprintf(store, "%.0f", sensorGetHigh(&sensors->sensor[STF_YEAR][SENSOR_SOLRAD]));
		break;
	case 185:
		sprintf(store, "%.1f", sensorGetDailyHigh(sensors->sensor, SENSOR_UV));
		break;
	case 186:
		strcpy(store, sensorGetDailyHighTime(sensors->sensor, SENSOR_UV, temp));
		break;
	case 187:
		sprintf(store, "%.1f", sensorGetHigh(&sensors->sensor[STF_MONTH][SENSOR_UV]));
		break;
	case 188:
		sprintf(store, "%.1f", sensorGetHigh(&sensors->sensor[STF_YEAR][SENSOR_UV]));
		break;
	case 189:
		sprintf(store, "%s", lunarPhaseGet(id->mphaseIncrease, id->mphaseDecrease, id->mphaseFull));
		break;
	case 190:
		sprintf(store, " kg/m^3");
		break;
	case 191:
		sprintf(store, "%.3f", wvutilsCalculateAirDensity(id->loopStore.outTemp, id->loopStore.barometer, id->loopStore.dewpoint));
		break;
	case 192:
		strcpy(store, " m");
		break;
	case 193:
		tempfloat = id->loopStore.outTemp;
		tempfloat -= id->loopStore.dewpoint;
		tempfloat *= 228;
		tempfloat = wvutilsConvertFeetToMeters(tempfloat);
		sprintf(store, "%.0f", tempfloat);
		break;
	case 194:
		sprintf(store, "%.0f", (float)id->loopStore.soilMoist1);
		break;
	case 195:
		sprintf(store, "%.0f", (float)id->loopStore.soilMoist2);
		break;
	case 196:
		sprintf(store, "%.0f", (float)id->loopStore.leafWet1);
		break;
	case 197:
		sprintf(store, "%.0f", (float)id->loopStore.leafWet2);
		break;
	case 198:
		// day high wind direction
		tempfloat = sensorGetDailyWhenHigh(sensors->sensor, SENSOR_WGUST) + 11.24;
		tempfloat /= 22.5;
		tempInt = (int)tempfloat;
		tempInt %= 16;
		sprintf(store, "%s", buildWindDirString(tempInt));
		break;
	case 199:
		// baromtrend
		sprintf(store, "%c", BPTrendLabels[id->baromTrendIndicator]);
		break;
	case 200:
		// dailyRainMM
		sprintf(store, "%.1f", wvutilsConvertINToMM(sensorGetCumulative(&id->hilowStore.sensor[STF_DAY][SENSOR_RAIN])));
		break;
	case 201:
		// stationDateMetric
		ntime = time(NULL);
		localtime_r(&ntime, &loctime);
		sprintf(store, "%4.4d%2.2d%2.2d", loctime.tm_year + 1900, loctime.tm_mon + 1, loctime.tm_mday);
		break;
	case 202:
		// middayTime
		strcpy(store, buildTimeTag(id->midday));
		break;
	case 203:
		// dayLength
		strcpy(store, buildTimeTag(id->dayLength));
		break;
	case 204:
		// civilriseTime
		strcpy(store, buildTimeTag(id->civilrise));
		break;
	case 205:
		// civilsetTime
		strcpy(store, buildTimeTag(id->civilset));
		break;
	case 206:
		// astroriseTime
		strcpy(store, buildTimeTag(id->astrorise));
		break;
	case 207:
		// astrosetTime
		strcpy(store, buildTimeTag(id->astroset));
		break;
	case 208:
		// stormStart
		if (id->loopStore.stormStart == (time_t)0)
		{
			strcpy(store, "-------- -----");
			break;
		}
		else
		{
			time_t Time = (time_t)id->loopStore.stormStart;
			localtime_r(&Time, &loctime);
			strftime(store, 64, id->dateFormat, &loctime);
			tempInt = strlen(store);
			snprintf(&store[tempInt], 64, " %2.2d:%2.2d", loctime.tm_hour, loctime.tm_min);
			break;
		}
	case 209:
		// forecastIconFile
		tempInt = id->loopStore.forecastIcon;
		if (tempInt > 0 && tempInt <= VP_FCAST_ICON_MAX && id->ForecastIconFile[tempInt] != NULL)
		{
			sprintf(store, "%s", id->ForecastIconFile[tempInt]);
		}
		else
			store[0] = 0;
		break;
	case 210:
		// rainSeasonStart (month)
		sprintf(store, "%s", monthLabel[id->loopStore.yearRainMonth]);
		break;
	case 211:
		// intervalAvgWindChill
		sprintf(store, "%.1f", wvutilsConvertFToC(id->loopStore.intervalAvgWCHILL));
		break;
	case 212:
		// intervalAvgWindSpeed
		sprintf(store, "%.1f", wvutilsGetWindSpeed(id->loopStore.intervalAvgWSPEEDF));
		break;
	case 213:
		// stationPressure
		sprintf(store, "%.1f", wvutilsConvertINHGToHPA(id->loopStore.stationPressure));
		break;
	case 214:
		// altimeter
		sprintf(store, "%.1f", wvutilsConvertINHGToHPA(id->loopStore.altimeter));
		break;
	case 215:
		// localRadarURL
		wvstrncpy(store, id->radarURL, HTML_MAX_LINE_LENGTH);
		break;
	case 216:
		// localForecastURL
		wvstrncpy(store, id->forecastURL, HTML_MAX_LINE_LENGTH);
		break;
	case 217:
		// windGustSpeed - "Current" wind gust speed
		tempfloat = id->loopStore.windGustF;
		if (tempfloat <= 0)
		{
			tempfloat = 0;
		}
		sprintf(store, "%.1f", wvutilsGetWindSpeed(tempfloat));
		break;
	case 218:
		// windGustDirectionDegrees - "Current" wind gust direction
		sprintf(store, "%.0f", id->loopStore.windGustDir);
		break;
	case 219:
		// windBeaufortScale
		sprintf(store, "%s", wvutilsConvertToBeaufortScale((int)id->loopStore.windSpeedF));
		break;
	case 220:
		// intervalAvgBeaufortScale
		sprintf(store, "%s", wvutilsConvertToBeaufortScale((int)id->loopStore.intervalAvgWSPEEDF));
		break;
	case 221:
		// station type
		strcpy(store, id->stationType);
		break;
	case 233:
		// "<!--rxCheckPercent-->", uint16_t              rxCheckPercent;          0 - 100
		sprintf(store, "%.1f", (float)id->loopStore.rxCheckPercent);
		break;
	case 234:
		// "<!--tenMinuteAvgWindSpeed-->", uint16_t              tenMinuteAvgWindSpeed;  mph
		sprintf(store, "%.1f", wvutilsConvertMPHToKPH((float)id->loopStore.tenMinuteAvgWindSpeed));
		break;
	case 236:
		// "<!--dayWindRoseList-->"
	{
		time_t      nowtime;
		struct tm   bknTime;
		nowtime = time(NULL);
		localtime_r(&nowtime, &bknTime);

		int     numValues = (int)((3600 * bknTime.tm_hour + 60 * bknTime.tm_min + bknTime.tm_sec) / (5 * 60)) + 1;
		float * values = id->windDayValues;
		int     numValuesTotal = DAILY_NUM_VALUES(id);
		int     sampleWidth = /*WR_SAMPLE_WIDTH_DAY*/ 15;
		int     numCounters = 360 / sampleWidth;

		int     i;
		char *  str;
		int     counters[WR_MAX_COUNTERS];

		// populate the counts in each direction
		memset((void *)counters, 0, sizeof counters);
		for (i = numValuesTotal - numValues; i < numValuesTotal; i++) {
			if (values[i] != ARCHIVE_VALUE_NULL) {
				int    bucket;
				bucket = (int)(values[i] + (sampleWidth - 1) / 2) / sampleWidth;
				bucket %= numCounters;
				counters[bucket]++;
			}
		}

		// comma-separated list of counters
		str = store;
		sprintf(str, "[%d", counters[0]);
		for (i = 1; i < numCounters; i++) {
			for (; *str; str++)
				;
			sprintf(str, ",%d", counters[i]);
		}
		sprintf(str, "]");
	}
	break;
	case 237:
		// "<!--txBatteryStatus-->",  uint16_t              txBatteryStatus;         VP only
		sprintf(store, "%2.2x", id->loopStore.txBatteryStatus);
		break;
	case 238:
		// "<!--consBatteryVoltage-->",   uint16_t              consBatteryVoltage;   VP only
		tempfloat = (((float)id->loopStore.consBatteryVoltage * 300) / 512) / 100;
		sprintf(store, "%.2f", tempfloat);
		break;
	case 239:
		ntime = time(NULL);
		// "<!--stationTimeNoSecs-->",
		localtime_r(&ntime, &loctime);
		sprintf(store, "%2.2d:%2.2d", loctime.tm_hour, loctime.tm_min);
		break;
		// ###### Begin EXTRA Wind #######################
	case 246:
		// "<!--windSpeed_ms-->"
		tempfloat = wvutilsConvertMPHToMPS(id->loopStore.windSpeedF);
		sprintf(store, "%.1f", tempfloat);
		break;
	case 247:
		//  "<!--windGustSpeed_ms-->"
		// windGustSpeed - "Current" wind gust speed
		tempfloat = id->loopStore.windGustF;
		if (tempfloat < 0)
		{
			tempfloat = id->loopStore.windSpeedF;
		}
		tempfloat = wvutilsConvertMPHToMPS(tempfloat);
		sprintf(store, "%.1f", tempfloat);
		break;
	case 248:
		// <!--intervalAvgWindSpeed_ms-->
		tempfloat = wvutilsConvertMPHToMPS(id->loopStore.intervalAvgWSPEEDF);
		sprintf(store, "%.1f", tempfloat);
		break;
	case 249:
		// "<!--tenMinuteAvgWindSpeed_ms-->"
		tempfloat = wvutilsConvertMPHToMPS((float)id->loopStore.tenMinuteAvgWindSpeed);
		sprintf(store, "%.1f", tempfloat);
		break;
	case 250:
		// <!--hiDailyWindSpeed_ms-->
		tempfloat = wvutilsConvertMPHToMPS((float)sensorGetDailyHigh(sensors->sensor, SENSOR_WGUST));
		sprintf(store, "%.1f", tempfloat);
		break;
	case 251:
		// <!--hiMonthlyWindSpeed_ms-->
		tempfloat = wvutilsConvertMPHToMPS((float)sensorGetHigh(&sensors->sensor[STF_MONTH][SENSOR_WGUST]));
		sprintf(store, "%.1f", tempfloat);
		break;
	case 252:
		// <!--hiYearlyWindSpeed_ms-->
		tempfloat = wvutilsConvertMPHToMPS((float)sensorGetHigh(&sensors->sensor[STF_YEAR][SENSOR_WGUST]));
		sprintf(store, "%.1f", tempfloat);
		break;
	case 253:
		// <!--houravgwind_ms-->
		tempfloat = wvutilsConvertMPHToMPS((float)sensorGetAvg(&sensors->sensor[STF_HOUR][SENSOR_WSPEED]));
		sprintf(store, "%.1f", tempfloat);
		break;
	case 254:
		// <!--dayavgwind_ms-->
		tempfloat = wvutilsConvertMPHToMPS((float)sensorGetAvg(&sensors->sensor[STF_DAY][SENSOR_WSPEED]));
		sprintf(store, "%.1f", tempfloat);
		break;
	case 255:
		// <!--weekavgwind_ms-->
		tempfloat = wvutilsConvertMPHToMPS((float)sensorGetAvg(&sensors->sensor[STF_WEEK][SENSOR_WSPEED]));
		sprintf(store, "%.1f", tempfloat);
		break;
	case 256:
		// <!--monthtodateavgwind_ms-->
		tempfloat = wvutilsConvertMPHToMPS((float)sensorGetAvg(&sensors->sensor[STF_MONTH][SENSOR_WSPEED]));
		sprintf(store, "%.1f", tempfloat);
		break;
	case 257:
		// <!--yeartodateavgwind_ms-->
		tempfloat = wvutilsConvertMPHToMPS((float)sensorGetAvg(&sensors->sensor[STF_YEAR][SENSOR_WSPEED]));
		sprintf(store, "%.1f", tempfloat);
		break;
	case 258:
		// <!--hourchangewind_ms-->
		tempfloat = wvutilsConvertMPHToMPS((float)id->hilowStore.hourchangewind);
		sprintf(store, "%.1f", tempfloat);
		break;
	case 259:
		// <!--daychangewind_ms-->
		tempfloat = wvutilsConvertMPHToMPS((float)id->hilowStore.daychangewind);
		sprintf(store, "%.1f", tempfloat);
		break;
	case 260:
		// <!--weekchangewind_ms-->
		tempfloat = wvutilsConvertMPHToMPS((float)id->hilowStore.weekchangewind * 0.447027);
		sprintf(store, "%.1f", tempfloat);
		break;
	case 261:
		// "<!--windSpeed_kts-->",
		tempfloat = wvutilsConvertMPHToKnots(id->loopStore.windSpeedF);
		sprintf(store, "%.1f", tempfloat);
		break;
	case 262:
		//  "<!--windGustSpeed_kts-->",
		tempfloat = id->loopStore.windGustF;
		if (tempfloat < 0)
		{
			tempfloat = (id->loopStore.windSpeedF);
		}
		tempfloat = wvutilsConvertMPHToKnots(tempfloat);
		sprintf(store, "%.1f", tempfloat);
		break;
	case 263:
		// <!--intervalAvgWindSpeed_kts-->
		tempfloat = wvutilsConvertMPHToKnots(id->loopStore.intervalAvgWSPEEDF);
		sprintf(store, "%.1f", tempfloat);
		break;
	case 264:
		// "<!--tenMinuteAvgWindSpeed_kts-->"
		tempfloat = wvutilsConvertMPHToKnots((float)(id->loopStore.tenMinuteAvgWindSpeed));
		sprintf(store, "%.1f", tempfloat);
		break;
	case 265:
		// <!--hiWindSpeed_kts-->
		tempfloat = wvutilsConvertMPHToKnots((float)sensorGetDailyHigh(sensors->sensor, SENSOR_WGUST));
		sprintf(store, "%.1f", tempfloat);
		break;
	case 266:
		// <!--hiMonthlyWindSpeed_kts-->
		tempfloat = wvutilsConvertMPHToKnots((float)sensorGetHigh(&sensors->sensor[STF_MONTH][SENSOR_WGUST]));
		sprintf(store, "%.1f", tempfloat);
		break;
	case 267:
		// <!--hiYearlyWindSpeed_kts-->
		tempfloat = wvutilsConvertMPHToKnots((float)sensorGetHigh(&sensors->sensor[STF_YEAR][SENSOR_WGUST]));
		sprintf(store, "%.1f", tempfloat);
		break;
	case 268:
		// <!--houravgwind_kts-->
		tempfloat = wvutilsConvertMPHToKnots((float)sensorGetAvg(&sensors->sensor[STF_HOUR][SENSOR_WSPEED]));
		sprintf(store, "%.1f", tempfloat);
		break;
	case 269:
		// <!--dayavgwind_kts-->
		tempfloat = wvutilsConvertMPHToKnots((float)sensorGetAvg(&sensors->sensor[STF_DAY][SENSOR_WSPEED]));
		sprintf(store, "%.1f", tempfloat);
		break;
	case 270:
		// <!--weekavgwind_kts-->
		tempfloat = wvutilsConvertMPHToKnots((float)sensorGetAvg(&sensors->sensor[STF_WEEK][SENSOR_WSPEED]));
		sprintf(store, "%.1f", tempfloat);
		break;
	case 271:
		// <!--monthtodateavgwind_kts-->
		tempfloat = wvutilsConvertMPHToKnots((float)sensorGetAvg(&sensors->sensor[STF_MONTH][SENSOR_WSPEED]));
		sprintf(store, "%.1f", tempfloat);
		break;
	case 272:
		// <!--yeartodateavgwind_kts-->
		tempfloat = wvutilsConvertMPHToKnots((float)sensorGetAvg(&sensors->sensor[STF_YEAR][SENSOR_WSPEED]));
		sprintf(store, "%.1f", tempfloat);
		break;
	case 273:
		// <!--hourchangewind_kts-->
		tempfloat = wvutilsConvertMPHToKnots((float)(id->hilowStore.hourchangewind));
		sprintf(store, "%.1f", tempfloat);
		break;
	case 274:
		// <!--daychangewind_kts-->
		tempfloat = wvutilsConvertMPHToKnots((float)(id->hilowStore.daychangewind));
		sprintf(store, "%.1f", tempfloat);
		break;
	case 275:
		// <!--weekchangewind_kts-->
		tempfloat = wvutilsConvertMPHToKnots((float)(id->hilowStore.weekchangewind));
		sprintf(store, "%.1f", tempfloat);
		break;
	case 282:
		// "<!--hiAllTimeOutsideTemp-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetHigh(&sensors->sensor[STF_ALL][SENSOR_OUTTEMP])));
		break;
	case 283:
		// "<!--lowAllTimeOutsideTemp-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetLow(&sensors->sensor[STF_ALL][SENSOR_OUTTEMP])));
		break;
	case 284:
		// "<!--hiAllTimeHumidity-->",
		sprintf(store, "%d", (int)sensorGetHigh(&sensors->sensor[STF_ALL][SENSOR_OUTHUMID]));
		break;
	case 285:
		// "<!--lowAllTimeHumidity-->",
		sprintf(store, "%d", (int)sensorGetLow(&sensors->sensor[STF_ALL][SENSOR_OUTHUMID]));
		break;
	case 286:
		// "<!--hiAllTimeDewpoint-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetHigh(&sensors->sensor[STF_ALL][SENSOR_DEWPOINT])));
		break;
	case 287:
		// "<!--lowAllTimeDewpoint-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetLow(&sensors->sensor[STF_ALL][SENSOR_DEWPOINT])));
		break;
	case 288:
		// "<!--hiAllTimeWindSpeed-->",
		sprintf(store, "%.1f", wvutilsGetWindSpeed(sensorGetHigh(&sensors->sensor[STF_ALL][SENSOR_WGUST])));
		break;
	case 289:
		// "<!--hiAllTimeBarometer-->",
		sprintf(store, "%.1f", wvutilsConvertINHGToHPA(sensorGetHigh(&sensors->sensor[STF_ALL][SENSOR_BP])));
		break;
	case 290:
		// "<!--lowAllTimeBarometer-->",
		sprintf(store, "%.1f", wvutilsConvertINHGToHPA(sensorGetLow(&sensors->sensor[STF_ALL][SENSOR_BP])));
		break;
	case 291:
		// "<!--lowAllTimeWindchill-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetLow(&sensors->sensor[STF_ALL][SENSOR_WCHILL])));
		break;
	case 292:
		// "<!--hiAllTimeHeatindex-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetHigh(&sensors->sensor[STF_ALL][SENSOR_HINDEX])));
		break;
	case 293:
		// "<!--hiAllTimeRainRate-->",
		sprintf(store, "%.2f", wvutilsConvertRainINToMetric(sensorGetHigh(&sensors->sensor[STF_ALL][SENSOR_RAINRATE])));
		break;
	case 294:
		// "<!--hiAllTimeRadiation-->",
		sprintf(store, "%.0f", sensorGetHigh(&sensors->sensor[STF_ALL][SENSOR_SOLRAD]));
		break;
	case 295:
		// "<!--hiAllTimeUV-->",
		sprintf(store, "%.1f", sensorGetHigh(&sensors->sensor[STF_ALL][SENSOR_UV]));
		break;
	case 296:
		// "<!--alltimeavgtemp-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_ALL][SENSOR_OUTTEMP])));
		break;
	case 297:
		// "<!--alltimeavgwind-->",
		sprintf(store, "%.1f", wvutilsGetWindSpeed(sensorGetAvg(&sensors->sensor[STF_ALL][SENSOR_WSPEED])));
		break;
	case 298:
		// "<!--alltimedomwinddir-->",
		sprintf(store, "%d", windAverageCompute(&sensors->wind[STF_ALL]));
		break;
	case 299:
		// "<!--alltimeavghumid-->",
		sprintf(store, "%.0f", sensorGetAvg(&sensors->sensor[STF_ALL][SENSOR_OUTHUMID]));
		break;
	case 300:
		// "<!--alltimeavgdewpt-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_ALL][SENSOR_DEWPOINT])));
		break;
	case 301:
		// "<!--alltimeavgbarom-->",
		sprintf(store, "%.1f", wvutilsConvertINHGToHPA(sensorGetAvg(&sensors->sensor[STF_ALL][SENSOR_BP])));
		break;
	case 302:
		// "<!--alltimemaxtempdate-->",
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_ALL][SENSOR_OUTTEMP], temp, id->dateFormat));
		break;
	case 303:
		// "<!--alltimemintempdate-->",
		strcpy(store, sensorGetLowDate(&sensors->sensor[STF_ALL][SENSOR_OUTTEMP], temp, id->dateFormat));
		break;
	case 304:
		// "<!--alltimeminchilldate-->",
		strcpy(store, sensorGetLowDate(&sensors->sensor[STF_ALL][SENSOR_WCHILL], temp, id->dateFormat));
		break;
	case 305:
		// "<!--alltimemaxheatdate-->",
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_ALL][SENSOR_HINDEX], temp, id->dateFormat));
		break;
	case 306:
		// "<!--alltimemaxhumiddate-->",
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_ALL][SENSOR_OUTHUMID], temp, id->dateFormat));
		break;
	case 307:
		// "<!--alltimeminhumiddate-->",
		strcpy(store, sensorGetLowDate(&sensors->sensor[STF_ALL][SENSOR_OUTHUMID], temp, id->dateFormat));
		break;
	case 308:
		// "<!--alltimemaxdewptdate-->",
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_ALL][SENSOR_DEWPOINT], temp, id->dateFormat));
		break;
	case 309:
		// "<!--alltimemindewptdate-->",
		strcpy(store, sensorGetLowDate(&sensors->sensor[STF_ALL][SENSOR_DEWPOINT], temp, id->dateFormat));
		break;
	case 310:
		// "<!--alltimemaxbaromdate-->",
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_ALL][SENSOR_BP], temp, id->dateFormat));
		break;
	case 311:
		// "<!--alltimeminbaromdate-->",
		strcpy(store, sensorGetLowDate(&sensors->sensor[STF_ALL][SENSOR_BP], temp, id->dateFormat));
		break;
	case 312:
		// "<!--alltimemaxwinddate-->",
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_ALL][SENSOR_WSPEED], temp, id->dateFormat));
		break;
	case 313:
		// "<!--alltimemaxgustdate-->",
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_ALL][SENSOR_WGUST], temp, id->dateFormat));
		break;
	case 314:
		// "<!--alltimemaxrainratedate-->",
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_ALL][SENSOR_RAINRATE], temp, id->dateFormat));
		break;
	case 316:
		// "<!--stationName-->",
		wvstrncpy(store, id->stationName, HTML_MAX_LINE_LENGTH);
		break;
	case 317:
		// "<!--moonriseTime-->",
		if (id->moonrise == -2 && id->moonset == -2)
			sprintf(store, "Down all day");
		else if (id->moonrise == -1 && id->moonset == -1)
			sprintf(store, "Up all day");
		else if (id->moonrise > 0)
			sprintf(store, "%2.2d:%2.2d", id->moonrise / 100, id->moonrise % 100);
		else
			sprintf(store, "--:--"); // No Moon Rise
		break;
	case 318:
		// "<!--moonsetTime-->",
		if (id->moonrise == -2 && id->moonset == -2)
			sprintf(store, "Down all day");
		else if (id->moonrise == -1 && id->moonset == -1)
			sprintf(store, "Up all day");
		else if (id->moonset > 0)
			sprintf(store, "%2.2d:%2.2d", id->moonset / 100, id->moonset % 100);
		else
			sprintf(store, "--:--"); // No Moon Set
		break;
	case 319:
		// "<!--apparentTemp-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(wvutilsCalculateApparentTemp(id->loopStore.outTemp, id->loopStore.windSpeedF, id->loopStore.outHumidity)));
		break;
	case 320:
		// "<!--genExtraTemp1-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(id->loopStore.extraTemp[0]));
		break;
	case 321:
		// "<!--genExtraTemp2-->",
		sprintf(store, "%.1f", id->loopStore.extraTemp[1]);
		break;
	case 322:
		// "<!--genExtraTemp3-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(id->loopStore.extraTemp[2]));
		break;
	case 323:
		// "<!--genExtraTemp4-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(id->loopStore.extraTemp[3]));
		break;
	case 324:
		// "<!--genExtraTemp5-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(id->loopStore.extraTemp[4]));
		break;
	case 325:
		// "<!--genExtraTemp6-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(id->loopStore.extraTemp[5]));
		break;
	case 326:
		// "<!--genExtraTemp7-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(id->loopStore.extraTemp[6]));
		break;
	case 327:
		// "<!--genExtraTemp8-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(id->loopStore.extraTemp[7]));
		break;
	case 328:
		// "<!--genExtraTemp9-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(id->loopStore.extraTemp[8]));
		break;
	case 329:
		// "<!--genExtraTemp10-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(id->loopStore.extraTemp[9]));
		break;
	case 330:
		// "<!--genExtraTemp11-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(id->loopStore.extraTemp[10]));
		break;
	case 331:
		// "<!--genExtraTemp12-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(id->loopStore.extraTemp[11]));
		break;
	case 332:
		// "<!--genExtraTemp13-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(id->loopStore.extraTemp[12]));
		break;
	case 333:
		// "<!--genExtraTemp14-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(id->loopStore.extraTemp[13]));
		break;
	case 334:
		// "<!--genExtraTemp15-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(id->loopStore.extraTemp[14]));
		break;
	case 335:
		// "<!--genExtraTemp16-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(id->loopStore.extraTemp[15]));
		break;
	case 336:
		// "<!--genExtraHumidity1-->",
		sprintf(store, "%d", id->loopStore.extraHumidity[0]);
		break;
	case 337:
		// "<!--genExtraHumidity2-->",
		sprintf(store, "%d", id->loopStore.extraHumidity[1]);
		break;
	case 374:
		// "<!--WetBulbTemp-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(wvutilsCalculateWetBulbTemp(id->loopStore.outTemp, id->loopStore.outHumidity, id->loopStore.barometer)));
		break;
	case 375:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetDailyHigh(sensors->sensor, SENSOR_EXTRATEMP1)));
		break;
	case 376:
		strcpy(store, sensorGetDailyHighTime(sensors->sensor, SENSOR_EXTRATEMP1, temp));
		break;
	case 377:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetDailyLow(sensors->sensor, SENSOR_EXTRATEMP1)));
		break;
	case 378:
		strcpy(store, sensorGetDailyLowTime(sensors->sensor, SENSOR_EXTRATEMP1, temp));
		break;
	case 379:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_HOUR][SENSOR_EXTRATEMP1])));
		break;
	case 380:
		sprintf(store, "%.1f", wvutilsConvertDeltaFToC(id->hilowStore.hourchangeatemp));
		break;
	case 381:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_WEEK][SENSOR_EXTRATEMP1])));
		break;
	case 382:
		sprintf(store, "%.1f", wvutilsConvertDeltaFToC(id->hilowStore.weekchangeatemp));
		break;
	case 383:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_DAY][SENSOR_EXTRATEMP1])));
		break;
	case 384:
		sprintf(store, "%.1f", wvutilsConvertDeltaFToC(id->hilowStore.daychangeatemp));
		break;
	case 385:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_MONTH][SENSOR_EXTRATEMP1])));
		break;
	case 386:
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_MONTH][SENSOR_EXTRATEMP1], temp, id->dateFormat));
		break;
	case 387:
		strcpy(store, sensorGetLowDate(&sensors->sensor[STF_MONTH][SENSOR_EXTRATEMP1], temp, id->dateFormat));
		break;
	case 388:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_YEAR][SENSOR_EXTRATEMP1])));
		break;
	case 389:
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_YEAR][SENSOR_EXTRATEMP1], temp, id->dateFormat));
		break;
	case 390:
		strcpy(store, sensorGetLowDate(&sensors->sensor[STF_YEAR][SENSOR_EXTRATEMP1], temp, id->dateFormat));
		break;
	case 391:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_ALL][SENSOR_EXTRATEMP1])));
		break;
	case 392:
		// "<!--alltimemaxatempdate-->",
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_ALL][SENSOR_EXTRATEMP1], temp, id->dateFormat));
		break;
	case 393:
		// "<!--alltimeminatempdate-->",
		strcpy(store, sensorGetLowDate(&sensors->sensor[STF_ALL][SENSOR_EXTRATEMP1], temp, id->dateFormat));
		break;
	case 394:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetHigh(&sensors->sensor[STF_MONTH][SENSOR_EXTRATEMP1])));
		break;
	case 395:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetLow(&sensors->sensor[STF_MONTH][SENSOR_EXTRATEMP1])));
		break;
	case 396:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetHigh(&sensors->sensor[STF_YEAR][SENSOR_EXTRATEMP1])));
		break;
	case 397:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetLow(&sensors->sensor[STF_YEAR][SENSOR_EXTRATEMP1])));
		break;
	case 398:
		// "<!--hiAllTimeOutsideTemp-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetHigh(&sensors->sensor[STF_ALL][SENSOR_EXTRATEMP1])));
		break;
	case 399:
		// "<!--lowAllTimeOutsideTemp-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetLow(&sensors->sensor[STF_ALL][SENSOR_EXTRATEMP1])));
		break;
	case 400:
		//"<!--hisoilTemp1-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetDailyHigh(sensors->sensor, SENSOR_SOILTEMP1)));
		break;
	case 401:
		//	"<!--hisoilTemp1Time-->",
		strcpy(store, sensorGetDailyHighTime(sensors->sensor, SENSOR_SOILTEMP1, temp));
		break;
	case 402:
		//	"<!--lowsoilTemp1-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetDailyLow(sensors->sensor, SENSOR_SOILTEMP1)));
		break;
	case 403:
		//	"<!--lowsoilTemp1Time-->",
		strcpy(store, sensorGetDailyLowTime(sensors->sensor, SENSOR_SOILTEMP1, temp));
		break;
	case 404:
		//	"<!--hisoilMoist1-->",
		sprintf(store, "%d", (int)sensorGetDailyHigh(sensors->sensor, SENSOR_SOILMOIST1));
		break;
	case 405:
		//	"<!--hisoilMoist1Time-->",
		strcpy(store, sensorGetDailyHighTime(sensors->sensor, SENSOR_SOILMOIST1, temp));
		break;
	case 406:
		//	"<!--lowsoilMoist1-->",
		sprintf(store, "%d", (int)sensorGetDailyLow(sensors->sensor, SENSOR_SOILMOIST1));
		break;
	case 407:
		//	"<!--lowsoilMoist1Time-->",
		strcpy(store, sensorGetDailyLowTime(sensors->sensor, SENSOR_SOILMOIST1, temp));
		break;
	case 408:
		//	"<!--hileafWet1->",
		sprintf(store, "%d", (int)sensorGetDailyHigh(sensors->sensor, SENSOR_LEAFWET1));
		break;
	case 409:
		//	"<!--hileafWet1Time-->",
		strcpy(store, sensorGetDailyHighTime(sensors->sensor, SENSOR_LEAFWET1, temp));
		break;
	case 410:
		//	"<!--lowleafWet1-->",
		sprintf(store, "%d", (int)sensorGetDailyLow(sensors->sensor, SENSOR_LEAFWET1));
		break;
	case 411:
		//	"<!--lowleafWet1Time-->",
		strcpy(store, sensorGetDailyLowTime(sensors->sensor, SENSOR_LEAFWET1, temp));
		break;
	case 412:
		//	"<!--houravgsoilTemp1-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_HOUR][SENSOR_SOILTEMP1])));
		break;
	case 413:
		//	"<!--hourchangesoilTemp1-->",
		sprintf(store, "%.1f", wvutilsConvertDeltaFToC(id->hilowStore.hourchangesoiltemp1));
		break;
	case 414:
		//	"<!--weekavgsoilTemp1-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_WEEK][SENSOR_SOILTEMP1])));
		break;
	case 415:
		//	"<!--weekchangesoilTemp1-->",
		sprintf(store, "%.1f", wvutilsConvertDeltaFToC(id->hilowStore.weekchangesoiltemp1));
		break;
	case 416:
		//	"<!--dayavgsoilTemp1-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_DAY][SENSOR_SOILTEMP1])));
		break;
	case 417:
		//	"<!--daychangesoilTemp1-->",
		sprintf(store, "%.1f", wvutilsConvertDeltaFToC(id->hilowStore.daychangesoiltemp1));
		break;
	case 418:
		//	"<!--monthtodateavgsoilTemp1-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_MONTH][SENSOR_SOILTEMP1])));
		break;
	case 419:
		//	"<!--monthtodatemaxsoilTemp1date-->",
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_MONTH][SENSOR_SOILTEMP1], temp, id->dateFormat));
		break;
	case 420:
		//	"<!--monthtodateminsoilTemp1date-->",
		strcpy(store, sensorGetLowDate(&sensors->sensor[STF_MONTH][SENSOR_SOILTEMP1], temp, id->dateFormat));
		break;
	case 421:
		//	"<!--yeartodateavgsoilTemp1-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_YEAR][SENSOR_SOILTEMP1])));
		break;
	case 422:
		//	"<!--yeartodatemaxsoilTemp1date-->",
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_YEAR][SENSOR_SOILTEMP1], temp, id->dateFormat));
		break;
	case 423:
		//	"<!--yeartodateminsoilTemp1date-->",
		strcpy(store, sensorGetLowDate(&sensors->sensor[STF_YEAR][SENSOR_SOILTEMP1], temp, id->dateFormat));
		break;
	case 424:
		//	"<!--alltimeavgsoilTemp1-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_ALL][SENSOR_SOILTEMP1])));
		break;
	case 425:
		//	"<!--alltimemaxsoilTemp1date-->",
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_ALL][SENSOR_SOILTEMP1], temp, id->dateFormat));
		break;
	case 426:
		//	"<!--alltimeminsoilTemp1date-->",
		strcpy(store, sensorGetLowDate(&sensors->sensor[STF_ALL][SENSOR_SOILTEMP1], temp, id->dateFormat));
		break;
	case 427:
		//	"<!--hiMonthlysoilTemp1-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetHigh(&sensors->sensor[STF_MONTH][SENSOR_SOILTEMP1])));
		break;
	case 428:
		//	"<!--lowMonthlysoilTemp1-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetLow(&sensors->sensor[STF_MONTH][SENSOR_SOILTEMP1])));
		break;
	case 429:
		//	"<!--hiYearlysoilTemp1-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetHigh(&sensors->sensor[STF_YEAR][SENSOR_SOILTEMP1])));
		break;
	case 430:
		//	"<!--lowYearlysoilTemp1-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetLow(&sensors->sensor[STF_YEAR][SENSOR_SOILTEMP1])));
		break;
	case 431:
		//	"<!--hiAllTimesoilTemp1-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetHigh(&sensors->sensor[STF_ALL][SENSOR_SOILTEMP1])));
		break;
	case 432:
		//	"<!--lowAllTimesoilTemp1-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetLow(&sensors->sensor[STF_ALL][SENSOR_SOILTEMP1])));
		break;
	case 433:
		//	"<!--houravgsoilMoist1-->",
		sprintf(store, "%.1f", sensorGetAvg(&sensors->sensor[STF_HOUR][SENSOR_SOILMOIST1]));
		break;
	case 434:
		//	"<!--hourchangesoilMoist1-->",
		sprintf(store, "%d", (int)id->hilowStore.hourchangesoilmoist1);
		break;
	case 435:
		//	"<!--weekavgsoilMoist1-->",
		sprintf(store, "%.1f", sensorGetAvg(&sensors->sensor[STF_WEEK][SENSOR_SOILMOIST1]));
		break;
	case 436:
		//	"<!--weekchangesoilMoist1-->",
		sprintf(store, "%d", (int)id->hilowStore.weekchangesoilmoist1);
		break;
	case 437:
		//	"<!--dayavgsoilMoist1-->",
		sprintf(store, "%.1f", sensorGetAvg(&sensors->sensor[STF_DAY][SENSOR_SOILMOIST1]));
		break;
	case 438:
		//	"<!--daychangesoilMoist1-->",
		sprintf(store, "%d", (int)id->hilowStore.daychangesoilmoist1);
		break;
	case 439:
		//	"<!--monthtodateavgsoilMoist1-->",
		sprintf(store, "%.1f", sensorGetAvg(&sensors->sensor[STF_MONTH][SENSOR_SOILMOIST1]));
		break;
	case 440:
		//	"<!--monthtodatemaxsoilMoist1date-->",
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_MONTH][SENSOR_SOILMOIST1], temp, id->dateFormat));
		break;
	case 441:
		//	"<!--monthtodateminsoilMoist1date-->",
		strcpy(store, sensorGetLowDate(&sensors->sensor[STF_MONTH][SENSOR_SOILMOIST1], temp, id->dateFormat));
		break;
	case 442:
		//	"<!--yeartodateavgsoilMoist1-->",
		sprintf(store, "%.1f", sensorGetAvg(&sensors->sensor[STF_YEAR][SENSOR_SOILMOIST1]));
		break;
	case 443:
		//	"<!--yeartodatemaxsoilMoist1date-->",
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_YEAR][SENSOR_SOILMOIST1], temp, id->dateFormat));
		break;
	case 444:
		//	"<!--yeartodatemintempdate-->",
		strcpy(store, sensorGetLowDate(&sensors->sensor[STF_YEAR][SENSOR_SOILMOIST1], temp, id->dateFormat));
		break;
	case 445:
		//	"<!--alltimeavgsoilMoist1-->",
		sprintf(store, "%.1f", sensorGetAvg(&sensors->sensor[STF_ALL][SENSOR_SOILMOIST1]));
		break;
	case 446:
		//	"<!--alltimemaxsoilMoist1date-->",
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_ALL][SENSOR_SOILMOIST1], temp, id->dateFormat));
		break;
	case 447:
		//	"<!--alltimeminsoilMoist1date-->",
		strcpy(store, sensorGetLowDate(&sensors->sensor[STF_ALL][SENSOR_SOILMOIST1], temp, id->dateFormat));
		break;
	case 448:
		//	"<!--hiMonthlysoilMoist1-->",
		sprintf(store, "%d", (int)sensorGetHigh(&sensors->sensor[STF_MONTH][SENSOR_SOILMOIST1]));
		break;
	case 449:
		//	"<!--lowMonthlysoilMoist1-->",
		sprintf(store, "%d", (int)sensorGetLow(&sensors->sensor[STF_MONTH][SENSOR_SOILMOIST1]));
		break;
	case 450:
		//	"<!--hiYearlysoilMoist1-->",
		sprintf(store, "%d", (int)sensorGetHigh(&sensors->sensor[STF_YEAR][SENSOR_SOILMOIST1]));
		break;
	case 451:
		//	"<!--lowYearlysoilMoist1-->",
		sprintf(store, "%d", (int)sensorGetLow(&sensors->sensor[STF_YEAR][SENSOR_SOILMOIST1]));
		break;
	case 452:
		//	"<!--hiAllTimesoilMoist1-->",
		sprintf(store, "%d", (int)sensorGetHigh(&sensors->sensor[STF_ALL][SENSOR_SOILMOIST1]));
		break;
	case 453:
		//	"<!--lowAllTimesoilMoist1-->",
		sprintf(store, "%d", (int)sensorGetLow(&sensors->sensor[STF_ALL][SENSOR_SOILMOIST1]));
		break;
	case 454:
		//	"<!--houravgleafWet1-->",
		sprintf(store, "%.1f", sensorGetAvg(&sensors->sensor[STF_HOUR][SENSOR_LEAFWET1]));
		break;
	case 455:
		//	"<!--hourchangeleafWet1-->",
		sprintf(store, "%d", (int)id->hilowStore.hourchangeleafwet1);
		break;
	case 456:
		//	"<!--weekavgleafWet1-->",
		sprintf(store, "%.1f", sensorGetAvg(&sensors->sensor[STF_WEEK][SENSOR_LEAFWET1]));
		break;
	case 457:
		//	"<!--weekchangeleafWet1-->",
		sprintf(store, "%d", (int)id->hilowStore.weekchangeleafwet1);
		break;
	case 458:
		//	"<!--dayavgleafWet1-->",
		sprintf(store, "%.1f", sensorGetAvg(&sensors->sensor[STF_DAY][SENSOR_LEAFWET1]));
		break;
	case 459:
		//	"<!--daychangeleafWet1-->",
		sprintf(store, "%d", id->hilowStore.daychangeleafwet1);
		break;
	case 460:
		//	"<!--monthtodateavgleafWet1-->",
		sprintf(store, "%.1f", sensorGetAvg(&sensors->sensor[STF_MONTH][SENSOR_LEAFWET1]));
		break;
	case 461:
		//	"<!--monthtodatemaxleafWet1date-->",
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_MONTH][SENSOR_LEAFWET1], temp, id->dateFormat));
		break;
	case 462:
		//	"<!--monthtodateminleafWet1date-->",
		strcpy(store, sensorGetLowDate(&sensors->sensor[STF_MONTH][SENSOR_LEAFWET1], temp, id->dateFormat));
		break;
	case 463:
		//	"<!--yeartodateavgleafWet1-->",
		sprintf(store, "%.1f", sensorGetAvg(&sensors->sensor[STF_YEAR][SENSOR_LEAFWET1]));
		break;
	case 464:
		//	"<!--yeartodatemaxleafWet1date-->",
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_YEAR][SENSOR_LEAFWET1], temp, id->dateFormat));
		break;
	case 465:
		//	"<!--yeartodatemintempdate-->",
		strcpy(store, sensorGetLowDate(&sensors->sensor[STF_YEAR][SENSOR_LEAFWET1], temp, id->dateFormat));
		break;
	case 466:
		//	"<!--alltimeavgleafWet1-->",
		sprintf(store, "%.1f", sensorGetAvg(&sensors->sensor[STF_ALL][SENSOR_LEAFWET1]));
		break;
	case 467:
		//	"<!--alltimemaxleafWet1date-->",
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_ALL][SENSOR_LEAFWET1], temp, id->dateFormat));
		break;
	case 468:
		//	"<!--alltimeminleafWet1date-->",
		strcpy(store, sensorGetLowDate(&sensors->sensor[STF_ALL][SENSOR_LEAFWET1], temp, id->dateFormat));
		break;
	case 469:
		//	"<!--hiMonthlyleafWet1-->",
		sprintf(store, "%d", (int)sensorGetHigh(&sensors->sensor[STF_MONTH][SENSOR_LEAFWET1]));
		break;
	case 470:
		//	"<!--lowMonthlyleafWet1-->",
		sprintf(store, "%d", (int)sensorGetLow(&sensors->sensor[STF_MONTH][SENSOR_LEAFWET1]));
		break;
	case 471:
		//	"<!--hiYearlyleafWet1-->",
		sprintf(store, "%d", (int)sensorGetHigh(&sensors->sensor[STF_YEAR][SENSOR_LEAFWET1]));
		break;
	case 472:
		//	"<!--lowYearlyleafWet1-->",
		sprintf(store, "%d", (int)sensorGetLow(&sensors->sensor[STF_YEAR][SENSOR_LEAFWET1]));
		break;
	case 473:
		//	"<!--hiAllTimeleafWet1-->",
		sprintf(store, "%d", (int)sensorGetHigh(&sensors->sensor[STF_ALL][SENSOR_LEAFWET1]));
		break;
	case 474:
		//	"<!--lowAllTimeleafWet1-->",
		sprintf(store, "%d", (int)sensorGetLow(&sensors->sensor[STF_ALL][SENSOR_LEAFWET1]));
		break;
	case 475:
		//	"<!--hourET-->",
		sprintf(store, "%.2f", wvutilsConvertRainINToMetric(sensorGetCumulative(&sensors->sensor[STF_HOUR][SENSOR_ET])));
		break;
	case 476:
		// "<!--weekET-->",
		sprintf(store, "%.2f", wvutilsConvertRainINToMetric(sensorGetCumulative(&sensors->sensor[STF_WEEK][SENSOR_ET])));
		break;
	case 477:
		// 	"<!--monthET-->",
		strcpy(store, wvutilsPrintFloat(wvutilsConvertRainINToMetric(id->loopStore.monthET), 2));
		break;
	case 478:
		// 	"<!--yearET-->",
		strcpy(store, wvutilsPrintFloat(wvutilsConvertRainINToMetric(id->loopStore.yearET), 2));
		break;
	case 479:
		//	"<!--weekrain-->",
		sprintf(store, "%.2f", wvutilsConvertRainINToMetric(sensorGetCumulative(&sensors->sensor[STF_WEEK][SENSOR_RAIN])));
		break;
	case 480:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetDailyHigh(sensors->sensor, SENSOR_EXTRATEMP2)));
		break;
	case 481:
		strcpy(store, sensorGetDailyHighTime(sensors->sensor, SENSOR_EXTRATEMP2, temp));
		break;
	case 482:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetDailyLow(sensors->sensor, SENSOR_EXTRATEMP2)));
		break;
	case 483:
		strcpy(store, sensorGetDailyLowTime(sensors->sensor, SENSOR_EXTRATEMP2, temp));
		break;
	case 484:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_HOUR][SENSOR_EXTRATEMP2])));
		break;
	case 485:
		sprintf(store, "%.1f", wvutilsConvertDeltaFToC(id->hilowStore.hourchangeatemp1));
		break;
	case 486:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_WEEK][SENSOR_EXTRATEMP2])));
		break;
	case 487:
		sprintf(store, "%.1f", wvutilsConvertDeltaFToC(id->hilowStore.weekchangeatemp1));
		break;
	case 488:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_DAY][SENSOR_EXTRATEMP2])));
		break;
	case 489:
		sprintf(store, "%.1f", wvutilsConvertDeltaFToC(id->hilowStore.daychangeatemp1));
		break;
	case 490:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_MONTH][SENSOR_EXTRATEMP2])));
		break;
	case 491:
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_MONTH][SENSOR_EXTRATEMP2], temp, id->dateFormat));
		break;
	case 492:
		strcpy(store, sensorGetLowDate(&sensors->sensor[STF_MONTH][SENSOR_EXTRATEMP2], temp, id->dateFormat));
		break;
	case 493:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_YEAR][SENSOR_EXTRATEMP2])));
		break;
	case 494:
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_YEAR][SENSOR_EXTRATEMP2], temp, id->dateFormat));
		break;
	case 495:
		strcpy(store, sensorGetLowDate(&sensors->sensor[STF_YEAR][SENSOR_EXTRATEMP2], temp, id->dateFormat));
		break;
	case 496:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetAvg(&sensors->sensor[STF_ALL][SENSOR_EXTRATEMP2])));
		break;
	case 497:
		// "<!--alltimemaxatempdate-->",
		strcpy(store, sensorGetHighDate(&sensors->sensor[STF_ALL][SENSOR_EXTRATEMP2], temp, id->dateFormat));
		break;
	case 498:
		// "<!--alltimeminatempdate-->",
		strcpy(store, sensorGetLowDate(&sensors->sensor[STF_ALL][SENSOR_EXTRATEMP2], temp, id->dateFormat));
		break;
	case 499:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetHigh(&sensors->sensor[STF_MONTH][SENSOR_EXTRATEMP2])));
		break;
	case 500:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetLow(&sensors->sensor[STF_MONTH][SENSOR_EXTRATEMP2])));
		break;
	case 501:
		sprintf(store, "%.1f",
			wvutilsConvertFToC(sensorGetHigh(&sensors->sensor[STF_YEAR][SENSOR_EXTRATEMP2])));
		break;
	case 502:
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetLow(&sensors->sensor[STF_YEAR][SENSOR_EXTRATEMP2])));
		break;
	case 503:
		// "<!--hiAllTimeOutsideTemp-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetHigh(&sensors->sensor[STF_ALL][SENSOR_EXTRATEMP2])));
		break;
	case 504:
		// "<!--lowAllTimeOutsideTemp-->",
		sprintf(store, "%.1f", wvutilsConvertFToC(sensorGetLow(&sensors->sensor[STF_ALL][SENSOR_EXTRATEMP2])));
		break;
	case 505:
	{
		time_t      nowtime;
		struct tm   bknTime;
		nowtime = time(NULL);
		localtime_r(&nowtime, &bknTime);

		int     numValues = (int)((3600 * bknTime.tm_hour + 60 * bknTime.tm_min + bknTime.tm_sec) / (5 * 60)) + 1;
		float * values = id->windDayValues;
		int     numValuesTotal = (int)((3600 * 24) / (5 * 60));

		int     i;
		float	sum = 0;
		int numsum = 0;

		for (i = numValuesTotal - numValues; i < numValuesTotal; i++) {
			if (values[i] != ARCHIVE_VALUE_NULL)
			{
				sum = sum + (float)values[i];
				++numsum;
			}
		}
		sum = sum / (float)numsum;
		sprintf(store, "%d", (int)sum);
	}
	break;
	case 506:
		sprintf(store, "%.1f", wvutilsConvertMPHToKPH((float)id->loopStore.twoMinuteAvgWindSpeed));
		break;
	case 507:
		sprintf(store, "%.1f", wvutilsConvertMPHToKPH((float)id->loopStore.tenMinuteWindGust));
		break;
	case 508:
		sprintf(store, "%d", id->loopStore.WinddirtenMinuteWindGust);
		break;
	default:
		store[0] = 0;
	}

	return;
}

static int replaceDataTags(HTML_MGR_ID id, char* oldline, char* newline)
{
	register int    i, j, k;
	int             len, taglen, found;
	char            temp[HTML_MAX_LINE_LENGTH];

	newline[0] = 0;

	for (i = 0, j = 0; oldline[i] != 0; i++)
	{
		if (!strncmp(&oldline[i], "<!--", 4))
		{
			// get tag length
			found = FALSE;
			for (k = i, taglen = 1; oldline[k] != 0; k++, taglen++)
			{
				if (oldline[k] == '>')
				{
					found = TRUE;
					break;
				}
			}

			if (found)
			{
				// first check for file inclusion
				if (!strncmp(&oldline[i], "<!--include ", 12))
				{
					// we must include an external file here:
					// only copy the include file name, no tag delimiters,
					// then return TRUE indicating a file should be included;
					// we ignore any other tags after a file inclusion tag...
					strncpy(newline, &oldline[i + 12], taglen - 15);
					newline[taglen - 15] = 0;
					return TRUE;
				}
				else
				{
					computeTag(id, &oldline[i], taglen, temp);
					len = strlen(temp);
					if (len > 0)
					{
						// wview tag found, do the replacement
						for (k = 0; k < len; k++)
						{
							newline[j] = temp[k];
							j++;
						}
					}
					else
					{
						// just copy old to new unchanged
						for (k = 0; k < taglen; k++)
						{
							newline[j] = oldline[i + k];
							j++;
						}
					}
				}

				// move the oldline index past the data tag
				i += (taglen - 1);
			}
			else
			{
				// no closing '>' found, just move on
				newline[j] = oldline[i];
				j++;
			}
		}
		else
		{
			newline[j] = oldline[i];
			j++;
		}
	}
	newline[j] = 0;
	return FALSE;
}

static int createOutFile(HTML_MGR_ID id, char* templatefile, uint64_t startTime)
{
	FILE*        infile, *outfile, *incfile;
	char*        ptr;
	char        oldfname[WVIEW_STRING2_SIZE];
	char        newfname[WVIEW_STRING2_SIZE];
	char        includefname[WVIEW_STRING2_SIZE];
	char        line[HTML_MAX_LINE_LENGTH], newline[HTML_MAX_LINE_LENGTH];

	sprintf(oldfname, "%s/%s", id->htmlPath, templatefile);

	// non-home page template
	sprintf(newfname, "%s/%s", id->imagePath, templatefile);

	// fix the extension
	ptr = strrchr(newfname, '.');
	strcpy(ptr, ".txt");

	//  ... now open the files up
	infile = fopen(oldfname, "r");
	if (infile == NULL)
	{
		MsgLog(PRI_MEDIUM, "createOutFile: cannot open %s for reading!",
			oldfname);
		return ERROR;
	}

	outfile = fopen(newfname, "w");
	if (outfile == NULL)
	{
		MsgLog(PRI_MEDIUM, "createOutFile: cannot open %s for writing!",
			newfname);
		fclose(infile);
		return ERROR;
	}

	//  ... now read each line of the template -
	//  ... replacing any data tags
	//  ... then writing to the output file
	while (fgets(line, HTML_MAX_LINE_LENGTH, infile) != NULL)
	{
		if (replaceDataTags(id, line, newline) == TRUE)
		{
			// we must include an external file here, we expect to find it
			// in the output directory for images and expansions...
			sprintf(includefname, "%s/%s", id->imagePath, newline);
			incfile = fopen(includefname, "r");
			if (incfile == NULL)
			{
				MsgLog(PRI_MEDIUM, "createOutFile: cannot open %s for reading!",
					includefname);
				fclose(infile);
				fclose(outfile);
				return ERROR;
			}
			while (fgets(line, HTML_MAX_LINE_LENGTH, incfile) != NULL)
			{
				if (fputs(line, outfile) == EOF)
				{
					fclose(incfile);
					fclose(infile);
					fclose(outfile);
					return ERROR;
				}
			}

			// done
			fclose(incfile);
		}
		else
		{
			if (fputs(newline, outfile) == EOF)
			{
				fclose(infile);
				fclose(outfile);
				return ERROR;
			}
		}
	}

	fclose(infile);
	fclose(outfile);
	return OK;
}

//  ... API methods

int htmlGenerateInit(void)
{
	char            instance[32], value[_MAX_PATH];
	struct stat     fileStatus;
	int             index;

	// Set up tag matching search tree:
	tagSearchEngine = radtextsearchInit();
	if (tagSearchEngine == NULL)
	{
		MsgLog(PRI_CATASTROPHIC, "htmlGenerateInit: radtextsearchInit failed!");
		return ERROR;
	}

	// Insert all of our tags:
	for (index = 0; dataTags[index] != NULL; index++)
	{
		if (radtextsearchInsert(tagSearchEngine, dataTags[index], index) == ERROR)
		{
			MsgLog(PRI_CATASTROPHIC, "htmlGenerateInit: radtextsearchInsert %d failed!", index);
			return ERROR;
		}
	}

	MsgLog(PRI_STATUS, "Tag Search red-black tree: max black node tree height: %d",
		radtextsearchDebug(tagSearchEngine->root));

	return OK;
}

int htmlgenOutputFiles(HTML_MGR_ID id, uint64_t startTime)
{
	register HTML_TMPL*  tmpl;
	int                 count = 0;

	for (tmpl = (HTML_TMPL*)radListGetFirst(&id->templateList);
		tmpl != NULL;
		tmpl = (HTML_TMPL*)radListGetNext(&id->templateList, (NODE_PTR)tmpl))
	{
#if _DEBUG_GENERATION
		wvutilsLogEvent(PRI_HIGH, "GENERATION: TEMPLATE: %s: %u ms",
			tmpl->fname, (uint32_t)(radTimeGetMSSinceEpoch() - startTime));
#endif

		if (createOutFile(id, tmpl->fname, startTime) == ERROR)
		{
			MsgLog(PRI_MEDIUM, "htmlgenOutputFiles: %s failed!", tmpl->fname);
		}
		else
		{
			count++;
		}
	}

	return count;
}