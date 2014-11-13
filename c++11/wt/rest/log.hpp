#ifndef LOG_H_
#define LOG_H_

#include <Wt/WLogger>

using namespace std;
using namespace Wt;

void info(WLogger& logger, const string& message);
void fatal(WLogger& logger, const string& message, const char* what);
void fatal(WLogger& logger, const string& message);
void configLogger(WLogger& logger);

#endif /* LOG_H_ */
