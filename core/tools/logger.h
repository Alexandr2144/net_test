#pragma once
#ifndef TOOLS_LOGGER_H
#define TOOLS_LOGGER_H

#include "core/data/string.h"
#include "core/data/array.h"


namespace Tools {
	using Data::String_CRef;

	enum class Severity {
		Trace, Debug, Message,
		Warning, Error, Critical
	};
	struct ILogger {
		virtual ~ILogger() {}
		virtual void log(Severity severity, uint32_t code, String_CRef message) = 0;
	};
	struct LogTree {
		Data::Array<ILogger*> loggers;
	};
	using LogTree_CRef = LogTree const&;
	
	void log(LogTree_CRef logtree, Severity severity, String_CRef category, String_CRef msg, ...);

} // namespace Tools


#endif // TOOLS_LOGGER_H