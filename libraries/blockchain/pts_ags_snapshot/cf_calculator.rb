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

class CFCalculator
  attr_reader :balance, :input_file, :output_file

  # total shares reward to user by purchase
  attr_reader :total_purchased_shares

  # total shares remaining after cf
  attr_reader :final_supply


  COIN = 100_000 #precision
  SUPPLY = 400_000_000
  ADDRESS = '3DPD7z66T7DXULvFZUsc4xNcVS2q68TM3u'
  START_DATE = Time.utc('2015','01','05')
  END_DATE = Time.utc('2015','02','02')

  # some users send fund from exchange, requested to substitute with address they control
  # verified by @logxing
  # [[old, new],...]
  ADDR_SUB = [
    ['1MPgmfBnbPPXi2iGz49znvud12UNmcpuQ','1PzDV53zJ3oaUHYvaez8QvL3mZu6WDKXa'],
    ['14VRNC8ncvA1wGL2HzgoKxQhhV24SoZoZQ','1PmQSsw1VsXvmtWnByiiCGowr4ag3aL5am'],
    ['1dLHXkugirKQE1THRv7XCpuWMzvzcWQj5','1PmQSsw1VsXvmtWnByiiCGowr4ag3aL5am']
  ]

  def initialize(input_file = nil, output_file = nil)
    @input_file   = input_file
    @output_file  = output_file

    @balance        = {}
    @total_donated  = 0
    @total_purchased_shares = 0
  end

  def total_donated
    processed? ? @total_donated.to_f / COIN : 0
  end

  def processed?
    !!@processed
  end

  def process
    parse_data(File.read(@input_file))
    @processed = true
  rescue => e
    @processed = false
    puts "parse error: #{e}"
  end

  def parse_data(response)
    response.each_line do |line|
      if line =~ /^"{0,1}\d+/
        height, time, txbits, addr, amount, total, rate, related_addrs = line.strip.gsub('"','').split(';')

        # if donation comes in after ags is finished, skip it
        next unless valid_date?(time) && valid_address?(addr)

        @total_donated += (amount.to_f * COIN).round

        # override ags-parser rate logic, using depend-on-week logic
        rate    = current_rate(time)
        reward  = (amount.to_f * COIN * rate).round

        @balance[addr] = 0 unless @balance[addr]
        @balance[addr] += reward

        @total_purchased_shares += reward
      end
    end

    # output
    deal_remaining
    substitute
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
    # puts "total pls: #{total}"

    puts @balance['1BNyQDcr7ECGBV2zeZL2LtzEAxrdXLXRw9']
  end

  # rule 7
  def deal_remaining
    total = @balance.inject(0){ |m,n| m+n[1] }
    remaining = SUPPLY * COIN - total

    # puts "remaining: #{remaining}"

    ratio = remaining.to_f / total

    @balance.keys.each do |k|
      @balance[k] += (@balance[k] * ratio).round
    end

    @final_supply = @balance.inject(0){ |m,n| m+n[1] }
  end

  def substitute
    ADDR_SUB.each do |sub|
      o_addr, n_addr = sub
      next unless @balance[o_addr]

      if @balance[n_addr]
        @balance[n_addr] += @balance[o_addr]
      else
        @balance[n_addr] = @balance[o_addr]
      end

      @balance.delete(o_addr)
    end
  end

  # write to output file
  def flush
    result = {
      balance: @balance.to_a,
      moneysupply: final_supply,
      presision: COIN
    }

    File.open(@output_file, 'w'){ |f| f.puts JSON.pretty_generate(result) }
    puts "#{@output_file} generated"
  end
end



cal = CFCalculator.new('./playcrowdfund.csv.txt', 'crowdfund_genesis.json')
cal.process

puts "total donated: #{cal.total_donated} BTC"
puts "purchased shares: #{cal.total_purchased_shares} PLS (precision #{CFCalculator::COIN})"
puts "total supply: #{cal.final_supply} PLS (precision #{CFCalculator::COIN})"
