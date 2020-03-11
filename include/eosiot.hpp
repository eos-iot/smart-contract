#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/system.hpp>

#include <string>

using namespace std;
using namespace eosio;

static symbol SYMB = symbol("NDX", 3);
static uint64_t MAX_SUPPLY = 100000000; // 100.000.000 NDX

using namespace std;
using namespace eosio;

CONTRACT eosiot : public contract {
 using contract::contract;

  ACTION create(const name& issuer);
  ACTION issue(const name& to, const asset& quantity, const string& memo);
  ACTION retire(const asset& quantity, const string& memo);
  ACTION transfer(const name& from, const name& to, const asset& quantity, const string& memo);
  
  ACTION createAccount( const name& user, const asset& balance, const name& gateway_id);
  ACTION incentive( const name& user, int level, const string& memo );

  inline asset get_balance(const name& token_contract_account, const name& owner, const symbol_code& sys_code)
  {
    accounts accountstable(token_contract_account, owner.value);
    const auto& ac = accountstable.get(sys_code.raw());
    return ac.balance;
  }

private:
  TABLE account
  {
    name user;
    asset balance;
    name gateway_id;

    uint64_t primary_key() const { return balance.symbol.raw(); }
  };

  TABLE currency_stats
  {
    asset supply;
    asset max_supply;
    name issuer;

    uint64_t primary_key() const { return supply.symbol.raw(); }
  };

  typedef multi_index<"accounts"_n, account> accounts;
  typedef multi_index<"stat"_n, currency_stats> stats;

  void sub_balance(const name owner, const asset value);
  void add_balance(name owner, asset value, name ram_payer);
};
