#!/usr/bin/env ruby

# RESULT
# bts: 69999999398775 vs 70000000000000 [diff: -601225]
# ags: 19999999995125 vs 20000000000000 [diff: -4875]
# pts: 19999999977410 vs 20000000000000 [diff: -22590]
#
# diff due to rounding issue, within acceptable range

require 'json'

COIN = 100_000

[
  { name: :presale, file: 'crowdfund_genesis.json', total_key: "total", totalsupply: 400_000_000 },
  { name: :bts, file: 'bts_20141208.json', total_key: "total", totalsupply: 700_000_000 },
  { name: :ags, file: 'ags_20140718.json', total_key: "moneysupply", totalsupply: 200_000_000 },
  { name: :pts, file: 'pts_20141105.json', total_key: "moneysupply", totalsupply: 200_000_000 }
].each do |part|
  json = JSON.parse( File.read( File.join(File.dirname(__FILE__), part[:file]) ))

  actual = json["balances"].inject(0){ |sum, entry| sum + entry[1] }
  expected = part[:totalsupply] * COIN
  puts "#{part[:name]}: #{actual} vs #{expected} [diff: #{actual - expected}]"
end
