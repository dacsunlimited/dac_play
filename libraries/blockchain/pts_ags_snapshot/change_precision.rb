#!/usr/bin/env ruby -rubygems

# generated pts and ags genesis json are of 8 precision
# pls use 5 precision, omit last 3 digits to standardize

require 'rubygems'
require 'json'

PRECISION = 100_000

[
  { name: :ags, file: 'ags_20140718.json' },
  { name: :pts, file: 'pts_20141105.json' }
].each do |part|
  json = JSON.parse( File.read( File.join(File.dirname(__FILE__), part[:file]) ))

  current_precision = json["precision"] || 100_000_000
  moneysupply = 0

  if current_precision != PRECISION
    offset =  current_precision / PRECISION
    json["balances"].each do |entry|
      entry[1] = (entry[1] / offset).to_i
      moneysupply += entry[1]
    end

    json["moneysupply"] = moneysupply
    json["precision"]   = PRECISION
  end

  File.write("./#{part[:file]}", json.to_json)
end
