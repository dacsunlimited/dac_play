[![Gitter](https://badges.gitter.im/Join Chat.svg)](https://gitter.im/Bitsuperlab/bitshares_play?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)
BitShares Development Toolkit [![Build Status](https://travis-ci.org/Bitsuperlab/bitshares_play.png)](https://travis-ci.org/Bitsuperlab/bitshares_play)
===============================
The BitShares development toolkit is a set of libraries used to facilitate
the development of Decentralized Autonomous Companies (DACs).  It provides
a framework upon which new DACs can be developed based upon a common 
architecture.  

Build Instructions
------------------
BitShares Toolkit uses git submodules for managing certain external dependencies. Before
you can build you will need to fetch the submodules with the following commands:

    git submodule init
    git submodule update
    cmake .
    make

Different platforms have different steps for handling dependencies, if you 
would like to build on OS X see BUILD_OSX.md

Documentation
------------------
Documentation is available at the GitHub wiki: https://github.com/BitShares/bitshares_toolkit/wiki.
