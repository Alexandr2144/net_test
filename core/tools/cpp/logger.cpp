#include "core/tools/logger.h"
#include "native/crash.h"

#include <stdarg.h>


namespace Tools {
	void log(LogTree_CRef logtree, Severity severity, String_CRef category, String_CRef msg, ...)
	{
		//ILogger* logger = Data::get(logtree.loggers, 0);
		//logger->log(severity, 0, msg);

		//va_list args;
		//va_start(args, &msg);
		//Tools::assert_vfail(M_CL, (char*)msg.begin(), args);
		//va_end(args);
	}

} // namespace Tools