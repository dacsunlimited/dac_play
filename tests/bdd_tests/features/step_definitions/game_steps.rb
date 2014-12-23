Given(/^I created a game asset called ([A-Z]+) with precision ([\d,\.]+), initial supply ([\d,\.]+), and inital collateral ([\d,\.]+)/) do |symbol, precision, initial_supply, initial_collateral|
  actor = @current_actor
  account = @current_actor.account
  actor.node.exec 'wallet_asset_create', symbol, symbol, account, symbol, 100000000, precision, true, true, initial_supply, initial_collateral
end

Given(/^I created a game called ([A-Z]+) with asset ([A-Z]+)/) do |symbol, asset_symbol|
  actor = @current_actor
  account = @current_actor.account
  actor.node.exec 'game_create', symbol, symbol, account, asset_symbol, 1, symbol, ""
end

