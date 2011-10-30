#ifndef IBAPI_VALUES_H_
#define IBAPI_VALUES_H_


namespace IBAPI {

// In general, these may not match exactly as the values defined for FIX.
// These are instead values that are suitable for IB's API.  This is ok
// since on the client side, when the messages are constructure, type-safe
// constants are used so the implementation details of the actual values
// are abstracted away from the client.

const int PutOrCall_CALL = 1;
const int PutOrCall_PUT = 0;
const char SecurityType_OPTION[] = "OPT";
const char SecurityType_COMMON_STOCK[] = "STK"; // Maps to IB's value

const char RoutingID_DEFAULT[] = "SMART";  // default for IB
const char RoutingID_SMART[] = "SMART";


};


#endif //IBAPI_VALUES_H_
