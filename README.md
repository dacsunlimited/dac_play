DAC PLAY
===============================
DAC PLAY is a protocal and DAC designed to help coordinate voluntary decentralized game store and peer-to-peer game asset exchange. It is also DACSunlimited's open source implementation of BitShares PLAY, which is originated and based on Bitshares Blockchain technology.


More Info Go to WebSite
------------------
https://dacsunlimited.com/
http://www.bitsuperlab.com/pdf/BitSharesPlayWhitePaper.pdf

Build Instructions
------------------
Different platforms have different steps for handling dependencies, check specific documents
for more details:
=======

Additional information is available at [BitShares.org](http://bitshares.org/) and the [BitShares Wiki](http://wiki.bitshares.org/index.php/Main_Page). Community discussion occurs at [BitSharesTalk.org](https://bitsharestalk.org/).

Building
--------
Different platforms have different build instructions:
* [OS X](https://github.com/dacsunlimited/dac_play/blob/master/BUILD_OSX.md)
* [Ubuntu](https://github.com/dacsunlimited/dac_play/blob/master/BUILD_UBUNTU.md)
* [Windows](https://github.com/dacsunlimited/dac_play/blob/master/BUILD_WIN32.md)


Using the RPC server
--------------------

For many applications, it is useful to execute BitShares commands from scripts.  The BitShares client includes RPC server functionality to allow programs to submit JSON-formatted commands and retrieve JSON-formatted results over an HTTP connection.  To enable the RPC server, you can edit the `rpc` section of `config.json` as follows:

      "rpc": {
        "enable": true,
        "rpc_user": "USERNAME",
        "rpc_password": "PASSWORD",
        "rpc_endpoint": "127.0.0.1:1775",
        "httpd_endpoint": "127.0.0.1:1776",

Here, `USERNAME` and `PASSWORD` are authentication credentials which must be presented by a client to gain access to the RPC interface.  These parameters may also be specified on the command line, but this is not recommended because some popular multi-user operating systems (Linux in particular) allow command line parameters of running programs to be visible to all users.

After editing the configuration file and (re)starting the BitShares client, you can use any HTTP client to POST a JSON object and read the JSON response.  Here is an example using the popular `curl` command line HTTP client:

    curl --user USERNAME:PASSWORD http://127.0.0.1:1776/rpc -X POST -H 'Content-Type: application/json' -d '{"method" : "blockchain_get_account", "params" : ["dev0.theoretical"], "id" : 1}'

The POST request returns a JSON result like this (some data elided for brevity):

    {"id":1,"result":{"id":31427,"name":"dev0.theoretical","public_data":{"version":"v0.4.27.1"},"owner_key":"PLS75vj8aaDWFwg7Wd6WinAAqVddUcSRJ1hSMDNayLAbCuxsmoQTf", ...},"meta_data":{"type":"public_account","data":""}}}

Since HTTP basic authentication is used, the authentication credentials are sent over the socket in unencrypted plaintext.
For this reason, binding to an interface other than localhost in the configuration file is not recommended.
If you wish to access the RPC interface from a remote system, you should establish a secure connection using SSH port forwarding (the `-L` option in OpenSSH) or a reverse proxy SSL/TLS tunnel (typically supported by general-purpose webservers such as `nginx`).

Please keep in mind that anyone able to connect to the RPC socket with the correct username and password will be able to access all funds, accounts and private keys in any open wallet (including wallets opened manually or by another RPC client connected to the same `bitshares_client` instance).
Thus, your security procedures should protect the username, password, and socket accordingly (including `config.json` since it contains the username and password)!

Contributing
------------
BitShares source code can always be found at the [BitShares GitHub Repository](https://github.com/BitShares/bitshares). There are four main branches:
- `master` - official BitShares releases are tagged from here; this should only change for a new release
- `test` - for testing
- `develop` - all new development happens here; this is what is used for internal BitShares XTS test networks

Some technical documentation is available at the [BitShares GitHub Wiki](https://github.com/BitShares/bitshares/wiki).

Support
-------
Bugs can be reported directly to the [DAC PLAY Issue Tracker](https://github.com/dacsunlimited/dac_play/issues).

Technical support can be obtained from the [PlayTalk Technical Support Forum](http://playtalk.org/).

License
-------
The BitShares source code is in the public domain under the Unlicense. See the [LICENSE](https://github.com/BitShares/bitshares/blob/master/LICENSE.txt) for more information.
