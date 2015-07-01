
#include <iostream>
#include <map>

#include <fc/crypto/elliptic.hpp>
#include <fc/io/fstream.hpp>
#include <fc/io/iostream.hpp>
#include <fc/io/sstream.hpp>
#include <fc/io/json.hpp>
#include <bts/blockchain/transaction.hpp>
#include <bts/wallet/wallet_records.hpp>
#include <bts/wallet/wallet.hpp>

#include "serial_tester.hpp"
#include "serial_tests.hpp"

using namespace std;

int main(int argc, char **argv, char **envp)
{
    try{

	//fc::ofstream unknown_output("unknown-serial.txt");
	
	//serial_test_gen_privkey(&unknown_output);
        
        
        
        fc::json::from_string("{\"record_id\":\"4fe11aefec1ef8da172763a83ba5a3a378932ca4\",\"block_num\":18,\"is_virtual\":true,\"is_confirmed\":true,\"contract\":\"DICE\",\"ledger_entries\":[{\"to_account\":\"XTS6XAa71GwaKtmSb752faj6Tfk84JhFKxNYuUhDaB8PyQ5KMqMic\",\"amount\":{\"amount\":0,\"asset_id\":1},\"memo\":\"lose, jackpot lucky number: 2\"}],\"fee\":{\"amount\":0,\"asset_id\":0},\"created_time\":\"\\\"2015-07-01T18:29:30\\\"\",\"received_time\":\"\\\"2015-07-01T18:29:30\\\"\"}").as<bts::wallet::transaction_info>();
    } FC_LOG_AND_RETHROW();

    cout << "done\n";

    return 0;
}
