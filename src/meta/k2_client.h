#pragma once
#include "k2_includes.h"

namespace k2pg {
namespace gate {
class PGK2Client {
public:
    PGK2Client();
    ~PGK2Client();
    // required for seastar::distributed interface
    seastar::future<> gracefulStop();
    seastar::future<> start();
private:
    bool _stop = false;
    k2::K23SIClient *_client;
    seastar::future<> _poller = seastar::make_ready_future();
    std::unordered_map<k2::dto::K23SI_MTR, k2::K2TxnHandle>* _txns;

    seastar::future<> _pollForWork();
     seastar::future<> _pollBeginQ();
    seastar::future<> _pollEndQ();
    seastar::future<> _pollSchemaGetQ();
    seastar::future<> _pollSchemaCreateQ();
    seastar::future<> _pollReadQ();
    // seastar::future<> _pollCreateScanReadQ();
    // seastar::future<> _pollScanReadQ();
    seastar::future<> _pollWriteQ();
    // seastar::future<> _pollUpdateQ();
    seastar::future<> _pollCreateCollectionQ();
    // seastar::future<> _pollDropCollectionQ();
};

class K23SITxn
        {
        public:
            K23SITxn(k2::dto::K23SI_MTR mtr, k2::TimePoint startTime):_mtr(std::move(mtr)), _startTime(startTime){};
            k2::dto::K23SI_MTR GetMtr() {return _mtr;};

        private:
            // fields
            k2::dto::K23SI_MTR _mtr; // mtr for this transaction
            k2::TimePoint _startTime;
        };

}//gate
}//k2pg