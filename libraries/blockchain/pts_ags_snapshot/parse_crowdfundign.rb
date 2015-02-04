#!/usr/bin/env ruby

# generate crowdfunding.json based on crowdfunding record
# input file: playcrowdfund.csv.txt (parsed from blockchain with tool from https://github.com/AlexChien/ags-parser)
# total supply: 400_000_000
# rules:
#   1. valid date range:
#   2. exclude transaction from 3DPD7z66T7DXULvFZUsc4xNcVS2q68TM3u
#   3. week1: 130_000
#   4. week2: 120_000
#   5. week3: 110_000
#   6. week4: 100_000
#   7. remaining shares shared proportionally by accquired PLS

require 'json'
require 'time'
require 'pry'

COIN = 100_000 #precision
SUPPLY = 400_000_000
ADDRESS = '3DPD7z66T7DXULvFZUsc4xNcVS2q68TM3u'
START_DATE = Time.utc('2015','01','05')
END_DATE = Time.utc('2015','02','02')

@input_file   = './playcrowdfund.csv.txt'
@output_file  = 'crowdfund_genesis.json'

def parse_data(response)

  response.each_line do |line|
    if line =~ /^"{0,1}\d+/
      height, time, txbits, addr, amount, total, rate, related_addrs = line.strip.gsub('"','').split(';')

      # if donation comes in after ags is finished, skip it
      next unless valid_date?(time) && valid_address?(addr)

      # amount = (amount.to_f).round
      # total = (total.to_f).round

      # override ags-parser rate logic, using depend-on-week logic
      rate    = current_rate(time)
      reward  = (amount.to_f * COIN * rate).round

      @balance[addr] = 0 unless @balance[addr]
      @balance[addr] += reward
    end
  end

  output
  deal_remaining
  flush
end

# rule 1
def valid_date?(time)
  Time.parse(time) <= END_DATE
rescue
  true
end

# rule 2
def valid_address?(address)
  address != ADDRESS
end

# rule 3-6
# week passed
# 0: 130_000
# 1: 120_000
# 2: 110_000
# 3: 100_000
def current_rate(time)
  t = Time.parse(time)

  # donatio prior start date counts as day 1
  t = START_DATE if t < START_DATE

  days_passed = (t.to_time - START_DATE).to_i / 3600 / 24
  week_passed = (days_passed / 7).to_i

  [130_000 - 10_000 * week_passed, 100_000].max
end

# debug print
def output
  @balance.each { |k,v| puts "#{k}: #{v}" }

  total = @balance.inject(0){ |m,n| m+n[1] }
  puts "total pls: #{total}"

  puts @balance['1BNyQDcr7ECGBV2zeZL2LtzEAxrdXLXRw9']
end

def main
  @balance = {}

  begin
    parse_data(File.read(@input_file))
  rescue => e
    puts "parse error: #{e}"
  end

end

# rule 7
def deal_remaining
  total = @balance.inject(0){ |m,n| m+n[1] }
  remaining = SUPPLY * COIN - total

  puts "remaining: #{remaining}"

  ratio = remaining.to_f / total

  @balance.keys.each do |k|
    @balance[k] += (@balance[k] * ratio).round
  end

  output
end

# write to output file
def flush
  result = {
    balance: @balance.to_a,
    moneysupply: @balance.inject(0){ |m,n| m+n[1] },
    presision: COIN
  }

  File.open(@output_file, 'w'){ |f| f.puts JSON.pretty_generate(result) }
end

main
