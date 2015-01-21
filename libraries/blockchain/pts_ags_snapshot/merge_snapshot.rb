#!/usr/bin/env ruby

require 'json'

COIN = 100_000

result = []

[
  { name: :bts, file: 'bts_20141208.json', total_key: "total", totalsupply: 700_000_000 },
  { name: :ags, file: 'ags_20140718.json', total_key: "moneysupply", totalsupply: 200_000_000 },
  { name: :pts, file: 'pts_20141105.json', total_key: "moneysupply", totalsupply: 200_000_000 }
].each do |part|
  json = JSON.parse( File.read( File.join(File.dirname(__FILE__), part[:file]) ))

  json["balances"].each do |bal|
    result.push( {
      "raw_address" => bal[0],
      "balance" => bal[1]
   })

   end
end

File.open("temp.json","w") do |f|
  f.write(JSON.pretty_generate(result))
end
