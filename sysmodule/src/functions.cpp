#include <sstream>
#include <string>
#include "functions.h"
#include "switch.h"

std::string accountUidToString(AccountUid& uid) {
    std::stringstream st;

    st << uid.uid[0]<< " " << uid.uid[1];

    return st.str();
}

AccountUid accountUidFromString(std::string& uid) {
    AccountUid u;
    std::stringstream st(uid);

    st >> u.uid[0] >> u.uid[1];

    return u;
}
