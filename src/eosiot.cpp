#include <eosiot.hpp>

ACTION eosiot::create(const name& issuer)
{
  require_auth(get_self());

  asset maximum_supply = asset(MAX_SUPPLY, SYMB);
  auto sym = maximum_supply.symbol;

  stats statstable(get_self(), sym.code().raw());
  auto existing = statstable.find(sym.code().raw());
  check(existing == statstable.end(), "token with symbol already exists");

  statstable.emplace(get_self(), [&](auto& s) {
    s.supply.symbol = maximum_supply.symbol;
    s.max_supply = maximum_supply;
    s.issuer = issuer;
  });
}

ACTION eosiot::issue(const name& to, const asset& quantity, const string& memo)
{
  auto sym = quantity.symbol;
  check(sym.is_valid(), "invalid symbol name");
  check(memo.size() <= 256, "memo has more than 256 bytes");

  stats statstable(get_self(), sym.code().raw());
  auto existing = statstable.find(sym.code().raw());
  check(existing != statstable.end(), "token with symbol does not exist, create token before issue");
  const auto& st = *existing;

  require_auth(st.issuer);
  check(quantity.is_valid(), "invalid quantity");
  check(quantity.amount > 0, "must issue positive quantity");

  check(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
  check(quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

  statstable.modify(st, same_payer, [&](auto& s) {
    s.supply += quantity;
  });

  add_balance(st.issuer, quantity, st.issuer);

  if (to != st.issuer)
  {
    SEND_INLINE_ACTION(*this, transfer, {{st.issuer, "active"_n}},
                       {st.issuer, to, quantity, memo});
  }
}

ACTION eosiot::retire(const asset& quantity, const string& memo )
{
    auto sym = quantity.symbol;
    check( sym.is_valid(), "invalid symbol name" );
    check( memo.size() <= 256, "memo has more than 256 bytes" );

    stats statstable( _self, sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing != statstable.end(), "token with symbol does not exist" );
    const auto& st = *existing;

    require_auth( st.issuer );
    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must retire positive quantity" );

    check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );

    statstable.modify( st, same_payer, [&]( auto& s ) {
       s.supply -= quantity;
    });

    sub_balance( st.issuer, quantity );
}

ACTION eosiot::transfer(const name& from, const name& to, const asset& quantity, const string& memo)
{
  check(from != to, "cannot transfer to self");
  require_auth(from);
  check(is_account(to), "to account does not exist");
  auto sym = quantity.symbol.code();
  stats statstable(get_self(), sym.raw());
  const auto& st = statstable.get(sym.raw());

  require_recipient(from);
  require_recipient(to);

  check(quantity.is_valid(), "invalid quantity");
  check(quantity.amount > 0, "must transfer positive quantity");
  check(quantity.symbol == st.supply.symbol, "symbol precision mismatch");
  check(memo.size() <= 256, "memo has more than 256 bytes");

  auto payer = has_auth(to) ? to : from;

  sub_balance(from, quantity);
  add_balance(to, quantity, payer);
}

ACTION eosiot::createAccount( const name& user, const asset& balance, const name& gateway_id )
{
    require_auth(user);

    accounts acnts( get_self(), user.value );

    auto itr = acnts.find(user);
    check(itr == acnts.end(), "account already exists");

    auto itr_gtwy = acnts.find(gateway_id);
    check(itr_gtwy == acnts.end(), "gateway already exists");

    acnts.emplace(account, [&]( auto& t) {
        t.user = user;
        t.balance = balance;
        t.gateway_id = gateway_id;
    });
}

ACTION eosiot::incentive( const name& user, const int level, const string& memo )
{
    require_auth( get_self() );

    asset quantity;

    // TOKEN ECONOMY
    if( level == 1 ){ // count of req 1%
      print ("level 1"); 

    }else if( level == 2 ){ // triangulation 16%
      print ("level 2"); 

    }else if( level == 3 ){ // proof of req 8%
      print ("level 3"); 

    }else if( level == 4 ){ // vote of trust 5%
      print ("level 4"); 

    }

    auto sym = quantity.symbol;
    check( sym.is_valid(), "invalid symbol name" );
    check( memo.size() <= 256, "memo has more than 256 bytes" );

    stats statstable( get_self(), sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing != statstable.end(), "token with symbol does not exist, create token before issue" );
    const auto& st = *existing;
    check( to == st.issuer, "tokens can only be issued to issuer account" );

    //require_auth( st.issuer );
    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must issue positive quantity" );

    check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    check( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

    statstable.modify( st, same_payer, [&]( auto& s ) {
       s.supply += quantity;
    });

    add_balance( st.issuer, quantity, st.issuer );
    
}

void eosiot::sub_balance(const name owner, const asset value)
{
  accounts from_acnts(get_self(), owner.value);

  const auto& from = from_acnts.get(value.symbol.code().raw(), "no balance object found");
  check(from.balance.amount >= value.amount, "overdrawn balance");

  from_acnts.modify(from, owner, [&](auto& a) {
    a.balance -= value;
  });
}

void eosiot::add_balance(name owner, asset value, name ram_payer)
{
  accounts to_acnts(get_self(), owner.value);
  auto to = to_acnts.find(value.symbol.code().raw());
  if (to == to_acnts.end())
  {
    to_acnts.emplace(ram_payer, [&](auto& a) {
      a.balance = value;
    });
  }
  else
  {
    to_acnts.modify(to, same_payer, [&](auto& a) {
      a.balance += value;
    });
  }
}


EOSIO_DISPATCH(eosiot, (create)(issue)(retire)(transfer)(createAccount)(incentive))
