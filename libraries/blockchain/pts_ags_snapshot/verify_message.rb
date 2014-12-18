#!/usr/bin/env ruby -rubygems
#
# source: https://bitsharestalk.org/index.php?topic=4748.0
#
# if there're new requests posted in the threads, simply copy the entry into substituion.txt
#
# @usages: ./verify_message.rb
#
# WARNING: PTS address valdiation is not working yet, PTS address failed during validation
# might still be valid signature
#


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

    # switch network
    Bitcoin.network = address =~ /^[13]/ ? :bitcoin : :protoshares

    # validate address and validate signature
    valid = Bitcoin.valid_address?(address) && (Bitcoin.verify_message(address, signature, message))

    # next if valid
    if valid
      new_address = message.gsub(/.+:\s*/, '')

      puts "#{address} => #{new_address}"
    else
      # puts address + ': ' + valid.to_s
      # puts 'address valid?: ' + Bitcoin.valid_address?(address).to_s
      # puts message
      # puts signature
      # puts new_address
      # puts '--------------'
    end

  end
end

# puts Bitcoin.verify_message('1Gaw39RvbkZxcXeYzGrjWvmiEqAB6PMqsX', 'INfjG+txvMe72T5K5yrKfHvHhbg1pGGJUvbMqYiHkYhmeXNgr23r8myPEVCUA5XmHDBAvX+34SJJMQu2IasdGxs=', 'New BTC Address: 1A2SAL7i5UwZ3pYuaf7rcBj1U3wffEAoo7')

# valid substitution candidates
# ==========
# 1Gaw39RvbkZxcXeYzGrjWvmiEqAB6PMqsX => 1A2SAL7i5UwZ3pYuaf7rcBj1U3wffEAoo7
# 13U3XLUTRHLGMwfCmhde7EmQtNdJE7X2zw => 1A2SAL7i5UwZ3pYuaf7rcBj1U3wffEAoo7
# 178RVtWSyrCD8N1BnSdiSbMMokD2foQhAd => 1KfjASdNiX97R8eJM9HPbnKcFWZ8RRhzzb
# 1GJczjbQF8evXsLCHpX9sNXQ3m2QbwH2Wv => 1KfjASdNiX97R8eJM9HPbnKcFWZ8RRhzzb
# 19btdFcvEgF6t7hyr5n4gzsGitHZjk7uF4 => 1KfjASdNiX97R8eJM9HPbnKcFWZ8RRhzzb
# 1J9FPXTMJXwh1mC4CYDp8kjtgsQehiVos4 => 1KfjASdNiX97R8eJM9HPbnKcFWZ8RRhzzb
# 18Cpgt8w1aFsGBc3s82NMCU5RiRE1kiEe3 => 1KfjASdNiX97R8eJM9HPbnKcFWZ8RRhzzb
# 1MnEmUHF9TYXMio86dTGXvRxeirW4VFE9w => 1KfjASdNiX97R8eJM9HPbnKcFWZ8RRhzzb
# 1DL9Rt8bWZbmNbSMZedqA2WeFRUeJS415s => 1KfjASdNiX97R8eJM9HPbnKcFWZ8RRhzzb
# 1QC9jKo53vAdo6zYUcHwpJWqGwCe5voxLv => 1KfjASdNiX97R8eJM9HPbnKcFWZ8RRhzzb
# 1JtuJEr3cGL7AyB4xcg9qMLjWV73qnHRBt => 1KfjASdNiX97R8eJM9HPbnKcFWZ8RRhzzb
# 1G3HMZzCr4QDihJEpz1arrmR4swm7krinM => 1KfjASdNiX97R8eJM9HPbnKcFWZ8RRhzzb
# 1HziNRPCtiY6HXsBouwpVzTYtZws5A25LZ => 1KfjASdNiX97R8eJM9HPbnKcFWZ8RRhzzb
# 135xcKg36YPGi2c1yDuacgDJuxqWcporDv => 1KfjASdNiX97R8eJM9HPbnKcFWZ8RRhzzb
# 15MeKFbkdHp7357Ez3p1jnNdGSovoBKop6 => 1KfjASdNiX97R8eJM9HPbnKcFWZ8RRhzzb
# 12tiMQ2eFAsG5SwG1xYaejigbsXiqac6hx => 1KfjASdNiX97R8eJM9HPbnKcFWZ8RRhzzb
# 1GqU61rhQ6sbwLiTc4FYSNpKZyjEA827VV => 1KfjASdNiX97R8eJM9HPbnKcFWZ8RRhzzb
# 1ED1wLdA3WW7Xr3EPRhZH6LmWP7gwJP5Qj => 1KfjASdNiX97R8eJM9HPbnKcFWZ8RRhzzb
# 1PegsunSa7ThjbEoRHxxFa5M4BgfXjbAj1 => 1KfjASdNiX97R8eJM9HPbnKcFWZ8RRhzzb
# 14seLs1EwvsXTb3dLvchFgMquJnF2yJwd2 => 1KfjASdNiX97R8eJM9HPbnKcFWZ8RRhzzb
# 1NhBbVJJUPN7X451bN9FEEq4LxyWLSWcft => 1KfjASdNiX97R8eJM9HPbnKcFWZ8RRhzzb
# 12ZoxtSZVqApmRTmY75P6jYpYDLkPJzNai => 1KfjASdNiX97R8eJM9HPbnKcFWZ8RRhzzb
# 1NZHLx6jNqY3R3953j6oALgkkZF4VoM6rH => 1KfjASdNiX97R8eJM9HPbnKcFWZ8RRhzzb
# 14Efv8ZHhbFz1fJ3eD4tfKLyHVqAXMug7y => 1KfjASdNiX97R8eJM9HPbnKcFWZ8RRhzzb
# 12ahkRrYhBcRGT9GbGRc7Y8uMGe9WTLibF => 1KfjASdNiX97R8eJM9HPbnKcFWZ8RRhzzb
# 1TDLLyd684EqFHHwCuQuh5Hh1aEBZpEhv => 12Zv4sdQbm6bKpz5artZYoM8WbfFXKuTHC
# 16qGZVP5rgwQCeLSnf2U94dHBa3QMNHJ3F => 12Zv4sdQbm6bKpz5artZYoM8WbfFXKuTHC
# 1FnqQz36y18HK1EvaTCaGS65xUYDSTxBtx => 12Zv4sdQbm6bKpz5artZYoM8WbfFXKuTHC
# 1JyiMXW7NXjsyhp4hvsm77xFn3J4tvxuZa => 16fgrvY7ACrZDLNz9GWiwPfMuk34kZWKmf
# 1AEv2pKdGqJomxE9ApkW68QBg32QjjwA7b => 1BYrChoJn2UNMhTtWfRrqfcJ5ntkMYjTo8
# 1KgyqkYwq1uFMwc6MTeQKsz72jJfqHSBD9 => 1KLNYniYHM2UwYxSA7rjRtXqNFQEEUDhPv
# 1PUZQbeu94iarZtrkKXiL4ryAEsvcsEvcE => 13P4or5Dz8734Arqg2CQLXFtxy2DSjKdXa
# 1CbkZbefm25BYFeUUUQGEvt9HYWh39hvJk => 1M69AMjAkeKkk6YfuEECZzbz54EnXijzvk
# 13XnJ6zKd6qgK5Uu4zJw4bdPT8M7232ZBf => 1KfjASdNiX97R8eJM9HPbnKcFWZ8RRhzzb
# 1CRXTFk9AuPm3gVQAgZQZ3tWwn8CiZGqSR => 1D8Wr6cG2YTCaZL8oyRaUxogHnD3CJoS1D
# PbXSuic9B1iEmgMiWqW93cdXFvPQsHXdUc => PfSQYEJYKN3YTmk74BwXy6fk2StTJczaQw
# PitE7xxJvjzkVcs6BT5yRxV55YJqgFrhCU => PfSQYEJYKN3YTmk74BwXy6fk2StTJczaQw
# ==========