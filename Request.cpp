#include "Request.h"

Request::Request() : ipIn("0.0.0.0"), ipOut("0.0.0.0"), timeRequired(1), jobType('P') {}

Request::Request(const std::string& in, const std::string& out, int time, char type)
    : ipIn(in), ipOut(out), timeRequired(time), jobType(type) {}
