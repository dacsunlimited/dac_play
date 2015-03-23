Given(/^I created a game asset called ([A-Z]+) for (\w+) with precision ([\d,\.]+), initial supply ([\d,\.]+), and inital collateral ([\d,\.]+)/) do |symbol, game_name, precision, initial_supply, initial_collateral|
  actor = @current_actor
  account = @current_actor.account
  actor.node.exec 'wallet_gia_create', account, game_name, symbol, symbol, symbol, precision, initial_supply, initial_collateral
end

Given(/^I created a game called (\w+)$/) do |name|
  actor = @current_actor
  account = @current_actor.account
  actor.node.exec 'game_create', name, account, 1, name, ""
end

When(/^I buy for (\d+) (\w+) chip/) do |amount, symbol|
  @current_actor.node.exec 'game_buy_chips', @current_actor.account, amount, symbol
end

When(/^I play game (\w+) using (\d+) (\w+) providing with (\d+) odds and (\d+) guess/) do |game_symbol, amount, symbol, odds, guess|
  actor = @current_actor
  account = @current_actor.account
  params = {}
  params['from_account_name'] = account
  params['amount'] = amount.to_f
  params['odds'] = odds.to_i
  params['guess'] = guess.to_i
  
  # https://github.com/BitShares/bitshares/issues/1110
  
  actor.node.exec 'game_play', game_symbol, params
end

Then /^I should win (\d+) (\w+) or lose/ do |amount, symbol|
    actor = @current_actor
    account = @current_actor.account
    trx_history = actor.node.exec 'wallet_account_transaction_history', account, symbol
    puts "wallet_account_transaction_history result:"
    puts trx_history
end

Then /^I should have (\d+) or (\d+) (\w+)/ do |amount1, amount2, currency|
  actor = @current_actor
  account = @current_actor.account
  data = actor.node.exec 'wallet_account_balance', account
  balance = get_balance(data, account, currency)
  
  expect(balance.to_f).to eq(amount1.to_f).or eq(amount2.to_f)
end
