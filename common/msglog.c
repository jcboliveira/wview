
/*      ... OS include files
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <msglog.h>

/*      ... Local include files
*/

/*      ... global memory declarations
*/

/*      ... global memory referenced
*/

/*      ... static (local) memory declarations
*/
static int      msTimestamp = 0;


/*  ... for use in system wide initialization ONLY
*/
int MsgLogInit
(
	char        *procName,
	int         useStderr,      /* T/F: also write to stderr */
	int         timeStamp       /* T/F: include millisecond timestamps */
)
{
	int         options;

	msTimestamp = timeStamp;

	options = LOG_NDELAY | LOG_PID;
	if (useStderr)
	{
		options |= LOG_PERROR;
	}

	openlog(procName, options, LOG_USER);

	return OK;
}


int MsgLogExit
(
	void
)
{
	closelog();

	return OK;
}

/*  ... log a message - allow variable length parameter list
*/
int MsgLog
(
	int         priority,
	char        *format,
	...
)
{
	va_list     argList;
	char        temp1[512];
	int         index;

	if (msTimestamp)
	{
		index = sprintf(temp1, "<%llu> : ", radTimeGetMSSinceEpoch());
	}
	else
	{
		index = 0;
	}

	/*  ... print the var arg stuff to the message
	*/
	va_start(argList, format);
	vsprintf(&temp1[index], format, argList);
	va_end(argList);

	syslog(priority, temp1);

	return OK;
}

void MsgLogData(void *data, int length)
{
	char        msg[256], temp[16], temp1[16], ascii[128];
	int         i, j;
	UCHAR       *ptr = (UCHAR *)data;
	int         dataPresent = 1;

	MsgLog(PRI_STATUS, "DBG: Dumping %p, %d bytes:", data, length);
	memset(msg, 0, sizeof(msg));
	memset(ascii, 0, sizeof(ascii));

	for (i = 0; i < length; i++)
	{
		dataPresent = 1;
		sprintf(temp, "%2.2X", ptr[i]);
		sprintf(temp1, "%c", ((isprint(ptr[i]) ? ptr[i] : '.')));

		if (i % 2)
		{
			strcat(temp, " ");
		}

		if (i && ((i % 16) == 0))
		{
			// we need to dump a line
			strcat(msg, "    ");
			strcat(msg, ascii);
			MsgLog(PRI_STATUS, msg);
			memset(msg, 0, sizeof(msg));
			memset(ascii, 0, sizeof(ascii));
			dataPresent = 0;
		}

		strcat(msg, temp);
		strcat(ascii, temp1);
	}

	if (dataPresent)
	{
		// we need to dump the last line
		for (j = (i % 16); j != 0 && j < 16; j++)
		{
			strcat(msg, "  ");
			if (j % 2)
				strcat(msg, " ");
		}
		strcat(msg, "    ");
		strcat(msg, ascii);
		MsgLog(PRI_STATUS, msg);
	}

	return;
}

