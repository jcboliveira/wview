#ifndef INC_wvconfigh
#define INC_wvconfigh
/*---------------------------------------------------------------------------

  FILENAME:
		wvconfig.h

  PURPOSE:
		Define the wview configuration API.

  REVISION HISTORY:
		Date            Engineer        Revision        Remarks
		07/05/2008      M.S. Teel       0               Original

  NOTES:

  LICENSE:
		Copyright (c) 2008, Mark S. Teel (mark@teel.ws)

		This source code is released for free distribution under the terms
		of the GNU General Public License.

----------------------------------------------------------------------------*/

//  ... includes
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <sys/stat.h>

#include <radshmem.h>
#include <radlist.h>
#include <radbuffers.h>
#include <radsemaphores.h>
#include <radsqlite.h>

#include <sysdefs.h>

//  ... macro definitions

//  ... typedefs

// wview configuration item IDs:

#define configItem_ENABLE_HTMLGEN                               "ENABLE_HTMLGEN"

#define configItem_ENABLE_EMAIL                                 "ENABLE_EMAIL_ALERTS"
#define configItem_TO_EMAIL_ADDRESS                             "EMAIL_ADDRESS"
#define configItem_FROM_EMAIL_ADDRESS                           "FROM_EMAIL_ADDRESS"
#define configItem_SEND_TEST_EMAIL                              "SEND_TEST_EMAIL"

#define configItem_STATION_STATION_TYPE                         "STATION_TYPE"
#define configItem_STATION_STATION_INTERFACE                    "STATION_INTERFACE"
#define configItem_STATION_STATION_DEV                          "STATION_DEV"
#define configItem_STATION_STATION_HOST                         "STATION_HOST"
#define configItem_STATION_STATION_PORT                         "STATION_PORT"
#define configItem_STATION_STATION_WLIP                         "STATION_WLIP"
#define configItem_STATION_STATION_RETRIEVE_ARCHIVE             "STATION_RETRIEVE_ARCHIVE"
#define configItem_STATION_STATION_DTR                          "STATION_DTR"
#define configItem_STATION_STATION_RAIN_SEASON_START            "STATION_RAIN_SEASON_START"
#define configItem_STATION_STATION_RAIN_STORM_TRIGGER_START     "STATION_RAIN_STORM_TRIGGER_START"
#define configItem_STATION_STATION_RAIN_STORM_IDLE_STOP         "STATION_RAIN_STORM_IDLE_STOP"
#define configItem_STATION_STATION_RAIN_YTD                     "STATION_RAIN_YTD"
#define configItem_STATION_STATION_ET_YTD                       "STATION_ET_YTD"
#define configItem_STATION_STATION_RAIN_ET_YTD_YEAR             "STATION_RAIN_ET_YTD_YEAR"
#define configItem_STATION_STATION_ELEVATION                    "STATION_ELEVATION"
#define configItem_STATION_STATION_LATITUDE                     "STATION_LATITUDE"
#define configItem_STATION_STATION_LONGITUDE                    "STATION_LONGITUDE"
#define configItem_STATION_STATION_ARCHIVE_INTERVAL             "STATION_ARCHIVE_INTERVAL"
#define configItem_STATION_ARCHIVE_PATH                         "STATION_ARCHIVE_PATH"
#define configItem_STATION_POLL_INTERVAL                        "STATION_POLL_INTERVAL"
#define configItem_STATION_PUSH_INTERVAL                        "STATION_PUSH_INTERVAL"
#define configItem_STATION_VERBOSE_MSGS                         "STATION_VERBOSE_MSGS"
#define configItem_STATION_DO_RXCHECK                           "STATION_DO_RCHECK"
#define configItem_STATION_OUTSIDE_CHANNEL                      "STATION_OUTSIDE_CHANNEL"

#define configItem_HTMLGEN_STATION_NAME                         "HTMLGEN_STATION_NAME"
#define configItem_HTMLGEN_STATION_CITY                         "HTMLGEN_STATION_CITY"
#define configItem_HTMLGEN_STATION_STATE                        "HTMLGEN_STATION_STATE"
#define configItem_HTMLGEN_STATION_SHOW_IF                      "HTMLGEN_STATION_SHOW_IF"
#define configItem_HTMLGEN_IMAGE_PATH                           "HTMLGEN_IMAGE_PATH"
#define configItem_HTMLGEN_HTML_PATH                            "HTMLGEN_HTML_PATH"
#define configItem_HTMLGEN_START_OFFSET                         "HTMLGEN_START_OFFSET"
#define configItem_HTMLGEN_GENERATE_INTERVAL                    "HTMLGEN_GENERATE_INTERVAL"
#define configItem_HTMLGEN_METRIC_UNITS                         "HTMLGEN_METRIC_UNITS"
#define configItem_HTMLGEN_METRIC_USE_RAIN_MM                   "HTMLGEN_METRIC_USE_RAIN_MM"
#define configItem_HTMLGEN_WIND_UNITS                           "HTMLGEN_WIND_UNITS"
#define configItem_HTMLGEN_DUAL_UNITS                           "HTMLGEN_DUAL_UNITS"
#define configItem_HTMLGEN_EXTENDED_DATA                        "HTMLGEN_EXTENDED_DATA"
#define configItem_HTMLGEN_ARCHIVE_BROWSER_FILES_TO_KEEP        "HTMLGEN_ARCHIVE_BROWSER_FILES_TO_KEEP"
#define configItem_HTMLGEN_MPHASE_INCREASE                      "HTMLGEN_MPHASE_INCREASE"
#define configItem_HTMLGEN_MPHASE_DECREASE                      "HTMLGEN_MPHASE_DECREASE"
#define configItem_HTMLGEN_MPHASE_FULL                          "HTMLGEN_MPHASE_FULL"
#define configItem_HTMLGEN_LOCAL_RADAR_URL                      "HTMLGEN_LOCAL_RADAR_URL"
#define configItem_HTMLGEN_LOCAL_FORECAST_URL                   "HTMLGEN_LOCAL_FORECAST_URL"
#define configItem_HTMLGEN_DATE_FORMAT                          "HTMLGEN_DATE_FORMAT"

#define configItemCAL_MULT_BAROMETER                            "CAL_MULT_BAROMETER"
#define configItemCAL_CONST_BAROMETER                           "CAL_CONST_BAROMETER"
#define configItemCAL_MULT_PRESSURE                             "CAL_MULT_PRESSURE"
#define configItemCAL_CONST_PRESSURE                            "CAL_CONST_PRESSURE"
#define configItemCAL_MULT_ALTIMETER                            "CAL_MULT_ALTIMETER"
#define configItemCAL_CONST_ALTIMETER                           "CAL_CONST_ALTIMETER"
#define configItemCAL_MULT_INTEMP                               "CAL_MULT_INTEMP"
#define configItemCAL_CONST_INTEMP                              "CAL_CONST_INTEMP"
#define configItemCAL_MULT_OUTTEMP                              "CAL_MULT_OUTTEMP"
#define configItemCAL_CONST_OUTTEMP                             "CAL_CONST_OUTTEMP"
#define configItemCAL_MULT_INHUMIDITY                           "CAL_MULT_INHUMIDITY"
#define configItemCAL_CONST_INHUMIDITY                          "CAL_CONST_INHUMIDITY"
#define configItemCAL_MULT_OUTHUMIDITY                          "CAL_MULT_OUTHUMIDITY"
#define configItemCAL_CONST_OUTHUMIDITY                         "CAL_CONST_OUTHUMIDITY"
#define configItemCAL_MULT_WINDSPEED                            "CAL_MULT_WINDSPEED"
#define configItemCAL_CONST_WINDSPEED                           "CAL_CONST_WINDSPEED"
#define configItemCAL_MULT_WINDDIR                              "CAL_MULT_WINDDIR"
#define configItemCAL_CONST_WINDDIR                             "CAL_CONST_WINDDIR"
#define configItemCAL_MULT_RAIN                                 "CAL_MULT_RAIN"
#define configItemCAL_CONST_RAIN                                "CAL_CONST_RAIN"
#define configItemCAL_MULT_RAINRATE                             "CAL_MULT_RAINRATE"
#define configItemCAL_CONST_RAINRATE                            "CAL_CONST_RAINRATE"

// Define the column names for wview-conf.sdb:
#define configCOLUMN_NAME                                       "name"
#define configCOLUMN_VALUE                                      "value"
#define configCOLUMN_DESCRIPTION                                "description"

//  ... API prototypes

//  wvconfigInit: Initialize/Attach to the wview configuration API:
//  "firstProcess" is a BOOL to indicate if this is the first process to call init;
//  Returns: OK or ERROR
extern int wvconfigInit(int firstProcess);

//  wvconfigExit: clean up and detach from the wview configuration API
extern void wvconfigExit(void);

//  wvconfigGetINTValue: retrieve the integer value for this parameter;
//  Returns: integer value
extern int wvconfigGetINTValue(const char* configItem);

//  wvconfigGetDOUBLEValue: retrieve the double value for this parameter;
//  Returns: double value
extern double wvconfigGetDOUBLEValue(const char* configItem);

//  wvconfigGetStringValue: retrieve the string value for this parameter
//  Returns: const static string reference or NULL
extern const char* wvconfigGetStringValue(const char* configItem);

//  wvconfigGetBooleanValue: retrieve the bool value for this parameter:
//  Assumption: anything other than lowercase "yes" or "1" or "TRUE"
//              is considered FALSE
//  Returns: TRUE or FALSE or ERROR
extern int wvconfigGetBooleanValue(const char* configItem);

#endif
