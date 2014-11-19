#!/usr/bin/env ruby -rubygems

gem 'bitcoin-ruby'
require 'bitcoin'
require 'pry'

module Bitcoin
  NETWORKS[:protoshares] = {
    :project => :protoshares,
    :magic_head => "\xF9\xBD\xB5\xD9",
    :address_version => "38", # 56
    :p2sh_version => "05", # 5
    :privkey_version => "B8" # 184
  }

  module Util
    def protoshares_signed_message_hash(message)
      # TODO: this will fail horribly on messages with len > 255. It's a cheap implementation of Bitcoin's CDataStream.
      data = "\x18ProtoShares Signed Message:\n" + [message.bytesize].pack("C") + message
      Digest::SHA256.digest(Digest::SHA256.digest(data))
    end

    def verify_message(address, signature, message)
      hash = Bitcoin.bitcoin? ?
             bitcoin_signed_message_hash(message) :
             protoshares_signed_message_hash(message)

      signature = signature.unpack("m0")[0] rescue nil # decode base64
      raise "invalid address"           unless valid_address?(address)
      raise "malformed base64 encoding" unless signature
      raise "malformed signature"       unless signature.bytesize == 65
      pubkey = Bitcoin::OpenSSL_EC.recover_compact(hash, signature)
      pubkey_to_address(pubkey) == address if pubkey
    rescue Exception => ex
      false
    end
  end
end


raw_file = 'substitution.txt'

File.open(raw_file, 'r') do |file|
  content = file.read.gsub(/#.*\n{1,2}/,'')

  content.split(/\n{2}/).each do |group|
    address, message, signature = group.split(/\n/).map{ |line| line.sub(/^[^:]+:\s*/, '') }

    Bitcoin.network = address =~ /^[13]/ ? :bitcoin : :protoshares

    valid = Bitcoin.valid_address?(address) && (Bitcoin.verify_message(address, signature, message))

    # next if valid
    if valid
      new_address = message.gsub(/.+:\s*/, '')

      puts "#{address} => #{new_address}"
      puts '-----------'
    else
      puts address + ': ' + valid.to_s
      puts 'address valid?: ' + Bitcoin.valid_address?(address).to_s
      puts message
      puts signature
      puts new_address
      puts '--------------'
    end

  end
end

# puts Bitcoin.verify_message('1Gaw39RvbkZxcXeYzGrjWvmiEqAB6PMqsX', 'INfjG+txvMe72T5K5yrKfHvHhbg1pGGJUvbMqYiHkYhmeXNgr23r8myPEVCUA5XmHDBAvX+34SJJMQu2IasdGxs=', 'New BTC Address: 1A2SAL7i5UwZ3pYuaf7rcBj1U3wffEAoo7')